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
#include <string.h>

#include "platform.h"

#include "common/utils.h"

#include "config/parameter_group_ids.h"

#include "drivers/time.h"

#include "display.h"

#define SW_BLINK_CYCLE_MS 200 // 200ms on / 200ms off
// XXX: This is the number of characters in a MAX7456 line.
// Increment this number appropiately or enable support for
// multiple iterations in displayWriteWithAttr() if bigger
// displays are supported.
#define DISPLAY_MAX_STRING_SIZE 30

PG_REGISTER_WITH_RESET_TEMPLATE(displayConfig_t, displayConfig, PG_DISPLAY_CONFIG, 0);

PG_RESET_TEMPLATE(displayConfig_t, displayConfig,
    .force_sw_blink = false,
);

static bool displayAttributesRequireEmulation(displayPort_t *instance, textAttributes_t attr)
{
    if (attr & ~instance->supportedTextAttributes) {
        // We only emulate blink for now
        return TEXT_ATTRIBUTES_HAVE_BLINK(attr);
    }
    return false;
}

static bool displayEmulateTextAttributes(displayPort_t *instance,
                                        char *buf,
                                        const char *s, size_t length,
                                        textAttributes_t inAttr,
                                        textAttributes_t *outAttr)
{
    UNUSED(instance);
    UNUSED(s);

    // We only emulate blink for now, so there's no need to test
    // for it again.
    TEXT_ATTRIBUTES_COPY(outAttr, &inAttr);
    TEXT_ATTRIBUTES_REMOVE_BLINK(*outAttr);
    if ((millis() / SW_BLINK_CYCLE_MS) % 2) {
        memset(buf, ' ', length);
        buf[length] = '\0';
        // Tell the caller to use buf
        return true;
    }
    // Tell the caller to use s but with outAttr rather than inAttr
    return false;
}

void displayClearScreen(displayPort_t *instance)
{
    instance->vTable->clearScreen(instance);
    instance->cleared = true;
    instance->cursorRow = -1;
}

void displayDrawScreen(displayPort_t *instance)
{
    instance->vTable->drawScreen(instance);
}

int displayScreenSize(const displayPort_t *instance)
{
    return instance->vTable->screenSize(instance);
}

void displayGrab(displayPort_t *instance)
{
    instance->vTable->grab(instance);
    instance->vTable->clearScreen(instance);
    ++instance->grabCount;
}

void displayRelease(displayPort_t *instance)
{
    instance->vTable->release(instance);
    // displayPort_t is changing owner. Clear it, since
    // the new owner might expect a clear canvas.
    instance->vTable->clearScreen(instance);
    --instance->grabCount;
}

void displayReleaseAll(displayPort_t *instance)
{
    instance->vTable->release(instance);
    instance->grabCount = 0;
}

bool displayIsGrabbed(const displayPort_t *instance)
{
    // can be called before initialised
    return (instance && instance->grabCount > 0);
}

void displaySetXY(displayPort_t *instance, uint8_t x, uint8_t y)
{
    instance->posX = x;
    instance->posY = y;
}

int displayWrite(displayPort_t *instance, uint8_t x, uint8_t y, const char *s)
{
    instance->posX = x + strlen(s);
    instance->posY = y;
    return instance->vTable->writeString(instance, x, y, s, TEXT_ATTRIBUTES_NONE);
}

int displayWriteWithAttr(displayPort_t *instance, uint8_t x, uint8_t y, const char *s, textAttributes_t attr)
{
    size_t length = strlen(s);

    instance->posX = x + length;
    instance->posY = y;

    if (displayAttributesRequireEmulation(instance, attr)) {
        // We can't overwrite s, so we use an intermediate buffer if we need
        // text attribute emulation.
        char buf[DISPLAY_MAX_STRING_SIZE + 1];
        textAttributes_t blockAttr;
        size_t blockSize = length > sizeof(buf) ? sizeof(buf) : length;
        if (displayEmulateTextAttributes(instance, buf, s, blockSize, attr, &blockAttr)) {
            // Emulation requires rewriting the string, which might be bigger than our buffer.
            int ret = instance->vTable->writeString(instance, x, y, buf, blockAttr);
#ifdef DISPLAY_ATTRIBUTE_EMULATION_MULTIPLE_WRITES
            if (ret != 0) {
                return ret;
            }
            size_t written = blockSize;
            while (written < length) {
                size_t remaining = length - written;
                blockSize = remaining > sizeof(buf) ? sizeof(buf) : remaining;
                displayEmulateTextAttributes(instance, buf, s + written, blockSize, attr, &blockAttr);
                ret = instance->vTable->writeString(instance, x + written, y, buf, blockAttr);
                if (ret != 0) {
                    return ret;
                }
                written += blockSize;
            }
#endif
            return ret;
        }

        // Emulation at this time requires just changing the attributes, we can use the same string
        TEXT_ATTRIBUTES_COPY(&attr, &blockAttr);
    }

    return instance->vTable->writeString(instance, x, y, s, attr);
}

int displayWriteChar(displayPort_t *instance, uint8_t x, uint8_t y, uint8_t c)
{
    instance->posX = x + 1;
    instance->posY = y;
    return instance->vTable->writeChar(instance, x, y, c, TEXT_ATTRIBUTES_NONE);
}

int displayWriteCharWithAttr(displayPort_t *instance, uint8_t x, uint8_t y, uint8_t c, textAttributes_t attr)
{
    if (displayAttributesRequireEmulation(instance, attr)) {
        displayEmulateTextAttributes(instance, (char *)&c, (char *)&c, 1, attr, &attr);
    }
    instance->posX = x + 1;
    instance->posY = y;
    return instance->vTable->writeChar(instance, x, y, c, attr);
}

bool displayIsTransferInProgress(const displayPort_t *instance)
{
    return instance->vTable->isTransferInProgress(instance);
}

void displayHeartbeat(displayPort_t *instance)
{
    instance->vTable->heartbeat(instance);
}

void displayResync(displayPort_t *instance)
{
    instance->vTable->resync(instance);
}

uint16_t displayTxBytesFree(const displayPort_t *instance)
{
    return instance->vTable->txBytesFree(instance);
}

void displayInit(displayPort_t *instance, const displayPortVTable_t *vTable)
{
    instance->vTable = vTable;
    instance->vTable->clearScreen(instance);
    instance->cleared = true;
    instance->grabCount = 0;
    instance->cursorRow = -1;
    instance->supportedTextAttributes = TEXT_ATTRIBUTES_NONE;
    if (vTable->supportedTextAttributes) {
        instance->supportedTextAttributes = vTable->supportedTextAttributes(instance);
    }
    if (displayConfig()->force_sw_blink) {
        TEXT_ATTRIBUTES_REMOVE_BLINK(instance->supportedTextAttributes);
    }
}

