/*             ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                   Copyright (c) 2014-2017 Datalight, Inc.
                       All Rights Reserved Worldwide.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; use version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but "AS-IS," WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
/*  Businesses and individuals that for commercial or other reasons cannot
    comply with the terms of the GPLv2 license may obtain a commercial license
    before incorporating Reliance Edge into proprietary software for
    distribution in any form.  Visit http://www.datalight.com/reliance-edge for
    more information.
*/
/** @file
    @brief Implements block device I/O.
*/
#include "hal.h"

#if (HAL_USE_SDMMC == TRUE)
#include "sama_sdmmc_lld.h"
#if SDMMC_USE_RELEDGE_LIB == 1

#include "sama_sdmmc_lld.h"
#include "ch_sdmmc_device.h"
#include "ch_sdmmc_cmds.h"
#include "ch_sdmmc_sdio.h"
#include "ch_sdmmc_sd.h"
#include "ch_sdmmc_mmc.h"
#include "ch_sdmmc_reledge.h"

#include <redfs.h>
#include <redvolume.h>
#include <redosdeviations.h>

#if REDCONF_API_POSIX == 0
#error "REDCONF_API_POSIX should be 1"
#endif

#if REDCONF_API_FSE == 1
#error "REDCONF_API_FSE not supported, should be 0"
#endif


/*  sd_mmc_mem_2_ram_multi() and sd_mmc_ram_2_mem_multi() use an unsigned
    16-bit value to specify the sector count, so no transfer can be larger
    than UINT16_MAX sectors.
*/
#define MAX_SECTOR_TRANSFER UINT16_MAX


/** @brief Initialize a disk.

    @param bVolNum  The volume number of the volume whose block device is being
                    initialized.
    @param mode     The open mode, indicating the type of access required.

    @return A negated ::REDSTATUS code indicating the operation result.

    @retval 0           Operation was successful.
    @retval -RED_EIO    A disk I/O error occurred.
    @retval -RED_EROFS  The device is read-only media and write access was
                        requested.
*/
static REDSTATUS DiskOpen(
    uint8_t         bVolNum,
    BDEVOPENMODE    mode)
{
    REDSTATUS       ret = 0;
    uint32_t        ulTries;


    eSDMMC_RC     cs;

    SdmmcDriver *sdmmcp = NULL;


	if (!sdmmcGetInstance(bVolNum, &sdmmcp))
		return RED_EINVAL;

    /*  Note: Assuming the volume number is the same as the SD card slot.  The
        ASF SD/MMC driver supports two SD slots.  This implementation will need
        to be modified if multiple volumes share a single SD card.
    */

    /*  The first time the disk is opened, the SD card can take a while to get
        ready, in which time sd_mmc_test_unit_ready() returns either CTRL_BUSY
        or CTRL_NO_PRESENT.  Try numerous times, waiting half a second after
        each failure.  Empirically, this has been observed to succeed on the
        second try, so trying 10x more than that provides a margin of error.
    */
    for(ulTries = 0U; ulTries < 20U; ulTries++)
    {
        cs = sd_mmc_test_unit_ready(sdmmcp);

        if((cs != SDMMC_OK) && (cs != SDMMC_BUSY))
        {
            break;
        }

       // t_msleep(sdmmcp,500);
    }

    if(cs == SDMMC_OK)
    {
      #if REDCONF_READ_ONLY == 0
        if(mode != BDEV_O_RDONLY)
        {
            if(sd_mmc_is_write_protected(sdmmcp))
            {
                ret = -RED_EROFS;
            }
        }

        if(ret == 0)
      #endif
        {
            uint32_t ulSectorLast;

            IGNORE_ERRORS(sd_mmc_read_capacity(sdmmcp, &ulSectorLast));

            /*  The ASF SD/MMC driver only supports 512-byte sectors.
            */
            if(    (gaRedVolConf[bVolNum].ulSectorSize != 512U)
                || (((uint64_t)ulSectorLast + 1U) < gaRedVolConf[bVolNum].ullSectorCount))
            {
                ret = -RED_EINVAL;
            }
        }
    }
    else
    {
        ret = -RED_EIO;
    }

    return ret;
}


/** @brief Uninitialize a disk.

    @param bVolNum  The volume number of the volume whose block device is being
                    uninitialized.

    @return A negated ::REDSTATUS code indicating the operation result.

    @retval 0   Operation was successful.
*/
static REDSTATUS DiskClose(
    uint8_t     bVolNum)
{
    (void)bVolNum;
    return 0;
}


/** @brief Read sectors from a disk.

    @param bVolNum          The volume number of the volume whose block device
                            is being read from.
    @param ullSectorStart   The starting sector number.
    @param ulSectorCount    The number of sectors to read.
    @param pBuffer          The buffer into which to read the sector data.

    @return A negated ::REDSTATUS code indicating the operation result.

    @retval 0   Operation was successful.
*/
static REDSTATUS DiskRead(
    uint8_t     bVolNum,
    uint64_t    ullSectorStart,
    uint32_t    ulSectorCount,
    void       *pBuffer)
{
    REDSTATUS   ret = 0;
    uint8_t    *pbBuffer = CAST_VOID_PTR_TO_UINT8_PTR(pBuffer);

    SdmmcDriver *sdmmcp = NULL;
    eSDMMC_RC cs;

	if (!sdmmcGetInstance(bVolNum, &sdmmcp))
		return RED_EINVAL;

	cs = SD_ReadBlocks(sdmmcp, ullSectorStart, pbBuffer,ulSectorCount);

	if(cs != SDMMC_OK)
	{
		ret = -RED_EIO;
	}


    return ret;
}


#if REDCONF_READ_ONLY == 0

/** @brief Write sectors to a disk.

    @param bVolNum          The volume number of the volume whose block device
                            is being written to.
    @param ullSectorStart   The starting sector number.
    @param ulSectorCount    The number of sectors to write.
    @param pBuffer          The buffer from which to write the sector data.

    @return A negated ::REDSTATUS code indicating the operation result.

    @retval 0   Operation was successful.
*/
static REDSTATUS DiskWrite(
    uint8_t         bVolNum,
    uint64_t        ullSectorStart,
    uint32_t        ulSectorCount,
    const void     *pBuffer)
{
    REDSTATUS       ret = 0;
    const uint8_t  *pbBuffer = CAST_VOID_PTR_TO_CONST_UINT8_PTR(pBuffer);

    SdmmcDriver *sdmmcp = NULL;
    eSDMMC_RC cs;

	if (!sdmmcGetInstance(bVolNum, &sdmmcp))
		return RED_EINVAL;


    cs = SD_WriteBlocks(sdmmcp, ullSectorStart, pbBuffer, ulSectorCount);
    if(cs != SDMMC_OK)
	{
		ret = -RED_EIO;
	}


	return ret;
}


/** @brief Flush any caches beneath the file system.

    @param bVolNum  The volume number of the volume whose block device is being
                    flushed.

    @return A negated ::REDSTATUS code indicating the operation result.

    @retval 0   Operation was successful.
*/
static REDSTATUS DiskFlush(
    uint8_t     bVolNum)
{
    REDSTATUS   ret;

    eSDMMC_RC cs;

    SdmmcDriver *sdmmcp = NULL;


	if (!sdmmcGetInstance(bVolNum, &sdmmcp))
		return RED_EINVAL;

    /*  The ASF SD/MMC driver appears to write sectors synchronously, so it
        should be fine to do nothing and return success.  However, Atmel's
        implementation of the FatFs diskio.c file does the equivalent of the
        below when the disk is flushed.  Just in case this is important for some
        non-obvious reason, do the same.
    */
    cs = sd_mmc_test_unit_ready(sdmmcp);
    if(cs == SDMMC_OK)
    {
        ret = 0;
    }
    else
    {
        ret = -RED_EIO;
    }

    return ret;
}


#if REDCONF_DISCARDS == 1
/** @brief Discard sectors on a disk.

    @param bVolNum          The volume number of the volume whose block device
                            is being accessed.
    @param ullSectorStart   The starting sector number.
    @param ullSectorCount   The number of sectors to discard.
*/
static void DiskDiscard(
    uint8_t     bVolNum,
    uint64_t    ullSectorStart,
    uint64_t    ullSectorCount)
{
#error "this SD/MMC driver does not support discards."
}
#endif /* REDCONF_DISCARDS == 1 */

#endif /* REDCONF_READ_ONLY == 0 */

/** @brief Initialize a block device.

    This function is called when the file system needs access to a block
    device.

    Upon successful return, the block device should be fully initialized and
    ready to service read/write/flush/close requests.

    The behavior of calling this function on a block device which is already
    open is undefined.

    @param bVolNum  The volume number of the volume whose block device is being
                    initialized.
    @param mode     The open mode, indicating the type of access required.

    @return A negated ::REDSTATUS code indicating the operation result.

    @retval 0           Operation was successful.
    @retval -RED_EINVAL @p bVolNum is an invalid volume number.
    @retval -RED_EIO    A disk I/O error occurred.
*/
REDSTATUS RedOsBDevOpen(
    uint8_t         bVolNum,
    BDEVOPENMODE    mode)
{
    REDSTATUS       ret;

    if(bVolNum >= REDCONF_VOLUME_COUNT)
    {
        ret = -RED_EINVAL;
    }
    else
    {
        ret = DiskOpen(bVolNum, mode);
    }

    return ret;
}


/** @brief Uninitialize a block device.

    This function is called when the file system no longer needs access to a
    block device.  If any resource were allocated by RedOsBDevOpen() to service
    block device requests, they should be freed at this time.

    Upon successful return, the block device must be in such a state that it
    can be opened again.

    The behavior of calling this function on a block device which is already
    closed is undefined.

    @param bVolNum  The volume number of the volume whose block device is being
                    uninitialized.

    @return A negated ::REDSTATUS code indicating the operation result.

    @retval 0           Operation was successful.
    @retval -RED_EINVAL @p bVolNum is an invalid volume number.
*/
REDSTATUS RedOsBDevClose(
    uint8_t     bVolNum)
{
    REDSTATUS   ret;

    if(bVolNum >= REDCONF_VOLUME_COUNT)
    {
        ret = -RED_EINVAL;
    }
    else
    {
        ret = DiskClose(bVolNum);
    }

    return ret;
}


/** @brief Read sectors from a physical block device.

    The behavior of calling this function is undefined if the block device is
    closed or if it was opened with ::BDEV_O_WRONLY.

    @param bVolNum          The volume number of the volume whose block device
                            is being read from.
    @param ullSectorStart   The starting sector number.
    @param ulSectorCount    The number of sectors to read.
    @param pBuffer          The buffer into which to read the sector data.

    @return A negated ::REDSTATUS code indicating the operation result.

    @retval 0           Operation was successful.
    @retval -RED_EINVAL @p bVolNum is an invalid volume number, @p pBuffer is
                        `NULL`, or @p ullStartSector and/or @p ulSectorCount
                        refer to an invalid range of sectors.
    @retval -RED_EIO    A disk I/O error occurred.
*/
REDSTATUS RedOsBDevRead(
    uint8_t     bVolNum,
    uint64_t    ullSectorStart,
    uint32_t    ulSectorCount,
    void       *pBuffer)
{
    REDSTATUS   ret = 0;

    if(    (bVolNum >= REDCONF_VOLUME_COUNT)
        || (ullSectorStart >= gaRedVolConf[bVolNum].ullSectorCount)
        || ((gaRedVolConf[bVolNum].ullSectorCount - ullSectorStart) < ulSectorCount)
        || (pBuffer == NULL))
    {
        ret = -RED_EINVAL;
    }
    else
    {
        ret = DiskRead(bVolNum, ullSectorStart, ulSectorCount, pBuffer);
    }

    return ret;
}


#if REDCONF_READ_ONLY == 0

/** @brief Write sectors to a physical block device.

    The behavior of calling this function is undefined if the block device is
    closed or if it was opened with ::BDEV_O_RDONLY.

    @param bVolNum          The volume number of the volume whose block device
                            is being written to.
    @param ullSectorStart   The starting sector number.
    @param ulSectorCount    The number of sectors to write.
    @param pBuffer          The buffer from which to write the sector data.

    @return A negated ::REDSTATUS code indicating the operation result.

    @retval 0           Operation was successful.
    @retval -RED_EINVAL @p bVolNum is an invalid volume number, @p pBuffer is
                        `NULL`, or @p ullStartSector and/or @p ulSectorCount
                        refer to an invalid range of sectors.
    @retval -RED_EIO    A disk I/O error occurred.
*/
REDSTATUS RedOsBDevWrite(
    uint8_t     bVolNum,
    uint64_t    ullSectorStart,
    uint32_t    ulSectorCount,
    const void *pBuffer)
{
    REDSTATUS   ret = 0;

    if(    (bVolNum >= REDCONF_VOLUME_COUNT)
        || (ullSectorStart >= gaRedVolConf[bVolNum].ullSectorCount)
        || ((gaRedVolConf[bVolNum].ullSectorCount - ullSectorStart) < ulSectorCount)
        || (pBuffer == NULL))
    {
        ret = -RED_EINVAL;
    }
    else
    {
        ret = DiskWrite(bVolNum, ullSectorStart, ulSectorCount, pBuffer);
    }

    return ret;
}


/** @brief Flush any caches beneath the file system.

    This function must synchronously flush all software and hardware caches
    beneath the file system, ensuring that all sectors written previously are
    committed to permanent storage.

    If the environment has no caching beneath the file system, the
    implementation of this function can do nothing and return success.

    The behavior of calling this function is undefined if the block device is
    closed or if it was opened with ::BDEV_O_RDONLY.

    @param bVolNum  The volume number of the volume whose block device is being
                    flushed.

    @return A negated ::REDSTATUS code indicating the operation result.

    @retval 0           Operation was successful.
    @retval -RED_EINVAL @p bVolNum is an invalid volume number.
    @retval -RED_EIO    A disk I/O error occurred.
*/
REDSTATUS RedOsBDevFlush(
    uint8_t     bVolNum)
{
    REDSTATUS   ret;

    if(bVolNum >= REDCONF_VOLUME_COUNT)
    {
        ret = -RED_EINVAL;
    }
    else
    {
        ret = DiskFlush(bVolNum);
    }

    return ret;
}

#endif /* REDCONF_READ_ONLY == 0 */
#endif

#endif
