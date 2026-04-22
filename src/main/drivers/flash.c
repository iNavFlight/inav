/*
 * This file is part of INAV.
 *
 * INAV is free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * INAV is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
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

#include "build/debug.h"

#ifdef USE_FLASHFS

#include "flash.h"
#include "flash_m25p16.h"
#include "flash_w25n.h"

#include "common/time.h"

#include "drivers/bus_spi.h"
#include "drivers/io.h"
#include "drivers/time.h"

static flashDriver_t flashDrivers[] = {

#ifdef USE_SPI

#ifdef USE_FLASH_M25P16
    {
        .init = m25p16_init,
        .isReady = m25p16_isReady,
        .waitForReady = m25p16_waitForReady,
        .eraseSector = m25p16_eraseSector,
        .eraseCompletely = m25p16_eraseCompletely,
        .pageProgram = m25p16_pageProgram,
        .readBytes = m25p16_readBytes,
        .getGeometry = m25p16_getGeometry,
        .flush = NULL
    },
#endif

#if defined(USE_FLASH_W25N01G) || defined(USE_FLASH_W25N02K)
    {
        .init = w25n_init,
        .isReady = w25n_isReady,
        .waitForReady = w25n_waitForReady,
        .eraseSector = w25n_eraseSector,
        .eraseCompletely = w25n_eraseCompletely,
        .pageProgram = w25n_pageProgram,
        .readBytes = w25n_readBytes,
        .getGeometry = w25n_getGeometry,
        .flush = w25n_flush
    },
#endif

#endif

};

static flashDriver_t *flash;

static bool flashDeviceInit(void)
{
    bool detected = false;

    for (uint32_t idx = 0; idx < ARRAYLEN(flashDrivers); idx++) //idx = ARRAYLEN may cause overflow
    {
        detected = flashDrivers[idx].init(0);
        if (detected)
        {
            flash = &flashDrivers[idx];
            break;
        }
    }
    return detected;
}

bool flashIsReady(void)
{
    // prevent the machine cycle from crashing if there is no external flash memory
    if (flash == NULL) {
        return false;
    }

    return flash->isReady();
}

bool flashWaitForReady(timeMs_t timeoutMillis)
{
    return flash->waitForReady(timeoutMillis);
}

void flashEraseSector(uint32_t address)
{
    flash->eraseSector(address);
}

void flashEraseCompletely(void)
{
    flash->eraseCompletely();
}

uint32_t flashPageProgram(uint32_t address, const uint8_t *data, int length)
{
    return flash->pageProgram(address, data, length);
}

int flashReadBytes(uint32_t address, uint8_t *buffer, int length)
{
    return flash->readBytes(address, buffer, length);
}

void flashFlush(void)
{
    flash->flush();
}

const flashGeometry_t *flashGetGeometry(void)
{
    static flashGeometry_t fgNone = {0};

    // prevent the machine cycle from crashing if there is no external flash memory
    if (flash == NULL) {
        return &fgNone;
    }

    return flash->getGeometry();
}

/*
 * Flash partitioning
 *
 * Partition table is not currently stored on the flash, in-memory only.
 *
 * Partitions are required so that Badblock management (inc spare blocks), FlashFS (Blackbox Logging), Configuration and Firmware can be kept separate and tracked.
 *
 * XXX FIXME
 * XXX Note that Flash FS must start at sector 0.
 * XXX There is existing blackbox/flash FS code the relies on this!!!
 * XXX This restriction can and will be fixed by creating a set of flash operation functions that take partition as an additional parameter.
 */

static flashPartitionTable_t flashPartitionTable;
static int flashPartitions = 0;

static __attribute__((unused)) void createPartition(flashPartitionType_e type, uint32_t size, flashSector_t *endSector)
{
    const flashGeometry_t *flashGeometry = flashGetGeometry();
    flashSector_t partitionSectors = (size / flashGeometry->sectorSize);

    if (size % flashGeometry->sectorSize > 0) {
        partitionSectors++; // needs a portion of a sector.
    }

    flashSector_t startSector = (*endSector + 1) - partitionSectors; // + 1 for inclusive

    flashPartitionSet(type, startSector, *endSector);

    *endSector = startSector - 1;
}

static void flashConfigurePartitions(void)
{
    const flashGeometry_t *flashGeometry = flashGetGeometry();
    if (flashGeometry->totalSize == 0) {
        return;
    }

    flashSector_t startSector = 0;
    flashSector_t endSector = flashGeometry->sectors - 1; // 0 based index

    const flashPartition_t *badBlockPartition = flashPartitionFindByType(FLASH_PARTITION_TYPE_BADBLOCK_MANAGEMENT);
    if (badBlockPartition) {
        endSector = badBlockPartition->startSector - 1;
    }

#if defined(FIRMWARE_SIZE)
    createPartition(FLASH_PARTITION_TYPE_FIRMWARE, FIRMWARE_SIZE * 1024, &endSector);
#endif

#if defined(MSP_FIRMWARE_UPDATE)
    createPartition(FLASH_PARTITION_TYPE_FIRMWARE_UPDATE_META, flashGeometry->sectorSize, &endSector);
    createPartition(FLASH_PARTITION_TYPE_UPDATE_FIRMWARE, MCU_FLASH_SIZE * 1024, &endSector);
    createPartition(FLASH_PARTITION_TYPE_FULL_BACKUP, MCU_FLASH_SIZE * 1024, &endSector);
#endif

#if defined(CONFIG_IN_EXTERNAL_FLASH)
    uint32_t configSize = EEPROM_SIZE;
    flashSector_t configSectors = (configSize / flashGeometry->sectorSize);

    if (configSize % flashGeometry->sectorSize > 0) {
        configSectors++; // needs a portion of a sector.
    }
    configSize = configSectors * flashGeometry->sectorSize;

    createPartition(FLASH_PARTITION_TYPE_CONFIG, configSize, &endSector);
#endif

#ifdef USE_FLASHFS
    flashPartitionSet(FLASH_PARTITION_TYPE_FLASHFS, startSector, endSector);
#endif
}

flashPartition_t *flashPartitionFindByType(flashPartitionType_e type)
{
    for (int index = 0; index < FLASH_MAX_PARTITIONS; index++) {
        flashPartition_t *candidate = &flashPartitionTable.partitions[index];
        if (candidate->type == type) {
            return candidate;
        }
    }

    return NULL;
}

const flashPartition_t *flashPartitionFindByIndex(uint8_t index)
{
    if (index >= flashPartitions) {
        return NULL;
    }

    return &flashPartitionTable.partitions[index];
}

void flashPartitionSet(uint8_t type, uint32_t startSector, uint32_t endSector)
{
    flashPartition_t *entry = flashPartitionFindByType(type);

    if (!entry) {
        if (flashPartitions == FLASH_MAX_PARTITIONS - 1) {
            return;
        }
        entry = &flashPartitionTable.partitions[flashPartitions++];
    }

    entry->type = type;
    entry->startSector = startSector;
    entry->endSector = endSector;
}

// Must be in sync with FLASH_PARTITION_TYPE
static const char *flashPartitionNames[] = {
    "UNKNOWN  ",
    "PARTITION",
    "FLASHFS  ",
    "BBMGMT   ",
    "FIRMWARE ",
    "CONFIG   ",
    "FW UPDT  ",
};

const char *flashPartitionGetTypeName(flashPartitionType_e type)
{
    if (type < ARRAYLEN(flashPartitionNames)) {
        return flashPartitionNames[type];
    }

    return NULL;
}

bool flashInit(void)
{
    memset(&flashPartitionTable, 0, sizeof(flashPartitionTable));

    bool haveFlash = flashDeviceInit();

    if (haveFlash) {
        flashConfigurePartitions();
    }

    return haveFlash;
}

int flashPartitionCount(void)
{
    return flashPartitions;
}

uint32_t flashPartitionSize(flashPartition_t *partition)
{
    const flashGeometry_t * const geometry = flashGetGeometry();
    return (partition->endSector - partition->startSector + 1) * geometry->sectorSize;
}

void flashPartitionErase(flashPartition_t *partition)
{
    const flashGeometry_t * const geometry = flashGetGeometry();

    // if there's a single FLASHFS partition and it uses the entire flash then do a full erase
    const bool doFullErase = (flashPartitionCount() == 1) && (FLASH_PARTITION_SECTOR_COUNT(partition) == geometry->sectors);
    if (doFullErase) {
        flashEraseCompletely();
        return;
    }

    for (unsigned i = partition->startSector; i <= partition->endSector; i++) {
        uint32_t flashAddress = geometry->sectorSize * i;
        flashEraseSector(flashAddress);
        flashWaitForReady(0);
    }
}
#endif // USE_FLASH_CHIP
