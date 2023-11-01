/*
 * This file is part of INAV.
 *
 * INAV is free software. You can redistribute this software
 * and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * INAV is distributed in the hope that they will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "drivers/time.h"
#include "config/parameter_group.h"

typedef enum  {
    FW_AUTOLAND_APPROACH_DIRECTION_LEFT,
    FW_AUTOLAND_APPROACH_DIRECTION_RIGHT
} fwAutolandApproachDirection_e;

typedef struct navFwAutolandConfig_s
{
    uint8_t approachAngle;
    uint16_t glideAltitude;
    uint16_t flareAltitude;
    uint16_t flarePitch;
    uint16_t maxTailwind;
} navFwAutolandConfig_t;

PG_DECLARE(navFwAutolandConfig_t, navFwAutolandConfig);

typedef enum {
    FW_AUTOLAND_STATE_ABOVE_LOITER_ALT,
    FW_AUTOLAND_STATE_LOITER,
    FW_AUTOLAND_STATE_DOWNWIND,
    FW_AUTOLAND_STATE_BASE,
    FW_AUTOLAND_STATE_FINAL_APPROACH,
    FW_AUTOLAND_STATE_GLIDE,
    FW_AUTOLAND_STATE_FLARE,
    FW_AUTOLAND_STATE_SWITCH_TO_EMERGENCY_LANDING,
    FW_AUTOLAND_STATE_ABORT,
    FW_AUTOLAND_STATE_LANDED,
    FW_AUTOLAND_STATE_COUNT
} fwAutolandState_t;

typedef enum {
    FW_AUTOLAND_MESSAGE_ABOVE_LOITER_ALT,
    FW_AUTOLAND_MESSAGE_LOITER,
    FW_AUTOLAND_MESSAGE_ABORT,
} fwAutolandMessageState_t;

fwAutolandState_t getFwAutolandState(void);
void resetFwAutolandController(timeUs_t currentTimeUs);
bool allowFwAutoland(void);
bool isFwAutolandActive(void);
void applyFixedWingAutolandController(timeUs_t currentTimeUs);