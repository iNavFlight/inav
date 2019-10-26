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

#include "platform.h"

#include "common/utils.h"

#include "drivers/display.h"
#include "drivers/display_canvas.h"
#include "drivers/osd.h"

#include "io/osd_canvas.h"
#include "io/osd_common.h"
#include "io/osd_grid.h"

#define CANVAS_DEFAULT_GRID_ELEMENT_WIDTH OSD_CHAR_WIDTH
#define CANVAS_DEFAULT_GRID_ELEMENT_HEIGHT OSD_CHAR_HEIGHT

void osdDrawPointGetGrid(uint8_t *gx, uint8_t *gy, const displayPort_t *display, const displayCanvas_t *canvas, const osdDrawPoint_t *p)
{
    UNUSED(display);

    switch (p->type) {
        case OSD_DRAW_POINT_TYPE_GRID:
            *gx = p->grid.gx;
            *gy = p->grid.gy;
            break;
        case OSD_DRAW_POINT_TYPE_PIXEL:
            *gx = p->pixel.px / (canvas ? canvas->gridElementWidth : CANVAS_DEFAULT_GRID_ELEMENT_WIDTH);
            *gy = p->pixel.py / (canvas ? canvas->gridElementHeight : OSD_CHAR_HEIGHT);
            break;
    }
}

void osdDrawPointGetPixels(int *px, int *py, const displayPort_t *display, const displayCanvas_t *canvas, const osdDrawPoint_t *p)
{
    UNUSED(display);

    switch (p->type) {
        case OSD_DRAW_POINT_TYPE_GRID:
            *px = p->grid.gx * (canvas ? canvas->gridElementWidth : CANVAS_DEFAULT_GRID_ELEMENT_WIDTH);
            *py = p->grid.gy * (canvas ? canvas->gridElementHeight : CANVAS_DEFAULT_GRID_ELEMENT_HEIGHT);
            break;
        case OSD_DRAW_POINT_TYPE_PIXEL:
            *px = p->pixel.px;
            *py = p->pixel.py;
            break;
    }
}

void osdDrawVario(displayPort_t *display, displayCanvas_t *canvas, const osdDrawPoint_t *p, float zvel)
{
    uint8_t gx;
    uint8_t gy;

#if defined(USE_CANVAS)
    if (canvas) {
        osdCanvasDrawVario(display, canvas, p, zvel);
    } else {
#endif
        osdDrawPointGetGrid(&gx, &gy, display, canvas, p);
        osdGridDrawVario(display, gx, gy, zvel);
#if defined(USE_CANVAS)
    }
#endif
}

void osdDrawDirArrow(displayPort_t *display, displayCanvas_t *canvas, const osdDrawPoint_t *p, float degrees, bool eraseBefore)
{
#if !defined(USE_CANVAS)
    UNUSED(eraseBefore);
#endif
    uint8_t gx;
    uint8_t gy;

#if defined(USE_CANVAS)
    if (canvas) {
        osdCanvasDrawDirArrow(display, canvas, p, degrees, eraseBefore);
    } else {
#endif
        osdDrawPointGetGrid(&gx, &gy, display, canvas, p);
        osdGridDrawDirArrow(display, gx, gy, degrees);
#if defined(USE_CANVAS)
    }
#endif
}

void osdDrawArtificialHorizon(displayPort_t *display, displayCanvas_t *canvas, const osdDrawPoint_t *p, float rollAngle, float pitchAngle)
{
    uint8_t gx;
    uint8_t gy;
#if defined(USE_CANVAS)
    if (canvas) {
        osdCanvasDrawArtificialHorizon(display, canvas, p, pitchAngle, rollAngle);
    } else {
#endif
        osdDrawPointGetGrid(&gx, &gy, display, canvas, p);
        osdGridDrawArtificialHorizon(display, gx, gy, pitchAngle, rollAngle);
#if defined(USE_CANVAS)
    }
#endif
}

void osdDrawHeadingGraph(displayPort_t *display, displayCanvas_t *canvas, const osdDrawPoint_t *p, int heading)
{
    uint8_t gx;
    uint8_t gy;
#if defined(USE_CANVAS)
    if (canvas) {
        osdCanvasDrawHeadingGraph(display, canvas, p, heading);
    } else {
#endif
        osdDrawPointGetGrid(&gx, &gy, display, canvas, p);
        osdGridDrawHeadingGraph(display, gx, gy, heading);
#if defined(USE_CANVAS)
    }
#endif
}
