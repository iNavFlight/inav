/*
 * This file is part of INAV Project.
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
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "config/parameter_group.h"

#include "terrain/terrain_nav_hold_core.h"

/*
 * TERRAIN AGL HOLD - cruise holds height above ground from SD terrain data.
 *
 * The single gate for map terrain into the altitude target path: one update
 * per navigation cycle, active only in the cruise whitelist, opt-in by RC box,
 * health-gated, all commands through the one altitude-target funnel
 * (updateClimbRateToAltitudeController). Everything else in the firmware
 * behaves bit-for-bit as stock when the box is off.
 *
 * Not to be confused with INAV's existing "terrain follow" naming, which is
 * the rangefinder SURFACE mode.
 */

typedef struct terrainNavConfig_s {
    uint16_t minAglCm;          // minimum held height above ground [cm]
    uint16_t lookaheadDistM;    // how far ahead rising terrain is checked [m], 0 = off
} terrainNavConfig_t;

PG_DECLARE(terrainNavConfig_t, terrainNavConfig);

// The gate. Called once per navigation cycle from applyWaypointNavigationAndAltitudeHold()
void terrainNavCruiseHoldUpdate(void);

terrainNavHoldStatus_e terrainNavHoldGetStatus(void);
terrainNavHoldWarning_e terrainNavHoldGetWarning(void);
bool terrainNavHoldAutoClimbRunning(void);

// True while the hold owns the altitude target (active or frozen)
bool terrainNavHoldIsEngaged(void);

// The held height above ground; false unless engaged
bool terrainNavHoldGetTargetAglCm(int32_t *targetAglCm);
