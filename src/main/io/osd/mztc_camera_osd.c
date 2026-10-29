/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#include "common/printf.h"
#include "common/utils.h"

#include "drivers/display.h"

#include "io/osd/mztc_camera_osd.h"
#include "io/mztc_camera.h"

#ifdef USE_MZTC

// Three letters per palette, in mztcPaletteMode_e order.
static const char *const mztcOsdPaletteNames[] = {
    "WHT", "BLK", "FU1", "RBW", "FU2", "IR1", "IR2",
    "SEP", "CL1", "CL2", "ICE", "RAI", "GRN", "RED"
};

static const char *const mztcOsdZoomNames[] = { "1X", "2X", "4X", "8X" };

void mztcOsdFormatStatus(char *buff, textAttributes_t *attr)
{
    const mztcStatus_t *status = mztcGetStatus();
    const char *text;
    bool linkUp = true;

    switch (status->status) {
    case MZTC_STATUS_READY:
    case MZTC_STATUS_CAPTURING:
        text = "OK";
        break;
    case MZTC_STATUS_CALIBRATING:
        text = "FFC";
        break;
    case MZTC_STATUS_RECORDING:
        text = "REC";
        break;
    case MZTC_STATUS_ALERT:
        text = "ALT";
        break;
    case MZTC_STATUS_INITIALIZING:
        text = "INI";
        linkUp = false;
        break;
    case MZTC_STATUS_ERROR:
        text = "ERR";
        linkUp = false;
        break;
    case MZTC_STATUS_OFFLINE:
    default:
        text = "OFF";
        linkUp = false;
        break;
    }

    if (linkUp) {
        // Zoom and palette are the values the driver last sent, the same
        // commanded state the in-flight adjustments step through. They are
        // only meaningful while a camera is listening.
        const mztcConfig_t *cfg = mztcConfig();
        const char *zoom = cfg->zoom_level < ARRAYLEN(mztcOsdZoomNames) ?
            mztcOsdZoomNames[cfg->zoom_level] : "--";
        const char *palette = cfg->palette_mode < ARRAYLEN(mztcOsdPaletteNames) ?
            mztcOsdPaletteNames[cfg->palette_mode] : "---";
        tfp_sprintf(buff, "IR %s %s %s", text, zoom, palette);
    } else {
        tfp_sprintf(buff, "IR %s", text);
    }

    // Pad to a constant width so a shorter state never leaves characters of
    // the previous one on screen.
    for (unsigned i = strlen(buff); i < MZTC_OSD_ELEMENT_LENGTH - 1; i++) {
        buff[i] = ' ';
    }
    buff[MZTC_OSD_ELEMENT_LENGTH - 1] = '\0';

    if (status->status == MZTC_STATUS_ERROR || status->status == MZTC_STATUS_OFFLINE) {
        TEXT_ATTRIBUTES_ADD_BLINK(*attr);
    }
}

#endif // USE_MZTC
