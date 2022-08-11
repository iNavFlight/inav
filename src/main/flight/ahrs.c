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
#include "common/vector.h"
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

// AHRS - Attitude Heading Reference System

// Sanity Check
#define RP_KP_MIN 0.05f
#define YAW_KP_MIN 0.05f

// This is the speed in cm/s above which we first get a yaw lock with the GPS
#define GPS_SPEED_MIN 300

// The limit (in degrees/second) beyond which we stop integrating omega_I. At larger spin rates the DCM PI controller can get 'dizzy' which results in false gyro drift.
#define SPIN_RATE_LIMIT 20

FASTRAM fpMat3_t rotationMatrix;
FASTRAM attitudeEulerAngles_t attitude;
FASTRAM fpVector3_t imuMeasuredRotationBF;
FASTRAM fpVector3_t imuMeasuredAccelBF;
STATIC_FASTRAM fpVector3_t _accel_ef;
STATIC_FASTRAM fpVector3_t _omega;
STATIC_FASTRAM fpVector3_t _omega_P;
STATIC_FASTRAM fpVector3_t _omega_yaw_P;
STATIC_FASTRAM fpVector3_t _omega_I;
STATIC_FASTRAM fpVector3_t _omega_I_sum;
STATIC_FASTRAM fpVector3_t _mag_earth;
STATIC_FASTRAM fpVector3_t _ra_sum;
STATIC_FASTRAM fpVector3_t _ra_delay_buffer;
STATIC_FASTRAM fpVector3_t _last_velocity;
STATIC_FASTRAM fpVector3_t _wind;
STATIC_FASTRAM fpVector3_t _last_fuse;
STATIC_FASTRAM fpVector3_t _last_vel;

STATIC_FASTRAM bool _have_gps_lock;
STATIC_FASTRAM bool have_initial_yaw;
STATIC_FASTRAM bool fly_forward; // true for planes, rover and boat / false for copter

STATIC_FASTRAM float _omega_I_sum_time;
STATIC_FASTRAM float _ra_deltat;
STATIC_FASTRAM float _last_airspeed;

// euler angles
STATIC_FASTRAM float _roll;
STATIC_FASTRAM float _pitch;
STATIC_FASTRAM float _yaw;

// helper trig variables
STATIC_FASTRAM float _cos_roll = 1.0f;
STATIC_FASTRAM float _cos_pitch = 1.0f;
STATIC_FASTRAM float _cos_yaw = 1.0f;
STATIC_FASTRAM float _sin_roll;
STATIC_FASTRAM float _sin_pitch;
STATIC_FASTRAM float _sin_yaw;

timeMs_t _last_consistent_heading;
timeMs_t _last_startup_ms;
timeMs_t _ra_sum_start;
timeMs_t _last_failure_ms;
timeMs_t _gps_last_update;
timeMs_t _last_wind_time;
timeUs_t _compass_last_update;

// not configurable
static float _ki = 0.0087f;
static float _ki_yaw = 0.01f;

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
    
    // Matrix identity
    // A
    rotationMatrix.m[0][0] = 1.0f;
    rotationMatrix.m[0][1] = 0.0f;
    rotationMatrix.m[0][2] = 0.0f;
    // B
    rotationMatrix.m[1][0] = 0.0f;
    rotationMatrix.m[1][1] = 1.0f;
    rotationMatrix.m[1][2] = 0.0f;
    // C
    rotationMatrix.m[2][0] = 0.0f;
    rotationMatrix.m[2][1] = 0.0f;
    rotationMatrix.m[2][2] = 1.0f;

    ahrsReset(false);

    if (STATE(FIXED_WING_LEGACY)) {
        fly_forward = true;
    }
}

void ahrsSetMagneticDeclination(float declinationDeg)
{
    const float declinationRad = -DEGREES_TO_RADIANS(declinationDeg);
    
    _mag_earth.x = cos_approx(declinationRad);
    _mag_earth.y = sin_approx(declinationRad);
    _mag_earth.z = 0.0f;
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

// reset the current gyro drift estimate should be called if gyro offsets are recalculated
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
            if (isnan(rotationMatrix.m[i][ii])) {
                isNaN = true;
                break;
            }
        }
    }

    return isNaN;
}

// Create a rotation matrix given some euler angles
void from_euler(float roll, float pitch, float yaw)
{
    const float cp = cos_approx(pitch);
    const float sp = sin_approx(pitch);
    const float sr = sin_approx(roll);
    const float cr = cos_approx(roll);
    const float sy = sin_approx(yaw);
    const float cy = cos_approx(yaw);

    rotationMatrix.m[0][0] = cp * cy;
    rotationMatrix.m[0][1] = (sr * sp * cy) - (cr * sy);
    rotationMatrix.m[0][2] = (cr * sp * cy) + (sr * sy);

    rotationMatrix.m[1][0] = cp * sy;
    rotationMatrix.m[1][1] = (sr * sp * sy) + (cr * cy);
    rotationMatrix.m[1][2] = (cr * sp * sy) - (sr * cy);

    rotationMatrix.m[2][0] = -sp;
    rotationMatrix.m[2][1] = sr * cp;
    rotationMatrix.m[2][2] = cr * cp;
}

// Apply an additional rotation from a body frame gyro vector to a rotation matrix.
void rotate(const fpVector3_t g)
{
    rotationMatrix.m[0][0] += rotationMatrix.m[0][1] * g.z - rotationMatrix.m[0][2] * g.y;
    rotationMatrix.m[0][1] += rotationMatrix.m[0][2] * g.x - rotationMatrix.m[0][0] * g.z;
    rotationMatrix.m[0][2] += rotationMatrix.m[0][0] * g.y - rotationMatrix.m[0][1] * g.x;

    rotationMatrix.m[1][0] += rotationMatrix.m[1][1] * g.z - rotationMatrix.m[1][2] * g.y;
    rotationMatrix.m[1][1] += rotationMatrix.m[1][2] * g.x - rotationMatrix.m[1][0] * g.z;
    rotationMatrix.m[1][2] += rotationMatrix.m[1][0] * g.y - rotationMatrix.m[1][1] * g.x;

    rotationMatrix.m[2][0] += rotationMatrix.m[2][1] * g.z - rotationMatrix.m[2][2] * g.y;
    rotationMatrix.m[2][1] += rotationMatrix.m[2][2] * g.x - rotationMatrix.m[2][0] * g.z;
    rotationMatrix.m[2][2] += rotationMatrix.m[2][0] * g.y - rotationMatrix.m[2][1] * g.x;
}

// multiplication of transpose by a vector
void mul_transpose(fpVector3_t *v)
{
    fpVector3_t v2;
    v2.x = v->x;
    v2.y = v->y;
    v2.z = v->z;

    v->x = rotationMatrix.m[0][0] * v2.x + rotationMatrix.m[1][0] * v2.y + rotationMatrix.m[2][0] * v2.z;
    v->y = rotationMatrix.m[0][1] * v2.x + rotationMatrix.m[1][1] * v2.y + rotationMatrix.m[2][1] * v2.z;
    v->z = rotationMatrix.m[0][2] * v2.x + rotationMatrix.m[1][2] * v2.y + rotationMatrix.m[2][2] * v2.z;                
}

// multiplication by a vector, extracting only the xy components
void mulXY(fpVector3_t *v) 
{
    fpVector3_t v2;
    v2.x = v->x;
    v2.y = v->y;
    v2.z = v->z;

    v->x = rotationMatrix.m[0][0] * v2.x + rotationMatrix.m[0][1] * v2.y + rotationMatrix.m[0][2] * v2.z;
    v->y = rotationMatrix.m[1][0] * v2.x + rotationMatrix.m[1][1] * v2.y + rotationMatrix.m[1][2] * v2.z; 
}

// Update the DCM matrix using only the gyros
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

// reset the DCM matrix and omega. Used on ground start, and on extreme errors in the matrix
void ahrsReset(bool recover_eulers)
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
        if (calc_length_pythagorean_3D(imuMeasuredAccelBF.x, imuMeasuredAccelBF.y, imuMeasuredAccelBF.z) > 500.0f) {
            // calculate initial pitch angle
            _pitch = atan2_approx(imuMeasuredAccelBF.x, calc_length_pythagorean_2D(imuMeasuredAccelBF.y, imuMeasuredAccelBF.z));
            // calculate initial roll angle
            _roll = atan2_approx(imuMeasuredAccelBF.y, imuMeasuredAccelBF.z);
        } else {
            // If we can't use the accel vector, then align flat
            _roll = 0.0f;
            _pitch = 0.0f;
        }
    
        from_euler(_roll, _pitch, 0.0f);
    }

    // pre-calculate some trig for CPU purposes:
    _cos_yaw = cos_approx(_yaw);
    _sin_yaw = sin_approx(_yaw);

    _last_startup_ms = millis();
}

// renormalise one vector component of the DCM matrix this will return false if renormalization fails
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
    const float error = rotationMatrix.m[0][0] * rotationMatrix.m[1][0] + rotationMatrix.m[0][1] * rotationMatrix.m[1][1] + rotationMatrix.m[0][2] * rotationMatrix.m[1][2]; // eq.18

    fpVector3_t t0;
    t0.x = rotationMatrix.m[0][0] - (rotationMatrix.m[1][0] * (0.5f * error)); // eq.19
    t0.y = rotationMatrix.m[0][1] - (rotationMatrix.m[1][1] * (0.5f * error)); // eq.19
    t0.z = rotationMatrix.m[0][2] - (rotationMatrix.m[1][2] * (0.5f * error)); // eq.19

    fpVector3_t t1;
    t1.x = rotationMatrix.m[1][0] - (rotationMatrix.m[0][0] * (0.5f * error)); // eq.19
    t1.y = rotationMatrix.m[1][1] - (rotationMatrix.m[0][1] * (0.5f * error)); // eq.19
    t1.z = rotationMatrix.m[1][2] - (rotationMatrix.m[0][2] * (0.5f * error)); // eq.19

    fpVector3_t t2; 
    vectorCrossProduct(&t2, &t0, &t1); // eq.20

    fpVector3_t matrixA = { .v = { rotationMatrix.m[0][0], rotationMatrix.m[0][1], rotationMatrix.m[0][2] } };
    fpVector3_t matrixB = { .v = { rotationMatrix.m[1][0], rotationMatrix.m[1][1], rotationMatrix.m[1][2] } };
    fpVector3_t matrixC = { .v = { rotationMatrix.m[2][0], rotationMatrix.m[2][1], rotationMatrix.m[2][2] } };

    const bool checkMatrixA = renorm(t0, &matrixA); 
    const bool checkMatrixB = renorm(t1, &matrixB); 
    const bool checkMatrixC = renorm(t2, &matrixC); 

    if (!checkMatrixA || 
        !checkMatrixB || 
        !checkMatrixC) {
        // Our solution is blowing up and we will force back to last euler angles
        _last_failure_ms = millis();
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

/*
 *  check the DCM matrix for pathological values
 */
void check_matrix(void)
{
    if (rotationMatrixIsNAN()) {
        ahrsReset(true);
        return;
    }

    flightLogEvent_IMUError_t imuErrorEvent;

    // Some DCM matrix values can lead to an out of range error in the pitch calculation via asin(). 
    // These NaN values can feed back into the rest of the DCM matrix via the error_course value.
    if (!(rotationMatrix.m[2][0] < 1.0f && rotationMatrix.m[2][0] > -1.0f)) {
        // We have an invalid matrix. Force a normalisation.
        normalize();

        LOG_E(IMU, "AHRS invalid Matrix. Forcing a new normalization");
        imuErrorEvent.errorCode = 1;

        if (rotationMatrixIsNAN() || fabsf(rotationMatrix.m[2][0]) > 10.0f) {
            LOG_E(IMU, "AHRS Matrix normalisation error. Reset to last known good value");
            imuErrorEvent.errorCode = 2;
            ahrsReset(true);
        }
    }

#ifdef USE_BLACKBOX
    if (feature(FEATURE_BLACKBOX)) {
        blackboxLogEvent(FLIGHT_LOG_EVENT_IMU_FAILURE, (flightLogEventData_t*)&imuErrorEvent);
    }
#endif
}

void getMagField(fpVector3_t *v) {

    const float range_scale = 1000.0f / 3000.0f; // scale for QMC5883

    v->x = (float)mag.magADC[X] * range_scale;
    v->y = (float)mag.magADC[Y] * range_scale;
    v->z = (float)mag.magADC[Z] * range_scale;
}

// produce a yaw error value. The returned value is proportional to sin() of the current heading error in earth frame
float yaw_error_compass(void)
{
    fpVector3_t magField;

    getMagField(&magField);

    // get the mag vector in the earth frame
    mulXY(&magField);
    
    const float magFieldLength = calc_length_pythagorean_2D(magField.x, magField.y);
    
    if (magFieldLength < 0.0f) {
        return 0.0f;
    }
    
    // Normalize the Mag Field
    magField.x /= magFieldLength;
    magField.y /= magFieldLength;

    if (isinf(magField.x) || isinf(magField.y)) {
        // not a valid vector
        return 0.0f;
    }

    // calculate the error term in earth frame
    // calculate the Z component of the cross product of magField and _mag_earth
    return magField.x * _mag_earth.y - magField.y * _mag_earth.x; 
}

// the _P_gain raises the gain of the PI controller when we are spinning fast. See the fastRotations paper from Bill.
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
    const float VdotEFmag = calc_length_pythagorean_2D(_accel_ef.x, _accel_ef.y) * 0.01f;

    if (VdotEFmag <= 4.0f) {
        return 0.2f * (4.5f - VdotEFmag);
    }

    return 0.1f;
}

// return true if we have and should use GPS
bool have_gps(void)
{
    if (!STATE(GPS_FIX) || !sensors(SENSOR_GPS)) {
        return false;
    }

    return true;
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

float wrap_360(const float angle)
{
    float res = fmodf(angle, 360.0f);
    if (res < 0.0f) {
        res += 360.0f;
    }
    return res;
}

float wrap_180(const float angle)
{
    float res = wrap_360(angle);
    if (res > 180.0f) {
        res -= 360.0f;
    }
    return res;
}

#define M_2PI (2.0f * M_PIf)

float wrap_2PI(const float radian)
{
    float res = fmodf(radian, M_2PI);
    if (res < 0.0f) {
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

    if (!fly_forward || !have_gps()) {
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
    const float error = fabsf(wrap_180(RADIANS_TO_DEGREES(_yaw) - wrap_360(gpsSol.groundCourse / 10.0f)));
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

// calculate a compass heading given the attitude from DCM and the mag vector
float calculate_heading(void)
{
    fpVector3_t magField;

    getMagField(&magField);

    float cos_pitch_sq = 1.0f - (rotationMatrix.m[2][0] * rotationMatrix.m[2][0]);

    float headY = magField.y * rotationMatrix.m[2][2] - magField.z * rotationMatrix.m[2][1];

    // Tilt compensated magnetic field X component:
    float headX = magField.x * cos_pitch_sq - rotationMatrix.m[2][0] * (magField.y * rotationMatrix.m[2][1] + magField.z * rotationMatrix.m[2][2]);

    // magnetic heading
    // 6/4/11 - added constrain to keep bad values from ruining DCM Yaw - Jason S.
    float heading = constrainf(atan2_approx(-headY, headX), -M_PIf, M_PIf);

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

    if (sensors(SENSOR_MAG)) {
        if (!compassIsCalibrationComplete()) {
            // don't do any yaw correction while calibrating
            return;
        }
    }
    
    if (use_compass()) {
        /*
          we are using compass for yaw
        */
        if (compassLastUpdate() != _compass_last_update) {
            yaw_deltat = US2S(compassLastUpdate() - _compass_last_update);
            _compass_last_update = compassLastUpdate();
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
            
            // also update the _gps_last_update, so if we later
            // disable the compass due to significant yaw error we
            // don't suddenly change yaw with a reset
            _gps_last_update = gpsStats.lastFixTime;
        }
    } else if (fly_forward && have_gps()) {
        /*
          we are using GPS for yaw
         */

        //DEBUG_SET(DEBUG_CRUISE, 1, gpsStats.lastFixTime);
        //DEBUG_SET(DEBUG_CRUISE, 2, _gps_last_update);

        if (gpsStats.lastFixTime != _gps_last_update && gpsSol.groundSpeed >= GPS_SPEED_MIN) {
            yaw_deltat = MS2S(gpsStats.lastFixTime - _gps_last_update);
            _gps_last_update = gpsStats.lastFixTime;
            new_value = true;
            const float gps_course_rad = DEGREES_TO_RADIANS(wrap_360(gpsSol.groundCourse / 10.0f));
            const float yaw_error_rad = wrap_PI(gps_course_rad - _yaw);
            yaw_error = sin_approx(yaw_error_rad);

            /* reset yaw to match GPS heading under any of the
               following 3 conditions:

               1) if we have reached GPS_SPEED_MIN and have never had
               yaw information before

               2) if the last time we got yaw information from the GPS
               is more than 20 seconds ago, which means we may have
               suffered from considerable gyro drift

               3) if we are over GPS_SPEED_MIN * 3 (which means 900cm/s)
               and our yaw error is over 60 degrees, which means very
               poor yaw. This can happen on bungee launch when the
               operator pulls back the plane rapidly enough then on
               release the GPS heading changes very rapidly
            */

            if (!have_initial_yaw || yaw_deltat > 20 || (gpsSol.groundSpeed >= GPS_SPEED_MIN * 3 && fabsf(yaw_error_rad) >= 1.047f)) {
                // reset DCM matrix based on current yaw
                from_euler(_roll, _pitch, gps_course_rad);
                // Force reset of heading hold target
                resetHeadingHoldTarget(DECIDEGREES_TO_DEGREES(attitude.values.yaw));
                _omega_yaw_P.x = 0.0f;
                _omega_yaw_P.y = 0.0f;
                _omega_yaw_P.z = 0.0f;
                have_initial_yaw = true;
                yaw_error = 0.0f;
            }
        }
    }
    
    //DEBUG_SET(DEBUG_CRUISE, 0, yaw_deltat * 1000);

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
    const float error_z = rotationMatrix.m[2][2] * yaw_error;

    // the spin rate changes the P gain, and disables the
    // integration at higher rates
    const float spin_rate = calc_length_pythagorean_3D(_omega.x, _omega.y, _omega.z);
    
    float kP_Mag = (float)ahrsConfig()->dcm_kp_mag / 100.0f;

    // sanity check kP_Mag
    if (kP_Mag < YAW_KP_MIN) {
        kP_Mag = YAW_KP_MIN;
    }

    // update the proportional control to drag the
    // yaw back to the right value. We use a gain
    // that depends on the spin rate. See the fastRotations.pdf
    // paper from Bill Premerlani
    // We also adjust the gain depending on the rate of change of horizontal velocity which
    // is proportional to how observable the heading is from the acceerations and GPS velocity
    // The accelration derived heading will be more reliable in turns than compass or GPS

    _omega_yaw_P.z = error_z * _P_gain(spin_rate) * kP_Mag * _yaw_gain();

    if (use_fast_gains()) {
        _omega_yaw_P.z *= 8.0f;
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
void ra_delayed(fpVector3_t ra, fpVector3_t *v)
{
    // get the old element, and then fill it with the new element
    fpVector3_t ret;
    ret.x = _ra_delay_buffer.x;
    ret.y = _ra_delay_buffer.y;
    ret.z = _ra_delay_buffer.z;

    _ra_delay_buffer.x = ra.x;
    _ra_delay_buffer.y = ra.y;
    _ra_delay_buffer.z = ra.z;

    if (ret.x == 0.0f && ret.y == 0.0f && ret.z == 0.0f) {
        // use the current vector if the previous vector is exactly
        // zero. This prevents an error on initialisation
        v->x = ra.x;
        v->y = ra.y;
        v->z = ra.z;
        return;
    }

    v->x = ret.x;
    v->y = ret.y;
    v->z = ret.z;
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
    timeMs_t last_correction_time;

    // perform yaw drift correction if we have a new yaw reference
    // vector
    drift_correction_yaw();

    // rotate accelerometer values into the earth frame
    /*
        by using get_imuMeasuredAccelBFinMss() instead of get_accel() the
        accel value is sampled over the right time delta for
        each sensor, which prevents an aliasing effect
    */
    if (deltat > 0.0f) {
        _accel_ef.x = rotationMatrix.m[0][0] * imuMeasuredAccelBF.x + rotationMatrix.m[0][1] * imuMeasuredAccelBF.y + rotationMatrix.m[0][2] * imuMeasuredAccelBF.z;
        _accel_ef.y = rotationMatrix.m[1][0] * imuMeasuredAccelBF.x + rotationMatrix.m[1][1] * imuMeasuredAccelBF.y + rotationMatrix.m[1][2] * imuMeasuredAccelBF.z;
        _accel_ef.z = rotationMatrix.m[2][0] * imuMeasuredAccelBF.x + rotationMatrix.m[2][1] * imuMeasuredAccelBF.y + rotationMatrix.m[2][2] * imuMeasuredAccelBF.z;
        // integrate the accel vector in the earth frame between GPS readings
        _ra_sum.x += _accel_ef.x * deltat;
        _ra_sum.y += _accel_ef.y * deltat;
        _ra_sum.z += _accel_ef.z * deltat;
    }

    // keep a sum of the deltat values, so we know how much time we have integrated over
    _ra_deltat += deltat;

    if (!have_gps() || gpsSol.numSat < 6) {
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

        if (realPitotEnabled()) {
            airspeed = pitotCalculateAirSpeed();
        }

        // use airspeed to estimate our ground velocity in earth frame by subtracting the wind
        velocity.x = rotationMatrix.m[0][0] * airspeed;
        velocity.y = rotationMatrix.m[1][0] * airspeed;
        velocity.z = rotationMatrix.m[2][0] * airspeed;

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
            // if we didn't have GPS lock in the last drift correction interval then set the velocities equal
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

        // take positive component in X direction. This mimics a pitot tube
        _last_airspeed = MAX(airspeed.x, 0.0f);
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
    const float gps_gain = (float)ahrsConfig()->dcm_gps_gain / 10.0f;
    
    const bool should_correct_centrifugal = STATE(FIXED_WING_LEGACY) ? true : ARMING_FLAG(ARMED);
/*
    if (should_correct_centrifugal && (_have_gps_lock || fly_forward)) {
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
    }*/

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
        ra_delayed(_ra_sum, &GA_b);
    } else {
        GA_b.x = _ra_sum.x;
        GA_b.y = _ra_sum.y;
        GA_b.z = _ra_sum.z;
    
    }

    if (GA_b.x == 0.0f && GA_b.y == 0.0f && GA_b.z == 0.0f) {
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
            error.z = 0.0f;
        }
    }

    // if inertial sensor is unhealthy then stop attitude drift correction and hope the gyros are OK for a while. Just slowly reduce _omega_P to prevent previous bad accels from throwing us off
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
    
    float kP_Acc = (float)ahrsConfig()->dcm_kp_acc / 100.0f;

    // sanity check kP_Acc value
    if (kP_Acc < RP_KP_MIN) {
        kP_Acc = RP_KP_MIN;
    }

    // we now want to calculate _omega_P and _omega_I. The
    // _omega_P value is what drags us quickly to the
    // accelerometer reading.
    _omega_P.x = error.x * _P_gain(spin_rate) * kP_Acc;
    _omega_P.y = error.y * _P_gain(spin_rate) * kP_Acc;
    _omega_P.z = error.z * _P_gain(spin_rate) * kP_Acc;

    if (use_fast_gains()) {
        _omega_P.x *= 8.0f;
        _omega_P.y *= 8.0f;
        _omega_P.z *= 8.0f;
    }

    if (fly_forward && have_gps() && gpsSol.groundSpeed < GPS_SPEED_MIN && imuMeasuredAccelBF.x >= 700.0f && _pitch > DEGREES_TO_RADIANS(-30) && _pitch < DEGREES_TO_RADIANS(30)) {
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

    if (_omega_I_sum_time >= 5.0f) {
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
        _omega_I_sum.x = 0.0f;
        _omega_I_sum.y = 0.0f;
        _omega_I_sum.z = 0.0f;
        _omega_I_sum_time = 0.0f;
    }

    // zero our accumulator ready for the next GPS step
    memset((void *)&_ra_sum, 0, sizeof(_ra_sum));
    _ra_deltat = 0.0f;
    _ra_sum_start = last_correction_time;

    // remember the velocity for next time
    _last_velocity.x = velocity.x;
    _last_velocity.y = velocity.y;
    _last_velocity.z = velocity.z;
}

/*
  calculate sin and cos of roll/pitch/yaw from a body_to_ned rotation matrix
 */
void calc_trig(float *cr, float *cp, float *cy, float *sr, float *sp, float *sy) {

    fpVector3_t yaw_vector;
    yaw_vector.x = -rotationMatrix.m[0][0];
    yaw_vector.y = -rotationMatrix.m[1][0];
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

    const float cx2 = rotationMatrix.m[2][0] * rotationMatrix.m[2][0];

    if (cx2 >= 1.0f) {
        *cp = 0.0f;
        *cr = 1.0f;
    } else {
        *cp = fast_fsqrtf(1.0f - cx2);
        *cr = rotationMatrix.m[2][2] / *cp;
    }

    *cp = constrainf(*cp, 0.0f, 1.0f);
    *cr = constrainf(*cr, -1.0f, 1.0f); // this relies on constrain_float() of infinity doing the right thing

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

// run a full DCM update round
void dcmUpdate(float delta_t)
{
    // if the update call took more than 0.2 seconds then discard it, otherwise we may move too far.
    if (delta_t > 0.2f) {
        memset((void *)&_ra_sum, 0, sizeof(_ra_sum));
        _ra_deltat = 0.0f;
        return;
    }
    
    if (!gyroIsCalibrationComplete()) {
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
    _roll = atan2_approx(rotationMatrix.m[2][1], rotationMatrix.m[2][2]);
    _pitch = -asin_approx(rotationMatrix.m[2][0]);
    _yaw = -atan2_approx(rotationMatrix.m[1][0], rotationMatrix.m[0][0]);

    // pre-calculate some trig for CPU purposes:
    _cos_yaw = cos_approx(_yaw);
    _sin_yaw = sin_approx(_yaw);
    
    attitude.values.roll = RADIANS_TO_DECIDEGREES(_roll);
    attitude.values.pitch = RADIANS_TO_DECIDEGREES(_pitch);
    attitude.values.yaw = RADIANS_TO_DECIDEGREES(_yaw);

    if (attitude.values.yaw < 0) {
        attitude.values.yaw += 3600;
    }

    calc_trig(&_cos_roll, &_cos_pitch, &_cos_yaw, &_sin_roll, &_sin_pitch, &_sin_yaw);

    DEBUG_SET(DEBUG_CRUISE, 0, RADIANS_TO_DECIDEGREES(_cos_roll));
    DEBUG_SET(DEBUG_CRUISE, 1, RADIANS_TO_DECIDEGREES(_cos_pitch));

    DEBUG_SET(DEBUG_CRUISE, 2, RADIANS_TO_DECIDEGREES(_cos_yaw));
    DEBUG_SET(DEBUG_CRUISE, 3, RADIANS_TO_DECIDEGREES(_sin_roll));

    DEBUG_SET(DEBUG_CRUISE, 4, RADIANS_TO_DECIDEGREES(_sin_pitch));
    DEBUG_SET(DEBUG_CRUISE, 5, RADIANS_TO_DECIDEGREES(_sin_yaw));

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

    /* Calculate dT */
    static timeUs_t previousIMUUpdateTimeUs;
    const float dT = US2S(currentTimeUs - previousIMUUpdateTimeUs);
    previousIMUUpdateTimeUs = currentTimeUs;
    
    if (sensors(SENSOR_ACC)) {
#ifdef HIL
        if (!hilActive) {
            accGetMeasuredAcceleration(&imuMeasuredAccelBF);     // Calculate accel in body frame in cm/s/s
            gyroGetMeasuredRotationRate(&imuMeasuredRotationBF); // Calculate gyro rate in body frame in rad/s
            imuCheckVibrationLevels();
            dcmUpdate(dT);
        }
        else {
            /* Set attitude */
            attitude.values.roll = hilToFC.rollAngle;
            attitude.values.pitch = hilToFC.pitchAngle;
            attitude.values.yaw = hilToFC.yawAngle;

            /* Compute matrix rotation for future use */
            from_euler(attitude.values.roll, attitude.values.pitch, attitude.values.yaw);

            /* Fake accADC readings */
            accADCf[X] = hilToFC.bodyAccel[X] / GRAVITY_CMSS;
            accADCf[Y] = hilToFC.bodyAccel[Y] / GRAVITY_CMSS;
            accADCf[Z] = hilToFC.bodyAccel[Z] / GRAVITY_CMSS;
            imuUpdateMeasuredAcceleration();
        }
#else
        accGetMeasuredAcceleration(&imuMeasuredAccelBF);     // Calculate accel in body frame in cm/s/s
        gyroGetMeasuredRotationRate(&imuMeasuredRotationBF); // Calculate gyro rate in body frame in rad/s
        imuCheckVibrationLevels();
        dcmUpdate(dT);
#endif
    } else {
        acc.accADCf[X] = 0.0f;
        acc.accADCf[Y] = 0.0f;
        acc.accADCf[Z] = 0.0f;
    }
}

// update our wind speed estimate
void updateWindEstimator(void)
{
    fpVector3_t velocity;
    velocity.x = _last_velocity.x;
    velocity.y = _last_velocity.y;
    velocity.z = _last_velocity.z;

    // this is based on the wind speed estimation code from MatrixPilot by Bill Premerlani. Adaption for ArduPilot by Jon Challinger
    // See http://gentlenav.googlecode.com/files/WindEstimation.pdf
    fpVector3_t fuselageDirection;
    fuselageDirection.x = rotationMatrix.m[0][0];
    fuselageDirection.y = rotationMatrix.m[1][0];
    fuselageDirection.z = rotationMatrix.m[2][0];

    fpVector3_t fuselageDirectionDiff;
    fuselageDirectionDiff.x = fuselageDirection.x - _last_fuse.x;
    fuselageDirectionDiff.y = fuselageDirection.y - _last_fuse.y;
    fuselageDirectionDiff.z = fuselageDirection.z - _last_fuse.z;

    const timeMs_t now = millis();

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

    if (now - _last_wind_time > 2000 && realPitotEnabled()) {
        // when flying straight use airspeed to get wind estimate if available
        fpVector3_t airspeed;
        airspeed.x = fuselageDirection.x * pitotCalculateAirSpeed();
        airspeed.y = fuselageDirection.y * pitotCalculateAirSpeed();
        airspeed.z = fuselageDirection.z * pitotCalculateAirSpeed();
        fpVector3_t wind;
        wind.x = velocity.x - airspeed.x;
        wind.y = velocity.y - airspeed.y;
        wind.z = velocity.z - airspeed.z;
        _wind.x = _wind.x * 0.92f + wind.x * 0.08f;
        _wind.y = _wind.y * 0.92f + wind.y * 0.08f;
        _wind.z = _wind.z * 0.92f + wind.z * 0.08f;
    }
}

float getEstimatedWindSpeed(int axis)
{
    return _wind.v[axis];
}

float getEstimatedHorizontalWindSpeed(uint16_t *angle)
{
    float xWindSpeed = getEstimatedWindSpeed(X);
    float yWindSpeed = getEstimatedWindSpeed(Y);
    if (angle) {
        float horizontalWindAngle = atan2_approx(yWindSpeed, xWindSpeed);
        // atan2 returns [-M_PI, M_PI], with 0 indicating the vector points in the X direction
        // We want [0, 360) in degrees
        if (horizontalWindAngle < 0) {
            horizontalWindAngle += 2 * M_PIf;
        }
        *angle = RADIANS_TO_CENTIDEGREES(horizontalWindAngle);
    }
    return calc_length_pythagorean_2D(xWindSpeed, yWindSpeed);
}

// airspeed_ret: will always be filled-in by get_unconstrained_airspeed_estimate which fills in airspeed_ret in this order:
//               airspeed as filled-in by an enabled airsped sensor
//               if no airspeed sensor: airspeed estimated using the GPS speed & wind_speed_estimation
//               Or if none of the above, fills-in using the previous airspeed estimate
// Return false: if we are using the previous airspeed estimate
bool airspeed_estimate(float *airspeed_ret)
{
    if (realPitotEnabled()) {
        *airspeed_ret = pitotCalculateAirSpeed();
        return true;
    }

    if (virtualPitotEnabled() && have_gps()) {
        // estimated via GPS speed and wind
        *airspeed_ret = _last_airspeed;
        return true;
    }

    // Else give the last estimate. This is used by the dead-reckoning code
    *airspeed_ret = _last_airspeed;

    return false;
}

// check if the AHRS subsystem is healthy
bool ahrsIsHealthy(void)
{
    // consider ourselves healthy if there have been no failures for 5 seconds
    return (_last_failure_ms == 0 || millis() - _last_failure_ms > 5000);
}

bool isAhrsHeadingValid(void)
{
    return (sensors(SENSOR_MAG) && STATE(COMPASS_CALIBRATED)) || (STATE(FIXED_WING_LEGACY) && have_initial_yaw);
}

float ahrsGetTiltAngle(void)
{
    return acos_approx(_cos_roll * _cos_pitch);
}

// Convert earth frame to body frame
void ahrsTransformVectorEarthToBody(fpVector3_t * v)
{
    fpVector3_t ef_vector;
    ef_vector.x = v->x;
    ef_vector.y = v->y;
    ef_vector.z = v->z;

    v->x = ef_vector.x - _sin_pitch * ef_vector.z;
    v->y = _cos_roll  * ef_vector.y + _sin_roll * _cos_pitch * ef_vector.z;
    v->z = _sin_roll * ef_vector.y + _cos_pitch * _cos_roll * ef_vector.z;
}

// Convert earth frame to body frame
void ahrsTransformVectorBodyToEarth(fpVector3_t * v)
{
    // avoid divide by zero
    if (_cos_pitch == 0.0f) {
        return;
    }
    
    fpVector3_t bf_vector;
    bf_vector.x = v->x;
    bf_vector.y = v->y;
    bf_vector.z = v->z;

    v->x = bf_vector.x + _sin_roll * (_sin_pitch / _cos_pitch) * bf_vector.y + _cos_roll * (_sin_pitch / _cos_pitch) * bf_vector.z;
    v->y = _cos_roll  * bf_vector.y - _sin_roll * bf_vector.z;
    v->z = (_sin_roll / _cos_pitch) * bf_vector.y + (_cos_roll / _cos_pitch) * bf_vector.z;
}

float ahrsGetCosYaw(void)
{
    return _cos_yaw;
}

float ahrsGetSinYaw(void)
{
    return _sin_yaw;
}