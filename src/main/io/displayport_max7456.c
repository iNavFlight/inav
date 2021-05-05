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

#ifdef USE_MAX7456

#include "common/utils.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/display.h"
#include "drivers/display_font_metadata.h"
#include "drivers/max7456.h"

#include "io/displayport_max7456.h"

displayPort_t max7456DisplayPort;

static uint8_t max7456Mode(textAttributes_t attr)
{
    uint8_t mode = 0;
    if (TEXT_ATTRIBUTES_HAVE_BLINK(attr)) {
        mode |= MAX7456_MODE_BLINK;
    }
    if (TEXT_ATTRIBUTES_HAVE_INVERTED(attr)) {
        mode |= MAX7456_MODE_INVERT;
    }
    if (TEXT_ATTRIBUTES_HAVE_SOLID_BG(attr)) {
        mode |= MAX7456_MODE_SOLID_BG;
    }
    return mode;
}

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
    max7456ClearScreen();
    return 0;
}

static int drawScreen(displayPort_t *displayPort)
{
    UNUSED(displayPort);
    max7456Update();

    return 0;
}

static int screenSize(const displayPort_t *displayPort)
{
    UNUSED(displayPort);
    return max7456GetScreenSize();
}

static int writeString(displayPort_t *displayPort, uint8_t x, uint8_t y, const char *s, textAttributes_t attr)
{
    UNUSED(displayPort);
    max7456Write(x, y, s, max7456Mode(attr));

    return 0;
}

static int writeChar(displayPort_t *displayPort, uint8_t x, uint8_t y, uint16_t c, textAttributes_t attr)
{
    UNUSED(displayPort);
    max7456WriteChar(x, y, c, max7456Mode(attr));

    return 0;
}

static bool readChar(displayPort_t *displayPort, uint8_t x, uint8_t y, uint16_t *c, textAttributes_t *attr)
{
    UNUSED(displayPort);
    return max7456ReadChar(x, y, c, attr);
}

static bool isTransferInProgress(const displayPort_t *displayPort)
{
    UNUSED(displayPort);
    return false;
}

static void resync(displayPort_t *displayPort)
{
    UNUSED(displayPort);
    max7456RefreshAll();
    displayPort->rows = max7456GetRowsCount();
    displayPort->cols = 30;
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
    TEXT_ATTRIBUTES_ADD_BLINK(attr);
    return attr;
}

static bool getFontMetadata(displayFontMetadata_t *metadata, const displayPort_t *displayPort)
{
    UNUSED(displayPort);

    osdCharacter_t chr;

    max7456ReadNvm(FONT_METADATA_CHR_INDEX, &chr);

    if (displayFontMetadataUpdateFromCharacter(metadata, &chr)) {
        // Not all MAX7456 chips support 512 characters. To detect this,
        // we place metadata in both characters 255 and 256. This way we
        // can find out how many characters the font in NVM has.
        max7456ReadNvm(FONT_METADATA_CHR_INDEX_2ND_PAGE, &chr);
        metadata->charCount = FONT_CHR_IS_METADATA(&chr) ? 512 : 256;
        return true;
    }

    return false;
}

static int writeFontCharacter(displayPort_t *instance, uint16_t addr, const osdCharacter_t *chr)
{
    UNUSED(instance);

    max7456WriteNvm(addr, chr);
    return 0;
}

static const displayPortVTable_t max7456VTable = {
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
};

displayPort_t *max7456DisplayPortInit(const videoSystem_e videoSystem)
{
    max7456Init(videoSystem);
    displayInit(&max7456DisplayPort, &max7456VTable);
    resync(&max7456DisplayPort);
    return &max7456DisplayPort;
}
#endif // USE_MAX7456
