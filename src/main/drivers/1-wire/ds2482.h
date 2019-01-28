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

#pragma once

//#include "drivers/io_types.h"
#include <stdbool.h>
#include "drivers/1-wire.h"

#if defined(USE_1WIRE) && defined(USE_1WIRE_DS2482)

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
#define DS2482_SBR_VALUE(status) ((status >> DS2482_STATUS_REG_SBR_POS) & 1)      // Extract single bit read value or triplet first bit from status register value
#define DS2482_TSB_VALUE(status) ((status >> DS2482_STATUS_REG_TSB_POS) & 1)      // Extract triplet second bit value from status register value
#define DS2482_DIR_VALUE(status) ((status >> DS2482_STATUS_REG_DIR_POS) & 1)      // Extract triplet chosen direction bit value from status register value

#define DS2482_1WIRE_SINGLE_BIT_WRITE0 0
#define DS2482_1WIRE_SINGLE_BIT_WRITE1_READ (1<<7)


bool ds2482Detect(_1WireDev_t *dev);
bool ds2482Init(_1WireDev_t *_1WireDev);

bool ds2482_reset(_1WireDev_t *_1WireDev);
bool ds2482_1wire_reset(_1WireDev_t *_1WireDev);
bool ds2482_1wire_reset_wait(_1WireDev_t *_1WireDev);
bool ds2482_wait_for_bus(_1WireDev_t *_1WireDev);
bool ds2482_config(_1WireDev_t *_1WireDev, uint8_t *config);
bool ds2482_write_config(_1WireDev_t *_1WireDev, uint8_t config);
bool ds2482_status(_1WireDev_t *_1WireDev, uint8_t *status);
bool ds2482_poll(_1WireDev_t *_1WireDev, bool wait_for_bus, uint8_t *status);
bool ds2482_read_byte(_1WireDev_t *_1WireDev, uint8_t *byte);

// 1-Wire ROM
bool ds2482_1wire_search_rom(_1WireDev_t *_1WireDev, uint8_t family_code, uint64_t *rom_table, uint8_t *rom_table_len);
bool ds2482_1wire_match_rom(_1WireDev_t *_1WireDev, uint64_t rom);
bool ds2482_1wire_reset_and_match_rom(_1WireDev_t *_1WireDev, uint64_t rom);
bool ds2482_1wire_skip_rom(_1WireDev_t *_1WireDev);
bool ds2482_1wire_skip_rom_wait(_1WireDev_t *_1WireDev);
bool ds2482_1wire_reset_and_skip_rom(_1WireDev_t *_1WireDev);

// 1-Wire read/write
bool ds2482_1wire_write_byte(_1WireDev_t *_1WireDev, uint8_t byte);
bool ds2482_1wire_write_byte_wait(_1WireDev_t *_1WireDev, uint8_t byte);
bool ds2482_1wire_write_buf(_1WireDev_t *_1WireDev, const uint8_t *buf, uint8_t len);
bool ds2482_1wire_read_byte(_1WireDev_t *_1WireDev);
bool ds2482_1wire_single_bit(_1WireDev_t *_1WireDev, uint8_t type);
bool ds2482_1wire_triplet(_1WireDev_t *_1WireDev, uint8_t direction);
bool ds2482_1wire_triplet_wait(_1WireDev_t *_1WireDev, uint8_t direction);

#endif /* defined(USE_1WIRE) && defined(USE_1WIRE_DS2482) */
