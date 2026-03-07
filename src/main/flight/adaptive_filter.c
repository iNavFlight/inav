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
#include "fc/runtime_config.h"
#include "fc/rc_controls.h"
#include "sensors/gyro.h"

STATIC_FASTRAM float32_t adaptiveFilterSamples[XYZ_AXIS_COUNT][ADAPTIVE_FILTER_BUFFER_SIZE];
STATIC_FASTRAM uint8_t adaptiveFilterSampleIndex = 0;

STATIC_FASTRAM pt1Filter_t stdFilter[XYZ_AXIS_COUNT];
STATIC_FASTRAM pt1Filter_t hpfFilter[XYZ_AXIS_COUNT];

/*
    We want to run adaptive filter only when UAV is commanded to stay stationary
    Any rotation request on axis will add noise that we are not interested in as it will
    automatically cause LPF frequency to be lowered
*/
STATIC_FASTRAM float axisAttenuationFactor[XYZ_AXIS_COUNT];

STATIC_FASTRAM uint8_t adaptiveFilterInitialized = 0;
STATIC_FASTRAM uint8_t hpfFilterInitialized = 0;

//Defines if current, min and max  values for the filter were set and filter is ready to work
STATIC_FASTRAM uint8_t targetsSet = 0;
STATIC_FASTRAM float currentLpf;
STATIC_FASTRAM float initialLpf;
STATIC_FASTRAM float minLpf;
STATIC_FASTRAM float maxLpf; 

STATIC_FASTRAM float adaptiveFilterIntegrator;
STATIC_FASTRAM float adaptiveIntegratorTarget;

/**
 * This function is called at pid rate, so has to be initialized at PID loop frequency
*/
void adaptiveFilterPush(const flight_dynamics_index_t index, const float value) {

    if (!hpfFilterInitialized) {
        //Initialize the filter
        for (flight_dynamics_index_t axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
            pt1FilterInit(&hpfFilter[axis], gyroConfig()->adaptiveFilterHpfHz, US2S(getLooptime()));
        }
        hpfFilterInitialized = 1;
    }

    //Apply high pass filter, we are not interested in slowly changing values, only noise
    const float filteredGyro = value - pt1FilterApply(&hpfFilter[index], value);

    //Push new sample to the buffer so later we can compute RMS and other measures
    adaptiveFilterSamples[index][adaptiveFilterSampleIndex] = filteredGyro;
    adaptiveFilterSampleIndex = (adaptiveFilterSampleIndex + 1) % ADAPTIVE_FILTER_BUFFER_SIZE;
}

void adaptiveFilterPushRate(const flight_dynamics_index_t index, const float rate, const uint8_t configRate) {
    const float maxRate = configRate * 10.0f;
    axisAttenuationFactor[index] = scaleRangef(fabsf(rate), 0.0f, maxRate, 1.0f, 0.0f);
    axisAttenuationFactor[index] = constrainf(axisAttenuationFactor[index], 0.0f, 1.0f);
}

void adaptiveFilterResetIntegrator(void) {
    adaptiveFilterIntegrator = 0.0f;
}

void adaptiveFilterSetDefaultFrequency(int lpf, int min, int max) {
    currentLpf = lpf;
    minLpf = min;
    maxLpf = max;
    initialLpf = currentLpf;

    targetsSet = 1;
}

void adaptiveFilterTask(timeUs_t currentTimeUs) {

    //If we don't have current, min and max values for the filter, we can't run it yet
    if (!targetsSet) {
        return;
    }

    static timeUs_t previousUpdateTimeUs = 0;

    //Initialization procedure, filter setup etc.
    if (!adaptiveFilterInitialized) {
        adaptiveIntegratorTarget = 3.5f;
        previousUpdateTimeUs = currentTimeUs;

        //Initialize the filter
        for (flight_dynamics_index_t axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
            pt1FilterInit(&stdFilter[axis], gyroConfig()->adaptiveFilterStdLpfHz, 1.0f / ADAPTIVE_FILTER_RATE_HZ);
        }
        adaptiveFilterInitialized = 1;
    }

    //If not armed, leave this routine but reset integrator and set default LPF
    if (!ARMING_FLAG(ARMED)) {
        currentLpf = initialLpf;
        adaptiveFilterResetIntegrator();
        gyroUpdateDynamicLpf(currentLpf);
        return;
    }

    //Do not run adaptive filter when throttle is low
    if (rcCommand[THROTTLE] < 1200) {
        return;
    }

    //Prepare time delta to normalize time factor of the integrator 
    const float dT = US2S(currentTimeUs - previousUpdateTimeUs);
    previousUpdateTimeUs = currentTimeUs;

    float combinedStd = 0.0f;

    //Compute RMS for each axis
    for (flight_dynamics_index_t axis = 0; axis < XYZ_AXIS_COUNT; axis++) {

        //Copy axis samples to a temporary buffer
        float32_t tempBuffer[ADAPTIVE_FILTER_BUFFER_SIZE];
        
        //Copute STD from buffer using arm_std_f32
        float32_t std;
        memcpy(tempBuffer, adaptiveFilterSamples[axis], sizeof(adaptiveFilterSamples[axis]));
        arm_std_f32(tempBuffer, ADAPTIVE_FILTER_BUFFER_SIZE, &std);

        const float filteredStd = pt1FilterApply(&stdFilter[axis], std);
        const float error = filteredStd - adaptiveIntegratorTarget;
        const float adjustedError = error * axisAttenuationFactor[axis];
        const float timeAdjustedError = adjustedError * dT;

        //Put into integrator
        adaptiveFilterIntegrator += timeAdjustedError;

        combinedStd += std;
    }

    if (adaptiveFilterIntegrator > gyroConfig()->adaptiveFilterIntegratorThresholdHigh) {
        //In this case there is too much noise, we need to lower the LPF frequency
        currentLpf = constrainf(currentLpf - 1.0f, minLpf, maxLpf);
        gyroUpdateDynamicLpf(currentLpf);
        adaptiveFilterResetIntegrator();
    } else if (adaptiveFilterIntegrator < gyroConfig()->adaptiveFilterIntegratorThresholdLow) {
        //In this case there is too little noise, we can to increase the LPF frequency
        currentLpf = constrainf(currentLpf + 1.0f, minLpf, maxLpf);
        gyroUpdateDynamicLpf(currentLpf);
        adaptiveFilterResetIntegrator();
    }

    combinedStd /= XYZ_AXIS_COUNT;

    DEBUG_SET(DEBUG_ADAPTIVE_FILTER, 0, combinedStd * 1000.0f);
    DEBUG_SET(DEBUG_ADAPTIVE_FILTER, 1, adaptiveFilterIntegrator * 10.0f);
    DEBUG_SET(DEBUG_ADAPTIVE_FILTER, 2, currentLpf);
}


#endif /* USE_ADAPTIVE_FILTER */