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

#include <stdint.h>
#include <string.h>
#include <math.h>

#include "build/debug.h"
#include "drivers/time.h"
#include "common/calibration.h"

void zeroCalibrationStartS(zeroCalibrationScalar_t * s, timeMs_t window, float threshold, bool allowFailure)
{
    // Reset parameters and state
    s->params.state = ZERO_CALIBRATION_IN_PROGRESS;
    s->params.startTimeMs = millis();
    s->params.windowSizeMs = window;
    s->params.stdDevThreshold = threshold;
    s->params.allowFailure = allowFailure;

    s->params.sampleCount = 0;
    s->val.accumulatedValue = 0;
    devClear(&s->val.stdDev);
}

bool zeroCalibrationIsCompleteS(zeroCalibrationScalar_t * s)
{
    return !(s->params.state == ZERO_CALIBRATION_IN_PROGRESS);
}

bool zeroCalibrationIsSuccessfulS(zeroCalibrationScalar_t * s)
{
    return (s->params.state == ZERO_CALIBRATION_DONE);
}

void zeroCalibrationAddValueS(zeroCalibrationScalar_t * s, const float v)
{
    if (s->params.state != ZERO_CALIBRATION_IN_PROGRESS) {
        return;
    }

    // Add value
    s->val.accumulatedValue += v;
    s->params.sampleCount++;
    devPush(&s->val.stdDev, v);

    // Check if calibration is complete
    if ((millis() - s->params.startTimeMs) > s->params.windowSizeMs) {
        const float stddev = devStandardDeviation(&s->val.stdDev);
        if (stddev > s->params.stdDevThreshold) {
            if (!s->params.allowFailure) {
                // If deviation is too big - restart calibration
                s->params.startTimeMs = millis();
                s->params.sampleCount = 0;
                s->val.accumulatedValue = 0;
                devClear(&s->val.stdDev);
            }
            else {
                // We are allowed to fail
                s->params.state = ZERO_CALIBRATION_FAIL;
            }
        }
        else {
            // All seems ok - calculate average value
            s->val.accumulatedValue = s->val.accumulatedValue / s->params.sampleCount;
            s->params.state = ZERO_CALIBRATION_DONE;
        }
    }
}

void zeroCalibrationGetZeroS(zeroCalibrationScalar_t * s, float * v)
{
    if (s->params.state != ZERO_CALIBRATION_DONE) {
        *v = 0.0f;
    }
    else {
        *v = s->val.accumulatedValue;
    }
}

void zeroCalibrationStartV(zeroCalibrationVector_t * s, timeMs_t window, float threshold, bool allowFailure)
{
    // Reset parameters and state
    s->params.state = ZERO_CALIBRATION_IN_PROGRESS;
    s->params.startTimeMs = millis();
    s->params.windowSizeMs = window;
    s->params.stdDevThreshold = threshold;
    s->params.allowFailure = allowFailure;

    s->params.sampleCount = 0;
    for (int i = 0; i < 3; i++) {
        s->val[i].accumulatedValue = 0;
        devClear(&s->val[i].stdDev);
    }
}

bool zeroCalibrationIsCompleteV(zeroCalibrationVector_t * s)
{
    return !(s->params.state == ZERO_CALIBRATION_IN_PROGRESS);
}

bool zeroCalibrationIsSuccessfulV(zeroCalibrationVector_t * s)
{
    return (s->params.state == ZERO_CALIBRATION_DONE);
}

void zeroCalibrationAddValueV(zeroCalibrationVector_t * s, const fpVector3_t * v)
{
    if (s->params.state != ZERO_CALIBRATION_IN_PROGRESS) {
        return;
    }

    // Add value
    for (int i = 0; i < 3; i++) {
        s->val[i].accumulatedValue += v->v[i];
        devPush(&s->val[i].stdDev, v->v[i]);
    }

    s->params.sampleCount++;

    // Check if calibration is complete
    if ((millis() - s->params.startTimeMs) > s->params.windowSizeMs) {
        bool needRecalibration = false;
        
        for (int i = 0; i < 3 && !needRecalibration; i++) {
            const float stddev = devStandardDeviation(&s->val[i].stdDev);
            //debug[i] = stddev;
            if (stddev > s->params.stdDevThreshold) {
                needRecalibration = true;
            }
        }

        if (needRecalibration) {
            if (!s->params.allowFailure) {
                // If deviation is too big - restart calibration
                s->params.startTimeMs = millis();
                s->params.sampleCount = 0;
                for (int i = 0; i < 3; i++) {
                    s->val[i].accumulatedValue = 0;
                    devClear(&s->val[i].stdDev);
                }
            }
            else {
                // We are allowed to fail
                s->params.state = ZERO_CALIBRATION_FAIL;
            }
        }
        else {
            // All seems ok - calculate average value
            s->val[0].accumulatedValue = s->val[0].accumulatedValue / s->params.sampleCount;
            s->val[1].accumulatedValue = s->val[1].accumulatedValue / s->params.sampleCount;
            s->val[2].accumulatedValue = s->val[2].accumulatedValue / s->params.sampleCount;
            s->params.state = ZERO_CALIBRATION_DONE;
        }
    }
}

void zeroCalibrationGetZeroV(zeroCalibrationVector_t * s, fpVector3_t * v)
{
    if (s->params.state != ZERO_CALIBRATION_DONE) {
        vectorZero(v);
    }
    else {
        v->v[0] = s->val[0].accumulatedValue;
        v->v[1] = s->val[1].accumulatedValue;
        v->v[2] = s->val[2].accumulatedValue;
    }    
}
