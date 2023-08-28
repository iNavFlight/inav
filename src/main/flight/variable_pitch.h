/*
 * This file is part of INAV
 *
 * INAV free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * INAV distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#include "config/parameter_group.h"

#if defined(USE_VARIABLE_PITCH)

typedef struct helicopterConfig_s {             // woga65: helicopter specific settings
    int16_t nav_hc_hover_collective[3];         // On helicopters the amount of collective-pitch needed to hover at a certain head speed
} helicopterConfig_t;

PG_DECLARE(helicopterConfig_t, helicopterConfig);   // woga65:

uint16_t getHoverCollectivePitch(void);

#endif
