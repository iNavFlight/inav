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
#include "drivers/display_canvas.h"
#include "drivers/display_font_metadata.h"

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

    frskyOSDDrawCharInGrid(x, y, c, attr);
    return 0;
}

static bool readChar(displayPort_t *displayPort, uint8_t x, uint8_t y, uint16_t *c, textAttributes_t *attr)
{
    UNUSED(displayPort);

    return frskyOSDReadCharInGrid(x, y, c, attr);
}

static bool isTransferInProgress(const displayPort_t *displayPort)
{
    UNUSED(displayPort);
    return false;
}

static void updateGridSize(displayPort_t *displayPort)
{
    displayPort->rows = frskyOSDGetGridRows();
    displayPort->cols = frskyOSDGetGridCols();
}

static void resync(displayPort_t *displayPort)
{
    UNUSED(displayPort);
    // TODO(agh): Do we need to flush the screen here?
    // MAX7456's driver does a full redraw in resync(),
    // so some callers might be expecting that.
    updateGridSize(displayPort);
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

static bool isReady(displayPort_t *instance)
{
    if (frskyOSDIsReady()) {
        updateGridSize(instance);
        return true;
    }
    return false;
}

static void beginTransaction(displayPort_t *instance, displayTransactionOption_e opts)
{
    UNUSED(instance);

    frskyOSDTransactionOptions_e frskyOpts = 0;
    if (opts & DISPLAY_TRANSACTION_OPT_PROFILED) {
        frskyOpts |= FRSKY_OSD_TRANSACTION_OPT_PROFILED;
    }
    if (opts & DISPLAY_TRANSACTION_OPT_RESET_DRAWING) {
        frskyOpts |= FRSKY_OSD_TRANSACTION_OPT_RESET_DRAWING;
    }

    frskyOSDBeginTransaction(frskyOpts);
}

static void commitTransaction(displayPort_t *instance)
{
    UNUSED(instance);

    frskyOSDCommitTransaction();
}

static frskyOSDColor_e frskyOSDGetColor(displayCanvasColor_e color)
{
    switch (color)
    {
        case DISPLAY_CANVAS_COLOR_BLACK:
            return FRSKY_OSD_COLOR_BLACK;
        case DISPLAY_CANVAS_COLOR_TRANSPARENT:
            return FRSKY_OSD_COLOR_TRANSPARENT;
        case DISPLAY_CANVAS_COLOR_WHITE:
            return FRSKY_OSD_COLOR_WHITE;
        case DISPLAY_CANVAS_COLOR_GRAY:
            return FRSKY_OSD_COLOR_GRAY;
    }
    return FRSKY_OSD_COLOR_BLACK;
}

static void setStrokeColor(displayCanvas_t *displayCanvas, displayCanvasColor_e color)
{
    UNUSED(displayCanvas);

    frskyOSDSetStrokeColor(frskyOSDGetColor(color));
}

static void setFillColor(displayCanvas_t *displayCanvas, displayCanvasColor_e color)
{
    UNUSED(displayCanvas);

    frskyOSDSetFillColor(frskyOSDGetColor(color));
}

static void setStrokeAndFillColor(displayCanvas_t *displayCanvas, displayCanvasColor_e color)
{
    UNUSED(displayCanvas);

    frskyOSDSetStrokeAndFillColor(frskyOSDGetColor(color));
}

static void setColorInversion(displayCanvas_t *displayCanvas, bool inverted)
{
    UNUSED(displayCanvas);

    frskyOSDSetColorInversion(inverted);
}

static void setPixel(displayCanvas_t *displayCanvas, int x, int y, displayCanvasColor_e color)
{
    UNUSED(displayCanvas);

    frskyOSDSetPixel(x, y, frskyOSDGetColor(color));
}

static void setPixelToStrokeColor(displayCanvas_t *displayCanvas, int x, int y)
{
    UNUSED(displayCanvas);

    frskyOSDSetPixelToStrokeColor(x, y);
}

static void setPixelToFillColor(displayCanvas_t *displayCanvas, int x, int y)
{
    UNUSED(displayCanvas);

    frskyOSDSetPixelToFillColor(x, y);
}

static void setStrokeWidth(displayCanvas_t *displayCanvas, unsigned w)
{
    UNUSED(displayCanvas);

    frskyOSDSetStrokeWidth(w);
}

static void setLineOutlineType(displayCanvas_t *displayCanvas, displayCanvasOutlineType_e outlineType)
{
    UNUSED(displayCanvas);

    frskyOSDSetLineOutlineType(outlineType);
}

static void setLineOutlineColor(displayCanvas_t *displayCanvas, displayCanvasColor_e outlineColor)
{
    UNUSED(displayCanvas);

    frskyOSDSetLineOutlineColor(outlineColor);
}

static void clipToRect(displayCanvas_t *displayCanvas, int x, int y, int w, int h)
{
    UNUSED(displayCanvas);

    frskyOSDClipToRect(x, y, w, h);
}

static void clearRect(displayCanvas_t *displayCanvas, int x, int y, int w, int h)
{
    UNUSED(displayCanvas);

    frskyOSDClearRect(x, y, w, h);
}

static void resetDrawingState(displayCanvas_t *displayCanvas)
{
    UNUSED(displayCanvas);

    frskyOSDResetDrawingState();
}

static void drawCharacter(displayCanvas_t *displayCanvas, int x, int y, uint16_t chr, displayCanvasBitmapOption_t opts)
{
    UNUSED(displayCanvas);

    frskyOSDDrawCharacter(x, y, chr, opts);
}

static void drawCharacterMask(displayCanvas_t *displayCanvas, int x, int y, uint16_t chr, displayCanvasColor_e color, displayCanvasBitmapOption_t opts)
{
    UNUSED(displayCanvas);

    frskyOSDDrawCharacterMask(x, y, chr, frskyOSDGetColor(color), opts);
}

static void drawString(displayCanvas_t *displayCanvas, int x, int y, const char *s, displayCanvasBitmapOption_t opts)
{
    UNUSED(displayCanvas);

    frskyOSDDrawString(x, y, s, opts);
}

static void drawStringMask(displayCanvas_t *displayCanvas, int x, int y, const char *s, displayCanvasColor_e color, displayCanvasBitmapOption_t opts)
{
    UNUSED(displayCanvas);

    frskyOSDDrawStringMask(x, y, s, frskyOSDGetColor(color), opts);
}

static void moveToPoint(displayCanvas_t *displayCanvas, int x, int y)
{
    UNUSED(displayCanvas);

    frskyOSDMoveToPoint(x, y);
}

static void strokeLineToPoint(displayCanvas_t *displayCanvas, int x, int y)
{
    UNUSED(displayCanvas);

    frskyOSDStrokeLineToPoint(x, y);
}

static void strokeTriangle(displayCanvas_t *displayCanvas, int x1, int y1, int x2, int y2, int x3, int y3)
{
    UNUSED(displayCanvas);

    frskyOSDStrokeTriangle(x1, y1, x2, y2, x3, y3);
}

static void fillTriangle(displayCanvas_t *displayCanvas, int x1, int y1, int x2, int y2, int x3, int y3)
{
    UNUSED(displayCanvas);

    frskyOSDFillTriangle(x1, y1, x2, y2, x3, y3);
}

static void fillStrokeTriangle(displayCanvas_t *displayCanvas, int x1, int y1, int x2, int y2, int x3, int y3)
{
    UNUSED(displayCanvas);

    frskyOSDFillStrokeTriangle(x1, y1, x2, y2, x3, y3);
}

static void strokeRect(displayCanvas_t *displayCanvas, int x, int y, int w, int h)
{
    UNUSED(displayCanvas);

    frskyOSDStrokeRect(x, y, w, h);
}

static void fillRect(displayCanvas_t *displayCanvas, int x, int y, int w, int h)
{
    UNUSED(displayCanvas);

    frskyOSDFillRect(x, y, w, h);
}

static void fillStrokeRect(displayCanvas_t *displayCanvas, int x, int y, int w, int h)
{
    UNUSED(displayCanvas);

    frskyOSDFillStrokeRect(x, y, w, h);
}

static void strokeEllipseInRect(displayCanvas_t *displayCanvas, int x, int y, int w, int h)
{
    UNUSED(displayCanvas);

    frskyOSDStrokeEllipseInRect(x, y, w, h);
}

static void fillEllipseInRect(displayCanvas_t *displayCanvas, int x, int y, int w, int h)
{
    UNUSED(displayCanvas);

    frskyOSDFillEllipseInRect(x, y, w, h);
}

static void fillStrokeEllipseInRect(displayCanvas_t *displayCanvas, int x, int y, int w, int h)
{
    UNUSED(displayCanvas);

    frskyOSDFillStrokeEllipseInRect(x, y, w, h);
}

static void ctmReset(displayCanvas_t *displayCanvas)
{
    UNUSED(displayCanvas);

    frskyOSDCtmReset();
}

static void ctmSet(displayCanvas_t *displayCanvas, float m11, float m12, float m21, float m22, float m31, float m32)
{
    UNUSED(displayCanvas);

    frskyOSDCtmSet(m11, m12, m21, m22, m31, m32);
}

static void ctmTranslate(displayCanvas_t *displayCanvas, float tx, float ty)
{
    UNUSED(displayCanvas);

    frskyOSDCtmTranslate(tx, ty);
}

static void ctmScale(displayCanvas_t *displayCanvas, float sx, float sy)
{
    UNUSED(displayCanvas);

    frskyOSDCtmScale(sx, sy);
}

static void ctmRotate(displayCanvas_t *displayCanvas, float r)
{
    UNUSED(displayCanvas);

    frskyOSDCtmRotate(r);
}

static void contextPush(displayCanvas_t *displayCanvas)
{
    UNUSED(displayCanvas);

    frskyOSDContextPush();
}

static void contextPop(displayCanvas_t *displayCanvas)
{
    UNUSED(displayCanvas);

    frskyOSDContextPop();
}


static const displayCanvasVTable_t frskyOSDCanvasVTable = {
    .setStrokeColor = setStrokeColor,
    .setFillColor = setFillColor,
    .setStrokeAndFillColor = setStrokeAndFillColor,
    .setColorInversion = setColorInversion,
    .setPixel = setPixel,
    .setPixelToStrokeColor = setPixelToStrokeColor,
    .setPixelToFillColor = setPixelToFillColor,
    .setStrokeWidth = setStrokeWidth,
    .setLineOutlineType = setLineOutlineType,
    .setLineOutlineColor = setLineOutlineColor,

    .clipToRect = clipToRect,
    .clearRect = clearRect,
    .resetDrawingState = resetDrawingState,
    .drawCharacter = drawCharacter,
    .drawCharacterMask = drawCharacterMask,
    .drawString = drawString,
    .drawStringMask = drawStringMask,
    .moveToPoint = moveToPoint,
    .strokeLineToPoint = strokeLineToPoint,
    .strokeTriangle = strokeTriangle,
    .fillTriangle = fillTriangle,
    .fillStrokeTriangle = fillStrokeTriangle,
    .strokeRect = strokeRect,
    .fillRect = fillRect,
    .fillStrokeRect = fillStrokeRect,
    .strokeEllipseInRect = strokeEllipseInRect,
    .fillEllipseInRect = fillEllipseInRect,
    .fillStrokeEllipseInRect = fillStrokeEllipseInRect,

    .ctmReset = ctmReset,
    .ctmSet = ctmSet,
    .ctmTranslate = ctmTranslate,
    .ctmScale = ctmScale,
    .ctmRotate = ctmRotate,

    .contextPush = contextPush,
    .contextPop = contextPop,
};

static bool getCanvas(displayCanvas_t *canvas, const displayPort_t *instance)
{
    UNUSED(instance);

    canvas->vTable = &frskyOSDCanvasVTable;
    canvas->width = frskyOSDGetPixelWidth();
    canvas->height = frskyOSDGetPixelHeight();
    return true;
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
    .beginTransaction = beginTransaction,
    .commitTransaction = commitTransaction,
    .getCanvas = getCanvas,
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
