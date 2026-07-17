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
// the aircraft has climbed above floor + margin once, a predicted floor
// breach engages an automatic recovery (shortest-path roll to upright plus
// climb pitch via the orientation hold controller) until the aircraft is
// back above the floor and climbing. Switch the box off to land.

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

// Recovery pitch target (deg, nose up)
float altitudeFloorRecoveryPitchDeg(void);
