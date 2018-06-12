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

#include "platform.h"

#include "blackbox/blackbox.h"

#include "build/build_config.h"
#include "build/debug.h"

#include "common/axis.h"
#include "common/filter.h"
#include "common/maths.h"
#include "common/vector.h"
#include "common/quaternion.h"

#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/time.h"

#include "fc/config.h"
#include "fc/runtime_config.h"

#include "flight/hil.h"
#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/pid.h"

#include "io/gps.h"

#include "sensors/acceleration.h"
#include "sensors/barometer.h"
#include "sensors/compass.h"
#include "sensors/gyro.h"
#include "sensors/sensors.h"


/**
 * In Cleanflight accelerometer is aligned in the following way:
 *      X-axis = Forward
 *      Y-axis = Left
 *      Z-axis = Up
 * Our INAV uses different convention
 *      X-axis = North/Forward
 *      Y-axis = East/Right
 *      Z-axis = Up
 */

// the limit (in degrees/second) beyond which we stop integrating
// omega_I. At larger spin rates the DCM PI controller can get 'dizzy'
// which results in false gyro drift. See
// http://gentlenav.googlecode.com/files/fastRotations.pdf

#define SPIN_RATE_LIMIT             20
#define MAX_ACC_SQ_NEARNESS         25      // 25% or G^2, accepted acceleration of (0.87 - 1.12G)
#define MAX_GPS_HEADING_ERROR_DEG   60      // Amount of error between GPS CoG and estimated Yaw at witch we stop trusting GPS and fallback to MAG

FASTRAM fpVector3_t imuMeasuredAccelBF;
FASTRAM fpVector3_t imuMeasuredRotationBF;
STATIC_FASTRAM float smallAngleCosZ;

STATIC_FASTRAM bool isAccelUpdatedAtLeastOnce;
STATIC_FASTRAM fpVector3_t vCorrectedMagNorth;             // Magnetic North vector in EF (true North rotated by declination)

FASTRAM fpQuaternion_t orientation;
FASTRAM attitudeEulerAngles_t attitude;             // absolute angle inclination in multiple of 0.1 degree    180 deg = 1800
FASTRAM float rMat[3][3];

STATIC_FASTRAM imuRuntimeConfig_t imuRuntimeConfig;

STATIC_FASTRAM bool gpsHeadingInitialized;

PG_REGISTER_WITH_RESET_TEMPLATE(imuConfig_t, imuConfig, PG_IMU_CONFIG, 0);

PG_RESET_TEMPLATE(imuConfig_t, imuConfig,
    .dcm_kp_acc = 2500,             // 0.25 * 10000
    .dcm_ki_acc = 50,               // 0.005 * 10000
    .dcm_kp_mag = 10000,            // 1.00 * 10000
    .dcm_ki_mag = 0,                // 0.00 * 10000
    .small_angle = 25
);

STATIC_UNIT_TESTED void imuComputeRotationMatrix(void)
{
    float q1q1 = orientation.q1 * orientation.q1;
    float q2q2 = orientation.q2 * orientation.q2;
    float q3q3 = orientation.q3 * orientation.q3;

    float q0q1 = orientation.q0 * orientation.q1;
    float q0q2 = orientation.q0 * orientation.q2;
    float q0q3 = orientation.q0 * orientation.q3;
    float q1q2 = orientation.q1 * orientation.q2;
    float q1q3 = orientation.q1 * orientation.q3;
    float q2q3 = orientation.q2 * orientation.q3;

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

void imuConfigure(void)
{
    imuRuntimeConfig.dcm_kp_acc = imuConfig()->dcm_kp_acc / 10000.0f;
    imuRuntimeConfig.dcm_ki_acc = imuConfig()->dcm_ki_acc / 10000.0f;
    imuRuntimeConfig.dcm_kp_mag = imuConfig()->dcm_kp_mag / 10000.0f;
    imuRuntimeConfig.dcm_ki_mag = imuConfig()->dcm_ki_mag / 10000.0f;
    imuRuntimeConfig.small_angle = imuConfig()->small_angle;
}

void imuInit(void)
{
    smallAngleCosZ = cos_approx(degreesToRadians(imuRuntimeConfig.small_angle));

    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        imuMeasuredAccelBF.v[axis] = 0;
    }

    // Explicitly initialize FASTRAM statics
    isAccelUpdatedAtLeastOnce = false;
    gpsHeadingInitialized = false;

    // Create magnetic declination matrix
    const int deg = compassConfig()->mag_declination / 100;
    const int min = compassConfig()->mag_declination   % 100;
    imuSetMagneticDeclination(deg + min / 60.0f);

    quaternionInitUnit(&orientation);
    imuComputeRotationMatrix();
}

void imuSetMagneticDeclination(float declinationDeg)
{
    const float declinationRad = -DEGREES_TO_RADIANS(declinationDeg);
    vCorrectedMagNorth.x = cos_approx(declinationRad);
    vCorrectedMagNorth.y = sin_approx(declinationRad);
    vCorrectedMagNorth.z = 0;
}

void imuTransformVectorBodyToEarth(fpVector3_t * v)
{
    // From body frame to earth frame
    quaternionRotateVectorInv(v, v, &orientation);

    // HACK: This is needed to correctly transform from NED (sensor frame) to NEU (navigation)
    v->y = -v->y;
}

void imuTransformVectorEarthToBody(fpVector3_t * v)
{
    // HACK: This is needed to correctly transform from NED (sensor frame) to NEU (navigation)
    v->y = -v->y;

    // From earth frame to body frame
    quaternionRotateVector(v, v, &orientation);
}

#if defined(USE_GPS) || defined(HIL)
STATIC_UNIT_TESTED void imuComputeQuaternionFromRPY(int16_t initialRoll, int16_t initialPitch, int16_t initialYaw)
{
    if (initialRoll > 1800) initialRoll -= 3600;
    if (initialPitch > 1800) initialPitch -= 3600;
    if (initialYaw > 1800) initialYaw -= 3600;

    const float cosRoll = cos_approx(DECIDEGREES_TO_RADIANS(initialRoll) * 0.5f);
    const float sinRoll = sin_approx(DECIDEGREES_TO_RADIANS(initialRoll) * 0.5f);

    const float cosPitch = cos_approx(DECIDEGREES_TO_RADIANS(initialPitch) * 0.5f);
    const float sinPitch = sin_approx(DECIDEGREES_TO_RADIANS(initialPitch) * 0.5f);

    const float cosYaw = cos_approx(DECIDEGREES_TO_RADIANS(-initialYaw) * 0.5f);
    const float sinYaw = sin_approx(DECIDEGREES_TO_RADIANS(-initialYaw) * 0.5f);

    orientation.q0 = cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw;
    orientation.q1 = sinRoll * cosPitch * cosYaw - cosRoll * sinPitch * sinYaw;
    orientation.q2 = cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw;
    orientation.q3 = cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw;

    imuComputeRotationMatrix();
}
#endif

static bool imuUseFastGains(void)
{
    return !ARMING_FLAG(ARMED) && millis() < 20000;
}

static float imuGetPGainScaleFactor(void)
{
    if (imuUseFastGains()) {
        return 10.0f;
    }
    else {
        return 1.0f;
    }
}

static void imuResetOrientationQuaternion(const fpVector3_t * accBF)
{
    const float accNorm = sqrtf(vectorNormSquared(accBF));

    orientation.q0 = accBF->z + accNorm;
    orientation.q1 = accBF->y;
    orientation.q2 = -accBF->x;
    orientation.q3 = 0.0f;

    quaternionNormalize(&orientation, &orientation);
}

static void imuCheckAndResetOrientationQuaternion(const fpVector3_t * accBF)
{
    // Check if some calculation in IMU update yield NAN or zero quaternion
    // Reset quaternion from accelerometer - this might be incorrect, but it's better than no attitude at all
    const float check = fabs(orientation.q0) + fabs(orientation.q1) + fabs(orientation.q2) + fabs(orientation.q3);

    if (!isnan(check) && !isinf(check)) {
        return;
    }

    const float normSq = quaternionNormSqared(&orientation);
    if (normSq > (1.0f - 1e-6f) && normSq < (1.0f + 1e-6f)) {
        return;
    }

    imuResetOrientationQuaternion(accBF);
    DEBUG_TRACE("AHRS orientation quaternion error");

#ifdef USE_BLACKBOX
    if (feature(FEATURE_BLACKBOX)) {
        blackboxLogEvent(FLIGHT_LOG_EVENT_IMU_FAILURE, NULL);
    }
#endif
}

static void imuMahonyAHRSupdate(float dt, const fpVector3_t * gyroBF, const fpVector3_t * accBF, const fpVector3_t * magBF, bool useCOG, float courseOverGround)
{
    STATIC_FASTRAM fpVector3_t vGyroDriftEstimate = { 0 };

    fpVector3_t vRotation = *gyroBF;

    /* Calculate general spin rate (rad/s) */
    const float spin_rate_sq = vectorNormSquared(&vRotation);

    /* Step 1: Yaw correction */
    // Use measured magnetic field vector
    if (magBF || useCOG) {
        static const fpVector3_t vForward = { .v = { 1.0f, 0.0f, 0.0f } };

        fpVector3_t vErr = { .v = { 0.0f, 0.0f, 0.0f } };

        if (magBF && vectorNormSquared(magBF) > 0.01f) {
            fpVector3_t vMag;

            // For magnetometer correction we make an assumption that magnetic field is perpendicular to gravity (ignore Z-component in EF).
            // This way magnetic field will only affect heading and wont mess roll/pitch angles

            // (hx; hy; 0) - measured mag field vector in EF (assuming Z-component is zero)
            // This should yield direction to magnetic North (1; 0; 0)
            quaternionRotateVectorInv(&vMag, magBF, &orientation);    // BF -> EF

            // Ignore magnetic inclination
            vMag.z = 0.0f;

            // Normalize to unit vector
            vectorNormalize(&vMag, &vMag);

            // Reference mag field vector heading is Magnetic North in EF. We compute that by rotating True North vector by declination and assuming Z-component is zero
            // magnetometer error is cross product between estimated magnetic north and measured magnetic north (calculated in EF)
            vectorCrossProduct(&vErr, &vMag, &vCorrectedMagNorth);

            // Rotate error back into body frame
            quaternionRotateVector(&vErr, &vErr, &orientation);
        }
        else if (useCOG) {
            fpVector3_t vHeadingEF;

            // Use raw heading error (from GPS or whatever else)
            while (courseOverGround >  M_PIf) courseOverGround -= (2.0f * M_PIf);
            while (courseOverGround < -M_PIf) courseOverGround += (2.0f * M_PIf);

            // William Premerlani and Paul Bizard, Direction Cosine Matrix IMU - Eqn. 22-23
            // (Rxx; Ryx) - measured (estimated) heading vector (EF)
            // (-cos(COG), sin(COG)) - reference heading vector (EF)

            // Compute heading vector in EF from scalar CoG
            fpVector3_t vCoG = { .v = { -cos_approx(courseOverGround), sin_approx(courseOverGround), 0.0f } };

            // Rotate Forward vector from BF to EF - will yield Heading vector in Earth frame
            quaternionRotateVectorInv(&vHeadingEF, &vForward, &orientation);
            vHeadingEF.z = 0.0f;

            // Normalize to unit vector
            vectorNormalize(&vHeadingEF, &vHeadingEF);

            // error is cross product between reference heading and estimated heading (calculated in EF)
            vectorCrossProduct(&vErr, &vCoG, &vHeadingEF);

            // Rotate error back into body frame
            quaternionRotateVector(&vErr, &vErr, &orientation);
        }

        // Compute and apply integral feedback if enabled
        if (imuRuntimeConfig.dcm_ki_mag > 0.0f) {
            // Stop integrating if spinning beyond the certain limit
            if (spin_rate_sq < sq(DEGREES_TO_RADIANS(SPIN_RATE_LIMIT))) {
                fpVector3_t vTmp;

                // integral error scaled by Ki
                vectorScale(&vTmp, &vErr, imuRuntimeConfig.dcm_ki_mag * dt);
                vectorAdd(&vGyroDriftEstimate, &vGyroDriftEstimate, &vTmp);
            }
        }

        // Calculate kP gain and apply proportional feedback
        vectorScale(&vErr, &vErr, imuRuntimeConfig.dcm_kp_mag * imuGetPGainScaleFactor());
        vectorAdd(&vRotation, &vRotation, &vErr);
    }


    /* Step 2: Roll and pitch correction -  use measured acceleration vector */
    if (accBF) {
        static const fpVector3_t vGravity = { .v = { 0.0f, 0.0f, 1.0f } };
        fpVector3_t vEstGravity, vAcc, vErr;

        // Calculate estimated gravity vector in body frame
        quaternionRotateVector(&vEstGravity, &vGravity, &orientation);    // EF -> BF

        // Error is sum of cross product between estimated direction and measured direction of gravity
        vectorNormalize(&vAcc, accBF);
        vectorCrossProduct(&vErr, &vAcc, &vEstGravity);

        // Compute and apply integral feedback if enabled
        if (imuRuntimeConfig.dcm_ki_acc > 0.0f) {
            // Stop integrating if spinning beyond the certain limit
            if (spin_rate_sq < sq(DEGREES_TO_RADIANS(SPIN_RATE_LIMIT))) {
                fpVector3_t vTmp;

                // integral error scaled by Ki
                vectorScale(&vTmp, &vErr, imuRuntimeConfig.dcm_ki_acc * dt);
                vectorAdd(&vGyroDriftEstimate, &vGyroDriftEstimate, &vTmp);
            }
        }

        // Calculate kP gain and apply proportional feedback
        vectorScale(&vErr, &vErr, imuRuntimeConfig.dcm_kp_acc * imuGetPGainScaleFactor());
        vectorAdd(&vRotation, &vRotation, &vErr);
    }

    // Apply gyro drift correction
    vectorAdd(&vRotation, &vRotation, &vGyroDriftEstimate);

    // Integrate rate of change of quaternion
    fpVector3_t vTheta;
    fpQuaternion_t deltaQ;

    vectorScale(&vTheta, &vRotation, 0.5f * dt);
    quaternionInitFromVector(&deltaQ, &vTheta);
    const float thetaMagnitudeSq = vectorNormSquared(&vTheta);

    // If calculated rotation is zero - don't update quaternion
    if (thetaMagnitudeSq >= 1e-20) {
        // Calculate quaternion delta:
        // Theta is a axis/angle rotation. Direction of a vector is axis, magnitude is angle/2.
        // Proper quaternion from axis/angle involves computing sin/cos, but the formula becomes numerically unstable as Theta approaches zero.
        // For near-zero cases we use the first 3 terms of the Taylor series expansion for sin/cos. We check if fourth term is less than machine precision -
        // then we can safely use the "low angle" approximated version without loss of accuracy.
        if (thetaMagnitudeSq < sqrtf(24.0f * 1e-6f)) {
            quaternionScale(&deltaQ, &deltaQ, 1.0f - thetaMagnitudeSq / 6.0f);
            deltaQ.q0 = 1.0f - thetaMagnitudeSq / 2.0f;
        }
        else {
            const float thetaMagnitude = sqrtf(thetaMagnitudeSq);
            quaternionScale(&deltaQ, &deltaQ, sin_approx(thetaMagnitude) / thetaMagnitude);
            deltaQ.q0 = cos_approx(thetaMagnitude);
        }

        // Calculate final orientation and renormalize
        quaternionMultiply(&orientation, &orientation, &deltaQ);
        quaternionNormalize(&orientation, &orientation);
    }

    // Check for invalid quaternion
    imuCheckAndResetOrientationQuaternion(accBF);

    // Pre-compute rotation matrix from quaternion
    imuComputeRotationMatrix();
}

STATIC_UNIT_TESTED void imuUpdateEulerAngles(void)
{
    /* Compute pitch/roll angles */
    attitude.values.roll = RADIANS_TO_DECIDEGREES(atan2_approx(rMat[2][1], rMat[2][2]));
    attitude.values.pitch = RADIANS_TO_DECIDEGREES((0.5f * M_PIf) - acos_approx(-rMat[2][0]));
    attitude.values.yaw = RADIANS_TO_DECIDEGREES(-atan2_approx(rMat[1][0], rMat[0][0]));

    if (attitude.values.yaw < 0)
        attitude.values.yaw += 3600;

    /* Update small angle state */
    if (calculateCosTiltAngle() > smallAngleCosZ) {
        ENABLE_STATE(SMALL_ANGLE);
    } else {
        DISABLE_STATE(SMALL_ANGLE);
    }
}

static bool imuCanUseAccelerometerForCorrection(void)
{
    float accMagnitudeSq = 0;

    for (int axis = 0; axis < 3; axis++) {
        accMagnitudeSq += acc.accADCf[axis] * acc.accADCf[axis];
    }

    // Magnitude^2 in percent of G^2
    const float nearness = ABS(100 - (accMagnitudeSq * 100));

    return (nearness > MAX_ACC_SQ_NEARNESS) ? false : true;
}

static void imuCalculateEstimatedAttitude(float dT)
{
#if defined(USE_MAG)
    const bool canUseMAG = sensors(SENSOR_MAG) && compassIsHealthy();
#else
    const bool canUseMAG = false;
#endif

    const bool useAcc = imuCanUseAccelerometerForCorrection();

    float courseOverGround = 0;
    bool useMag = false;
    bool useCOG = false;

#if defined(USE_GPS)
    if (STATE(FIXED_WING)) {
        bool canUseCOG = isGPSHeadingValid();

        if (canUseCOG) {
            if (gpsHeadingInitialized) {
                // Use GPS heading if error is acceptable or if it's the only source of heading
                if (ABS(gpsSol.groundCourse - attitude.values.yaw) < DEGREES_TO_DECIDEGREES(MAX_GPS_HEADING_ERROR_DEG) || !canUseMAG) {
                    courseOverGround = DECIDEGREES_TO_RADIANS(gpsSol.groundCourse);
                    useCOG = true;
                }
            }
            else {
                // Re-initialize quaternion from known Roll, Pitch and GPS heading
                imuComputeQuaternionFromRPY(attitude.values.roll, attitude.values.pitch, gpsSol.groundCourse);
                gpsHeadingInitialized = true;

                // Force reset of heading hold target
                resetHeadingHoldTarget(DECIDEGREES_TO_DEGREES(attitude.values.yaw));
            }

            // If we can't use COG and there's MAG available - fallback
            if (!useCOG && canUseMAG) {
                useMag = true;
            }
        }
        else if (canUseMAG) {
            useMag = true;
            gpsHeadingInitialized = true;   // GPS heading initialised from MAG, continue on GPS if possible
        }
    }
    else {
        // Multicopters don't use GPS heading
        if (canUseMAG) {
            useMag = true;
        }
    }
#else
    // In absence of GPS MAG is the only option
    if (canUseMAG) {
        useMag = true;
    }
#endif

    fpVector3_t measuredMagBF = { .v = { mag.magADC[X], mag.magADC[Y], mag.magADC[Z] } };

    imuMahonyAHRSupdate(dT, &imuMeasuredRotationBF, 
                            useAcc ? &imuMeasuredAccelBF : NULL, 
                            useMag ? &measuredMagBF : NULL, 
                            useCOG, courseOverGround);

    imuUpdateEulerAngles();
}

#ifdef HIL
void imuHILUpdate(void)
{
    /* Set attitude */
    attitude.values.roll = hilToFC.rollAngle;
    attitude.values.pitch = hilToFC.pitchAngle;
    attitude.values.yaw = hilToFC.yawAngle;

    /* Compute rotation quaternion for future use */
    imuComputeQuaternionFromRPY(attitude.values.roll, attitude.values.pitch, attitude.values.yaw);

    /* Fake accADC readings */
    accADCf[X] = hilToFC.bodyAccel[X] / GRAVITY_CMSS;
    accADCf[Y] = hilToFC.bodyAccel[Y] / GRAVITY_CMSS;
    accADCf[Z] = hilToFC.bodyAccel[Z] / GRAVITY_CMSS;
}
#endif

void imuUpdateAccelerometer(void)
{
#ifdef HIL
    if (sensors(SENSOR_ACC) && !hilActive) {
        accUpdate();
        isAccelUpdatedAtLeastOnce = true;
    }
#else
    if (sensors(SENSOR_ACC)) {
        accUpdate();
        isAccelUpdatedAtLeastOnce = true;
    }
#endif
}

void imuUpdateAttitude(timeUs_t currentTimeUs)
{
    /* Calculate dT */
    static timeUs_t previousIMUUpdateTimeUs;
    const float dT = (currentTimeUs - previousIMUUpdateTimeUs) * 1e-6;
    previousIMUUpdateTimeUs = currentTimeUs;

    if (sensors(SENSOR_ACC) && isAccelUpdatedAtLeastOnce) {
#ifdef HIL
        if (!hilActive) {
            gyroGetMeasuredRotationRate(&imuMeasuredRotationBF);    // Calculate gyro rate in body frame in rad/s
            accGetMeasuredAcceleration(&imuMeasuredAccelBF);  // Calculate accel in body frame in cm/s/s
            imuCalculateEstimatedAttitude(dT);  // Update attitude estimate
        }
        else {
            imuHILUpdate();
            imuUpdateMeasuredAcceleration();
        }
#else
        gyroGetMeasuredRotationRate(&imuMeasuredRotationBF);    // Calculate gyro rate in body frame in rad/s
        accGetMeasuredAcceleration(&imuMeasuredAccelBF);  // Calculate accel in body frame in cm/s/s
        imuCalculateEstimatedAttitude(dT);  // Update attitude estimate
#endif
    } else {
        acc.accADCf[X] = 0.0f;
        acc.accADCf[Y] = 0.0f;
        acc.accADCf[Z] = 0.0f;
    }
}

bool isImuReady(void)
{
    return sensors(SENSOR_ACC) && gyroIsCalibrationComplete();
}

bool isImuHeadingValid(void)
{
    return (sensors(SENSOR_MAG) && STATE(COMPASS_CALIBRATED)) || (STATE(FIXED_WING) && gpsHeadingInitialized);
}

float calculateCosTiltAngle(void)
{
    return 1.0f - 2.0f * sq(orientation.q1) - 2.0f * sq(orientation.q2);
}
