/*
 * This file is part of iNav.
 *
 * iNav is free software. You can redistribute this software
 * and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * iNav is distributed in the hope that they will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#include "common/crc.h"

#include "drivers/flash.h"
#include "drivers/light_led.h"
#include "drivers/persistent.h"
#include "drivers/system.h"

#include "fc/firmware_update.h"
#include "fc/firmware_update_common.h"
#include "fc/runtime_config.h"

#include "io/asyncfatfs/asyncfatfs.h"


#ifdef MSP_FIRMWARE_UPDATE

#if !(defined(USE_FLASHFS) || defined(USE_SDCARD))
#error No storage backend available
#endif

static firmwareUpdateMetadata_t updateMetadata;
static uint8_t updateFirmwareCalcCRC = 0;
static uint32_t receivedSize = 0;
static bool rollbackPrepared = false;

#if defined(USE_SDCARD)
static uint32_t firmwareSize;
static afatfsFilePtr_t updateFile = NULL;
static afatfsFilePtr_t backupFile = NULL;

static void updateFileOpenCallback(afatfsFilePtr_t file)
{
    updateFile = file;
}

static void backupFileOpenCallback(afatfsFilePtr_t file)
{
    backupFile = file;
}

#elif defined(USE_FLASHFS)
static uint32_t flashStartAddress, flashOverflowAddress;

#endif

static bool fullBackup(void)
{
    const uint8_t *const backupSrcEnd = (const uint8_t*)FLASH_END;
    uint8_t *backupSrcPtr = (uint8_t*)&__firmware_start;
    uint32_t counter = 0;
    updateMetadata.backupCRC = 0;

    LED0_OFF;
    LED1_OFF;

#if defined(USE_SDCARD)
    if ((afatfs_getFilesystemState() != AFATFS_FILESYSTEM_STATE_READY) || !afatfs_fopen(FIRMWARE_UPDATE_BACKUP_FILENAME, "w+", backupFileOpenCallback)) return false;

    while (backupSrcPtr < backupSrcEnd) {

        const uint16_t writeBlockSize = 512;
        uint32_t justWritten = afatfs_fwriteSync(backupFile, backupSrcPtr, writeBlockSize);
        updateMetadata.backupCRC = crc8_dvb_s2_update(updateMetadata.backupCRC, backupSrcPtr, justWritten);

        afatfs_poll();
        backupSrcPtr += justWritten;

        if (++counter % (50*1024/512) == 0) {
            LED0_TOGGLE;
            LED1_TOGGLE;
        }

    }

    afatfs_fcloseSync(backupFile);

#elif defined(USE_FLASHFS)
    flashPartition_t *flashPartition = flashPartitionFindByType(FLASH_PARTITION_TYPE_FULL_BACKUP);
    if (!flashPartition) return false;

    const flashGeometry_t *flashGeometry = flashGetGeometry();
    const uint32_t flashSectorSize = flashGeometry->sectorSize;
    const uint32_t flashPartitionSize = (flashPartition->endSector - flashPartition->startSector + 1) * flashSectorSize;
    const uint32_t backupSize = AVAILABLE_FIRMWARE_SPACE;
    if (backupSize > flashPartitionSize) return false;

    uint32_t flashAddress = flashPartition->startSector * flashSectorSize;

    const uint32_t flashPageSize = flashGeometry->pageSize;
    while (backupSrcPtr < backupSrcEnd) {

        if (flashAddress % flashSectorSize == 0) {
            flashEraseSector(flashAddress);
            flashWaitForReady(1000);
        }

        flashPageProgram(flashAddress, backupSrcPtr, flashPageSize);
        updateMetadata.backupCRC = crc8_dvb_s2_update(updateMetadata.backupCRC, backupSrcPtr, flashPageSize);

        flashAddress += flashPageSize;
        backupSrcPtr += flashPageSize;

        if (++counter % (10*1024/256) == 0) {
            LED0_TOGGLE;
            LED1_TOGGLE;
        }

    }

#endif

    return true;
}

static bool backupIsValid(void)
{
    if (!firmwareUpdateMetadataRead(&updateMetadata) || (updateMetadata.magic != FIRMWARE_UPDATE_METADATA_MAGIC)) {
        return false;
    }

    LED0_OFF;
    LED1_OFF;

    uint32_t counter = 0;
    uint8_t calcCRC = 0;

#if defined(USE_SDCARD)
#define SD_BACKUP_FILE_BLOCK_READ_SIZE 512
    if ((afatfs_getFilesystemState() != AFATFS_FILESYSTEM_STATE_READY) || !afatfs_fopen(FIRMWARE_UPDATE_BACKUP_FILENAME, "w+", backupFileOpenCallback)) return false;

    uint32_t totalRead = 0;

    uint8_t buffer[SD_BACKUP_FILE_BLOCK_READ_SIZE];
    while (!afatfs_feof(backupFile)) {

        uint32_t readBytes = afatfs_freadSync(backupFile, buffer, SD_BACKUP_FILE_BLOCK_READ_SIZE);
        calcCRC = crc8_dvb_s2_update(calcCRC, buffer, readBytes);

        totalRead += readBytes;

        if (++counter % (50*1024/SD_BACKUP_FILE_BLOCK_READ_SIZE) == 0) {
            LED0_TOGGLE;
            LED1_TOGGLE;
        }

    }

    afatfs_fcloseSync(backupFile);

#elif defined(USE_FLASHFS)
    flashPartition_t *flashPartition = flashPartitionFindByType(FLASH_PARTITION_TYPE_FULL_BACKUP);
    if (!flashPartition) return false;

    const flashGeometry_t *flashGeometry = flashGetGeometry();
    const uint32_t flashSectorSize = flashGeometry->sectorSize;
    const uint32_t flashPartitionSize = (flashPartition->endSector - flashPartition->startSector + 1) * flashSectorSize;
    const uint32_t backupSize = FLASH_END - (uint32_t)&__firmware_start;
    if (backupSize > flashPartitionSize) return false;

    uint32_t flashAddress = flashPartition->startSector * flashSectorSize;
    const uint32_t flashEndAddress = flashAddress + backupSize;

    uint8_t buffer[256];
    while (flashAddress < flashEndAddress) {

        flashReadBytes(flashAddress, buffer, sizeof(buffer));
        calcCRC = crc8_dvb_s2_update(calcCRC, buffer, sizeof(buffer));

        flashAddress += sizeof(buffer);

        if (++counter % (10*1024/256) == 0) {
            LED0_TOGGLE;
            LED1_TOGGLE;
        }

    }

#endif

    return (calcCRC == updateMetadata.backupCRC);
}

bool firmwareUpdatePrepare(uint32_t updateSize)
{
    if (ARMING_FLAG(ARMED) || (updateSize > AVAILABLE_FIRMWARE_SPACE)) return false;

#if defined(USE_SDCARD)
    if ((afatfs_getFilesystemState() != AFATFS_FILESYSTEM_STATE_READY) || !afatfs_fopen(FIRMWARE_UPDATE_FIRMWARE_FILENAME, "w+", updateFileOpenCallback)) return false;

    firmwareSize = updateSize;

#elif defined(USE_FLASHFS)
    flashPartition_t *flashUpdatePartition = flashPartitionFindByType(FLASH_PARTITION_TYPE_UPDATE_FIRMWARE);
    if (!flashUpdatePartition) return false;

    const flashGeometry_t *flashGeometry = flashGetGeometry();

    flashStartAddress = flashUpdatePartition->startSector * flashGeometry->sectorSize;
    flashOverflowAddress = ((flashUpdatePartition->endSector + 1) * flashGeometry->sectorSize);
    receivedSize = 0;

    uint32_t partitionSize = (flashUpdatePartition->endSector - flashUpdatePartition->startSector + 1) * (flashGeometry->sectorSize * flashGeometry->pageSize);

    if (updateSize > partitionSize) {
        return false;
    }

    updateMetadata.firmwareSize = updateSize;

#endif

    updateFirmwareCalcCRC = 0;

    return true;
}

bool firmwareUpdateStore(uint8_t *data, uint16_t length)
{
    if (ARMING_FLAG(ARMED)) {
        return false;
    }

#if defined(USE_SDCARD)

    if (!updateFile || !firmwareSize || (receivedSize + length > firmwareSize)
            || (afatfs_fwriteSync(updateFile, data, length) != length)) {
        return false;
    }

#elif defined(USE_FLASHFS)
    if  (!updateMetadata.firmwareSize || (receivedSize + length > updateMetadata.firmwareSize)) return false;

    const uint32_t flashAddress = flashStartAddress + receivedSize;

    if ((flashAddress + length > flashOverflowAddress) || (receivedSize + length > updateMetadata.firmwareSize)) {
        updateMetadata.firmwareSize = 0;
        return false;
    }

    const flashGeometry_t *flashGeometry = flashGetGeometry();
    const uint32_t flashSectorSize = flashGeometry->sectorSize;

    if (flashAddress % flashSectorSize == 0) {
        flashEraseSector(flashAddress);
        flashWaitForReady(1000);
    }

    flashPageProgram(flashAddress, data, length);

#endif

    updateFirmwareCalcCRC = crc8_dvb_s2_update(updateFirmwareCalcCRC, data, length);
    receivedSize += length;

    return true;
}

void firmwareUpdateExec(uint8_t expectCRC)
{
    if (ARMING_FLAG(ARMED)) return;

#if defined(USE_SDCARD)
    if (!afatfs_fclose(updateFile, NULL)) return;
    if (firmwareSize && (receivedSize == firmwareSize) &&
            (updateFirmwareCalcCRC == expectCRC) && fullBackup() && firmwareUpdateMetadataWrite(&updateMetadata)) {
        systemResetRequest(RESET_BOOTLOADER_FIRMWARE_UPDATE);
    }
#elif defined(USE_FLASHFS)
    if (updateMetadata.firmwareSize && (receivedSize == updateMetadata.firmwareSize) &&
            (updateFirmwareCalcCRC == expectCRC) && fullBackup() && firmwareUpdateMetadataWrite(&updateMetadata)) {
        systemResetRequest(RESET_BOOTLOADER_FIRMWARE_UPDATE);
    }
#endif

}

bool firmwareUpdateRollbackPrepare(void)
{
    if (ARMING_FLAG(ARMED) || !(rollbackPrepared || backupIsValid())) return false;

    rollbackPrepared = true;
    return true;
}

void firmwareUpdateRollbackExec(void)
{
    if (ARMING_FLAG(ARMED) || !firmwareUpdateRollbackPrepare()) return;

    systemResetRequest(RESET_BOOTLOADER_FIRMWARE_ROLLBACK);
}

#endif
