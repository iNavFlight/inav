/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
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

// Available RTC backup registers (4-byte words) per MCU type
// F4: 20 words
// F7: 32 words
// H7: 32 words

typedef enum {
    PERSISTENT_OBJECT_MAGIC = 0,
    PERSISTENT_OBJECT_RESET_REASON,
    PERSISTENT_OBJECT_COUNT,
} persistentObjectId_e;

// Values for PERSISTENT_OBJECT_RESET_REASON
#define RESET_NONE                                      0
#define RESET_BOOTLOADER_REQUEST_ROM                    1  // DFU request
#define RESET_MSC_REQUEST                               2  // MSC invocation was requested
#define RESET_BOOTLOADER_FIRMWARE_UPDATE                3  // Bootloader request to flash stored firmware update
#define RESET_BOOTLOADER_FIRMWARE_ROLLBACK              4  // Bootloader request to rollback to stored firmware and config backup
#define RESET_BOOTLOADER_FIRMWARE_UPDATE_SUCCESS        5
#define RESET_BOOTLOADER_FIRMWARE_UPDATE_FAILED         6

void persistentObjectInit(void);
uint32_t persistentObjectRead(persistentObjectId_e id);
void persistentObjectWrite(persistentObjectId_e id, uint32_t value);
