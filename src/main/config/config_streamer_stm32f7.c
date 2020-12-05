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

#if defined(STM32F7)

#if defined(STM32F745xx) || defined(STM32F746xx) || defined(STM32F765xx)
/*
Sector 0    0x08000000 - 0x08007FFF 32 Kbytes
Sector 1    0x08008000 - 0x0800FFFF 32 Kbytes
Sector 2    0x08010000 - 0x08017FFF 32 Kbytes
Sector 3    0x08018000 - 0x0801FFFF 32 Kbytes
Sector 4    0x08020000 - 0x0803FFFF 128 Kbytes
Sector 5    0x08040000 - 0x0807FFFF 256 Kbytes
Sector 6    0x08080000 - 0x080BFFFF 256 Kbytes
Sector 7    0x080C0000 - 0x080FFFFF 256 Kbytes
*/
#define FLASH_PAGE_SIZE ((uint32_t)0x8000)
static uint32_t getFLASHSectorForEEPROM(uint32_t address)
{
    if (address <= 0x08007FFF)
        return FLASH_SECTOR_0;
    if (address <= 0x0800FFFF)
        return FLASH_SECTOR_1;
    if (address <= 0x08017FFF)
        return FLASH_SECTOR_2;
    if (address <= 0x0801FFFF)
        return FLASH_SECTOR_3;
    if (address <= 0x0803FFFF)
        return FLASH_SECTOR_4;
    if (address <= 0x0807FFFF)
        return FLASH_SECTOR_5;
    if (address <= 0x080BFFFF)
        return FLASH_SECTOR_6;
    if (address <= 0x080FFFFF)
        return FLASH_SECTOR_7;

    // Not good
    while (1) {
        failureMode(FAILURE_FLASH_WRITE_FAILED);
    }
}
#elif defined(STM32F722xx)
/*
Sector 0    0x08000000 - 0x08003FFF 16 Kbytes
Sector 1    0x08004000 - 0x08007FFF 16 Kbytes
Sector 2    0x08008000 - 0x0800BFFF 16 Kbytes
Sector 3    0x0800C000 - 0x0800FFFF 16 Kbytes
Sector 4    0x08010000 - 0x0801FFFF 64 Kbytes
Sector 5    0x08020000 - 0x0803FFFF 128 Kbytes
Sector 6    0x08040000 - 0x0805FFFF 128 Kbytes
Sector 7    0x08060000 - 0x0807FFFF 128 Kbytes
*/
#define FLASH_PAGE_SIZE ((uint32_t)0x4000)
static uint32_t getFLASHSectorForEEPROM(uint32_t address)
{
    if (address <= 0x08003FFF)
        return FLASH_SECTOR_0;
    if (address <= 0x08007FFF)
        return FLASH_SECTOR_1;
    if (address <= 0x0800BFFF)
        return FLASH_SECTOR_2;
    if (address <= 0x0800FFFF)
        return FLASH_SECTOR_3;
    if (address <= 0x0801FFFF)
        return FLASH_SECTOR_4;
    if (address <= 0x0803FFFF)
        return FLASH_SECTOR_5;
    if (address <= 0x0805FFFF)
        return FLASH_SECTOR_6;
    if (address <= 0x0807FFFF)
        return FLASH_SECTOR_7;

    // Not good
    while (1) {
        failureMode(FAILURE_FLASH_WRITE_FAILED);
    }
}
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
            .NbSectors     = 1
        };
        EraseInitStruct.Sector = getFLASHSectorForEEPROM(c->address);

        uint32_t SECTORError;
        const HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError);
        if (status != HAL_OK){
            return -1;
        }
    }

    const HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, c->address, (uint64_t)*buffer);
    if (status != HAL_OK) {
        return -2;
    }

    c->address += CONFIG_STREAMER_BUFFER_SIZE;
    return 0;    
}

#endif