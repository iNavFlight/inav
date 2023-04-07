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
#include "common/log.h"
#include "common/maths.h"
#include "common/vector.h"
#include "common/quaternion.h"
#include "common/time.h"


#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/time.h"

#include "fc/config.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/pid.h"
#if defined(USE_WIND_ESTIMATOR)
#include "flight/wind_estimator.h"
#endif

#include "io/gps.h"

#include "sensors/acceleration.h"
#include "sensors/barometer.h"
#include "sensors/pitotmeter.h"
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
#define MAX_ACC_NEARNESS            0.2    // 20% or G error soft-accepted (0.8-1.2G)
#define IMU_ROTATION_LPF         3       // Hz
FASTRAM fpVector3_t imuMeasuredAccelBF;
FASTRAM fpVector3_t imuMeasuredRotationBF;
//centrifugal force compensated using gps
FASTRAM fpVector3_t compansatedGravityBF;// cm/s/s

STATIC_FASTRAM float smallAngleCosZ;

STATIC_FASTRAM bool isAccelUpdatedAtLeastOnce;
STATIC_FASTRAM fpVector3_t vCorrectedMagNorth;             // Magnetic North vector in EF (true North rotated by declination)

FASTRAM fpQuaternion_t orientation;
FASTRAM attitudeEulerAngles_t attitude;             // absolute angle inclination in multiple of 0.1 degree    180 deg = 1800
FASTRAM float rMat[3][3];

STATIC_FASTRAM imuRuntimeConfig_t imuRuntimeConfig;

STATIC_FASTRAM pt1Filter_t rotRateFilterX;
STATIC_FASTRAM pt1Filter_t rotRateFilterY;
STATIC_FASTRAM pt1Filter_t rotRateFilterZ;
FASTRAM fpVector3_t imuMeasuredRotationBFFiltered = {.v = {0.0f, 0.0f, 0.0f}};

STATIC_FASTRAM pt1Filter_t HeadVecEFFilterX;
STATIC_FASTRAM pt1Filter_t HeadVecEFFilterY;
STATIC_FASTRAM pt1Filter_t HeadVecEFFilterZ;
FASTRAM fpVector3_t HeadVecEFFiltered = {.v = {0.0f, 0.0f, 0.0f}};

STATIC_FASTRAM float GPS3DspeedFiltered=0.0f;
STATIC_FASTRAM pt1Filter_t GPS3DspeedFilter;

FASTRAM bool gpsHeadingInitialized;

FASTRAM bool imuUpdated = false;

PG_REGISTER_WITH_RESET_TEMPLATE(imuConfig_t, imuConfig, PG_IMU_CONFIG, 2);

PG_RESET_TEMPLATE(imuConfig_t, imuConfig,
    .dcm_kp_acc = SETTING_AHRS_DCM_KP_DEFAULT,                   // 0.20 * 10000
    .dcm_ki_acc = SETTING_AHRS_DCM_KI_DEFAULT,                   // 0.005 * 10000
    .dcm_kp_mag = SETTING_AHRS_DCM_KP_MAG_DEFAULT,               // 0.20 * 10000
    .dcm_ki_mag = SETTING_AHRS_DCM_KI_MAG_DEFAULT,               // 0.005 * 10000
    .small_angle = SETTING_SMALL_ANGLE_DEFAULT,
    .acc_ignore_rate = SETTING_AHRS_ACC_IGNORE_RATE_DEFAULT,
    .acc_ignore_slope = SETTING_AHRS_ACC_IGNORE_SLOPE_DEFAULT,
    .gps_yaw_windcomp = SETTING_AHRS_GPS_YAW_WINDCOMP_DEFAULT,
    .inertia_comp_method = SETTING_AHRS_INERTIA_COMP_METHOD_DEFAULT
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
    pt1FilterReset(&rotRateFilterX, 0);
    pt1FilterReset(&rotRateFilterY, 0);
    pt1FilterReset(&rotRateFilterZ, 0);
    // Initialize Heading vector filter
    pt1FilterReset(&HeadVecEFFilterX, 0);
    pt1FilterReset(&HeadVecEFFilterY, 0);
    pt1FilterReset(&HeadVecEFFilterZ, 0);
    // Initialize 3d speed filter
    pt1FilterReset(&GPS3DspeedFilter, 0);
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

#if defined(USE_GPS)
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
        LOG_ERROR(IMU, "AHRS orientation quaternion error. Reset to last known good value");
    }
    else {
        // No valid reference. Best guess from accelerometer
        imuResetOrientationQuaternion(accBF);
        imuErrorEvent.errorCode = 2;
        LOG_ERROR(IMU, "AHRS orientation quaternion error. Best guess from ACC");
    }

#ifdef USE_BLACKBOX
    if (feature(FEATURE_BLACKBOX)) {
        blackboxLogEvent(FLIGHT_LOG_EVENT_IMU_FAILURE, (flightLogEventData_t*)&imuErrorEvent);
    }
#endif
}

bool isGPSTrustworthy(void)
{
    return sensors(SENSOR_GPS) && STATE(GPS_FIX) && gpsSol.numSat >= 6;
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

#ifdef USE_SIMULATOR
            if (ARMING_FLAG(SIMULATOR_MODE_HITL) || ARMING_FLAG(SIMULATOR_MODE_SITL)) {
                imuSetMagneticDeclination(0);
            }
#endif

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

            // Compute heading vector in EF from scalar CoG,x axis of accelerometer is pointing backwards.
            fpVector3_t vCoG = { .v = { -cos_approx(courseOverGround), sin_approx(courseOverGround), 0.0f } };
#if defined(USE_WIND_ESTIMATOR)
            // remove wind elements in vCoG for better heading estimation
            if (isEstimatedWindSpeedValid() && imuConfig()->gps_yaw_windcomp)
            {
                vectorScale(&vCoG, &vCoG, gpsSol.groundSpeed);
                vCoG.x += getEstimatedWindSpeed(X);
                vCoG.y -= getEstimatedWindSpeed(Y);
                vectorNormalize(&vCoG, &vCoG);
            }
#endif

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
                vectorScale(&vTmp, &vErr, imuRuntimeConfig.dcm_ki_mag * magWScaler * dt);
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
                vectorScale(&vTmp, &vErr, imuRuntimeConfig.dcm_ki_acc * accWScaler * dt);
                vectorAdd(&vGyroDriftEstimate, &vGyroDriftEstimate, &vTmp);
            }
        }

        // Calculate kP gain and apply proportional feedback
        vectorScale(&vErr, &vErr, imuRuntimeConfig.dcm_kp_acc * accWScaler);
        vectorAdd(&vRotation, &vRotation, &vErr);
    }
    // Anti wind-up
    float i_limit = DEGREES_TO_RADIANS(2.0f) * (imuRuntimeConfig.dcm_kp_acc + imuRuntimeConfig.dcm_kp_mag) / 2.0f;
    vGyroDriftEstimate.x = constrainf(vGyroDriftEstimate.x, -i_limit, i_limit);
    vGyroDriftEstimate.y = constrainf(vGyroDriftEstimate.y, -i_limit, i_limit);
    vGyroDriftEstimate.z = constrainf(vGyroDriftEstimate.z, -i_limit, i_limit);

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
#ifdef USE_SIMULATOR
	if ((ARMING_FLAG(SIMULATOR_MODE_HITL) && !SIMULATOR_HAS_OPTION(HITL_USE_IMU)) || (ARMING_FLAG(SIMULATOR_MODE_SITL) && imuUpdated)) {
		imuComputeQuaternionFromRPY(attitude.values.roll, attitude.values.pitch, attitude.values.yaw);
		imuComputeRotationMatrix();
	}
	else
#endif
	{
		/* Compute pitch/roll angles */
		attitude.values.roll = RADIANS_TO_DECIDEGREES(atan2_approx(rMat[2][1], rMat[2][2]));
		attitude.values.pitch = RADIANS_TO_DECIDEGREES((0.5f * M_PIf) - acos_approx(-rMat[2][0]));
		attitude.values.yaw = RADIANS_TO_DECIDEGREES(-atan2_approx(rMat[1][0], rMat[0][0]));
	}

    if (attitude.values.yaw < 0)
        attitude.values.yaw += 3600;

    /* Update small angle state */
    if (calculateCosTiltAngle() > smallAngleCosZ) {
        ENABLE_STATE(SMALL_ANGLE);
    } else {
        DISABLE_STATE(SMALL_ANGLE);
    }
}

static float imuCalculateAccelerometerWeightNearness(void)
{
    fpVector3_t accBFNorm;
    vectorScale(&accBFNorm, &compansatedGravityBF, 1.0f / GRAVITY_CMSS);
    const float accMagnitudeSq = vectorNormSquared(&accBFNorm);
    const float accWeight_Nearness = bellCurve(fast_fsqrtf(accMagnitudeSq) - 1.0f, MAX_ACC_NEARNESS);
    return accWeight_Nearness;
}

static float imuCalculateAccelerometerWeightRateIgnore(const float acc_ignore_slope_multipiler)
{
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

    if (ARMING_FLAG(ARMED) && imuConfig()->acc_ignore_rate)
    {
        float rotRateMagnitude = fast_fsqrtf(vectorNormSquared(&imuMeasuredRotationBFFiltered));
        rotRateMagnitude = rotRateMagnitude / (acc_ignore_slope_multipiler + 0.001f);
        if (imuConfig()->acc_ignore_slope)
        {
            const float rateSlopeMin = DEGREES_TO_RADIANS((imuConfig()->acc_ignore_rate - imuConfig()->acc_ignore_slope));
            const float rateSlopeMax = DEGREES_TO_RADIANS((imuConfig()->acc_ignore_rate + imuConfig()->acc_ignore_slope));

            accWeight_RateIgnore = scaleRangef(constrainf(rotRateMagnitude, rateSlopeMin, rateSlopeMax), rateSlopeMin, rateSlopeMax, 1.0f, 0.0f);
        }
        else
        {
            if (rotRateMagnitude > DEGREES_TO_RADIANS(imuConfig()->acc_ignore_rate))
            {
                accWeight_RateIgnore = 0.0f;
            }
        }
    }

    return accWeight_RateIgnore;
}

static void imuCalculateFilters(float dT)
{
    //flitering
    imuMeasuredRotationBFFiltered.x = pt1FilterApply4(&rotRateFilterX, imuMeasuredRotationBF.x, IMU_ROTATION_LPF, dT);
    imuMeasuredRotationBFFiltered.y = pt1FilterApply4(&rotRateFilterY, imuMeasuredRotationBF.y, IMU_ROTATION_LPF, dT);
    imuMeasuredRotationBFFiltered.z = pt1FilterApply4(&rotRateFilterZ, imuMeasuredRotationBF.z, IMU_ROTATION_LPF, dT);
    HeadVecEFFiltered.x = pt1FilterApply4(&HeadVecEFFilterX, rMat[0][0], IMU_ROTATION_LPF, dT);
    HeadVecEFFiltered.y = pt1FilterApply4(&HeadVecEFFilterY, rMat[1][0], IMU_ROTATION_LPF, dT);
    HeadVecEFFiltered.z = pt1FilterApply4(&HeadVecEFFilterZ, rMat[2][0], IMU_ROTATION_LPF, dT);

    //anti aliasing
    float GPS3Dspeed = calc_length_pythagorean_3D(gpsSol.velNED[X],gpsSol.velNED[Y],gpsSol.velNED[Z]);
    GPS3DspeedFiltered = pt1FilterApply4(&GPS3DspeedFilter, GPS3Dspeed, IMU_ROTATION_LPF, dT);
}

static void imuCalculateGPSacceleration(fpVector3_t *vEstcentrifugalAccelBF, float *acc_ignore_slope_multipiler)
{
    static rtcTime_t lastGPSNewDataTime = 0;
    static bool lastGPSHeartbeat;
    static fpVector3_t lastGPSvel;

    const fpVector3_t currentGPSvel = {.v = {gpsSol.velNED[X], gpsSol.velNED[Y], gpsSol.velNED[Z]}}; // cm/s gps speed
    const rtcTime_t currenttime = millis();

    // on first gps data acquired, time_delta_ms will be large, vEstcentrifugalAccelBF will be minimal to disable the compensation
    rtcTime_t time_delta_ms = currenttime - lastGPSNewDataTime;
    if (lastGPSHeartbeat != gpsSol.flags.gpsHeartbeat && time_delta_ms > 0)
    {
        // on new gps frame, update accEF and estimate centrifugal accleration
        fpVector3_t vGPSacc = {.v = {0.0f, 0.0f, 0.0f}};
        vGPSacc.x = -(currentGPSvel.x - lastGPSvel.x) / (MS2S(time_delta_ms)); // the x axis of accerometer is pointing backward
        vGPSacc.y = (currentGPSvel.y - lastGPSvel.y) / (MS2S(time_delta_ms));
        vGPSacc.z = (currentGPSvel.z - lastGPSvel.z) / (MS2S(time_delta_ms));
        // Calculate estimated centrifugal accleration vector in body frame
        quaternionRotateVector(vEstcentrifugalAccelBF, &vGPSacc, &orientation); // EF -> BF
        lastGPSNewDataTime = currenttime;
        lastGPSvel = currentGPSvel;
    }
    lastGPSHeartbeat = gpsSol.flags.gpsHeartbeat;
    *acc_ignore_slope_multipiler = 4;
}

static void imuCalculateTurnRateacceleration(fpVector3_t *vEstcentrifugalAccelBF, float dT, float *acc_ignore_slope_multipiler)
{   
    //fixed wing only
    static float lastspeed = -1.0f;
    float currentspeed = 0;
    if (isGPSTrustworthy()){
        //first speed choice is gps
        currentspeed = GPS3DspeedFiltered;
        *acc_ignore_slope_multipiler = 4.0f;
    }
#ifdef USE_PITOT
    if (sensors(SENSOR_PITOT) && pitotIsHealthy() && currentspeed < 0)
    {
        // second choice is pitot
		currentspeed = getAirspeedEstimate();
        *acc_ignore_slope_multipiler = 2.0f;
    }
#endif
    if (currentspeed < 0)
    {
        //third choice is fixedWingReferenceAirspeed
        currentspeed = pidProfile()->fixedWingReferenceAirspeed;
        *acc_ignore_slope_multipiler = 1.0f;
    }
    float speed_change = lastspeed > 0 ? currentspeed - lastspeed : 0;
    vEstcentrifugalAccelBF->x = -speed_change/dT;
    vEstcentrifugalAccelBF->y = -currentspeed*imuMeasuredRotationBFFiltered.z;
    vEstcentrifugalAccelBF->z = currentspeed*imuMeasuredRotationBFFiltered.y;
    lastspeed = currentspeed;
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

    imuCalculateFilters(dT);
    // centrifugal force compensation
    static fpVector3_t vEstcentrifugalAccelBF_velned;
    static fpVector3_t vEstcentrifugalAccelBF_turnrate;
    float acc_ignore_slope_multipiler = 1.0f; // when using gps centrifugal_force_compensation, AccelerometerWeightRateIgnore slope will be multiplied by this value
    if (isGPSTrustworthy())
    {
        imuCalculateGPSacceleration(&vEstcentrifugalAccelBF_velned, &acc_ignore_slope_multipiler);
    }
    if (STATE(AIRPLANE))
    {
        imuCalculateTurnRateacceleration(&vEstcentrifugalAccelBF_turnrate, dT, &acc_ignore_slope_multipiler);
    }
    if (imuConfig()->inertia_comp_method == COMPMETHOD_ADAPTIVE && isGPSTrustworthy() && STATE(AIRPLANE))
    {
        fpVector3_t compansatedGravityBF_velned;
        vectorAdd(&compansatedGravityBF_velned, &imuMeasuredAccelBF, &vEstcentrifugalAccelBF_velned);
        float velned_magnitude = fabsf(fast_fsqrtf(vectorNormSquared(&compansatedGravityBF_velned)) - GRAVITY_CMSS);

        fpVector3_t compansatedGravityBF_turnrate;
        vectorAdd(&compansatedGravityBF_turnrate, &imuMeasuredAccelBF, &vEstcentrifugalAccelBF_turnrate);
        float turnrate_magnitude = fabsf(fast_fsqrtf(vectorNormSquared(&compansatedGravityBF_turnrate)) - GRAVITY_CMSS);
        if (velned_magnitude > turnrate_magnitude)
        {
            compansatedGravityBF = compansatedGravityBF_turnrate;
        }
        else
        {
            compansatedGravityBF = compansatedGravityBF_velned;
        }
    }
    else if (imuConfig()->inertia_comp_method == COMPMETHOD_VELNED && isGPSTrustworthy())
    {
        vectorAdd(&compansatedGravityBF, &imuMeasuredAccelBF, &vEstcentrifugalAccelBF_velned);
    }
    else if (STATE(AIRPLANE))
    {
        vectorAdd(&compansatedGravityBF, &imuMeasuredAccelBF, &vEstcentrifugalAccelBF_turnrate);
    }
    else
    {
        compansatedGravityBF = imuMeasuredAccelBF;
    }
#else
    // In absence of GPS MAG is the only option
    if (canUseMAG) {
        useMag = true;
    }
    compansatedGravityBF = imuMeasuredAccelBF
#endif
    float accWeight = imuGetPGainScaleFactor() * imuCalculateAccelerometerWeightNearness();
    accWeight = accWeight * imuCalculateAccelerometerWeightRateIgnore(acc_ignore_slope_multipiler);
    const bool useAcc = (accWeight > 0.001f);

    const float magWeight = imuGetPGainScaleFactor() * 1.0f;
    fpVector3_t measuredMagBF = {.v = {mag.magADC[X], mag.magADC[Y], mag.magADC[Z]}};
    imuMahonyAHRSupdate(dT, &imuMeasuredRotationBF,
                            useAcc ? &compansatedGravityBF : NULL,
                            useMag ? &measuredMagBF : NULL,
                            useCOG, courseOverGround,
                            accWeight,
                            magWeight);
    imuUpdateEulerAngles();
}

void imuUpdateAccelerometer(void)
{
    if (sensors(SENSOR_ACC)) {
        accUpdate();
        isAccelUpdatedAtLeastOnce = true;
    }
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
        imuCalculateEstimatedAttitude(dT);  // Update attitude estimate
    } else {
        acc.accADCf[X] = 0.0f;
        acc.accADCf[Y] = 0.0f;
        acc.accADCf[Z] = 0.0f;
    }
}

bool isImuReady(void)
{
    return sensors(SENSOR_ACC) && STATE(ACCELEROMETER_CALIBRATED) && gyroIsCalibrationComplete();
}

bool isImuHeadingValid(void)
{
    return (sensors(SENSOR_MAG) && STATE(COMPASS_CALIBRATED)) || (STATE(FIXED_WING_LEGACY) && gpsHeadingInitialized);
}

float calculateCosTiltAngle(void)
{
    return 1.0f - 2.0f * sq(orientation.q1) - 2.0f * sq(orientation.q2);
}

#if defined(SITL_BUILD) || defined (USE_SIMULATOR)

void imuSetAttitudeRPY(int16_t roll, int16_t pitch, int16_t yaw)
{
    attitude.values.roll = roll;
    attitude.values.pitch = pitch;
    attitude.values.yaw = yaw;
    imuUpdated = true;
}
#endif

