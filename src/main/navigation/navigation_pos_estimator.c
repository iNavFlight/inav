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

#include "build/build_config.h"
#include "build/debug.h"

#include "common/axis.h"
#include "common/log.h"
#include "common/maths.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/time.h"

#include "fc/config.h"
#include "fc/settings.h"
#include "fc/rc_modes.h"

#include "flight/imu.h"

#include "io/gps.h"

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"
#include "navigation/navigation_pos_estimator_private.h"

#include "sensors/acceleration.h"
#include "sensors/barometer.h"
#include "sensors/compass.h"
#include "sensors/gyro.h"
#include "sensors/opflow.h"
#include "sensors/pitotmeter.h"
#include "sensors/sensors.h"

navigationPosEstimator_t posEstimator;
static float initialBaroAltitudeOffset = 0.0f;

PG_REGISTER_WITH_RESET_TEMPLATE(positionEstimationConfig_t, positionEstimationConfig, PG_POSITION_ESTIMATION_CONFIG, 8);

PG_RESET_TEMPLATE(positionEstimationConfig_t, positionEstimationConfig,
        // Inertial position estimator parameters
        .automatic_mag_declination = SETTING_INAV_AUTO_MAG_DECL_DEFAULT,
        .reset_altitude_type = SETTING_INAV_RESET_ALTITUDE_DEFAULT,
        .reset_home_type = SETTING_INAV_RESET_HOME_DEFAULT,
        .gravity_calibration_tolerance = SETTING_INAV_GRAVITY_CAL_TOLERANCE_DEFAULT,  // 5 cm/s/s calibration error accepted (0.5% of gravity)
        .allow_dead_reckoning = SETTING_INAV_ALLOW_DEAD_RECKONING_DEFAULT,

        .max_surface_altitude = SETTING_INAV_MAX_SURFACE_ALTITUDE_DEFAULT,

        .w_z_baro_p = SETTING_INAV_W_Z_BARO_P_DEFAULT,
        .w_z_baro_v = SETTING_INAV_W_Z_BARO_V_DEFAULT,

        .w_z_surface_p = SETTING_INAV_W_Z_SURFACE_P_DEFAULT,
        .w_z_surface_v = SETTING_INAV_W_Z_SURFACE_V_DEFAULT,

        .w_z_gps_p = SETTING_INAV_W_Z_GPS_P_DEFAULT,
        .w_z_gps_v = SETTING_INAV_W_Z_GPS_V_DEFAULT,

        .w_xy_gps_p = SETTING_INAV_W_XY_GPS_P_DEFAULT,
        .w_xy_gps_v = SETTING_INAV_W_XY_GPS_V_DEFAULT,

        .w_xy_flow_p = SETTING_INAV_W_XY_FLOW_P_DEFAULT,
        .w_xy_flow_v = SETTING_INAV_W_XY_FLOW_V_DEFAULT,

        .w_z_res_v = SETTING_INAV_W_Z_RES_V_DEFAULT,
        .w_xy_res_v = SETTING_INAV_W_XY_RES_V_DEFAULT,

        .w_acc_bias = SETTING_INAV_W_ACC_BIAS_DEFAULT,

        .max_eph_epv = SETTING_INAV_MAX_EPH_EPV_DEFAULT,
        .baro_epv = SETTING_INAV_BARO_EPV_DEFAULT,

        .default_alt_sensor = SETTING_INAV_DEFAULT_ALT_SENSOR_DEFAULT,
#ifdef USE_GPS_FIX_ESTIMATION
        .allow_gps_fix_estimation = SETTING_INAV_ALLOW_GPS_FIX_ESTIMATION_DEFAULT
#endif
);

#define resetTimer(tim, currentTimeUs) { (tim)->deltaTime = 0; (tim)->lastTriggeredTime = currentTimeUs; }
#define getTimerDeltaMicros(tim) ((tim)->deltaTime)
static bool updateTimer(navigationTimer_t * tim, timeUs_t interval, timeUs_t currentTimeUs)
{
    if ((currentTimeUs - tim->lastTriggeredTime) >= interval) {
        tim->deltaTime = currentTimeUs - tim->lastTriggeredTime;
        tim->lastTriggeredTime = currentTimeUs;
        return true;
    }
    else {
        return false;
    }
}

static bool shouldResetReferenceAltitude(void)
{
    /* Reference altitudes reset constantly when disarmed.
     * On arming ref altitudes saved as backup in case of emerg in flight rearm
     * If emerg in flight rearm active ref altitudes reset to backup values to avoid unwanted altitude reset */

    static float backupInitialBaroAltitudeOffset = 0.0f;
    static int32_t backupGpsOriginAltitude = 0;
    static bool emergRearmResetCheck = false;

    if (ARMING_FLAG(ARMED) && emergRearmResetCheck) {
        if (STATE(IN_FLIGHT_EMERG_REARM)) {
            initialBaroAltitudeOffset = backupInitialBaroAltitudeOffset;
            posControl.gpsOrigin.alt = backupGpsOriginAltitude;
        } else {
            backupInitialBaroAltitudeOffset = initialBaroAltitudeOffset;
            backupGpsOriginAltitude = posControl.gpsOrigin.alt;
        }
    }
    emergRearmResetCheck = !ARMING_FLAG(ARMED);

    switch ((nav_reset_type_e)positionEstimationConfig()->reset_altitude_type) {
        case NAV_RESET_NEVER:
            return false;
        case NAV_RESET_ON_FIRST_ARM:
            return !ARMING_FLAG(ARMED) && !ARMING_FLAG(WAS_EVER_ARMED);
        case NAV_RESET_ON_EACH_ARM:
            return !ARMING_FLAG(ARMED);
    }

    return false;
}

#if defined(USE_GPS)
/* Why is this here: Because GPS will be sending at quiet a nailed rate (if not overloaded by junk tasks at the brink of its specs)
 * but we might read out with timejitter because Irq might be off by a few us so we do a +-10% margin around the time between GPS
 * datasets representing the most common Hz-rates today. You might want to extend the list or find a smarter way.
 * Don't overload your GPS in its config with trash, choose a Hz rate that it can deliver at a sustained rate.
 * (c) CrashPilot1000
 */
static timeUs_t getGPSDeltaTimeFilter(timeUs_t dTus)
{
    if (dTus >= 225000 && dTus <= 275000) return HZ2US(4);       //  4Hz Data 250ms
    if (dTus >= 180000 && dTus <= 220000) return HZ2US(5);       //  5Hz Data 200ms
    if (dTus >=  90000 && dTus <= 110000) return HZ2US(10);      // 10Hz Data 100ms
    if (dTus >=  45000 && dTus <=  55000) return HZ2US(20);      // 20Hz Data  50ms
    if (dTus >=  30000 && dTus <=  36000) return HZ2US(30);      // 30Hz Data  33ms
    if (dTus >=  23000 && dTus <=  27000) return HZ2US(40);      // 40Hz Data  25ms
    if (dTus >=  18000 && dTus <=  22000) return HZ2US(50);      // 50Hz Data  20ms
    return dTus;                                                 // Filter failed. Set GPS Hz by measurement
}

/**
 * Update GPS topic
 *  Function is called on each GPS update
 */
void onNewGPSData(void)
{
    static timeUs_t lastGPSNewDataTime;
    static int32_t previousLat;
    static int32_t previousLon;
    static int32_t previousAlt;
    static bool isFirstGPSUpdate = true;

    gpsLocation_t newLLH;
    const timeUs_t currentTimeUs = micros();

    newLLH.lat = gpsSol.llh.lat;
    newLLH.lon = gpsSol.llh.lon;
    newLLH.alt = gpsSol.llh.alt;

    if (sensors(SENSOR_GPS)
#ifdef USE_GPS_FIX_ESTIMATION
            || STATE(GPS_ESTIMATED_FIX)
#endif
        ) {
        if (!(STATE(GPS_FIX)
#ifdef USE_GPS_FIX_ESTIMATION
                || STATE(GPS_ESTIMATED_FIX)
#endif
            )) {
            isFirstGPSUpdate = true;
            return;
        }

        if ((currentTimeUs - lastGPSNewDataTime) > MS2US(INAV_GPS_TIMEOUT_MS)) {
            isFirstGPSUpdate = true;
        }

        /* Automatic magnetic declination calculation - do this once */
        if(STATE(GPS_FIX_HOME)){
            static bool magDeclinationSet = false;
            if (positionEstimationConfig()->automatic_mag_declination && !magDeclinationSet) {
                const float declination = geoCalculateMagDeclination(&newLLH);
                imuSetMagneticDeclination(declination);
                magDeclinationSet = true;
            }
        }
        /* Process position update if GPS origin is already set, or precision is good enough */
        // FIXME: Add HDOP check for acquisition of GPS origin
        /* Set GPS origin or reset the origin altitude - keep initial pre-arming altitude at zero */
        if (!posControl.gpsOrigin.valid) {
            geoSetOrigin(&posControl.gpsOrigin, &newLLH, GEO_ORIGIN_SET);
        }
        else if (shouldResetReferenceAltitude()) {
            /* If we were never armed - keep altitude at zero */
            geoSetOrigin(&posControl.gpsOrigin, &newLLH, GEO_ORIGIN_RESET_ALTITUDE);
        }

        if (posControl.gpsOrigin.valid) {
            /* Convert LLH position to local coordinates */
            geoConvertGeodeticToLocal(&posEstimator.gps.pos, &posControl.gpsOrigin, &newLLH, GEO_ALT_ABSOLUTE);

            /* If not the first update - calculate velocities */
            if (!isFirstGPSUpdate) {
                float dT = US2S(getGPSDeltaTimeFilter(currentTimeUs - lastGPSNewDataTime));

                /* Use VELNED provided by GPS if available, calculate from coordinates otherwise */
                float gpsScaleLonDown = constrainf(cos_approx((ABS(gpsSol.llh.lat) / 10000000.0f) * 0.0174532925f), 0.01f, 1.0f);
                if (!ARMING_FLAG(SIMULATOR_MODE_SITL) && gpsSol.flags.validVelNE) {
                    posEstimator.gps.vel.x = gpsSol.velNED[X];
                    posEstimator.gps.vel.y = gpsSol.velNED[Y];
                }
                else {
                    posEstimator.gps.vel.x = (posEstimator.gps.vel.x + (DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR * (gpsSol.llh.lat - previousLat) / dT)) / 2.0f;
                    posEstimator.gps.vel.y = (posEstimator.gps.vel.y + (gpsScaleLonDown * DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR * (gpsSol.llh.lon - previousLon) / dT)) / 2.0f;
                }

                if (gpsSol.flags.validVelD) {
                    posEstimator.gps.vel.z = -gpsSol.velNED[Z];   // NEU
                }
                else {
                    posEstimator.gps.vel.z = (posEstimator.gps.vel.z + (gpsSol.llh.alt - previousAlt) / dT) / 2.0f;
                }

                /* FIXME: use HDOP/VDOP */
                if (gpsSol.flags.validEPE) {
                    posEstimator.gps.eph = gpsSol.eph;
                    posEstimator.gps.epv = gpsSol.epv;
                }
                else {
                    posEstimator.gps.eph = INAV_GPS_DEFAULT_EPH;
                    posEstimator.gps.epv = INAV_GPS_DEFAULT_EPV;
                }

                /* Indicate a last valid reading of Pos/Vel */
                posEstimator.gps.lastUpdateTime = currentTimeUs;
            }

            previousLat = gpsSol.llh.lat;
            previousLon = gpsSol.llh.lon;
            previousAlt = gpsSol.llh.alt;
            isFirstGPSUpdate = false;

            lastGPSNewDataTime = currentTimeUs;
        }
    }
    else {
        posEstimator.gps.lastUpdateTime = 0;
    }
}
#endif

#if defined(USE_BARO)
/**
 * Read BARO and update alt/vel topic
 *  Function is called from TASK_BARO
 */
void updatePositionEstimator_BaroTopic(timeUs_t currentTimeUs)
{
    float newBaroAlt = baroCalculateAltitude();

    if (sensors(SENSOR_BARO) && baroIsCalibrationComplete()) {
        /* If required - keep altitude at zero */
        if (shouldResetReferenceAltitude()) {
            initialBaroAltitudeOffset = newBaroAlt;
        }

        const timeUs_t baroDtUs = currentTimeUs - posEstimator.baro.lastUpdateTime;

        posEstimator.baro.alt = newBaroAlt - initialBaroAltitudeOffset;
        posEstimator.baro.epv = positionEstimationConfig()->baro_epv;
        posEstimator.baro.lastUpdateTime = currentTimeUs;

        if (baroDtUs <= MS2US(INAV_BARO_TIMEOUT_MS)) {
            posEstimator.baro.alt = pt1FilterApply3(&posEstimator.baro.avgFilter, posEstimator.baro.alt, US2S(baroDtUs));

            // baro altitude rate
            static float baroAltPrevious = 0;
            posEstimator.baro.baroAltRate = (posEstimator.baro.alt - baroAltPrevious) / US2S(baroDtUs);
            baroAltPrevious = posEstimator.baro.alt;
            updateBaroAltitudeRate(posEstimator.baro.baroAltRate);
        }
    }
    else {
        posEstimator.baro.alt = 0;
        posEstimator.baro.lastUpdateTime = 0;
    }
}
#endif

#if defined(USE_PITOT)
/**
 * Read Pitot and update airspeed topic
 *  Function is called at main loop rate, updates happen at reduced rate
 */
void updatePositionEstimator_PitotTopic(timeUs_t currentTimeUs)
{
    posEstimator.pitot.airspeed = getAirspeedEstimate();
    posEstimator.pitot.lastUpdateTime = currentTimeUs;
}
#endif

/**
 * Update IMU topic
 *  Function is called at main loop rate
 */
static void restartGravityCalibration(void)
{
    if (!gyroConfig()->init_gyro_cal_enabled) {
        return;
    }

    zeroCalibrationStartS(&posEstimator.imu.gravityCalibration, CALIBRATING_GRAVITY_TIME_MS, positionEstimationConfig()->gravity_calibration_tolerance, false);
}

static bool gravityCalibrationComplete(void)
{
    if (!gyroConfig()->init_gyro_cal_enabled) {
        return true;
    }

    return zeroCalibrationIsCompleteS(&posEstimator.imu.gravityCalibration);
}

#define ACC_VIB_FACTOR_S 1.0f
#define ACC_VIB_FACTOR_E 3.0f
static void updateIMUEstimationWeight(const float dt)
{
    static float acc_clip_factor = 1.0f;
    // If accelerometer measurement is clipped - drop the acc weight to 0.3
    // and gradually restore weight back to 1.0 over time
    if (accIsClipped()) {
        acc_clip_factor = 0.5f;
    }
    else {
        const float relAlpha = dt / (dt + INAV_ACC_CLIPPING_RC_CONSTANT);
        acc_clip_factor = acc_clip_factor * (1.0f - relAlpha) + 1.0f * relAlpha;
    }
    // Update accelerometer weight based on vibration levels and clipping
    float acc_vibration_factor = scaleRangef(constrainf(accGetVibrationLevel(),ACC_VIB_FACTOR_S,ACC_VIB_FACTOR_E),ACC_VIB_FACTOR_S,ACC_VIB_FACTOR_E,1.0f,0.3f); // g/s
    posEstimator.imu.accWeightFactor = acc_vibration_factor * acc_clip_factor;
    // DEBUG_VIBE[0-3] are used in IMU
    DEBUG_SET(DEBUG_VIBE, 4, posEstimator.imu.accWeightFactor * 1000);
}

static void updateIMUTopic(timeUs_t currentTimeUs)
{
    const float dt = US2S(currentTimeUs - posEstimator.imu.lastUpdateTime);
    posEstimator.imu.lastUpdateTime = currentTimeUs;

    if (!isImuReady()) {
        posEstimator.imu.accelNEU.x = 0.0f;
        posEstimator.imu.accelNEU.y = 0.0f;
        posEstimator.imu.accelNEU.z = 0.0f;

        restartGravityCalibration();
    }
    else {
        /* Update acceleration weight based on vibration levels and clipping */
        updateIMUEstimationWeight(dt);

        fpVector3_t accelReading;

        /* Read acceleration data in body frame */
        accelReading.x = imuMeasuredAccelBF.x;
        accelReading.y = imuMeasuredAccelBF.y;
        accelReading.z = imuMeasuredAccelBF.z;

        /* Adjust reading from Body to Earth frame - from Forward-Right-Down to North-East-Up*/
        imuTransformVectorBodyToEarth(&accelReading);

        /* Apply reading to NEU frame including correction for accelerometer bias */
        posEstimator.imu.accelNEU.x = accelReading.x + posEstimator.imu.accelBias.x;
        posEstimator.imu.accelNEU.y = accelReading.y + posEstimator.imu.accelBias.y;
        posEstimator.imu.accelNEU.z = accelReading.z + posEstimator.imu.accelBias.z;

        DEBUG_SET(DEBUG_VIBE, 5, posEstimator.imu.accelBias.x);
        DEBUG_SET(DEBUG_VIBE, 6, posEstimator.imu.accelBias.y);
        DEBUG_SET(DEBUG_VIBE, 7, posEstimator.imu.accelBias.z);

        /* When unarmed, assume that accelerometer should measure 1G. Use that to correct accelerometer gain */
        if (gyroConfig()->init_gyro_cal_enabled) {
            if (!ARMING_FLAG(ARMED) && !gravityCalibrationComplete()) {
                zeroCalibrationAddValueS(&posEstimator.imu.gravityCalibration, posEstimator.imu.accelNEU.z);

                if (gravityCalibrationComplete()) {
                    zeroCalibrationGetZeroS(&posEstimator.imu.gravityCalibration, &posEstimator.imu.calibratedGravityCMSS);
                    setGravityCalibration(posEstimator.imu.calibratedGravityCMSS);
                    LOG_DEBUG(POS_ESTIMATOR, "Gravity calibration complete (%d)", (int)lrintf(posEstimator.imu.calibratedGravityCMSS));
                }
            }
        } else {
            posEstimator.imu.gravityCalibration.params.state = ZERO_CALIBRATION_DONE;
            posEstimator.imu.calibratedGravityCMSS = gyroConfig()->gravity_cmss_cal;
        }

        if (gravityCalibrationComplete()) {
#ifdef USE_SIMULATOR
            if (ARMING_FLAG(SIMULATOR_MODE_HITL) || ARMING_FLAG(SIMULATOR_MODE_SITL)) {
                posEstimator.imu.calibratedGravityCMSS = GRAVITY_CMSS;
            }
#endif
            posEstimator.imu.accelNEU.z -= posEstimator.imu.calibratedGravityCMSS;
            posEstimator.imu.accelNEU.z += applySensorTempCompensation(10 * gyroGetTemperature(), imuMeasuredAccelBF.z, SENSOR_INDEX_ACC);
        }
        else {      // If calibration is incomplete - report zero acceleration
            posEstimator.imu.accelNEU.x = 0.0f;
            posEstimator.imu.accelNEU.y = 0.0f;
            posEstimator.imu.accelNEU.z = 0.0f;
        }

        /* Update blackbox values */
        navAccNEU[X] = posEstimator.imu.accelNEU.x;
        navAccNEU[Y] = posEstimator.imu.accelNEU.y;
        navAccNEU[Z] = posEstimator.imu.accelNEU.z;
    }
}

float updateEPE(const float oldEPE, const float dt, const float newEPE, const float w)
{
    return oldEPE + (newEPE - oldEPE) * w * dt;
}

static bool navIsAccelerationUsable(void)
{
    return true;
}

static bool navIsHeadingUsable(void)
{
    if (sensors(SENSOR_GPS)
#ifdef USE_GPS_FIX_ESTIMATION
        || STATE(GPS_ESTIMATED_FIX)
#endif
        ) {
        // If we have GPS - we need true IMU north (valid heading)
        return isImuHeadingValid();
    }
    else {
        // If we don't have GPS - we may use whatever we have, other sensors are operating in body frame
        return isImuHeadingValid() || positionEstimationConfig()->allow_dead_reckoning;
    }
}

static uint32_t calculateCurrentValidityFlags(timeUs_t currentTimeUs)
{
    /* Figure out if we have valid position data from our data sources */
    uint32_t newFlags = 0;

    const float max_eph_epv = positionEstimationConfig()->max_eph_epv;

    if ((sensors(SENSOR_GPS)
#ifdef USE_GPS_FIX_ESTIMATION
            || STATE(GPS_ESTIMATED_FIX)
#endif
        ) && posControl.gpsOrigin.valid &&
        ((currentTimeUs - posEstimator.gps.lastUpdateTime) <= MS2US(INAV_GPS_TIMEOUT_MS)) &&
        (posEstimator.gps.eph < max_eph_epv)) {
        if (posEstimator.gps.epv < max_eph_epv) {
            newFlags |= EST_GPS_XY_VALID | EST_GPS_Z_VALID;
        }
        else {
            newFlags |= EST_GPS_XY_VALID;
        }
    }

    if (sensors(SENSOR_BARO) && ((currentTimeUs - posEstimator.baro.lastUpdateTime) <= MS2US(INAV_BARO_TIMEOUT_MS))) {
        newFlags |= EST_BARO_VALID;
    }

    if (sensors(SENSOR_RANGEFINDER) && ((currentTimeUs - posEstimator.surface.lastUpdateTime) <= MS2US(INAV_SURFACE_TIMEOUT_MS))) {
        newFlags |= EST_SURFACE_VALID;
    }

    if (sensors(SENSOR_OPFLOW) && posEstimator.flow.isValid && ((currentTimeUs - posEstimator.flow.lastUpdateTime) <= MS2US(INAV_FLOW_TIMEOUT_MS))) {
        newFlags |= EST_FLOW_VALID;
    }

    if (posEstimator.est.eph < max_eph_epv) {
        newFlags |= EST_XY_VALID;
    }

    if (posEstimator.est.epv < max_eph_epv) {
        newFlags |= EST_Z_VALID;
    }

    return newFlags;
}

static void estimationPredict(estimationContext_t * ctx)
{

    /* Prediction step: Z-axis */
    if ((ctx->newFlags & EST_Z_VALID)) {
        posEstimator.est.pos.z += posEstimator.est.vel.z * ctx->dt;
        posEstimator.est.pos.z += posEstimator.imu.accelNEU.z * sq(ctx->dt) / 2.0f;
        if (ARMING_FLAG(WAS_EVER_ARMED)) {   // Hold at zero until first armed
            posEstimator.est.vel.z += posEstimator.imu.accelNEU.z * ctx->dt;
        }
    }

    /* Prediction step: XY-axis */
    if ((ctx->newFlags & EST_XY_VALID)) {
        // Predict based on known velocity
        posEstimator.est.pos.x += posEstimator.est.vel.x * ctx->dt;
        posEstimator.est.pos.y += posEstimator.est.vel.y * ctx->dt;

        // If heading is valid, accelNEU is valid as well. Account for acceleration
        if (navIsHeadingUsable() && navIsAccelerationUsable()) {
            posEstimator.est.pos.x += posEstimator.imu.accelNEU.x * sq(ctx->dt) / 2.0f;
            posEstimator.est.pos.y += posEstimator.imu.accelNEU.y * sq(ctx->dt) / 2.0f;
            posEstimator.est.vel.x += posEstimator.imu.accelNEU.x * ctx->dt;
            posEstimator.est.vel.y += posEstimator.imu.accelNEU.y * ctx->dt;
        }
    }
}

static bool estimationCalculateCorrection_Z(estimationContext_t * ctx)
{
    DEBUG_SET(DEBUG_ALTITUDE, 0, posEstimator.est.pos.z);       // Position estimate
    DEBUG_SET(DEBUG_ALTITUDE, 2, posEstimator.baro.alt);        // Baro altitude
    DEBUG_SET(DEBUG_ALTITUDE, 4, posEstimator.gps.pos.z);       // GPS altitude
    DEBUG_SET(DEBUG_ALTITUDE, 6, accGetVibrationLevel());       // Vibration level
    DEBUG_SET(DEBUG_ALTITUDE, 1, posEstimator.est.vel.z);       // Vertical speed estimate
    DEBUG_SET(DEBUG_ALTITUDE, 3, posEstimator.imu.accelNEU.z);  // Vertical acceleration on earth frame
    DEBUG_SET(DEBUG_ALTITUDE, 5, posEstimator.gps.vel.z);       // GPS vertical speed
    DEBUG_SET(DEBUG_ALTITUDE, 7, accGetClipCount());            // Clip count

    bool correctOK = false;
    const uint8_t defaultAltitudeSource = positionEstimationConfig()->default_alt_sensor;
    float wGps = defaultAltitudeSource == ALTITUDE_SOURCE_BARO_ONLY && ctx->newFlags & EST_BARO_VALID ? 0.0f : 1.0f;
    float wBaro = defaultAltitudeSource == ALTITUDE_SOURCE_GPS_ONLY && ctx->newFlags & EST_GPS_Z_VALID ? 0.0f : 1.0f;

    if (wBaro && ctx->newFlags & EST_BARO_VALID && wGps && ctx->newFlags & EST_GPS_Z_VALID) {
        const float gpsBaroResidual = fabsf(posEstimator.gps.pos.z - posEstimator.baro.alt);

        // Fade out the non default sensor to prevent sudden jump
        uint16_t residualErrorEpvLimit = defaultAltitudeSource == ALTITUDE_SOURCE_BARO ? 2 * positionEstimationConfig()->baro_epv : positionEstimationConfig()->max_eph_epv;
        const float start_epv = residualErrorEpvLimit;
        const float end_epv = residualErrorEpvLimit * 2.0f;

        // Calculate residual gps/baro sensor weighting based on assumed default altitude source = GPS
        wBaro = scaleRangef(constrainf(gpsBaroResidual, start_epv, end_epv), start_epv, end_epv, 1.0f, 0.0f);

        if (defaultAltitudeSource == ALTITUDE_SOURCE_BARO) {    // flip residual sensor weighting if default = BARO
            wGps = wBaro;
            wBaro = 1.0f;
        }
    }

    if (ctx->newFlags & EST_BARO_VALID && wBaro) {
        bool isAirCushionEffectDetected = false;
        static float baroGroundAlt = 0.0f;

        if (STATE(MULTIROTOR)) {
            static bool isBaroGroundValid = false;

            if (!ARMING_FLAG(ARMED)) {
                baroGroundAlt = posEstimator.baro.alt;
                isBaroGroundValid = true;
            }
            else if (isBaroGroundValid) {
                // We might be experiencing air cushion effect during takeoff - use sonar or baro ground altitude to detect it
                if (isMulticopterThrottleAboveMidHover()) {
                    // Disable ground effect detection at lift off when est alt and baro alt converge. Always disable if baro alt > 1m.
                    isBaroGroundValid = fabsf(posEstimator.est.pos.z - posEstimator.baro.alt) > 20.0f && posEstimator.baro.alt < 100.0f;
                }

                isAirCushionEffectDetected = (isEstimatedAglTrusted() && posEstimator.surface.alt < 20.0f) || posEstimator.baro.alt < baroGroundAlt + 20.0f;
            }
        }

        // Altitude
        float baroAltResidual = wBaro * ((isAirCushionEffectDetected ? baroGroundAlt : posEstimator.baro.alt) - posEstimator.est.pos.z);

        // Disable alt pos correction at point of lift off if ground effect active
        if (isAirCushionEffectDetected && isMulticopterThrottleAboveMidHover()) {
            baroAltResidual = 0.0f;
        }

        const float baroVelZResidual = isAirCushionEffectDetected ? 0.0f : wBaro * (posEstimator.baro.baroAltRate - posEstimator.est.vel.z);
        const float w_z_baro_p = positionEstimationConfig()->w_z_baro_p;
        const float w_z_baro_v = positionEstimationConfig()->w_z_baro_v;

        ctx->estPosCorr.z += baroAltResidual * w_z_baro_p * ctx->dt;
        ctx->estVelCorr.z += baroVelZResidual * w_z_baro_v * ctx->dt;

        ctx->newEPV = updateEPE(posEstimator.est.epv, ctx->dt, MAX(posEstimator.baro.epv, fabsf(baroAltResidual)), w_z_baro_p);

        // Accelerometer bias
        if (!isAirCushionEffectDetected) {
            ctx->accBiasCorr.z += (baroAltResidual * sq(w_z_baro_p) + baroVelZResidual * sq(w_z_baro_v));
        }

        correctOK = ARMING_FLAG(WAS_EVER_ARMED);    // No correction until first armed
    }

    if (ctx->newFlags & EST_GPS_Z_VALID && (wGps || !(ctx->newFlags & EST_Z_VALID))) {
        // Reset current estimate to GPS altitude if estimate not valid (used for GPS and Baro)
        if (!(ctx->newFlags & EST_Z_VALID)) {
            ctx->estPosCorr.z += posEstimator.gps.pos.z - posEstimator.est.pos.z;
            ctx->estVelCorr.z += posEstimator.gps.vel.z - posEstimator.est.vel.z;
            ctx->newEPV = posEstimator.gps.epv;
        }
        else {
            // Altitude
            const float gpsAltResidual = wGps * (posEstimator.gps.pos.z - posEstimator.est.pos.z);
            const float gpsVelZResidual = wGps * (posEstimator.gps.vel.z - posEstimator.est.vel.z);
            const float w_z_gps_p = positionEstimationConfig()->w_z_gps_p;
            const float w_z_gps_v = positionEstimationConfig()->w_z_gps_v;

            ctx->estPosCorr.z += gpsAltResidual * w_z_gps_p * ctx->dt;
            ctx->estVelCorr.z += gpsVelZResidual * w_z_gps_v * ctx->dt;
            ctx->newEPV = updateEPE(posEstimator.est.epv, ctx->dt, MAX(posEstimator.gps.epv, fabsf(gpsAltResidual)), w_z_gps_p);

            // Accelerometer bias
            ctx->accBiasCorr.z += (gpsAltResidual * sq(w_z_gps_p) + gpsVelZResidual * sq(w_z_gps_v));
        }

        correctOK = ARMING_FLAG(WAS_EVER_ARMED);    // No correction until first armed
    }

    // Factor corrections for sensor weightings to ensure magnitude consistency
    ctx->estPosCorr.z *= 2.0f / (wGps + wBaro);
    ctx->estVelCorr.z *= 2.0f / (wGps + wBaro);
    ctx->accBiasCorr.z *= 2.0f / (wGps + wBaro);

    return correctOK;
}

static bool estimationCalculateCorrection_XY_GPS(estimationContext_t * ctx)
{
    if (ctx->newFlags & EST_GPS_XY_VALID) {
        /* If GPS is valid and our estimate is NOT valid - reset it to GPS coordinates and velocity */
        if (!(ctx->newFlags & EST_XY_VALID)) {
            ctx->estPosCorr.x += posEstimator.gps.pos.x - posEstimator.est.pos.x;
            ctx->estPosCorr.y += posEstimator.gps.pos.y - posEstimator.est.pos.y;
            ctx->estVelCorr.x += posEstimator.gps.vel.x - posEstimator.est.vel.x;
            ctx->estVelCorr.y += posEstimator.gps.vel.y - posEstimator.est.vel.y;
            ctx->newEPH = posEstimator.gps.eph;
        }
        else {
            const float gpsPosXResidual = posEstimator.gps.pos.x - posEstimator.est.pos.x;
            const float gpsPosYResidual = posEstimator.gps.pos.y - posEstimator.est.pos.y;
            const float gpsVelXResidual = posEstimator.gps.vel.x - posEstimator.est.vel.x;
            const float gpsVelYResidual = posEstimator.gps.vel.y - posEstimator.est.vel.y;
            const float gpsPosResidualMag = calc_length_pythagorean_2D(gpsPosXResidual, gpsPosYResidual);

            //const float gpsWeightScaler = scaleRangef(bellCurve(gpsPosResidualMag, INAV_GPS_ACCEPTANCE_EPE), 0.0f, 1.0f, 0.1f, 1.0f);
            const float gpsWeightScaler = 1.0f;

            const float w_xy_gps_p = positionEstimationConfig()->w_xy_gps_p * gpsWeightScaler;
            const float w_xy_gps_v = positionEstimationConfig()->w_xy_gps_v * sq(gpsWeightScaler);

            // Coordinates
            ctx->estPosCorr.x += gpsPosXResidual * w_xy_gps_p * ctx->dt;
            ctx->estPosCorr.y += gpsPosYResidual * w_xy_gps_p * ctx->dt;

            // Velocity from direct measurement
            ctx->estVelCorr.x += gpsVelXResidual * w_xy_gps_v * ctx->dt;
            ctx->estVelCorr.y += gpsVelYResidual * w_xy_gps_v * ctx->dt;

            // Accelerometer bias
            ctx->accBiasCorr.x += (gpsPosXResidual * sq(w_xy_gps_p) + gpsVelXResidual * sq(w_xy_gps_v));
            ctx->accBiasCorr.y += (gpsPosYResidual * sq(w_xy_gps_p) + gpsVelYResidual * sq(w_xy_gps_v));

            /* Adjust EPH */
            ctx->newEPH = updateEPE(posEstimator.est.eph, ctx->dt, MAX(posEstimator.gps.eph, gpsPosResidualMag), w_xy_gps_p);
        }

        return true;
    }

    return false;
}

static void estimationCalculateGroundCourse(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);
    if ((STATE(GPS_FIX)
#ifdef USE_GPS_FIX_ESTIMATION
            || STATE(GPS_ESTIMATED_FIX)
#endif
    ) && navIsHeadingUsable()) {
        uint32_t groundCourse = wrap_36000(RADIANS_TO_CENTIDEGREES(atan2_approx(posEstimator.est.vel.y, posEstimator.est.vel.x)));
        posEstimator.est.cog = CENTIDEGREES_TO_DECIDEGREES(groundCourse);
    }
}

/**
 * Calculate next estimate using IMU and apply corrections from reference sensors (GPS, BARO etc)
 *  Function is called at main loop rate
 */
static void updateEstimatedTopic(timeUs_t currentTimeUs)
{
    estimationContext_t ctx;

    const float max_eph_epv = positionEstimationConfig()->max_eph_epv;

    /* Calculate dT */
    ctx.dt = US2S(currentTimeUs - posEstimator.est.lastUpdateTime);
    posEstimator.est.lastUpdateTime = currentTimeUs;

    /* If IMU is not ready we can't estimate anything */
    if (!isImuReady()) {
        posEstimator.est.eph = max_eph_epv + 0.001f;
        posEstimator.est.epv = max_eph_epv + 0.001f;
        posEstimator.flags = 0;
        return;
    }

    /* Calculate new EPH and EPV for the case we didn't update position */
    ctx.newEPH = posEstimator.est.eph * ((posEstimator.est.eph <= max_eph_epv) ? 1.0f + ctx.dt : 1.0f);
    ctx.newEPV = posEstimator.est.epv * ((posEstimator.est.epv <= max_eph_epv) ? 1.0f + ctx.dt : 1.0f);
    ctx.newFlags = calculateCurrentValidityFlags(currentTimeUs);
    vectorZero(&ctx.estPosCorr);
    vectorZero(&ctx.estVelCorr);
    vectorZero(&ctx.accBiasCorr);

    /* AGL estimation - separate process, decouples from Z coordinate */
    estimationCalculateAGL(&ctx);

    /* Prediction stage: X,Y,Z */
    estimationPredict(&ctx);

    /* Correction stage: Z */
    const bool estZCorrectOk =
        estimationCalculateCorrection_Z(&ctx);

    /* Correction stage: XY: GPS, FLOW */
    // FIXME: Handle transition from FLOW to GPS and back - seamlessly fly indoor/outdoor
    const bool estXYCorrectOk =
        estimationCalculateCorrection_XY_GPS(&ctx) ||
        estimationCalculateCorrection_XY_FLOW(&ctx);

    // If we can't apply correction or accuracy is off the charts - decay velocity to zero
    if (!estXYCorrectOk || ctx.newEPH > max_eph_epv) {
        ctx.estVelCorr.x = (0.0f - posEstimator.est.vel.x) * positionEstimationConfig()->w_xy_res_v * ctx.dt;
        ctx.estVelCorr.y = (0.0f - posEstimator.est.vel.y) * positionEstimationConfig()->w_xy_res_v * ctx.dt;
    }

    if (!estZCorrectOk || ctx.newEPV > max_eph_epv) {
        ctx.estVelCorr.z = (0.0f - posEstimator.est.vel.z) * positionEstimationConfig()->w_z_res_v * ctx.dt;
    }
    // Boost the corrections based on accWeight
    vectorScale(&ctx.estPosCorr, &ctx.estPosCorr, 1.0f / posEstimator.imu.accWeightFactor);
    vectorScale(&ctx.estVelCorr, &ctx.estVelCorr, 1.0f / posEstimator.imu.accWeightFactor);

    // Apply corrections
    vectorAdd(&posEstimator.est.pos, &posEstimator.est.pos, &ctx.estPosCorr);
    vectorAdd(&posEstimator.est.vel, &posEstimator.est.vel, &ctx.estVelCorr);

    /* Correct accelerometer bias */
    const float w_acc_bias = positionEstimationConfig()->w_acc_bias;
    if (w_acc_bias > 0.0f) {
        /* Correct accel bias */
        posEstimator.imu.accelBias.x += ctx.accBiasCorr.x * w_acc_bias * ctx.dt;
        posEstimator.imu.accelBias.y += ctx.accBiasCorr.y * w_acc_bias * ctx.dt;
        posEstimator.imu.accelBias.z += ctx.accBiasCorr.z * w_acc_bias * ctx.dt;

        posEstimator.imu.accelBias.x = constrainf(posEstimator.imu.accelBias.x, -INAV_ACC_BIAS_ACCEPTANCE_VALUE, INAV_ACC_BIAS_ACCEPTANCE_VALUE);
        posEstimator.imu.accelBias.y = constrainf(posEstimator.imu.accelBias.y, -INAV_ACC_BIAS_ACCEPTANCE_VALUE, INAV_ACC_BIAS_ACCEPTANCE_VALUE);
        posEstimator.imu.accelBias.z = constrainf(posEstimator.imu.accelBias.z, -INAV_ACC_BIAS_ACCEPTANCE_VALUE, INAV_ACC_BIAS_ACCEPTANCE_VALUE);
    }

    /* Update ground course */
    estimationCalculateGroundCourse(currentTimeUs);

    /* Update uncertainty */
    posEstimator.est.eph = ctx.newEPH;
    posEstimator.est.epv = ctx.newEPV;

    // Keep flags for further usage
    posEstimator.flags = ctx.newFlags;
}

/**
 * Examine estimation error and update navigation system if estimate is good enough
 *  Function is called at main loop rate, but updates happen less frequently - at a fixed rate
 */
static void publishEstimatedTopic(timeUs_t currentTimeUs)
{
    static navigationTimer_t posPublishTimer;

    /* Position and velocity are published with INAV_POSITION_PUBLISH_RATE_HZ */
    if (updateTimer(&posPublishTimer, HZ2US(INAV_POSITION_PUBLISH_RATE_HZ), currentTimeUs)) {
        /* Publish heading update */
        /* IMU operates in decidegrees while INAV operates in deg*100
        * Use course over ground when GPS heading valid */
        int16_t cogValue = isGPSHeadingValid() ? posEstimator.est.cog : attitude.values.yaw;
        updateActualHeading(navIsHeadingUsable(), DECIDEGREES_TO_CENTIDEGREES(attitude.values.yaw), DECIDEGREES_TO_CENTIDEGREES(cogValue));

        /* Publish position update */
        if (posEstimator.est.eph < positionEstimationConfig()->max_eph_epv) {
            // FIXME!!!!!
            updateActualHorizontalPositionAndVelocity(true, true, posEstimator.est.pos.x, posEstimator.est.pos.y, posEstimator.est.vel.x, posEstimator.est.vel.y);
        }
        else {
            updateActualHorizontalPositionAndVelocity(false, false, posEstimator.est.pos.x, posEstimator.est.pos.y, 0, 0);
        }

        /* Publish altitude update and set altitude validity */
        if (posEstimator.est.epv < positionEstimationConfig()->max_eph_epv) {
            const float gpsCfEstimatedAltitudeError = STATE(GPS_FIX) ? posEstimator.gps.pos.z - posEstimator.est.pos.z : 0;
            navigationEstimateStatus_e aglStatus = (posEstimator.est.aglQual == SURFACE_QUAL_LOW) ? EST_USABLE : EST_TRUSTED;
            updateActualAltitudeAndClimbRate(true, posEstimator.est.pos.z, posEstimator.est.vel.z, posEstimator.est.aglAlt, posEstimator.est.aglVel, aglStatus, gpsCfEstimatedAltitudeError);
        }
        else {
            updateActualAltitudeAndClimbRate(false, posEstimator.est.pos.z, 0, posEstimator.est.aglAlt, 0, EST_NONE, 0);
        }

        //Update Blackbox states
        navEPH = posEstimator.est.eph;
        navEPV = posEstimator.est.epv;

        DEBUG_SET(DEBUG_POS_EST, 0, (int32_t) posEstimator.est.pos.x*1000.0F);                // Position estimate X
        DEBUG_SET(DEBUG_POS_EST, 1, (int32_t) posEstimator.est.pos.y*1000.0F);                // Position estimate Y
        if (IS_RC_MODE_ACTIVE(BOXSURFACE) && posEstimator.est.aglQual!=SURFACE_QUAL_LOW){
            // SURFACE (following) MODE
            DEBUG_SET(DEBUG_POS_EST, 2, (int32_t) posControl.actualState.agl.pos.z*1000.0F);  // Position estimate Z
            DEBUG_SET(DEBUG_POS_EST, 5, (int32_t) posControl.actualState.agl.vel.z*1000.0F);  // Speed estimate VZ
        } else {
            DEBUG_SET(DEBUG_POS_EST, 2, (int32_t) posEstimator.est.pos.z*1000.0F);            // Position estimate Z
            DEBUG_SET(DEBUG_POS_EST, 5, (int32_t) posEstimator.est.vel.z*1000.0F);            // Speed estimate VZ
        }
        DEBUG_SET(DEBUG_POS_EST, 3, (int32_t) posEstimator.est.vel.x*1000.0F);                // Speed estimate VX
        DEBUG_SET(DEBUG_POS_EST, 4, (int32_t) posEstimator.est.vel.y*1000.0F);                // Speed estimate VY
        DEBUG_SET(DEBUG_POS_EST, 6, (int32_t) attitude.values.yaw);                           // Yaw estimate (4 bytes still available here)
        DEBUG_SET(DEBUG_POS_EST, 7, (int32_t) (posEstimator.flags & 0b1111111)<<20 |          // navPositionEstimationFlags fit into 8bits
                                              (MIN(navEPH, 1000) & 0x3FF)<<10 |
                                              (MIN(navEPV, 1000) & 0x3FF));                   // Horizontal and vertical uncertainties (max value = 1000, fit into 20bits)
    }
}

float getEstimatedAglPosition(void) {
    return posEstimator.est.aglAlt;
}

bool isEstimatedAglTrusted(void) {
    return (posEstimator.est.aglQual == SURFACE_QUAL_HIGH) ? true : false;
}

/**
 * Initialize position estimator
 *  Should be called once before any update occurs
 */
void initializePositionEstimator(void)
{
    int axis;

    posEstimator.est.eph = positionEstimationConfig()->max_eph_epv + 0.001f;
    posEstimator.est.epv = positionEstimationConfig()->max_eph_epv + 0.001f;

    posEstimator.imu.lastUpdateTime = 0;
    posEstimator.gps.lastUpdateTime = 0;
    posEstimator.baro.lastUpdateTime = 0;
    posEstimator.surface.lastUpdateTime = 0;

    posEstimator.est.aglAlt = 0;
    posEstimator.est.aglVel = 0;

    posEstimator.est.flowCoordinates[X] = 0;
    posEstimator.est.flowCoordinates[Y] = 0;

    posEstimator.imu.accWeightFactor = 0;

    restartGravityCalibration();

    for (axis = 0; axis < 3; axis++) {
        posEstimator.imu.accelBias.v[axis] = 0;
        posEstimator.est.pos.v[axis] = 0;
        posEstimator.est.vel.v[axis] = 0;
    }

    pt1FilterInit(&posEstimator.baro.avgFilter, INAV_BARO_AVERAGE_HZ, 0.0f);
    pt1FilterInit(&posEstimator.surface.avgFilter, INAV_SURFACE_AVERAGE_HZ, 0.0f);
}

/**
 * Update estimator
 *  Update rate: loop rate (>100Hz)
 */
void updatePositionEstimator(void)
{
    static bool isInitialized = false;

    if (!isInitialized) {
        initializePositionEstimator();
        isInitialized = true;
    }

    const timeUs_t currentTimeUs = micros();

    /* Read updates from IMU, preprocess */
    updateIMUTopic(currentTimeUs);

    /* Update estimate */
    updateEstimatedTopic(currentTimeUs);

    /* Publish estimate */
    publishEstimatedTopic(currentTimeUs);
}

bool navIsCalibrationComplete(void)
{
    return gravityCalibrationComplete();
}
