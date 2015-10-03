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
#include "flight/filter.h"

#include "io/gps.h"

#include "config/runtime_config.h"

t_fp_vector imuAccelInBodyFrame;
int16_t accSmooth[XYZ_AXIS_COUNT];
float smallAngleCosZ = 0;

float magneticDeclination = 0.0f;       // calculated at startup from config

static float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;    // quaternion of sensor frame relative to earth frame
static float rMat[3][3];

attitudeEulerAngles_t attitude = { { 0, 0, 0 } };     // absolute angle inclination in multiple of 0.1 degree    180 deg = 1800

static imuRuntimeConfig_t *imuRuntimeConfig;
static pidProfile_t *pidProfile;

static float gyroScale;

static void imuCompureRotationMatrix(void)
{
    float q1q1 = q1 * q1;
    float q2q2 = q2 * q2;
    float q3q3 = q3 * q3;
    
    float q0q1 = q0 * q1;
    float q0q2 = q0 * q2;
    float q0q3 = q0 * q3;
    float q1q2 = q1 * q2;
    float q1q3 = q1 * q3;
    float q2q3 = q2 * q3;

    rMat[0][0] = 1.0f - 2.0f * q2q2 - 2.0f * q3q3;
    rMat[0][1] = 2.0f * (q1q2 + -q0q3);
    rMat[0][2] = 2.0f * (q1q3 - -q0q2);

    rMat[1][0] = 2.0f * (q1q2 - -q0q3);
    rMat[1][1] = 1.0f - 2.0f * q1q1 - 2.0f * q3q3;
    rMat[1][2] = 2.0f * (q2q3 + -q0q1);

    rMat[2][0] = 2.0f * (q1q3 + -q0q2);
    rMat[2][1] = 2.0f * (q2q3 - -q0q1);
    rMat[2][2] = 1.0f - 2.0f * q1q1 - 2.0f * q2q2;
}

void imuConfigure(imuRuntimeConfig_t *initialImuRuntimeConfig, pidProfile_t *initialPidProfile)
{
    imuRuntimeConfig = initialImuRuntimeConfig;
    pidProfile = initialPidProfile;
}

void imuInit(void)
{
    int axis;

    smallAngleCosZ = cos_approx(degreesToRadians(imuRuntimeConfig->small_angle));
    gyroScale = gyro.scale * (M_PIf / 180.0f);  // gyro output scaled to rad per second

    for (axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        imuAccelInBodyFrame.A[axis] = 0;
    }

    imuCompureRotationMatrix();
}

void imuTransformVectorBodyToEarth(t_fp_vector * v)
{
    float x,y,z;

    /* From body frame to earth frame */
    x = rMat[0][0] * v->V.X + rMat[0][1] * v->V.Y + rMat[0][2] * v->V.Z;
    y = rMat[1][0] * v->V.X + rMat[1][1] * v->V.Y + rMat[1][2] * v->V.Z;
    z = rMat[2][0] * v->V.X + rMat[2][1] * v->V.Y + rMat[2][2] * v->V.Z;

    v->V.X = x;
    v->V.Y = -y;
    v->V.Z = z;
}

void imuTransformVectorEarthToBody(t_fp_vector * v)
{
    float x,y,z;

    v->V.Y = -v->V.Y;
    
    /* From earth frame to body frame */
    x = rMat[0][0] * v->V.X + rMat[1][0] * v->V.Y + rMat[2][0] * v->V.Z;
    y = rMat[0][1] * v->V.X + rMat[1][1] * v->V.Y + rMat[2][1] * v->V.Z;
    z = rMat[0][2] * v->V.X + rMat[1][2] * v->V.Y + rMat[2][2] * v->V.Z;

    v->V.X = x;
    v->V.Y = y;
    v->V.Z = z;
}

#define ACCELERATION_LPF_HZ             10

// Calculate acceleration in body frame
static void imuCalculateAcceleration(uint32_t deltaT)
{
    static float accGainCorrectionFactor = 1.0f;

    int axis;
    float accDt = deltaT * 1e-6f;

    for (axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        // Read sensor and convert to cm/s^2
        float accValueCMSS = accADC[axis] * (GRAVITY_CMSS / acc_1G) * accGainCorrectionFactor;

        // Apply LPF to acceleration: y[i] = y[i-1] + alpha * (x[i] - y[i-1])
        imuAccelInBodyFrame.A[axis] += (accDt / ((0.5f / (M_PIf * ACCELERATION_LPF_HZ)) + accDt)) * (accValueCMSS - imuAccelInBodyFrame.A[axis]);
    }

    /* When unarmed, assume that accelerometer should measure 1G. Use that to correct accelerometer gain */
    if (!ARMING_FLAG(ARMED) && imuRuntimeConfig->acc_unarmedcal) {
        float accMagnitude = sqrtf(sq(imuAccelInBodyFrame.V.X) + sq(imuAccelInBodyFrame.V.Y) + sq(imuAccelInBodyFrame.V.Z));
        if (accMagnitude >= 0.01f) {
            accGainCorrectionFactor += ((GRAVITY_CMSS / accMagnitude) - accGainCorrectionFactor) * 0.01f;
        }
    }
}

static float invSqrt(float x)
{
    return 1.0f / sqrtf(x);
}

static bool imuUseFastGains(void)
{
    return !ARMING_FLAG(ARMED) && millis() < 20000;
}

static void imuMahonyAHRSupdate(float dt, float gx, float gy, float gz,
                                bool useAcc, float ax, float ay, float az,
                                bool useMag, float mx, float my, float mz,
                                bool useYaw, float yawError)
{
    static float integralFBx = 0.0f,  integralFBy = 0.0f, integralFBz = 0.0f;    // integral error terms scaled by Ki
    float recipNorm;
    float hx, hy, bx;
    float ex = 0, ey = 0, ez = 0;
    float qa, qb, qc;

    // Use raw heading error (from GPS or whatever else)
    if (useYaw) {
        while (yawError >  M_PIf) yawError -= (2.0f * M_PIf);
        while (yawError < -M_PIf) yawError += (2.0f * M_PIf);

        ez += sin_approx(yawError / 2.0f);
    }

    // Use measured magnetic field vector
    recipNorm = mx * mx + my * my + mz * mz;
    if (useMag && recipNorm > 0.01f) {
        // Normalise magnetometer measurement
        recipNorm = invSqrt(recipNorm);
        mx *= recipNorm;
        my *= recipNorm;
        mz *= recipNorm;

        // For magnetometer correction we make an assumption that magnetic field is perpendicular to gravity (ignore Z-component in EF).
        // This way magnetic field will only affect heading and wont mess roll/pitch angles

        // (hx; hy; 0) - measured mag field vector in EF (assuming Z-component is zero)
        // (bx; 0; 0) - reference mag field vector heading due North in EF (assuming Z-component is zero)
        hx = rMat[0][0] * mx + rMat[0][1] * my + rMat[0][2] * mz;
        hy = rMat[1][0] * mx + rMat[1][1] * my + rMat[1][2] * mz;
        bx = sqrtf(hx * hx + hy * hy);

        // magnetometer error is cross product between estimated magnetic north and measured magnetic north (calculated in EF)
        float ez_ef = -(hy * bx);

        // Rotate mag error vector back to BF and accumulate
        ex += rMat[2][0] * ez_ef;
        ey += rMat[2][1] * ez_ef;
        ez += rMat[2][2] * ez_ef;
    }

    // Use measured acceleration vector
    recipNorm = ax * ax + ay * ay + az * az;
    if (useAcc && recipNorm > 0.01f) {
        // Normalise accelerometer measurement
        recipNorm = invSqrt(recipNorm);
        ax *= recipNorm;
        ay *= recipNorm;
        az *= recipNorm;

        // Error is sum of cross product between estimated direction and measured direction of gravity
        ex += (ay * rMat[2][2] - az * rMat[2][1]);
        ey += (az * rMat[2][0] - ax * rMat[2][2]);
        ez += (ax * rMat[2][1] - ay * rMat[2][0]);
    }

    // Compute and apply integral feedback if enabled
    if(imuRuntimeConfig->dcm_ki > 0.0f) {
        float dcmKiGain = imuRuntimeConfig->dcm_ki;
        integralFBx += dcmKiGain * ex * dt;    // integral error scaled by Ki
        integralFBy += dcmKiGain * ey * dt;
        integralFBz += dcmKiGain * ez * dt;
    }
    else {
        integralFBx = 0.0f;    // prevent integral windup
        integralFBy = 0.0f;
        integralFBz = 0.0f;
    }

    // Calculate kP gain. If we are acquiring initial attitude (not armed and within 20 sec from powerup) scale the kP to converge faster
    float dcmKpGain = imuRuntimeConfig->dcm_kp;
    if (imuUseFastGains()) {
        dcmKpGain *= 10;
    }

    // Apply proportional and integral feedback
    gx += dcmKpGain * ex + integralFBx;
    gy += dcmKpGain * ey + integralFBy;
    gz += dcmKpGain * ez + integralFBz;

    // Integrate rate of change of quaternion
    gx *= (0.5f * dt);
    gy *= (0.5f * dt);
    gz *= (0.5f * dt);

    qa = q0;
    qb = q1;
    qc = q2;
    q0 += (-qb * gx - qc * gy - q3 * gz);
    q1 += (qa * gx + qc * gz - q3 * gy);
    q2 += (qa * gy - qb * gz + q3 * gx);
    q3 += (qa * gz + qb * gy - qc * gx);

    // Normalise quaternion
    recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    q0 *= recipNorm;
    q1 *= recipNorm;
    q2 *= recipNorm;
    q3 *= recipNorm;

    // Pre-compute rotation matrix from quaternion
    imuCompureRotationMatrix();
}

static void imuUpdateEulerAngles(void)
{
    /* Compute pitch/roll angles */
    attitude.values.roll = lrintf(atan2_approx(rMat[2][1], rMat[2][2]) * (1800.0f / M_PIf));
    attitude.values.pitch = lrintf(((0.5f * M_PIf) - acos_approx(-rMat[2][0])) * (1800.0f / M_PIf));
    attitude.values.yaw = lrintf((-atan2_approx(rMat[1][0], rMat[0][0]) * (1800.0f / M_PIf) + magneticDeclination));

    if (attitude.values.yaw < 0)
        attitude.values.yaw += 3600;

    /* Update small angle state */
    if (rMat[2][2] > smallAngleCosZ) {
        ENABLE_STATE(SMALL_ANGLE);
    } else {
        DISABLE_STATE(SMALL_ANGLE);
    }
}

static bool imuIsAccelerometerHealthy(void)
{
    int32_t axis;
    int32_t accMagnitude = 0;

    for (axis = 0; axis < 3; axis++) {
        accMagnitude += (int32_t)accADC[axis] * accADC[axis];
    }

    accMagnitude = accMagnitude * 100 / ((int32_t)acc_1G * acc_1G);

    // Accept accel readings only in range 0.85g - 1.15g
    return (72 < accMagnitude) && (accMagnitude < 133);
}

static bool isMagnetometerHealthy(void)
{
    return (magADC[X] != 0) && (magADC[Y] != 0) && (magADC[Z] != 0);
}

static void imuCalculateEstimatedAttitude(void)
{
    static filterStatePt1_t accLPFState[3];
    static uint32_t previousIMUUpdateTime;
    float rawYawError;
    int32_t axis;
    bool useAcc = false;
    bool useMag = false;
    bool useYaw = false;

    uint32_t currentTime = micros();
    uint32_t deltaT = currentTime - previousIMUUpdateTime;
    previousIMUUpdateTime = currentTime;

    // Smooth and use only valid accelerometer readings
    if (imuIsAccelerometerHealthy()) {
        // If reading is considered valid - apply filter
        for (axis = 0; axis < 3; axis++) {
            if (imuRuntimeConfig->acc_cut_hz > 0) {
                accSmooth[axis] = filterApplyPt1(accADC[axis], &accLPFState[axis], imuRuntimeConfig->acc_cut_hz);
            } else {
                accSmooth[axis] = accADC[axis];
            }
        }

        useAcc = true;
    }

    if (sensors(SENSOR_MAG) && isMagnetometerHealthy()) {
        useMag = true;
    }
#if defined(GPS)
    else if (STATE(FIXED_WING) && sensors(SENSOR_GPS) && STATE(GPS_FIX) && GPS_numSat >= 5 && GPS_speed >= 300) {
        // In case of a fixed-wing aircraft we can use GPS course over ground to correct heading
        rawYawError = DECIDEGREES_TO_RADIANS(GPS_ground_course - attitude.values.yaw);
        useYaw = true;
    }
#endif

    imuMahonyAHRSupdate(deltaT * 1e-6f,
                        gyroADC[X] * gyroScale, gyroADC[Y] * gyroScale, gyroADC[Z] * gyroScale,
                        useAcc, accSmooth[X], accSmooth[Y], accSmooth[Z],
                        useMag, magADC[X], magADC[Y], magADC[Z],
                        useYaw, rawYawError);

    imuUpdateEulerAngles();

    imuCalculateAcceleration(deltaT); // rotate acc vector into earth frame and store it for future usage
}

void imuUpdate(rollAndPitchTrims_t *accelerometerTrims)
{
    gyroUpdate();

    if (sensors(SENSOR_ACC)) {
        updateAccelerationReadings(accelerometerTrims);
        imuCalculateEstimatedAttitude();
    } else {
        accADC[X] = 0;
        accADC[Y] = 0;
        accADC[Z] = 0;
    }
}

float calculateCosTiltAngle(void)
{
    return rMat[2][2];
}

float calculateThrottleTiltCompensationFactor(uint8_t throttleTiltCompensationStrength)
{
    if (throttleTiltCompensationStrength && rMat[2][2] >= 0.6f) {
        float tiltCompFactor = 1.0f / constrainf(rMat[2][2], 0.6f, 1.0f);  // max tilt about 50 deg
        return 1.0f + (tiltCompFactor - 1.0f) * (throttleTiltCompensationStrength / 100.f);
    }
    else {
        return 1.0f;
    }
}
