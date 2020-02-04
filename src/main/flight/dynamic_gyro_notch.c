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

#include "platform.h"

#ifdef USE_DYNAMIC_FILTERS

#include <stdint.h>
#include "dynamic_gyro_notch.h"
#include "fc/config.h"
#include "build/debug.h"
#include "sensors/gyro.h"

void dynamicGyroNotchFiltersInit(dynamicGyroNotchState_t *state) {
    state->filtersApplyFn[0] = nullFilterApply;
    state->filtersApplyFn[1] = nullFilterApply;
    state->filtersApplyFn[2] = nullFilterApply;

    state->dynNotchQ = gyroConfig()->dyn_notch_q / 100.0f;
    state->filterType = gyroConfig()->dynamicGyroNotchType;
    state->dynNotch1Ctr = 1 - gyroConfig()->dyn_notch_width_percent / 100.0f;
    state->dynNotch2Ctr = 1 + gyroConfig()->dyn_notch_width_percent / 100.0f;
    state->looptime = getLooptime();

    if (state->filterType != DYNAMIC_GYRO_NOTCH_OFF) {
        const float notchQ = filterGetNotchQ(DYNAMIC_NOTCH_DEFAULT_CENTER_HZ, DYNAMIC_NOTCH_DEFAULT_CUTOFF_HZ); // any defaults OK here

        /*
         * Step 1 - init all filters even if they will not be used further down the road
         */
        for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
            biquadFilterInit(&state->filters[axis][0], DYNAMIC_NOTCH_DEFAULT_CENTER_HZ, state->looptime, notchQ, FILTER_NOTCH);
            biquadFilterInit(&state->filters[axis][1], DYNAMIC_NOTCH_DEFAULT_CENTER_HZ, state->looptime, notchQ, FILTER_NOTCH);
            biquadFilterInit(&state->filters[axis][2], DYNAMIC_NOTCH_DEFAULT_CENTER_HZ, state->looptime, notchQ, FILTER_NOTCH);
        }

        /*
         * Step 2 - configure filter apply functions depending on used filter type
         */
        if (state->filterType == DYNAMIC_GYRO_NOTCH_SINGLE) {
            state->filtersApplyFn[0] = (filterApplyFnPtr)biquadFilterApplyDF1;
        } else if (state->filterType == DYNAMIC_GYRO_NOTCH_DUAL) {
            state->filtersApplyFn[0] = (filterApplyFnPtr)biquadFilterApplyDF1;
            state->filtersApplyFn[1] = (filterApplyFnPtr)biquadFilterApplyDF1;
        } else {
            //Type MATRIX
            state->filtersApplyFn[0] = (filterApplyFnPtr)biquadFilterApplyDF1;
            state->filtersApplyFn[1] = (filterApplyFnPtr)biquadFilterApplyDF1;
            state->filtersApplyFn[2] = (filterApplyFnPtr)biquadFilterApplyDF1;
        }
    }
}

void dynamicGyroNotchFiltersUpdate(dynamicGyroNotchState_t *state, int axis, uint16_t frequency) {

    state->frequency[axis] = frequency;

    DEBUG_SET(DEBUG_DYNAMIC_FILTER_FREQUENCY, axis, frequency);

    if (state->filterType == DYNAMIC_GYRO_NOTCH_SINGLE) {
        biquadFilterUpdate(&state->filters[axis][0], frequency, state->looptime, state->dynNotchQ, FILTER_NOTCH);
    } else if (state->filterType == DYNAMIC_GYRO_NOTCH_DUAL) {
        biquadFilterUpdate(&state->filters[axis][0], frequency * state->dynNotch1Ctr, state->looptime, state->dynNotchQ, FILTER_NOTCH);
        biquadFilterUpdate(&state->filters[axis][1], frequency * state->dynNotch2Ctr, state->looptime, state->dynNotchQ, FILTER_NOTCH);
    } else if (state->filterType == DYNAMIC_GYRO_NOTCH_MATRIX) {
        biquadFilterUpdate(&state->filters[0][axis], frequency, state->looptime, state->dynNotchQ, FILTER_NOTCH);
        biquadFilterUpdate(&state->filters[1][axis], frequency, state->looptime, state->dynNotchQ, FILTER_NOTCH);
        biquadFilterUpdate(&state->filters[2][axis], frequency, state->looptime, state->dynNotchQ, FILTER_NOTCH);
    }

}

float dynamicGyroNotchFiltersApply(dynamicGyroNotchState_t *state, int axis, float input) {
    float output = input; 

    /*
     * We always apply all filters. If a filter dimension is disabled, one of
     * the function pointers will be a null apply function
     * 
     * Thanks to this, there is no branching 
     */
    output = state->filtersApplyFn[0]((filter_t *)&state->filters[axis][0], output);
    output = state->filtersApplyFn[1]((filter_t *)&state->filters[axis][1], output);
    output = state->filtersApplyFn[2]((filter_t *)&state->filters[axis][2], output);

    return output;
}

#endif