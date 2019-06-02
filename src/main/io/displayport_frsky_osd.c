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

#if defined(USE_FRSKYOSD)

#include "common/utils.h"

#include "drivers/display.h"
#include "drivers/display_font_metadata.h"
#include "drivers/max7456.h"

#include "io/displayport_frsky_osd.h"
#include "io/frsky_osd.h"

static displayPort_t frskyOSDDisplayPort;

static int grab(displayPort_t *displayPort)
{
    UNUSED(displayPort);
    return 0;
}

static int release(displayPort_t *displayPort)
{
    UNUSED(displayPort);
    return 0;
}

static int clearScreen(displayPort_t *displayPort)
{
    UNUSED(displayPort);
    frskyOSDClearScreen();
    return 0;
}

static int drawScreen(displayPort_t *displayPort)
{
    UNUSED(displayPort);
    frskyOSDUpdate();

    return 0;
}

static int screenSize(const displayPort_t *displayPort)
{
    UNUSED(displayPort);
    return frskyOSDGetGridRows() * frskyOSDGetGridCols();
}

static int writeString(displayPort_t *displayPort, uint8_t x, uint8_t y, const char *s, textAttributes_t attr)
{
    UNUSED(displayPort);

    frskyOSDDrawStringInGrid(x, y, s, attr);
    return 0;
}

static int writeChar(displayPort_t *displayPort, uint8_t x, uint8_t y, uint16_t c, textAttributes_t attr)
{
    UNUSED(displayPort);
    unsigned pos = y * MAX7456_CHARS_PER_LINE + x;
    max7456ScreenBuffer[pos] = c;
    frskyOSDDrawCharInGrid(x, y, c, attr);
    return 0;
}

static bool readChar(displayPort_t *displayPort, uint8_t x, uint8_t y, uint16_t *c, textAttributes_t *attr)
{
    UNUSED(displayPort);
    UNUSED(attr);
    // XXX: We don't actually read the attributes, just the character
    unsigned pos = y * MAX7456_CHARS_PER_LINE + x;
    *c = max7456ScreenBuffer[pos];
    return true;
}

static bool isTransferInProgress(const displayPort_t *displayPort)
{
    UNUSED(displayPort);
    return false;
}

static void resync(displayPort_t *displayPort)
{
    UNUSED(displayPort);
    // TODO(agh): Do we need to flush the screen here?
    // MAX7456's driver does a full redraw in resync(),
    // so some callers might be expecting that.
    displayPort->rows = frskyOSDGetGridRows();
    displayPort->cols = frskyOSDGetGridCols();
}

static int heartbeat(displayPort_t *displayPort)
{
    UNUSED(displayPort);
    return 0;
}

static uint32_t txBytesFree(const displayPort_t *displayPort)
{
    UNUSED(displayPort);
    return UINT32_MAX;
}

static textAttributes_t supportedTextAttributes(const displayPort_t *displayPort)
{
    UNUSED(displayPort);

    textAttributes_t attr = TEXT_ATTRIBUTES_NONE;
    TEXT_ATTRIBUTES_ADD_INVERTED(attr);
    TEXT_ATTRIBUTES_ADD_SOLID_BG(attr);
    // TEXT_ATTRIBUTES_ADD_BLINK(attr); TODO?
    return attr;
}

static bool getFontMetadata(displayFontMetadata_t *metadata, const displayPort_t *displayPort)
{
    UNUSED(displayPort);

    osdCharacter_t chr;

    metadata->charCount = 512;
    return frskyOSDReadFontCharacter(FONT_METADATA_CHR_INDEX, &chr) &&
        displayFontMetadataUpdateFromCharacter(metadata, &chr);

}

static int writeFontCharacter(displayPort_t *instance, uint16_t addr, const osdCharacter_t *chr)
{
    UNUSED(instance);

    frskyOSDWriteFontCharacter(addr, chr);
    return 0;
}

static bool isReady(const displayPort_t *instance)
{
    UNUSED(instance);

    return frskyOSDIsReady();
}

static const displayPortVTable_t frskyOSDVTable = {
    .grab = grab,
    .release = release,
    .clearScreen = clearScreen,
    .drawScreen = drawScreen,
    .screenSize = screenSize,
    .writeString = writeString,
    .writeChar = writeChar,
    .readChar = readChar,
    .isTransferInProgress = isTransferInProgress,
    .heartbeat = heartbeat,
    .resync = resync,
    .txBytesFree = txBytesFree,
    .supportedTextAttributes = supportedTextAttributes,
    .getFontMetadata = getFontMetadata,
    .writeFontCharacter = writeFontCharacter,
    .isReady = isReady,
};

displayPort_t *frskyOSDDisplayPortInit(const videoSystem_e videoSystem)
{
    if (frskyOSDInit(videoSystem)) {
        displayInit(&frskyOSDDisplayPort, &frskyOSDVTable);
        resync(&frskyOSDDisplayPort);
        return &frskyOSDDisplayPort;
    }
    return NULL;
}

#endif // USE_FRSKYOSD
