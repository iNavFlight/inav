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

#include "config/parameter_group.h"

// Tip-over guard for autogyros. The rotor is the wing: its lift and its
// roll authority both scale with rotor rpm squared, and the rpm lives on
// the inflow through the disk. In slow flight or a botched launch the rpm
// decays, the lateral tilt goes soft, and the aircraft rolls away with the
// stick already at the stop - unrecoverable by attitude control alone.
// There is no rpm feedback on these airframes; the ONLY lever that restores
// authority is THRUST (thrust -> speed -> inflow -> rpm). The guard detects
// the uncommanded roll excursion while sinking and flies the recovery:
// wings level, nose slightly DOWN (feed the disk), and a throttle floor.
typedef struct rotorGuardConfig_s {
    uint8_t bankDeg;        // an autogyro never flies beyond this bank on
                            // purpose - excursion past it while sinking is
                            // the tip-over signature
    uint16_t sinkCms;       // minimum sink rate [cm/s] to qualify
    int8_t recoveryPitchDeg; // nose-down pitch target during recovery
                             // (negative = down): restores inflow
    uint16_t throttleAddUs;  // recovery throttle floor = cruise + this
} rotorGuardConfig_t;

PG_DECLARE(rotorGuardConfig_t, rotorGuardConfig);

void rotorGuardUpdate(void);
bool rotorGuardRecoveryActive(void);
float rotorGuardRecoveryPitchDeg(void);

// Recovery throttle floor [us] (cruise + rotor_guard_throttle_add);
// 0 while inactive. Consumed as a MAX claim by the throttle path.
int16_t rotorGuardThrottleFloorUs(void);
