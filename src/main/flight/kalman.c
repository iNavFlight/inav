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
#include "platform.h"
#ifdef USE_GYRO_KALMAN

FILE_COMPILE_FOR_SPEED

#include <string.h>
#if !defined(SITL_BUILD)
#include "arm_math.h"
#else
#include <math.h>
#endif

#include "kalman.h"
#include "build/debug.h"

kalman_t kalmanFilterStateRate[XYZ_AXIS_COUNT];

static void gyroKalmanInitAxis(kalman_t *filter, uint16_t q)
{
    memset(filter, 0, sizeof(kalman_t));
    filter->q = q * 0.03f; //add multiplier to make tuning easier
    filter->r = 88.0f;      //seeding R at 88.0f
    filter->p = 30.0f;      //seeding P at 30.0f
    filter->e = 1.0f;
    filter->w = MAX_KALMAN_WINDOW_SIZE;         
    filter->inverseN = 1.0f / (float)(filter->w);
}

void gyroKalmanInitialize(uint16_t q)
{
    gyroKalmanInitAxis(&kalmanFilterStateRate[X], q);
    gyroKalmanInitAxis(&kalmanFilterStateRate[Y], q);
    gyroKalmanInitAxis(&kalmanFilterStateRate[Z], q);
}

float kalman_process(kalman_t *kalmanState, float input)
{
    //project the state ahead using acceleration
    kalmanState->x += (kalmanState->x - kalmanState->lastX);

    //update last state
    kalmanState->lastX = kalmanState->x;

    if (kalmanState->lastX != 0.0f)
    {
        kalmanState->e = fabsf(1.0f - (kalmanState->setpoint / kalmanState->lastX));
    }

    //prediction update
    kalmanState->p = kalmanState->p + (kalmanState->q * kalmanState->e);

    //measurement update
    kalmanState->k = kalmanState->p / (kalmanState->p + kalmanState->r);
    kalmanState->x += kalmanState->k * (input - kalmanState->x);
    kalmanState->p = (1.0f - kalmanState->k) * kalmanState->p;
    return kalmanState->x;
}

static void updateAxisVariance(kalman_t *kalmanState, float rate)
{
    kalmanState->axisWindow[kalmanState->windex] = rate;

    kalmanState->axisSumMean += kalmanState->axisWindow[kalmanState->windex];
    float varianceElement = kalmanState->axisWindow[kalmanState->windex] - kalmanState->axisMean;
    varianceElement = varianceElement * varianceElement;
    kalmanState->axisSumVar += varianceElement;
    kalmanState->varianceWindow[kalmanState->windex] = varianceElement;

    kalmanState->windex++;
    if (kalmanState->windex > kalmanState->w) {
        kalmanState->windex = 0;
    }

    kalmanState->axisSumMean -= kalmanState->axisWindow[kalmanState->windex];
    kalmanState->axisSumVar -= kalmanState->varianceWindow[kalmanState->windex];

    //New mean
    kalmanState->axisMean = kalmanState->axisSumMean * kalmanState->inverseN;
    kalmanState->axisVar = kalmanState->axisSumVar * kalmanState->inverseN;

#if !defined(SITL_BUILD)
    float squirt;
    arm_sqrt_f32(kalmanState->axisVar, &squirt);
#else
    float squirt = sqrtf(kalmanState->axisVar);
#endif
    
    kalmanState->r = squirt * VARIANCE_SCALE;
}

float NOINLINE gyroKalmanUpdate(uint8_t axis, float input)
{
    updateAxisVariance(&kalmanFilterStateRate[axis], input);
    return kalman_process(&kalmanFilterStateRate[axis], input);
}

void gyroKalmanUpdateSetpoint(uint8_t axis, float setpoint) {
    kalmanFilterStateRate[axis].setpoint = setpoint;
}

#endif