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

#include <stdint.h>
#include "common/maths.h"
#include "common/quaternion.h"

#define EARTH_RADIUS ((double)6378.137)
#define PWM_TO_FLOAT_0_1(x) (((int)x - 1000) / 1000.0f)
#define PWM_TO_FLOAT_MINUS_1_1(x) (((int)x - 1500) / 500.0f)
#define FLOAT_0_1_TO_PWM(x) ((uint16_t)(x * 1000.0f) + 1000.0f)
#define FLOAT_MINUS_1_1_TO_PWM(x) ((uint16_t)((x + 1.0f) / 2.0f * 1000.0f) + 1000.0f)

int16_t constrainToInt16(double value);
void transformVectorEarthToBody(fpVector3_t *v, const fpQuaternion_t *quat);
void computeQuaternionFromRPY(fpQuaternion_t *quat, int16_t initialRoll, int16_t initialPitch, int16_t initialYaw);