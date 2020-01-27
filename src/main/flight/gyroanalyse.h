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

// max for F3 targets
#define FFT_WINDOW_SIZE 32

#define DYNAMIC_NOTCH_DEFAULT_CENTER_HZ 350
#define DYNAMIC_NOTCH_DEFAULT_CUTOFF_HZ 300

typedef struct gyroAnalyseState_s {
    // accumulator for oversampled data => no aliasing and less noise
    uint8_t sampleCount;
    uint8_t maxSampleCount;
    float maxSampleCountRcp;
    float oversampledGyroAccumulator[XYZ_AXIS_COUNT];

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

    filterApplyFnPtr notchFilterDynApplyFn;
    filterApplyFnPtr notchFilterDynApplyFn2;
    biquadFilter_t notchFilterDyn[XYZ_AXIS_COUNT];
    biquadFilter_t notchFilterDyn2[XYZ_AXIS_COUNT];

    /*
     * Extended Dynamic Filtets are 3x3 filter matrix
     * In this approach, we assume that vibration peak on one axis
     * can be also detected on other axises, but with lower amplitude
     * that causes this freqency not to be attenuated.
     * 
     * This approach is similiar to the approach on RPM filter when motor base
     * frequency is attenuated on every axis even tho it might not be appearing
     * in gyro traces
     * 
     * extendedDynamicFilter[GYRO_AXIS][ANALYZED_AXIS]
     * 
     */
    biquadFilter_t extendedDynamicFilter[XYZ_AXIS_COUNT][XYZ_AXIS_COUNT];
    filterApplyFnPtr extendedDynamicFilterApplyFn;
} gyroAnalyseState_t;

STATIC_ASSERT(FFT_WINDOW_SIZE <= (uint8_t) -1, window_size_greater_than_underlying_type);

void gyroDataAnalyseStateInit(gyroAnalyseState_t *gyroAnalyse, uint32_t targetLooptime);
void gyroDataAnalysePush(gyroAnalyseState_t *gyroAnalyse, int axis, float sample);
void gyroDataAnalyse(gyroAnalyseState_t *gyroAnalyse);
uint16_t getMaxFFT(void);
void resetMaxFFT(void);
void dynamicFiltersInit(gyroAnalyseState_t *gyroAnalyse);
float dynamicFiltersApply(gyroAnalyseState_t *gyroAnalyse, uint8_t axis, float input);
#endif