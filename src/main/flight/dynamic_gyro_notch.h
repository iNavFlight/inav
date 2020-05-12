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
#include "common/axis.h"
#include "common/filter.h"

#define DYNAMIC_NOTCH_DEFAULT_CENTER_HZ 350

typedef struct dynamicGyroNotchState_s {
    uint16_t frequency[XYZ_AXIS_COUNT];
    float dynNotchQ;
    float dynNotch1Ctr;
    float dynNotch2Ctr;
    uint32_t looptime;
    uint8_t enabled;
    /*
     * Dynamic gyro filter can be 3x1, 3x2 or 3x3 depending on filter type
     */
    biquadFilter_t filters[XYZ_AXIS_COUNT][XYZ_AXIS_COUNT];
    filterApplyFnPtr filtersApplyFn;
} dynamicGyroNotchState_t;

void dynamicGyroNotchFiltersInit(dynamicGyroNotchState_t *state);
void dynamicGyroNotchFiltersUpdate(dynamicGyroNotchState_t *state, int axis, uint16_t frequency);
float dynamicGyroNotchFiltersApply(dynamicGyroNotchState_t *state, int axis, float input);
