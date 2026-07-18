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

#include <stdint.h>

#include "config/parameter_group.h"

// Thrust vectoring: dedicated servo mixer input sources (INPUT_TVC_*) that
// carry the same stabilized commands as the control surfaces but with a
// thrust dependent gain. The torque a vectoring vane / tilting motor can
// produce scales with thrust, so the deflection is compensated inversely
// (capped at low thrust) to keep the control loop gain roughly constant --
// full authority in a prop hang, no overcontrol at full power.

typedef struct thrustVectoringConfig_s {
    uint16_t gain;          // % overall TVC deflection gain at full thrust
    uint8_t  thrustComp;    // % inverse thrust compensation: 0 = none, 100 = full 1/thrust
} thrustVectoringConfig_t;

PG_DECLARE(thrustVectoringConfig_t, thrustVectoringConfig);

// Combined TVC gain for the current thrust fraction [0..1]
float thrustVectoringGain(float thrustFraction);

// Feed the TVC mixer input rows from the stabilized commands, scaled by
// the thrust dependent gain (servo mixer hook)
void thrustVectoringApplyInputs(int16_t *input, int16_t mixerThrottleCommand);
