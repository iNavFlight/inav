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

#include <platform.h>

#ifdef USE_THRUST_VECTORING

#include "common/maths.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "fc/settings.h"

#include "flight/thrust_vectoring.h"

PG_REGISTER_WITH_RESET_TEMPLATE(thrustVectoringConfig_t, thrustVectoringConfig, PG_THRUST_VECTORING_CONFIG, 0);

PG_RESET_TEMPLATE(thrustVectoringConfig_t, thrustVectoringConfig,
    .gain = SETTING_TVC_GAIN_DEFAULT,
    .thrustComp = SETTING_TVC_THRUST_COMP_DEFAULT,
);

// Below this thrust fraction the compensation stops growing: vane authority
// is gone anyway and the servos should not flail against the stops
#define TVC_THRUST_COMP_FLOOR 0.25f

float thrustVectoringGain(float thrustFraction)
{
    const float t = constrainf(thrustFraction, TVC_THRUST_COMP_FLOOR, 1.0f);
    const float fullComp = 1.0f / t;    // 1 .. 1/floor
    const float comp = 1.0f + (fullComp - 1.0f) * (thrustVectoringConfig()->thrustComp / 100.0f);
    return (thrustVectoringConfig()->gain / 100.0f) * comp;
}

#endif // USE_THRUST_VECTORING
