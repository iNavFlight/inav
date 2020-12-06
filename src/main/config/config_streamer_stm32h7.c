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
#include "config/config_streamer.h"

#if defined(STM32H7)

#if defined(STM32H743xx)
/* Sectors 0-7 of 128K each */
#define FLASH_PAGE_SIZE     ((uint32_t)0x20000) // 128K sectors
static uint32_t getFLASHSectorForEEPROM(uint32_t address)
{
    if (address <= 0x0801FFFF)
        return FLASH_SECTOR_0;
    if (address <= 0x0803FFFF)
        return FLASH_SECTOR_1;
    if (address <= 0x0805FFFF)
        return FLASH_SECTOR_2;
    if (address <= 0x0807FFFF)
        return FLASH_SECTOR_3;
    if (address <= 0x0809FFFF)
        return FLASH_SECTOR_4;
    if (address <= 0x080BFFFF)
        return FLASH_SECTOR_5;
    if (address <= 0x080DFFFF)
        return FLASH_SECTOR_6;
    if (address <= 0x080FFFFF)
        return FLASH_SECTOR_7;

    while (1) {
        failureMode(FAILURE_FLASH_WRITE_FAILED);
    }
}
#elif defined(STM32H750xx)
#  error "STM32750xx only has one flash page which contains the bootloader, no spare flash pages available, use external storage for persistent config or ram for target testing"
#else
#  error "Unsupported CPU!"
#endif

void config_streamer_impl_unlock(void)
{
    HAL_FLASH_Unlock();
}

void config_streamer_impl_lock(void)
{
    HAL_FLASH_Lock();
}

int config_streamer_impl_write_word(config_streamer_t *c, config_streamer_buffer_align_type_t *buffer)
{
    if (c->err != 0) {
        return c->err;
    }

    if (c->address % FLASH_PAGE_SIZE == 0) {
        FLASH_EraseInitTypeDef EraseInitStruct = {
            .TypeErase     = FLASH_TYPEERASE_SECTORS,
            .VoltageRange  = FLASH_VOLTAGE_RANGE_3, // 2.7-3.6V
            .NbSectors     = 1,
            .Banks         = FLASH_BANK_1
        };
        EraseInitStruct.Sector = getFLASHSectorForEEPROM(c->address);

        uint32_t SECTORError;
        const HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError);
        if (status != HAL_OK) {
            return -1;
        }
    }

    // On H7 HAL_FLASH_Program takes data address, not the raw word value
    const HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, c->address, (uint32_t)buffer);
    if (status != HAL_OK) {
        return -2;
    }

    c->address += CONFIG_STREAMER_BUFFER_SIZE;
    return 0;
}

#endif