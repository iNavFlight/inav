/*
 * AHRS - Attitude Heading Reference System 
 *
 * AHRS system using Direct Cosine Matrix (DCM)
 * 
 * Adapted for the general iNav AHRS interface by Julio Cesar Matias
 * 
 * This file is part of iNav.
 *
 * iNav is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * iNav is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with iNav.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#include "platform.h"

FILE_COMPILE_FOR_SPEED

#include "blackbox/blackbox.h"

#include "build/build_config.h"
#include "build/debug.h"

#include "common/axis.h"
#include "common/filter.h"
#include "common/log.h"
#include "common/maths.h"
#include "common/quaternion.h"

#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/time.h"

#include "fc/config.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "flight/ahrs.h"
#include "flight/hil.h"
#include "flight/mixer.h"
#include "flight/pid.h"

#include "io/gps.h"

#include "sensors/acceleration.h"
#include "sensors/barometer.h"
#include "sensors/compass.h"
#include "sensors/gyro.h"
#include "sensors/pitotmeter.h"
#include "sensors/sensors.h"

// Sanity Check (Please do not change these values!)
#define RP_KP_MIN 0.05f
#define YAW_KP_MIN 0.05f

// These are experimentally derived from the simulator with large drift levels (Please do not change these values!)
#define DCM_KI_ACC 0.0087f
#define DCM_KI_MAG 0.01f

// This is the speed in cm/s above which we first get a yaw lock with the GPS
#define GPS_SPEED_MIN 300

// The limit (in degrees/second) beyond which we stop integrating omega_I. At larger spin rates the DCM PI controller can get 'dizzy' which results in false gyro drift.
#define SPIN_RATE_LIMIT 20

FASTRAM fpMat3_t rotationMatrix;

FASTRAM attitudeEulerAngles_t attitude;

FASTRAM fpVector3_t imuMeasuredRotationBF;
FASTRAM fpVector3_t imuMeasuredAccelBF;
STATIC_FASTRAM fpVector3_t accel_ef;
STATIC_FASTRAM fpVector3_t omega;
STATIC_FASTRAM fpVector3_t omega_sum;
STATIC_FASTRAM fpVector3_t last_omega;
STATIC_FASTRAM fpVector3_t delta_angle;
STATIC_FASTRAM fpVector3_t last_delta_angle;
STATIC_FASTRAM fpVector3_t omega_P;
STATIC_FASTRAM fpVector3_t omega_yaw_P;
STATIC_FASTRAM fpVector3_t omega_I;
STATIC_FASTRAM fpVector3_t omega_I_sum;
STATIC_FASTRAM fpVector3_t mag_earth;
STATIC_FASTRAM fpVector3_t ra_sum;
STATIC_FASTRAM fpVector3_t ra_delay_buffer;
STATIC_FASTRAM fpVector3_t last_velocity;
STATIC_FASTRAM fpVector3_t _wind;
STATIC_FASTRAM fpVector3_t last_fuse;
STATIC_FASTRAM fpVector3_t last_vel;

STATIC_FASTRAM flightLogEvent_IMUError_t ahrsErrorEvent;
STATIC_FASTRAM flightLogEvent_IMUError_t prev_ahrsErrorEvent;

static timeMs_t last_consistent_heading;
static timeMs_t last_startup_ms;
static timeMs_t ra_sum_start;
static timeMs_t last_failure_ms;
static timeMs_t gps_last_update;
static timeMs_t last_wind_time;
static timeUs_t compass_last_update;

STATIC_FASTRAM bool have_gps_lock;
STATIC_FASTRAM bool have_initial_yaw;
STATIC_FASTRAM bool fly_forward; // True for planes, rover and boat / false for copter

STATIC_FASTRAM float omega_I_sum_time;
STATIC_FASTRAM float ra_deltaTime;
STATIC_FASTRAM float last_airspeed;

// Euler angles
STATIC_FASTRAM float _roll;
STATIC_FASTRAM float _pitch;
STATIC_FASTRAM float _yaw;

// Helper trigonometry variables
STATIC_FASTRAM float cos_roll = 1.0f;
STATIC_FASTRAM float cos_pitch = 1.0f;
STATIC_FASTRAM float cos_yaw = 1.0f;
STATIC_FASTRAM float sin_roll;
STATIC_FASTRAM float sin_pitch;
STATIC_FASTRAM float sin_yaw;

PG_REGISTER_WITH_RESET_TEMPLATE(ahrsConfig_t, ahrsConfig, PG_AHRS_CONFIG, 3);

PG_RESET_TEMPLATE(ahrsConfig_t, ahrsConfig,
    .dcm_kp_acc = SETTING_AHRS_KP_ACC_DEFAULT,
    .dcm_kp_mag = SETTING_AHRS_KP_MAG_DEFAULT,
    .dcm_gps_gain = SETTING_AHRS_GPS_GAIN_DEFAULT,
    .small_angle = SETTING_SMALL_ANGLE_DEFAULT
);

void ahrsInit(void)
{
    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        imuMeasuredAccelBF.v[axis] = 0.0f;
        imuMeasuredRotationBF.v[axis] = 0.0f;
    }

    // Create magnetic declination matrix
#ifdef USE_MAG
    const int16_t deg = compassConfig()->mag_declination / 100;
    const int16_t min = compassConfig()->mag_declination % 100;
#else
    const int16_t deg = 0;
    const int16_t min = 0;
#endif

    ahrsSetMagneticDeclination(deg + min / 60.0f);
    
    matrixIdentity(&rotationMatrix);

    ahrsReset(false);

    if (STATE(FIXED_WING_LEGACY)) {
        fly_forward = true;
    }
}

void ahrsSetMagneticDeclination(float declinationDeg)
{
    const float declinationRad = -DEGREES_TO_RADIANS(declinationDeg);
    
    mag_earth.x = cos_approx(declinationRad);
    mag_earth.y = sin_approx(declinationRad);
    mag_earth.z = 0.0f;
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

// Reset the current gyro drift estimate should be called if gyro offsets are recalculated
static void resetGyroDrift(void)
{
    omega.x = 0.0f;
    omega.y = 0.0f;
    omega.z = 0.0f;
    delta_angle.x = 0.0f;
    delta_angle.y = 0.0f;
    delta_angle.z = 0.0f;
    last_omega.x = 0.0f;
    last_omega.y = 0.0f;
    last_omega.z = 0.0f;
    last_delta_angle.x = 0.0f;
    last_delta_angle.y = 0.0f;
    last_delta_angle.z = 0.0f;
    omega_I_sum.x = 0.0f;
    omega_I_sum.y = 0.0f;
    omega_I_sum.z = 0.0f;
    omega_I_sum_time = 0.0f;
}

// Update the DCM matrix using only the gyros
static void matrixUpdate(float deltaTime)
{
    // Note that we do not include the P terms in omega. This is  because the spin_rate is calculated from calc_length_pythagorean_3D(omega.x, omega.y, omega.z), 
    // and including the P terms would give positive feedback into the proportionalGain() calculation, which can lead to a very large P value.
    
    delta_angle.x = (imuMeasuredRotationBF.x + last_omega.x) * 0.5f;
    delta_angle.y = (imuMeasuredRotationBF.y + last_omega.y) * 0.5f;
    delta_angle.z = (imuMeasuredRotationBF.z + last_omega.z) * 0.5f;

    // Compute coning correction
    fpVector3_t delta_coning = { .v = { delta_angle.x + last_delta_angle.x * (1.0f / 6.0f), 
                                        delta_angle.y + last_delta_angle.y * (1.0f / 6.0f), 
                                        delta_angle.z + last_delta_angle.z * (1.0f / 6.0f) } };

    vectorCrossProduct(&delta_coning, &delta_coning, &delta_angle);

    delta_coning.x *= 0.5f;
    delta_coning.y *= 0.5f;
    delta_coning.z *= 0.5f;

    if (deltaTime > 0.0f) {
        // Integrate delta angle accumulator. The angles and coning corrections are accumulated separately.
        omega.x = (delta_angle.x + delta_coning.x);
        omega.y = (delta_angle.y + delta_coning.y);
        omega.z = (delta_angle.z + delta_coning.z);
        omega.x += omega_I.x;
        omega.y += omega_I.y;
        omega.z += omega_I.z;
        omega_sum.x = (omega.x + omega_P.x + omega_yaw_P.x) * deltaTime;
        omega_sum.y = (omega.y + omega_P.y + omega_yaw_P.y) * deltaTime;
        omega_sum.z = (omega.z + omega_P.z + omega_yaw_P.z) * deltaTime;
        matrixRotate(omega_sum, &rotationMatrix);
    }

    last_omega.x = imuMeasuredRotationBF.x;
    last_omega.y = imuMeasuredRotationBF.y;
    last_omega.z = imuMeasuredRotationBF.z;

    last_delta_angle.x = delta_angle.x;
    last_delta_angle.y = delta_angle.y;
    last_delta_angle.z = delta_angle.z;
}

// Reset the DCM matrix and omega. Used on ground start, and on extreme errors in the matrix
void ahrsReset(bool recover_eulers)
{
    // Reset the integration terms
    omega.x = 0.0f;
    omega.y = 0.0f;
    omega.z = 0.0f;
    delta_angle.x = 0.0f;
    delta_angle.y = 0.0f;
    delta_angle.z = 0.0f;
    last_omega.x = 0.0f;
    last_omega.y = 0.0f;
    last_omega.z = 0.0f;
    last_delta_angle.x = 0.0f;
    last_delta_angle.y = 0.0f;
    last_delta_angle.z = 0.0f;
    omega_yaw_P.x = 0.0f;
    omega_yaw_P.y = 0.0f;
    omega_yaw_P.z = 0.0f;
    omega_I.x = 0.0f;
    omega_I.y = 0.0f;
    omega_I.z = 0.0f;
    omega_P.x = 0.0f;
    omega_P.y = 0.0f;
    omega_P.z = 0.0f;


    if (recover_eulers && !isnan(_roll) && !isnan(_pitch) && !isnan(_yaw)) {
        matrixFromEuler(_roll, _pitch, _yaw, &rotationMatrix);
    } else {
        // Normalise the acceleration vector
        if (calc_length_pythagorean_3D(imuMeasuredAccelBF.x, imuMeasuredAccelBF.y, imuMeasuredAccelBF.z) > 500.0f) {
            // Calculate initial pitch angle
            _pitch = atan2_approx(imuMeasuredAccelBF.x, calc_length_pythagorean_2D(imuMeasuredAccelBF.y, imuMeasuredAccelBF.z));
            // Calculate initial roll angle
            _roll = atan2_approx(imuMeasuredAccelBF.y, imuMeasuredAccelBF.z);
        } else {
            // If we can't use the accel vector, then align flat
            _roll = 0.0f;
            _pitch = 0.0f;
        }
        matrixFromEuler(_roll, _pitch, 0.0f, &rotationMatrix);
    }

    // Pre-calculate some trig for CPU purposes
    cos_yaw = cos_approx(_yaw);
    sin_yaw = sin_approx(_yaw);

    last_startup_ms = millis();
}

// Renormalise one vector component of the DCM matrix this will return false if renormalization fails
static bool renorm(fpVector3_t a, fpVector3_t *result)
{
    // Numerical errors will slowly build up over time in DCM, causing inaccuracies. 
    // We can keep ahead of those errors using the renormalization technique from the DCM IMU paper (see equations 18 to 21).

    const float renorm_val = 1.0f / calc_length_pythagorean_3D(a.x, a.y, a.z);

    if (!(renorm_val < 2.0f && renorm_val > 0.5f)) {
        // This is larger than it should get - log it as a warning
        if (!(renorm_val < 1.0e6f && renorm_val > 1.0e-6f)) {
            // We are getting values which are way out of range, we will reset the matrix and hope we can recover our attitude using drift correction before we hit the ground!
            LOG_E(AHRS, "AHRS ERROR: DCM renormalisation error");
            ahrsErrorEvent.errorCode = 4;
            return false;
        }
    }

    result->x = a.x * renorm_val;
    result->y = a.y * renorm_val;
    result->z = a.z * renorm_val;

    return true;
}

/*
 *  Direction Cosine Matrix IMU: Theory
 *  William Premerlani and Paul Bizard
 *
 *  Numerical errors will gradually reduce the orthogonality conditions expressed by equation 5 to approximations rather than identities. In effect, the axes in the two frames of reference no
 *  longer describe a rigid body. Fortunately, numerical error accumulates very slowly, so it is a simple matter to stay ahead of it.
 *  We call the process of enforcing the orthogonality conditions: renormalization.
 */
static void normalize(void)
{
    const float error = rotationMatrix.m[0][0] * rotationMatrix.m[1][0] + rotationMatrix.m[0][1] * rotationMatrix.m[1][1] + rotationMatrix.m[0][2] * rotationMatrix.m[1][2]; // eq.18
    
    // eq.19
    fpVector3_t t0 = { .v = { rotationMatrix.m[0][0] - (rotationMatrix.m[1][0] * (0.5f * error)), 
                              rotationMatrix.m[0][1] - (rotationMatrix.m[1][1] * (0.5f * error)),
                              rotationMatrix.m[0][2] - (rotationMatrix.m[1][2] * (0.5f * error)) } };
    
    // eq.19
    fpVector3_t t1 = { .v = { rotationMatrix.m[1][0] - (rotationMatrix.m[0][0] * (0.5f * error)),
                              rotationMatrix.m[1][1] - (rotationMatrix.m[0][1] * (0.5f * error)),
                              rotationMatrix.m[1][2] - (rotationMatrix.m[0][2] * (0.5f * error)) } }; 

    // eq.20
    fpVector3_t t2; 
    vectorCrossProduct(&t2, &t0, &t1);

    fpVector3_t matrixA = { .v = { rotationMatrix.m[0][0], rotationMatrix.m[0][1], rotationMatrix.m[0][2] } };
    fpVector3_t matrixB = { .v = { rotationMatrix.m[1][0], rotationMatrix.m[1][1], rotationMatrix.m[1][2] } };
    fpVector3_t matrixC = { .v = { rotationMatrix.m[2][0], rotationMatrix.m[2][1], rotationMatrix.m[2][2] } };

    const bool checkMatrixA = renorm(t0, &matrixA); 
    const bool checkMatrixB = renorm(t1, &matrixB); 
    const bool checkMatrixC = renorm(t2, &matrixC); 

    if (!checkMatrixA || !checkMatrixB || !checkMatrixC) {
        // Our solution is blowing up and we will force back to last euler angles
        last_failure_ms = millis();
        ahrsReset(true);
    }
    
    if (checkMatrixA) {
        rotationMatrix.m[0][0] = matrixA.x;
        rotationMatrix.m[0][1] = matrixA.y;
        rotationMatrix.m[0][2] = matrixA.z;
    }

    if (checkMatrixB) {
        rotationMatrix.m[1][0] = matrixB.x;
        rotationMatrix.m[1][1] = matrixB.y;
        rotationMatrix.m[1][2] = matrixB.z;
    }

    if (checkMatrixC) {
        rotationMatrix.m[2][0] = matrixC.x;
        rotationMatrix.m[2][1] = matrixC.y;
        rotationMatrix.m[2][2] = matrixC.z;
    }
}

// Check the DCM matrix for pathological values
static void checkMatrix(void)
{
    if (rotationMatrixIsNAN(rotationMatrix)) {
        LOG_E(AHRS, "AHRS Matrix is NaN. Reseting the AHRS");
        ahrsErrorEvent.errorCode = 1;
        ahrsReset(true);
        return;
    }

    // Some DCM matrix values can lead to an out of range error in the pitch calculation via asin(). These NaN values can feed back into the rest of the DCM matrix via the error_course value.
    if (!(rotationMatrix.m[2][0] < 1.0f && rotationMatrix.m[2][0] > -1.0f)) {
        // We have an invalid matrix. Force a normalisation.
        normalize();

        LOG_E(AHRS, "AHRS invalid Matrix. Forcing a new normalization");
        ahrsErrorEvent.errorCode = 2;

        if (rotationMatrixIsNAN(rotationMatrix) || fabsf(rotationMatrix.m[2][0]) > 10.0f) {
            LOG_E(AHRS, "AHRS Matrix normalisation error. Reset to last known good value");
            ahrsErrorEvent.errorCode = 3;
            ahrsReset(true);
        } 
    }
}

static void updateLogError(void) 
{
    if (prev_ahrsErrorEvent.errorCode == ahrsErrorEvent.errorCode) {
        return;
    }

    prev_ahrsErrorEvent.errorCode = ahrsErrorEvent.errorCode;

#ifdef USE_BLACKBOX
    if (feature(FEATURE_BLACKBOX)) {
        blackboxLogEvent(FLIGHT_LOG_EVENT_IMU_FAILURE, (flightLogEventData_t*)&ahrsErrorEvent);
    }
#endif
}

// Produce a yaw error value. The returned value is proportional to sin() of the current heading error in Earth-Frame
static float yawErrorCompass(void)
{
    fpVector3_t magField;

    getMagField(&magField);

    // Get the mag vector in the Earth-Frame
    matrixMulXY(&magField, rotationMatrix);
    
    const float magFieldLength = calc_length_pythagorean_2D(magField.x, magField.y);
    
    if (magFieldLength < 0.0f) {
        return 0.0f;
    }
    
    // Normalize the Mag Field
    magField.x /= magFieldLength;
    magField.y /= magFieldLength;

    if (isinf(magField.x) || isinf(magField.y)) {
        // Not a valid vector
        return 0.0f;
    }
    
#ifdef USE_SIMULATOR
    if (ARMING_FLAG(SIMULATOR_MODE)) {
        ahrsSetMagneticDeclination(0.0f);
    }
#endif

    // Calculate the Z component of the cross product of magField and mag_earth
    return magField.x * mag_earth.y - magField.y * mag_earth.x; 
}

// The _P_gain raises the gain of the PI controller when we are spinning fast.
static float proportionalGain(float spin_rate)
{
    if (spin_rate < DEGREES_TO_RADIANS(50)) {
        return 1.0f;
    }

    if (spin_rate > DEGREES_TO_RADIANS(500)) {
        return 10.0f;
    }
    
    return spin_rate / DEGREES_TO_RADIANS(50);
}

// This function reduces the gain of the PI controller applied to heading errors when observability from change of velocity is good (eg changing speed or turning)
// This reduces unwanted roll and pitch coupling due to compass errors for planes.
// High levels of noise on accel_ef will cause the gain to drop and could lead to increased heading drift during straight and level flight, however some gain is always available.
static float yawGain(void)
{
    const float VdotEFmag = CENTIMETERS_TO_METERS(calc_length_pythagorean_2D(accel_ef.x, accel_ef.y));

    if (VdotEFmag <= 4.0f) {
        return 0.2f * (4.5f - VdotEFmag);
    }

    return 0.1f;
}

// Return true if we have and should use GPS
static bool haveGPS(void)
{
    if (!STATE(GPS_FIX) || !sensors(SENSOR_GPS)) {
        return false;
    }

    return true;
}

// When we are getting the initial attitude we want faster gains so that if the board starts upside down we quickly approach the right attitude.
// We don't want to keep those high gains for too long though as high P gains cause slow gyro offset learning. So we keep the high gains for a maximum of 20 seconds
static bool useFastGains(void)
{
    return !ARMING_FLAG(ARMED) && (millis() - last_startup_ms) < 20000U;
}

// Wrap an angle in radians to -PI ~ PI (equivalent to +- 180 degrees)
float wrap_PI(const float radian)
{
    float res = fmodf(radian, 2.0f * M_PIf);

    if (res < 0.0f) {
        res += 2.0f * M_PIf;
    }

    if (res > M_PIf) {
        res -= 2.0f * M_PIf;
    }

    return res;
}

// Return true if we should use the compass for yaw correction
static bool useCompass(void)
{
    if (!sensors(SENSOR_MAG)) {
        // No compass available
        return false;
    }

    if (!fly_forward || !haveGPS()) {
        // We don't have any alterative to the compass
        return true;
    }

    if (gpsSol.groundSpeed < GPS_SPEED_MIN) {
        // We are not going fast enough to use the GPS
        return true;
    }

    // If the current yaw differs from the GPS yaw by more than 45 degrees and the estimated wind speed is less than 80% of the ground speed, then switch to GPS navigation. 
    // This will help prevent flyaways with very bad compass offsets
    const float error = RADIANS_TO_DEGREES(wrap_PI(-DEGREES_TO_RADIANS(DECIDEGREES_TO_DEGREES(gpsSol.groundCourse)) - _yaw));
    if (error > 45.0f && calc_length_pythagorean_3D(_wind.x, _wind.y, _wind.z) < gpsSol.groundSpeed * 0.8f) {
        if (millis() - last_consistent_heading > 2000) {
            // Start using the GPS for heading if the compass has been inconsistent with the GPS for 2 seconds
            return false;
        }
    } else {
        last_consistent_heading = millis();
    }

    // Use the compass
    return true;
}

// Calculate a compass heading given the attitude from DCM and the mag vector
static float calculateHeading(void)
{
    fpVector3_t magField;

    getMagField(&magField);

    float cos_pitch_sq = 1.0f - (rotationMatrix.m[2][0] * rotationMatrix.m[2][0]);

    float headY = magField.y * rotationMatrix.m[2][2] - magField.z * rotationMatrix.m[2][1];

    // Tilt compensated magnetic field X component
    float headX = magField.x * cos_pitch_sq - rotationMatrix.m[2][0] * (magField.y * rotationMatrix.m[2][1] + magField.z * rotationMatrix.m[2][2]);

    // Magnetic Heading
    float heading = constrainf(atan2_approx(-headY, headX), -M_PIf, M_PIf);

    return heading;
}

// Yaw drift correction using the compass or GPS this function prodoces the omega_yaw_P vector, and also contributes to the omega_I.z long term yaw drift estimate
static void driftCorrectionYaw(void)
{
    bool new_value = false;
    float yaw_error;
    float yaw_deltaTime;

    if (sensors(SENSOR_MAG)) {
        if (!compassIsCalibrationComplete()) {
            // Don't do any yaw correction while calibrating
            return;
        }
    }
    
    if (useCompass()) {
        // We are using compass for yaw
        if (compassLastUpdate() != compass_last_update) {
            yaw_deltaTime = US2S(compassLastUpdate() - compass_last_update);
            compass_last_update = compassLastUpdate();
            // We force an additional compass read() here. This has the effect of throwing away the first compass value, which can be bad
            if (!have_initial_yaw) {
                const float heading = calculateHeading();
                matrixFromEuler(_roll, _pitch, heading, &rotationMatrix);
                omega_yaw_P.x = 0.0f;
                omega_yaw_P.y = 0.0f;
                omega_yaw_P.z = 0.0f;
                have_initial_yaw = true;
            }
            new_value = true;
            yaw_error = yawErrorCompass();
            
            // Also update the gps_last_update, so if we later disable the compass due to significant yaw error we don't suddenly change yaw with a reset
            gps_last_update = gpsStats.lastFixTime;
        }
    } else if (fly_forward && haveGPS()) {
        // We are using GPS for yaw
        if (gpsStats.lastFixTime != gps_last_update && gpsSol.groundSpeed >= GPS_SPEED_MIN) {
            yaw_deltaTime = MS2S(gpsStats.lastFixTime - gps_last_update);
            gps_last_update = gpsStats.lastFixTime;
            const float gps_course_rad = -DEGREES_TO_RADIANS(DECIDEGREES_TO_DEGREES(gpsSol.groundCourse));
            const float yaw_error_rad = wrap_PI(gps_course_rad - _yaw);
            yaw_error = sin_approx(yaw_error_rad);
            new_value = true;

            /* 
            Reset yaw to match GPS heading under any of the following 3 conditions:

            1) if we have reached GPS_SPEED_MIN and have never had yaw information before.

            2) if the last time we got yaw information from the GPS is more than 20 seconds ago, which means we may have suffered from considerable gyro drift.

            3) if we are over GPS_SPEED_MIN * 3 (which means 900cm/s) and our yaw error is over 60 degrees, which means very poor yaw. 
            This can happen on bungee launch when the operator pulls back the plane rapidly enough then on release the GPS heading changes very rapidly.
            */

            if (!have_initial_yaw || yaw_deltaTime > 20 || (gpsSol.groundSpeed >= GPS_SPEED_MIN * 3 && fabsf(yaw_error_rad) >= 1.047f)) {
                // Reset DCM matrix based on current yaw
                matrixFromEuler(_roll, _pitch, gps_course_rad, &rotationMatrix);
                // Force reset of heading hold target
                resetHeadingHoldTarget(DECIDEGREES_TO_DEGREES(attitude.values.yaw));
                omega_yaw_P.x = 0.0f;
                omega_yaw_P.y = 0.0f;
                omega_yaw_P.z = 0.0f;
                have_initial_yaw = true;
                yaw_error = 0.0f;
            }
        }
    }
    
    if (!new_value) {
        // We don't have any new yaw information slowly decay omega_yaw_P to cope with loss of our yaw source
        omega_yaw_P.x *= 0.97f;
        omega_yaw_P.y *= 0.97f;
        omega_yaw_P.z *= 0.97f;
        return;
    }

    // Convert the error vector to Body-Frame
    const float error_z = rotationMatrix.m[2][2] * yaw_error;

    // The spin rate changes the P gain, and disables the integration at higher rates (get the value in rad/s/s)
    const float spin_rate = calc_length_pythagorean_3D(omega.x * RAD, omega.y * RAD, omega.z * RAD);
    
    float kP_Mag = (float)ahrsConfig()->dcm_kp_mag / 100.0f;

    // Sanity check kP_Mag
    if (kP_Mag < YAW_KP_MIN) {
        kP_Mag = YAW_KP_MIN;
    }

    // Update the proportional control to drag the  yaw back to the right value. We use a gain that depends on the spin rate.
    // We also adjust the gain depending on the rate of change of horizontal velocity which is proportional to how observable the heading is from the acceerations and GPS velocity.
    // The accelration derived heading will be more reliable in turns than compass or GPS.
    omega_yaw_P.z = error_z * proportionalGain(spin_rate) * kP_Mag * yawGain();

    if (useFastGains()) {
        omega_yaw_P.z *= 8.0f;
    }

    // Don't update the drift term if we lost the yaw reference for more than 2 seconds
    if (yaw_deltaTime < 2.0f && spin_rate < DEGREES_TO_RADIANS(SPIN_RATE_LIMIT)) {
        // Also add to the I term
        omega_I_sum.z += error_z * DCM_KI_MAG * yaw_deltaTime;
    }
}

// Return an accel vector delayed
static void raDelayed(fpVector3_t ra, fpVector3_t *v)
{
    // Get the old element, and then fill it with the new element
    const fpVector3_t ret = { .v = { ra_delay_buffer.x, ra_delay_buffer.y, ra_delay_buffer.z } };

    ra_delay_buffer.x = ra.x;
    ra_delay_buffer.y = ra.y;
    ra_delay_buffer.z = ra.z;

    if (ret.x == 0.0f && ret.y == 0.0f && ret.z == 0.0f) {
        // Use the current vector if the previous vector is exactly zero. This prevents an error on initialisation
        v->x = ra.x;
        v->y = ra.y;
        v->z = ra.z;
        return;
    }

    v->x = ret.x;
    v->y = ret.y;
    v->z = ret.z;
}

// Perform drift correction. This function aims to update omega_P and omega_I with our best estimate of the short term and long term gyro error. 
// The omega_P value is what pulls our attitude solution back towards the reference vector quickly. The omega_I term is an attempt to learn the long term drift rate of the gyros.
// This drift correction implementation is based on a paper by Bill Premerlani
static void driftCorrection(float deltaTime)
{
    fpVector3_t velocity;
    timeMs_t last_correction_time;

    // Perform yaw drift correction if we have a new yaw reference vector
    driftCorrectionYaw();

    // Rotate accelerometer values into the Earth-Frame
    if (deltaTime > 0.0f) {
        accel_ef.x = rotationMatrix.m[0][0] * imuMeasuredAccelBF.x + rotationMatrix.m[0][1] * imuMeasuredAccelBF.y + rotationMatrix.m[0][2] * imuMeasuredAccelBF.z;
        accel_ef.y = rotationMatrix.m[1][0] * imuMeasuredAccelBF.x + rotationMatrix.m[1][1] * imuMeasuredAccelBF.y + rotationMatrix.m[1][2] * imuMeasuredAccelBF.z;
        accel_ef.z = rotationMatrix.m[2][0] * imuMeasuredAccelBF.x + rotationMatrix.m[2][1] * imuMeasuredAccelBF.y + rotationMatrix.m[2][2] * imuMeasuredAccelBF.z;
        // Integrate the accel vector in the Earth-Frame between GPS readings
        ra_sum.x += accel_ef.x * deltaTime;
        ra_sum.y += accel_ef.y * deltaTime;
        ra_sum.z += accel_ef.z * deltaTime;
    }

    // Keep a sum of the deltaTime values, so we know how much time we have integrated over
    ra_deltaTime += deltaTime;

    if (!haveGPS() || gpsSol.numSat < 6) {
        // As a fallback we use the fixed wing acceleration correction if we have an airspeed estimate (which we only have if _fly_forward is set), otherwise no correction
        if (ra_deltaTime < 0.2f) {
            // Not enough time has accumulated
            return;
        }

        float airspeed = last_airspeed;

        if (realPitotEnabled()) {
            airspeed = getAirspeedEstimate();
        }

        // Use airspeed to estimate our ground velocity in Earth-Frame by subtracting the wind
        velocity.x = rotationMatrix.m[0][0] * airspeed;
        velocity.y = -rotationMatrix.m[1][0] * airspeed;
        velocity.z = -rotationMatrix.m[2][0] * airspeed;

        // Add in wind estimate
        velocity.x += _wind.x;
        velocity.y += _wind.y;
        velocity.z += _wind.z;

        last_correction_time = millis();

        have_gps_lock = false;
    } else {
        if (gpsStats.lastFixTime == ra_sum_start) {
            // We don't have a new GPS fix - nothing more to do
            return;
        }

        velocity.x = gpsSol.velNED[X];
        velocity.y = -gpsSol.velNED[Y];
        velocity.z = -gpsSol.velNED[Z];

        last_correction_time = gpsStats.lastFixTime;

        if (have_gps_lock == false) {
            // If we didn't have GPS lock in the last drift correction interval then set the velocities equal
            last_velocity.x = velocity.x;
            last_velocity.y = velocity.y;
            last_velocity.z = velocity.z;
        }

        have_gps_lock = true;

        // Keep last airspeed estimate for dead-reckoning purposes
        fpVector3_t airspeed = { .v = { velocity.x - _wind.x, velocity.y - _wind.y, velocity.z - _wind.z } };

        // Rotate vector to Body-Frame
        matrixMulTranspose(&airspeed, rotationMatrix);

        // Take positive component in X direction. This mimics a pitot tube
        last_airspeed = MAX(airspeed.x, 0.0f);
    }

    // See if this is our first time through - in which case we just setup the start times and return
    if (ra_sum_start == 0) {
        ra_sum_start = last_correction_time;
        last_velocity.x = velocity.x;
        last_velocity.y = velocity.y;
        last_velocity.z = velocity.z;
        return;
    }

    // Equation 9: get the corrected acceleration vector in Earth-Frame. Units are cm/s/s
    fpVector3_t GA_e = { .v = { 0.0f, 0.0f, 1.0f } };

    if (ra_deltaTime <= 0.0f) {
        // Waiting for more data
        return;
    }
    
    bool using_gps_corrections = false;
    const float ra_scale = 1.0f / (ra_deltaTime * GRAVITY_CMSS);
    const float gps_gain = (float)ahrsConfig()->dcm_gps_gain / 10.0f;
    
    const bool should_correct_centrifugal = STATE(FIXED_WING_LEGACY) ? true : ARMING_FLAG(ARMED);

    if (should_correct_centrifugal && (have_gps_lock || fly_forward)) {
        const float vel_scale = gps_gain * ra_scale;
        const fpVector3_t vel_delta = { .v = { (velocity.x - last_velocity.x) * vel_scale, (velocity.y - last_velocity.y) * vel_scale, (velocity.z - last_velocity.z) * vel_scale } };
        GA_e.x += vel_delta.x;
        GA_e.y += vel_delta.y;
        GA_e.z += vel_delta.z;
        GA_e.x /= calc_length_pythagorean_3D(GA_e.x, GA_e.y, GA_e.z);
        GA_e.y /= calc_length_pythagorean_3D(GA_e.x, GA_e.y, GA_e.z);
        GA_e.z /= calc_length_pythagorean_3D(GA_e.x, GA_e.y, GA_e.z);
        if (isinf(GA_e.x) || isinf(GA_e.y) || isinf(GA_e.z)) {
            // Wait for some non-zero acceleration information
            last_failure_ms = millis();
            return;
        }
        using_gps_corrections = true;
    }

    // Calculate the error term in Earth-Frame.
    fpVector3_t error;
    fpVector3_t GA_b;
    
    ra_sum.x *= ra_scale;
    ra_sum.y *= ra_scale;
    ra_sum.z *= ra_scale;

    // Get the delayed ra_sum to match the GPS lag
    if (using_gps_corrections) {
        raDelayed(ra_sum, &GA_b);
    } else {
        GA_b.x = ra_sum.x;
        GA_b.y = ra_sum.y;
        GA_b.z = ra_sum.z;
    
    }

    if (GA_b.x == 0.0f && GA_b.y == 0.0f && GA_b.z == 0.0f) {
        // Wait for some non-zero acceleration information
        return;
    }

    GA_b.x /= calc_length_pythagorean_3D(GA_b.x, GA_b.y, GA_b.z);
    GA_b.y /= calc_length_pythagorean_3D(GA_b.x, GA_b.y, GA_b.z);
    GA_b.z /= calc_length_pythagorean_3D(GA_b.x, GA_b.y, GA_b.z);

    if (isinf(GA_b.x) || isinf(GA_b.y) || isinf(GA_b.z)) {
        // Wait for some non-zero acceleration information
        return;
    }

    vectorCrossProduct(&error, &GA_b, &GA_e);

    // Yaw correction using the accelerometer at high roll angles as long as we have a GPS
    if (useCompass()) {
        if (haveGPS() && gps_gain == 1.0f) {
            error.z *= sin_approx(fabsf(_roll));
        } else {
            error.z = 0.0f;
        }
    }

    // If inertial sensor is unhealthy then stop attitude drift correction and hope the gyros are OK for a while. Just slowly reduce omega_P to prevent previous bad accels from throwing us off
    if (!sensors(SENSOR_ACC) && !sensors(SENSOR_GYRO)) {
        error.x = 0.0f;
        error.y = 0.0f;
        error.z = 0.0f;
    } else {
        // Convert the error term to Body-Frame
        matrixMulTranspose(&error, rotationMatrix);
    }

    if (isnan(error.x) || isnan(error.y) || isnan(error.z) ||
        isinf(error.x) || isinf(error.y) || isinf(error.z)) {
        // Don't allow bad values
        checkMatrix();
        last_failure_ms = millis();
        return;
    }
    
    // Base the P gain on the spin rate (get the value in rad/s/s)
    const float spin_rate = calc_length_pythagorean_3D(omega.x * RAD, omega.y * RAD, omega.z * RAD);

    float kP_Acc = (float)ahrsConfig()->dcm_kp_acc / 100.0f;

    // Sanity check kP_Acc value
    if (kP_Acc < RP_KP_MIN) {
        kP_Acc = RP_KP_MIN;
    }

    // We now want to calculate omega_P and omega_I. The omega_P value is what drags us quickly to the accelerometer reading.
    omega_P.x = error.x * proportionalGain(spin_rate) * kP_Acc;
    omega_P.y = error.y * proportionalGain(spin_rate) * kP_Acc;
    omega_P.z = error.z * proportionalGain(spin_rate) * kP_Acc;

    if (useFastGains()) {
        omega_P.x *= 8.0f;
        omega_P.y *= 8.0f;
        omega_P.z *= 8.0f;
    }

    if (fly_forward && haveGPS() && gpsSol.groundSpeed < GPS_SPEED_MIN && imuMeasuredAccelBF.x >= 700.0f && _pitch > DEGREES_TO_RADIANS(-30) && _pitch < DEGREES_TO_RADIANS(30)) {
        // Assume we are in a launch acceleration, and reduce the rp gain by 50% to reduce the impact of GPS lag on takeoff attitude when using a catapult
        omega_P.x *= 0.5f;
        omega_P.y *= 0.5f;
        omega_P.z *= 0.5f;
    }

    // Accumulate some integrator error
    if (spin_rate < DEGREES_TO_RADIANS(SPIN_RATE_LIMIT)) {
        omega_I_sum.x += error.x * DCM_KI_ACC * ra_deltaTime;
        omega_I_sum.y += error.y * DCM_KI_ACC * ra_deltaTime;
        omega_I_sum.z += error.z * DCM_KI_ACC * ra_deltaTime;
        omega_I_sum_time += ra_deltaTime;
    }

    if (omega_I_sum_time >= 5.0f) {
        // Limit the rate of change of omega_I to the hardware reported maximum gyro drift rate. 
        // This ensures that short term errors don't cause a buildup of omega_I beyond the physical limits of the device
        const float change_limit = DEGREES_TO_RADIANS(0.5f / 60.0f) * omega_I_sum_time;
        omega_I_sum.x = constrainf(omega_I_sum.x, -change_limit, change_limit);
        omega_I_sum.y = constrainf(omega_I_sum.y, -change_limit, change_limit);
        omega_I_sum.z = constrainf(omega_I_sum.z, -change_limit, change_limit);
        omega_I.x += omega_I_sum.x;
        omega_I.y += omega_I_sum.y;
        omega_I.z += omega_I_sum.z;
        omega_I_sum.x = 0.0f;
        omega_I_sum.y = 0.0f;
        omega_I_sum.z = 0.0f;
        omega_I_sum_time = 0.0f;
    }

    // Zero our accumulator ready for the next GPS step
    memset(&ra_sum, 0, sizeof(ra_sum));
    ra_deltaTime = 0.0f;
    ra_sum_start = last_correction_time;

    // Remember the velocity for next time
    last_velocity.x = velocity.x;
    last_velocity.y = velocity.y;
    last_velocity.z = velocity.z;
}

// Calculate Sine and Cosine of Roll, Pitch and Yaw from a NED rotation matrix
static void calculateTrigonometry(float *cr, float *cp, float *cy, float *sr, float *sp, float *sy) {

    fpVector3_t yaw_vector = { .v = { rotationMatrix.m[0][0], -rotationMatrix.m[1][0], 0.0f } };

    if (fabsf(yaw_vector.x) > 0.0f || fabsf(yaw_vector.y) > 0.0f) {
        yaw_vector.x /= calc_length_pythagorean_2D(yaw_vector.x, yaw_vector.y);
        yaw_vector.y /= calc_length_pythagorean_2D(yaw_vector.x, yaw_vector.y);
    }

    *sy = constrainf(yaw_vector.y, -1.0f, 1.0f);
    *cy = constrainf(yaw_vector.x, -1.0f, 1.0f);

    // Sanity checks
    if (isinf(yaw_vector.x) || isinf(yaw_vector.y) || 
        isnan(yaw_vector.x) || isnan(yaw_vector.y)) {
        *sy = 0.0f;
        *cy = 1.0f;
    }

    const float cx2 = rotationMatrix.m[2][0] * rotationMatrix.m[2][0];

    if (cx2 >= 1.0f) {
        *cp = 0.0f;
        *cr = 1.0f;
    } else {
        *cp = fast_fsqrtf(1.0f - cx2);
        *cr = rotationMatrix.m[2][2] / *cp;
    }

    *cp = constrainf(*cp, 0.0f, 1.0f);
    *cr = constrainf(*cr, -1.0f, 1.0f); // This relies on constrainf() of infinity doing the right thing

    *sp = -rotationMatrix.m[2][0];

    if (*cp > 0.0f) {
        *sr = rotationMatrix.m[2][1] / *cp;
    }

    if (*cp <= 0.0f || isinf(*cr) || isnan(*cr) || isinf(*sr) || isnan(*sr)) {
        const float r = atan2_approx(rotationMatrix.m[2][1], rotationMatrix.m[2][2]);
        *cr = cos_approx(r);
        *sr = sin_approx(r);
    }
}

// Run a full DCM update round
static void dcmUpdate(float deltaTime)
{
    // If the update call took more than 0.2 seconds then discard it, otherwise we may move too far.
    if (deltaTime > 0.2f) {
        memset(&ra_sum, 0, sizeof(ra_sum));
        ra_deltaTime = 0.0f;
        return;
    }
    
    if (!gyroIsCalibrationComplete()) {
        resetGyroDrift();
    }
    
    bool useHITLOutAngles = false;

#ifdef USE_SIMULATOR
    useHITLOutAngles = ARMING_FLAG(SIMULATOR_MODE) && ((simulatorData.flags & SIMU_USE_SENSORS) == 0);

    if (useHITLOutAngles) {
        matrixFromEuler(DECIDEGREES_TO_RADIANS(attitude.values.roll) * 0.5f, DECIDEGREES_TO_RADIANS(attitude.values.pitch) * 0.5f, DECIDEGREES_TO_RADIANS(attitude.values.yaw) * 0.5f, &rotationMatrix);
    }
#endif

    // Integrate the DCM matrix using gyro inputs
    matrixUpdate(deltaTime);

    // Normalize the DCM matrix
    normalize();

    // Perform drift correction
    driftCorrection(deltaTime);

    // Paranoid check for bad values in the DCM matrix
    checkMatrix();
    
    // Perform AHRS log error
    updateLogError();
    
    // Calculate the euler angles and DCM matrix which will be used for high level navigation control. 
    _roll = atan2_approx(rotationMatrix.m[2][1], rotationMatrix.m[2][2]);
    _pitch = -asin_approx(rotationMatrix.m[2][0]);
    _yaw = atan2_approx(-rotationMatrix.m[1][0], rotationMatrix.m[0][0]);

    // Pre-calculate some trig for CPU purposes
    cos_yaw = cos_approx(_yaw);
    sin_yaw = sin_approx(_yaw);
    
    if (!useHITLOutAngles) {
        attitude.values.roll = RADIANS_TO_DECIDEGREES(_roll);
        attitude.values.pitch = RADIANS_TO_DECIDEGREES(_pitch);
        attitude.values.yaw = RADIANS_TO_DECIDEGREES(_yaw);
    }

    if (attitude.values.yaw < 0) {
        attitude.values.yaw += 3600;
    }

    calculateTrigonometry(&cos_roll, &cos_pitch, &cos_yaw, &sin_roll, &sin_pitch, &sin_yaw);

    // Update small angle state 
    if (RADIANS_TO_DEGREES(ahrsGetTiltAngle()) < ahrsConfig()->small_angle) {
        ENABLE_STATE(SMALL_ANGLE);
    } else {
        DISABLE_STATE(SMALL_ANGLE);
    }
}

void ahrsUpdate(timeUs_t currentTimeUs)
{
#ifdef HIL
    if (sensors(SENSOR_ACC) && !hilActive) {
        accUpdate();
    }
#else
    if (sensors(SENSOR_ACC)) {
        accUpdate();
    }
#endif

    // Calculate the AHRS Delta-Time
    static timeUs_t previousTime;
    const float deltaTime = US2S(currentTimeUs - previousTime);
    previousTime = currentTimeUs;
    
    if (sensors(SENSOR_ACC)) {
#ifdef HIL
        if (!hilActive) {
            accGetMeasuredAcceleration(&imuMeasuredAccelBF);     // Calculate accel in Body-Frame in cm/s/s
            gyroGetMeasuredRotationRate(&imuMeasuredRotationBF); // Calculate gyro rate in Body-Frame in rad/s
            imuCheckVibrationLevels();
            dcmUpdate(deltaTime);
        }
        else {
            /* Set attitude */
            attitude.values.roll = hilToFC.rollAngle;
            attitude.values.pitch = hilToFC.pitchAngle;
            attitude.values.yaw = hilToFC.yawAngle;

            /* Compute matrix rotation for future use */
            matrixFromEuler(attitude.values.roll, attitude.values.pitch, attitude.values.yaw);

            /* Fake accADC readings */
            accADCf[X] = hilToFC.bodyAccel[X] / GRAVITY_CMSS;
            accADCf[Y] = hilToFC.bodyAccel[Y] / GRAVITY_CMSS;
            accADCf[Z] = hilToFC.bodyAccel[Z] / GRAVITY_CMSS;
            imuUpdateMeasuredAcceleration();
        }
#else
        accGetMeasuredAcceleration(&imuMeasuredAccelBF);     // Calculate accel in Body-Frame in cm/s/s
        gyroGetMeasuredRotationRate(&imuMeasuredRotationBF); // Calculate gyro rate in Body-Frame in rad/s
        imuCheckVibrationLevels();
        dcmUpdate(deltaTime);
#endif
    } else {
        acc.accADCf[X] = 0.0f;
        acc.accADCf[Y] = 0.0f;
        acc.accADCf[Z] = 0.0f;
    }
}

// Update our wind speed estimate
void ahrsUpdateWindEstimator(void)
{
    const fpVector3_t velocity = { .v = { last_velocity.x, last_velocity.y, last_velocity.z } };

    const fpVector3_t fuselageDirection = { .v = { rotationMatrix.m[0][0], rotationMatrix.m[1][0], rotationMatrix.m[2][0] } };

    const fpVector3_t fuselageDirectionDiff = { .v = { fuselageDirection.x - last_fuse.x, fuselageDirection.y - last_fuse.y, fuselageDirection.z - last_fuse.z } };

    const timeMs_t now = millis();

    // Scrap our data and start over if we're taking too long to get a direction change
    if (now - last_wind_time > 10000) {
        last_wind_time = now;
        last_fuse.x = fuselageDirection.x;
        last_fuse.y = fuselageDirection.y;
        last_fuse.z = fuselageDirection.z;
        last_vel.x = velocity.x;
        last_vel.y = velocity.y;
        last_vel.z = velocity.z;
        return;
    }

    const float diff_length = calc_length_pythagorean_3D(fuselageDirectionDiff.x, fuselageDirectionDiff.y, fuselageDirectionDiff.z);

    if (diff_length > 0.2f) {
        // When turning, use the attitude response to estimate wind speed
        const fpVector3_t velocityDiff = { .v = { velocity.x - last_vel.x, velocity.y - last_vel.y, velocity.z - last_vel.z } };

        // Estimate airspeed it using equation 6
        const float velDiff = calc_length_pythagorean_3D(velocityDiff.x, velocityDiff.y, velocityDiff.z) / diff_length;

        const fpVector3_t fuselageDirectionSum = { .v = { fuselageDirection.x + last_fuse.x, fuselageDirection.y + last_fuse.y, fuselageDirection.z + last_fuse.z } };

        const fpVector3_t velocitySum = { .v = { velocity.x + last_vel.x, velocity.y + last_vel.y, velocity.z + last_vel.z } };

        last_fuse.x = fuselageDirection.x;
        last_fuse.y = fuselageDirection.y;
        last_fuse.z = fuselageDirection.z;
        last_vel.x = velocity.x;
        last_vel.y = velocity.y;
        last_vel.z = velocity.z;

        const float theta = atan2_approx(velocityDiff.y, velocityDiff.x) - atan2_approx(fuselageDirectionDiff.y, fuselageDirectionDiff.x);
        const float sinTheta = sin_approx(theta);
        const float cosTheta = cos_approx(theta);

        fpVector3_t wind = { .v = { velocitySum.x - velDiff * (cosTheta * fuselageDirectionSum.x - sinTheta * fuselageDirectionSum.y),
                                    velocitySum.y - velDiff * (sinTheta * fuselageDirectionSum.x + cosTheta * fuselageDirectionSum.y),
                                    velocitySum.z - velDiff * fuselageDirectionSum.z } };
        wind.x *= 0.5f;
        wind.y *= 0.5f;
        wind.z *= 0.5f;

        if (calc_length_pythagorean_3D(wind.x, wind.y, wind.z) < calc_length_pythagorean_3D(_wind.x, _wind.y, _wind.z) + 2000) {
            _wind.x = _wind.x * 0.95f + wind.x * 0.05f;
            _wind.y = _wind.y * 0.95f + wind.y * 0.05f;
            _wind.z = _wind.z * 0.95f + wind.z * 0.05f;
        }

        last_wind_time = now;

        return;
    }

    // When flying straight use airspeed to get wind estimate if available
    if (now - last_wind_time > 2000 && realPitotEnabled()) {
        const fpVector3_t airspeed = { .v = { fuselageDirection.x * getAirspeedEstimate(), fuselageDirection.y * getAirspeedEstimate(), fuselageDirection.z * getAirspeedEstimate() } };
        const fpVector3_t wind = { .v = { velocity.x - airspeed.x, velocity.y - airspeed.y, velocity.z - airspeed.z  } };
        _wind.x = _wind.x * 0.92f + wind.x * 0.08f;
        _wind.y = _wind.y * 0.92f + wind.y * 0.08f;
        _wind.z = _wind.z * 0.92f + wind.z * 0.08f;
    }
}

// Wind velocity vectors in cm/s relative to the Earth-Frame
float ahrsGetEstimatedWindSpeed(uint8_t axis)
{
    return _wind.v[axis];
}

// Returns the horizontal wind angle
uint16_t ahrsGetEstimatedHorizontalWindAngle(void)
{
    float horizontalWindAngle = atan2_approx(ahrsGetEstimatedWindSpeed(Y), ahrsGetEstimatedWindSpeed(X));
    // atan2 returns [-M_PI, M_PI], with 0 indicating the vector points in the X direction
    // We want [0, 360) in degrees
    if (horizontalWindAngle < 0.0f) {
        horizontalWindAngle += (2 * M_PIf);
    }

    return RADIANS_TO_CENTIDEGREES(horizontalWindAngle);
}

// Returns the estimated horizontal wind speed (using the Wind Estimator)
float ahrsGetEstimatedHorizontalWindSpeed(void)
{
    return calc_length_pythagorean_2D(ahrsGetEstimatedWindSpeed(X), ahrsGetEstimatedWindSpeed(Y));
}

// Returns the air velocity value of the real Pitot Tube. If not, return estimated via GPS speed and wind, or give the last estimate.
float ahrsGetAirspeedEstimate(void)
{
    if (realPitotEnabled()) {
        return getAirspeedEstimate();
    }

    // Estimated via GPS speed and wind, or give the last estimate.
    return last_airspeed;
}

// Check if the AHRS subsystem is healthy
bool ahrsIsHealthy(void)
{
    // Consider ourselves healthy if there have been no failures for 5 seconds
    return (last_failure_ms == 0 || millis() - last_failure_ms > 5000);
}

bool ahrsYawInitialised(void)
{
    return (sensors(SENSOR_MAG) && STATE(COMPASS_CALIBRATED)) || (STATE(FIXED_WING_LEGACY) && have_initial_yaw);
}

// AHRS tilt angle in Radians
float ahrsGetTiltAngle(void)
{
    return acos_approx(cos_roll * cos_pitch);
}

// Convert earth-frame to body-frame
void ahrsTransformVectorEarthToBody(fpVector3_t *v)
{
    // HACK: This is needed to correctly transform from NEU (navigation) to NED (sensor frame)
    v->y = -v->y;

    matrixMulTranspose(v, rotationMatrix);
}

// Convert body-frame to earth-frame
void ahrsTransformVectorBodyToEarth(fpVector3_t *v)
{
    vectorRowTimesMat(v, rotationMatrix);

    // HACK: This is needed to correctly transform from NED (sensor frame) to NEU (navigation)
    v->y = -v->y;
}

float ahrsGetCosYaw(void)
{
    return cos_yaw;
}

float ahrsGetSinYaw(void)
{
    return sin_yaw;
}