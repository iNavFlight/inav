/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

#include "fx_stm32_levelx_nand_driver.h"

/* define the struct used to identify the levelx driver to instantiate */
struct fx_lx_nand_driver_instance
{
    LX_NAND_FLASH flash_instance;

    CHAR name[32];

    UINT id;

    UINT (*nand_driver_initialize)(LX_NAND_FLASH *);

    UINT initialized;

};

static struct fx_lx_nand_driver_instance  fx_lx_nand_drivers[MAX_LX_NAND_DRIVERS] =
{
#ifdef LX_NAND_SIMULATOR_DRIVER
    { .name = LX_NAND_SIMULATOR_DRIVER_NAME, .id = LX_NAND_SIMULATOR_DRIVER_ID, .nand_driver_initialize = lx_stm32_nand_simulator_initialize},
#endif

#ifdef LX_NAND_CUSTOM_DRIVER
    LX_NAND_CUSTOM_DRIVERS
#endif
};

static struct fx_lx_nand_driver_instance *current_driver = NULL;

/* Exported constants --------------------------------------------------------*/
static ULONG  num_drivers = sizeof(fx_lx_nand_drivers)/sizeof(fx_lx_nand_drivers[0]);

/* Exported functions ------------------------------------------------------- */

static UINT find_driver_id(UINT driver_id)
{
    UINT i = 0;

    for (i = 0; i < num_drivers; i++)
    {
        if (fx_lx_nand_drivers[i].id == driver_id)
            return i;
    }

    return UNKNOWN_DRIVER_ID;
}

VOID  fx_stm32_levelx_nand_driver(FX_MEDIA *media_ptr)
{
    ULONG i;
    UINT status;
    UCHAR *source_buffer;
    UCHAR *destination_buffer;
    ULONG logical_sector;


    /* Process the driver request specified in the media control block.*/
#ifdef USE_LX_NAND_DEFAULT_DRIVER
    i = find_driver_id(NAND_DEFAULT_DRIVER);
#else
    if (media_ptr->fx_media_driver_info == NULL)
    {
        i = UNKNOWN_DRIVER_ID;
    }
    else
    {
        i = find_driver_id((UINT)media_ptr->fx_media_driver_info);
    }

#endif

    if (i == UNKNOWN_DRIVER_ID)
    {
        /* No Driver found return an error */
        media_ptr->fx_media_driver_status = FX_MEDIA_INVALID;
        return;
    }
    else
    {
        current_driver = &fx_lx_nand_drivers[i];
    }

    switch(media_ptr->fx_media_driver_request)
    {

        case FX_DRIVER_INIT:
            {
                if (current_driver->initialized == FX_FALSE)
                {
                    /* Open flash instance*/
                    status = lx_nand_flash_open(&current_driver->flash_instance, current_driver->name, current_driver->nand_driver_initialize);

                    /* LevelX driver correctly initialized */
                    if (status == LX_SUCCESS)
                    {
                        current_driver->initialized = FX_TRUE;
                        media_ptr->fx_media_driver_status = FX_SUCCESS;

                        media_ptr->fx_media_driver_free_sector_update = FX_TRUE;
                        break;
                    }
                    else
                    {
                        media_ptr->fx_media_driver_status = FX_IO_ERROR;
                    }
                }
                else
                {
                    media_ptr->fx_media_driver_status = FX_SUCCESS;
                }

                break;
            }

        case FX_DRIVER_UNINIT:
            {
                /* Successful driver */
                status = lx_nand_flash_close(&current_driver->flash_instance);

                if (status == LX_SUCCESS)
                {
                    media_ptr->fx_media_driver_status =  FX_SUCCESS;
                }
                else
                {
                    media_ptr->fx_media_driver_status =  FX_IO_ERROR;
                }

                break;
            }

        case FX_DRIVER_READ:
            {
                /* Setup the destination buffer and logical sector.  */
                logical_sector = media_ptr->fx_media_driver_logical_sector;
                destination_buffer =(UCHAR *)media_ptr->fx_media_driver_buffer;

                /* Loop to read sectors from flash.  */
                for (i = 0; i < media_ptr->fx_media_driver_sectors; i++)
                {

                    /* Read a sector from NAND flash.  */
                    status =  lx_nand_flash_sector_read(&current_driver->flash_instance, logical_sector, destination_buffer);

                    /* Determine if the read was successful.  */
                    if (status != LX_SUCCESS)
                    {

                        /* Return an I/O error to FileX.  */
                        media_ptr->fx_media_driver_status = FX_IO_ERROR;

                        return;
                    }

                    /* Move to the next entries.  */
                    logical_sector++;
                    destination_buffer = destination_buffer + media_ptr->fx_media_bytes_per_sector;
                }

                /* Successful driver request.  */
                media_ptr->fx_media_driver_status =  FX_SUCCESS;

                break;
            }

        case FX_DRIVER_BOOT_READ:
            {

                /* Read the boot record and return to the caller.  */

                /* Setup the destination buffer.  */
                destination_buffer =  (UCHAR *) media_ptr -> fx_media_driver_buffer;

                /* Read boot sector from NAND flash.  */
                status =  lx_nand_flash_sector_read(&current_driver->flash_instance, 0, destination_buffer);

                /* Determine if the boot read was successful.  */
                if (status != LX_SUCCESS)
                {
                    /* Return an I/O error to FileX.  */
                    media_ptr -> fx_media_driver_status =  FX_IO_ERROR;

                    return;
                }

                /* Successful driver request.  */
                media_ptr -> fx_media_driver_status =  FX_SUCCESS;
                break;
            }

        case FX_DRIVER_WRITE:
            {
                /* Setup the source buffer and logical sector.  */
                logical_sector = media_ptr->fx_media_driver_logical_sector;
                source_buffer = (UCHAR *) media_ptr->fx_media_driver_buffer;

                /* Loop to write sectors to flash.  */
                for (i = 0; i < media_ptr->fx_media_driver_sectors; i++)
                {
                    /* Write a sector to NAND flash.  */
                    status =  lx_nand_flash_sector_write(&current_driver->flash_instance, logical_sector, source_buffer);

                    /* Determine if the write was successful.  */
                    if (status != LX_SUCCESS)
                    {
                        /* Return an I/O error to FileX.  */
                        media_ptr->fx_media_driver_status =  FX_IO_ERROR;
                        return;
                    }

                    /* Move to the next entries.  */
                    logical_sector++;
                    source_buffer =  source_buffer + media_ptr->fx_media_bytes_per_sector;
                }

                /* Successful driver request.  */
                media_ptr->fx_media_driver_status =  FX_SUCCESS;
                break;
            }

        case FX_DRIVER_BOOT_WRITE:
            {

                /* Write the boot record and return to the caller.  */

                /* Setup the source buffer.  */
                source_buffer =       (UCHAR *) media_ptr -> fx_media_driver_buffer;

                /* Write boot sector to NAND flash.  */
                status =  lx_nand_flash_sector_write(&current_driver->flash_instance, 0, source_buffer);

                /* Determine if the boot write was successful.  */
                if (status != LX_SUCCESS)
                {

                    /* Return an I/O error to FileX.  */
                    media_ptr -> fx_media_driver_status =  FX_IO_ERROR;

                    return;
                }

                /* Successful driver request.  */
                media_ptr -> fx_media_driver_status =  FX_SUCCESS;
                break ;
            }
        case FX_DRIVER_RELEASE_SECTORS:
            {
                /* Setup the logical sector.  */
                logical_sector =  media_ptr->fx_media_driver_logical_sector;

                /* Release sectors.  */
                for (i = 0; i < media_ptr->fx_media_driver_sectors; i++)
                {
                    /* Release NAND flash sector.  */
                    status = lx_nand_flash_sector_release(&current_driver->flash_instance, logical_sector);

                    /* Determine if the sector release was successful.  */
                    if (status != LX_SUCCESS)
                    {
                        /* Return an I/O error to FileX.  */
                        media_ptr->fx_media_driver_status = FX_IO_ERROR;
                        return;
                    }

                    /* Move to the next entries.  */
                    logical_sector++;
                }

                /* Successful driver request.  */
                media_ptr->fx_media_driver_status =  FX_SUCCESS;
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
                /* Return driver success.  */
                media_ptr->fx_media_driver_status =  FX_SUCCESS;
                break;
            }

        default:
            {
                /* Invalid driver request.  */
                media_ptr->fx_media_driver_status =  FX_IO_ERROR;
                break;
            }
    }
}
