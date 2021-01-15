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

#include <stdint.h>

#include "platform.h"

#define FIRMWARE_UPDATE_FIRMWARE_FILENAME "firmware.upt"
#define FIRMWARE_UPDATE_BACKUP_FILENAME "firmware.bak"
#define FIRMWARE_UPDATE_META_FILENAME "update.mta"

#define FIRMWARE_UPDATE_METADATA_MAGIC 0xAABBCCDD

#undef FLASH_END

#define FIRMWARE_START_ADDRESS ((uint32_t)&__firmware_start)
#define FLASH_START_ADDRESS 0x08000000UL
#define FLASH_END (FLASH_START_ADDRESS + FLASH_SIZE * 1024)
#define CONFIG_START_ADDRESS ((uint32_t)&__config_start)
#define CONFIG_END_ADDRESS ((uint32_t)&__config_end)

#define AVAILABLE_FIRMWARE_SPACE (FLASH_END - FIRMWARE_START_ADDRESS)

extern uint8_t __firmware_start; // set via linker
extern uint8_t __config_start;
extern uint8_t __config_end;

typedef struct {
    uint32_t magic;
#ifdef USE_FLASHFS
    uint32_t firmwareSize;
    bool dataFlashErased;
#endif
    uint8_t backupCRC;
} firmwareUpdateMetadata_t;

bool firmwareUpdateMetadataRead(firmwareUpdateMetadata_t *updateMetadata);
bool firmwareUpdateMetadataWrite(firmwareUpdateMetadata_t *updateMetadata);
