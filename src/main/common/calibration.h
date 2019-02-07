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

#pragma once 

#include <stdint.h>
#include <string.h>
#include <math.h>

#include "common/maths.h"
#include "common/time.h"
#include "common/vector.h"

typedef enum {
    ZERO_CALIBRATION_NONE,
    ZERO_CALIBRATION_IN_PROGRESS,
    ZERO_CALIBRATION_DONE,
    ZERO_CALIBRATION_FAIL,
} zeroCalibrationState_e;

typedef struct {
    zeroCalibrationState_e  state;
    timeMs_t                startTimeMs;
    timeMs_t                windowSizeMs;
    bool                    allowFailure;
    unsigned                sampleCount;
    float                   stdDevThreshold;
} zeroCalibrationParams_t;

typedef struct {
    float                   accumulatedValue;
    stdev_t                 stdDev;
} zeroCalibrationValue_t;

typedef struct {
    zeroCalibrationParams_t params;
    zeroCalibrationValue_t  val;
} zeroCalibrationScalar_t;

typedef struct {
    zeroCalibrationParams_t params;
    zeroCalibrationValue_t  val[3];
} zeroCalibrationVector_t;

void zeroCalibrationStartS(zeroCalibrationScalar_t * s, timeMs_t window, float threshold, bool allowFailure);
bool zeroCalibrationIsCompleteS(zeroCalibrationScalar_t * s);
bool zeroCalibrationIsSuccessfulS(zeroCalibrationScalar_t * s);
void zeroCalibrationAddValueS(zeroCalibrationScalar_t * s, const float v);
void zeroCalibrationGetZeroS(zeroCalibrationScalar_t * s, float * v);

void zeroCalibrationStartV(zeroCalibrationVector_t * s, timeMs_t window, float threshold, bool allowFailure);
bool zeroCalibrationIsCompleteV(zeroCalibrationVector_t * s);
bool zeroCalibrationIsSuccessfulV(zeroCalibrationVector_t * s);
void zeroCalibrationAddValueV(zeroCalibrationVector_t * s, const fpVector3_t * v);
void zeroCalibrationGetZeroV(zeroCalibrationVector_t * s, fpVector3_t * v);
