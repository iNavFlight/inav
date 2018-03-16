/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "hal.h"
#include "ffconf.h"
#include "diskio.h"

#include "usbh.h"
#include "usbh/dev/msd.h"

/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */
#define MSDLUN0	0

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */

DSTATUS disk_initialize (
    BYTE pdrv                /* Physical drive nmuber (0..) */
)
{
  DSTATUS stat;

  switch (pdrv) {
  case MSDLUN0:
    stat = 0;
    /* It is initialized externally, just reads the status.*/
    if (blkGetDriverState(&MSBLKD[0]) != BLK_READY)
      stat |= STA_NOINIT;
    return stat;
  }
  return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
    BYTE pdrv        /* Physical drive nmuber (0..) */
)
{
  DSTATUS stat;

  switch (pdrv) {
  case MSDLUN0:
    stat = 0;
    /* It is initialized externally, just reads the status.*/
    if (blkGetDriverState(&MSBLKD[0]) != BLK_READY)
      stat |= STA_NOINIT;
    return stat;
  }
  return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read (
    BYTE pdrv,        /* Physical drive nmuber (0..) */
    BYTE *buff,        /* Data buffer to store read data */
    DWORD sector,    /* Sector address (LBA) */
    UINT count        /* Number of sectors to read (1..255) */
)
{
  switch (pdrv) {
  case MSDLUN0:
	/* It is initialized externally, just reads the status.*/
	if (blkGetDriverState(&MSBLKD[0]) != BLK_READY)
		return RES_NOTRDY;
	if (usbhmsdLUNRead(&MSBLKD[0], sector, buff, count))
		return RES_ERROR;
	return RES_OK;
  }
  return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _USE_WRITE
DRESULT disk_write (
    BYTE pdrv,            /* Physical drive nmuber (0..) */
    const BYTE *buff,    /* Data to be written */
    DWORD sector,        /* Sector address (LBA) */
    UINT count            /* Number of sectors to write (1..255) */
)
{
  switch (pdrv) {
  case MSDLUN0:
	/* It is initialized externally, just reads the status.*/
	if (blkGetDriverState(&MSBLKD[0]) != BLK_READY)
		return RES_NOTRDY;
	if (usbhmsdLUNWrite(&MSBLKD[0], sector, buff, count))
		return RES_ERROR;
	return RES_OK;
  }
  return RES_PARERR;
}
#endif /* _USE_WRITE */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

#if _USE_IOCTL
DRESULT disk_ioctl (
    BYTE pdrv,        /* Physical drive nmuber (0..) */
    BYTE cmd,        /* Control code */
    void *buff        /* Buffer to send/receive control data */
)
{
  switch (pdrv) {
  case MSDLUN0:
    switch (cmd) {
    case CTRL_SYNC:
        return RES_OK;
    case GET_SECTOR_COUNT:
        *((DWORD *)buff) = MSBLKD[0].info.blk_num;
        return RES_OK;
    case GET_SECTOR_SIZE:
        *((WORD *)buff) = MSBLKD[0].info.blk_size;
        return RES_OK;
    default:
        return RES_PARERR;
    }
  }
  return RES_PARERR;
}
#endif /* _USE_IOCTL */

DWORD get_fattime(void) {
    return ((uint32_t)0 | (1 << 16)) | (1 << 21); /* wrong but valid time */
}
