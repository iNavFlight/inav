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

#if defined(USE_AGHOSD)

#include "common/utils.h"

#include "drivers/display.h"
#include "drivers/display_font_metadata.h"

#include "io/agh_osd.h"
#include "io/displayport_agh_osd.h"

static displayPort_t aghOSDDisplayPort;

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
    aghOSDClearScreen();
    return 0;
}

static int drawScreen(displayPort_t *displayPort)
{
    UNUSED(displayPort);
    aghOSDUpdate();

    return 0;
}

static int screenSize(const displayPort_t *displayPort)
{
    UNUSED(displayPort);
    return aghOSDGetGridRows() * aghOSDGetGridCols();
}

static int writeString(displayPort_t *displayPort, uint8_t x, uint8_t y, const char *s, textAttributes_t attr)
{
    UNUSED(displayPort);

    aghOSDDrawStringInGrid(x, y, s, attr);
    return 0;
}

static int writeChar(displayPort_t *displayPort, uint8_t x, uint8_t y, uint16_t c, textAttributes_t attr)
{
    UNUSED(displayPort);

    aghOSDDrawCharInGrid(x, y, c, attr);
    return 0;
}

static bool readChar(displayPort_t *displayPort, uint8_t x, uint8_t y, uint16_t *c, textAttributes_t *attr)
{
    UNUSED(displayPort);
    return false;
}

static bool isTransferInProgress(const displayPort_t *displayPort)
{
    UNUSED(displayPort);
    return false;
}

static void resync(displayPort_t *displayPort)
{
    UNUSED(displayPort);
    // TODO: Refresh here?
    displayPort->rows = aghOSDGetGridRows();
    displayPort->cols = aghOSDGetGridCols();
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
    return aghOSDReadFontCharacter(FONT_METADATA_CHR_INDEX, &chr) &&
        displayFontMetadataUpdateFromCharacter(metadata, &chr);

}

static int writeFontCharacter(displayPort_t *instance, uint16_t addr, const osdCharacter_t *chr)
{
    UNUSED(instance);

    aghOSDWriteFontCharacter(addr, chr);
    return 0;
}

static bool isReady(const displayPort_t *instance)
{
    UNUSED(instance);

    return aghOSDIsReady();
}

static const displayPortVTable_t aghOSDVTable = {
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

displayPort_t *aghOSDDisplayPortInit(const videoSystem_e videoSystem)
{
    if (aghOSDInit(videoSystem)) {
        displayInit(&aghOSDDisplayPort, &aghOSDVTable);
        resync(&aghOSDDisplayPort);
        return &aghOSDDisplayPort;
    }
    return NULL;
}

#endif // USE_AGHOSD
