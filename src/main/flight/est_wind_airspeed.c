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

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "build/build_config.h"

#include "platform.h"

#include "common/axis.h"
#include "common/filter.h"
#include "common/maths.h"

#include "drivers/time.h"

#include "fc/config.h"
#include "fc/runtime_config.h"

#include "io/gps.h"
#include "io/gps_private.h"

#include "flight/imu.h"

float estimatedWind[3];    // wind velocity vectors in cm / sec

void estimate_wind(void)
{
    float lastGroundVelocity[3];
    float groundVelocity[3];
    float groundVelocityDiff[3];
    float groundVelocitySum[3];

    float lastFuselageDirection[3];
    float fuselageDirection[3];
    float fuselageDirectionDiff[3];
    float fuselageDirectionSum[3];

    groundVelocity[0] = gpsSol.velNED[0] / 100;
    groundVelocity[1] = gpsSol.velNED[1] / 100;
    groundVelocity[2] = gpsSol.velNED[2] / 100;

    fuselageDirection[0] = rMat[0][0];
    fuselageDirection[1] = rMat[1][0];
    fuselageDirection[2] = rMat[2][0];

    fuselageDirectionDiff[0] = fuselageDirection[0] - lastFuselageDirection[0];
    fuselageDirectionDiff[1] = fuselageDirection[1] - lastFuselageDirection[1];
    fuselageDirectionDiff[2] = fuselageDirection[2] - lastFuselageDirection[2];
		
    static timeMs_t _last_wind_time;
    timeMs_t now = millis();
    // scrap our data and start over if we're taking too long to get a direction change
    if (now - _last_wind_time > 10000) {
        _last_wind_time = now;

        memcpy(lastFuselageDirection, fuselageDirection, sizeof(lastFuselageDirection));
        memcpy(lastGroundVelocity, groundVelocity, sizeof(lastGroundVelocity));
        return;
    }

    float diff_length = sqrtf(sq(fuselageDirectionDiff[0]) + sq(fuselageDirectionDiff[1]) + sq(fuselageDirectionDiff[2]));
    if (diff_length > 0.2f) {
        // when turning, use the attitude response to estimate wind speed
        groundVelocityDiff[0] = groundVelocity[0] - lastGroundVelocity[0];
        groundVelocityDiff[1] = groundVelocity[1] - lastGroundVelocity[1];
        groundVelocityDiff[2] = groundVelocity[2] - lastGroundVelocity[2];
			
        // estimate airspeed it using equation 6
        float V = (sqrtf(sq(groundVelocityDiff[0]) + sq(groundVelocityDiff[1]) + sq(groundVelocityDiff[2]))) / diff_length;

        fuselageDirectionSum[0] = fuselageDirection[0] + lastFuselageDirection[0];
        fuselageDirectionSum[1] = fuselageDirection[1] + lastFuselageDirection[1];
        fuselageDirectionSum[2] = fuselageDirection[2] + lastFuselageDirection[2];

        groundVelocitySum[0] = groundVelocity[0] + lastGroundVelocity[0];
        groundVelocitySum[1] = groundVelocity[1] + lastGroundVelocity[1];
        groundVelocitySum[2] = groundVelocity[2] + lastGroundVelocity[2];

        memcpy(lastFuselageDirection, fuselageDirection, sizeof(lastFuselageDirection));
        memcpy(lastGroundVelocity, groundVelocity, sizeof(lastGroundVelocity));

        float theta = atan2f(groundVelocityDiff[1], groundVelocityDiff[0]) - atan2f(groundVelocityDiff[1], groundVelocityDiff[0]);// equation 9
        float sintheta = sinf(theta);
        float costheta = cosf(theta);

        float wind[3];
        wind[0] = (groundVelocitySum[0] - V * (costheta * fuselageDirectionSum[0] - sintheta * fuselageDirectionSum[1])) * 0.5f;// equation 10
        wind[1] = (groundVelocitySum[1] - V * (sintheta * fuselageDirectionSum[0] + costheta * fuselageDirectionSum[1])) * 0.5f;// equation 11
        wind[2] = (groundVelocitySum[2] - V * fuselageDirectionSum[2]) * 0.5f;// equation 12

        float wind_length = sqrtf(sq(wind[0]) + sq(wind[1]) + sq(wind[2]));
        float _wind_length = sqrtf(sq(estimatedWind[0]) + sq(estimatedWind[1]) + sq(estimatedWind[2]));

        if (wind_length < _wind_length + 20) {
            estimatedWind[0] = estimatedWind[0] * 0.95f + wind[0] * 0.05f;;
            estimatedWind[1] = estimatedWind[1] * 0.95f + wind[1] * 0.05f;
            estimatedWind[2] = estimatedWind[2] * 0.95f + wind[2] * 0.05f;
        }

        _last_wind_time = now;
    }
}
