/*
 * This file is part of Cleanflight, Betaflight and INAV
 *
 * Cleanflight, Betaflight and INAV are free software. You can
 * redistribute this software and/or modify this software under
 * the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cleanflight, Betaflight and INAV are distributed in the hope that
 * they will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdint.h>

#include "common/utils.h"

#define OSD_CHAR_WIDTH 12
#define OSD_CHAR_HEIGHT 18
#define OSD_CHAR_BITS_PER_PIXEL 2
#define OSD_CHAR_VISIBLE_BYTES (OSD_CHAR_WIDTH * OSD_CHAR_HEIGHT * OSD_CHAR_BITS_PER_PIXEL / 8)
// Only the first 54 bytes of a character represent visible data. However, some OSD drivers
// accept 64 bytes and use the extra 10 bytes for metadata.
#define OSD_CHAR_BYTES 64

#define OSD_CHARACTER_COLOR_BLACK 0
#define OSD_CHARACTER_COLOR_TRANSPARENT 1
#define OSD_CHARACTER_COLOR_WHITE 2

// 3 is unused but it's interpreted as transparent by all drivers

typedef struct displayCanvas_s displayCanvas_t;

// Video Character Display parameters

typedef enum {
    VIDEO_SYSTEM_AUTO = 0,
    VIDEO_SYSTEM_PAL,
    VIDEO_SYSTEM_NTSC,
    VIDEO_SYSTEM_HDZERO,
    VIDEO_SYSTEM_DJIWTF,
    VIDEO_SYSTEM_AVATAR,
    VIDEO_SYSTEM_DJICOMPAT,
    VIDEO_SYSTEM_DJICOMPAT_HD,
    VIDEO_SYSTEM_AUTOHD
} videoSystem_e;

typedef enum {
    OSD_DRIVER_NONE = 0,
    OSD_DRIVER_MAX7456 = 1,
} osdDriver_e;

// osdCharacter_t represents the binary data for an OSD
// character. All OSD drivers use the same character format
// as defined by OSD_CHARACTER_WIDTH, OSD_CHARACTER_HEIGHT
// and OSD_CHARACTER_BITS_PER_PIXEL.
typedef struct osdCharacter_s {
    uint8_t data[OSD_CHAR_BYTES];
} osdCharacter_t;

typedef struct osdUnit_t
{
    uint16_t scale;             // The scale between the value and the represented unit. e.g. if you're providing cms but you want to draw meters this should be 100 ([0, 1023])
    uint16_t symbol;            // Symbol to append/prepend to the value when it's not scaled [0, 511]
    uint16_t divisor;           // If abs(value) > divisor, divide it by this. e.g. for meters and km you'd set this to 1000 [0, UINT16_MAX)
    uint16_t divided_symbol;    // Symbol to append/prepend to the value when it's divided (e.g. the km symbol) [0, 511]
} osdUnit_t;

#define OSD_CHARACTER_GRID_MAX_WIDTH 30
#define OSD_CHARACTER_GRID_MAX_HEIGHT 16
#define OSD_CHARACTER_GRID_BUFFER_SIZE (OSD_CHARACTER_GRID_MAX_WIDTH * OSD_CHARACTER_GRID_MAX_HEIGHT)

extern uint16_t osdCharacterGridBuffer[OSD_CHARACTER_GRID_BUFFER_SIZE] ALIGNED(4);

// Sets all buffer entries to SYM_BLANK
void osdCharacterGridBufferClear(void);
void osdGridBufferClearGridRect(int x, int y, int w, int h);
void osdGridBufferClearPixelRect(displayCanvas_t *canvas, int x, int y, int w, int h);

uint16_t *osdCharacterGridBufferGetEntryPtr(unsigned x, unsigned y);
