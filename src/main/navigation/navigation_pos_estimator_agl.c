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

#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#include "platform.h"

#include "build/build_config.h"
#include "build/debug.h"

#if defined(USE_NAV)

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"
#include "navigation/navigation_pos_estimator_private.h"

#include "sensors/rangefinder.h"
#include "sensors/barometer.h"

extern navigationPosEstimator_t posEstimator;

#ifdef USE_RANGEFINDER
/**
 * Read surface and update alt/vel topic
 *  Function is called from TASK_RANGEFINDER at arbitrary rate - as soon as new measurements are available
 */
void updatePositionEstimator_SurfaceTopic(timeUs_t currentTimeUs, float newSurfaceAlt)
{
    const float surfaceDtUs = currentTimeUs - posEstimator.surface.lastUpdateTime;
    float newReliabilityMeasurement = 0;
    bool surfaceMeasurementWithinRange = false;

    posEstimator.surface.lastUpdateTime = currentTimeUs;

    if (newSurfaceAlt >= 0) {
        if (newSurfaceAlt <= positionEstimationConfig()->max_surface_altitude) {
            newReliabilityMeasurement = 1.0f;
            surfaceMeasurementWithinRange = true;
            posEstimator.surface.alt = newSurfaceAlt;
        }
        else {
            newReliabilityMeasurement = 0.0f;
        }
    }
    else {
        // Negative values - out of range or failed hardware
        newReliabilityMeasurement = 0.0f;
    }

    /* Reliability is a measure of confidence of rangefinder measurement. It's increased with each valid sample and decreased with each invalid sample */
    if (surfaceDtUs > MS2US(INAV_SURFACE_TIMEOUT_MS)) {
        posEstimator.surface.reliability = 0.0f;
    }
    else {
        const float surfaceDt = US2S(surfaceDtUs);
        const float relAlpha = surfaceDt / (surfaceDt + RANGEFINDER_RELIABILITY_RC_CONSTANT);
        posEstimator.surface.reliability = posEstimator.surface.reliability * (1.0f - relAlpha) + newReliabilityMeasurement * relAlpha;

        // Update average sonar altitude if range is good
        if (surfaceMeasurementWithinRange) {
            pt1FilterApply3(&posEstimator.surface.avgFilter, newSurfaceAlt, surfaceDt);
        }
    }
}
#endif

void estimationCalculateAGL(estimationContext_t * ctx)
{
#if defined(USE_RANGEFINDER) && defined(USE_BARO)
    if ((ctx->newFlags & EST_SURFACE_VALID) && (ctx->newFlags & EST_BARO_VALID)) {
        navAGLEstimateQuality_e newAglQuality = posEstimator.est.aglQual;
        bool resetSurfaceEstimate = false;
        switch (posEstimator.est.aglQual) {
            case SURFACE_QUAL_LOW:
                if (posEstimator.surface.reliability >= RANGEFINDER_RELIABILITY_HIGH_THRESHOLD) {
                    newAglQuality = SURFACE_QUAL_HIGH;
                    resetSurfaceEstimate = true;
                }
                else if (posEstimator.surface.reliability >= RANGEFINDER_RELIABILITY_LOW_THRESHOLD) {
                    newAglQuality = SURFACE_QUAL_LOW;
                }
                else {
                    newAglQuality = SURFACE_QUAL_LOW;
                }
                break;

            case SURFACE_QUAL_MID:
                if (posEstimator.surface.reliability >= RANGEFINDER_RELIABILITY_HIGH_THRESHOLD) {
                    newAglQuality = SURFACE_QUAL_HIGH;
                }
                else if (posEstimator.surface.reliability >= RANGEFINDER_RELIABILITY_LOW_THRESHOLD) {
                    newAglQuality = SURFACE_QUAL_MID;
                }
                else {
                    newAglQuality = SURFACE_QUAL_LOW;
                }
                break;

            case SURFACE_QUAL_HIGH:
                if (posEstimator.surface.reliability >= RANGEFINDER_RELIABILITY_HIGH_THRESHOLD) {
                    newAglQuality = SURFACE_QUAL_HIGH;
                }
                else if (posEstimator.surface.reliability >= RANGEFINDER_RELIABILITY_LOW_THRESHOLD) {
                    newAglQuality = SURFACE_QUAL_MID;
                }
                else {
                    newAglQuality = SURFACE_QUAL_LOW;
                }
                break;
        }

        posEstimator.est.aglQual = newAglQuality;

        if (resetSurfaceEstimate) {
            posEstimator.est.aglAlt = pt1FilterGetLastOutput(&posEstimator.surface.avgFilter);
            // If we have acceptable average estimate
            if (posEstimator.est.epv < positionEstimationConfig()->max_eph_epv) {
                posEstimator.est.aglVel = posEstimator.est.vel.z;
                posEstimator.est.aglOffset = posEstimator.est.pos.z - posEstimator.surface.alt;
            }
            else {
                posEstimator.est.aglVel = 0;
                posEstimator.est.aglOffset = 0;
            }
        }

        // Update estimate
        const float accWeight = navGetAccelerometerWeight();
        posEstimator.est.aglAlt += posEstimator.est.aglVel * ctx->dt;
        posEstimator.est.aglAlt += posEstimator.imu.accelNEU.z * sq(ctx->dt) / 2.0f * accWeight;
        posEstimator.est.aglVel += posEstimator.imu.accelNEU.z * ctx->dt * sq(accWeight);

        // Apply correction
        if (posEstimator.est.aglQual == SURFACE_QUAL_HIGH) {
            // Correct estimate from rangefinder
            const float surfaceResidual = posEstimator.surface.alt - posEstimator.est.aglAlt;
            const float bellCurveScaler = scaleRangef(bellCurve(surfaceResidual, 75.0f), 0.0f, 1.0f, 0.1f, 1.0f);

            posEstimator.est.aglAlt += surfaceResidual * positionEstimationConfig()->w_z_surface_p * bellCurveScaler * posEstimator.surface.reliability * ctx->dt;
            posEstimator.est.aglVel += surfaceResidual * positionEstimationConfig()->w_z_surface_v * sq(bellCurveScaler) * sq(posEstimator.surface.reliability) * ctx->dt;

            // Update estimate offset
            if ((posEstimator.est.aglQual == SURFACE_QUAL_HIGH) && (posEstimator.est.epv < positionEstimationConfig()->max_eph_epv)) {
                posEstimator.est.aglOffset = posEstimator.est.pos.z - pt1FilterGetLastOutput(&posEstimator.surface.avgFilter);
            }
        }
        else if (posEstimator.est.aglQual == SURFACE_QUAL_MID) {
            // Correct estimate from altitude fused from rangefinder and global altitude
            const float estAltResidual = (posEstimator.est.pos.z - posEstimator.est.aglOffset) - posEstimator.est.aglAlt;
            const float surfaceResidual = posEstimator.surface.alt - posEstimator.est.aglAlt;
            const float surfaceWeightScaler = scaleRangef(bellCurve(surfaceResidual, 50.0f), 0.0f, 1.0f, 0.1f, 1.0f) * posEstimator.surface.reliability;
            const float mixedResidual = surfaceResidual * surfaceWeightScaler + estAltResidual * (1.0f - surfaceWeightScaler);

            posEstimator.est.aglAlt += mixedResidual * positionEstimationConfig()->w_z_surface_p * ctx->dt;
            posEstimator.est.aglVel += mixedResidual * positionEstimationConfig()->w_z_surface_v * ctx->dt;
        }
        else {  // SURFACE_QUAL_LOW
            // In this case rangefinder can't be trusted - simply use global altitude
            posEstimator.est.aglAlt = posEstimator.est.pos.z - posEstimator.est.aglOffset;
            posEstimator.est.aglVel = posEstimator.est.vel.z;
        }
    }
    else {
        posEstimator.est.aglAlt = posEstimator.est.pos.z - posEstimator.est.aglOffset;
        posEstimator.est.aglVel = posEstimator.est.vel.z;
        posEstimator.est.aglQual = SURFACE_QUAL_LOW;
    }

    DEBUG_SET(DEBUG_AGL, 0, posEstimator.surface.reliability * 1000);
    DEBUG_SET(DEBUG_AGL, 1, posEstimator.est.aglQual);
    DEBUG_SET(DEBUG_AGL, 2, posEstimator.est.aglAlt);
    DEBUG_SET(DEBUG_AGL, 3, posEstimator.est.aglVel);

#else
    UNUSED(ctx);
    posEstimator.est.aglAlt = posEstimator.est.pos.z;
    posEstimator.est.aglVel = posEstimator.est.vel.z;
    posEstimator.est.aglQual = SURFACE_QUAL_LOW;
#endif
}

#endif  // NAV
