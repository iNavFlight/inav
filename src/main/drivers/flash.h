/*
 * This file is part of iNav.
 *
 * iNav is free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * iNav is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdint.h>

#include "common/time.h"

//#ifdef USE_FLASHFS

typedef uint16_t flashSector_t;

typedef struct flashGeometry_s {
    flashSector_t sectors; // Count of the number of erasable blocks on the device
    uint16_t pageSize; // In bytes
    uint32_t sectorSize; // This is just pagesPerSector * pageSize
    uint32_t totalSize;  // This is just sectorSize * sectors
    uint16_t pagesPerSector;
} flashGeometry_t;

bool flashInit(void);

bool flashIsReady(void);
bool flashWaitForReady(timeMs_t timeoutMillis);
void flashEraseSector(uint32_t address);
void flashEraseCompletely(void);
#if 0
void flashPageProgramBegin(uint32_t address);
void flashPageProgramContinue(const uint8_t *data, int length);
void flashPageProgramFinish(void);
#endif
uint32_t flashPageProgram(uint32_t address, const uint8_t *data, int length);
int flashReadBytes(uint32_t address, uint8_t *buffer, int length);
void flashFlush(void);
const flashGeometry_t *flashGetGeometry(void);

//
// flash partitioning api
//

typedef struct flashPartition_s {
    uint8_t type;
    flashSector_t startSector;
    flashSector_t endSector;
} flashPartition_t;

#define FLASH_PARTITION_SECTOR_COUNT(partition) (partition->endSector + 1 - partition->startSector) // + 1 for inclusive, start and end sector can be the same sector.

// Must be in sync with flashPartitionTypeNames[]
// Should not be deleted or reordered once the code is writing a table to a flash.
typedef enum {
    FLASH_PARTITION_TYPE_UNKNOWN = 0,
    FLASH_PARTITION_TYPE_PARTITION_TABLE,
    FLASH_PARTITION_TYPE_FLASHFS,
    FLASH_PARTITION_TYPE_BADBLOCK_MANAGEMENT,
    FLASH_PARTITION_TYPE_FIRMWARE,
    FLASH_PARTITION_TYPE_CONFIG,
    FLASH_PARTITION_TYPE_FULL_BACKUP,
    FLASH_PARTITION_TYPE_FIRMWARE_UPDATE_META,
    FLASH_PARTITION_TYPE_UPDATE_FIRMWARE,
    FLASH_MAX_PARTITIONS
} flashPartitionType_e;

typedef struct flashPartitionTable_s {
    flashPartition_t partitions[FLASH_MAX_PARTITIONS];
} flashPartitionTable_t;

void flashPartitionSet(uint8_t index, uint32_t startSector, uint32_t endSector);
flashPartition_t *flashPartitionFindByType(flashPartitionType_e type);
const flashPartition_t *flashPartitionFindByIndex(uint8_t index);
const char *flashPartitionGetTypeName(flashPartitionType_e type);
int flashPartitionCount(void);
uint32_t flashPartitionSize(flashPartition_t *partition);
void flashPartitionErase(flashPartition_t *partition);

//#endif [> USE_FLASHFS <]
