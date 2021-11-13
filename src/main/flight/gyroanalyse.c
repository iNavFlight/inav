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

/* original work by Rav
 * 2018_07 updated by ctzsnooze to post filter, wider Q, different peak detection
 * coding assistance and advice from DieHertz, Rav, eTracer
 * test pilots icr4sh, UAV Tech, Flint723
 */
#include <stdint.h>

#include "platform.h"
FILE_COMPILE_FOR_SPEED

#ifdef USE_DYNAMIC_FILTERS

#include "build/debug.h"

#include "common/filter.h"
#include "common/maths.h"
#include "common/time.h"
#include "common/utils.h"
#include "config/feature.h"

#include "drivers/accgyro/accgyro.h"
#include "drivers/time.h"

#include "sensors/gyro.h"
#include "fc/config.h"

#include "gyroanalyse.h"

enum {
    STEP_ARM_CFFT_F32,
    STEP_BITREVERSAL_AND_STAGE_RFFT_F32,
    STEP_MAGNITUDE_AND_FREQUENCY,
    STEP_UPDATE_FILTERS_AND_HANNING,
    STEP_COUNT
};

// The FFT splits the frequency domain into an number of bins
// A sampling frequency of 1000 and max frequency of 500 at a window size of 32 gives 16 frequency bins each 31.25Hz wide
// Eg [0,31), [31,62), [62, 93) etc
// for gyro loop >= 4KHz, sample rate 2000 defines FFT range to 1000Hz, 16 bins each 62.5 Hz wide
// NB  FFT_WINDOW_SIZE is set to 32 in gyroanalyse.h
#define FFT_BIN_COUNT             (FFT_WINDOW_SIZE / 2)
// smoothing frequency for FFT centre frequency
#define DYN_NOTCH_SMOOTH_FREQ_HZ  35

/*
 * Slow down gyro sample acquisition. This lowers the max frequency but increases the resolution.
 * On default 500us looptime and denominator 1, max frequency is 1000Hz with a resolution of 31.25Hz
 * On default 500us looptime and denominator 2, max frequency is 500Hz with a resolution of 15.6Hz
 */
#define FFT_SAMPLING_DENOMINATOR 2

void gyroDataAnalyseStateInit(
    gyroAnalyseState_t *state, 
    uint16_t minFrequency,
    uint32_t targetLooptimeUs
) {
    state->minFrequency = minFrequency;

    state->fftSamplingRateHz = 1e6f / targetLooptimeUs / FFT_SAMPLING_DENOMINATOR;
    state->maxFrequency = state->fftSamplingRateHz / 2; //max possible frequency is half the sampling rate
    state->fftResolution = (float)state->maxFrequency / FFT_BIN_COUNT;

    state->fftStartBin = state->minFrequency / lrintf(state->fftResolution);

    for (int i = 0; i < FFT_WINDOW_SIZE; i++) {
        state->hanningWindow[i] = (0.5f - 0.5f * cos_approx(2 * M_PIf * i / (FFT_WINDOW_SIZE - 1)));
    }

    arm_rfft_fast_init_f32(&state->fftInstance, FFT_WINDOW_SIZE);

    // Frequency filter is executed every 12 cycles. 4 steps per cycle, 3 axises
    const uint32_t filterUpdateUs = targetLooptimeUs * STEP_COUNT * XYZ_AXIS_COUNT;

    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        
        for (int i = 0; i < DYN_NOTCH_PEAK_COUNT; i++) {
            state->centerFrequency[axis][i] = state->maxFrequency;
            pt2FilterInit(&state->detectedFrequencyFilter[axis][i], pt2FilterGain(DYN_NOTCH_SMOOTH_FREQ_HZ, filterUpdateUs * 1e-6f));
        }

    }
}

void gyroDataAnalysePush(gyroAnalyseState_t *state, const int axis, const float sample)
{
    state->currentSample[axis] = sample;
}

static void gyroDataAnalyseUpdate(gyroAnalyseState_t *state);

/*
 * Collect gyro data, to be analysed in gyroDataAnalyseUpdate function
 */
void gyroDataAnalyse(gyroAnalyseState_t *state)
{
    state->filterUpdateExecute = false; //This will be changed to true only if new data is present

    static uint8_t samplingIndex = 0;

    if (samplingIndex == 0) {
        // calculate mean value of accumulated samples
        for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
            state->downsampledGyroData[axis][state->circularBufferIdx] = state->currentSample[axis];
        }

        state->circularBufferIdx = (state->circularBufferIdx + 1) % FFT_WINDOW_SIZE;
    }

    samplingIndex = (samplingIndex + 1) % FFT_SAMPLING_DENOMINATOR;

    gyroDataAnalyseUpdate(state);
}

void stage_rfft_f32(arm_rfft_fast_instance_f32 *S, float32_t *p, float32_t *pOut);
void arm_cfft_radix8by4_f32(arm_cfft_instance_f32 *S, float32_t *p1);
void arm_bitreversal_32(uint32_t *pSrc, const uint16_t bitRevLen, const uint16_t *pBitRevTable);

static float computeParabolaMean(gyroAnalyseState_t *state, uint8_t peakBinIndex) {
    float preciseBin = peakBinIndex;

    // Height of peak bin (y1) and shoulder bins (y0, y2)
    const float y0 = state->fftData[peakBinIndex - 1];
    const float y1 = state->fftData[peakBinIndex];
    const float y2 = state->fftData[peakBinIndex - 1];

    // Estimate true peak position aka. preciseBin (fit parabola y(x) over y0, y1 and y2, solve dy/dx=0 for x)
    const float denom = 2.0f * (y0 - 2 * y1 + y2);
    if (denom != 0.0f) {
        preciseBin += (y0 - y2) / denom;
    }

    return preciseBin;
}

/*
 * Analyse last gyro data from the last FFT_WINDOW_SIZE milliseconds
 */
static NOINLINE void gyroDataAnalyseUpdate(gyroAnalyseState_t *state)
{

    arm_cfft_instance_f32 *Sint = &(state->fftInstance.Sint);

    switch (state->updateStep) {
        case STEP_ARM_CFFT_F32:
        {
            // Important this works only with FFT windows size of 64 elements!
            arm_cfft_radix8by4_f32(Sint, state->fftData);
            break;
        }
        case STEP_BITREVERSAL_AND_STAGE_RFFT_F32:
        {
            arm_bitreversal_32((uint32_t*) state->fftData, Sint->bitRevLength, Sint->pBitRevTable);
            stage_rfft_f32(&state->fftInstance, state->fftData, state->rfftData);
            break;
        }
        case STEP_MAGNITUDE_AND_FREQUENCY:
        {
            // 8us
            arm_cmplx_mag_f32(state->rfftData, state->fftData, FFT_BIN_COUNT);

            //Compute mean - needed to work with only peaks above the noise floor
            state->fftMeanValue = 0;
            for (int bin = (state->fftStartBin + 1); bin < FFT_BIN_COUNT; bin++) {   
                state->fftMeanValue += state->fftData[bin];
            }
            state->fftMeanValue /= FFT_BIN_COUNT - state->fftStartBin - 1;

            //Zero the data structure
            for (int i = 0; i < DYN_NOTCH_PEAK_COUNT; i++) {
                state->peaks[i].bin = 0;
                state->peaks[i].value = 0.0f;
            }

            // Find peaks
            for (int bin = (state->fftStartBin + 1); bin < FFT_BIN_COUNT - 1; bin++) {
                /*
                 * Peak is defined if the current bin is greater than the previous bin and the next bin
                 */
                if (
                    state->fftData[bin] > state->fftMeanValue && 
                    state->fftData[bin] > state->fftData[bin - 1] && 
                    state->fftData[bin] > state->fftData[bin + 1]
                ) {
                    /*
                     * We are only interested in N biggest peaks
                     * Check previously found peaks and update the structure if necessary
                     */
                    for (int p = 0; p < DYN_NOTCH_PEAK_COUNT; p++) {
                        if (state->fftData[bin] > state->peaks[p].value) {
                            for (int k = DYN_NOTCH_PEAK_COUNT - 1; k > p; k--) {
                                state->peaks[k] = state->peaks[k - 1];
                            }
                            state->peaks[p].bin = bin;
                            state->peaks[p].value = state->fftData[bin];
                            break;
                        }
                    }
                    bin++; // If bin is peak, next bin can't be peak => jump it
                }
            }

            // Sort N biggest peaks in ascending bin order (example: 3, 8, 25, 0, 0, ..., 0)
            for (int p = DYN_NOTCH_PEAK_COUNT - 1; p > 0; p--) {
                for (int k = 0; k < p; k++) {
                    // Swap peaks but ignore swapping void peaks (bin = 0). This leaves
                    // void peaks at the end of peaks array without moving them
                    if (state->peaks[k].bin > state->peaks[k + 1].bin && state->peaks[k + 1].bin != 0) {
                        peak_t temp = state->peaks[k];
                        state->peaks[k] = state->peaks[k + 1];
                        state->peaks[k + 1] = temp;
                    }
                }
            }

            break;
        }
        case STEP_UPDATE_FILTERS_AND_HANNING:
        {

            /*
             * Update frequencies
             */
            for (int i = 0; i < DYN_NOTCH_PEAK_COUNT; i++) {

                if (state->peaks[i].bin > 0) {
                    const int bin = constrain(state->peaks[i].bin, state->fftStartBin, FFT_BIN_COUNT - 1);
                    float frequency = computeParabolaMean(state, bin) * state->fftResolution;

                    state->centerFrequency[state->updateAxis][i] = pt2FilterApply(&state->detectedFrequencyFilter[state->updateAxis][i], frequency);
                } else {
                    state->centerFrequency[state->updateAxis][i] = 0.0f;
                }
            }

            /*
             * Filters will be updated inside dynamicGyroNotchFiltersUpdate()
             */
            state->filterUpdateExecute = true;
            state->filterUpdateAxis = state->updateAxis;

            //Switch to the next axis
            state->updateAxis = (state->updateAxis + 1) % XYZ_AXIS_COUNT;
            
            // apply hanning window to gyro samples and store result in fftData
            // hanning starts and ends with 0, could be skipped for minor speed improvement
            arm_mult_f32(state->downsampledGyroData[state->updateAxis], state->hanningWindow, state->fftData, FFT_WINDOW_SIZE);
        }
    }

    state->updateStep = (state->updateStep + 1) % STEP_COUNT;
}

#endif // USE_DYNAMIC_FILTERS
