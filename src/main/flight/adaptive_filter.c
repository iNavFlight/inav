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

#ifdef USE_ADAPTIVE_FILTER

#include <stdlib.h>
#include "flight/adaptive_filter.h"
#include "arm_math.h"
#include <math.h>
#include "common/maths.h"
#include "common/axis.h"
#include "common/filter.h"
#include "build/debug.h"
#include "fc/config.h"

STATIC_FASTRAM float32_t adaptiveFilterSamples[XYZ_AXIS_COUNT][ADAPTIVE_FILTER_BUFFER_SIZE];
STATIC_FASTRAM uint8_t adaptiveFilterSampleIndex = 0;

STATIC_FASTRAM pt1Filter_t rmsFilter[XYZ_AXIS_COUNT];
STATIC_FASTRAM pt1Filter_t hpfFilter[XYZ_AXIS_COUNT];

STATIC_FASTRAM uint8_t adaptiveFilterInitialized = 0;
STATIC_FASTRAM uint8_t hpfFilterInitialized = 0;

/**
 * This function is called at pid rate, so has to be initialized at PID loop frequency
*/
void adaptiveFilterPush(const flight_dynamics_index_t index, const float value) {

    if (!hpfFilterInitialized) {
        //Initialize the filter
        for (flight_dynamics_index_t axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
            pt1FilterInit(&hpfFilter[axis], ADAPTIVE_FILTER_HPF_HZ, US2S(getLooptime()));
        }
        hpfFilterInitialized = 1;
    }

    const float filteredGyro = value - pt1FilterApply(&hpfFilter[index], value);

    //Push new sample to the buffer so later we can compute RMS and other measures
    adaptiveFilterSamples[index][adaptiveFilterSampleIndex] = filteredGyro;
    adaptiveFilterSampleIndex = (adaptiveFilterSampleIndex + 1) % ADAPTIVE_FILTER_BUFFER_SIZE;
}

void adaptiveFilterTask(timeUs_t currentTimeUs) {
    static timeUs_t previousUpdateTimeUs;
    const float dT = US2S(currentTimeUs - previousUpdateTimeUs);

    if (!adaptiveFilterInitialized) {
        //Initialize the filter
        for (flight_dynamics_index_t axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
            pt1FilterInit(&rmsFilter[axis], ADAPTIVE_FILTER_LPF_HZ, 1.0f / ADAPTIVE_FILTER_RATE_HZ);
        }
        adaptiveFilterInitialized = 1;
    }

    float combinedRms = 0.0f;

    //Compute RMS for each axis
    for (flight_dynamics_index_t axis = 0; axis < XYZ_AXIS_COUNT; axis++) {

        //Copy axis samples to a temporary buffer
        float32_t tempBuffer[ADAPTIVE_FILTER_BUFFER_SIZE];
        //Use memcpy to copy the samples to the temporary buffer
        memcpy(tempBuffer, adaptiveFilterSamples[axis], sizeof(adaptiveFilterSamples[axis]));

        //Compute RMS from normalizedBuffer using arm_rms_f32
        float32_t rms;
        arm_rms_f32(tempBuffer, ADAPTIVE_FILTER_BUFFER_SIZE, &rms);

        float32_t filteredRms = pt1FilterApply(&rmsFilter[axis], rms);

        combinedRms += filteredRms;

        DEBUG_SET(DEBUG_ADAPTIVE_FILTER, axis, filteredRms * 1000.0f);
    }

    combinedRms /= XYZ_AXIS_COUNT;

    DEBUG_SET(DEBUG_ADAPTIVE_FILTER, 3, combinedRms * 1000.0f);


}


#endif /* USE_ADAPTIVE_FILTER */