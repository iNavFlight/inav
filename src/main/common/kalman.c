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

#include <stdint.h>
#include <stdbool.h>

#include "build/build_config.h"
#include "common/kalman.h"
#include "common/axis.h"

static EXTENDED_FASTRAM kalmanState_t attitudeKalman[2];

void kalmanInit(kalmanState_t *filter, float qValue, float qRateBias, float r) {
    filter->Q[KALMAN_COVARIANCE_VALUE] = qValue;
    filter->Q[KALMAN_COVARIANCE_RATE_BIAS] = qRateBias;
    filter->R = r;

    filter->bias = 0.0f;
    filter->out = 0.0f;
    
    for (uint8_t i = 0; i < KALMAN_MATRIX_SIZE; i++) {
        for (uint8_t j = 0; j < KALMAN_MATRIX_SIZE; j++) {
            filter->P[i][j] = 0.0f;
        }    
    }
};

void kalmanUpdate(kalmanState_t *filter, float input, float inputRate, float dT) {
    filter->rate = inputRate - filter->bias;
    filter->out += filter->rate * dT;

    filter->P[0][0] += dT * (dT * filter->P[1][1] - filter->P[0][1] - filter->P[1][0] + filter->Q[KALMAN_COVARIANCE_VALUE]);
    filter->P[0][1] -= dT * filter->P[1][1];
    filter->P[1][0] -= dT * filter->P[1][1];
    filter->P[1][1] += filter->Q[KALMAN_COVARIANCE_RATE_BIAS] * dT;

    const float estimateError = filter->P[0][0] + filter->R;
    float kalmanGain[2];
    kalmanGain[0] = filter->P[0][0] / estimateError;
    kalmanGain[1] = filter->P[1][0] / estimateError;

    const float outputDelta = input - filter->out;
    const float temp00 = filter->P[0][0];
    const float temp01 = filter->P[0][1];

    filter->out += kalmanGain[0] * outputDelta;
    filter->bias += kalmanGain[1] * outputDelta;

    filter->P[0][0] -= kalmanGain[0] * temp00;
    filter->P[0][1] -= kalmanGain[0] * temp01;
    filter->P[1][0] -= kalmanGain[1] * temp00;
    filter->P[1][1] -= kalmanGain[1] * temp01;
}

void processKalmanAttitude(void)
{
    static bool initialized = false;

    if (!initialized) {
        initialized = true;
        kalmanInit(&attitudeKalman[FD_ROLL], 0.001f, 0.003f, 0.003f);
        kalmanInit(&attitudeKalman[FD_PITCH], 0.001f, 0.003f, 0.003f);
    }
    //TODO put the Kalman usage over here....
}