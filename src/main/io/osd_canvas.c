/*
 * This file is part of INAV.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 * @author Alberto Garcia Hierro <alberto@garciahierro.com>
 */

#include <math.h>

#include "platform.h"

#if defined(USE_CANVAS)

#include "common/log.h"
#include "common/maths.h"
#include "common/printf.h"
#include "common/utils.h"

#include "drivers/display.h"
#include "drivers/display_canvas.h"
#include "drivers/osd_symbols.h"

#include "io/osd_common.h"

void osdCanvasDrawVarioShape(displayCanvas_t *canvas, unsigned ex, unsigned ey, float zvel, bool erase)
{
    char sym;
    float ratio = zvel / (OSD_VARIO_CM_S_PER_ARROW * 2);
    int height = -ratio * canvas->gridElementHeight;
    int step;
    int x = ex * canvas->gridElementWidth;
    int start;
    int dstart;
    if (zvel > 0) {
        sym = SYM_VARIO_UP_2A;
        start = ceilf(OSD_VARIO_HEIGHT_ROWS / 2.0f);
        dstart = start - 1;
        step = -canvas->gridElementHeight;
    } else {
        sym = SYM_VARIO_DOWN_2A;
        start = floorf(OSD_VARIO_HEIGHT_ROWS / 2.0f);
        dstart = start;
        step = canvas->gridElementHeight;
    }
    int y = (start + ey) * canvas->gridElementHeight;
    displayCanvasClipToRect(canvas, x, y, canvas->gridElementWidth, height);
    int dy = (dstart + ey) * canvas->gridElementHeight;
    for (int ii = 0, yy = dy; ii < (OSD_VARIO_HEIGHT_ROWS + 1) / 2; ii++, yy += step) {
        if (erase) {
            displayCanvasDrawCharacterMask(canvas, x, yy, sym, DISPLAY_CANVAS_COLOR_TRANSPARENT, 0);
        } else {
            displayCanvasDrawCharacter(canvas, x, yy, sym, 0);
        }
    }
}

void osdCanvasDrawVario(displayPort_t *display, displayCanvas_t *canvas, const osdDrawPoint_t *p, float zvel)
{
    UNUSED(display);

    static float prev = 0;

    if (fabsf(prev - zvel) < (OSD_VARIO_CM_S_PER_ARROW / 20.0f)) {
        return;
    }

    uint8_t ex;
    uint8_t ey;

    osdDrawPointGetGrid(&ex, &ey, display, canvas, p);

    osdCanvasDrawVarioShape(canvas, ex, ey, prev, true);
    osdCanvasDrawVarioShape(canvas, ex, ey, zvel, false);
    prev = zvel;
}

void osdCanvasDrawDirArrow(displayPort_t *display, displayCanvas_t *canvas, const osdDrawPoint_t *p, float degrees, bool eraseBefore)
{
    UNUSED(display);

    int px;
    int py;
    osdDrawPointGetPixels(&px, &py, display, canvas, p);

    displayCanvasClipToRect(canvas, px, py, canvas->gridElementWidth, canvas->gridElementHeight);

    if (eraseBefore) {
        displayCanvasSetFillColor(canvas, DISPLAY_CANVAS_COLOR_TRANSPARENT);
        displayCanvasFillRect(canvas, px, py, canvas->gridElementWidth, canvas->gridElementHeight);
    }

    displayCanvasSetFillColor(canvas, DISPLAY_CANVAS_COLOR_WHITE);
    displayCanvasSetStrokeColor(canvas, DISPLAY_CANVAS_COLOR_BLACK);

    displayCanvasCtmRotate(canvas, -DEGREES_TO_RADIANS(degrees));
    displayCanvasCtmTranslate(canvas, px + canvas->gridElementWidth / 2, py + canvas->gridElementHeight / 2);
    displayCanvasFillStrokeTriangle(canvas, 0, 6, 5, -6, -5, -6);
    displayCanvasSetFillColor(canvas, DISPLAY_CANVAS_COLOR_TRANSPARENT);
    displayCanvasFillStrokeTriangle(canvas, 0, -2, 6, -7, -6, -7);
    displayCanvasMoveToPoint(canvas, 6, -7);
    displayCanvasSetStrokeColor(canvas, DISPLAY_CANVAS_COLOR_TRANSPARENT);
    displayCanvasStrokeLineToPoint(canvas, -6, -7);
}

static void osdDrawArtificialHorizonLevelLine(displayCanvas_t *canvas, int width, int pos, int margin, bool erase)
{
    // Vertical strokes
    displayCanvasFillStrokeRect(canvas, -width / 2, -pos - 1, 3, -10);
    displayCanvasFillStrokeRect(canvas, width / 2, -pos - 1, 3, -10);
    // Horizontal strokes
    displayCanvasFillStrokeRect(canvas, -width / 2 + 1, -pos - 1, width / 2 - margin, 3);
    displayCanvasFillStrokeRect(canvas, width / 2 + 1, -pos - 1, -width / 2 + margin, 3);

    if (!erase) {
        // When we're not erasing the old AHI position, we need to clean up so black
        // strokes left by the horizontal strokes.
        displayCanvasSetStrokeColor(canvas, DISPLAY_CANVAS_COLOR_WHITE);

        displayCanvasMoveToPoint(canvas, -width / 2, -pos - 1);
        displayCanvasStrokeLineToPoint(canvas, -width / 2 + 3, -pos - 1);

        displayCanvasMoveToPoint(canvas, width / 2, -pos - 1);
        displayCanvasStrokeLineToPoint(canvas, width / 2 - 3, -pos - 1);

        displayCanvasSetStrokeColor(canvas, DISPLAY_CANVAS_COLOR_BLACK);
    }
}

static void osdDrawArtificialHorizonShapes(displayCanvas_t *canvas, float pitchAngle, float rollAngle, bool erase)
{
    int barWidth = (OSD_AHI_WIDTH - 1) * canvas->gridElementWidth;
    int levelBarWidth = barWidth * (3.0/4);
    int crosshairMargin = 6;
    float pixelsPerDegreeLevel = 3.5f;
    int maxWidth = (OSD_AHI_WIDTH + 1) * canvas->gridElementWidth;
    int maxHeight = OSD_AHI_HEIGHT * canvas->gridElementHeight;
    int borderSize = 3;
    char buf[12];

    displayCanvasContextPush(canvas);

    int lx = (canvas->width - maxWidth) / 2;
    int ty = (canvas->height - maxHeight) / 2;

    if (!erase) {
        int rx = lx + maxWidth;
        int by = ty + maxHeight;

        displayCanvasSetStrokeColor(canvas, DISPLAY_CANVAS_COLOR_BLACK);

        displayCanvasMoveToPoint(canvas, lx, ty + borderSize);
        displayCanvasStrokeLineToPoint(canvas, lx, ty);
        displayCanvasStrokeLineToPoint(canvas, lx + borderSize, ty);

        displayCanvasMoveToPoint(canvas, rx, ty + borderSize);
        displayCanvasStrokeLineToPoint(canvas, rx, ty);
        displayCanvasStrokeLineToPoint(canvas, rx - borderSize, ty);

        displayCanvasMoveToPoint(canvas,lx, by - borderSize);
        displayCanvasStrokeLineToPoint(canvas, lx, by);
        displayCanvasStrokeLineToPoint(canvas, lx + borderSize, by);

        displayCanvasMoveToPoint(canvas, rx, by - borderSize);
        displayCanvasStrokeLineToPoint(canvas, rx, by);
        displayCanvasStrokeLineToPoint(canvas, rx - borderSize, by);
    }

    displayCanvasClipToRect(canvas, lx + 1, ty + 1, maxWidth - 2, maxHeight - 2);

    if (erase) {
        displayCanvasSetFillColor(canvas, DISPLAY_CANVAS_COLOR_TRANSPARENT);
        displayCanvasSetStrokeColor(canvas, DISPLAY_CANVAS_COLOR_TRANSPARENT);
    } else {
        displayCanvasSetFillColor(canvas, DISPLAY_CANVAS_COLOR_WHITE);
        displayCanvasSetStrokeColor(canvas, DISPLAY_CANVAS_COLOR_BLACK);
    }

    // The draw just the 5 bars closest to the current pitch level
    float pitchDegrees = RADIANS_TO_DEGREES(pitchAngle);
    float pitchCenter = roundf(pitchDegrees / 10.0f);
    float pitchOffset = -pitchDegrees * pixelsPerDegreeLevel;
    float translateX = canvas->width / 2;
    float translateY = canvas->height / 2;

    displayCanvasCtmTranslate(canvas, 0, pitchOffset);
    displayCanvasContextPush(canvas);
    displayCanvasCtmRotate(canvas, -rollAngle);

    displayCanvasCtmTranslate(canvas, translateX, translateY);

    for (int ii = pitchCenter - 2; ii <= pitchCenter + 2; ii++) {
        if (ii == 0) {
            displayCanvasFillStrokeRect(canvas, -barWidth / 2, -1, barWidth / 2 - crosshairMargin, 3);
            displayCanvasFillStrokeRect(canvas, barWidth / 2, -1, -barWidth / 2 + crosshairMargin, 3);
            continue;
        }

        int pos = ii * 10 * pixelsPerDegreeLevel;
        int margin = (ii > 9 || ii < -9) ? 9 : 6;
        if (pos > 0) {
            osdDrawArtificialHorizonLevelLine(canvas, levelBarWidth, -pos, margin, erase);
        } else {
            displayCanvasContextPush(canvas);
            displayCanvasCtmScale(canvas, 1, -1);
            osdDrawArtificialHorizonLevelLine(canvas, levelBarWidth, pos, margin, erase);
            displayCanvasContextPop(canvas);
        }
    }

    displayCanvasContextPop(canvas);

#if 0



    // The draw just the 5 bars closest to the current pitch level
    float pitchDegrees = RADIANS_TO_DEGREES(pitchAngle);
    float pitchCenter = roundf(pitchDegrees / 10.0f);
    float pitchOffset = -pitchDegrees * pixelsDegreeLevel;
    float translateX = osdCanvas.widthPixels / 2;
    float translateY = osdCanvas.heightPixels / 2;

    displayCanvasCtmTranslate(canvas, 0, pitchOffset);
    displayCanvasContextPush(canvas);
    displayCanvasCtmRotate(canvas, -rollAngle);
    displayCanvasCtmTranslate(canvas, translateX, translateY);

    for (int ii = pitchCenter - 2; ii <= pitchCenter + 2; ii++) {
        if (ii == 0) {
            displayCanvasFillStrokeRect(canvas, -width / 2, -1, width / 2 - crosshairMargin, 3);
            displayCanvasFillStrokeRect(canvas, width / 2, -1, -width / 2 + crosshairMargin, 3);
            continue;
        }

        int pos = ii * 10 * pixelsDegreeLevel;
        int margin = 6;
        if (pos > 0) {
            osdDrawArtificialHorizonLevelLine(levelWidth, -pos, margin, erase);
        } else {
            displayCanvasContextPush(canvas);
            displayCanvasCtmScale(canvas, 1, -1);
            osdDrawArtificialHorizonLevelLine(levelWidth, pos, margin, erase);
            displayCanvasContextPop(canvas);
        }
    }

    displayCanvasContextPop(canvas);
#endif
    displayCanvasCtmTranslate(canvas, translateX, translateY);
    displayCanvasCtmScale(canvas, 0.5f, 0.5f);

    // Draw line labels
    float sx = sin_approx(-rollAngle);
    float sy = cos_approx(rollAngle);
    for (int ii = pitchCenter - 2; ii <= pitchCenter + 2; ii++) {
        if (ii == 0) {
            continue;
        }

        int level = ii * 10;
        int absLevel = ABS(level);
        tfp_snprintf(buf, sizeof(buf), "%d", absLevel);
        int pos = level * pixelsPerDegreeLevel;
        int charY = 9 - pos * 2;
        int cx = (absLevel >= 100 ? -1.5f : -1.0) * canvas->gridElementWidth;
        int px = cx + (pitchOffset + pos) * sx * 2;
        int py = -charY - (pitchOffset + pos) * (1 - sy) * 2;
        if (erase) {
            displayCanvasDrawStringMask(canvas, px, py, buf, DISPLAY_CANVAS_COLOR_TRANSPARENT, 0);
        } else {
            displayCanvasDrawString(canvas, px, py, buf, 0);
        }
    }
    displayCanvasContextPop(canvas);
}

void osdCanvasDrawArtificialHorizon(displayPort_t *display, displayCanvas_t *canvas, const osdDrawPoint_t *p, float pitchAngle, float rollAngle)
{
    UNUSED(display);
    UNUSED(p);

    static float prevPitchAngle = 9999;
    static float prevRollAngle = 9999;

    if (fabsf(prevPitchAngle - pitchAngle) > 0.01f || fabsf(prevRollAngle - rollAngle) > 0.01f) {
        osdDrawArtificialHorizonShapes(canvas, prevPitchAngle, prevRollAngle, true);
        osdDrawArtificialHorizonShapes(canvas, pitchAngle, rollAngle, false);
        prevPitchAngle = pitchAngle;
        prevRollAngle = rollAngle;
    }
}

#endif
