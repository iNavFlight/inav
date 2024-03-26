/*
 * At32 MSC scsi interface
 * FLASH emfat reads and writes are implemented 
 * Other storage read and write services, such as sd, are not supported
 *
 *
 *
 */


#include <stdbool.h>

#include "at32_msc_diskio.h"
#include "platform.h"

#include "common/utils.h"
#include "msc_bot_scsi.h"
#include "drivers/usb_msc.h"



#define STORAGE_LUN_NBR 1
#define SCSI_BLOCK_SIZE 512

static const uint8_t scsi_inquiry[MSC_SUPPORT_MAX_LUN][SCSI_INQUIRY_DATA_LENGTH] =
{
  /* lun = 0 */
  {
    0x00,         /* peripheral device type (direct-access device) */
    0x80,         /* removable media bit */
    0x00,         /* ansi version, ecma version, iso version */
    0x01,         /* respond data format */
    SCSI_INQUIRY_DATA_LENGTH - 5, /* additional length */
    0x00, 0x00, 0x00, /* reserved */
	'I', 'N', 'A', 'V', ' ', 'F', 'C', ' ', // Manufacturer : 8 bytes
	'O', 'n', 'b', 'o', 'a', 'r', 'd', ' ', // Product      : 16 Bytes
	'F', 'l', 'a', 's', 'h', ' ', ' ', ' ', //
	' ', ' ', ' ' ,' ',                     // Version      : 4 Bytes
  }
};

usb_sts_type msc_disk_capacity(uint8_t lun, uint32_t *block_num, uint32_t *block_size)
{
    UNUSED(lun);
    *block_size = SCSI_BLOCK_SIZE;
    *block_num = emfat.disk_sectors;
    return USB_OK;
}

/*

 if( ((USBD_StorageTypeDef *)pdev->pUserData)->Read(lun ,
                              hmsc->bot_data,
                              hmsc->scsi_blk_addr / hmsc->scsi_blk_size,
                              len / hmsc->scsi_blk_size) < 0)

static int8_t STORAGE_Read(
    uint8_t lun,        // logical unit number
    uint8_t *buf,       // Pointer to the buffer to save data
    uint32_t blk_addr,  // address of 1st block to be read
    uint16_t blk_len)   // nmber of blocks to be read
{
    UNUSED(lun);
    mscSetActive();
    emfat_read(&emfat, buf, blk_addr, blk_len);
    return 0;
}

*/
usb_sts_type msc_disk_read(
    uint8_t lun,        // logical unit number
    uint32_t blk_addr,  // address of 1st block to be read
    uint8_t *buf,       // Pointer to the buffer to save data
    uint32_t blk_len)   // nmber of blocks to be read
{
    UNUSED(lun);
    //mscSetActive();
    emfat_read(&emfat, buf, blk_addr/SCSI_BLOCK_SIZE , blk_len/SCSI_BLOCK_SIZE);
    return USB_OK;
}

usb_sts_type msc_disk_write(uint8_t lun,
    uint32_t blk_addr,
    uint8_t *buf,
    uint32_t blk_len)
{
    UNUSED(lun);
    UNUSED(buf);
    UNUSED(blk_addr);
    UNUSED(blk_len);

    return USB_FAIL;
}


/**
  * @brief  get disk inquiry
  * @param  lun: logical units number
  * @retval inquiry string
  */
uint8_t * get_inquiry(uint8_t lun)
{
  if(lun < MSC_SUPPORT_MAX_LUN)
    return (uint8_t *)scsi_inquiry[lun];
  else
    return NULL;
}
