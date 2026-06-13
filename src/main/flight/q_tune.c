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

#include <stdlib.h>
#include "q_tune.h"
#include <math.h>
#include "arm_math.h"
#include "common/maths.h"

#include "common/filter.h"
#include "build/debug.h"
#include "fc/control_profile.h"
#include "fc/rc_controls.h"
#include "flight/pid.h"

#define Q_TUNE_SHORT_BUFFER_PERIOD_MS 350
#define Q_TUNE_LONG_BUFFER_PERIOD_MS  800

#define Q_TUNE_SHORT_BUFFER_LENGTH 64 // Q_TUNE_UPDATE_RATE_HZ * Q_TUNE_SHORT_BUFFER_PERIOD_MS / 1000
#define Q_TUNE_LONG_BUFFER_LENGTH 128  // Q_TUNE_UPDATE_RATE_HZ * Q_TUNE_LONG_BUFFER_PERIOD_MS / 1000

#define Q_TUNE_HI_FREQ_THRESHOLD 12 // Osciallation above this frequency means we do vibrate and PID is not stable
#define Q_TUNE_HI_FREQ_EVENT_THRESHOLD_US 150000 // If we have a high frequency event for longer than this, we do vibrate 
#define Q_TUNE_HI_FREQ_EVENT_PERIOD_US 1000000 // Hi Freq event can happen only from time to time

#define Q_TUNE_LOW_FREQ_MIN_HZ 0.3f
#define Q_TUNE_LOW_FREQ_MAX_HZ 5.0f
#define Q_TUNE_LOW_FREQ_DECIMATION 4
#define Q_TUNE_LOW_FREQ_EFFECTIVE_HZ (Q_TUNE_UPDATE_RATE_HZ / Q_TUNE_LOW_FREQ_DECIMATION)
#define Q_TUNE_LOW_FREQ_SETPOINT_RMS_MAX 0.06f
#define Q_TUNE_LOW_FREQ_ERROR_ENERGY_MIN 0.020f
#define Q_TUNE_LOW_FREQ_ITERM_ENERGY_MIN 0.010f
#define Q_TUNE_LOW_FREQ_EVENT_THRESHOLD_US 600000
#define Q_TUNE_LOW_FREQ_EVENT_PERIOD_US 2000000


typedef struct currentSample_s {
    float setpoint;
    float measurement;
    float iTerm;
} currentSample_t;

typedef enum {
    Q_TUNE_STATE_HI_FREQ_OSCILLATION        = (1 << 0),     // 1
    Q_TUNE_STATE_LOW_FREQ_OSCILLATION       = (1 << 1),     // 2
    Q_TUNE_STATE_HI_FREQ_START              = (1 << 2),     // 4 start of a high frequency oscillation    
    Q_TUNE_STATE_LOW_FREQ_START             = (1 << 3),     // 8 start of a low frequency oscillation
} qTuneState_e;

typedef struct samples_s {
    uint8_t state;

    timeUs_t lastExecution;
    uint16_t indexShort;
    uint16_t indexLong;
    uint16_t decimationCounter;
    float rate;
    float setpointFiltered;
    float measurementFiltered;
    float error[Q_TUNE_SHORT_BUFFER_LENGTH];
    float errorLong[Q_TUNE_LONG_BUFFER_LENGTH];
    float setpointLong[Q_TUNE_LONG_BUFFER_LENGTH];
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
    arm_rfft_fast_instance_f32 longWindowFft;

    float fftPeakFrequency;
    float fftPeakValue;
    float fftMean;

    float lowFftPeakFrequency;
    float lowFftPeakValue;
    float lowFftBandMean;
    float lowITermBandMean;
    
    timeUs_t hiFreqEvenStartUs;
    timeUs_t lowFreqEvenStartUs;

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

static void getSampleFrequencyInRange(
    float *peakFrequency,
    float *peakValue,
    float *bandMean,
    arm_rfft_fast_instance_f32 *structure,
    float buffer[],
    const uint16_t bufferLength,
    const float minFrequency,
    const float maxFrequency
) {

    float rfftOutput[bufferLength];
    float magnitude[bufferLength];

    arm_rfft_fast_f32(structure, buffer, rfftOutput, 0);
    arm_cmplx_mag_f32(rfftOutput, magnitude, bufferLength / 2);

    const float hzPerBin = (float)Q_TUNE_LOW_FREQ_EFFECTIVE_HZ / bufferLength;
    uint16_t startBin = (uint16_t)ceilf(minFrequency / hzPerBin);
    uint16_t endBin = (uint16_t)floorf(maxFrequency / hzPerBin);
    const uint16_t lastBin = (bufferLength / 2) - 1;

    if (startBin < 1) {
        startBin = 1;
    }
    if (endBin > lastBin) {
        endBin = lastBin;
    }
    if (startBin > endBin) {
        *peakFrequency = 0.0f;
        *peakValue = 0.0f;
        *bandMean = 0.0f;
        return;
    }

    float maxValue = 0.0f;
    uint16_t maxBin = startBin;
    float sum = 0.0f;

    for (uint16_t bin = startBin; bin <= endBin; bin++) {
        const float value = magnitude[bin];
        sum += value;
        if (value > maxValue) {
            maxValue = value;
            maxBin = bin;
        }
    }

    const uint16_t bins = endBin - startBin + 1;
    *peakFrequency = maxBin * hzPerBin;
    *peakValue = maxValue;
    *bandMean = sum / bins;
}

/*
 * Low-frequency oscillation state machine (per axis):
 * 1) Run only when throttle > 1200; otherwise clear START and OSCILLATION.
 * 2) Candidate low-frequency oscillation requires:
 *    - peak in [Q_TUNE_LOW_FREQ_MIN_HZ, Q_TUNE_LOW_FREQ_MAX_HZ]
 *    - low setpoint activity (setpoint RMS below threshold)
 *    - sufficient low-band error and I-term energy
 * 3) START is rate-limited by Q_TUNE_LOW_FREQ_EVENT_PERIOD_US.
 * 4) Once START is set and candidate persists, set OSCILLATION after
 *    Q_TUNE_LOW_FREQ_EVENT_THRESHOLD_US.
 */
static void lowFrequencyDetector(samples_t *data, timeUs_t currentTimeUs)
{
    if (rcCommand[THROTTLE] <= 1200) {
        qTuneDisableState(&data->state, Q_TUNE_STATE_LOW_FREQ_START);
        qTuneDisableState(&data->state, Q_TUNE_STATE_LOW_FREQ_OSCILLATION);
        return;
    }

    const bool inLowBand =
        data->lowFftPeakFrequency >= Q_TUNE_LOW_FREQ_MIN_HZ &&
        data->lowFftPeakFrequency <= Q_TUNE_LOW_FREQ_MAX_HZ;
    const bool setpointQuiet = data->setpointRms <= Q_TUNE_LOW_FREQ_SETPOINT_RMS_MAX;
    const bool hasErrorEnergy = data->lowFftBandMean >= Q_TUNE_LOW_FREQ_ERROR_ENERGY_MIN;
    const bool hasITermEnergy = data->lowITermBandMean >= Q_TUNE_LOW_FREQ_ITERM_ENERGY_MIN;
    const bool lowFreqCandidate = inLowBand && setpointQuiet && hasErrorEnergy && hasITermEnergy;

    if (!lowFreqCandidate) {
        qTuneDisableState(&data->state, Q_TUNE_STATE_LOW_FREQ_START);
        qTuneDisableState(&data->state, Q_TUNE_STATE_LOW_FREQ_OSCILLATION);
        return;
    }

    const bool lowFreqStarted = qTuneState(data->state, Q_TUNE_STATE_LOW_FREQ_START);
    const timeUs_t eventAgeUs = currentTimeUs - data->lowFreqEvenStartUs;

    if (!lowFreqStarted) {
        if (eventAgeUs > Q_TUNE_LOW_FREQ_EVENT_PERIOD_US) {
            qTuneEnableState(&data->state, Q_TUNE_STATE_LOW_FREQ_START);
            data->lowFreqEvenStartUs = currentTimeUs;
        }
        return;
    }

    if (eventAgeUs > Q_TUNE_LOW_FREQ_EVENT_THRESHOLD_US) {
        qTuneEnableState(&data->state, Q_TUNE_STATE_LOW_FREQ_OSCILLATION);
    }
}

/*
 * High-frequency oscillation state machine (per axis):
 * 1) Run only when throttle > 1200. If throttle <= 1200, clear START and OSCILLATION.
 * 2) If peak frequency is below Q_TUNE_HI_FREQ_THRESHOLD, clear START and OSCILLATION.
 * 3) If peak frequency is above threshold and START is not set:
 *    START can be armed only after Q_TUNE_HI_FREQ_EVENT_PERIOD_US from the previous start time.
 * 4) Once START is set and frequency stays above threshold:
 *    set OSCILLATION after Q_TUNE_HI_FREQ_EVENT_THRESHOLD_US has elapsed.
 */
static void hiFrequencyDetector(samples_t * data, timeUs_t currentTimeUs) {

    /*
     * High frequency oscillation happens only when throttle is not low. This is due to the fact that the motors do not have enough
     * authority to feed enough energy to the whole system.
     * If a multitoror oscialltes on high throttle, it will oscillate on low as well
     * as it will start on high throttle. 
     */
    if (rcCommand[THROTTLE] <= 1200) {
        qTuneDisableState(&data->state, Q_TUNE_STATE_HI_FREQ_OSCILLATION);
        qTuneDisableState(&data->state, Q_TUNE_STATE_HI_FREQ_START);
        return;
    }

    const bool frequencyAboveThreshold = data->fftPeakFrequency >= Q_TUNE_HI_FREQ_THRESHOLD;
    if (!frequencyAboveThreshold) {
        // Oscillation ended
        qTuneDisableState(&data->state, Q_TUNE_STATE_HI_FREQ_START);
        qTuneDisableState(&data->state, Q_TUNE_STATE_HI_FREQ_OSCILLATION);
        return;
    }

    const bool hiFreqStarted = qTuneState(data->state, Q_TUNE_STATE_HI_FREQ_START);
    const timeUs_t eventAgeUs = currentTimeUs - data->hiFreqEvenStartUs;

    if (!hiFreqStarted) {
        // Osciallation started
        if (eventAgeUs > Q_TUNE_HI_FREQ_EVENT_PERIOD_US) {
            qTuneEnableState(&data->state, Q_TUNE_STATE_HI_FREQ_START);
            data->hiFreqEvenStartUs = currentTimeUs;
        }
        return;
    }

    // Oscillation continues for at least Q_TUNE_HI_FREQ_EVENT_THRESHOLD_US us
    if (eventAgeUs > Q_TUNE_HI_FREQ_EVENT_THRESHOLD_US) {
        qTuneEnableState(&data->state, Q_TUNE_STATE_HI_FREQ_OSCILLATION);
    }
}

static void getSampleFrequency(float *frequency, float *energy, float *mean, arm_rfft_fast_instance_f32 *structure, float buffer[], const uint16_t bufferLength) {

    //RFFT transform
    float rfft_output[bufferLength];
    float test_output[bufferLength];

    arm_rfft_fast_f32(structure, buffer, rfft_output, 0);

    //Calculate magnitude of imaginary coefficients
    arm_cmplx_mag_f32(rfft_output, test_output, bufferLength / 2);
    
    float maxvalue;
    uint32_t maxindex;
    float sampleMean;

    //Clear the first bin
    test_output[0] = 0;

    //Obtain peak frequency
    arm_max_f32(test_output, bufferLength / 2, &maxvalue, &maxindex);

    //Obtain mean value
    arm_mean_f32(test_output, bufferLength / 2, &sampleMean);

    *frequency = (float) maxindex * Q_TUNE_UPDATE_RATE_HZ / bufferLength;
    *energy = maxvalue;
    *mean = sampleMean;
}

void qTuneProcessTask(timeUs_t currentTimeUs) {

    static bool initialized = false;
    if (!initialized) {
        for (int i = 0; i < XYZ_AXIS_COUNT; i++) {

            arm_rfft_fast_init_f32(&samples[i].errorFft, Q_TUNE_SHORT_BUFFER_LENGTH);
            arm_rfft_fast_init_f32(&samples[i].longWindowFft, Q_TUNE_LONG_BUFFER_LENGTH);

            samples[i].state = 0;
            samples[i].indexShort = 0;
            samples[i].indexLong = 0;
            samples[i].decimationCounter = 0;
            samples[i].hiFreqEvenStartUs = 0;
            samples[i].lowFreqEvenStartUs = 0;
            samples[i].lastExecution = 0;

            pt1FilterInit(&samples[i].setpointFilter, Q_TUNE_SETPOINT_LPF_HZ, Q_TUNE_UPDATE_US * 1e-6f);
            pt1FilterInit(&samples[i].measurementFilter, Q_TUNE_MEASUREMENT_LPF_HZ, Q_TUNE_UPDATE_US * 1e-6f);
            pt1FilterInit(&samples[i].pt1ErrorHpfFilter, Q_TUNE_ERROR_HPF_HZ, Q_TUNE_UPDATE_US * 1e-6f);

            if (i == FD_ROLL) {
                samples[i].rate = currentControlProfile->stabilized.rates[FD_ROLL] * 10.0f;
            } else if (i == FD_PITCH) {
                samples[i].rate = currentControlProfile->stabilized.rates[FD_PITCH] * 10.0f;
            } else if (i == FD_YAW) {
                samples[i].rate = currentControlProfile->stabilized.rates[FD_YAW] * 10.0f;
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

        // filter the data with normalized stepoint and measurement
        axisSample->setpointFiltered = pt1FilterApply(&axisSample->setpointFilter, sample->setpoint / axisSample->rate);
        axisSample->measurementFiltered = pt1FilterApply(&axisSample->measurementFilter, sample->measurement / axisSample->rate);

        // calculate the error

        const float trackingError = axisSample->setpointFiltered - axisSample->measurementFiltered;
        float error = trackingError - pt1FilterApply(&axisSample->pt1ErrorHpfFilter, trackingError);  // Value - LPF = HPF

        axisSample->error[axisSample->indexShort] = error;

        // Low-frequency path uses decimation by 4
        axisSample->decimationCounter++;
        if (axisSample->decimationCounter >= Q_TUNE_LOW_FREQ_DECIMATION) {
            axisSample->decimationCounter = 0;
            axisSample->iTerm[axisSample->indexLong] = sample->iTerm / axisSample->rate;
            axisSample->errorLong[axisSample->indexLong] = trackingError;
            axisSample->setpointLong[axisSample->indexLong] = axisSample->setpointFiltered;
        }
    
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

        arm_rms_f32(axisSample->setpointLong, Q_TUNE_LONG_BUFFER_LENGTH, &out);
        axisSample->setpointRms = out;

        // Step 6 - calculate setpoint Derivative
        axisSample->setpointDerivative = axisSample->setpointFiltered - axisSample->setpointPrevious;
        axisSample->setpointPrevious = axisSample->setpointFiltered;

        float dataBufferShort[Q_TUNE_SHORT_BUFFER_LENGTH];

        memcpy(dataBufferShort, axisSample->error, sizeof(axisSample->error));
        getSampleFrequency(
            &axisSample->fftPeakFrequency, 
            &axisSample->fftPeakValue,
            &axisSample->fftMean,
            &axisSample->errorFft, 
            dataBufferShort,
            Q_TUNE_SHORT_BUFFER_LENGTH
        );
    
        hiFrequencyDetector(axisSample, currentTimeUs);

        // Low-frequency FFT only on decimation boundary
        if (axisSample->decimationCounter == 0) {
            float dataBufferLong[Q_TUNE_LONG_BUFFER_LENGTH];

            memcpy(dataBufferLong, axisSample->errorLong, sizeof(axisSample->errorLong));
            getSampleFrequencyInRange(
                &axisSample->lowFftPeakFrequency,
                &axisSample->lowFftPeakValue,
                &axisSample->lowFftBandMean,
                &axisSample->longWindowFft,
                dataBufferLong,
                Q_TUNE_LONG_BUFFER_LENGTH,
                Q_TUNE_LOW_FREQ_MIN_HZ,
                Q_TUNE_LOW_FREQ_MAX_HZ
            );

            memcpy(dataBufferLong, axisSample->iTerm, sizeof(axisSample->iTerm));
            getSampleFrequencyInRange(
                &axisSample->measurementMean,
                &axisSample->measurementStdDev,
                &axisSample->lowITermBandMean,
                &axisSample->longWindowFft,
                dataBufferLong,
                Q_TUNE_LONG_BUFFER_LENGTH,
                Q_TUNE_LOW_FREQ_MIN_HZ,
                Q_TUNE_LOW_FREQ_MAX_HZ
            );

            lowFrequencyDetector(axisSample, currentTimeUs);
        }
    
    }

    // Step 3 - Write blackbox data
    DEBUG_SET(DEBUG_Q_TUNE, 0, samples[FD_ROLL].error[samples[FD_ROLL].indexShort] * 1000.0f);
    DEBUG_SET(DEBUG_Q_TUNE, 1, samples[FD_ROLL].state);
    DEBUG_SET(DEBUG_Q_TUNE, 2, samples[FD_ROLL].fftPeakFrequency);
    DEBUG_SET(DEBUG_Q_TUNE, 3, samples[FD_ROLL].fftPeakValue * 10000.0f);
    DEBUG_SET(DEBUG_Q_TUNE, 4, samples[FD_ROLL].fftMean * 10000.0f);
    DEBUG_SET(DEBUG_Q_TUNE, 5, pidBank()->pid[FD_ROLL].P);
    
    // DEBUG_SET(DEBUG_Q_TUNE, 1, samples[FD_ROLL].errorRms * 10000.0f);
    // DEBUG_SET(DEBUG_Q_TUNE, 2, samples[FD_ROLL].errorStdDev * 10000.0f);
    // DEBUG_SET(DEBUG_Q_TUNE, 3, samples[FD_ROLL].iTermRms * 10000.0f);
    // DEBUG_SET(DEBUG_Q_TUNE, 4, samples[FD_ROLL].iTermStdDev * 10000.0f);
    // DEBUG_SET(DEBUG_Q_TUNE, 5, samples[FD_ROLL].setpointDerivative * Q_TUNE_UPDATE_RATE_HZ * 1000.0f);

    for (int i = 0; i < XYZ_AXIS_COUNT; i++) {
        samples[i].indexShort = (samples[i].indexShort + 1) % Q_TUNE_SHORT_BUFFER_LENGTH;
        if (samples[i].decimationCounter == 0) {
            samples[i].indexLong = (samples[i].indexLong + 1) % Q_TUNE_LONG_BUFFER_LENGTH;
        }
    }
}

#endif