/*
 * This file is part of INAV.
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

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include "platform.h"

#include "blackbox/blackbox.h"

#include "build/debug.h"

#include "common/maths.h"
#include "common/filter.h"

#include "drivers/time.h"

#include "rx/rx.h"

#include "fc/config.h"
#include "fc/fc_core.h"
#include "fc/rc_controls.h"
#include "fc/rc_smoothing.h"
#include "fc/runtime_config.h"

#include "flight/mixer.h"

// RC Interpolation is not allowed to go below this value.
#define RC_INTERPOLATION_MIN_FREQUENCY 15
#define RC_FILTER_AVERAGE_SIZE 9

// average filter
static uint8_t sampleIndex;                           // pointer to the next empty slot in the buffer
static uint8_t numSamples;       	                  // the number of samples in the filter, maxes out at size of the filter
static int32_t bufferSamples[RC_FILTER_AVERAGE_SIZE]; // buffer of samples

// LPF
static pt3Filter_t rcSmoothFilter[4];
static float rcStickUnfiltered[4];
static uint16_t rcUpdateFrequency;

uint16_t getRcUpdateFrequency(void) {
    return rcUpdateFrequency;
}

static int32_t applyRcUpdateFrequencyMedianFilter(int32_t sample)
{
	float result = 0.0f;

	// add sample to array
	bufferSamples[sampleIndex++] = sample;

	// wrap index if necessary
	if (sampleIndex >= RC_FILTER_AVERAGE_SIZE)
		sampleIndex = 0;

	// increment the number of samples so far
	numSamples++;
	if (numSamples > RC_FILTER_AVERAGE_SIZE || numSamples == 0)
		numSamples = RC_FILTER_AVERAGE_SIZE;

	// get sum of all values
	for (uint8_t i = 0; i < RC_FILTER_AVERAGE_SIZE; i++) {
		result += bufferSamples[i];
	}

	return (int32_t)(result / numSamples);
}

void rcInterpolationApply(bool isRXDataNew, timeUs_t currentTimeUs)
{
    // Compute the RC update frequency
    static timeUs_t previousRcData;
    static int filterFrequency;
    static bool initDone = false;

    const float dT = US2S(getLooptime());

    if (isRXDataNew) {
        if (!initDone) {

            filterFrequency = rxConfig()->rcFilterFrequency;

            // Initialize the RC smooth filter
            for (int stick = 0; stick < 4; stick++) {
                pt3FilterInit(&rcSmoothFilter[stick], pt3FilterGain(filterFrequency, dT));
            }

            initDone = true;
        }

        for (int stick = 0; stick < 4; stick++) {
            rcStickUnfiltered[stick] = rcCommand[stick];
        }
    }

    // Don't filter if not initialized
    if (!initDone) {
        return;
    }

    if (isRXDataNew) {
        const timeDelta_t delta = cmpTimeUs(currentTimeUs, previousRcData);
        rcUpdateFrequency = applyRcUpdateFrequencyMedianFilter(1.0f / (delta * 0.000001f));
        previousRcData = currentTimeUs;

        /*
         * If auto smoothing is enabled, update the filters
         */
        if (rxConfig()->autoSmooth) {
            const int nyquist = rcUpdateFrequency / 2;

            int newFilterFrequency = scaleRange(
                rxConfig()->autoSmoothFactor,
                1,
                100,
                nyquist,
                rcUpdateFrequency / 10
            );
            
            // Do not allow filter frequency to go below RC_INTERPOLATION_MIN_FREQUENCY or above nuyquist frequency.
            newFilterFrequency = constrain(newFilterFrequency, RC_INTERPOLATION_MIN_FREQUENCY, nyquist);

            if (newFilterFrequency != filterFrequency) {

                for (int stick = 0; stick < 4; stick++) {
                    pt3FilterUpdateCutoff(&rcSmoothFilter[stick], pt3FilterGain(newFilterFrequency, dT));
                }
                filterFrequency = newFilterFrequency;
            }
        }

    }

    for (int stick = 0; stick < 4; stick++) {
        rcCommand[stick] = pt3FilterApply(&rcSmoothFilter[stick], rcStickUnfiltered[stick]);
    }
}
