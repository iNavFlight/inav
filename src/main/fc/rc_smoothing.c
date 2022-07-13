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

static pt3Filter_t rcSmoothFilter[4];
static float rcStickUnfiltered[4];
static uint16_t rcUpdateFrequency;

uint16_t getRcUpdateFrequency(void) {
    return rcUpdateFrequency;
}

static int32_t applyRcUpdateFrequencyMedianFilter(int32_t newReading)
{
    #define RC_FILTER_SAMPLES_MEDIAN 9
    static int32_t filterSamples[RC_FILTER_SAMPLES_MEDIAN];
    static int filterSampleIndex = 0;
    static bool medianFilterReady = false;

    filterSamples[filterSampleIndex] = newReading;
    ++filterSampleIndex;
    if (filterSampleIndex == RC_FILTER_SAMPLES_MEDIAN) {
        filterSampleIndex = 0;
        medianFilterReady = true;
    }

    return medianFilterReady ? quickMedianFilter9(filterSamples) : newReading;
}

void rcInterpolationApply(bool isRXDataNew, timeUs_t currentTimeUs)
{
    // Compute the RC update frequency
    static timeUs_t previousRcData;
    static int filterFrequency;
    static bool initDone = false;

    const float dT = getLooptime() * 1e-6f;

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
