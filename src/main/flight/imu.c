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

FILE_COMPILE_FOR_SPEED

#include "blackbox/blackbox.h"

#include "build/build_config.h"
#include "build/debug.h"

#include "common/axis.h"
#include "common/filter.h"
#include "common/log.h"
#include "common/maths.h"
#include "common/vector.h"
#include "common/quaternion.h"

#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/time.h"

#include "fc/config.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

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


/*
 *      X-axis = North/Forward
 *      Y-axis = East/Right
 *      Z-axis = Up
 */

// the limit (in degrees/second) beyond which we stop integrating
// omega_I. At larger spin rates the DCM PI controller can get 'dizzy'
// which results in false gyro drift. See
// http://gentlenav.googlecode.com/files/fastRotations.pdf

#define SPIN_RATE_LIMIT             20
#define MAX_ACC_NEARNESS            0.33    // 33% or G error soft-accepted (0.67-1.33G)
#define IMU_CENTRIFUGAL_LPF         1       // Hz

FASTRAM fpVector3_t imuMeasuredAccelBF;
FASTRAM fpVector3_t imuMeasuredRotationBF;
STATIC_FASTRAM float smallAngleCosZ;

STATIC_FASTRAM bool isAccelUpdatedAtLeastOnce;
STATIC_FASTRAM fpVector3_t vCorrectedMagNorth;             // Magnetic North vector in EF (true North rotated by declination)

FASTRAM fpQuaternion_t orientation;
FASTRAM attitudeEulerAngles_t attitude;             // absolute angle inclination in multiple of 0.1 degree    180 deg = 1800
FASTRAM float rMat[3][3];

STATIC_FASTRAM imuRuntimeConfig_t imuRuntimeConfig;
STATIC_FASTRAM pt1Filter_t rotRateFilter;

STATIC_FASTRAM bool gpsHeadingInitialized;

PG_REGISTER_WITH_RESET_TEMPLATE(imuConfig_t, imuConfig, PG_IMU_CONFIG, 2);

PG_RESET_TEMPLATE(imuConfig_t, imuConfig,
    .dcm_kp_acc = SETTING_IMU_DCM_KP_DEFAULT,                   // 0.25 * 10000
    .dcm_ki_acc = SETTING_IMU_DCM_KI_DEFAULT,                   // 0.005 * 10000
    .dcm_kp_mag = SETTING_IMU_DCM_KP_MAG_DEFAULT,               // 1.00 * 10000
    .dcm_ki_mag = SETTING_IMU_DCM_KI_MAG_DEFAULT,               // 0.00 * 10000
    .small_angle = SETTING_SMALL_ANGLE_DEFAULT,
    .acc_ignore_rate = SETTING_IMU_ACC_IGNORE_RATE_DEFAULT,
    .acc_ignore_slope = SETTING_IMU_ACC_IGNORE_SLOPE_DEFAULT
);

float DeclinationDeg;

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
/*
    // Explicitly initialize FASTRAM statics
    isAccelUpdatedAtLeastOnce = false;
    gpsHeadingInitialized = false;

    // Create magnetic declination matrix
#ifdef USE_MAG
    const int deg = compassConfig()->mag_declination / 100;
    const int min = compassConfig()->mag_declination   % 100;
#else
    const int deg = 0;
    const int min = 0;
#endif
    imuSetMagneticDeclination(deg + min / 60.0f);

    quaternionInitUnit(&orientation);
    imuComputeRotationMatrix();

    // Initialize rotation rate filter
    pt1FilterReset(&rotRateFilter, 0);
    */

    rMat[0][0] = 1.0f;
    rMat[0][1] = 0.0f;
    rMat[0][2] = 0.0f;

    rMat[1][0] = 0.0f;
    rMat[1][1] = 1.0f;
    rMat[1][2] = 0.0f;

    rMat[2][0] = 0.0f;
    rMat[2][1] = 0.0f;
    rMat[2][2] = 1.0f;
}

void imuSetMagneticDeclination(float declinationDeg)
{
    DeclinationDeg = declinationDeg;
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
    const float accNorm = fast_fsqrtf(vectorNormSquared(accBF));

    orientation.q0 = accBF->z + accNorm;
    orientation.q1 = accBF->y;
    orientation.q2 = -accBF->x;
    orientation.q3 = 0.0f;

    quaternionNormalize(&orientation, &orientation);
}

static bool imuValidateQuaternion(const fpQuaternion_t * quat)
{
    const float check = fabs(quat->q0) + fabs(quat->q1) + fabs(quat->q2) + fabs(quat->q3);

    if (!isnan(check) && !isinf(check)) {
        return true;
    }

    const float normSq = quaternionNormSqared(&orientation);
    if (normSq > (1.0f - 1e-6f) && normSq < (1.0f + 1e-6f)) {
        return true;
    }

    return false;
}

static void imuCheckAndResetOrientationQuaternion(const fpQuaternion_t * quat, const fpVector3_t * accBF)
{
    // Check if some calculation in IMU update yield NAN or zero quaternion
    if (imuValidateQuaternion(&orientation)) {
        return;
    }

    flightLogEvent_IMUError_t imuErrorEvent;

    // Orientation is invalid. We need to reset it
    if (imuValidateQuaternion(quat)) {
        // Previous quaternion valid. Reset to it
        orientation = *quat;
        imuErrorEvent.errorCode = 1;
        LOG_E(IMU, "AHRS orientation quaternion error. Reset to last known good value");
    }
    else {
        // No valid reference. Best guess from accelerometer
        imuResetOrientationQuaternion(accBF);
        imuErrorEvent.errorCode = 2;
        LOG_E(IMU, "AHRS orientation quaternion error. Best guess from ACC");
    }

#ifdef USE_BLACKBOX
    if (feature(FEATURE_BLACKBOX)) {
        blackboxLogEvent(FLIGHT_LOG_EVENT_IMU_FAILURE, (flightLogEventData_t*)&imuErrorEvent);
    }
#endif
}

static void imuMahonyAHRSupdate(float dt, const fpVector3_t * gyroBF, const fpVector3_t * accBF, const fpVector3_t * magBF, bool useCOG, float courseOverGround, float accWScaler, float magWScaler)
{
    STATIC_FASTRAM fpVector3_t vGyroDriftEstimate = { 0 };

    fpQuaternion_t prevOrientation = orientation;
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

            // We zeroed out vMag.z -  make sure the whole vector didn't go to zero
            if (vectorNormSquared(&vMag) > 0.01f) {
                // Normalize to unit vector
                vectorNormalize(&vMag, &vMag);

                // Reference mag field vector heading is Magnetic North in EF. We compute that by rotating True North vector by declination and assuming Z-component is zero
                // magnetometer error is cross product between estimated magnetic north and measured magnetic north (calculated in EF)
                vectorCrossProduct(&vErr, &vMag, &vCorrectedMagNorth);

                // Rotate error back into body frame
                quaternionRotateVector(&vErr, &vErr, &orientation);
            }
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

            // We zeroed out vHeadingEF.z -  make sure the whole vector didn't go to zero
            if (vectorNormSquared(&vHeadingEF) > 0.01f) {
                // Normalize to unit vector
                vectorNormalize(&vHeadingEF, &vHeadingEF);

                // error is cross product between reference heading and estimated heading (calculated in EF)
                vectorCrossProduct(&vErr, &vCoG, &vHeadingEF);

                // Rotate error back into body frame
                quaternionRotateVector(&vErr, &vErr, &orientation);
            }
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
        vectorScale(&vErr, &vErr, imuRuntimeConfig.dcm_kp_mag * magWScaler);
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
        vectorScale(&vErr, &vErr, imuRuntimeConfig.dcm_kp_acc * accWScaler);
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
        if (thetaMagnitudeSq < fast_fsqrtf(24.0f * 1e-6f)) {
            quaternionScale(&deltaQ, &deltaQ, 1.0f - thetaMagnitudeSq / 6.0f);
            deltaQ.q0 = 1.0f - thetaMagnitudeSq / 2.0f;
        }
        else {
            const float thetaMagnitude = fast_fsqrtf(thetaMagnitudeSq);
            quaternionScale(&deltaQ, &deltaQ, sin_approx(thetaMagnitude) / thetaMagnitude);
            deltaQ.q0 = cos_approx(thetaMagnitude);
        }

        // Calculate final orientation and renormalize
        quaternionMultiply(&orientation, &orientation, &deltaQ);
        quaternionNormalize(&orientation, &orientation);
    }

    // Check for invalid quaternion and reset to previous known good one
    imuCheckAndResetOrientationQuaternion(&prevOrientation, accBF);

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

static float imuCalculateAccelerometerWeight(const float dT)
{
    float accMagnitudeSq = 0;
    for (int axis = 0; axis < 3; axis++) {
        accMagnitudeSq += acc.accADCf[axis] * acc.accADCf[axis];
    }

    const float accWeight_Nearness = bellCurve(fast_fsqrtf(accMagnitudeSq) - 1.0f, MAX_ACC_NEARNESS);

    // Experiment: if rotation rate on a FIXED_WING_LEGACY is higher than a threshold - centrifugal force messes up too much and we
    // should not use measured accel for AHRS comp
    //      Centrifugal acceleration AccelC = Omega^2 * R = Speed^2 / R
    //          Omega = Speed / R
    //      For a banked turn R = Speed^2 / (G * tan(Roll))
    //          Omega = G * tan(Roll) / Speed
    //      Knowing the typical airspeed is around ~20 m/s we can calculate roll angles that yield certain angular rate
    //          1 deg   =>  0.49 deg/s
    //          2 deg   =>  0.98 deg/s
    //          5 deg   =>  2.45 deg/s
    //         10 deg   =>  4.96 deg/s
    //      Therefore for a typical plane a sustained angular rate of ~2.45 deg/s will yield a banking error of ~5 deg
    //  Since we can't do proper centrifugal compensation at the moment we pass the magnitude of angular rate through an
    //  LPF with a low cutoff and if it's larger than our threshold - invalidate accelerometer

    // Default - don't apply rate/ignore scaling
    float accWeight_RateIgnore = 1.0f;

    if (ARMING_FLAG(ARMED) && STATE(FIXED_WING_LEGACY) && imuConfig()->acc_ignore_rate) {
        const float rotRateMagnitude = calc_length_pythagorean_2D(imuMeasuredRotationBF.y, imuMeasuredRotationBF.z);
        const float rotRateMagnitudeFiltered = pt1FilterApply4(&rotRateFilter, rotRateMagnitude, IMU_CENTRIFUGAL_LPF, dT);

        if (imuConfig()->acc_ignore_slope) {
            const float rateSlopeMin = DEGREES_TO_RADIANS((imuConfig()->acc_ignore_rate - imuConfig()->acc_ignore_slope));
            const float rateSlopeMax = DEGREES_TO_RADIANS((imuConfig()->acc_ignore_rate + imuConfig()->acc_ignore_slope));

            accWeight_RateIgnore = scaleRangef(constrainf(rotRateMagnitudeFiltered, rateSlopeMin, rateSlopeMax), rateSlopeMin, rateSlopeMax, 1.0f, 0.0f);
        }
        else {
            if (rotRateMagnitudeFiltered > DEGREES_TO_RADIANS(imuConfig()->acc_ignore_rate)) {
                accWeight_RateIgnore = 0.0f;
            }
        }
    }

    return accWeight_Nearness * accWeight_RateIgnore;
}

static void imuCalculateEstimatedAttitude(float dT)
{
#if defined(USE_MAG)
    const bool canUseMAG = sensors(SENSOR_MAG) && compassIsHealthy();
#else
    const bool canUseMAG = false;
#endif

    float courseOverGround = 0;
    bool useMag = false;
    bool useCOG = false;

#if defined(USE_GPS)
    if (STATE(FIXED_WING_LEGACY)) {
        bool canUseCOG = isGPSHeadingValid();

        // Prefer compass (if available)
        if (canUseMAG) {
            useMag = true;
            gpsHeadingInitialized = true;   // GPS heading initialised from MAG, continue on GPS if compass fails
        }
        else if (canUseCOG) {
            if (gpsHeadingInitialized) {
                courseOverGround = DECIDEGREES_TO_RADIANS(gpsSol.groundCourse);
                useCOG = true;
            }
            else {
                // Re-initialize quaternion from known Roll, Pitch and GPS heading
                imuComputeQuaternionFromRPY(attitude.values.roll, attitude.values.pitch, gpsSol.groundCourse);
                gpsHeadingInitialized = true;

                // Force reset of heading hold target
                resetHeadingHoldTarget(DECIDEGREES_TO_DEGREES(attitude.values.yaw));
            }
        } else if (!ARMING_FLAG(ARMED)) {
            gpsHeadingInitialized = false;
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

    const float magWeight = imuGetPGainScaleFactor() * 1.0f;
    const float accWeight = imuGetPGainScaleFactor() * imuCalculateAccelerometerWeight(dT);
    const bool useAcc = (accWeight > 0.001f);

    imuMahonyAHRSupdate(dT, &imuMeasuredRotationBF,
                            useAcc ? &imuMeasuredAccelBF : NULL,
                            useMag ? &measuredMagBF : NULL,
                            useCOG, courseOverGround,
                            accWeight,
                            magWeight);

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

void imuCheckVibrationLevels(void)
{
    fpVector3_t accVibeLevels;

    accGetVibrationLevels(&accVibeLevels);
    const uint32_t accClipCount = accGetClipCount();

    DEBUG_SET(DEBUG_VIBE, 0, accVibeLevels.x * 100);
    DEBUG_SET(DEBUG_VIBE, 1, accVibeLevels.y * 100);
    DEBUG_SET(DEBUG_VIBE, 2, accVibeLevels.z * 100);
    DEBUG_SET(DEBUG_VIBE, 3, accClipCount);
    // DEBUG_VIBE values 4-7 are used by NAV estimator
}

void update(float delta_t);

void imuUpdateAttitude(timeUs_t currentTimeUs)
{
    /* Calculate dT */
    static timeUs_t previousIMUUpdateTimeUs;
    const float dT = (currentTimeUs - previousIMUUpdateTimeUs) * 1e-6;
    previousIMUUpdateTimeUs = currentTimeUs;
    
    if (sensors(SENSOR_ACC) && isAccelUpdatedAtLeastOnce) {
        gyroGetMeasuredRotationRate(&imuMeasuredRotationBF);    // Calculate gyro rate in body frame in rad/s
        accGetMeasuredAcceleration(&imuMeasuredAccelBF);  // Calculate accel in body frame in cm/s/s
        imuCheckVibrationLevels();
        update(dT);
    } else {
        acc.accADCf[X] = 0.0f;
        acc.accADCf[Y] = 0.0f;
        acc.accADCf[Z] = 0.0f;
    }

/*
    if (sensors(SENSOR_ACC) && isAccelUpdatedAtLeastOnce) {
#ifdef HIL
        if (!hilActive) {
            gyroGetMeasuredRotationRate(&imuMeasuredRotationBF);    // Calculate gyro rate in body frame in rad/s
            accGetMeasuredAcceleration(&imuMeasuredAccelBF);  // Calculate accel in body frame in cm/s/s
            imuCheckVibrationLevels();
            imuCalculateEstimatedAttitude(dT);  // Update attitude estimate
        }
        else {
            imuHILUpdate();
            imuUpdateMeasuredAcceleration();
        }
#else
        gyroGetMeasuredRotationRate(&imuMeasuredRotationBF);    // Calculate gyro rate in body frame in rad/s
        accGetMeasuredAcceleration(&imuMeasuredAccelBF);  // Calculate accel in body frame in cm/s/s
        imuCheckVibrationLevels();
        imuCalculateEstimatedAttitude(dT);  // Update attitude estimate
#endif
    } else {
        acc.accADCf[X] = 0.0f;
        acc.accADCf[Y] = 0.0f;
        acc.accADCf[Z] = 0.0f;
    }
    */
}

bool isImuReady(void)
{
    return sensors(SENSOR_ACC) && gyroIsCalibrationComplete();
}

bool isImuHeadingValid(void)
{
    return (sensors(SENSOR_MAG) && STATE(COMPASS_CALIBRATED)) || (STATE(FIXED_WING_LEGACY) && gpsHeadingInitialized);
}



















#include <string.h>

fpVector3_t _omega;
fpVector3_t _omega_I;
fpVector3_t _omega_I_sum;
fpVector3_t _omega_P;
fpVector3_t _omega_yaw_P;
fpVector3_t _ra_sum;
fpVector3_t _ra_delay_buffer;
fpVector3_t _last_velocity;
fpVector3_t _accel_ef;
fpVector3_t _wind;
fpVector3_t _last_fuse;
fpVector3_t _last_vel;

bool _have_gps_lock;
bool have_initial_yaw;

float _omega_I_sum_time;
float _ra_deltat;
float _last_airspeed;

// euler angles
float _roll;
float _pitch;
float _yaw;

// helper trig variables
float _cos_roll = 1.0f;
float _cos_pitch = 1.0f;
float _cos_yaw = 1.0f;
float _sin_roll;
float _sin_pitch;
float _sin_yaw;

uint32_t _last_startup_ms;
uint32_t _ra_sum_start;
uint32_t _last_failure_ms;
uint32_t _last_consistent_heading;
uint32_t _gps_last_update;
uint32_t _last_wind_time;

#define AP_AHRS_RP_P_MIN 0.05f // minimum value for AHRS_RP_P parameter
#define AP_AHRS_YAW_P_MIN 0.05f // minimum value for AHRS_YAW_P parameter

// this is the speed in m/s above which we first get a yaw lock with
// the GPS
#define GPS_SPEED_MIN 3

// the limit (in degrees/second) beyond which we stop integrating
// omega_I. At larger spin rates the DCM PI controller can get 'dizzy'
// which results in false gyro drift. See
// http://gentlenav.googlecode.com/files/fastRotations.pdf
#define SPIN_RATE_LIMIT 20

// not configurable
static float _ki = 0.0087f;
static  float _ki_yaw = 0.01f;

// configurable
float _kp = 0.2f;
float _kp_yaw = 0.2f;
float gps_gain = 1.0f;
float beta = 0.1f;

// reset the current gyro drift estimate
//  should be called if gyro offsets are recalculated
void reset_gyro_drift(void)
{
    _omega.x = 0.0f;
    _omega.y = 0.0f;
    _omega.z = 0.0f;
    _omega_I_sum.x = 0.0f;
    _omega_I_sum.y = 0.0f;
    _omega_I_sum.z = 0.0f;
    _omega_I_sum_time = 0.0f;
}

bool rotationMatrixIsNAN(void) {

    bool isNaN = false;

    for (uint8_t i = 0; i < 3; i++) {
        for (uint8_t ii = 0; ii < 3; ii++) {
            if (isnan(rMat[i][ii])) {
                isNaN = true;
                break;
            }
        }
    }

    return isNaN;
}

// create a rotation matrix given some euler angles
// this is based on http://gentlenav.googlecode.com/files/EulerAngles.pdf
void from_euler(float roll, float pitch, float yaw)
{
    const float cp = cos_approx(pitch);
    const float sp = sin_approx(pitch);
    const float sr = sin_approx(roll);
    const float cr = cos_approx(roll);
    const float sy = sin_approx(yaw);
    const float cy = cos_approx(yaw);

    rMat[0][0] = cp * cy;
    rMat[0][1] = (sr * sp * cy) - (cr * sy);
    rMat[0][2] = (cr * sp * cy) + (sr * sy);

    rMat[1][0] = cp * sy;
    rMat[1][1] = (sr * sp * sy) + (cr * cy);
    rMat[1][2] = (cr * sp * sy) - (sr * cy);

    rMat[2][0] = -sp;
    rMat[2][1] = sr * cp;
    rMat[2][2] = cr * cp;
}

// apply an additional rotation from a body frame gyro vector
// to a rotation matrix.
void rotate(const fpVector3_t g)
{
    rMat[0][0] += rMat[0][1] * g.z - rMat[0][2] * g.y;
    rMat[0][1] += rMat[0][2] * g.x - rMat[0][0] * g.z;
    rMat[0][2] += rMat[0][0] * g.y - rMat[0][1] * g.x;

    rMat[1][0] += rMat[1][1] * g.z - rMat[1][2] * g.y;
    rMat[1][1] += rMat[1][2] * g.x - rMat[1][0] * g.z;
    rMat[1][2] += rMat[1][0] * g.y - rMat[1][1] * g.x;

    rMat[2][0] += rMat[2][1] * g.z - rMat[2][2] * g.y;
    rMat[2][1] += rMat[2][2] * g.x - rMat[2][0] * g.z;
    rMat[2][2] += rMat[2][0] * g.y - rMat[2][1] * g.x;
}

// multiplication of transpose by a vector
void mul_transpose(fpVector3_t *v)
{
    v->x = rMat[0][0] * v->x + rMat[1][0] * v->y + rMat[2][0] * v->z;
    v->y = rMat[0][1] * v->x + rMat[1][1] * v->y + rMat[2][1] * v->z;
    v->z = rMat[0][2] * v->x + rMat[1][2] * v->y + rMat[2][2] * v->z;                
}

// update the DCM matrix using only the gyros
void matrix_update(float _G_Dt)
{
    // note that we do not include the P terms in _omega. This is
    // because the spin_rate is calculated from _omega.length(),
    // and including the P terms would give positive feedback into
    // the _P_gain() calculation, which can lead to a very large P value
    _omega.x = 0.0f;
    _omega.y = 0.0f;
    _omega.z = 0.0f;

    // average across first two healthy gyros. This reduces noise on
    // systems with more than one gyro. We don't use the 3rd gyro
    // unless another is unhealthy as 3rd gyro on PH2 has a lot more noise
    fpVector3_t allOmegaSum;
    
    if (_G_Dt > 0.0f) {
        _omega.x = imuMeasuredRotationBF.x;
        _omega.y = imuMeasuredRotationBF.y;
        _omega.z = imuMeasuredRotationBF.z;
        _omega.x += _omega_I.x;
        _omega.y += _omega_I.y;
        _omega.z += _omega_I.z;
        allOmegaSum.x = (_omega.x + _omega_P.x + _omega_yaw_P.x) * _G_Dt;
        allOmegaSum.y = (_omega.y + _omega_P.y + _omega_yaw_P.y) * _G_Dt;
        allOmegaSum.z = (_omega.z + _omega_P.z + _omega_yaw_P.z) * _G_Dt;
        rotate(allOmegaSum);
    }
}

/*
 *  reset the DCM matrix and omega. Used on ground start, and on
 *  extreme errors in the matrix
 */
void dcmReset(bool recover_eulers)
{
    // reset the integration terms
    _omega_I.x = 0.0f;
    _omega_I.y = 0.0f;
    _omega_I.z = 0.0f;
    _omega_P.x = 0.0f;
    _omega_P.y = 0.0f;
    _omega_P.z = 0.0f;
    _omega_yaw_P.x = 0.0f;
    _omega_yaw_P.y = 0.0f;
    _omega_yaw_P.z = 0.0f;
    _omega.x = 0.0f;
    _omega.y = 0.0f;
    _omega.z = 0.0f;

    if (recover_eulers && !isnan(_roll) && !isnan(_pitch) && !isnan(_yaw)) {
        from_euler(_roll, _pitch, _yaw);
    } else {
        // normalise the acceleration vector
        if (calc_length_pythagorean_3D(imuMeasuredAccelBF.x, imuMeasuredAccelBF.y, imuMeasuredAccelBF.z) > 5.0f) {
            // calculate initial pitch angle
            _pitch = atan2_approx(imuMeasuredAccelBF.x, calc_length_pythagorean_2D(imuMeasuredAccelBF.y, imuMeasuredAccelBF.z));
            // calculate initial roll angle
            _roll = atan2_approx(-imuMeasuredAccelBF.y, -imuMeasuredAccelBF.z);
        } else {
            // If we can't use the accel vector, then align flat
            _roll = 0.0f;
            _pitch = 0.0f;
        }
    
        from_euler(_roll, _pitch, 0.0f);
    }

    // pre-calculate some trig for CPU purposes:
    _cos_yaw = cosf(_yaw);
    _sin_yaw = sinf(_yaw);

    _last_startup_ms = millis();
}

// renormalise one vector component of the DCM matrix
// this will return false if renormalization fails
bool renorm(fpVector3_t a, fpVector3_t *result)
{
    // numerical errors will slowly build up over time in DCM,
    // causing inaccuracies. We can keep ahead of those errors
    // using the renormalization technique from the DCM IMU paper
    // (see equations 18 to 21).

    // For APM we don't bother with the taylor expansion
    // optimisation from the paper as on our 2560 CPU the cost of
    // the sqrt() is 44 microseconds, and the small time saving of
    // the taylor expansion is not worth the potential of
    // additional error buildup.

    // Note that we can get significant renormalisation values
    // when we have a larger delta_t due to a glitch eleswhere in
    // APM, such as a I2c timeout or a set of EEPROM writes. While
    // we would like to avoid these if possible, if it does happen
    // we don't want to compound the error by making DCM less
    // accurate.

    const float renorm_val = 1.0f / calc_length_pythagorean_3D(a.x, a.y, a.z);

    if (!(renorm_val < 2.0f && renorm_val > 0.5f)) {
        // this is larger than it should get - log it as a warning
        if (!(renorm_val < 1.0e6f && renorm_val > 1.0e-6f)) {
            // we are getting values which are way out of
            // range, we will reset the matrix and hope we
            // can recover our attitude using drift
            // correction before we hit the ground!
            //Serial.printf("ERROR: DCM renormalisation error. renorm_val=%f\n",
            //	   renorm_val);
            return false;
        }
    }

    result->x = a.x * renorm_val;
    result->y = a.y * renorm_val;
    result->z = a.z * renorm_val;

    return true;
}

/*************************************************
 *  Direction Cosine Matrix IMU: Theory
 *  William Premerlani and Paul Bizard
 *
 *  Numerical errors will gradually reduce the orthogonality conditions expressed by equation 5
 *  to approximations rather than identities. In effect, the axes in the two frames of reference no
 *  longer describe a rigid body. Fortunately, numerical error accumulates very slowly, so it is a
 *  simple matter to stay ahead of it.
 *  We call the process of enforcing the orthogonality conditions: renormalization.
 */
void normalize(void)
{
    const float error = rMat[0][0] * rMat[1][0] + rMat[0][1] * rMat[1][1] + rMat[0][2] * rMat[1][2]; // eq.18

    fpVector3_t t0;
    t0.x = rMat[0][0] - (rMat[1][0] * (0.5f * error)); // eq.19
    t0.y = rMat[0][1] - (rMat[1][1] * (0.5f * error)); // eq.19
    t0.z = rMat[0][2] - (rMat[1][2] * (0.5f * error)); // eq.19

    fpVector3_t t1;
    t1.x = rMat[1][0] - (rMat[0][0] * (0.5f * error)); // eq.19
    t1.y = rMat[1][1] - (rMat[0][1] * (0.5f * error)); // eq.19
    t1.z = rMat[1][2] - (rMat[0][2] * (0.5f * error)); // eq.19

    fpVector3_t t2; 
    vectorCrossProduct(&t2, &t0, &t1); // eq.20

    fpVector3_t matrixA = { .v = { rMat[0][0], rMat[0][1], rMat[0][2] } };
    fpVector3_t matrixB = { .v = { rMat[1][0], rMat[1][1], rMat[1][2] } };
    fpVector3_t matrixC = { .v = { rMat[2][0], rMat[2][1], rMat[2][2] } };

    const bool checkMatrixA = renorm(t0, &matrixA); 
    const bool checkMatrixB = renorm(t1, &matrixB); 
    const bool checkMatrixC = renorm(t2, &matrixC); 

    if (!checkMatrixA || 
        !checkMatrixB || 
        !checkMatrixC) {
        // Our solution is blowing up and we will force back
        // to last euler angles
        _last_failure_ms = millis();
        dcmReset(true);
    }
    
    if (checkMatrixA) {
        rMat[0][0] = matrixA.x;
        rMat[0][1] = matrixA.y;
        rMat[0][2] = matrixA.z;
    }

    if (checkMatrixB) {
        rMat[1][0] = matrixB.x;
        rMat[1][1] = matrixB.y;
        rMat[1][2] = matrixB.z;
    }

    if (checkMatrixC) {
        rMat[2][0] = matrixC.x;
        rMat[2][1] = matrixC.y;
        rMat[2][2] = matrixC.z;
    }
}

/*
 *  check the DCM matrix for pathological values
 */
void check_matrix(void)
{
    if (rotationMatrixIsNAN()) {
        //Serial.printf("ERROR: DCM matrix NAN\n");
        dcmReset(true);
        return;
    }

    // some DCM matrix values can lead to an out of range error in
    // the pitch calculation via asin().  These NaN values can
    // feed back into the rest of the DCM matrix via the
    // error_course value.
    if (!(rMat[2][0] < 1.0f && rMat[2][0] > -1.0f)) {
        // We have an invalid matrix. Force a normalisation.
        normalize();

        if (rotationMatrixIsNAN() || fabsf(rMat[2][0]) > 10.0f) {
            // See Issue #20284: regarding the selection of 10.0 for DCM reset
            // This won't be lowered without evidence of an issue or mathematical proof & testing of a lower bound

            // normalisation didn't fix the problem! We're
            // in real trouble. All we can do is reset
            //Serial.printf("ERROR: DCM matrix error. _dcm_matrix.c.x=%f\n",
            //	   _dcm_matrix.c.x);
            dcmReset(true);
        }
    }
}

// produce a yaw error value. The returned value is proportional
// to sin() of the current heading error in earth frame
float yaw_error_compass(void)
{/*
    const Vector3f &mag = compass.get_field();

    // get the mag vector in the earth frame
    Vector2f rb = _dcm_matrix.mulXY(mag);

    if (rb.length() < FLT_EPSILON) {
        return 0.0f;
    }

    rb.normalize();
    if (rb.is_inf()) {
        // not a valid vector
        return 0.0f;
    }

    // update vector holding earths magnetic field (if required)
    if( !is_equal(_last_declination, compass.get_declination()) ) {
        _last_declination = compass.get_declination();
        _mag_earth.x = cosf(_last_declination);
        _mag_earth.y = sinf(_last_declination);
    }

    // calculate the error term in earth frame
    // calculate the Z component of the cross product of rb and _mag_earth
    return rb % _mag_earth;*/
    return 0;
}

// the _P_gain raises the gain of the PI controller
// when we are spinning fast. See the fastRotations
// paper from Bill.
float _P_gain(float spin_rate)
{
    if (spin_rate < DEGREES_TO_RADIANS(50)) {
        return 1.0f;
    }
    if (spin_rate > DEGREES_TO_RADIANS(500)) {
        return 10.0f;
    }
    return spin_rate / DEGREES_TO_RADIANS(50);
}

// _yaw_gain reduces the gain of the PI controller applied to heading errors
// when observability from change of velocity is good (eg changing speed or turning)
// This reduces unwanted roll and pitch coupling due to compass errors for planes.
// High levels of noise on _accel_ef will cause the gain to drop and could lead to
// increased heading drift during straight and level flight, however some gain is
// always available. TODO check the necessity of adding adjustable acc threshold
// and/or filtering accelerations before getting magnitude
float _yaw_gain(void)
{
    const float VdotEFmag = calc_length_pythagorean_2D(_accel_ef.x, _accel_ef.y);

    if (VdotEFmag <= 4.0f) {
        return 0.2f * (4.5f - VdotEFmag);
    }

    return 0.1f;
}


// return true if we have and should use GPS
bool have_gps(void)
{
    return sensors(SENSOR_GPS) && STATE(GPS_FIX) && gpsSol.numSat >= 6;
}

bool get_fly_forward(void) {
    return gpsSol.groundSpeed >= 300;
}

/*
  when we are getting the initial attitude we want faster gains so
  that if the board starts upside down we quickly approach the right
  attitude.
  We don't want to keep those high gains for too long though as high P
  gains cause slow gyro offset learning. So we keep the high gains for
  a maximum of 20 seconds
 */
bool use_fast_gains(void)
{
    return !ARMING_FLAG(ARMED) && (millis() - _last_startup_ms) < 20000U;
}

int wrap_360(const int angle)
{
    int res = angle % 360;
    if (res < 0) {
        res += 360;
    }
    return res;
}

int16_t wrap_180(const int16_t angle)
{
    int16_t res = wrap_360(angle);
    if (res > 180) {
        res -= 360;
    }
    return res;
}

#define M_2PI (2.0f * M_PIf)

float wrap_2PI(const float radian)
{
    float res = fmod(radian, M_2PI);
    if (res < 0) {
        res += M_2PI;
    }
    return res;
}

float wrap_PI(const float radian)
{
    float res = wrap_2PI(radian);
    if (res > M_PIf) {
        res -= M_2PI;
    }
    return res;
}

// return true if we should use the compass for yaw correction
bool use_compass(void)
{
    if (!sensors(SENSOR_MAG)) {
        // no compass available
        return false;
    }

    if (!get_fly_forward() || !have_gps()) {
        // we don't have any alterative to the compass
        return true;
    }

    if (gpsSol.groundSpeed < GPS_SPEED_MIN) {
        // we are not going fast enough to use the GPS
        return true;
    }

    // if the current yaw differs from the GPS yaw by more than 45
    // degrees and the estimated wind speed is less than 80% of the
    // ground speed, then switch to GPS navigation. This will help
    // prevent flyaways with very bad compass offsets
    const float error = ABS(wrap_180(RADIANS_TO_DEGREES(_yaw) - gpsSol.groundCourse));
    if (error > 45 && calc_length_pythagorean_3D(_wind.x, _wind.y, _wind.z) < gpsSol.groundSpeed * 0.8f) {
        if (millis() - _last_consistent_heading > 2000) {
            // start using the GPS for heading if the compass has been
            // inconsistent with the GPS for 2 seconds
            return false;
        }
    } else {
        _last_consistent_heading = millis();
    }

    // use the compass
    return true;
}
/*
// return the quaternion defining the rotation from NED to XYZ (body) axes
bool get_quaternion(Quaternion &quat) const
{
    quat.from_rotation_matrix(_dcm_matrix);
    return true;
}
*/
/*
  calculate a compass heading given the attitude from DCM and the mag vector
 */
float calculate_heading(void)
{
    float cos_pitch_sq = 1.0f - (rMat[2][0] * rMat[2][0]);

    float headY =  rMat[2][2] * rMat[2][1];

    // Tilt compensated magnetic field X component:
    float headX = cos_pitch_sq - rMat[2][0] * (rMat[2][1] + rMat[2][2]);

    // magnetic heading
    // 6/4/11 - added constrain to keep bad values from ruining DCM Yaw - Jason S.
    float heading = constrainf(atan2_approx(-headY,headX), -M_PIf, M_PIf);
    
    float _declination = DEGREES_TO_RADIANS(DeclinationDeg);

    // Declination correction (if supplied)
    if ( fabsf(_declination) > 0.0f) {
        heading = heading + _declination;
        if (heading > M_PIf) {  // Angle normalization (-180 deg, 180 deg)
            heading -= (2.0f * M_PIf);
        } else if (heading < -M_PIf) {
            heading += (2.0f * M_PIf);
        }
    }

    return heading;
}

// yaw drift correction using the compass or GPS
// this function prodoces the _omega_yaw_P vector, and also
// contributes to the _omega_I.z long term yaw drift estimate
void drift_correction_yaw(void)
{
    bool new_value = false;
    float yaw_error;
    float yaw_deltat;

    if (!compassIsCalibrationComplete()) {
        // don't do any yaw correction while calibrating
        return;
    }
    
    if (use_compass()) {
        /*
          we are using compass for yaw
         */
        // we force an additional compass read()
        // here. This has the effect of throwing away
        // the first compass value, which can be bad
        if (!have_initial_yaw) {
            const float heading = calculate_heading();
            from_euler(_roll, _pitch, heading);
            _omega_yaw_P.x = 0.0f;
            _omega_yaw_P.y = 0.0f;
            _omega_yaw_P.z = 0.0f;
            have_initial_yaw = true;
        }
        new_value = true;
        yaw_error = yaw_error_compass();

        _gps_last_update = gpsStats.lastFixTime;
    } else if (get_fly_forward() && have_gps()) {
        /*
          we are using GPS for yaw
         */
        if (gpsStats.lastFixTime != _gps_last_update && gpsSol.groundSpeed >= GPS_SPEED_MIN) {
            yaw_deltat = (gpsStats.lastFixTime - _gps_last_update) * 1.0e-3f;
            _gps_last_update = gpsStats.lastFixTime;
            new_value = true;
            const float gps_course_rad = DEGREES_TO_RADIANS(gpsSol.groundCourse);
            const float yaw_error_rad = wrap_PI(gps_course_rad - _yaw);
            yaw_error = sin_approx(yaw_error_rad);

            /* reset yaw to match GPS heading under any of the
               following 3 conditions:

               1) if we have reached GPS_SPEED_MIN and have never had
               yaw information before

               2) if the last time we got yaw information from the GPS
               is more than 20 seconds ago, which means we may have
               suffered from considerable gyro drift

               3) if we are over 3*GPS_SPEED_MIN (which means 9m/s)
               and our yaw error is over 60 degrees, which means very
               poor yaw. This can happen on bungee launch when the
               operator pulls back the plane rapidly enough then on
               release the GPS heading changes very rapidly
            */
            if (!have_initial_yaw || (gpsSol.groundSpeed >= 3 * GPS_SPEED_MIN && fabsf(yaw_error_rad) >= 1.047f)) {
                // reset DCM matrix based on current yaw
                from_euler(_roll, _pitch, gps_course_rad);
                _omega_yaw_P.x = 0.0f;
                _omega_yaw_P.y = 0.0f;
                _omega_yaw_P.z = 0.0f;
                have_initial_yaw = true;
                yaw_error = 0;
            }
        }
    }

    if (!new_value) {
        // we don't have any new yaw information
        // slowly decay _omega_yaw_P to cope with loss
        // of our yaw source
        _omega_yaw_P.x *= 0.97f;
        _omega_yaw_P.y *= 0.97f;
        _omega_yaw_P.z *= 0.97f;
        return;
    }

    // convert the error vector to body frame
    const float error_z = rMat[2][2] * yaw_error;

    // the spin rate changes the P gain, and disables the
    // integration at higher rates
    const float spin_rate = calc_length_pythagorean_3D(_omega.x, _omega.y, _omega.z);

    // sanity check _kp_yaw
    if (_kp_yaw < AP_AHRS_YAW_P_MIN) {
        _kp_yaw = AP_AHRS_YAW_P_MIN;
    }

    // update the proportional control to drag the
    // yaw back to the right value. We use a gain
    // that depends on the spin rate. See the fastRotations.pdf
    // paper from Bill Premerlani
    // We also adjust the gain depending on the rate of change of horizontal velocity which
    // is proportional to how observable the heading is from the acceerations and GPS velocity
    // The accelration derived heading will be more reliable in turns than compass or GPS

    _omega_yaw_P.z = error_z * _P_gain(spin_rate) * _kp_yaw * _yaw_gain();

    if (use_fast_gains()) {
        _omega_yaw_P.z *= 8;
    }

    // don't update the drift term if we lost the yaw reference for more than 2 seconds
    if (yaw_deltat < 2.0f && spin_rate < DEGREES_TO_RADIANS(SPIN_RATE_LIMIT)) {
        // also add to the I term
        _omega_I_sum.z += error_z * _ki_yaw * yaw_deltat;
    }
}

/**
   return an accel vector delayed by AHRS_ACCEL_DELAY samples for a
   specific accelerometer instance
 */
fpVector3_t ra_delayed(fpVector3_t ra)
{
    // get the old element, and then fill it with the new element
    fpVector3_t ret;
    ret.x = _ra_delay_buffer.x;
    ret.y = _ra_delay_buffer.y;
    ret.z = _ra_delay_buffer.z;

    _ra_delay_buffer.x = ra.x;
    _ra_delay_buffer.y = ra.x;
    _ra_delay_buffer.z = ra.z;

    if (ret.x == 0 && ret.y == 0 && ret.z == 0) {
        // use the current vector if the previous vector is exactly
        // zero. This prevents an error on initialisation
        return ra;
    }

    return ret;
}

// perform drift correction. This function aims to update _omega_P and
// _omega_I with our best estimate of the short term and long term
// gyro error. The _omega_P value is what pulls our attitude solution
// back towards the reference vector quickly. The _omega_I term is an
// attempt to learn the long term drift rate of the gyros.
//
// This drift correction implementation is based on a paper
// by Bill Premerlani from here:
//   http://gentlenav.googlecode.com/files/RollPitchDriftCompensation.pdf
void drift_correction(float deltat)
{
    fpVector3_t velocity;
    uint32_t last_correction_time;

    // perform yaw drift correction if we have a new yaw reference
    // vector
    drift_correction_yaw();

    // rotate accelerometer values into the earth frame
    /*
        by using get_imuMeasuredAccelBF() instead of get_accel() the
        accel value is sampled over the right time delta for
        each sensor, which prevents an aliasing effect
    */
    if (deltat > 0.0f) {
        _accel_ef.x = rMat[0][0] * imuMeasuredAccelBF.x + rMat[0][1] * imuMeasuredAccelBF.y + rMat[0][2] * imuMeasuredAccelBF.z;
        _accel_ef.y = rMat[1][0] * imuMeasuredAccelBF.x + rMat[1][1] * imuMeasuredAccelBF.y + rMat[1][2] * imuMeasuredAccelBF.z;
        _accel_ef.z = rMat[2][0] * imuMeasuredAccelBF.x + rMat[2][1] * imuMeasuredAccelBF.y + rMat[2][2] * imuMeasuredAccelBF.z;
        // integrate the accel vector in the earth frame between GPS readings
        _ra_sum.x += _accel_ef.x * deltat;
        _ra_sum.y += _accel_ef.y * deltat;
        _ra_sum.z += _accel_ef.z * deltat;
    }

    // keep a sum of the deltat values, so we know how much time
    // we have integrated over
    _ra_deltat += deltat;

    const bool fly_forward = get_fly_forward();

    if (!have_gps()) {
        // no GPS, or not a good lock. From experience we need at
        // least 6 satellites to get a really reliable velocity number
        // from the GPS.
        //
        // As a fallback we use the fixed wing acceleration correction
        // if we have an airspeed estimate (which we only have if
        // _fly_forward is set), otherwise no correction
        if (_ra_deltat < 0.2f) {
            // not enough time has accumulated
            return;
        }

        float airspeed = _last_airspeed;
#if AP_AIRSPEED_ENABLED
        if (airspeed_sensor_enabled()) {
            airspeed = AP::airspeed()->get_airspeed();
        }
#endif

        // use airspeed to estimate our ground velocity in
        // earth frame by subtracting the wind
        //velocity = _dcm_matrix.colx() * airspeed;
        velocity.x = rMat[0][0] * airspeed;
        velocity.y = rMat[1][0] * airspeed;
        velocity.z = rMat[2][0] * airspeed;

        // add in wind estimate
        velocity.x += _wind.x;
        velocity.y += _wind.y;
        velocity.z += _wind.z;

        last_correction_time = millis();

        _have_gps_lock = false;
    } else {
        if (gpsStats.lastFixTime == _ra_sum_start) {
            // we don't have a new GPS fix - nothing more to do
            return;
        }

        velocity.x = gpsSol.velNED[X];
        velocity.y = gpsSol.velNED[Y];
        velocity.z = gpsSol.velNED[Z];

        last_correction_time = gpsStats.lastFixTime;

        if (_have_gps_lock == false) {
            // if we didn't have GPS lock in the last drift
            // correction interval then set the velocities equal
            _last_velocity.x = velocity.x;
            _last_velocity.y = velocity.y;
            _last_velocity.z = velocity.z;
        }

        _have_gps_lock = true;

        // keep last airspeed estimate for dead-reckoning purposes
        fpVector3_t airspeed;
        airspeed.x = velocity.x - _wind.x;
        airspeed.y = velocity.y - _wind.y;
        airspeed.z = velocity.z - _wind.z;

        // rotate vector to body frame
        mul_transpose(&airspeed);

        // take positive component in X direction. This mimics a pitot
        // tube
        _last_airspeed = MAX(airspeed.x, 0);
    }

    // see if this is our first time through - in which case we
    // just setup the start times and return
    if (_ra_sum_start == 0) {
        _ra_sum_start = last_correction_time;
        _last_velocity.x = velocity.x;
        _last_velocity.y = velocity.y;
        _last_velocity.z = velocity.z;
        return;
    }

    // equation 9: get the corrected acceleration vector in earth frame. Units are cm/s/s
    fpVector3_t GA_e;
    GA_e.x = 0.0f;
    GA_e.y = 0.0f;
    GA_e.z = 1.0f;

    if (_ra_deltat <= 0) {
        // waiting for more data
        return;
    }
    
    bool using_gps_corrections = false;
    float ra_scale = 1.0f / (_ra_deltat * GRAVITY_CMSS);

    if (ARMING_FLAG(ARMED) && (_have_gps_lock || fly_forward)) {
        const float v_scale = gps_gain * ra_scale;
        fpVector3_t vdelta;
        vdelta.x = (velocity.x - _last_velocity.x) * v_scale;
        vdelta.y = (velocity.y - _last_velocity.y) * v_scale;
        vdelta.z = (velocity.z - _last_velocity.z) * v_scale;
        GA_e.x += vdelta.x;
        GA_e.y += vdelta.y;
        GA_e.z += vdelta.z;
        GA_e.x /= calc_length_pythagorean_3D(GA_e.x, GA_e.y, GA_e.z);
        GA_e.y /= calc_length_pythagorean_3D(GA_e.x, GA_e.y, GA_e.z);
        GA_e.z /= calc_length_pythagorean_3D(GA_e.x, GA_e.y, GA_e.z);
        if (isinf(GA_e.x) || isinf(GA_e.y) || isinf(GA_e.z)) {
            // wait for some non-zero acceleration information
            _last_failure_ms = millis();
            return;
        }
        using_gps_corrections = true;
    }

    // calculate the error term in earth frame.
    // we do this for each available accelerometer then pick the
    // accelerometer that leads to the smallest error term. This takes
    // advantage of the different sample rates on different
    // accelerometers to dramatically reduce the impact of aliasing
    // due to harmonics of vibrations that match closely the sampling
    // rate of our accelerometers. On the Pixhawk we have the LSM303D
    // running at 800Hz and the MPU6000 running at 1kHz, by combining
    // the two the effects of aliasing are greatly reduced.
    fpVector3_t error;
    fpVector3_t GA_b;

    _ra_sum.x *= ra_scale;
    _ra_sum.y *= ra_scale;
    _ra_sum.z *= ra_scale;

    // get the delayed ra_sum to match the GPS lag
    if (using_gps_corrections) {
        GA_b.x = ra_delayed(_ra_sum).x;
        GA_b.y = ra_delayed(_ra_sum).y;
        GA_b.z = ra_delayed(_ra_sum).z;
    } else {
        GA_b.x = _ra_sum.x;
        GA_b.y = _ra_sum.y;
        GA_b.z = _ra_sum.z;
    
    }

    if (GA_b.x == 0 && GA_b.y == 0 && GA_b.z == 0) {
        // wait for some non-zero acceleration information
        return;
    }

    GA_b.x /= calc_length_pythagorean_3D(GA_b.x, GA_b.y, GA_b.z);
    GA_b.y /= calc_length_pythagorean_3D(GA_b.x, GA_b.y, GA_b.z);
    GA_b.z /= calc_length_pythagorean_3D(GA_b.x, GA_b.y, GA_b.z);

    if (isinf(GA_b.x) || isinf(GA_b.y) || isinf(GA_b.z)) {
        // wait for some non-zero acceleration information
        return;
    }

    vectorCrossProduct(&error, &GA_b, &GA_e);

    // to reduce the impact of two competing yaw controllers, we
    // reduce the impact of the gps/accelerometers on yaw when we are
    // flat, but still allow for yaw correction using the
    // accelerometers at high roll angles as long as we have a GPS
    if (use_compass()) {
        if (have_gps() && gps_gain == 1.0f) {
            error.z *= sin_approx(fabsf(_roll));
        } else {
            error.z = 0;
        }
    }

    // if ins is unhealthy then stop attitude drift correction and
    // hope the gyros are OK for a while. Just slowly reduce _omega_P
    // to prevent previous bad accels from throwing us off
    if (!sensors(SENSOR_ACC) && !sensors(SENSOR_GYRO)) {
        error.x = 0.0f;
        error.y = 0.0f;
        error.z = 0.0f;
    } else {
        // convert the error term to body frame
        mul_transpose(&error);
    }

    if (isnan(error.x) || isnan(error.y) || isnan(error.z) ||
        isinf(error.x) || isinf(error.y) || isinf(error.z)) {
        // don't allow bad values
        check_matrix();
        _last_failure_ms = millis();
        return;
    }
    
    // base the P gain on the spin rate
    const float spin_rate = calc_length_pythagorean_3D(_omega.x, _omega.y, _omega.z);

    // sanity check _kp value
    if (_kp < AP_AHRS_RP_P_MIN) {
        _kp = AP_AHRS_RP_P_MIN;
    }

    // we now want to calculate _omega_P and _omega_I. The
    // _omega_P value is what drags us quickly to the
    // accelerometer reading.
    _omega_P.x = error.x * _P_gain(spin_rate) * _kp;
    _omega_P.y = error.y * _P_gain(spin_rate) * _kp;
    _omega_P.z = error.z * _P_gain(spin_rate) * _kp;

    if (use_fast_gains()) {
        _omega_P.x *= 8;
        _omega_P.y *= 8;
        _omega_P.z *= 8;
    }

    fpVector3_t get_accel;
    accGetMeasuredAcceleration(&get_accel);

    if (fly_forward && have_gps() && gpsSol.groundSpeed < GPS_SPEED_MIN && get_accel.x >= 7 && _pitch > DEGREES_TO_RADIANS(-30) && _pitch < DEGREES_TO_RADIANS(30)) {
        // assume we are in a launch acceleration, and reduce the
        // rp gain by 50% to reduce the impact of GPS lag on
        // takeoff attitude when using a catapult
        _omega_P.x *= 0.5f;
        _omega_P.y *= 0.5f;
        _omega_P.z *= 0.5f;
    }

    // accumulate some integrator error
    if (spin_rate < DEGREES_TO_RADIANS(SPIN_RATE_LIMIT)) {
        _omega_I_sum.x += error.x * _ki * _ra_deltat;
        _omega_I_sum.y += error.y * _ki * _ra_deltat;
        _omega_I_sum.z += error.z * _ki * _ra_deltat;
        _omega_I_sum_time += _ra_deltat;
    }

    if (_omega_I_sum_time >= 5) {
        // limit the rate of change of omega_I to the hardware
        // reported maximum gyro drift rate. This ensures that
        // short term errors don't cause a buildup of omega_I
        // beyond the physical limits of the device
        const float change_limit = DEGREES_TO_RADIANS(0.5f / 60.0f) * _omega_I_sum_time;
        _omega_I_sum.x = constrainf(_omega_I_sum.x, -change_limit, change_limit);
        _omega_I_sum.y = constrainf(_omega_I_sum.y, -change_limit, change_limit);
        _omega_I_sum.z = constrainf(_omega_I_sum.z, -change_limit, change_limit);
        _omega_I.x += _omega_I_sum.x;
        _omega_I.y += _omega_I_sum.y;
        _omega_I.z += _omega_I_sum.z;
        _omega_I_sum.x = 0;
        _omega_I_sum.y = 0;
        _omega_I_sum.z = 0;
        _omega_I_sum_time = 0;
    }

    // zero our accumulator ready for the next GPS step
    memset((void *)&_ra_sum, 0, sizeof(_ra_sum));
    _ra_deltat = 0;
    _ra_sum_start = last_correction_time;

    // remember the velocity for next time
    _last_velocity = velocity;
}

/*
  calculate sin and cos of roll/pitch/yaw from a body_to_ned rotation matrix
 */
void calc_trig(float *cr, float *cp, float *cy, float *sr, float *sp, float *sy) {

    fpVector3_t yaw_vector;
    yaw_vector.x = rMat[0][0];
    yaw_vector.y = rMat[1][0];
    yaw_vector.z = 0.0f;

    if (fabsf(yaw_vector.x) > 0.0f || fabsf(yaw_vector.y) > 0.0f) {
        yaw_vector.x /= calc_length_pythagorean_2D(yaw_vector.x, yaw_vector.y);
        yaw_vector.y /= calc_length_pythagorean_2D(yaw_vector.x, yaw_vector.y);
    }

    *sy = constrainf(yaw_vector.y, -1.0f, 1.0f);
    *cy = constrainf(yaw_vector.x, -1.0f, 1.0f);

    // sanity checks
    if (isinf(yaw_vector.x) || isinf(yaw_vector.y) || isinf(yaw_vector.z) || 
        isnan(yaw_vector.x) || isnan(yaw_vector.y) || isnan(yaw_vector.z)) {
        *sy = 0.0f;
        *cy = 1.0f;
    }

    const float cx2 = rMat[2][0] * rMat[2][0];

    if (cx2 >= 1.0f) {
        *cp = 0.0f;
        *cr = 1.0f;
    } else {
        *cp = fast_fsqrtf(1.0f - cx2);
        *cr = rMat[2][2] / *cp;
    }

    *cp = constrainf(*cp, 0.0f, 1.0f);
    *cr = constrainf(*cr, -1.0f, 1.0f); // this relies on constrain_float() of infinity doing the right thing

    *sp = -rMat[2][0];

    if (*cp > 0.0f) {
        *sr = rMat[2][1] / *cp;
    }

    if (*cp == 0 || isinf(*cr) || isnan(*cr) || isinf(*sr) || isnan(*sr)) {
        float r;
        r = atan2_approx(rMat[2][1], rMat[2][2]);
        *cr = cos_approx(r);
        *sr = sin_approx(r);
    }
}

// run a full DCM update round
void update(float delta_t)
{
    // if the update call took more than 0.2 seconds then discard it,
    // otherwise we may move too far.
    if (delta_t > 0.2f) {
        memset((void *)&_ra_sum, 0, sizeof(_ra_sum));
        _ra_deltat = 0;
        return;
    }
    
    static bool reseted = false;
    if (!gyroIsCalibrationComplete()) {
        if(!reseted){
            dcmReset(false);
            reseted = true;
        }
        reset_gyro_drift();
    }

    // Integrate the DCM matrix using gyro inputs
    matrix_update(delta_t);

    // Normalize the DCM matrix
    normalize();

    // Perform drift correction
    drift_correction(delta_t);

    // paranoid check for bad values in the DCM matrix
    check_matrix();

    // calculate the euler angles and DCM matrix which will be used
    // for high level navigation control. Apply trim such that a
    // positive trim value results in a positive vehicle rotation
    // about that axis (ie a negative offset)
    _roll = atan2_approx(rMat[2][1], rMat[2][2]);
    _pitch = -asin_approx(rMat[2][0]);
    _yaw = -atan2_approx(rMat[1][0], rMat[0][0]);

    // pre-calculate some trig for CPU purposes:
    _cos_yaw = cos_approx(_yaw);
    _sin_yaw = sin_approx(_yaw);
    
    attitude.values.roll = RADIANS_TO_DECIDEGREES(_roll);
    attitude.values.pitch = RADIANS_TO_DECIDEGREES(_pitch);
    attitude.values.yaw = RADIANS_TO_DECIDEGREES(_yaw);

    if (_yaw < 0) {
        attitude.values.yaw += 3600;
    }

    calc_trig(&_cos_roll, &_cos_pitch, &_cos_yaw, &_sin_roll, &_sin_pitch, &_sin_yaw);
    
    DEBUG_SET(DEBUG_CRUISE, 0, RADIANS_TO_DEGREES(calculateCosTiltAngle()));

    // Update small angle state 
    if (RADIANS_TO_DEGREES(calculateCosTiltAngle()) < imuRuntimeConfig.small_angle) {
        ENABLE_STATE(SMALL_ANGLE);
    } else {
        DISABLE_STATE(SMALL_ANGLE);
    }
}

// update our wind speed estimate
void updateWindEstimator(void)
{
    fpVector3_t velocity;
    velocity.x = _last_velocity.x;
    velocity.y = _last_velocity.y;
    velocity.z = _last_velocity.z;

    // this is based on the wind speed estimation code from MatrixPilot by
    // Bill Premerlani. Adaption for ArduPilot by Jon Challinger
    // See http://gentlenav.googlecode.com/files/WindEstimation.pdf
    fpVector3_t fuselageDirection;
    fuselageDirection.x = rMat[0][0];
    fuselageDirection.y = rMat[1][0];
    fuselageDirection.z = rMat[2][0];

    fpVector3_t fuselageDirectionDiff;
    fuselageDirectionDiff.x = fuselageDirection.x - _last_fuse.x;
    fuselageDirectionDiff.y = fuselageDirection.y - _last_fuse.y;
    fuselageDirectionDiff.z = fuselageDirection.z - _last_fuse.z;

    const uint32_t now = millis();

    // scrap our data and start over if we're taking too long to get a direction change
    if (now - _last_wind_time > 10000) {
        _last_wind_time = now;
        _last_fuse.x = fuselageDirection.x;
        _last_fuse.y = fuselageDirection.y;
        _last_fuse.z = fuselageDirection.z;
        _last_vel.x = velocity.x;
        _last_vel.y = velocity.y;
        _last_vel.z = velocity.z;
        return;
    }

    float diff_length = calc_length_pythagorean_3D(fuselageDirectionDiff.x, fuselageDirectionDiff.y, fuselageDirectionDiff.z);
    if (diff_length > 0.2f) {
        // when turning, use the attitude response to estimate
        // wind speed
        float V;
        fpVector3_t velocityDiff;
        velocityDiff.x = velocity.x - _last_vel.x;
        velocityDiff.y = velocity.y - _last_vel.y;
        velocityDiff.z = velocity.z - _last_vel.z;

        // estimate airspeed it using equation 6
        V = calc_length_pythagorean_3D(velocityDiff.x, velocityDiff.y, velocityDiff.z) / diff_length;

        fpVector3_t fuselageDirectionSum;
        fuselageDirectionSum.x = fuselageDirection.x + _last_fuse.x;
        fuselageDirectionSum.y = fuselageDirection.y + _last_fuse.y;
        fuselageDirectionSum.z = fuselageDirection.z + _last_fuse.z;

        fpVector3_t velocitySum;
        velocitySum.x = velocity.x + _last_vel.x;
        velocitySum.y = velocity.y + _last_vel.y;
        velocitySum.z = velocity.z + _last_vel.z;

        _last_fuse.x = fuselageDirection.x;
        _last_fuse.y = fuselageDirection.y;
        _last_fuse.z = fuselageDirection.z;
        _last_vel.x = velocity.x;
        _last_vel.y = velocity.y;
        _last_vel.z = velocity.z;

        const float theta = atan2_approx(velocityDiff.y, velocityDiff.x) - atan2_approx(fuselageDirectionDiff.y, fuselageDirectionDiff.x);
        const float sintheta = sin_approx(theta);
        const float costheta = cos_approx(theta);

        fpVector3_t wind;
        wind.x = velocitySum.x - V * (costheta * fuselageDirectionSum.x - sintheta * fuselageDirectionSum.y);
        wind.y = velocitySum.y - V * (sintheta * fuselageDirectionSum.x + costheta * fuselageDirectionSum.y);
        wind.z = velocitySum.z - V * fuselageDirectionSum.z;
        wind.x *= 0.5f;
        wind.y *= 0.5f;
        wind.z *= 0.5f;

        if (calc_length_pythagorean_3D(wind.x, wind.y, wind.z) < calc_length_pythagorean_3D(_wind.x, _wind.y, _wind.z) + 20) {
            _wind.x = _wind.x * 0.95f + wind.x * 0.05f;
            _wind.y = _wind.y * 0.95f + wind.y * 0.05f;
            _wind.z = _wind.z * 0.95f + wind.z * 0.05f;
        }

        _last_wind_time = now;

        return;
    }

#if AP_AIRSPEED_ENABLED
    if (now - _last_wind_time > 2000 && airspeed_sensor_enabled()) {
        // when flying straight use airspeed to get wind estimate if available
        const Vector3f airspeed = _dcm_matrix.colx() * AP::airspeed()->get_airspeed();
        const Vector3f wind = velocity - (airspeed * get_EAS2TAS());
        _wind = _wind * 0.92f + wind * 0.08f;
    }
#endif
}

float airspeed_estimate(void)
{
    float airspeed_ret;

#if AP_AIRSPEED_ENABLED
    if (airspeed_sensor_enabled(airspeed_index)) {
        airspeed_ret = AP::airspeed()->get_airspeed(airspeed_index);
        return airspeed_ret;
    }
#endif

    // Else give the last estimate, but return false.
    // This is used by the dead-reckoning code
    airspeed_ret = _last_airspeed;

    return airspeed_ret;
}

/*
  check if the AHRS subsystem is healthy
*/
bool ahrsIsHealthy(void)
{
    // consider ourselves healthy if there have been no failures for 5 seconds
    return (_last_failure_ms == 0 || millis() - _last_failure_ms > 5000);
}

float calculateCosTiltAngle(void)
{
    return acos_approx(_cos_roll * _cos_pitch);
}