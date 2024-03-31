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
#include "build/debug.h"

static float32_t adaptiveFilterSamples[XYZ_AXIS_COUNT][ADAPTIVE_FILTER_BUFFER_SIZE];
static uint8_t adaptiveFilterSampleIndex = 0;

void adaptiveFilterPush(const flight_dynamics_index_t index, const float value) {
    //Push new sample to the buffer so later we can compute RMS and other measures
    adaptiveFilterSamples[index][adaptiveFilterSampleIndex] = value;
    adaptiveFilterSampleIndex = (adaptiveFilterSampleIndex + 1) % ADAPTIVE_FILTER_BUFFER_SIZE;
}

void adaptiveFilterTask(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);

    //Compute RMS for each axis
    for (flight_dynamics_index_t axis = 0; axis < XYZ_AXIS_COUNT; axis++) {

        //Copy axis samples to a temporary buffer
        float32_t tempBuffer[ADAPTIVE_FILTER_BUFFER_SIZE];
        //Use memcpy to copy the samples to the temporary buffer
        memcpy(tempBuffer, adaptiveFilterSamples[axis], sizeof(adaptiveFilterSamples[axis]));

        //Use arm_mean_f32 to compute the mean of the samples
        float32_t mean;
        arm_mean_f32(tempBuffer, ADAPTIVE_FILTER_BUFFER_SIZE, &mean);

        float32_t normalizedBuffer[ADAPTIVE_FILTER_BUFFER_SIZE];

        //Use arm_offset_f32 to subtract the mean from the samples
        arm_offset_f32(tempBuffer, -mean, normalizedBuffer, ADAPTIVE_FILTER_BUFFER_SIZE);

        //Compute RMS from normalizedBuffer using arm_rms_f32
        float32_t rms;
        arm_rms_f32(normalizedBuffer, ADAPTIVE_FILTER_BUFFER_SIZE, &rms);

        DEBUG_SET(DEBUG_ADAPTIVE_FILTER, axis, rms * 1000.0f);
    }


}


#endif /* USE_ADAPTIVE_FILTER */