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

#define DS2482_CONFIG_REG_APU (1<<0)    // Active pull-up
#define DS2482_CONFIG_REG_SPU (1<<2)    // Strong pull-up
#define DS2482_CONFIG_REG_WS (1<<3)     // 1-Wire speed

#define DS2482_STATUS_REG_1WB_POS 0     // 1-Wire busy
#define DS2482_STATUS_REG_PPD_POS 1     // Presense-pulse detect
#define DS2482_STATUS_REG_SD_POS 2      // Short detected
#define DS2482_STATUS_REG_LL_POS 3      // Logic level
#define DS2482_STATUS_REG_RST_POS 4     // Device reset
#define DS2482_STATUS_REG_SBR_POS 5     // Single bit result
#define DS2482_STATUS_REG_TSB_POS 6     // Triplet second bit
#define DS2482_STATUS_REG_DIR_POS 7     // Branch direction taken

#define DS2482_1WIRE_BUSY(status) (status & (1 << DS2482_STATUS_REG_1WB_POS))
#define DS2482_DEVICE_PRESENT(status) (status & (1 << DS2482_STATUS_REG_PPD_POS)) // True if a device have been detected on the bus after a bus reset
#define DS2482_RESET(status) (status & (1 << DS2482_STATUS_REG_RST_POS))
#define DS2482_LOGIC_LEVEL(status) (status & (1 << DS2482_STATUS_REG_LL_POS))
#define DS2482_SHORT_DETECTED(status) (status & (1 << DS2482_STATUS_REG_SD_POS))
#define DS2482_SBR_VALUE(status) ((status >> DS2482_STATUS_REG_SBR_POS) & 1)      // Extract single bit read value or triplet first bit from status register value
#define DS2482_TSB_VALUE(status) ((status >> DS2482_STATUS_REG_TSB_POS) & 1)      // Extract triplet second bit value from status register value
#define DS2482_DIR_VALUE(status) ((status >> DS2482_STATUS_REG_DIR_POS) & 1)      // Extract triplet chosen direction bit value from status register value

#define DS2482_1WIRE_SINGLE_BIT_WRITE0 0
#define DS2482_1WIRE_SINGLE_BIT_WRITE1_READ (1<<7)

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


static bool ds2482Reset(owDev_t *owDev)
{
    return busWrite(owDev->busDev, 0xFF, DS2482_RESET_CMD);
}

static bool ds2482SetReadPtr(owDev_t *owDev, uint8_t reg)
{
    return busWrite(owDev->busDev, DS2482_SET_READ_PTR_CMD, reg);
}

static bool ds2482ReadReg(owDev_t *owDev, uint8_t reg, uint8_t *byte)
{
    bool ack = ds2482SetReadPtr(owDev, reg);
    if (!ack) return false;
    return busRead(owDev->busDev, 0xFF, byte);
}

static bool ds2482ReadByte(owDev_t *owDev, uint8_t *byte)
{
    return ds2482ReadReg(owDev, DS2482_READ_DATA_REG_ADDR, byte);
}

static bool ds2482ReadConfig(owDev_t *owDev, uint8_t *config)
{
    return ds2482ReadReg(owDev, DS2482_CONFIG_REG_ADDR, config);
}

static bool ds2482WriteConfig(owDev_t *owDev, uint8_t config)
{
    return busWrite(owDev->busDev, DS2482_WRITE_CONFIG_CMD, DS2482_CONFIG_WRITE_BYTE(config));
}

static bool ds2482ReadStatus(owDev_t *owDev, uint8_t *status)
{
    bool ack = ds2482ReadReg(owDev, DS2482_STATUS_REG_ADDR, &owDev->status);
    if (!ack) return false;
    *status = owDev->status;
    return true;
}

static uint8_t ds2482GetStatus(owDev_t *owDev)
{
    return owDev->status;
}

static bool ds2482Poll(owDev_t *owDev, bool waitForBus, uint8_t *status)
{
    do {
        bool ack = busRead(owDev->busDev, 0xFF, &owDev->status);
        if (!ack) return false;
    } while (waitForBus && DS2482_1WIRE_BUSY(owDev->status));
    if (status) *status = owDev->status;
    return true;
}

static bool ds2482WaitForBus(owDev_t *owDev)
{
    return ds2482Poll(owDev, true, NULL);
}

static bool ds2482OwBusReady(owDev_t *owDev)
{
    bool ack = busRead(owDev->busDev, 0xFF, &owDev->status);
    if (!ack) return false;
    return !DS2482_1WIRE_BUSY(owDev->status);
}

static bool ds2482OwResetCommand(owDev_t *owDev)
{
    return busWrite(owDev->busDev, 0xFF, DS2482_1WIRE_RESET_CMD);
}

static bool ds2482OwReset(owDev_t *owDev)
{
    bool ack = ds2482OwResetCommand(owDev);
    if (!ack) return false;
    return ds2482WaitForBus(owDev);
}

static bool ds2482OwWriteByteCommand(owDev_t *owDev, uint8_t byte)
{
    return busWrite(owDev->busDev, DS2482_1WIRE_WRITE_BYTE_CMD, byte);
}

static bool ds2482OwWriteByte(owDev_t *owDev, uint8_t byte)
{
    bool ack = ds2482OwWriteByteCommand(owDev, byte);
    if (!ack) return false;
    return ds2482WaitForBus(owDev);
}

static bool ds2482OwWriteBuf(owDev_t *owDev, const uint8_t *buf, uint8_t len)
{
    for (uint8_t index = 0; index < len; ++index) {
        bool ack = ds2482OwWriteByte(owDev, buf[index]);
        if (!ack) return false;
    }
    return true;
}

static bool ds2482OwReadByteCommand(owDev_t *owDev)
{
    return busWrite(owDev->busDev, 0xFF, DS2482_1WIRE_READ_BYTE_CMD);
}

static bool ds2482OwReadByte(owDev_t *owDev, uint8_t *result)
{
    bool ack = ds2482OwReadByteCommand(owDev);
    if (!ack) return false;

    ack = ds2482WaitForBus(owDev);
    if (!ack) return false;

    return ds2482ReadByte(owDev, result);
}

static bool ds2482OwReadBuf(owDev_t *owDev, uint8_t *buf, uint8_t len)
{
    for (uint8_t index = 0; index < len; ++index) {
        bool ack = ds2482OwReadByte(owDev, buf + index);
        if (!ack) return false;
    }
    return true;
}

static bool ds2482OwSingleBitCommand(owDev_t *owDev, uint8_t type)
{
    return busWrite(owDev->busDev, DS2482_1WIRE_SINGLE_BIT_CMD, type);
}

static bool ds2482OwSingleBitResult(owDev_t *owDev)
{
    return DS2482_SBR_VALUE(owDev->status);
}

static bool ds2482OwSingleBit(owDev_t *owDev, uint8_t type, bool *result)
{
    bool ack = ds2482OwSingleBitCommand(owDev, type);

    ack = ds2482WaitForBus(owDev);
    if (!ack) return false;

    if (result) *result = ds2482OwSingleBitResult(owDev);
    return true;
}

static bool ds2482OwTripletCommand(owDev_t *owDev, uint8_t direction)
{
    return busWrite(owDev->busDev, DS2482_1WIRE_TRIPLET_CMD, direction << 7);
}

static uint8_t ds2482OwTripletResult(owDev_t *owDev)
{
    return owDev->status >> 5;
}

static bool ds2482OwTriplet(owDev_t *owDev, uint8_t direction, uint8_t *result)
{
    bool ack = ds2482OwTripletCommand(owDev, direction << 7);
    if (!ack) return false;
    ack = ds2482Poll(owDev, true, NULL);
    if (!ack) return false;
    *result = ds2482OwTripletResult(owDev);
    return true;
}

static bool ds2482OwSkipRomCommand(owDev_t *owDev)
{
    return ds2482OwWriteByte(owDev, _1WIRE_SKIP_ROM_CMD);
}

static bool ds2482OwSkipRom(owDev_t *owDev)
{
    bool ack = ds2482OwReset(owDev);
    if (!ack) return false;

    ack = ds2482OwSkipRomCommand(owDev);
    if (!ack) return false;

    return ds2482WaitForBus(owDev);
}

static bool ds2482OwMatchRomCommand(owDev_t *owDev)
{
    return ds2482OwWriteByteCommand(owDev, _1WIRE_MATCH_ROM_CMD);
}

static bool ds2482OwMatchRom(owDev_t *owDev, uint64_t rom)
{
    bool ack = ds2482OwReset(owDev);
    if (!ack) return false;

    ack = ds2482OwMatchRomCommand(owDev);
    if (!ack) return false;

    ack = ds2482WaitForBus(owDev);
    if (!ack) return false;

    for (uint8_t romByteIndex = 0; romByteIndex < 8; ++romByteIndex) {
        ack = ds2482OwWriteByte(owDev, ((uint8_t *)&rom)[romByteIndex]);
        if (!ack) return false;
    }

    return true;
}

static bool ds2482OwSearchRom(owDev_t *owDev, uint8_t family_code, uint64_t *rom_table, uint8_t *rom_table_len)
{
    bool ack;
    uint8_t last_collision_index = 0, rom_index = 0;

    do {

        uint8_t rom_byte_index = 0, rom_bit_index = 1, rom_byte_mask = 1;
        uint8_t dir_zero_last_index = 0; // Bit index where the 0 direction has been chosen after collision
        uint8_t *rom = (uint8_t *)&rom_table[rom_index];

        ack = ds2482OwReset(owDev);
        if (!ack) goto ds2482SearchRomReturn;

        ack = ds2482Poll(owDev, true, NULL);
        if (!ack) goto ds2482SearchRomReturn;

        if (!DS2482_DEVICE_PRESENT(owDev->status))
            goto ds2482SearchRomReturn;

        ack = ds2482OwWriteByte(owDev, _1WIRE_SEARCH_ROM_CMD);
        if (!ack) goto ds2482SearchRomReturn;

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

            ack = ds2482OwTripletCommand(owDev, direction);
            if (!ack) goto ds2482SearchRomReturn;

            ack = ds2482Poll(owDev, true, NULL);
            if (!ack) goto ds2482SearchRomReturn;

            uint8_t triplet_sbr = DS2482_SBR_VALUE(owDev->status);
            uint8_t triplet_tsb = DS2482_TSB_VALUE(owDev->status);
            uint8_t triplet_dir = DS2482_DIR_VALUE(owDev->status);

            if (triplet_sbr && triplet_tsb) break; // Error, the device have been disconnected during the search, restart

            if (family_code && (rom_bit_index < 9) && (triplet_dir != direction))
                goto ds2482SearchRomReturn;

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

ds2482SearchRomReturn:

    *rom_table_len = rom_index;

    return ack;

}


static bool ds2482Init(owDev_t *owDev)
{
    return ds2482WriteConfig(owDev, DS2482_CONFIG_REG_APU);
}

#define DETECTION_MAX_RETRY_COUNT 5
static bool deviceDetect(owDev_t *owDev)
{
    for (int retryCount = 0; retryCount < DETECTION_MAX_RETRY_COUNT; retryCount++) {
        delay(10);
        if (ds2482Reset(owDev)) return true;
    }

    return false;
}

bool ds2482Detect(owDev_t *owDev)
{
    owDev->busDev = busDeviceInit(BUSTYPE_I2C, DEVHW_DS2482, 0, OWNER_1WIRE);
    if (owDev->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(owDev)) {
        busDeviceDeInit(owDev->busDev);
        return false;
    }

    ds2482Init(owDev);

    owDev->reset = ds2482Reset;
    owDev->owResetCommand = ds2482OwResetCommand;
    owDev->owReset = ds2482OwReset;
    owDev->waitForBus = ds2482WaitForBus;
    owDev->readConfig = ds2482ReadConfig;
    owDev->writeConfig = ds2482WriteConfig;
    owDev->readStatus = ds2482ReadStatus;
    owDev->getStatus = ds2482GetStatus;
    owDev->poll = ds2482Poll;
    owDev->owBusReady = ds2482OwBusReady;

    owDev->owSearchRom = ds2482OwSearchRom;
    owDev->owMatchRomCommand = ds2482OwMatchRomCommand;
    owDev->owMatchRom = ds2482OwMatchRom;
    owDev->owSkipRomCommand = ds2482OwSkipRomCommand;
    owDev->owSkipRom = ds2482OwSkipRom;

    owDev->owWriteByteCommand = ds2482OwWriteByteCommand;
    owDev->owWriteByte = ds2482OwWriteByte;
    owDev->owWriteBuf = ds2482OwWriteBuf;
    owDev->owReadByteCommand = ds2482OwReadByteCommand;
    owDev->owReadByteResult = ds2482ReadByte;
    owDev->owReadByte = ds2482OwReadByte;
    owDev->owReadBuf = ds2482OwReadBuf;
    owDev->owSingleBitCommand = ds2482OwSingleBitCommand;
    owDev->owSingleBitResult = ds2482OwSingleBitResult;
    owDev->owSingleBit = ds2482OwSingleBit;
    owDev->owTripletCommand = ds2482OwTripletCommand;
    owDev->owTripletResult = ds2482OwTripletResult;
    owDev->owTriplet = ds2482OwTriplet;

    return true;
}

#endif /* defined(USE_1WIRE) && defined(USE_1WIRE_DS2482) */
