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

#if defined(USE_1WIRE) && defined(USE_TEMPERATURE_DS18B20)


bool ds18b20Enumerate(owDev_t *owDev, uint64_t *rom_table, uint8_t *rom_table_len)
{
    return owDev->owSearchRom(owDev, DS18B20_FAMILY_CODE, rom_table, rom_table_len);
}

bool ds18b20Configure(owDev_t *owDev, uint64_t rom, uint8_t config)
{
    bool ack = owDev->owMatchRom(owDev, rom);
    if (!ack) return false;

    uint8_t buf[4] = { DS18B20_WRITE_SCRATCHPAD_CMD, 0, 0, config };
    return owDev->owWriteBuf(owDev, buf, sizeof(buf));
}

static bool readPowerSupply(owDev_t *owDev, bool *result)
{
    bool ack = owDev->owWriteByte(owDev, DS18B20_READ_POWER_SUPPLY_CMD);
    if (!ack) return false;

    bool sbr;
    ack = owDev->owSingleBit(owDev, OW_SINGLE_BIT_WRITE1_READ, &sbr);
    if (!ack) return false;

    *result = !sbr;
    return true;
}

bool ds18b20ParasiticPoweredPresent(owDev_t *owDev, bool *result)
{
    bool ack = owDev->owSkipRom(owDev);
    if (!ack) return false;

    return readPowerSupply(owDev, result);
}

bool ds18b20ReadPowerSupply(owDev_t *owDev, uint64_t rom, bool *parasiticPowered)
{
    bool ack = owDev->owMatchRom(owDev, rom);
    if (!ack) return false;

    return readPowerSupply(owDev, parasiticPowered);
}

bool ds18b20ReadScratchpadCommand(owDev_t *owDev)
{
    return owDev->owWriteByteCommand(owDev, DS18B20_READ_SCRATCHPAD_CMD);
}

static bool ds18b20ReadScratchpadBuf(owDev_t *owDev, uint8_t *buf, uint8_t len)
{
    bool ack = ds18b20ReadScratchpadCommand(owDev);
    if (!ack) return false;
    ack = owDev->waitForBus(owDev);
    if (!ack) return false;
    return owDev->owReadBuf(owDev, buf, len);
}

bool ds18b20StartConversionCommand(owDev_t *owDev)
{
    return owDev->owWriteByteCommand(owDev, DS18B20_START_CONVERSION_CMD);
}

// start conversion on all devices present on the bus
// note: parasitic power only supports one device converting at a time
bool ds18b20StartConversion(owDev_t *owDev)
{
    bool ack = owDev->owSkipRom(owDev);
    if (!ack) return false;
    return ds18b20StartConversionCommand(owDev);
}

bool ds18b20WaitForConversion(owDev_t *owDev)
{
    bool ack = owDev->waitForBus(owDev);
    if (!ack) return false;

    bool read_bit;
    do {
        ack = owDev->owSingleBit(owDev, OW_SINGLE_BIT_WRITE1_READ, &read_bit);
        if (!ack) return false;
    } while (!read_bit);

    return true;
}

bool ds18b20ReadTemperatureFromScratchPadBuf(const uint8_t *buf, int16_t *temperature)
{
    if (buf[8] != ds_crc8(buf, 8)) return false;
    *temperature = (int16_t)(((buf[0] | (buf[1] << 8)) >> 3) | ((buf[1] & 0x80) ? 0xE000 : 0)) * 5;
    return true;
}

bool ds18b20ReadTemperature(owDev_t *owDev, uint64_t rom, int16_t *temperature)
{
    bool ack = owDev->owMatchRom(owDev, rom);
    if (!ack) return false;

    uint8_t buf[9];
    ack = ds18b20ReadScratchpadBuf(owDev, buf, 9);
    if (!ack) return false;

    return ds18b20ReadTemperatureFromScratchPadBuf(buf, temperature);
}

#endif /* defined(USE_1WIRE) && defined(USE_TEMPERATURE_DS18B20) */
