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
    state->filtersApplyFn = nullFilterApply;

    state->dynNotchQ = gyroConfig()->dynamicGyroNotchQ / 100.0f;
    state->enabled = gyroConfig()->dynamicGyroNotchEnabled;
    state->looptime = getLooptime();

    if (state->enabled) {
        /*
         * Step 1 - init all filters even if they will not be used further down the road
         */
        for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
            //Any initial notch Q is valid sice it will be updated immediately after
            biquadFilterInit(&state->filters[axis][0], DYNAMIC_NOTCH_DEFAULT_CENTER_HZ, state->looptime, 1.0f, FILTER_NOTCH);
            biquadFilterInit(&state->filters[axis][1], DYNAMIC_NOTCH_DEFAULT_CENTER_HZ, state->looptime, 1.0f, FILTER_NOTCH);
            biquadFilterInit(&state->filters[axis][2], DYNAMIC_NOTCH_DEFAULT_CENTER_HZ, state->looptime, 1.0f, FILTER_NOTCH);
        }

        state->filtersApplyFn = (filterApplyFnPtr)biquadFilterApplyDF1;
    }
}

void dynamicGyroNotchFiltersUpdate(dynamicGyroNotchState_t *state, int axis, uint16_t frequency) {

    state->frequency[axis] = frequency;

    DEBUG_SET(DEBUG_DYNAMIC_FILTER_FREQUENCY, axis, frequency);

    if (state->enabled) {
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
     */
    output = state->filtersApplyFn((filter_t *)&state->filters[axis][0], output);
    output = state->filtersApplyFn((filter_t *)&state->filters[axis][1], output);
    output = state->filtersApplyFn((filter_t *)&state->filters[axis][2], output);

    return output;
}

#endif