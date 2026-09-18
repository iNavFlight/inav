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

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "drivers/display.h"

#ifdef USE_MZTC

/*
 * MassZero Thermal Camera OSD element.
 *
 * Position and visibility come from INAV's own OSD layout configuration. The
 * item is OSD_MZTC_STATUS in osd_items_e. This module owns no parameter group
 * and schedules no updates of its own. It only formats the text that
 * osdDrawSingleElement() then draws.
 *
 * The function writes a NUL-terminated string into buff. The caller must
 * supply at least MZTC_OSD_ELEMENT_LENGTH bytes. It sets *attr for blink when
 * the link needs attention.
 */

#define MZTC_OSD_ELEMENT_LENGTH 12

void mztcOsdFormatStatus(char *buff, textAttributes_t *attr);

#endif // USE_MZTC
