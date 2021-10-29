/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#ifdef USE_DYNAMIC_FILTERS

#include "arm_math.h"
#include "common/filter.h"

/*
 * Current code works only with 64 window size. Changing it do a different size would require
 * adapting the gyroDataAnalyseUpdate in STEP_ARM_CFFT_F32 step
 */
#define FFT_WINDOW_SIZE 64

typedef struct gyroAnalyseState_s {
    // accumulator for oversampled data => no aliasing and less noise
    float currentSample[XYZ_AXIS_COUNT];

    // downsampled gyro data circular buffer for frequency analysis
    uint8_t circularBufferIdx;
    float downsampledGyroData[XYZ_AXIS_COUNT][FFT_WINDOW_SIZE];

    // update state machine step information
    uint8_t updateTicks;
    uint8_t updateStep;
    uint8_t updateAxis;

    arm_rfft_fast_instance_f32 fftInstance;
    float fftData[FFT_WINDOW_SIZE];
    float rfftData[FFT_WINDOW_SIZE];

    biquadFilter_t detectedFrequencyFilter[XYZ_AXIS_COUNT];
    uint16_t centerFreq[XYZ_AXIS_COUNT];
    uint16_t prevCenterFreq[XYZ_AXIS_COUNT];
    float detectedFrequencyRaw[XYZ_AXIS_COUNT];
    bool filterUpdateExecute;
    uint8_t filterUpdateAxis;
    uint16_t filterUpdateFrequency;
    uint16_t fftSamplingRateHz;
    uint8_t fftStartBin;
    float fftResolution;
    uint16_t minFrequency;
    uint16_t maxFrequency;

    // Hanning window, see https://en.wikipedia.org/wiki/Window_function#Hann_.28Hanning.29_window
    float hanningWindow[FFT_WINDOW_SIZE];
} gyroAnalyseState_t;

STATIC_ASSERT(FFT_WINDOW_SIZE <= (uint8_t) -1, window_size_greater_than_underlying_type);

void gyroDataAnalyseStateInit(
    gyroAnalyseState_t *state, 
    uint16_t minFrequency,
    uint32_t targetLooptimeUs
);
void gyroDataAnalysePush(gyroAnalyseState_t *gyroAnalyse, int axis, float sample);
void gyroDataAnalyse(gyroAnalyseState_t *gyroAnalyse);
#endif