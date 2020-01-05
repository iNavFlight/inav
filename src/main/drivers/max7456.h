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

#include <stdbool.h>

#include "drivers/osd.h"

#ifndef MAX7456_WHITEBRIGHTNESS
  #define MAX7456_WHITEBRIGHTNESS 0x01
#endif
#ifndef MAX7456_BLACKBRIGHTNESS
  #define MAX7456_BLACKBRIGHTNESS 0x00
#endif

#define MAX7456_BWBRIGHTNESS ((MAX7456_BLACKBRIGHTNESS << 2) | MAX7456_WHITEBRIGHTNESS)

#define MAX7456_CHARS_PER_LINE          30

/** PAL or NTSC, value is number of chars total */
#define MAX7456_LINES_NTSC          13
#define MAX7456_LINES_PAL           16
#define MAX7456_BUFFER_CHARS_NTSC   (MAX7456_LINES_NTSC * MAX7456_CHARS_PER_LINE)
#define MAX7456_BUFFER_CHARS_PAL    (MAX7456_LINES_PAL * MAX7456_CHARS_PER_LINE)

enum VIDEO_TYPES { AUTO = 0, PAL, NTSC };

#define MAX7456_MODE_INVERT   (1 << 3)
#define MAX7456_MODE_BLINK    (1 << 4)
#define MAX7456_MODE_SOLID_BG (1 << 5)

void max7456Init(const videoSystem_e videoSystem);
void max7456Update(void);
void max7456ReadNvm(uint16_t char_address, osdCharacter_t *chr);
void max7456WriteNvm(uint16_t char_address, const osdCharacter_t *chr);
uint16_t max7456GetScreenSize(void);
uint8_t max7456GetRowsCount(void);
void max7456Write(uint8_t x, uint8_t y, const char *buff, uint8_t mode);
void max7456WriteChar(uint8_t x, uint8_t y, uint16_t c, uint8_t mode);
bool max7456ReadChar(uint8_t x, uint8_t y, uint16_t *c, uint8_t *mode);
void max7456ClearScreen(void);
void max7456RefreshAll(void);
uint8_t* max7456GetScreenBuffer(void);
