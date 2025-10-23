/**
  ******************************************************************************
  * @file    sd_diskio_dma_rtos_template_bspv2.c
  * @author  MCD Application Team
  * @brief   SD Disk I/O DMA with RTOS driver template using the BSPv2 API.
  *          This file needs to be copied at user project alongside
  *          the respective header file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017-2019 STMicroelectronics. All rights reserved.
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                       opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
**/

/* Includes ------------------------------------------------------------------*/
#include "ff_gen_drv.h"
#include "sd_diskio_dma_rtos.h"

#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define QUEUE_SIZE         (uint32_t) 10
#define READ_CPLT_MSG      (uint32_t) 1
#define WRITE_CPLT_MSG     (uint32_t) 2
#define RW_ABORT_MSG       (uint32_t) 3
/*
 * the following Timeout is useful to give the control back to the applications
 * in case of errors in either BSP_SD_ReadCpltCallback() or BSP_SD_WriteCpltCallback()
 * the value by default is as defined in the BSP platform driver otherwise 30 secs
 *
 */

#define SD_TIMEOUT 30 * 1000

#define SD_DEFAULT_BLOCK_SIZE 512

#ifndef BSP_SD_INSTANCE
#define BSP_SD_INSTANCE 0
#endif

/*
 * Depending on the usecase, the SD card initialization could be done at the
 * application level, if it is the case define the flag below to disable
 * the BSP_SD_Init() call in the SD_Initialize().
 */

#define DISABLE_SD_INIT


/*
 * when using cachable memory region, it may be needed to maintain the cache
 * validity. Enable the define below to activate a cache maintenance at each
 * read and write operation.
 * Notice: This is applicable only for cortex M7 based platform.
 */

/* #define ENABLE_SD_DMA_CACHE_MAINTENANCE  1 */

/*
* Some DMA requires 4-Byte aligned address buffer to correctly read/wite data,
* in FatFs some accesses aren't thus we need a 4-byte aligned scratch buffer to correctly
* transfer data
*/
/* #define ENABLE_SCRATCH_BUFFER */

/* Private variables ---------------------------------------------------------*/

#if defined(ENABLE_SCRATCH_BUFFER)
#if defined (ENABLE_SD_DMA_CACHE_MAINTENANCE)
ALIGN_32BYTES(static uint8_t scratch[BLOCKSIZE]); // 32-Byte aligned for cache maintenance
#else
__ALIGN_BEGIN static uint8_t scratch[BLOCKSIZE] __ALIGN_END;
#endif
#endif

/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;
#if (osCMSIS <= 0x20000U)
static osMessageQId SDQueueID = NULL;
#else
static osMessageQueueId_t SDQueueID = NULL;
#endif
/* Private function prototypes -----------------------------------------------*/
static DSTATUS SD_CheckStatus(BYTE lun);
DSTATUS SD_initialize (BYTE);
DSTATUS SD_status (BYTE);
DRESULT SD_read (BYTE, BYTE*, DWORD, UINT);
#if _USE_WRITE == 1
  DRESULT SD_write (BYTE, const BYTE*, DWORD, UINT);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
  DRESULT SD_ioctl (BYTE, BYTE, void*);
#endif  /* _USE_IOCTL == 1 */

const Diskio_drvTypeDef  SD_Driver =
{
  SD_initialize,
  SD_status,
  SD_read,
#if  _USE_WRITE == 1
  SD_write,
#endif /* _USE_WRITE == 1 */

#if  _USE_IOCTL == 1
  SD_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* Private functions ---------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
static int SD_CheckStatusWithTimeout(uint32_t timeout)
{
  uint32_t timer;
#if (osCMSIS < 0x20000U)
  timer = osKernelSysTick();
  /* block until SDIO IP is ready or a timeout occur */
  while(osKernelSysTick() - timer <timeout)
#else
    timer = osKernelGetTickCount();
  /* block until SDIO IP is ready or a timeout occur */
  while(osKernelGetTickCount() - timer <timeout)
#endif
  {
    if (BSP_SD_GetCardState(BSP_SD_INSTANCE) == SD_TRANSFER_OK)
    {
      return 0;
    }
  }

  return -1;
}

static DSTATUS SD_CheckStatus(BYTE lun)
{
  Stat = STA_NOINIT;

  if(BSP_SD_GetCardState(BSP_SD_INSTANCE) == SD_TRANSFER_OK)
  {
    Stat &= ~STA_NOINIT;
  }

  return Stat;
}

/**
  * @brief  Initializes a Drive
  * @param  lun : not used
  * @retval DSTATUS: Operation status
  */
DSTATUS SD_initialize(BYTE lun)
{
  Stat = STA_NOINIT;
  /*
   * check that the kernel has been started before continuing
   * as the osMessage API will fail otherwise
   */
#if (osCMSIS <= 0x20000U)
  if(osKernelRunning())
#else
  if(osKernelGetState() == osKernelRunning)
#endif
  {
#if !defined(DISABLE_SD_INIT)

    if(BSP_SD_Init(BSP_SD_INSTANCE) == BSP_ERROR_NONE)
    {
      Stat = SD_CheckStatus(lun);
    }

#else
    Stat = SD_CheckStatus(lun);
#endif

    /*
     * if the SD is correctly initialized, create the operation queue
     */

    if (Stat != STA_NOINIT)
    {
      if (SDQueueID == NULL)
      {
 #if (osCMSIS <= 0x20000U)
      osMessageQDef(SD_Queue, QUEUE_SIZE, uint16_t);
      SDQueueID = osMessageCreate (osMessageQ(SD_Queue), NULL);
#else
      SDQueueID = osMessageQueueNew(QUEUE_SIZE, 2, NULL);
#endif
      }

      if (SDQueueID == NULL)
      {
        Stat |= STA_NOINIT;
      }
    }
  }

  return Stat;
}

/**
  * @brief  Gets Disk Status
  * @param  lun : not used
  * @retval DSTATUS: Operation status
  */
DSTATUS SD_status(BYTE lun)
{
  return SD_CheckStatus(lun);
}

/**
  * @brief  Reads Sector(s)
  * @param  lun : not used
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT SD_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
  int32_t ret;
  DRESULT res = RES_ERROR;
  uint32_t timer;

#if (osCMSIS < 0x20000U)
  osEvent event;
#else
  uint16_t event;
  osStatus_t status;
#endif

  /*
  * ensure the SDCard is ready for a new operation
  */

  if (SD_CheckStatusWithTimeout(SD_TIMEOUT) < 0)
  {
    return res;
  }

#if defined(ENABLE_SCRATCH_BUFFER)
  /* Check if buffer currently used is aligned on 32 bytes address */
  if (!((uint32_t)buff & 0x1F))
  {
#endif
  ret = BSP_SD_ReadBlocks_DMA(BSP_SD_INSTANCE, (uint32_t*)buff, (uint32_t) (sector), count);
  if (ret == BSP_ERROR_NONE)
  {
#if (osCMSIS < 0x20000U)
    /* wait for a message from the queue or a timeout */
    event = osMessageGet(SDQueueID, SD_TIMEOUT);

    if (event.status == osEventMessage)
    {
      if (event.value.v == READ_CPLT_MSG)
      {
        timer = osKernelSysTick();
        /* block until SDIO IP is ready or a timeout occur */
        while(osKernelSysTick() - timer <SD_TIMEOUT)
#else
    status = osMessageQueueGet(SDQueueID, (void *)&event, NULL, SD_TIMEOUT);
    if ((status == osOK) && (event == READ_CPLT_MSG))
    {
      timer = osKernelGetTickCount();
      /* block until SDIO IP is ready or a timeout occur */
      while(osKernelGetTickCount() - timer <SD_TIMEOUT)
#endif
        {
          if (BSP_SD_GetCardState(BSP_SD_INSTANCE) == SD_TRANSFER_OK)
          {
            res = RES_OK;
#if (ENABLE_SD_DMA_CACHE_MAINTENANCE == 1)
            /*
               the SCB_InvalidateDCache_by_Addr() requires a 32-Byte aligned address,
               adjust the address and the D-Cache size to invalidate accordingly.
             */
            SCB_InvalidateDCache_by_Addr((uint32_t*)buff, count*BLOCKSIZE);
#endif
            break;
          }
        }
#if (osCMSIS < 0x20000U)
      }
    }
#else
    }
#endif
   }
#if defined(ENABLE_SCRATCH_BUFFER)
  }
  else {
    /* Slow path, fetch each sector a part and memcpy to destination buffer */
    int i;

    for (i = 0; i < count; i++)
    {
      ret = BSP_SD_ReadBlocks_DMA(BSP_SD_INSTANCE, (uint32_t*)scratch, (uint32_t)sector++, 1);
      if (ret == BSP_ERROR_NONE)
      {
        /* wait until the read is successful or a timeout occurs */
#if (osCMSIS < 0x20000U)
        /* wait for a message from the queue or a timeout */
        event = osMessageGet(SDQueueID, SD_TIMEOUT);

        if (event.status == osEventMessage)
        {
          if (event.value.v == READ_CPLT_MSG)
          {
            timer = osKernelSysTick();
            /* block until SDIO IP is ready or a timeout occur */
            while(osKernelSysTick() - timer <SD_TIMEOUT)
#else
          status = osMessageQueueGet(SDQueueID, (void *)&event, NULL, SD_TIMEOUT);
          if ((status == osOK) && (event == READ_CPLT_MSG))
          {
            timer = osKernelGetTickCount();
            /* block until SDIO IP is ready or a timeout occur */
            ret = BSP_ERROR_BUSY;
            while(osKernelGetTickCount() - timer < SD_TIMEOUT)
#endif
            {
              ret = BSP_SD_GetCardState(BSP_SD_INSTANCE);

              if (ret == BSP_ERROR_NONE)
              {
                break;
              }
            }

            if (ret != BSP_ERROR_NONE)
            {
              break;
            }
#if (osCMSIS < 0x20000U)
      }
    }
#else
    }
#endif
#if (ENABLE_SD_DMA_CACHE_MAINTENANCE == 1)
          /*
          *
          * invalidate the scratch buffer before the next read to get the actual data instead of the cached one
          */
          SCB_InvalidateDCache_by_Addr((uint32_t*)scratch, BLOCKSIZE);
#endif
          memcpy(buff, scratch, BLOCKSIZE);
          buff += BLOCKSIZE;
        }
        else
        {
          break;
        }
      }

      if ((i == count) && (ret == BSP_ERROR_NONE ))
        res = RES_OK;
    }
#endif
  return res;
}

/**
  * @brief  Writes Sector(s)
  * @param  lun : not used
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1
DRESULT SD_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
  int32_t ret;
  DRESULT res = RES_ERROR;
  uint32_t timer;

#if (osCMSIS < 0x20000U)
  osEvent event;
#else
  uint16_t event;
  osStatus_t status;
#endif

  /*
  * ensure the SDCard is ready for a new operation
  */

  if (SD_CheckStatusWithTimeout(SD_TIMEOUT) < 0)
  {
    return res;
  }

#if defined(ENABLE_SCRATCH_BUFFER)
  /* Check if buffer currently used is aligned on 32 bytes address */
  if (!((uint32_t)buff & 0x1F))
  {
#endif
#if (ENABLE_SD_DMA_CACHE_MAINTENANCE == 1)
  /*
    the SCB_CleanDCache_by_Addr() requires a 32-Byte aligned address
    adjust the address and the D-Cache size to clean accordingly.
  */
  SCB_CleanDCache_by_Addr((uint32_t*)buff, count*BLOCKSIZE);
#endif


  ret = BSP_SD_WriteBlocks_DMA(BSP_SD_INSTANCE, (uint32_t*)buff, (uint32_t) (sector), count);
  if (ret == BSP_ERROR_NONE)
  {
#if (osCMSIS < 0x20000U)
    /* Get the message from the queue */
    event = osMessageGet(SDQueueID, SD_TIMEOUT);

    if (event.status == osEventMessage)
    {
      if (event.value.v == WRITE_CPLT_MSG)
      {
#else
    status = osMessageQueueGet(SDQueueID, (void *)&event, NULL, SD_TIMEOUT);
    if ((status == osOK) && (event == WRITE_CPLT_MSG))
    {
#endif
 #if (osCMSIS < 0x20000U)
        timer = osKernelSysTick();
        /* block until SDIO IP is ready or a timeout occur */
        while(osKernelSysTick() - timer  < SD_TIMEOUT)
#else
        timer = osKernelGetTickCount();
        /* block until SDIO IP is ready or a timeout occur */
        while(osKernelGetTickCount() - timer  < SD_TIMEOUT)
#endif
        {
          if (BSP_SD_GetCardState(BSP_SD_INSTANCE) == SD_TRANSFER_OK)
          {
            res = RES_OK;
            break;
          }
        }
#if (osCMSIS < 0x20000U)
      }
    }
#else
    }
#endif
  }
#if defined(ENABLE_SCRATCH_BUFFER)
  }
  else
  {
    /* Slow path, fetch each sector a part and memcpy to destination buffer */
    int i;

#if (ENABLE_SD_DMA_CACHE_MAINTENANCE == 1)
    /*
     * invalidate the scratch buffer before the next write to get the actual data instead of the cached one
     */
     SCB_InvalidateDCache_by_Addr((uint32_t*)scratch, BLOCKSIZE);
#endif
    for (i = 0; i < count; i++)
    {
      memcpy((void *)scratch, buff, BLOCKSIZE);
      buff += BLOCKSIZE;

      ret = BSP_SD_WriteBlocks_DMA(BSP_SD_INSTANCE, (uint32_t*)scratch, (uint32_t)sector++, 1);
      if (ret == BSP_ERROR_NONE)
      {
        /* wait until the read is successful or a timeout occurs */
#if (osCMSIS < 0x20000U)
        /* wait for a message from the queue or a timeout */
        event = osMessageGet(SDQueueID, SD_TIMEOUT);

        if (event.status == osEventMessage)
        {
          if (event.value.v == WRITE_CPLT_MSG)
          {
            timer = osKernelSysTick();
            /* block until SDIO IP is ready or a timeout occur */
            while(osKernelSysTick() - timer <SD_TIMEOUT)
#else
          status = osMessageQueueGet(SDQueueID, (void *)&event, NULL, SD_TIMEOUT);
          if ((status == osOK) && (event == WRITE_CPLT_MSG))
          {
            timer = osKernelGetTickCount();
            /* block until SDIO IP is ready or a timeout occur */
            ret = BSP_ERROR_BUSY;
            while(osKernelGetTickCount() - timer < SD_TIMEOUT)
#endif
            {
              ret = BSP_SD_GetCardState(BSP_SD_INSTANCE);

              if (ret == BSP_ERROR_NONE)
              {
                break;
              }
            }

            if (ret != BSP_ERROR_NONE)
            {
              break;
            }
#if (osCMSIS < 0x20000U)
      }
    }
#else
    }
#endif
        }
        else
        {
          break;
        }
      }

      if ((i == count) && (ret == BSP_ERROR_NONE ))
        res = RES_OK;
    }
#endif

  return res;
}
#endif /* _USE_WRITE == 1 */

/**
  * @brief  I/O control operation
  * @param  lun : not used
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
DRESULT SD_ioctl(BYTE lun, BYTE cmd, void *buff)
{
  DRESULT res = RES_ERROR;
  BSP_SD_CardInfo CardInfo;

  if (Stat & STA_NOINIT) return RES_NOTRDY;

  switch (cmd)
  {
  /* Make sure that no pending write process */
  case CTRL_SYNC :
    res = RES_OK;
    break;

  /* Get number of sectors on the disk (DWORD) */
  case GET_SECTOR_COUNT :
    BSP_SD_GetCardInfo(BSP_SD_INSTANCE, &CardInfo);
    *(DWORD*)buff = CardInfo.LogBlockNbr;
    res = RES_OK;
    break;

  /* Get R/W sector size (WORD) */
  case GET_SECTOR_SIZE :
    BSP_SD_GetCardInfo(BSP_SD_INSTANCE, &CardInfo);
    *(WORD*)buff = CardInfo.LogBlockSize;
    res = RES_OK;
    break;

  /* Get erase block size in unit of sector (DWORD) */
  case GET_BLOCK_SIZE :
    BSP_SD_GetCardInfo(BSP_SD_INSTANCE, &CardInfo);
    *(DWORD*)buff = CardInfo.LogBlockSize / SD_DEFAULT_BLOCK_SIZE;
	res = RES_OK;
    break;

  default:
    res = RES_PARERR;
  }

  return res;
}
#endif /* _USE_IOCTL == 1 */



/**
  * @brief Tx Transfer completed callbacks
  * @param hsd: SD handle
  * @retval None
  */
void BSP_SD_WriteCpltCallback(uint32_t Instance)
{
  /*
   * No need to add an "osKernelRunning()" check here, as the SD_initialize()
   * is always called before any SD_Read()/SD_Write() call
   */
  if (Instance == BSP_SD_INSTANCE)
  {
#if (osCMSIS < 0x20000U)
   osMessagePut(SDQueueID, WRITE_CPLT_MSG, 0);
#else
   const uint16_t msg = WRITE_CPLT_MSG;
   osMessageQueuePut(SDQueueID, (const void *)&msg, NULL, 0);
#endif
  }
}

/**
  * @brief Rx Transfer completed callbacks
  * @param hsd: SD handle
  * @retval None
  */
void BSP_SD_ReadCpltCallback(uint32_t Instance)
{
  /*
   * No need to add an "osKernelRunning()" check here, as the SD_initialize()
   * is always called before any SD_Read()/SD_Write() call
   */
  if (Instance == BSP_SD_INSTANCE)
  {
#if (osCMSIS < 0x20000U)
    osMessagePut(SDQueueID, READ_CPLT_MSG, 0);
#else
    const uint16_t msg = READ_CPLT_MSG;
    osMessageQueuePut(SDQueueID, (const void *)&msg, NULL, 0);
#endif
  }
}

void BSP_SD_AbortCallback(uint32_t Instance)
{
  if (Instance == BSP_SD_INSTANCE)
  {
#if (osCMSIS < 0x20000U)
    osMessagePut(SDQueueID, RW_ABORT_MSG, 0);
#else
    const uint16_t msg = RW_ABORT_MSG;
    osMessageQueuePut(SDQueueID, (const void *)&msg, NULL, 0);
#endif
  }
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

