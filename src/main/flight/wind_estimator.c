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

#if defined(USE_WIND_ESTIMATOR)

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "build/build_config.h"
#include "build/debug.h"

#include "common/axis.h"
#include "common/filter.h"
#include "common/maths.h"

#include "drivers/time.h"

#include "fc/config.h"
#include "fc/runtime_config.h"

#include "flight/imu.h"

#include "navigation/navigation_pos_estimator_private.h"

#include "io/gps.h"

#include "navigation/navigation_pos_estimator_private.h"

#define WINDESTIMATOR_TIMEOUT       60*15 // 15min with out altitude change
#define WINDESTIMATOR_ALTITUDE_SCALE WINDESTIMATOR_TIMEOUT/500.0f //or 500m altitude change
// Based on WindEstimation.pdf paper

static bool hasValidWindEstimate = false;
static float estimatedWind[XYZ_AXIS_COUNT] = {0, 0, 0};    // wind velocity vectors in cm / sec in earth frame
static float lastGroundVelocity[XYZ_AXIS_COUNT];
static float lastFuselageDirection[XYZ_AXIS_COUNT];

bool isEstimatedWindSpeedValid(void)
{
    return hasValidWindEstimate 
#ifdef USE_GPS_FIX_ESTIMATION
        || STATE(GPS_ESTIMATED_FIX)  //use any wind estimate with GPS fix estimation.
#endif
        ;
}

float getEstimatedWindSpeed(int axis)
{
    return estimatedWind[axis];
}

float getEstimatedHorizontalWindSpeed(uint16_t *angle)
{
    float xWindSpeed = getEstimatedWindSpeed(X);
    float yWindSpeed = getEstimatedWindSpeed(Y);
    if (angle) {
        float horizontalWindAngle = atan2_approx(yWindSpeed, xWindSpeed);
        // atan2 returns [-M_PI, M_PI], with 0 indicating the vector points in the X direction
        // We want [0, 360) in degrees
        if (horizontalWindAngle < 0) {
            horizontalWindAngle += 2 * M_PIf;
        }
        *angle = RADIANS_TO_CENTIDEGREES(horizontalWindAngle);
    }
    return calc_length_pythagorean_2D(xWindSpeed, yWindSpeed);
}

void updateWindEstimator(timeUs_t currentTimeUs)
{
    static timeUs_t lastUpdateUs = 0;
    static timeUs_t lastValidWindEstimate = 0;
    static float lastValidEstimateAltitude = 0.0f;
    float currentAltitude = gpsSol.llh.alt / 100.0f; // altitude in m

    if ((US2S(currentTimeUs - lastValidWindEstimate) + WINDESTIMATOR_ALTITUDE_SCALE * fabsf(currentAltitude - lastValidEstimateAltitude)) > WINDESTIMATOR_TIMEOUT) {
        hasValidWindEstimate = false;
    }

    if (!STATE(FIXED_WING_LEGACY) ||
        !isGPSHeadingValid() ||
        !gpsSol.flags.validVelNE ||
        !gpsSol.flags.validVelD 
#ifdef USE_GPS_FIX_ESTIMATION
            || STATE(GPS_ESTIMATED_FIX)
#endif
            ) {
        return;
    }

    float groundVelocity[XYZ_AXIS_COUNT];
    float groundVelocityDiff[XYZ_AXIS_COUNT];
    float groundVelocitySum[XYZ_AXIS_COUNT];

    float fuselageDirection[XYZ_AXIS_COUNT];
    float fuselageDirectionDiff[XYZ_AXIS_COUNT];
    float fuselageDirectionSum[XYZ_AXIS_COUNT];

    // Get current 3D velocity from poisition estimator in cm/s
    // relative to earth frame
    groundVelocity[X] = posEstimator.gps.vel.x;
    groundVelocity[Y] = posEstimator.gps.vel.y;
    groundVelocity[Z] = posEstimator.gps.vel.z;

    // Fuselage direction in earth frame
    fuselageDirection[X] = HeadVecEFFiltered.x;
    fuselageDirection[Y] = -HeadVecEFFiltered.y;
    fuselageDirection[Z] = -HeadVecEFFiltered.z;

    timeDelta_t timeDelta = cmpTimeUs(currentTimeUs, lastUpdateUs);
    // scrap our data and start over if we're taking too long to get a direction change
    if (lastUpdateUs == 0 || timeDelta > 10 * USECS_PER_SEC) {

        lastUpdateUs = currentTimeUs;

        memcpy(lastFuselageDirection, fuselageDirection, sizeof(lastFuselageDirection));
        memcpy(lastGroundVelocity, groundVelocity, sizeof(lastGroundVelocity));
        return;
    }

    fuselageDirectionDiff[X] = fuselageDirection[X] - lastFuselageDirection[X];
    fuselageDirectionDiff[Y] = fuselageDirection[Y] - lastFuselageDirection[Y];
    fuselageDirectionDiff[Z] = fuselageDirection[Z] - lastFuselageDirection[Z];

    float diffLengthSq = sq(fuselageDirectionDiff[X]) + sq(fuselageDirectionDiff[Y]) + sq(fuselageDirectionDiff[Z]);
    
    // Very small changes in attitude will result in a denominator
    // very close to zero which will introduce too much error in the
    // estimation.
    //
    // TODO: Is 0.2f an adequate threshold?
    if (diffLengthSq > sq(0.2f)) {
        // when turning, use the attitude response to estimate wind speed
        groundVelocityDiff[X] = groundVelocity[X] - lastGroundVelocity[X];
        groundVelocityDiff[Y] = groundVelocity[Y] - lastGroundVelocity[Y];
        groundVelocityDiff[Z] = groundVelocity[Z] - lastGroundVelocity[Z];

        // estimate airspeed it using equation 6
        float V = (calc_length_pythagorean_3D(groundVelocityDiff[X], groundVelocityDiff[Y], groundVelocityDiff[Z])) / fast_fsqrtf(diffLengthSq);

        fuselageDirectionSum[X] = fuselageDirection[X] + lastFuselageDirection[X];
        fuselageDirectionSum[Y] = fuselageDirection[Y] + lastFuselageDirection[Y];
        fuselageDirectionSum[Z] = fuselageDirection[Z] + lastFuselageDirection[Z];

        groundVelocitySum[X] = groundVelocity[X] + lastGroundVelocity[X];
        groundVelocitySum[Y] = groundVelocity[Y] + lastGroundVelocity[Y];
        groundVelocitySum[Z] = groundVelocity[Z] + lastGroundVelocity[Z];

        memcpy(lastFuselageDirection, fuselageDirection, sizeof(lastFuselageDirection));
        memcpy(lastGroundVelocity, groundVelocity, sizeof(lastGroundVelocity));

        float theta = atan2f(groundVelocityDiff[Y], groundVelocityDiff[X]) - atan2f(fuselageDirectionDiff[Y], fuselageDirectionDiff[X]);// equation 9
        float sintheta = sinf(theta);
        float costheta = cosf(theta);

        float wind[XYZ_AXIS_COUNT];
        wind[X] = (groundVelocitySum[X] - V * (costheta * fuselageDirectionSum[X] - sintheta * fuselageDirectionSum[Y])) * 0.5f;// equation 10
        wind[Y] = (groundVelocitySum[Y] - V * (sintheta * fuselageDirectionSum[X] + costheta * fuselageDirectionSum[Y])) * 0.5f;// equation 11
        wind[Z] = (groundVelocitySum[Z] - V * fuselageDirectionSum[Z]) * 0.5f;// equation 12

        float prevWindLength = calc_length_pythagorean_3D(estimatedWind[X], estimatedWind[Y], estimatedWind[Z]);
        float windLength = calc_length_pythagorean_3D(wind[X], wind[Y], wind[Z]);

        //is this really needed? The reason it is here might be above equation was wrong in early implementations
        if (windLength < prevWindLength + 4000) {
            // TODO: Better filtering
            estimatedWind[X] = estimatedWind[X] * 0.98f + wind[X] * 0.02f;
            estimatedWind[Y] = estimatedWind[Y] * 0.98f + wind[Y] * 0.02f;
            estimatedWind[Z] = estimatedWind[Z] * 0.98f + wind[Z] * 0.02f;
        }

        lastUpdateUs = currentTimeUs;
        lastValidWindEstimate = currentTimeUs;
        hasValidWindEstimate = true;
        lastValidEstimateAltitude = currentAltitude;
    }
}

#endif
