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

#define AHI_MIN_DRAW_INTERVAL_MS 100
#define AHI_MAX_DRAW_INTERVAL_MS 1000

#include "common/log.h"
#include "common/maths.h"
#include "common/typeconversion.h"
#include "common/utils.h"

#include "drivers/display.h"
#include "drivers/display_canvas.h"
#include "drivers/osd.h"
#include "drivers/osd_symbols.h"
#include "drivers/time.h"

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

    displayCanvasCtmRotate(canvas, -DEGREES_TO_RADIANS(180 + degrees));
    displayCanvasCtmTranslate(canvas, px + canvas->gridElementWidth / 2, py + canvas->gridElementHeight / 2);
    displayCanvasFillStrokeTriangle(canvas, 0, 6, 5, -6, -5, -6);
    displayCanvasSetFillColor(canvas, DISPLAY_CANVAS_COLOR_TRANSPARENT);
    displayCanvasFillStrokeTriangle(canvas, 0, -2, 6, -7, -6, -7);
    displayCanvasMoveToPoint(canvas, 6, -7);
    displayCanvasSetStrokeColor(canvas, DISPLAY_CANVAS_COLOR_TRANSPARENT);
    displayCanvasStrokeLineToPoint(canvas, -6, -7);
}

static void osdDrawArtificialHorizonLevelLine(displayCanvas_t *canvas, int width, int pos, int margin)
{
    displayCanvasSetLineOutlineType(canvas, DISPLAY_CANVAS_OUTLINE_TYPE_BOTTOM);

    int yoff = pos >= 0 ? 10 : -10;
    int yc = -pos - 1;
    int sz = width / 2;

    // Horizontal strokes
    displayCanvasMoveToPoint(canvas, -sz, yc);
    displayCanvasStrokeLineToPoint(canvas, -margin, yc);
    displayCanvasMoveToPoint(canvas, sz, yc);
    displayCanvasStrokeLineToPoint(canvas, margin, yc);

    // Vertical strokes
    displayCanvasSetLineOutlineType(canvas, DISPLAY_CANVAS_OUTLINE_TYPE_LEFT);
    displayCanvasMoveToPoint(canvas, -sz, yc);
    displayCanvasStrokeLineToPoint(canvas, -sz, yc + yoff);
    displayCanvasSetLineOutlineType(canvas, DISPLAY_CANVAS_OUTLINE_TYPE_RIGHT);
    displayCanvasMoveToPoint(canvas, sz, yc);
    displayCanvasStrokeLineToPoint(canvas, sz, yc + yoff);
}

static void osdArtificialHorizonRect(displayCanvas_t *canvas, int *lx, int *ty, int *w, int *h)
{
    *w = (OSD_AHI_WIDTH + 1) * canvas->gridElementWidth;
    *h = OSD_AHI_HEIGHT * canvas->gridElementHeight;

    *lx = (canvas->width - *w) / 2;
    *ty = (canvas->height - *h) / 2;
}

static void osdDrawArtificialHorizonShapes(displayCanvas_t *canvas, float pitchAngle, float rollAngle)
{
    int barWidth = (OSD_AHI_WIDTH - 1) * canvas->gridElementWidth;
    int levelBarWidth = barWidth * (3.0/4);
    int crosshairMargin = 6;
    float pixelsPerDegreeLevel = 3.5f;
    int borderSize = 3;
    char buf[12];

    int lx;
    int ty;
    int maxWidth;
    int maxHeight;

    osdArtificialHorizonRect(canvas, &lx, &ty, &maxWidth, &maxHeight);

    displayCanvasContextPush(canvas);

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

    displayCanvasClipToRect(canvas, lx + 1, ty + 1, maxWidth - 2, maxHeight - 2);
    osdGridBufferClearPixelRect(canvas, lx, ty, maxWidth, maxHeight);

    displayCanvasSetStrokeColor(canvas, DISPLAY_CANVAS_COLOR_WHITE);
    displayCanvasSetLineOutlineColor(canvas, DISPLAY_CANVAS_COLOR_BLACK);

    // The draw just the 5 bars closest to the current pitch level
    float pitchDegrees = RADIANS_TO_DEGREES(pitchAngle);
    float pitchCenter = roundf(pitchDegrees / 10.0f);
    float pitchOffset = -pitchDegrees * pixelsPerDegreeLevel;
    float translateX = canvas->width / 2;
    float translateY = canvas->height / 2;

    displayCanvasCtmTranslate(canvas, 0, pitchOffset);
    displayCanvasContextPush(canvas);
    displayCanvasCtmRotate(canvas, rollAngle);

    displayCanvasCtmTranslate(canvas, translateX, translateY);

    for (int ii = pitchCenter - 2; ii <= pitchCenter + 2; ii++) {
        if (ii == 0) {
            displayCanvasSetLineOutlineType(canvas, DISPLAY_CANVAS_OUTLINE_TYPE_BOTTOM);
            displayCanvasMoveToPoint(canvas, -barWidth / 2, 0);
            displayCanvasStrokeLineToPoint(canvas, -crosshairMargin, 0);
            displayCanvasMoveToPoint(canvas, barWidth / 2, 0);
            displayCanvasStrokeLineToPoint(canvas, crosshairMargin, 0);
            continue;
        }

        int pos = ii * 10 * pixelsPerDegreeLevel;
        int margin = (ii > 9 || ii < -9) ? 9 : 6;
        osdDrawArtificialHorizonLevelLine(canvas, levelBarWidth, -pos, margin);
    }

    displayCanvasContextPop(canvas);

    displayCanvasCtmTranslate(canvas, translateX, translateY);
    displayCanvasCtmScale(canvas, 0.5f, 0.5f);

    // Draw line labels
    float sx = sin_approx(rollAngle);
    float sy = cos_approx(-rollAngle);
    for (int ii = pitchCenter - 2; ii <= pitchCenter + 2; ii++) {
        if (ii == 0) {
            continue;
        }

        int level = ii * 10;
        int absLevel = ABS(level);
        itoa(absLevel, buf, 10);
        int pos = level * pixelsPerDegreeLevel;
        int charY = 9 - pos * 2;
        int cx = (absLevel >= 100 ? -1.5f : -1.0) * canvas->gridElementWidth;
        int px = cx + (pitchOffset + pos) * sx * 2;
        int py = -charY - (pitchOffset + pos) * (1 - sy) * 2;
        displayCanvasDrawString(canvas, px, py, buf, 0);
    }
    displayCanvasContextPop(canvas);
}

void osdCanvasDrawArtificialHorizon(displayPort_t *display, displayCanvas_t *canvas, const osdDrawPoint_t *p, float pitchAngle, float rollAngle)
{
    UNUSED(display);
    UNUSED(p);

    static float prevPitchAngle = 9999;
    static float prevRollAngle = 9999;
    static timeMs_t nextDrawMaxMs = 0;
    static timeMs_t nextDrawMinMs = 0;

    timeMs_t now = millis();

    float totalError = fabsf(prevPitchAngle - pitchAngle) + fabsf(prevRollAngle - rollAngle);
    if ((now > nextDrawMinMs && totalError > 0.05f)|| now > nextDrawMaxMs) {

        int x, y, w, h;
        osdArtificialHorizonRect(canvas, &x, &y, &w, &h);
        displayCanvasClearRect(canvas, x, y, w, h);

        osdDrawArtificialHorizonShapes(canvas, pitchAngle, rollAngle);
        prevPitchAngle = pitchAngle;
        prevRollAngle = rollAngle;
        nextDrawMinMs = now + AHI_MIN_DRAW_INTERVAL_MS;
        nextDrawMaxMs = now + AHI_MAX_DRAW_INTERVAL_MS;
    }
}

void osdCanvasDrawHeadingGraph(displayPort_t *display, displayCanvas_t *canvas, const osdDrawPoint_t *p, int heading)
{
    static const uint8_t graph[] = {
        SYM_HEADING_W,
        SYM_HEADING_LINE,
        SYM_HEADING_DIVIDED_LINE,
        SYM_HEADING_LINE,
        SYM_HEADING_N,
        SYM_HEADING_LINE,
        SYM_HEADING_DIVIDED_LINE,
        SYM_HEADING_LINE,
        SYM_HEADING_E,
        SYM_HEADING_LINE,
        SYM_HEADING_DIVIDED_LINE,
        SYM_HEADING_LINE,
        SYM_HEADING_S,
        SYM_HEADING_LINE,
        SYM_HEADING_DIVIDED_LINE,
        SYM_HEADING_LINE,
        SYM_HEADING_W,
        SYM_HEADING_LINE,
        SYM_HEADING_DIVIDED_LINE,
        SYM_HEADING_LINE,
        SYM_HEADING_N,
        SYM_HEADING_LINE,
        SYM_HEADING_DIVIDED_LINE,
        SYM_HEADING_LINE,
        SYM_HEADING_E,
        SYM_HEADING_LINE,
    };

    STATIC_ASSERT(sizeof(graph) > (3599 / OSD_HEADING_GRAPH_DECIDEGREES_PER_CHAR) + OSD_HEADING_GRAPH_WIDTH + 1, graph_is_too_short);

    char buf[OSD_HEADING_GRAPH_WIDTH + 1];
    int px;
    int py;

    osdDrawPointGetPixels(&px, &py, display, canvas, p);
    int rw = OSD_HEADING_GRAPH_WIDTH * canvas->gridElementWidth;
    int rh = canvas->gridElementHeight;

    displayCanvasClipToRect(canvas, px, py, rw, rh);

    int idx = heading / OSD_HEADING_GRAPH_DECIDEGREES_PER_CHAR;
    int offset = ((heading % OSD_HEADING_GRAPH_DECIDEGREES_PER_CHAR) * canvas->gridElementWidth) / OSD_HEADING_GRAPH_DECIDEGREES_PER_CHAR;
    memcpy_fn(buf, graph + idx, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    // We need a +1 because characters are 12px wide, so
    // they can't have a 1px arrow centered. All existing fonts
    // place the arrow at 5px, hence there's a 1px offset.
    // TODO: Put this in font metadata and read it back.
    displayCanvasDrawString(canvas, px - offset + 1, py, buf, DISPLAY_CANVAS_BITMAP_OPT_ERASE_TRANSPARENT);

    displayCanvasSetStrokeColor(canvas, DISPLAY_CANVAS_COLOR_BLACK);
    displayCanvasSetFillColor(canvas, DISPLAY_CANVAS_COLOR_WHITE);
    int rmx = px + rw / 2;
    displayCanvasFillStrokeTriangle(canvas, rmx - 2, py - 1, rmx + 2, py - 1, rmx, py + 1);
}

#endif
