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

// Based on WindEstimation.pdf paper
#define WINDESTIMATOR_TIMEOUT       60*15 // 15min with out altitude change
#define WINDESTIMATOR_ALTITUDE_SCALE WINDESTIMATOR_TIMEOUT/500.0f //or 500m altitude change

#define WINDESTIMATOR_VALIDITY_THRESHOLD    15
#define WINDESTIMATOR_SPIKE_FILTER_ADJ_FACTOR   40

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
    static timeUs_t lastValidWindEstimateUs = 0;
    static float lastValidEstimateAltitude = 0.0f;
    float currentAltitude = gpsSol.llh.alt / 100.0f; // altitude in m
    static uint8_t validityScore = 0;
    bool updateTimedout = false;
    static uint8_t spikeFilterDynAdjustment = WINDESTIMATOR_SPIKE_FILTER_ADJ_FACTOR;
    static bool initialEstimate = true;

    if ((US2S(currentTimeUs - lastValidWindEstimateUs) + WINDESTIMATOR_ALTITUDE_SCALE * fabsf(currentAltitude - lastValidEstimateAltitude)) > WINDESTIMATOR_TIMEOUT) {
        hasValidWindEstimate = false;
    }

    /* validityScore used to indicate validity of wind estimate in a more reactive way compared to the basic method used above.
     * Each new estimate calc adds to score and each updateTimedout decrements from score.
     * hasValidWindEstimate considered valid when score > WINDESTIMATOR_VALIDITY_THRESHOLD with max count limit of WINDESTIMATOR_VALIDITY_THRESHOLD + 15.
     * WINDESTIMATOR_VALIDITY_THRESHOLD should result in a valid estimate based on the spike elimination and filtering used.
     * hasValidWindEstimate considered invalid when score = 0 which approximates to around 2.5 to 5 minutes if no new estimate calcs occur */

    if (US2S(cmpTimeUs(currentTimeUs, lastUpdateUs)) > 10 || lastUpdateUs == 0) {
        if (validityScore > 0) validityScore--;

        lastUpdateUs = currentTimeUs;
        updateTimedout = true;

        // Rapidly reset spikeFilterDynAdjustment if no new wind calcs
        if (!initialEstimate && spikeFilterDynAdjustment >= 5) {
            spikeFilterDynAdjustment -= 5;
        }
    }

    if (!validityScore) {
        hasValidWindEstimate = false;
    } else if (!hasValidWindEstimate && validityScore > WINDESTIMATOR_VALIDITY_THRESHOLD) {
        hasValidWindEstimate = true;
    }

    if (!isGPSHeadingValid() || !gpsSol.flags.validVelNE || !gpsSol.flags.validVelD
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

    // Get current 3D velocity from GPS in cm/s relative to earth frame
    groundVelocity[X] = posEstimator.gps.vel.x;
    groundVelocity[Y] = posEstimator.gps.vel.y;
    groundVelocity[Z] = posEstimator.gps.vel.z;

    // Fuselage direction in earth frame
    fuselageDirection[X] = HeadVecEFFiltered.x;
    fuselageDirection[Y] = -HeadVecEFFiltered.y;
    fuselageDirection[Z] = -HeadVecEFFiltered.z;

    // scrap our data and start over if we're taking too long (> 10s) to get a direction change
    if (updateTimedout) {
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
        lastUpdateUs = currentTimeUs;

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

        float theta = atan2f(groundVelocityDiff[Y], groundVelocityDiff[X]) - atan2f(fuselageDirectionDiff[Y], fuselageDirectionDiff[X]);    // equation 9
        float sintheta = sinf(theta);
        float costheta = cosf(theta);

        float wind[XYZ_AXIS_COUNT];
        wind[X] = (groundVelocitySum[X] - V * (costheta * fuselageDirectionSum[X] - sintheta * fuselageDirectionSum[Y])) * 0.5f;    // equation 10
        wind[Y] = (groundVelocitySum[Y] - V * (sintheta * fuselageDirectionSum[X] + costheta * fuselageDirectionSum[Y])) * 0.5f;    // equation 11
        wind[Z] = (groundVelocitySum[Z] - V * fuselageDirectionSum[Z]) * 0.5f;  // equation 12

        /* Spike filter used to filter out large spikes that can occur in the raw wind calcs.
         * Filter is based on a threshold between new wind updates and current estimated wind.
         * A baseline threshold of 3 m/s is used with an additional dynamic threshold to clear a stuck estimate.
         * The dynamic threshold relaxes spike filtering until the estimate recovers then falls back to the baseline threshold.
         * The dynamic threshold is active on initialisation and also if new updates haven't made it past the spike filter > 30s.
         * New wind values are discarded if a single axis exceeds the spike threshhold */

        if (initialEstimate) {
            if (validityScore == WINDESTIMATOR_VALIDITY_THRESHOLD + 15) {
                initialEstimate = false;
                spikeFilterDynAdjustment = 0;
            }
        } else if (spikeFilterDynAdjustment || US2S(cmpTimeUs(currentTimeUs, lastValidWindEstimateUs)) > 30) {  // 30s estimate update timeout
            if (spikeFilterDynAdjustment < WINDESTIMATOR_SPIKE_FILTER_ADJ_FACTOR) {
                spikeFilterDynAdjustment++;
                if (hasValidWindEstimate && validityScore > 1) validityScore -= 2;  // degrade valid estimate if update stuck too long
            }
        }

        // Spike filter
        for (uint8_t axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
            if (ABS(wind[axis] - estimatedWind[axis]) > (300 + spikeFilterDynAdjustment * WINDESTIMATOR_SPIKE_FILTER_ADJ_FACTOR)) {
                return;
            }
        }

        // Spike free filter
        float filterAlpha = 0.1f;
        estimatedWind[X] = estimatedWind[X] + filterAlpha * (wind[X] - estimatedWind[X]);
        estimatedWind[Y] = estimatedWind[Y] + filterAlpha * (wind[Y] - estimatedWind[Y]);
        estimatedWind[Z] = estimatedWind[Z] + filterAlpha * (wind[Z] - estimatedWind[Z]);

        if (validityScore < WINDESTIMATOR_VALIDITY_THRESHOLD + 15) validityScore++;

        if (spikeFilterDynAdjustment) {
            spikeFilterDynAdjustment -= (spikeFilterDynAdjustment == 1 || initialEstimate) ? 1 : 2;
        }

        lastValidWindEstimateUs = currentTimeUs;
        lastValidEstimateAltitude = currentAltitude;
    }
}

#endif
