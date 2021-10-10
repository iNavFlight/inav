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

#define Q_TUNE_SHORT_BUFFER_PERIOD_MS 350
#define Q_TUNE_LONG_BUFFER_PERIOD_MS  800

#define Q_TUNE_SHORT_BUFFER_LENGTH 52 // Q_TUNE_UPDATE_RATE_HZ * Q_TUNE_SHORT_BUFFER_PERIOD_MS / 1000
#define Q_TUNE_LONG_BUFFER_LENGTH 120  // Q_TUNE_UPDATE_RATE_HZ * Q_TUNE_LONG_BUFFER_PERIOD_MS / 1000

typedef struct currentSample_s {
    float setpoint;
    float measurement;
    float iTerm;
} currentSample_t;

typedef struct samples_s {
    timeUs_t lastExecution;
    uint16_t indexShort;
    uint16_t indexLong;
    float rate;
    float setpointFiltered[Q_TUNE_SHORT_BUFFER_LENGTH];
    float measurementFiltered[Q_TUNE_SHORT_BUFFER_LENGTH];
    float error[Q_TUNE_SHORT_BUFFER_LENGTH];
    float iTerm[Q_TUNE_LONG_BUFFER_LENGTH];
    pt1Filter_t setpointFilter;
    pt1Filter_t measurementFilter;
    float setpointMean;
    float measurementMean;
    float setpointStdDev;
    float measurementStdDev;
    float setpointRms;
    float measurementRms;
    float errorRms;
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

void qTuneProcessTask(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);

    static bool initialized = false;
    if (!initialized) {
        for (int i = 0; i < XYZ_AXIS_COUNT; i++) {
            samples[i].indexShort = 0;
            samples[i].indexLong = 0;
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

    // Pick last sample and start filling in the data

    for (int i = 0; i < XYZ_AXIS_COUNT; i++) {

        //FIXME For now we exclude YAW from processing
        if (i == FD_YAW) {
            continue;
        }

        currentSample_t *sample = &currentSample[i];

        samples_t *axisSample = &samples[i];

        // Store and normalize iTerm
        axisSample->iTerm[samples->indexLong] = sample->iTerm / axisSample->rate;

        // filter the data with normalized stepoint and measurement
        axisSample->setpointFiltered[axisSample->indexShort] = pt1FilterApply(&axisSample->setpointFilter, sample->setpoint / axisSample->rate);
        axisSample->measurementFiltered[axisSample->indexShort] = pt1FilterApply(&axisSample->measurementFilter, sample->measurement / axisSample->rate);

        // calculate the error
        axisSample->error[axisSample->indexShort] = axisSample->setpointFiltered[axisSample->indexShort] - axisSample->measurementFiltered[axisSample->indexShort];
    
        // compute variance, RMS and other factors
        float out;

        arm_rms_f32(axisSample->error, Q_TUNE_SHORT_BUFFER_LENGTH, &out);
        axisSample->errorRms = out;

        arm_std_f32(axisSample->error, Q_TUNE_SHORT_BUFFER_LENGTH, &out);
        axisSample->errorStdDev = out;

        arm_rms_f32(axisSample->iTerm, Q_TUNE_LONG_BUFFER_LENGTH, &out);
        axisSample->iTermRms = out;

        arm_std_f32(axisSample->iTerm, Q_TUNE_LONG_BUFFER_LENGTH, &out);
        axisSample->iTermStdDev = out;

        // Step 6 - calculate setpoint Derivative
        axisSample->setpointDerivative = axisSample->setpointFiltered[axisSample->indexShort] - axisSample->setpointPrevious;
        axisSample->setpointPrevious = axisSample->setpointFiltered[axisSample->indexShort];
    }

    // Step 3 - Write blackbox data
    DEBUG_SET(DEBUG_Q_TUNE, 0, samples[FD_ROLL].error[samples[FD_ROLL].indexShort] * 1000.0f);
    DEBUG_SET(DEBUG_Q_TUNE, 1, samples[FD_ROLL].errorRms * 10000.0f);
    DEBUG_SET(DEBUG_Q_TUNE, 2, samples[FD_ROLL].errorStdDev * 10000.0f);
    DEBUG_SET(DEBUG_Q_TUNE, 3, samples[FD_ROLL].iTermRms * 10000.0f);
    DEBUG_SET(DEBUG_Q_TUNE, 4, samples[FD_ROLL].iTermStdDev * 10000.0f);
    DEBUG_SET(DEBUG_Q_TUNE, 5, samples[FD_ROLL].setpointDerivative * Q_TUNE_UPDATE_RATE_HZ * 1000.0f);

    for (int i = 0; i < XYZ_AXIS_COUNT; i++) {
        samples[i].indexShort = (samples[i].indexShort + 1) % Q_TUNE_SHORT_BUFFER_LENGTH;
        samples[i].indexLong = (samples[i].indexLong + 1) % Q_TUNE_LONG_BUFFER_LENGTH;
    }
}

#endif