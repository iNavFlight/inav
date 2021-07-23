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

#include <stdint.h>
#include <stdbool.h>

#include "drivers/flash_m25p16.h"

// Streams data out to the EEPROM, padding to the write size as
// needed, and updating the checksum as it goes.

#ifdef CONFIG_IN_EXTERNAL_FLASH
#define CONFIG_STREAMER_BUFFER_SIZE M25P16_PAGESIZE // Must match flash device page size
typedef uint32_t config_streamer_buffer_align_type_t;
#elif defined(STM32H7)
#define CONFIG_STREAMER_BUFFER_SIZE 32  // Flash word = 256-bits
typedef uint64_t config_streamer_buffer_align_type_t;
#else
#define CONFIG_STREAMER_BUFFER_SIZE 4
typedef uint32_t config_streamer_buffer_align_type_t;
#endif

typedef struct config_streamer_s {
    uintptr_t address;
    uintptr_t end;
    int size;
    union {
        uint8_t b[CONFIG_STREAMER_BUFFER_SIZE];
        config_streamer_buffer_align_type_t w;
    } buffer;
    int at;
    int err;
    bool unlocked;
} config_streamer_t;

void config_streamer_init(config_streamer_t *c);

void config_streamer_start(config_streamer_t *c, uintptr_t base, int size);
int config_streamer_write(config_streamer_t *c, const uint8_t *p, uint32_t size);
int config_streamer_flush(config_streamer_t *c);

int config_streamer_finish(config_streamer_t *c);
int config_streamer_status(config_streamer_t *c);
