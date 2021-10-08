/* 
 * This file is part of the INAV distribution https://github.com/iNavFlight/inav.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "platform.h"

#ifdef USE_Q_TUNE

FILE_COMPILE_FOR_SPEED

#include <stdlib.h>
#include "q_tune.h"
#include <math.h>
#include "arm_math.h"

#include "common/filter.h"
#include "build/debug.h"
#include "fc/controlrate_profile.h"

#define Q_TUNE_WINDOW_LENGTH 150

typedef struct currentSample_s {
    float setpoint;
    float measurement;
    float gyroFrequency;
    float iTerm;
} currentSample_t;

typedef struct samples_s {
    timeUs_t lastExecution;
    uint8_t index;
    float rate;
    float setpointRaw[Q_TUNE_WINDOW_LENGTH];
    float measurementRaw[Q_TUNE_WINDOW_LENGTH];
    float setpointFiltered[Q_TUNE_WINDOW_LENGTH];
    float measurementFiltered[Q_TUNE_WINDOW_LENGTH];
    float iTerm[Q_TUNE_WINDOW_LENGTH];
    float gyroFrequency;
    float error[Q_TUNE_WINDOW_LENGTH];
    pt1Filter_t setpointFilter;
    pt1Filter_t measurementFilter;
    float setpointMean;
    float measurementMean;
    float setpointStdDev;
    float measurementStdDev;
    float setpointRms;
    float measurementRms;
    float setpointVariance;
    float measurementVariance;
    float errorRms;
    float errorVariance;
    float errorStdDev;
    float iTermRms;
    float iTermStdDev;
    float setpointPrevious;
    float setpointDerivative;
} samples_t;

static currentSample_t currentSample[XYZ_AXIS_COUNT];
static samples_t samples[XYZ_AXIS_COUNT];

void qTunePushSample(const flight_dynamics_index_t axis, const float setpoint, const float measurement, const float iTerm) {
    currentSample[axis].setpoint = setpoint;
    currentSample[axis].measurement = measurement;
    currentSample[axis].iTerm = iTerm;
}

void qTunePushGyroPeakFrequency(const flight_dynamics_index_t axis, const float frequency) {
    currentSample[axis].gyroFrequency = frequency;
}

void qTuneProcessTask(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);

    static bool initialized = false;
    if (!initialized) {
        for (int i = 0; i < XYZ_AXIS_COUNT; i++) {
            samples[i].index = 0;
            samples[i].lastExecution = 0;
            pt1FilterInit(&samples[i].setpointFilter, Q_TUNE_SETPOINT_LPF_HZ, Q_TUNE_UPDATE_US * 1e-6f);
            pt1FilterInit(&samples[i].measurementFilter, Q_TUNE_MEASUREMENT_LPF_HZ, Q_TUNE_UPDATE_US * 1e-6f);
            if (i == FD_ROLL) {
                samples[i].rate = currentControlRateProfile->stabilized.rates[FD_ROLL] * 10.0f;
            } else if (i == FD_PITCH) {
                samples[i].rate = currentControlRateProfile->stabilized.rates[FD_PITCH] * 10.0f;
            } else if (i == FD_YAW) {
                samples[i].rate = currentControlRateProfile->stabilized.rates[FD_YAW] * 10.0f;
            }
        }

        initialized = true;
    }

    // Step 1 - pick last sample and start filling in the data

    for (int i = 0; i < XYZ_AXIS_COUNT; i++) {

        //FIXME For now we exclude YAW from processing
        if (i == FD_YAW) {
            continue;
        }

        currentSample_t *sample = &currentSample[i];

        samples_t *axisSample = &samples[i];

        // Step 2 - fill in the data
        axisSample->gyroFrequency = sample->gyroFrequency;
        axisSample->setpointRaw[samples->index] = sample->setpoint / axisSample->rate;
        axisSample->measurementRaw[samples->index] = sample->measurement / axisSample->rate;
        axisSample->iTerm[samples->index] = sample->iTerm / axisSample->rate;

        // Step 3 - filter the data
        axisSample->setpointFiltered[axisSample->index] = pt1FilterApply(&axisSample->setpointFilter, axisSample->setpointRaw[axisSample->index]);
        axisSample->measurementFiltered[axisSample->index] = pt1FilterApply(&axisSample->measurementFilter, axisSample->measurementRaw[axisSample->index]);

        // Step 4 - calculate the error
        axisSample->error[axisSample->index] = axisSample->setpointFiltered[axisSample->index] - axisSample->measurementFiltered[axisSample->index];
    
        // Step 5 compute variance, RMS and other factors
        float out;

        arm_var_f32(axisSample->setpointFiltered, Q_TUNE_WINDOW_LENGTH, &out);
        axisSample->setpointVariance = out;

        arm_var_f32(axisSample->measurementFiltered, Q_TUNE_WINDOW_LENGTH, &out);
        axisSample->measurementVariance = out;

        arm_var_f32(axisSample->error, Q_TUNE_WINDOW_LENGTH, &out);
        axisSample->errorVariance = out;

        arm_rms_f32(axisSample->error, Q_TUNE_WINDOW_LENGTH, &out);
        axisSample->errorRms = out;

        arm_std_f32(axisSample->error, Q_TUNE_WINDOW_LENGTH, &out);
        axisSample->errorStdDev = out;

        arm_rms_f32(axisSample->iTerm, Q_TUNE_WINDOW_LENGTH, &out);
        axisSample->iTermRms = out;

        arm_std_f32(axisSample->iTerm, Q_TUNE_WINDOW_LENGTH, &out);
        axisSample->iTermStdDev = out;

        // Step 6 - calculate setpoint Derivative
        axisSample->setpointDerivative = axisSample->setpointFiltered[axisSample->index] - axisSample->setpointPrevious;
        axisSample->setpointPrevious = axisSample->setpointFiltered[axisSample->index];
    }

    // Step 3 - Write blackbox data
    DEBUG_SET(DEBUG_Q_TUNE, 0, samples[FD_ROLL].error[samples[FD_ROLL].index] * 1000.0f);
    DEBUG_SET(DEBUG_Q_TUNE, 1, samples[FD_ROLL].errorVariance * 10000.0f);
    DEBUG_SET(DEBUG_Q_TUNE, 2, samples[FD_ROLL].errorRms * 10000.0f);
    DEBUG_SET(DEBUG_Q_TUNE, 3, samples[FD_ROLL].errorStdDev * 10000.0f);
    DEBUG_SET(DEBUG_Q_TUNE, 4, samples[FD_ROLL].gyroFrequency);
    DEBUG_SET(DEBUG_Q_TUNE, 5, samples[FD_ROLL].iTermStdDev * 10000.0f);
    DEBUG_SET(DEBUG_Q_TUNE, 6, samples[FD_ROLL].iTermRms * 10000.0f);
    DEBUG_SET(DEBUG_Q_TUNE, 7, samples[FD_ROLL].setpointDerivative * Q_TUNE_UPDATE_RATE_HZ * 1000.0f);

    for (int i = 0; i < XYZ_AXIS_COUNT; i++) {
        samples[i].index = (samples[i].index + 1) % Q_TUNE_WINDOW_LENGTH;
    }
}

#endif