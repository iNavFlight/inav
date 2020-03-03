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

#define KALMAN_MATRIX_SIZE 2

typedef enum {
    KALMAN_COVARIANCE_VALUE = 0,
    KALMAN_COVARIANCE_RATE_BIAS,
    KALMAN_COVARIANCE_MAX
} kalmanCovarianceTypes_t;

typedef struct kalmanState_s {
    float P[2][2];
    float Q[KALMAN_COVARIANCE_MAX];
    float R;
    float out;
    float bias;
    float rate;
} kalmanState_t;

void kalmanInit(kalmanState_t *filter, float qValue, float qRateBias, float r);
void kalmanUpdate(kalmanState_t *filter, float inputAngle, float inputRate, float dT);