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
#include "arm_math.h"

#include "kalman.h"
#include "build/debug.h"

kalman_t kalmanFilterStateRate[XYZ_AXIS_COUNT];

static void gyroKalmanInitAxis(kalman_t *filter, uint16_t q, uint16_t w, uint16_t sharpness)
{
    memset(filter, 0, sizeof(kalman_t));
    filter->q = q * 0.03f; //add multiplier to make tuning easier
    filter->r = 88.0f;      //seeding R at 88.0f
    filter->p = 30.0f;      //seeding P at 30.0f
    filter->e = 1.0f;
    filter->s = sharpness / 10.0f;
    filter->w = w * 8;
    filter->inverseN = 1.0f / (float)(filter->w);
}

void gyroKalmanInitialize(uint16_t q, uint16_t w, uint16_t sharpness)
{
    gyroKalmanInitAxis(&kalmanFilterStateRate[X], q, w, sharpness);
    gyroKalmanInitAxis(&kalmanFilterStateRate[Y], q, w, sharpness);
    gyroKalmanInitAxis(&kalmanFilterStateRate[Z], q, w, sharpness);
}

float kalman_process(kalman_t *kalmanState, float input, float target)
{
    float targetAbs = fabsf(target);
    //project the state ahead using acceleration
    kalmanState->x += (kalmanState->x - kalmanState->lastX);

    //figure out how much to boost or reduce our error in the estimate based on setpoint target.
    //this should be close to 0 as we approach the sepoint and really high the futher away we are from the setpoint.
    //update last state
    kalmanState->lastX = kalmanState->x;

    if (kalmanState->lastX != 0.0f)
    {
        // calculate the error and add multiply sharpness boost
        float errorMultiplier = fabsf(target - kalmanState->x) * kalmanState->s;

        // give a boost to the setpoint, used to caluclate the kalman q, based on the error and setpoint/gyrodata
        errorMultiplier = constrainf(errorMultiplier * fabsf(1.0f - (target / kalmanState->lastX)) + 1.0f, 1.0f, 50.0f);

        kalmanState->e = fabsf(1.0f - (((targetAbs + 1.0f) * errorMultiplier) / fabsf(kalmanState->lastX)));
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
    if (kalmanState->windex >= kalmanState->w) {
        kalmanState->windex = 0;
    }

    kalmanState->axisSumMean -= kalmanState->axisWindow[kalmanState->windex];
    kalmanState->axisSumVar -= kalmanState->varianceWindow[kalmanState->windex];

    //New mean
    kalmanState->axisMean = kalmanState->axisSumMean * kalmanState->inverseN;
    kalmanState->axisVar = kalmanState->axisSumVar * kalmanState->inverseN;

    float squirt;
    arm_sqrt_f32(kalmanState->axisVar, &squirt);
    kalmanState->r = squirt * VARIANCE_SCALE;
}

float gyroKalmanUpdate(uint8_t axis, float input, float setpoint)
{
    updateAxisVariance(&kalmanFilterStateRate[axis], input);

    DEBUG_SET(DEBUG_KALMAN_GAIN, axis, kalmanFilterStateRate[axis].k * 1000.0f); //Kalman gain

    return kalman_process(&kalmanFilterStateRate[axis], input, setpoint);
}

#endif