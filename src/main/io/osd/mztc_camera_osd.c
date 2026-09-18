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

void mztcOsdFormatStatus(char *buff, textAttributes_t *attr)
{
    const mztcStatus_t *status = mztcGetStatus();
    const char *text;

    switch (status->status) {
    case MZTC_STATUS_READY:
    case MZTC_STATUS_CAPTURING:
        text = "OK ";
        break;
    case MZTC_STATUS_INITIALIZING:
        text = "INI";
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
    case MZTC_STATUS_ERROR:
        text = "ERR";
        break;
    case MZTC_STATUS_OFFLINE:
    default:
        text = "OFF";
        break;
    }

    tfp_sprintf(buff, "IR %s", text);

    if (status->status == MZTC_STATUS_ERROR || status->status == MZTC_STATUS_OFFLINE) {
        TEXT_ATTRIBUTES_ADD_BLINK(*attr);
    }
}

#endif // USE_MZTC
