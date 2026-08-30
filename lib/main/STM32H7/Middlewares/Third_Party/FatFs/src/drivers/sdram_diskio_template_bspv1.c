/**
  ******************************************************************************
  * @file    sdram_diskio_template_bspv1.c
  * @author  MCD Application Team
  * @brief   SDRAM Disk I/O template driver using BSP v1 API.
  * This file needs to be copied under the application project
  * alongside the respective header file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics. All rights reserved.
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
#include "sdram_diskio.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Block Size in Bytes */
#define BLOCK_SIZE                512

/* Private variables ---------------------------------------------------------*/
/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;

/* Private function prototypes -----------------------------------------------*/
DSTATUS SDRAMDISK_initialize (BYTE);
DSTATUS SDRAMDISK_status (BYTE);
DRESULT SDRAMDISK_read (BYTE, BYTE*, DWORD, UINT);
#if _USE_WRITE == 1
  DRESULT SDRAMDISK_write (BYTE, const BYTE*, DWORD, UINT);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
  DRESULT SDRAMDISK_ioctl (BYTE, BYTE, void*);
#endif /* _USE_IOCTL == 1 */

const Diskio_drvTypeDef  SDRAMDISK_Driver =
{
  SDRAMDISK_initialize,
  SDRAMDISK_status,
  SDRAMDISK_read,
#if  _USE_WRITE
  SDRAMDISK_write,
#endif  /* _USE_WRITE == 1 */
#if  _USE_IOCTL == 1
  SDRAMDISK_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes a Drive
  * @param  lun : not used
  * @retval DSTATUS: Operation status
  */
DSTATUS SDRAMDISK_initialize(BYTE lun)
{
  Stat = STA_NOINIT;

  /* Configure the SDRAM device */
  if(BSP_SDRAM_Init() == SDRAM_OK)
  {
    Stat &= ~STA_NOINIT;
  }

  return Stat;
}

/**
  * @brief  Gets Disk Status
  * @param  lun : not used
  * @retval DSTATUS: Operation status
  */
DSTATUS SDRAMDISK_status(BYTE lun)
{
  return Stat;
}

/**
  * @brief  Reads Sector(s)
  * @param  lun : not used
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT SDRAMDISK_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
  uint32_t *pSrcBuffer = (uint32_t *)buff;
  uint32_t BufferSize = (BLOCK_SIZE * count)/4;
  uint32_t *pSdramAddress = (uint32_t *) (SDRAM_DEVICE_ADDR + (sector * BLOCK_SIZE));

  for(; BufferSize != 0; BufferSize--)
  {
    *pSrcBuffer++ = *(__IO uint32_t *)pSdramAddress++;
  }

  return RES_OK;
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
DRESULT SDRAMDISK_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
  uint32_t *pDstBuffer = (uint32_t *)buff;
  uint32_t BufferSize = (BLOCK_SIZE * count)/4;
  uint32_t *pSramAddress = (uint32_t *) (SDRAM_DEVICE_ADDR + (sector * BLOCK_SIZE));

  for(; BufferSize != 0; BufferSize--)
  {
    *(__IO uint32_t *)pSramAddress++ = *pDstBuffer++;
  }

  return RES_OK;
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
DRESULT SDRAMDISK_ioctl(BYTE lun, BYTE cmd, void *buff)
{
  DRESULT res = RES_ERROR;

  if (Stat & STA_NOINIT) return RES_NOTRDY;

  switch (cmd)
  {
  /* Make sure that no pending write process */
  case CTRL_SYNC :
    res = RES_OK;
    break;

  /* Get number of sectors on the disk (DWORD) */
  case GET_SECTOR_COUNT :
    *(DWORD*)buff = SDRAM_DEVICE_SIZE / BLOCK_SIZE;
    res = RES_OK;
    break;

  /* Get R/W sector size (WORD) */
  case GET_SECTOR_SIZE :
    *(WORD*)buff = BLOCK_SIZE;
    res = RES_OK;
    break;

  /* Get erase block size in unit of sector (DWORD) */
  case GET_BLOCK_SIZE :
    *(DWORD*)buff = 1;
	res = RES_OK;
    break;

  default:
    res = RES_PARERR;
  }

  return res;
}
#endif /* _USE_IOCTL == 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

