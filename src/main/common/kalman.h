/*
 * This file is part of INAV.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
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
void kalmanUpdate(kalmanState_t *filter, float input, float inputRate, float dT);
void processKalmanAttitude(void);