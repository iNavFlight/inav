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
#include "common/maths.h"

#include "common/filter.h"
#include "build/debug.h"
#include "fc/controlrate_profile.h"
#include "fc/rc_controls.h"

#define Q_TUNE_SHORT_BUFFER_PERIOD_MS 350
#define Q_TUNE_LONG_BUFFER_PERIOD_MS  800

#define Q_TUNE_SHORT_BUFFER_LENGTH 64 // Q_TUNE_UPDATE_RATE_HZ * Q_TUNE_SHORT_BUFFER_PERIOD_MS / 1000
#define Q_TUNE_LONG_BUFFER_LENGTH 128  // Q_TUNE_UPDATE_RATE_HZ * Q_TUNE_LONG_BUFFER_PERIOD_MS / 1000

#define Q_TUNE_HI_FREQ_THRESHOLD 12 // Osciallation above this frequency means we do vibrate and PID is not stable
#define Q_TUNE_HI_FREQ_EVENT_THRESHOLD_US 150000 // If we have a high frequency event for longer than this, we do vibrate 
#define Q_TUNE_HI_FREQ_EVENT_PERIOD_US 1000000 // Hi Freq event can happen only from time to time


typedef struct currentSample_s {
    float setpoint;
    float measurement;
    float iTerm;
} currentSample_t;

typedef enum {
    Q_TUNE_STATE_HI_FREQ_OSCILLATION        = (1 << 0),
    Q_TUNE_STATE_LOW_FREQ_OSCILLATION       = (1 << 1),
    Q_TUNE_STATE_HI_FREQ_START              = (1 << 2),   // start of a high frequency oscillation    
} qTuneState_e;

typedef struct samples_s {
    uint8_t state;

    timeUs_t lastExecution;
    uint16_t indexShort;
    uint16_t indexLong;
    float rate;
    float setpointFiltered;
    float measurementFiltered;
    float error[Q_TUNE_SHORT_BUFFER_LENGTH];
    float iTerm[Q_TUNE_LONG_BUFFER_LENGTH];

    pt1Filter_t setpointFilter;
    pt1Filter_t measurementFilter;
    pt1Filter_t pt1ErrorHpfFilter;

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

    arm_rfft_fast_instance_f32 errorFft;

    float errorFrequency;
    float errorFrequencyEnergy;
    
    timeUs_t hiFreqEvenStartUs;

} samples_t;

static currentSample_t currentSample[XYZ_AXIS_COUNT];
static samples_t samples[XYZ_AXIS_COUNT];

void qTunePushSample(const flight_dynamics_index_t axis, const float setpoint, const float measurement, const float iTerm) {
    currentSample[axis].setpoint = setpoint;
    currentSample[axis].measurement = measurement;
    currentSample[axis].iTerm = iTerm;
}

static void qTuneEnableState(uint8_t *data, qTuneState_e state) {
    *data |= state;
}

static void qTuneDisableState(uint8_t *data, qTuneState_e state) {
    *data &= ~state;
}

static bool qTuneState(uint8_t data, qTuneState_e state) {
    return data & state;
}

static void hiFrequencyDetector(samples_t * data, timeUs_t currentTimeUs) {

    /*
     * High frequency oscillation happens only when throttle is not low. This is due to the fact that the motors do not have enough
     * authority to feed enough energy to the whole system.
     * If a multitoror oscialltes on high throttle, it will oscillate on low as well
     * as it will start on high throttle. 
     */
    if (rcCommand[THROTTLE] > 1200) {

        if (
            !qTuneState(data->state, Q_TUNE_STATE_HI_FREQ_START) && 
            data->errorFrequency >= Q_TUNE_HI_FREQ_THRESHOLD &&
            currentTimeUs - data->hiFreqEvenStartUs > Q_TUNE_HI_FREQ_EVENT_PERIOD_US    // At least so many us between two events
        ) {
            // Osciallation started
            qTuneEnableState(&data->state, Q_TUNE_STATE_HI_FREQ_START);
            data->hiFreqEvenStartUs = currentTimeUs;
        } else if (
            qTuneState(data->state, Q_TUNE_STATE_HI_FREQ_START) && 
            currentTimeUs - data->hiFreqEvenStartUs > Q_TUNE_HI_FREQ_EVENT_THRESHOLD_US
        ) {
            //Oscillation continues for at least Q_TUNE_HI_FREQ_EVENT_THRESHOLD_US us
            qTuneEnableState(&data->state, Q_TUNE_STATE_HI_FREQ_OSCILLATION);
        } else if (data->errorFrequency < Q_TUNE_HI_FREQ_THRESHOLD) {
            //Oscillation ended
            qTuneDisableState(&data->state, Q_TUNE_STATE_HI_FREQ_START);
            qTuneDisableState(&data->state, Q_TUNE_STATE_HI_FREQ_OSCILLATION);
        }
    
    } else {
        qTuneDisableState(&data->state, Q_TUNE_STATE_HI_FREQ_OSCILLATION);
        qTuneDisableState(&data->state, Q_TUNE_STATE_HI_FREQ_START);
    }
}

static void getSampleFrequency(float *frequency, float *energy, arm_rfft_fast_instance_f32 *structure, float buffer[], const uint16_t bufferLength) {

    //RFFT transform
    float rfft_output[bufferLength];
    float test_output[bufferLength];

    arm_rfft_fast_f32(structure, buffer, rfft_output, 0);

    //Calculate magnitude of imaginary coefficients
    arm_cmplx_mag_f32(rfft_output, test_output, bufferLength / 2);
    
    float maxvalue;
    uint32_t maxindex;

    //Clear the first bin
    test_output[0] = 0;

    //Obtain peak frequency
    arm_max_f32(test_output, bufferLength / 2, &maxvalue, &maxindex);

    *frequency = (float) maxindex * Q_TUNE_UPDATE_RATE_HZ / bufferLength;
    *energy = maxvalue;   
}

void qTuneProcessTask(timeUs_t currentTimeUs) {

    static bool initialized = false;
    if (!initialized) {
        for (int i = 0; i < XYZ_AXIS_COUNT; i++) {

            arm_rfft_fast_init_f32(&samples[i].errorFft, Q_TUNE_SHORT_BUFFER_LENGTH);

            samples[i].state = 0;
            samples[i].indexShort = 0;
            samples[i].indexLong = 0;
            samples[i].hiFreqEvenStartUs = 0;
            samples[i].lastExecution = 0;

            pt1FilterInit(&samples[i].setpointFilter, Q_TUNE_SETPOINT_LPF_HZ, Q_TUNE_UPDATE_US * 1e-6f);
            pt1FilterInit(&samples[i].measurementFilter, Q_TUNE_MEASUREMENT_LPF_HZ, Q_TUNE_UPDATE_US * 1e-6f);
            pt1FilterInit(&samples[i].pt1ErrorHpfFilter, Q_TUNE_ERROR_HPF_HZ, Q_TUNE_UPDATE_US * 1e-6f);

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
        axisSample->setpointFiltered = pt1FilterApply(&axisSample->setpointFilter, sample->setpoint / axisSample->rate);
        axisSample->measurementFiltered = pt1FilterApply(&axisSample->measurementFilter, sample->measurement / axisSample->rate);

        // calculate the error

        float error = axisSample->setpointFiltered - axisSample->measurementFiltered;
        error = error - pt1FilterApply(&axisSample->pt1ErrorHpfFilter, error);  // Value - LPF = HPF

        axisSample->error[axisSample->indexShort] = error;
    
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
        axisSample->setpointDerivative = axisSample->setpointFiltered - axisSample->setpointPrevious;
        axisSample->setpointPrevious = axisSample->setpointFiltered;

        float dataBuffer[Q_TUNE_LONG_BUFFER_LENGTH];

        memcpy(dataBuffer, axisSample->error, sizeof(axisSample->error));
        getSampleFrequency(&axisSample->errorFrequency, &axisSample->errorFrequencyEnergy, &axisSample->errorFft, dataBuffer, Q_TUNE_SHORT_BUFFER_LENGTH);
    
        hiFrequencyDetector(axisSample, currentTimeUs);
    
    }

    // Step 3 - Write blackbox data
    DEBUG_SET(DEBUG_Q_TUNE, 0, samples[FD_ROLL].error[samples[FD_ROLL].indexShort] * 1000.0f);
    DEBUG_SET(DEBUG_Q_TUNE, 1, samples[FD_ROLL].errorRms * 10000.0f);
    DEBUG_SET(DEBUG_Q_TUNE, 2, samples[FD_ROLL].errorStdDev * 10000.0f);
    DEBUG_SET(DEBUG_Q_TUNE, 3, samples[FD_ROLL].state);
    // DEBUG_SET(DEBUG_Q_TUNE, 3, samples[FD_ROLL].iTermRms * 10000.0f);
    // DEBUG_SET(DEBUG_Q_TUNE, 4, samples[FD_ROLL].iTermStdDev * 10000.0f);
    DEBUG_SET(DEBUG_Q_TUNE, 5, samples[FD_ROLL].setpointDerivative * Q_TUNE_UPDATE_RATE_HZ * 1000.0f);
    DEBUG_SET(DEBUG_Q_TUNE, 6, samples[FD_ROLL].errorFrequency);
    DEBUG_SET(DEBUG_Q_TUNE, 7, samples[FD_ROLL].errorFrequencyEnergy * 10000.0f);

    for (int i = 0; i < XYZ_AXIS_COUNT; i++) {
        samples[i].indexShort = (samples[i].indexShort + 1) % Q_TUNE_SHORT_BUFFER_LENGTH;
        samples[i].indexLong = (samples[i].indexLong + 1) % Q_TUNE_LONG_BUFFER_LENGTH;
    }
}

#endif