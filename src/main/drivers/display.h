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
#include <stdint.h>

#include "config/parameter_group.h"

typedef struct displayConfig_s {
    bool force_sw_blink; // Enable SW blinking. Used for chips which don't work correctly with HW blink.
} displayConfig_t;

PG_DECLARE(displayConfig_t, displayConfig);

// Represents the attributes for a given piece of text
// either a single character or a string. For forward
// compatibility, always use the TEXT_ATTRIBUTE...
// macros when manipulating or testing textAttributes_t.
typedef uint8_t textAttributes_t;


#define _TEXT_ATTRIBUTES_BLINK_BIT          (1 << 0)
#define _TEXT_ATTRIBUTES_INVERTED_BIT       (1 << 1)
#define _TEXT_ATTRIBUTES_SOLID_BG_BIT       (1 << 2)

#define TEXT_ATTRIBUTES_NONE                0
#define TEXT_ATTRIBUTES_ADD_BLINK(x)        ((x) |= _TEXT_ATTRIBUTES_BLINK_BIT)
#define TEXT_ATTRIBUTES_ADD_INVERTED(x)     ((x) |= _TEXT_ATTRIBUTES_INVERTED_BIT)
#define TEXT_ATTRIBUTES_ADD_SOLID_BG(x)     ((x) |= _TEXT_ATTRIBUTES_SOLID_BG_BIT)

#define TEXT_ATTRIBUTES_REMOVE_BLINK(x)     ((x) &= ~_TEXT_ATTRIBUTES_BLINK_BIT)
#define TEXT_ATTRIBUTES_REMOVE_INVERTED(x)  ((x) &= ~_TEXT_ATTRIBUTES_INVERTED_BIT)
#define TEXT_ATTRIBUTES_REMOVE_SOLID_BG(x)  ((x) &= ~_TEXT_ATTRIBUTES_SOLID_BG_BIT)

#define TEXT_ATTRIBUTES_HAVE_BLINK(x)       (x & _TEXT_ATTRIBUTES_BLINK_BIT)
#define TEXT_ATTRIBUTES_HAVE_INVERTED(x)    (x & _TEXT_ATTRIBUTES_INVERTED_BIT)
#define TEXT_ATTRIBUTES_HAVE_SOLID_BG(x)    (x & _TEXT_ATTRIBUTES_SOLID_BG_BIT)

static inline void TEXT_ATTRIBUTES_COPY(textAttributes_t *dst, textAttributes_t *src) { *dst = *src; }

typedef struct displayFontMetadata_s {
    uint8_t version;
    uint16_t charCount;
} displayFontMetadata_t;

struct displayPortVTable_s;
typedef struct displayPort_s {
    const struct displayPortVTable_s *vTable;
    void *device;
    uint8_t rows;
    uint8_t cols;
    uint8_t posX;
    uint8_t posY;

    // CMS state
    bool cleared;
    int8_t cursorRow;
    int8_t grabCount;
    textAttributes_t cachedSupportedTextAttributes;
    uint16_t maxChar;
} displayPort_t;

typedef struct displayPortVTable_s {
    int (*grab)(displayPort_t *displayPort);
    int (*release)(displayPort_t *displayPort);
    int (*clearScreen)(displayPort_t *displayPort);
    int (*drawScreen)(displayPort_t *displayPort);
    int (*screenSize)(const displayPort_t *displayPort);
    int (*writeString)(displayPort_t *displayPort, uint8_t x, uint8_t y, const char *text, textAttributes_t attr);
    int (*writeChar)(displayPort_t *displayPort, uint8_t x, uint8_t y, uint16_t c, textAttributes_t attr);
    bool (*readChar)(displayPort_t *displayPort, uint8_t x, uint8_t y, uint16_t *c, textAttributes_t *attr);
    bool (*isTransferInProgress)(const displayPort_t *displayPort);
    int (*heartbeat)(displayPort_t *displayPort);
    void (*resync)(displayPort_t *displayPort);
    uint32_t (*txBytesFree)(const displayPort_t *displayPort);
    textAttributes_t (*supportedTextAttributes)(const displayPort_t *displayPort);
    bool (*getFontMetadata)(displayFontMetadata_t *metadata, const displayPort_t *displayPort);
} displayPortVTable_t;

typedef struct displayPortProfile_s {
    int8_t colAdjust;
    int8_t rowAdjust;
    bool invert;
    uint8_t blackBrightness;
    uint8_t whiteBrightness;
} displayPortProfile_t;

void displayGrab(displayPort_t *instance);
void displayRelease(displayPort_t *instance);
void displayReleaseAll(displayPort_t *instance);
bool displayIsGrabbed(const displayPort_t *instance);
void displayClearScreen(displayPort_t *instance);
void displayDrawScreen(displayPort_t *instance);
int displayScreenSize(const displayPort_t *instance);
void displaySetXY(displayPort_t *instance, uint8_t x, uint8_t y);
int displayWrite(displayPort_t *instance, uint8_t x, uint8_t y, const char *s);
int displayWriteWithAttr(displayPort_t *instance, uint8_t x, uint8_t y, const char *s, textAttributes_t attr);
int displayWriteChar(displayPort_t *instance, uint8_t x, uint8_t y, uint16_t c);
int displayWriteCharWithAttr(displayPort_t *instance, uint8_t x, uint8_t y, uint16_t c, textAttributes_t attr);
bool displayReadCharWithAttr(displayPort_t *instance, uint8_t x, uint8_t y, uint16_t *c, textAttributes_t *attr);
bool displayIsTransferInProgress(const displayPort_t *instance);
void displayHeartbeat(displayPort_t *instance);
void displayResync(displayPort_t *instance);
uint16_t displayTxBytesFree(const displayPort_t *instance);
bool displayGetFontMetadata(displayFontMetadata_t *metadata, const displayPort_t *instance);
void displayInit(displayPort_t *instance, const displayPortVTable_t *vTable);
