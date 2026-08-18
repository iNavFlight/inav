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

#include "stdint.h"
#include "stdbool.h"

#include "color.h"
#include "colorconversion.h"

/*
 * Source below found here: http://www.kasperkamperman.com/blog/arduino/arduino-programming-hsb-to-rgb/
 */

// Small direct-mapped cache of recent HSV->RGB conversions. LED strip
// patterns tend to reuse a handful of distinct colors across many LEDs and
// updates, so a few cached entries catch most repeats without the cost of a
// full RGB-native color store. Round-robin replacement is enough here: the
// working set per frame is normally <= 4 distinct colors, so eviction order
// doesn't matter much.
static struct {
    hsvColor_t in;
    rgbColor24bpp_t out;
    bool valid;
} hsvToRgbCache[4];
static uint8_t hsvToRgbCacheNextSlot = 0;

static bool hsvColorEqual(const hsvColor_t *a, const hsvColor_t *b)
{
    return a->h == b->h && a->s == b->s && a->v == b->v;
}

rgbColor24bpp_t* hsvToRgb24(const hsvColor_t* c)
{
    for (int i = 0; i < 4; i++) {
        if (hsvToRgbCache[i].valid && hsvColorEqual(&hsvToRgbCache[i].in, c)) {
            return &hsvToRgbCache[i].out;
        }
    }

    rgbColor24bpp_t r;

    uint16_t val = c->v;
    uint16_t sat = 255 - c->s;
    uint32_t base;
    uint16_t hue = c->h;

    if (sat == 0) { // Acromatic color (gray). Hue doesn't mind.
        r.rgb.r = val;
        r.rgb.g = val;
        r.rgb.b = val;
    } else {

        base = ((255 - sat) * val) >> 8;

        switch (hue / 60) {
            case 0:
            r.rgb.r = val;
            r.rgb.g = (((val - base) * hue) / 60) + base;
            r.rgb.b = base;
            break;
            case 1:
            r.rgb.r = (((val - base) * (60 - (hue % 60))) / 60) + base;
            r.rgb.g = val;
            r.rgb.b = base;
            break;

            case 2:
            r.rgb.r = base;
            r.rgb.g = val;
            r.rgb.b = (((val - base) * (hue % 60)) / 60) + base;
            break;

            case 3:
            r.rgb.r = base;
            r.rgb.g = (((val - base) * (60 - (hue % 60))) / 60) + base;
            r.rgb.b = val;
            break;

            case 4:
            r.rgb.r = (((val - base) * (hue % 60)) / 60) + base;
            r.rgb.g = base;
            r.rgb.b = val;
            break;

            case 5:
            r.rgb.r = val;
            r.rgb.g = base;
            r.rgb.b = (((val - base) * (60 - (hue % 60))) / 60) + base;
            break;

        }
    }

    hsvToRgbCache[hsvToRgbCacheNextSlot].in = *c;
    hsvToRgbCache[hsvToRgbCacheNextSlot].out = r;
    hsvToRgbCache[hsvToRgbCacheNextSlot].valid = true;
    rgbColor24bpp_t *cached = &hsvToRgbCache[hsvToRgbCacheNextSlot].out;
    hsvToRgbCacheNextSlot = (hsvToRgbCacheNextSlot + 1) % 4;

    return cached;
}

