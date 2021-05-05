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
#include "sensors/opflow.h"

#include "flight/imu.h"

extern navigationPosEstimator_t posEstimator;

#ifdef USE_OPFLOW
/**
 * Read optical flow topic
 *  Function is called by OPFLOW task as soon as new update is available
 */
void updatePositionEstimator_OpticalFlowTopic(timeUs_t currentTimeUs)
{
    posEstimator.flow.lastUpdateTime = currentTimeUs;
    posEstimator.flow.isValid = opflow.isHwHealty && (opflow.flowQuality == OPFLOW_QUALITY_VALID);
    posEstimator.flow.flowRate[X] = opflow.flowRate[X];
    posEstimator.flow.flowRate[Y] = opflow.flowRate[Y];
    posEstimator.flow.bodyRate[X] = opflow.bodyRate[X];
    posEstimator.flow.bodyRate[Y] = opflow.bodyRate[Y];
}
#endif

bool estimationCalculateCorrection_XY_FLOW(estimationContext_t * ctx)
{
#if defined(USE_RANGEFINDER) && defined(USE_OPFLOW)
    if (!((ctx->newFlags & EST_FLOW_VALID) && (ctx->newFlags & EST_SURFACE_VALID) && (ctx->newFlags & EST_Z_VALID))) {
        return false;
    }

    // FIXME: flow may use AGL estimate if available
    const bool canUseFlow = (posEstimator.surface.reliability >= RANGEFINDER_RELIABILITY_LOW_THRESHOLD);

    if (!canUseFlow) {
        return false;
    }

    // Calculate linear velocity based on angular velocity and altitude
    // Technically we should calculate arc length here, but for fast sampling this is accurate enough
    fpVector3_t flowVel = {
        .x = - (posEstimator.flow.flowRate[Y] - posEstimator.flow.bodyRate[Y]) * posEstimator.surface.alt,
        .y =   (posEstimator.flow.flowRate[X] - posEstimator.flow.bodyRate[X]) * posEstimator.surface.alt,
        .z =    posEstimator.est.vel.z
    };

    // At this point flowVel will hold linear velocities in earth frame
    imuTransformVectorBodyToEarth(&flowVel);

    // Calculate velocity correction
    const float flowVelXInnov = flowVel.x - posEstimator.est.vel.x;
    const float flowVelYInnov = flowVel.y - posEstimator.est.vel.y;

    ctx->estVelCorr.x = flowVelXInnov * positionEstimationConfig()->w_xy_flow_v * ctx->dt;
    ctx->estVelCorr.y = flowVelYInnov * positionEstimationConfig()->w_xy_flow_v * ctx->dt;

    // Calculate position correction if possible/allowed
    if ((ctx->newFlags & EST_GPS_XY_VALID)) {
        // If GPS is valid - reset flow estimated coordinates to GPS
        posEstimator.est.flowCoordinates[X] = posEstimator.gps.pos.x;
        posEstimator.est.flowCoordinates[Y] = posEstimator.gps.pos.y;
    }
    else if (positionEstimationConfig()->allow_dead_reckoning) {
        posEstimator.est.flowCoordinates[X] += flowVel.x * ctx->dt;
        posEstimator.est.flowCoordinates[Y] += flowVel.y * ctx->dt;

        const float flowResidualX = posEstimator.est.flowCoordinates[X] - posEstimator.est.pos.x;
        const float flowResidualY = posEstimator.est.flowCoordinates[Y] - posEstimator.est.pos.y;

        ctx->estPosCorr.x = flowResidualX * positionEstimationConfig()->w_xy_flow_p * ctx->dt;
        ctx->estPosCorr.y = flowResidualY * positionEstimationConfig()->w_xy_flow_p * ctx->dt;

        ctx->newEPH = updateEPE(posEstimator.est.eph, ctx->dt, sqrtf(sq(flowResidualX) + sq(flowResidualY)), positionEstimationConfig()->w_xy_flow_p);
    }

    DEBUG_SET(DEBUG_FLOW, 0, RADIANS_TO_DEGREES(posEstimator.flow.flowRate[X]));
    DEBUG_SET(DEBUG_FLOW, 1, RADIANS_TO_DEGREES(posEstimator.flow.flowRate[Y]));
    DEBUG_SET(DEBUG_FLOW, 2, posEstimator.est.flowCoordinates[X]);
    DEBUG_SET(DEBUG_FLOW, 3, posEstimator.est.flowCoordinates[Y]);

    return true;
#else
    UNUSED(ctx);
    return false;
#endif
}


#endif  // NAV
