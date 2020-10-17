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

#include "sensors/gyro.h"
#include "common/filter.h"

#define MAX_KALMAN_WINDOW_SIZE 512

#define VARIANCE_SCALE 0.67f

typedef struct kalman
{
    float q;     //process noise covariance
    float r;     //measurement noise covariance
    float p;     //estimation error covariance matrix
    float k;     //kalman gain
    float x;     //state
    float lastX; //previous state
    float e;
    float s;

    float axisVar;
    uint16_t windex;
    float axisWindow[MAX_KALMAN_WINDOW_SIZE];
    float varianceWindow[MAX_KALMAN_WINDOW_SIZE];
    float axisSumMean;
    float axisMean;
    float axisSumVar;
    float inverseN;
    uint16_t w;
} kalman_t;

void gyroKalmanInitialize(uint16_t q, uint16_t w, uint16_t sharpness);
float gyroKalmanUpdate(uint8_t axis, float input, float setpoint);
