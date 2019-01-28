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
#include "drivers/1-wire/ds2482.h"
#include "drivers/1-wire/ds_crc.h"
#include "drivers/temperature/ds18b20.h"


#define DS18B20_FAMILY_CODE 0x28

#define DS18B20_ALARM_SEARCH_CMD 0xEC
#define DS18B20_START_CONVERSION_CMD 0x44
#define DS18B20_WRITE_SCRATCHPAD_CMD 0x4E
#define DS18B20_READ_SCRATCHPAD_CMD 0xBE
#define DS18B20_COPY_SCRATCHPAD_CMD 0x48
#define DS18B20_RECALL_EEPROM_CMD 0xB8
#define DS18B20_READ_POWER_SUPPLY_CMD 0xB4

#if defined(USE_1WIRE) && defined(USE_1WIRE_DS2482) && defined(USE_TEMPERATURE_DS18B20)

#define _1WireDev (&ds2482Dev)

bool ds18b20_enumerate(uint64_t *rom_table, uint8_t *rom_table_len)
{
    return ds2482_1wire_search_rom(_1WireDev, DS18B20_FAMILY_CODE, rom_table, rom_table_len);
}

bool ds18b20_configure(uint64_t rom, uint8_t config)
{
    bool ack = ds2482_1wire_reset_and_match_rom(_1WireDev, rom);
    if (!ack) return false;

    uint8_t buf[4] = { DS18B20_WRITE_SCRATCHPAD_CMD, 0, 0, config };
    return ds2482_1wire_write_buf(_1WireDev, buf, sizeof(buf));
}

static bool ds18b20_parasitic_powered_present()
{
    bool ack = ds2482_1wire_reset_and_skip_rom(_1WireDev);
    if (!ack) return false;

    ack = ds2482_1wire_write_byte(_1WireDev, DS18B20_READ_POWER_SUPPLY_CMD);
    if (!ack) return false;

    uint8_t status;
    ack = ds2482_poll(_1WireDev, true, &status);
    if (!ack) return false;

    return !DS2482_SBR_VALUE(status);
}

static bool ds18b20_read_scratchpad()
{
    return ds2482_1wire_write_byte(_1WireDev, DS18B20_READ_SCRATCHPAD_CMD);
}

static bool ds18b20_read_scratchpad_buf(uint8_t *buf, uint8_t len)
{
    ds18b20_read_scratchpad(_1WireDev);
    ds2482_wait_for_bus(_1WireDev);

    for (uint8_t index = 0; index < len; ++index) {
        bool ack = ds2482_1wire_read_byte(_1WireDev);
        if (!ack) return false;

        ack = ds2482_wait_for_bus(_1WireDev);
        if (!ack) return false;

        ack = ds2482_read_byte(_1WireDev, buf + index);
        if (!ack) return false;
    }
    return true;
}

bool ds18b20_start_conversion()
{
    bool parasitic_power = ds18b20_parasitic_powered_present(_1WireDev);
    if (parasitic_power) return false;
    bool ack = ds2482_1wire_reset_and_skip_rom(_1WireDev);
    if (!ack) return false;
    return ds2482_1wire_write_byte(_1WireDev, DS18B20_START_CONVERSION_CMD);
}

bool ds18b20_wait_for_conversion()
{
    bool ack;
    uint8_t status, read_bit = 0;
    while (!read_bit) {
        ack = ds2482_wait_for_bus(_1WireDev);
        if (!ack) return false;

        ack = ds2482_1wire_single_bit(_1WireDev, DS2482_1WIRE_SINGLE_BIT_WRITE1_READ);
        if (!ack) return false;

        while (1) {
            ack = busRead(_1WireDev->busDev, 0xFF, &status);
            if (!ack) return false;
            if ((status & 1) == 0) break;
        }

        read_bit = DS2482_SBR_VALUE(status);

    }
    return true;
}

bool ds18b20_read_temperature(uint64_t rom, int16_t *temperature)
{
    uint8_t buf[9];
    bool ack = ds2482_1wire_reset_and_match_rom(_1WireDev, rom);
    if (!ack) return false;
    ack = ds18b20_read_scratchpad_buf(buf, 9);
    if (!ack) return false;
    if (buf[8] != ds_crc8(buf, 8)) return false;
    *temperature = (int16_t)(((buf[0] | (buf[1] << 8)) >> 3) | ((buf[1] & 0x80) ? 0xE000 : 0)) * 5;
    return true;
}

#endif /* defined(USE_1WIRE) && defined(USE_1WIRE_DS2482) && defined(USE_TEMPERATURE_DS18B20) */
