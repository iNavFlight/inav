/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include "platform.h"
#include "drivers/system.h"
#include "drivers/flash.h"
#include "config/config_streamer.h"

#if defined(CONFIG_IN_EXTERNAL_FLASH)

static bool streamerLocked = true;

void config_streamer_impl_unlock(void)
{
    const flashGeometry_t *flashGeometry = flashGetGeometry();
    if (flashGeometry->pageSize == CONFIG_STREAMER_BUFFER_SIZE) {
        // streamer needs to buffer exactly one flash page
        streamerLocked = false;
    }
}

void config_streamer_impl_lock(void)
{
    streamerLocked = true;
}

int config_streamer_impl_write_word(config_streamer_t *c, config_streamer_buffer_align_type_t *buffer)
{
    if (streamerLocked) {
        return -1;
    }

    uint32_t dataOffset = (uint32_t)(c->address - (uintptr_t)&eepromData[0]);

    const flashPartition_t *flashPartition = flashPartitionFindByType(FLASH_PARTITION_TYPE_CONFIG);
    const flashGeometry_t *flashGeometry = flashGetGeometry();

    uint32_t flashStartAddress = flashPartition->startSector * flashGeometry->sectorSize;
    uint32_t flashOverflowAddress = ((flashPartition->endSector + 1) * flashGeometry->sectorSize); // +1 to sector for inclusive

    uint32_t flashAddress = flashStartAddress + dataOffset;
    if (flashAddress + CONFIG_STREAMER_BUFFER_SIZE > flashOverflowAddress) {
        return -2; // address is past end of partition
    }

    uint32_t flashSectorSize = flashGeometry->sectorSize;

    if (flashAddress % flashSectorSize == 0) {
        flashEraseSector(flashAddress);
    }

    if (flashPageProgram(flashAddress, (uint8_t *)buffer, CONFIG_STREAMER_BUFFER_SIZE) == flashAddress) {
        // returned same address: programming failed
        return -3;
    }

    c->address += CONFIG_STREAMER_BUFFER_SIZE;

    return 0;
}

#endif
