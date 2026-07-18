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

// Altitude floor ("training floor"): while the ALT FLOOR box is active and
// the aircraft has climbed above floor + margin once, BREAKING THROUGH the
// floor while sinking engages an automatic recovery (shortest-path roll to
// upright plus climb pitch via the orientation hold controller) until the
// aircraft is back above the floor and climbing. No prediction - a piloted
// trajectory is not predictable; the line itself is the law. Switch the
// box off to land.

typedef struct altitudeFloorConfig_s {
    uint16_t floorAltitude;     // m above home
    uint16_t floorMargin;       // m above the floor to arm / release
    uint8_t  floorClimbPitch;   // deg nose-up target during recovery
} altitudeFloorConfig_t;

PG_DECLARE(altitudeFloorConfig_t, altitudeFloorConfig);

// Run once per RC processing cycle (before flight mode selection)
void altitudeFloorUpdate(void);

// True while the automatic recovery is flying the aircraft
bool altitudeFloorRecoveryActive(void);

// True once the floor is armed (climbed above floor + margin)
bool altitudeFloorArmed(void);

// Recovery pitch target (deg, nose up): full climb pitch while below the
// floor, a gentle altitude-hold pitch once orbiting
float altitudeFloorRecoveryPitchDeg(void);

// Recovery roll target (deg): wings level during the climb, the orbit
// bank once the aircraft is back at the floor waiting for the pilot
float altitudeFloorRecoveryRollDeg(void);

// True in recovery phase 2: circling at the floor around the breach
// point until the pilot takes over (stick input) or switches the box off
bool altitudeFloorOrbitActive(void);

// True while the orbit is flown by the forced nav loiter (healthy
// position estimate); false in the degraded constant-bank circle
bool altitudeFloorOrbitViaNav(void);

// Metres above (positive) / below (negative) the floor line - the
// telemetry/OSD readout of how much sky is left before the net
float altitudeFloorDistanceM(void);
