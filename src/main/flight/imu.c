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

// Inertial Measurement Unit (IMU)

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "common/maths.h"

#include "platform.h"
#include "debug.h"

#include "common/axis.h"

#include "drivers/system.h"
#include "drivers/sensor.h"
#include "drivers/accgyro.h"
#include "drivers/compass.h"

#include "sensors/sensors.h"
#include "sensors/gyro.h"
#include "sensors/compass.h"
#include "sensors/acceleration.h"
#include "sensors/barometer.h"
#include "sensors/sonar.h"

#include "flight/mixer.h"
#include "flight/pid.h"
#include "flight/imu.h"

#include "config/runtime_config.h"

// Velocities and accelerations in ENU coordinates
float imuAverageVelocity[XYZ_AXIS_COUNT];
float imuAverageAcceleration[XYZ_AXIS_COUNT];

// Variables for velocity estimation from accelerometer. Update rates can be different for each axis.
typedef struct {
    int32_t accSum;             // sum of accelerometer samples (for averaging)
    uint32_t accTimeSum;        // total time of accelerometer samples
    uint32_t accSumCount;       // total number of accelerometer samples
    uint8_t velLpfHz;           // cutoff frequency of 1-st order RC-filter for velocity calculation
    uint32_t lastRefVelUpdate;  // last reference velocity update (when this happened too far in the past - decay estimated velocity to zero
} imuAccVelContext_s;

static imuAccVelContext_s accVel[XYZ_AXIS_COUNT];

int16_t accSmooth[XYZ_AXIS_COUNT];

int16_t smallAngle = 0;

float magneticDeclination = 0.0f;       // calculated at startup from config
float gyroScaleRad;

rollAndPitchInclination_t inclination = { { 0, 0 } };     // absolute angle inclination in multiple of 0.1 degree    180 deg = 1800
float anglerad[2] = { 0.0f, 0.0f };    // absolute angle inclination in radians

static imuRuntimeConfig_t *imuRuntimeConfig;
static pidProfile_t *pidProfile;
static uint8_t accDeadband;

void imuConfigure(
    imuRuntimeConfig_t *initialImuRuntimeConfig,
    pidProfile_t *initialPidProfile,
    uint8_t initialAccDeadband
)
{
    imuRuntimeConfig = initialImuRuntimeConfig;
    pidProfile = initialPidProfile;
    accDeadband = initialAccDeadband;
}

void imuInit()
{
    int axis;

    smallAngle = lrintf(acc_1G * cos_approx(degreesToRadians(imuRuntimeConfig->small_angle)));
    gyroScaleRad = gyro.scale * (M_PIf / 180.0f) * 0.000001f;

    for (axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        imuAverageVelocity[axis] = 0;
        imuAverageAcceleration[axis] = 0;
    }

    // Set integrator LPF HZ
    accVel[X].velLpfHz = 15;
    accVel[Y].velLpfHz = 15;
    accVel[Z].velLpfHz = 10;
}

float calculateThrottleAngleScale(uint16_t throttle_correction_angle)
{
    return (1800.0f / M_PIf) * (900.0f / throttle_correction_angle);
}

// **************************************************
// Simplified IMU based on "Complementary Filter"
// Inspired by http://starlino.com/imu_guide.html
//
// adapted by ziss_dm : http://www.multiwii.com/forum/viewtopic.php?f=8&t=198
//
// The following ideas was used in this project:
// 1) Rotation matrix: http://en.wikipedia.org/wiki/Rotation_matrix
//
// Currently Magnetometer uses separate CF which is used only
// for heading approximation.
//
// **************************************************

t_fp_vector EstG;

static void imuResetAccelerationSum(int axis)
{
    accVel[axis].accSum = 0;
    accVel[axis].accSumCount = 0;
    accVel[axis].accTimeSum = 0;
}

#define MAX_REF_VEL_UPDATE_INTERVAL     1000000     // 1s = 1Hz
#define REF_VEL_DECAY_FACTOR            0.995f
void imuSampleAverageAccelerationAndVelocity(uint8_t axis)
{
    if (accVel[axis].accTimeSum == 0)
        return;

    float accSumDt = accVel[axis].accTimeSum * 1e-6f;

    // RC-LPF: y[i] = y[i-1] + alpha * (x[i] - y[i-1])
    imuAverageAcceleration[axis] = ((float)accVel[axis].accSum / (float)accVel[axis].accSumCount) * (100.0f * 9.80665f / acc_1G);   // cm/s^2
    imuAverageVelocity[axis] += (accSumDt / ((0.5f / (M_PIf * accVel[axis].velLpfHz)) + accSumDt)) * (imuAverageAcceleration[axis] * accSumDt);
    imuResetAccelerationSum(axis);

    // If reference was updated far in the past - decay to zero. This provides somewhat accurate result in short-term perspective, but prevents accumulation of integration error
    if ((micros() - accVel[axis].lastRefVelUpdate) > MAX_REF_VEL_UPDATE_INTERVAL) {
        imuAverageVelocity[axis] = imuAverageVelocity[axis] * REF_VEL_DECAY_FACTOR;
    }
}

void imuApplyFilterToActualVelocity(uint8_t axis, float cfFactor, float referenceVelocity)
{
    // apply Complimentary Filter to keep the calculated velocity based on reference (near real) velocity.
    imuAverageVelocity[axis] = imuAverageVelocity[axis] * cfFactor + referenceVelocity * (1.0f - cfFactor);
    accVel[axis].lastRefVelUpdate = micros();
}

// rotate acc into Earth frame and calculate acceleration in it
static void imuCalculateAccelerationAndVelocity(uint32_t deltaT)
{
    static int32_t accZoffset = 0;
    int axis;
    fp_angles_t rpy;
    t_fp_vector accel_ned;

    // the accel values have to be rotated into the earth frame
    rpy.angles.roll = -(float)anglerad[AI_ROLL];
    rpy.angles.pitch = -(float)anglerad[AI_PITCH];
    rpy.angles.yaw = -(float)heading * RAD;

    accel_ned.V.X = accSmooth[0];
    accel_ned.V.Y = accSmooth[1];
    accel_ned.V.Z = accSmooth[2];
    //accel_ned.V.X = accADC[0];
    //accel_ned.V.Y = accADC[1];
    //accel_ned.V.Z = accADC[2];

    rotateV(&accel_ned.V, &rpy);

    if (imuRuntimeConfig->acc_unarmedcal == 1) {
        if (!ARMING_FLAG(ARMED)) {
            accZoffset -= accZoffset / 64;
            accZoffset += accel_ned.V.Z;
        }
        accel_ned.V.Z -= accZoffset / 64;  // compensate for gravitation on z-axis
    } else
        accel_ned.V.Z -= acc_1G;

    // apply Deadband to reduce integration drift and vibration influence
    accVel[X].accSum += applyDeadband(lrintf(accel_ned.V.X), accDeadband);
    accVel[Y].accSum += applyDeadband(lrintf(accel_ned.V.Y), accDeadband);
    accVel[Z].accSum += applyDeadband(lrintf(accel_ned.V.Z), accDeadband);

    // Accumulate acceleration for averaging and integration to get velocity
    for (axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        accVel[axis].accTimeSum += deltaT;
        accVel[axis].accSumCount++;
    }
}

/*
* Baseflight calculation by Luggi09 originates from arducopter
* ============================================================
* This function rotates magnetic vector to cancel actual yaw and
* pitch of craft. Then it computes it's direction in X/Y plane.
* This value is returned as compass heading, value is 0-360 degrees.
*
* Note that Earth's magnetic field is not parallel with ground unless
* you are near equator. Its inclination is considerable, >60 degrees
* towards ground in most of Europe.
*
* First we consider it in 2D:
*
* An example, the vector <1, 1> would be turned into the heading
* 45 degrees, representing it's angle clockwise from north.
*
*      ***************** *
*      *       |   <1,1> *
*      *       |  /      *
*      *       | /       *
*      *       |/        *
*      *       *         *
*      *                 *
*      *                 *
*      *                 *
*      *                 *
*      *******************
*
* //TODO: Add explanation for how it uses the Z dimension.
*/
int16_t imuCalculateHeading(t_fp_vector *vec)
{
    int16_t head;

    float cosineRoll = cos_approx(anglerad[AI_ROLL]);
    float sineRoll = sin_approx(anglerad[AI_ROLL]);
    float cosinePitch = cos_approx(anglerad[AI_PITCH]);
    float sinePitch = sin_approx(anglerad[AI_PITCH]);
    float Xh = vec->A[X] * cosinePitch + vec->A[Y] * sineRoll * sinePitch + vec->A[Z] * sinePitch * cosineRoll;
    float Yh = vec->A[Y] * cosineRoll - vec->A[Z] * sineRoll;
    //TODO: Replace this comment with an explanation of why Yh and Xh can never simultanoeusly be zero,
    // or handle the case in which they are and (atan2f(0, 0) is undefined.
    float hd = (atan2_approx(Yh, Xh) * 1800.0f / M_PIf + magneticDeclination) / 10.0f;
    head = lrintf(hd);

    // Arctan returns a value in the range -180 to 180 degrees. We 'normalize' negative angles to be positive.
    if (head < 0)
        head += 360;

    return head;
}

static void imuCalculateEstimatedAttitude(void)
{
    int32_t axis;
    int32_t accMag = 0;
    static t_fp_vector EstM;
    static t_fp_vector EstN = { .A = { 1.0f, 0.0f, 0.0f } };
    static float accLPF[3];
    static uint32_t previousT;
    uint32_t currentT = micros();
    uint32_t deltaT;
    float scale;
    fp_angles_t deltaGyroAngle;
    deltaT = currentT - previousT;
    scale = deltaT * gyroScaleRad;
    previousT = currentT;

    // Initialization
    for (axis = 0; axis < 3; axis++) {
        deltaGyroAngle.raw[axis] = gyroADC[axis] * scale;
        if (imuRuntimeConfig->acc_lpf_factor > 0) {
            accLPF[axis] = accLPF[axis] * (1.0f - (1.0f / imuRuntimeConfig->acc_lpf_factor)) + accADC[axis] * (1.0f / imuRuntimeConfig->acc_lpf_factor);
            accSmooth[axis] = accLPF[axis];
        } else {
            accSmooth[axis] = accADC[axis];
        }
        accMag += (int32_t)accSmooth[axis] * accSmooth[axis];
    }
    accMag = accMag * 100 / ((int32_t)acc_1G * acc_1G);

    rotateV(&EstG.V, &deltaGyroAngle);

    // Apply complimentary filter (Gyro drift correction)
    // If accel magnitude >1.15G or <0.85G and ACC vector outside of the limit range => we neutralize the effect of accelerometers in the angle estimation.
    // To do that, we just skip filter, as EstV already rotated by Gyro

    float invGyroComplimentaryFilterFactor = (1.0f / (imuRuntimeConfig->gyro_cmpf_factor + 1.0f));

    if (72 < (uint16_t)accMag && (uint16_t)accMag < 133) {
        for (axis = 0; axis < 3; axis++)
            EstG.A[axis] = (EstG.A[axis] * imuRuntimeConfig->gyro_cmpf_factor + accSmooth[axis]) * invGyroComplimentaryFilterFactor;
    }

    if (EstG.A[Z] > smallAngle) {
        ENABLE_STATE(SMALL_ANGLE);
    } else {
        DISABLE_STATE(SMALL_ANGLE);
    }

    // Attitude of the estimated vector
    anglerad[AI_ROLL] = atan2_approx(EstG.V.Y, EstG.V.Z);
    anglerad[AI_PITCH] = atan2_approx(-EstG.V.X, sqrtf(EstG.V.Y * EstG.V.Y + EstG.V.Z * EstG.V.Z));
    inclination.values.rollDeciDegrees = lrintf(anglerad[AI_ROLL] * (1800.0f / M_PIf));
    inclination.values.pitchDeciDegrees = lrintf(anglerad[AI_PITCH] * (1800.0f / M_PIf));

    if (sensors(SENSOR_MAG)) {
        rotateV(&EstM.V, &deltaGyroAngle);
        // FIXME what does the _M_ mean?
        float invGyroComplimentaryFilter_M_Factor = (1.0f / (imuRuntimeConfig->gyro_cmpfm_factor + 1.0f));
        for (axis = 0; axis < 3; axis++) {
            EstM.A[axis] = (EstM.A[axis] * imuRuntimeConfig->gyro_cmpfm_factor + magADC[axis]) * invGyroComplimentaryFilter_M_Factor;
        }
        heading = imuCalculateHeading(&EstM);
    } else {
        rotateV(&EstN.V, &deltaGyroAngle);
        normalizeV(&EstN.V, &EstN.V);
        heading = imuCalculateHeading(&EstN);
    }

    imuCalculateAccelerationAndVelocity(deltaT); // rotate acc vector into earth frame
}

void imuUpdate(rollAndPitchTrims_t *accelerometerTrims)
{
    gyroUpdate();

    if (sensors(SENSOR_ACC)) {
        updateAccelerationReadings(accelerometerTrims); // TODO rename to accelerometerUpdate and rename many other 'Acceleration' references to be 'Accelerometer'
        imuCalculateEstimatedAttitude();
    } else {
        accADC[X] = 0;
        accADC[Y] = 0;
        accADC[Z] = 0;
    }
}

int16_t calculateTiltAngle(void)
{
    float cosZ = EstG.V.Z / sqrtf(EstG.V.X * EstG.V.X + EstG.V.Y * EstG.V.Y + EstG.V.Z * EstG.V.Z);
    return lrintf(fabsf(acos_approx(cosZ) * (1800.0f / M_PIf)));
}

int16_t calculateThrottleAngleCorrection(uint8_t throttle_correction_value, int16_t throttle_correction_angle)
{
    float cosZ = EstG.V.Z / sqrtf(EstG.V.X * EstG.V.X + EstG.V.Y * EstG.V.Y + EstG.V.Z * EstG.V.Z);

    if (cosZ <= 0.015f)
        return 0;

    int angle = lrintf(acos_approx(cosZ) * calculateThrottleAngleScale(throttle_correction_angle));
    if (angle > 900)
        angle = 900;
    return lrintf(throttle_correction_value * sin_approx((angle / 900.0f) * (M_PIf / 2.0f)));
}

// this function does the opposite of the calculateThrottleAngleCorrection - takes an actual correction and returns throttle_correction_value
uint8_t calculateThrottleCorrectionValue(uint16_t throttle_tilt_compensation, int16_t throttle_correction_angle)
{
    float cosZ = EstG.V.Z / sqrtf(EstG.V.X * EstG.V.X + EstG.V.Y * EstG.V.Y + EstG.V.Z * EstG.V.Z);

    /*
    * Use 0 as the throttle angle correction if we are inverted, vertical or with a
    * small angle < 0.86 deg
    * TODO: Define this small angle in config.
    */
    if (cosZ <= 0.015f) {
        return 0;
    }
    int angle = lrintf(acos_approx(cosZ) * calculateThrottleAngleScale(throttle_correction_angle));
    if (angle > 900)
        angle = 900;
    // a precaution to prevent DIV0 error
    if (angle < 1) 
        return 0;

    return lrintf(throttle_tilt_compensation / sin_approx((angle / 900.0f) * (M_PIf / 2.0f)));
}
