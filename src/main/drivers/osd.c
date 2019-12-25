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
 */

#include "drivers/display_canvas.h"
#include "drivers/osd.h"

uint16_t osdCharacterGridBuffer[OSD_CHARACTER_GRID_BUFFER_SIZE] ALIGNED(4);

void osdCharacterGridBufferClear(void)
{
    uint32_t *ptr = (uint32_t *)osdCharacterGridBuffer;
    uint32_t *end = (uint32_t *)(ARRAYEND(osdCharacterGridBuffer));
    for (; ptr < end; ptr++) {
        *ptr = 0;
    }
}

static void osdGridBufferConstrainRect(int *x, int *y, int *w, int *h, int totalWidth, int totalHeight)
{
    if (*w < 0) {
        *x += *w;
        *w = -*w;
    }
    if (*h < 0) {
        *y += *h;
        *h = -*h;
    }
    if (*x < 0) {
        *w -= *x;
        *x = 0;
    }
    if (*y < 0) {
        *h += *y;
        *y = 0;
    }
    int maxX = *x + *w;
    int extraWidth = maxX - totalWidth;
    if (extraWidth > 0) {
        *w -= extraWidth;
    }
    int maxY = *y + *h;
    int extraHeight = maxY - totalHeight;
    if (extraHeight > 0) {
        *h -= extraHeight;
    }
}

void osdGridBufferClearGridRect(int x, int y, int w, int h)
{
    osdGridBufferConstrainRect(&x, &y, &w, &h, OSD_CHARACTER_GRID_MAX_WIDTH, OSD_CHARACTER_GRID_MAX_HEIGHT);
    int maxX = x + w;
    int maxY = y + h;
    for (int ii = x; ii <= maxX + w; ii++) {
        for (int jj = y; jj <= maxY; jj++) {
            *osdCharacterGridBufferGetEntryPtr(ii, jj) = 0;
        }
    }
}

void osdGridBufferClearPixelRect(displayCanvas_t *canvas, int x, int y, int w, int h)
{
    // Ensure we clear all grid slots that overlap with this rect
    if (w < 0) {
        x += w;
        w = -w;
    }
    if (h < 0) {
        y += h;
        h = -h;
    }
    int gx = x / canvas->gridElementWidth;
    int gy = y / canvas->gridElementHeight;
    int gw = (w + canvas->gridElementWidth - 1) / canvas->gridElementWidth;
    int gh = (h + canvas->gridElementHeight - 1) / canvas->gridElementHeight;
    osdGridBufferClearGridRect(gx, gy, gw, gh);
}

uint16_t *osdCharacterGridBufferGetEntryPtr(unsigned x, unsigned y)
{
    unsigned pos = y * OSD_CHARACTER_GRID_MAX_WIDTH + x;
    return &osdCharacterGridBuffer[pos];
}
