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

FILE_COMPILE_FOR_SPEED

#include <stdint.h>
#include "secondary_dynamic_gyro_notch.h"
#include "fc/config.h"
#include "build/debug.h"
#include "sensors/gyro.h"

#define SECONDARY_DYNAMIC_NOTCH_DEFAULT_CENTER_HZ 150

void secondaryDynamicGyroNotchFiltersInit(secondaryDynamicGyroNotchState_t *state) {

    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        state->filtersApplyFn[axis] = nullFilterApply;
    }

    state->dynNotchQ = gyroConfig()->dynamicGyroNotch3dQ / 100.0f;
    state->enabled = gyroConfig()->dynamicGyroNotchMode != DYNAMIC_NOTCH_MODE_2D;
    state->looptime = getLooptime();

    if (
        gyroConfig()->dynamicGyroNotchMode == DYNAMIC_NOTCH_MODE_R ||
        gyroConfig()->dynamicGyroNotchMode == DYNAMIC_NOTCH_MODE_RP ||
        gyroConfig()->dynamicGyroNotchMode == DYNAMIC_NOTCH_MODE_RY ||
        gyroConfig()->dynamicGyroNotchMode == DYNAMIC_NOTCH_MODE_3D
    ) {
        /* 
         * Enable ROLL filter
         */
        biquadFilterInit(&state->filters[FD_ROLL], SECONDARY_DYNAMIC_NOTCH_DEFAULT_CENTER_HZ, state->looptime, 1.0f, FILTER_NOTCH);
        state->filtersApplyFn[FD_ROLL] = (filterApplyFnPtr)biquadFilterApplyDF1;
    }

    if (
        gyroConfig()->dynamicGyroNotchMode == DYNAMIC_NOTCH_MODE_P ||
        gyroConfig()->dynamicGyroNotchMode == DYNAMIC_NOTCH_MODE_RP ||
        gyroConfig()->dynamicGyroNotchMode == DYNAMIC_NOTCH_MODE_PY ||
        gyroConfig()->dynamicGyroNotchMode == DYNAMIC_NOTCH_MODE_3D
    ) {
        /* 
         * Enable PITCH filter
         */
        biquadFilterInit(&state->filters[FD_PITCH], SECONDARY_DYNAMIC_NOTCH_DEFAULT_CENTER_HZ, state->looptime, 1.0f, FILTER_NOTCH);
        state->filtersApplyFn[FD_PITCH] = (filterApplyFnPtr)biquadFilterApplyDF1;
    }

    if (
        gyroConfig()->dynamicGyroNotchMode == DYNAMIC_NOTCH_MODE_Y ||
        gyroConfig()->dynamicGyroNotchMode == DYNAMIC_NOTCH_MODE_PY ||
        gyroConfig()->dynamicGyroNotchMode == DYNAMIC_NOTCH_MODE_RY ||
        gyroConfig()->dynamicGyroNotchMode == DYNAMIC_NOTCH_MODE_3D
    ) {
        /* 
         * Enable YAW filter
         */
        biquadFilterInit(&state->filters[FD_YAW], SECONDARY_DYNAMIC_NOTCH_DEFAULT_CENTER_HZ, state->looptime, 1.0f, FILTER_NOTCH);
        state->filtersApplyFn[FD_YAW] = (filterApplyFnPtr)biquadFilterApplyDF1;
    }

    
}

void secondaryDynamicGyroNotchFiltersUpdate(secondaryDynamicGyroNotchState_t *state, int axis, float frequency[]) {

    if (state->enabled) {

        /*
         * The secondary dynamic notch uses only the first detected frequency
         * The rest of peaks are ignored
         */
        state->frequency[axis] = frequency[0];

        // Filter update happens only if peak was detected 
        if (frequency[0] > 0.0f) {
            biquadFilterUpdate(&state->filters[axis], state->frequency[axis], state->looptime, state->dynNotchQ, FILTER_NOTCH);
        }
    }
}

float secondaryDynamicGyroNotchFiltersApply(secondaryDynamicGyroNotchState_t *state, int axis, float input) {
    return state->filtersApplyFn[axis]((filter_t *)&state->filters[axis], input);
}

#endif