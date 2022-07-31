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

#pragma once

#include "common/axis.h"
#include "common/maths.h"
#include "common/vector.h"
#include "common/quaternion.h"
#include "common/time.h"
#include "config/parameter_group.h"

extern fpVector3_t imuMeasuredAccelBF;    // cm/s/s
extern fpVector3_t imuMeasuredRotationBF; // rad/s

typedef union {
    int16_t raw[XYZ_AXIS_COUNT];
    struct {
        int16_t roll;
        int16_t pitch;
        int16_t yaw;
    } values;
} attitudeEulerAngles_t;

extern attitudeEulerAngles_t attitude;

typedef struct ahrsConfig_s {
    uint16_t dcm_kp_acc;
    uint16_t dcm_kp_mag;
    uint16_t dcm_gps_gain;
    uint8_t small_angle;
} ahrsConfig_t;

PG_DECLARE(ahrsConfig_t, ahrsConfig);

void ahrsInit(void);

void ahrsSetMagneticDeclination(float declinationDeg);
void ahrsReset(bool recover_eulers);
void ahrsUpdate(timeUs_t currentTimeUs);
bool ahrsIsHealthy(void);
bool isAhrsHeadingValid(void);
float ahrsGetTiltAngle(void);
void updateWindEstimator(void);

void ahrsTransformVectorBodyToEarth(fpVector3_t * v);
void ahrsTransformVectorEarthToBody(fpVector3_t * v);

float ahrsGetCosYaw(void);
float ahrsGetSinYaw(void);
