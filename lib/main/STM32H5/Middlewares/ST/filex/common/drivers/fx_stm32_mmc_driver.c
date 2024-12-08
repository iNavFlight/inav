/**************************************************************************/
/*                                                                        */
/*       Partial Copyright (c) Microsoft Corporation. All rights reserved.*/
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*      Partial Copyright (c) STMicroelctronics 2020. All rights reserved */
/**************************************************************************/


/* Include necessary system files.  */
#include "fx_stm32_mmc_driver.h"

/*
 * the scratch buffer is required when performing DMA transfers using unaligned addresses
 * When CPU cache is enabled, the scratch buffer should be 32-byte aligned to match a whole cache line
 * otherwise it is 4-byte aligned to match the DMA alignment constraints
 */

#if (FX_STM32_MMC_CACHE_MAINTENANCE == 1)
static UCHAR scratch[FX_STM32_MMC_DEFAULT_SECTOR_SIZE] __attribute__ ((aligned (32)));
#else
static UCHAR scratch[FX_STM32_MMC_DEFAULT_SECTOR_SIZE] __attribute__ ((aligned (4)));
#endif

UINT  _fx_partition_offset_calculate(void  *partition_sector, UINT partition, ULONG *partition_start, ULONG *partition_size);

static UINT mmc_read_data(FX_MEDIA *media_ptr, ULONG sector, UINT num_sectors, UINT use_scratch_buffer);
static UINT mmc_write_data(FX_MEDIA *media_ptr, ULONG sector, UINT num_sectors, UINT use_scratch_buffer);

static UINT is_initialized = 0;


static INT check_mmc_status(uint32_t instance)
{
  uint32_t start = FX_STM32_MMC_CURRENT_TIME();

  while (FX_STM32_MMC_CURRENT_TIME() - start < FX_STM32_MMC_DEFAULT_TIMEOUT)
  {
    if (fx_stm32_mmc_get_status(instance) == 0)
    {
      return 0;
    }
  }

  return 1;
}

/**
* @brief This function is the entry point to the STM32 SDIO disk driver.
* It relies on the STM32 peripheral library from ST.
* @param media_ptr: FileX's Media Config Block
* @retval None
*/
VOID  fx_stm32_mmc_driver(FX_MEDIA *media_ptr)
{
  UINT status;
  UINT unaligned_buffer;
  ULONG partition_start;
  ULONG partition_size;

#if (FX_STM32_MMC_INIT == 0)
 /* the SD  was initialized by the application */
  is_initialized = 1;
#endif
  /* before performing any operation, check the status of the SDMMC */
  if (is_initialized == 1)
  {
    if (check_mmc_status(FX_STM32_MMC_INSTANCE) != 0)
    {
      media_ptr->fx_media_driver_status =  FX_IO_ERROR;
      return;
    }
  }

#if (FX_STM32_MMC_DMA_API == 1)
  /* the SD DMA requires a 4-byte aligned buffers */
  unaligned_buffer = (UINT)(media_ptr->fx_media_driver_buffer) & 0x3;
#else
  /* if the DMA is not used there isn't any constraint on buffer alignment */
  unaligned_buffer = 0;
#endif
  /* Process the driver request specified in the media control block.  */
  switch(media_ptr->fx_media_driver_request)
  {
  case FX_DRIVER_INIT:
    {
      media_ptr->fx_media_driver_status = FX_SUCCESS;

      FX_STM32_MMC_PRE_INIT(media_ptr);

#if (FX_STM32_MMC_INIT == 1)
      /* Initialize the SD instance */
      if (is_initialized == 0)
      {
        status = fx_stm32_mmc_init(FX_STM32_MMC_INSTANCE);

        if (status == 0)
        {
          is_initialized = 1;
        }
        else
        {
          media_ptr->fx_media_driver_status =  FX_IO_ERROR;
        }
      }
#endif
      /* call post init user macro */
      FX_STM32_MMC_POST_INIT(media_ptr);
      break;
    }

  case FX_DRIVER_UNINIT:
    {
      media_ptr->fx_media_driver_status = FX_SUCCESS;

#if (FX_STM32_MMC_INIT == 1)
      status = fx_stm32_mmc_deinit(FX_STM32_MMC_INSTANCE);

      if (status == 0)
      {
        is_initialized = 0;
      }
      else
      {
        media_ptr->fx_media_driver_status = FX_IO_ERROR;
      }
#endif
      /* call post deinit processing */
      FX_STM32_MMC_POST_DEINIT(media_ptr);

      break;
    }

  case FX_DRIVER_READ:
    {
      media_ptr->fx_media_driver_status = FX_IO_ERROR;

      if (mmc_read_data(media_ptr, media_ptr->fx_media_driver_logical_sector + media_ptr->fx_media_hidden_sectors,
                       media_ptr->fx_media_driver_sectors, unaligned_buffer) == FX_SUCCESS)
      {
        media_ptr->fx_media_driver_status = FX_SUCCESS;
      }

      break;
    }

  case FX_DRIVER_WRITE:
    {
      media_ptr->fx_media_driver_status = FX_IO_ERROR;

      if (mmc_write_data(media_ptr, media_ptr->fx_media_driver_logical_sector + media_ptr->fx_media_hidden_sectors,
                        media_ptr->fx_media_driver_sectors, unaligned_buffer) == FX_SUCCESS)
      {
        media_ptr->fx_media_driver_status = FX_SUCCESS;
      }

      break;
    }

  case FX_DRIVER_FLUSH:
    {
      /* Return driver success.  */
      media_ptr->fx_media_driver_status =  FX_SUCCESS;
      break;
    }

  case FX_DRIVER_ABORT:
    {
      media_ptr->fx_media_driver_status =  FX_SUCCESS;
      /* call post driver abort macro*/

      FX_STM32_MMC_POST_ABORT(media_ptr);

      break;
    }

  case FX_DRIVER_BOOT_READ:
    {
      /* the boot sector is the sector zero */
      status = mmc_read_data(media_ptr, 0, media_ptr->fx_media_driver_sectors, unaligned_buffer);

      if (status != FX_SUCCESS)
      {
        media_ptr->fx_media_driver_status = status;
        break;
      }

      /* Check if the sector 0 is the actual boot sector, otherwise calculate the offset into it.
      Please note that this should belong to higher level of MW to do this check and it is here
      as a temporary work solution */

      partition_start =  0;

      status =  _fx_partition_offset_calculate(media_ptr -> fx_media_driver_buffer, 0,
                                               &partition_start, &partition_size);

      /* Check partition read error.  */
      if (status)
      {
        /* Unsuccessful driver request.  */
        media_ptr -> fx_media_driver_status =  FX_IO_ERROR;
        break;
      }

      /* Now determine if there is a partition...   */
      if (partition_start)
      {

        if (check_mmc_status(FX_STM32_MMC_INSTANCE) != 0)
        {
          media_ptr->fx_media_driver_status =  FX_IO_ERROR;
          break;
        }

        /* Yes, now lets read the actual boot record.  */
        status = mmc_read_data(media_ptr, partition_start, media_ptr->fx_media_driver_sectors, unaligned_buffer);

        if (status != FX_SUCCESS)
        {
          media_ptr->fx_media_driver_status = status;
          break;
        }
      }

      /* Successful driver request.  */
      media_ptr -> fx_media_driver_status =  FX_SUCCESS;
      break;
    }

  case FX_DRIVER_BOOT_WRITE:
    {
      status = mmc_write_data(media_ptr, 0, 1, unaligned_buffer);

      media_ptr->fx_media_driver_status = status;

      break;
    }

  default:
    {
      media_ptr->fx_media_driver_status =  FX_IO_ERROR;
      break;
    }
  }
}

/**
* @brief Read data from uSD into destination buffer
* @param FX_MEDIA *media_ptr a pointer the main FileX structure
* @param ULONG start_sector first sector to start reading from
* @param UINT num_sectors number of sectors to be read
* @param UINT use_scratch_buffer to enable scratch buffer usage or not.
* @retval FX_SUCCESS on success FX_BUFFER_ERROR / FX_ACCESS_ERROR / FX_IO_ERROR otherwise
*/

static UINT mmc_read_data(FX_MEDIA *media_ptr, ULONG start_sector, UINT num_sectors, UINT use_scratch_buffer)
{
  INT i = 0;
  UINT status;
  UCHAR *read_addr;

 /* perform the Pre read operations */
  FX_STM32_MMC_PRE_READ_TRANSFER(media_ptr);

  if (use_scratch_buffer)
  {
    read_addr = media_ptr->fx_media_driver_buffer;

    for (i = 0; i < num_sectors; i++)
    {
      /* Start reading into the scratch buffer */
      status = fx_stm32_mmc_read_blocks(FX_STM32_MMC_INSTANCE, (UINT *)scratch, (UINT)start_sector++, 1);

      if (status != 0)
      {
        /* read error occurred, call the error handler code then return immediately */
        FX_STM32_MMC_READ_TRANSFER_ERROR(status);
        return FX_IO_ERROR;
      }

    /* wait for read transfer notification */
       FX_STM32_MMC_READ_CPLT_NOTIFY();

#if (FX_STM32_MMC_CACHE_MAINTENANCE == 1)
      invalidate_cache_by_addr((uint32_t*)scratch, FX_STM32_MMC_DEFAULT_SECTOR_SIZE);
#endif

      _fx_utility_memory_copy(scratch, read_addr, FX_STM32_MMC_DEFAULT_SECTOR_SIZE);
      read_addr += FX_STM32_MMC_DEFAULT_SECTOR_SIZE;
    }

    /* Check if all sectors were read */
    if (i == num_sectors)
    {
      status = FX_SUCCESS;
    }
    else
    {
      status = FX_BUFFER_ERROR;
    }
  }
  else
  {

    status = fx_stm32_mmc_read_blocks(FX_STM32_MMC_INSTANCE, (UINT *)media_ptr->fx_media_driver_buffer, (UINT)start_sector, num_sectors);

    if (status != 0)
    {
      /* read error occurred, call the error handler code then return immediately */
      FX_STM32_MMC_READ_TRANSFER_ERROR(status);

      return FX_IO_ERROR;
    }

    /* wait for read transfer notification */
       FX_STM32_MMC_READ_CPLT_NOTIFY();

#if (FX_STM32_MMC_CACHE_MAINTENANCE == 1)
    invalidate_cache_by_addr((uint32_t*)media_ptr->fx_media_driver_buffer, num_sectors * FX_STM32_MMC_DEFAULT_SECTOR_SIZE);
#endif

    status = FX_SUCCESS;
  }

  /* Operation finished, call the post read macro if defined */

  FX_STM32_MMC_POST_READ_TRANSFER(media_ptr);
  return status;
}

/**
* @brief write data buffer into the uSD
* @param FX_MEDIA *media_ptr a pointer the main FileX structure
* @param ULONG start_sector first sector to start writing from
* @param UINT num_sectors number of sectors to be written
* @param UINT use_scratch_buffer to enable scratch buffer usage or not.
* @retval FX_SUCCESS on success FX_BUFFER_ERROR / FX_ACCESS_ERROR / FX_IO_ERROR otherwise
*/

static UINT mmc_write_data(FX_MEDIA *media_ptr, ULONG start_sector, UINT num_sectors, UINT use_scratch_buffer)
{
  INT i = 0;
  UINT status;
  UCHAR *write_addr;

  /* call Pre write operation macro */
  FX_STM32_MMC_PRE_WRITE_TRANSFER(media_ptr);

  if (use_scratch_buffer)
  {
    write_addr = media_ptr->fx_media_driver_buffer;

    for (i = 0; i < num_sectors; i++)
    {
      _fx_utility_memory_copy(write_addr, scratch, FX_STM32_MMC_DEFAULT_SECTOR_SIZE);
      write_addr += FX_STM32_MMC_DEFAULT_SECTOR_SIZE;

#if (FX_STM32_MMC_CACHE_MAINTENANCE == 1)
      /* Clean the DCache to make the SD DMA see the actual content of the scratch buffer */
      clean_cache_by_addr((uint32_t*)scratch, FX_STM32_MMC_DEFAULT_SECTOR_SIZE);
#endif

      status = fx_stm32_mmc_write_blocks(FX_STM32_MMC_INSTANCE, (UINT *)scratch, (UINT)start_sector++, 1);

      if (status != 0)
      {
        /* in case of error call the error handling macro */
        FX_STM32_MMC_WRITE_TRANSFER_ERROR(status);
        return FX_IO_ERROR;
      }

      /* call the write completion notification macro */
       FX_STM32_MMC_WRITE_CPLT_NOTIFY();
    }

    if (i == num_sectors)
    {
      status = FX_SUCCESS;
    }
    else
    {
      status = FX_BUFFER_ERROR;
    }
  }
  else
  {
#if (FX_STM32_MMC_CACHE_MAINTENANCE == 1)
    clean_cache_by_addr((uint32_t*)media_ptr->fx_media_driver_buffer, num_sectors * FX_STM32_MMC_DEFAULT_SECTOR_SIZE);
#endif
    status = fx_stm32_mmc_write_blocks(FX_STM32_MMC_INSTANCE, (UINT *)media_ptr->fx_media_driver_buffer, (UINT)start_sector, num_sectors);

    if (status != 0)
    {
      FX_STM32_MMC_WRITE_TRANSFER_ERROR(status);
      return FX_IO_ERROR;
    }

    /* when defined, wait for the write notification */
     FX_STM32_MMC_WRITE_CPLT_NOTIFY();

    status = FX_SUCCESS;
  }

  /* perform post write operations */
  FX_STM32_MMC_POST_WRITE_TRANSFER(media_ptr);


  return status;
}
