/*
 * This file is part of Cleanflight.
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

#include "platform.h"

#if defined(USE_NAV) && defined(USE_WIND_ESTIMATOR)
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "build/build_config.h"

#include "common/axis.h"
#include "common/filter.h"
#include "common/maths.h"

#include "drivers/time.h"

#include "fc/config.h"
#include "fc/runtime_config.h"

#include "flight/imu.h"

#include "io/gps.h"


static bool hasValidWindEstimate = false;
static float estimatedWind[XYZ_AXIS_COUNT] = {0, 0, 0};    // wind velocity vectors in cm / sec
static float lastGroundVelocity[XYZ_AXIS_COUNT];
static float lastFuselageDirection[XYZ_AXIS_COUNT];

float getEstimatedWindVelocity(int axis)
{
    return estimatedWind[axis];
}

bool getEstimatedWindVelocityBodyFrame(float *horizontalSpeed, float *horizontalAngle, float *verticalSpeed)
{
    // This is intended to be used by the OSD to show the wind
    // horizontal speed and direction as well as vertical wind
    // speed in order to detect thermals.
    // TODO: Should just rotate on YAW?
    fpVector3_t wind = {
        .x = estimatedWind[X],
        .y = estimatedWind[Y],
        .z = estimatedWind[Z],
    };

    imuTransformVectorEarthToBody(&wind);

    if (horizontalSpeed) {
        *horizontalSpeed = sqrtf(sq(wind.x) + sq(wind.y));
    }

    if (horizontalAngle) {
        *horizontalAngle = atan2_approx(wind.y, wind.y);
    }

    if (verticalSpeed) {
        *verticalSpeed = wind.z;
    }

    return hasValidWindEstimate;
}

void updateWindEstimator(timeUs_t currentTimeUs)
{
    static timeUs_t lastUpdateUs = 0;

    if (!STATE(FIXED_WING) ||
        !isGPSHeadingValid() ||
        !gpsSol.flags.validVelNE ||
        !gpsSol.flags.validVelD) {
        return;
    }

    float groundVelocity[XYZ_AXIS_COUNT];
    float groundVelocityDiff[XYZ_AXIS_COUNT];
    float groundVelocitySum[XYZ_AXIS_COUNT];

    float fuselageDirection[XYZ_AXIS_COUNT];
    float fuselageDirectionDiff[XYZ_AXIS_COUNT];
    float fuselageDirectionSum[XYZ_AXIS_COUNT];

    groundVelocity[X] = gpsSol.velNED[X];
    groundVelocity[Y] = gpsSol.velNED[Y];
    groundVelocity[Z] = gpsSol.velNED[Z];

    // TODO: Can get this from the nav system without exposing rMat?
    fuselageDirection[X] = rMat[0][0];
    fuselageDirection[Y] = rMat[1][0];
    fuselageDirection[Z] = rMat[2][0];

    timeDelta_t timeDelta = cmpTimeUs(currentTimeUs, lastUpdateUs);
    // scrap our data and start over if we're taking too long to get a direction change
    if (lastUpdateUs == 0 || timeDelta > 2 * USECS_PER_SEC) {

        lastUpdateUs = currentTimeUs;

        memcpy(lastFuselageDirection, fuselageDirection, sizeof(lastFuselageDirection));
        memcpy(lastGroundVelocity, groundVelocity, sizeof(lastGroundVelocity));
        return;
    }

    fuselageDirectionDiff[X] = fuselageDirection[X] - lastFuselageDirection[X];
    fuselageDirectionDiff[Y] = fuselageDirection[Y] - lastFuselageDirection[Y];
    fuselageDirectionDiff[Z] = fuselageDirection[Z] - lastFuselageDirection[Z];

    float diff_length = sqrtf(sq(fuselageDirectionDiff[X]) + sq(fuselageDirectionDiff[Y]) + sq(fuselageDirectionDiff[Z]));
    // TODO: Is 0.2f an adequate threshold?
    if (diff_length > 0.2f) {
        // when turning, use the attitude response to estimate wind speed
        groundVelocityDiff[X] = groundVelocity[X] - lastGroundVelocity[X];
        groundVelocityDiff[Y] = groundVelocity[Y] - lastGroundVelocity[Y];
        groundVelocityDiff[Z] = groundVelocity[X] - lastGroundVelocity[Z];

        // estimate airspeed it using equation 6
        float V = (sqrtf(sq(groundVelocityDiff[0]) + sq(groundVelocityDiff[1]) + sq(groundVelocityDiff[2]))) / diff_length;

        fuselageDirectionSum[X] = fuselageDirection[X] + lastFuselageDirection[X];
        fuselageDirectionSum[Y] = fuselageDirection[Y] + lastFuselageDirection[Y];
        fuselageDirectionSum[Z] = fuselageDirection[Z] + lastFuselageDirection[Z];

        groundVelocitySum[X] = groundVelocity[X] + lastGroundVelocity[X];
        groundVelocitySum[Y] = groundVelocity[Y] + lastGroundVelocity[Y];
        groundVelocitySum[Z] = groundVelocity[Z] + lastGroundVelocity[Z];

        memcpy(lastFuselageDirection, fuselageDirection, sizeof(lastFuselageDirection));
        memcpy(lastGroundVelocity, groundVelocity, sizeof(lastGroundVelocity));

        float theta = atan2f(groundVelocityDiff[1], groundVelocityDiff[0]) - atan2f(groundVelocityDiff[1], groundVelocityDiff[0]);// equation 9
        float sintheta = sinf(theta);
        float costheta = cosf(theta);

        float wind[XYZ_AXIS_COUNT];
        wind[X] = (groundVelocitySum[X] - V * (costheta * fuselageDirectionSum[X] - sintheta * fuselageDirectionSum[Y])) * 0.5f;// equation 10
        wind[Y] = (groundVelocitySum[Y] - V * (sintheta * fuselageDirectionSum[X] + costheta * fuselageDirectionSum[Y])) * 0.5f;// equation 11
        wind[Z] = (groundVelocitySum[Z] - V * fuselageDirectionSum[Z]) * 0.5f;// equation 12

        // TODO: Better filtering
        estimatedWind[X] = estimatedWind[X] * 0.95f + wind[X] * 0.05f;
        estimatedWind[Y] = estimatedWind[Y] * 0.95f + wind[Y] * 0.05f;
        estimatedWind[Z] = estimatedWind[Z] * 0.95f + wind[Z] * 0.05f;

        lastUpdateUs = currentTimeUs;
        hasValidWindEstimate = true;
    }
}

#endif