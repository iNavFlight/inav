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

// Hover throttle: while PROP HANG is held (nose near vertical) the thrust
// carries the weight and owns the altitude axis. A dedicated throttle PID
// holds the altitude captured at engage; its I-term is seeded from the
// pilot's throttle (learning the hover throttle online) and the output is
// tilt compensated. Moving the throttle stick out of the mid deadband
// hands control back to the pilot and re-captures the target.

typedef struct hoverThrottleConfig_s {
    uint8_t pGain;         // throttle us per m of altitude error
    uint8_t iGain;         // throttle us per m per second
    uint8_t dGain;         // throttle us per m/s of climb rate
    uint16_t minThrottle;  // throttle floor [us] while hovering: preserves
                           // the control authority that scales with thrust,
                           // prop wash over the surfaces as well as thrust
                           // vectoring (an updraft otherwise makes the PID
                           // cut the throttle and with it the authority).
                           // One propeller, one floor: the same value
                           // covers both steering paths. Found by
                           // experiment near the model's hover throttle;
                           // 1000 = no floor beyond the motor idle.
} hoverThrottleConfig_t;

PG_DECLARE(hoverThrottleConfig_t, hoverThrottleConfig);

// Called from the mixer throttle path; returns the pilot throttle when the
// hover throttle is not active, the controller output otherwise.
int16_t hoverThrottleApply(int16_t pilotThrottle);
