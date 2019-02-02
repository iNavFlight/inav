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

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#include "build/debug.h"

#include "drivers/1-wire.h"
#include "drivers/1-wire/ds_crc.h"
#include "drivers/1-wire/ds2482.h"
#include "drivers/time.h"


#define DS2482_STATUS_REG_ADDR 0xF0
#define DS2482_READ_DATA_REG_ADDR 0xE1
#define DS2482_CONFIG_REG_ADDR 0xC3

#define DS2482_CONFIG_WRITE_BYTE(config) (config | ((~config & 0xF) << 4)) // Config's upper nibble should be the one's complement of lower nibble when writing

#define DS2482_RESET_CMD 0xF0
#define DS2482_SET_READ_PTR_CMD 0xE1
#define DS2482_WRITE_CONFIG_CMD 0xD2
#define DS2482_1WIRE_RESET_CMD 0xB4
#define DS2482_1WIRE_SINGLE_BIT_CMD 0x87
#define DS2482_1WIRE_WRITE_BYTE_CMD 0xA5
#define DS2482_1WIRE_READ_BYTE_CMD 0x96
#define DS2482_1WIRE_TRIPLET_CMD 0x78

#define _1WIRE_SEARCH_ROM_CMD 0xF0
#define _1WIRE_READ_ROM_CMD 0x33
#define _1WIRE_MATCH_ROM_CMD 0x55
#define _1WIRE_SKIP_ROM_CMD 0xCC



#if defined(USE_1WIRE) && defined(USE_1WIRE_DS2482)

bool ds2482_reset(_1WireDev_t *_1WireDev)
{
    return busWrite(_1WireDev->busDev, 0xFF, DS2482_RESET_CMD);
}

static bool ds2482_set_read_ptr(_1WireDev_t *_1WireDev, uint8_t reg)
{
    return busWrite(_1WireDev->busDev, DS2482_SET_READ_PTR_CMD, reg);
}

static bool ds2482_read_reg(_1WireDev_t *_1WireDev, uint8_t reg, uint8_t *byte)
{
    bool ack = ds2482_set_read_ptr(_1WireDev, reg);
    if (!ack) return false;
    return busRead(_1WireDev->busDev, 0xFF, byte);
}

bool ds2482_read_byte(_1WireDev_t *_1WireDev, uint8_t *byte)
{
    return ds2482_read_reg(_1WireDev, DS2482_READ_DATA_REG_ADDR, byte);
}

bool ds2482_config(_1WireDev_t *_1WireDev, uint8_t *config)
{
    return ds2482_read_reg(_1WireDev, DS2482_CONFIG_REG_ADDR, config);
}

bool ds2482_status(_1WireDev_t *_1WireDev, uint8_t *status)
{
    return ds2482_read_reg(_1WireDev, DS2482_STATUS_REG_ADDR, status);
}

bool ds2482_write_config(_1WireDev_t *_1WireDev, uint8_t config)
{
    return busWrite(_1WireDev->busDev, DS2482_WRITE_CONFIG_CMD, DS2482_CONFIG_WRITE_BYTE(config));
}

bool ds2482_poll(_1WireDev_t *_1WireDev, bool wait_for_bus, uint8_t *status)
{
    uint8_t status_temp;
    do {
        bool ack = busRead(_1WireDev->busDev, 0xFF, &status_temp);
        if (!ack) return false;
    } while (wait_for_bus && DS2482_1WIRE_BUSY(status_temp));
    if (status) *status = status_temp;
    return true;
}

bool ds2482_wait_for_bus(_1WireDev_t *_1WireDev)
{
    return ds2482_poll(_1WireDev, true, NULL);
}

bool ds2482_1wire_reset(_1WireDev_t *_1WireDev)
{
    return busWrite(_1WireDev->busDev, 0xFF, DS2482_1WIRE_RESET_CMD);
}

bool ds2482_1wire_reset_wait(_1WireDev_t *_1WireDev)
{
    bool ack = ds2482_1wire_reset(_1WireDev);
    if (!ack) return false;
    return ds2482_wait_for_bus(_1WireDev);
}

bool ds2482_1wire_write_byte(_1WireDev_t *_1WireDev, uint8_t byte)
{
    return busWrite(_1WireDev->busDev, DS2482_1WIRE_WRITE_BYTE_CMD, byte);
}

bool ds2482_1wire_write_byte_wait(_1WireDev_t *_1WireDev, uint8_t byte)
{
    bool ack = ds2482_1wire_write_byte(_1WireDev, byte);
    if (!ack) return false;
    return ds2482_wait_for_bus(_1WireDev);
}

bool ds2482_1wire_write_buf(_1WireDev_t *_1WireDev, const uint8_t *buf, uint8_t len)
{
    for (uint8_t index = 0; index < len; ++index) {
        bool ack = ds2482_1wire_write_byte_wait(_1WireDev, buf[index]);
        if (!ack) return false;
    }
    return true;
}

bool ds2482_1wire_read_byte(_1WireDev_t *_1WireDev)
{
    return busWrite(_1WireDev->busDev, 0xFF, DS2482_1WIRE_READ_BYTE_CMD);
}

bool ds2482_1wire_single_bit(_1WireDev_t *_1WireDev, uint8_t type)
{
    return busWrite(_1WireDev->busDev, DS2482_1WIRE_SINGLE_BIT_CMD, type);
}

bool ds2482_1wire_triplet(_1WireDev_t *_1WireDev, uint8_t direction)
{
    return busWrite(_1WireDev->busDev, DS2482_1WIRE_TRIPLET_CMD, direction << 7);
}

bool ds2482_1wire_triplet_wait(_1WireDev_t *_1WireDev, uint8_t direction)
{
    bool ack = ds2482_1wire_triplet(_1WireDev, direction << 7);
    if (!ack) return false;
    return ds2482_wait_for_bus(_1WireDev);
}

bool ds2482_1wire_skip_rom(_1WireDev_t *_1WireDev)
{
    return ds2482_1wire_write_byte(_1WireDev, _1WIRE_SKIP_ROM_CMD);
}

bool ds2482_1wire_skip_rom_wait(_1WireDev_t *_1WireDev)
{
    bool ack = ds2482_1wire_write_byte(_1WireDev, _1WIRE_SKIP_ROM_CMD);
    if (!ack) return false;
    return ds2482_wait_for_bus(_1WireDev);
}

bool ds2482_1wire_match_rom(_1WireDev_t *_1WireDev, uint64_t rom)
{
    bool ack = ds2482_1wire_write_byte_wait(_1WireDev, _1WIRE_MATCH_ROM_CMD);
    if (!ack) return false;

    for (uint8_t rom_byte_index = 0; rom_byte_index < 8; ++rom_byte_index) {
        ack = ds2482_1wire_write_byte_wait(_1WireDev, ((uint8_t *)&rom)[rom_byte_index]);
        if (!ack) return false;
    }

    return true;
}

bool ds2482_1wire_reset_and_match_rom(_1WireDev_t *_1WireDev, uint64_t rom)
{
    bool ack = ds2482_1wire_reset_wait(_1WireDev);
    if (!ack) return false;
    return ds2482_1wire_match_rom(_1WireDev, rom);
}

bool ds2482_1wire_search_rom(_1WireDev_t *_1WireDev, uint8_t family_code, uint64_t *rom_table, uint8_t *rom_table_len)
{
    bool ack;
    uint8_t last_collision_index = 0, rom_index = 0;

    do {

        uint8_t status;
        uint8_t rom_byte_index = 0, rom_bit_index = 1, rom_byte_mask = 1;
        uint8_t dir_zero_last_index = 0; // Bit index where the 0 direction has been chosen after collision
        uint8_t *rom = (uint8_t *)&rom_table[rom_index];

        ack = ds2482_1wire_reset(_1WireDev);
        if (!ack) goto ds2482_search_rom_return;

        ack = ds2482_poll(_1WireDev, true, &status);
        if (!ack) goto ds2482_search_rom_return;

        if (!DS2482_DEVICE_PRESENT(status))
            goto ds2482_search_rom_return;

        ack = ds2482_1wire_write_byte_wait(_1WireDev, _1WIRE_SEARCH_ROM_CMD);
        if (!ack) goto ds2482_search_rom_return;

        do {

            uint8_t direction;
            if (family_code && (rom_bit_index < 9)) {
                direction = (family_code >> (rom_bit_index - 1)) & 1;
            } else if ((rom_index > 0) && (rom_bit_index < last_collision_index)) {
                const uint8_t *previous_rom = (uint8_t *)&rom_table[rom_index-1];
                direction = (previous_rom[rom_byte_index] & rom_byte_mask) > 0;
            } else {
                direction = rom_bit_index == last_collision_index;
            }

            ack = ds2482_1wire_triplet(_1WireDev, direction);
            if (!ack) goto ds2482_search_rom_return;

            ack = ds2482_poll(_1WireDev, true, &status);
            if (!ack) goto ds2482_search_rom_return;

            uint8_t triplet_sbr = DS2482_SBR_VALUE(status);
            uint8_t triplet_tsb = DS2482_TSB_VALUE(status);
            uint8_t triplet_dir = DS2482_DIR_VALUE(status);

            if (triplet_sbr && triplet_tsb) break; // Error, the device have been disconnected during the search, restart

            if (family_code && (rom_bit_index < 9) && (triplet_dir != direction))
                goto ds2482_search_rom_return;

            if (triplet_dir)
                rom[rom_byte_index] |= rom_byte_mask;
            else {
                if (!(triplet_sbr || triplet_tsb)) dir_zero_last_index = rom_bit_index; // Collision
                rom[rom_byte_index] &= ~rom_byte_mask;
            }

            rom_bit_index += 1;

            if (!(rom_byte_mask <<= 1)) {
                rom_byte_index += 1;
                rom_byte_mask = 1;
            }

        } while (rom_byte_index < 8);

        if ((rom_bit_index > 64) && (rom[7] == ds_crc8(rom, 7))) {
            rom_index += 1;
            last_collision_index = dir_zero_last_index;
            if (!last_collision_index) break; // All the devices have been found
        }

    } while (rom_index < *rom_table_len);

ds2482_search_rom_return:

    *rom_table_len = rom_index;

    return ack;

}

bool ds2482_1wire_reset_and_skip_rom(_1WireDev_t *_1WireDev)
{
    bool ack = ds2482_1wire_reset_wait(_1WireDev);
    if (!ack) return false;
    return ds2482_1wire_skip_rom_wait(_1WireDev);
}


#define DETECTION_MAX_RETRY_COUNT 5
static bool deviceDetect(_1WireDev_t *_1WireDev)
{
    for (int retryCount = 0; retryCount < DETECTION_MAX_RETRY_COUNT; retryCount++) {
        delay(10);
        if (ds2482_reset(_1WireDev)) return true;
    }

    return false;
}

bool ds2482Detect(_1WireDev_t *_1WireDev)
{
    _1WireDev->busDev = busDeviceInit(BUSTYPE_I2C, DEVHW_DS2482, 0, OWNER_1WIRE);
    if (_1WireDev->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(_1WireDev)) {
        busDeviceDeInit(_1WireDev->busDev);
        return false;
    }

    return true;
}

bool ds2482Init(_1WireDev_t *_1WireDev)
{
    return ds2482_write_config(_1WireDev, DS2482_CONFIG_REG_APU);
}

#endif /* defined(USE_1WIRE) && defined(USE_1WIRE_DS2482) */
