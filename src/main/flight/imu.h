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

typedef struct imuConfig_s {
    uint16_t dcm_kp_acc;                    // DCM filter proportional gain ( x 10000) for accelerometer
    uint16_t dcm_ki_acc;                    // DCM filter integral gain ( x 10000) for accelerometer
    uint16_t dcm_kp_mag;                    // DCM filter proportional gain ( x 10000) for magnetometer and GPS heading
    uint16_t dcm_ki_mag;                    // DCM filter integral gain ( x 10000) for magnetometer and GPS heading
    uint8_t small_angle;
    uint8_t acc_ignore_rate;
    uint8_t acc_ignore_slope;
} imuConfig_t;

PG_DECLARE(imuConfig_t, imuConfig);

typedef struct imuRuntimeConfig_s {
    float dcm_kp_acc;
    float dcm_kp_mag;
    float gps_gain;
    float beta;
} imuRuntimeConfig_t;

void ahrsInit(void);

void ahrsConfigure(void);

bool ahrsSetMagneticDeclination(float declinationDeg);
void ahrsReset(bool recover_eulers);
void ahrsUpdate(timeUs_t currentTimeUs);
float ahrsGetCosTiltAngle(void);
bool ahrsIsHealthy(void);
bool isAhrsHeadingValid(void);

void ahrsTransformVectorBodyToEarth(fpVector3_t * v);
void ahrsTransformVectorEarthToBody(fpVector3_t * v);

void updateWindEstimator(void);
