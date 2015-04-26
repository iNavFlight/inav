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

#include "build_config.h"
#include "platform.h"
#include "debug.h"

#include "common/axis.h"
#include "common/color.h"
#include "common/maths.h"

#include "drivers/sensor.h"
#include "drivers/system.h"
#include "drivers/gpio.h"
#include "drivers/timer.h"
#include "drivers/serial.h"
#include "drivers/accgyro.h"
#include "drivers/compass.h"
#include "drivers/pwm_rx.h"

#include "rx/rx.h"

#include "sensors/sensors.h"
#include "sensors/sonar.h"
#include "sensors/barometer.h"
#include "sensors/compass.h"
#include "sensors/acceleration.h"
#include "sensors/gyro.h"
#include "sensors/battery.h"
#include "sensors/boardalignment.h"

#include "io/serial.h"
#include "io/gps.h"
#include "io/gimbal.h"
#include "io/ledstrip.h"

#include "telemetry/telemetry.h"
#include "blackbox/blackbox.h"

#include "flight/pid.h"
#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/failsafe.h"
#include "flight/gps_conversion.h"
#include "flight/navigation_rewrite.h"
#include "flight/navigation_rewrite_private.h"

#include "config/runtime_config.h"
#include "config/config.h"
#include "config/config_profile.h"
#include "config/config_master.h"

/**
 * Model-identification based position estimator
 * Based on INAV position estimator for PX4 by Anton Babushkin <anton.babushkin@me.com> 
 * @author Konstantin Sharlaimov <konstantin.sharlaimov@gmail.com>
 */
#define INAV_ENABLE_DEAD_RECKONING

#define INAV_GPS_EPV        500.0f  // 5m GPS VDOP
#define INAV_GPS_EPH        200.0f  // 2m GPS HDOP  (gives about 1.6s of dead-reckoning if GPS is temporary lost)

#define INAV_POSITION_PUBLISH_RATE_HZ       50      // Publish position updates at this rate
#define INAV_BARO_UPDATE_RATE               20
#define INAV_SONAR_UPDATE_RATE              20

#define INAV_GPS_TIMEOUT_MS                 1500    // GPS timeout
#define INAV_BARO_TIMEOUT_MS                200     // Baro timeout
#define INAV_SONAR_TIMEOUT_MS               200     // Sonar timeout

#define INAV_HISTORY_BUF_SIZE               (INAV_POSITION_PUBLISH_RATE_HZ / 2)     // Enough to hold 0.5 sec historical data

typedef struct {
    uint32_t    lastUpdateTime; // Last update time (us)
    t_fp_vector pos;            // GPS position in NEU coordinate system (cm)
    t_fp_vector vel;            // GPS velocity (cms)
    float       eph;
    float       epv;
} navPositionEstimatorGPS_t;

typedef struct {
    uint32_t    lastUpdateTime; // Last update time (us)
    float       alt;            // Raw barometric altitude (cm)
    float       vel;            // Altitude derivative - velocity (cms)
    float       epv;
} navPositionEstimatorBARO_t;

typedef struct {
    uint32_t    lastUpdateTime; // Last update time (us)
    float       alt;            // Raw altitude measurement (cm)
    float       vel;            // Altitude derivative - velocity (cms)
    float       epv;
} navPositionEstimatorSONAR_t;

typedef struct {
    uint32_t    lastUpdateTime; // Last update time (us)
    t_fp_vector pos;
    t_fp_vector vel;
    float       eph;
    float       epv;
} navPositionEstimatorESTIMATE_t;

typedef struct {
    uint8_t     index;
    t_fp_vector pos[INAV_HISTORY_BUF_SIZE];
    t_fp_vector vel[INAV_HISTORY_BUF_SIZE];
} navPosisitonEstimatorHistory_t;

typedef struct {
    // Data sources
    navPositionEstimatorGPS_t   gps;
    navPositionEstimatorBARO_t  baro;
    navPositionEstimatorSONAR_t sonar;

    // Estimate
    navPositionEstimatorESTIMATE_t  est;

    // Estimation history
    navPosisitonEstimatorHistory_t  history;
} navigationPosEstimator_s;

static navigationPosEstimator_s posEstimator;

/* Inertial filter, taken from PX4 implementation by Anton Babushkin <rk3dov@gmail.com> */
static void inavFilterPredict(float * pos, float * vel, float dt, float acc)
{
    *pos += *vel * dt + acc * dt * dt / 2.0f;
    *vel += acc * dt;
}

static void inavFilterCorrectPos(float * pos, float * vel, float dt, float e, float w)
{
    float ewdt = e * w * dt;
    *pos += ewdt;
    *vel += w * ewdt;
}

static void inavFilterCorrectVel(float * vel, float dt, float e, float w)
{
    *vel += e * w * dt;
}

#if defined(GPS)
/* Why is this here: Because GPS will be sending at quiet a nailed rate (if not overloaded by junk tasks at the brink of its specs)
 * but we might read out with timejitter because Irq might be off by a few us so we do a +-10% margin around the time between GPS
 * datasets representing the most common Hz-rates today. You might want to extend the list or find a smarter way.
 * Don't overload your GPS in its config with trash, choose a Hz rate that it can deliver at a sustained rate.
 * (c) CrashPilot1000 
 */
static uint32_t getGPSDeltaTimeFilter(uint32_t dTus)
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
void onNewGPSData(int32_t newLat, int32_t newLon, int32_t newAlt)
{
    static uint32_t lastGPSNewDataTime;
    static int32_t previousLat;
    static int32_t previousLon;
    static int32_t previousAlt;
    static bool isFirstGPSUpdate = true;

    gpsLocation_t newLLH;
    uint32_t currentTime = micros();

    if (sensors(SENSOR_GPS)) {
        if (!(STATE(GPS_FIX) && GPS_numSat >= 5)) {
            isFirstGPSUpdate = true;
            return;
        }

        if ((currentTime - lastGPSNewDataTime) > MS2US(INAV_GPS_TIMEOUT_MS)) {
            isFirstGPSUpdate = true;
        }

        /* Process position update if GPS origin is already set, or precision is good enough */
        // FIXME: use HDOP here
        if ((posControl.gpsOrigin.valid) || (GPS_numSat >= 6)) {
            /* Convert LLH position to local coordinates */
            newLLH.lat = newLat;
            newLLH.lon = newLon;
            newLLH.alt = newAlt;
            gpsConvertGeodeticToLocal(&posControl.gpsOrigin, &newLLH, &posEstimator.gps.pos);

            /* If not the first update - calculate velocities */
            if (!isFirstGPSUpdate) {
                float dT = US2S(getGPSDeltaTimeFilter(currentTime - lastGPSNewDataTime));
                float gpsScaleLonDown = constrainf(cos_approx((ABS(newLat) / 10000000.0f) * 0.0174532925f), 0.01f, 1.0f);

                // TODO: use GPS velocities if available

                /* Calculate NEU velocities from _unfiltered_ coordinates - we apply averaging to velocity, this reduces noise */
                posEstimator.gps.vel.V.X = (posEstimator.gps.vel.V.X + (DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR * (newLat - previousLat) / dT)) / 2.0f;
                posEstimator.gps.vel.V.Y = (posEstimator.gps.vel.V.Y + (gpsScaleLonDown * DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR * (newLon - previousLon) / dT)) / 2.0f;
                posEstimator.gps.vel.V.Z = (posEstimator.gps.vel.V.Z + (newAlt - previousAlt) / dT) / 2.0f;

                /* FIXME: use HDOP/VDOP */
                posEstimator.gps.eph = INAV_GPS_EPH;
                posEstimator.gps.epv = INAV_GPS_EPV;

                /* Indicate a last valid reading of Pos/Vel */
                posEstimator.gps.lastUpdateTime = currentTime;
            }

            previousLat = newLat;
            previousLon = newLon;
            previousAlt = newAlt;
            isFirstGPSUpdate = false;

            lastGPSNewDataTime = currentTime;
        }
    }
    else {
        posEstimator.gps.lastUpdateTime = 0;
    }
}
#endif

#if defined(BARO)
/**
 * Read BARO and update alt/vel topic
 *  Function is called at main loop rate, updates happen at reduced rate
 */
static void updateBaroTopic(uint32_t currentTime)
{
    static filterWithBufferSample_t baroClimbRateFilterBuffer[NAV_BARO_CLIMB_RATE_FILTER_SIZE];
    static filterWithBufferState_t baroClimbRateFilter;
    static bool climbRateFiltersInitialized = false;
    static uint32_t previousTimeUpdate = 0;
    uint32_t deltaMicros = currentTime - previousTimeUpdate;

    if (deltaMicros < HZ2US(INAV_BARO_UPDATE_RATE))
        return;

    previousTimeUpdate = currentTime;

    if (!climbRateFiltersInitialized) {
        filterWithBufferInit(&baroClimbRateFilter, &baroClimbRateFilterBuffer[0], NAV_BARO_CLIMB_RATE_FILTER_SIZE);
        climbRateFiltersInitialized = true;
    }

    float newBaroAlt = baroCalculateAltitude();
    if (sensors(SENSOR_BARO) && isBaroCalibrationComplete()) {
        filterWithBufferUpdate(&baroClimbRateFilter, newBaroAlt, currentTime);
        float baroClimbRate = filterWithBufferApply_Derivative(&baroClimbRateFilter) * 1e6f;

        // FIXME: do we need this?
        baroClimbRate = constrainf(baroClimbRate, -1500, 1500);  // constrain baro velocity +/- 1500cm/s
        baroClimbRate = applyDeadband(baroClimbRate, 10);       // to reduce noise near zero

        posEstimator.baro.alt = newBaroAlt;
        posEstimator.baro.vel = baroClimbRate;
        posEstimator.baro.epv = posControl.navConfig->inav.baro_epv;
        posEstimator.baro.lastUpdateTime = currentTime;
    }
    else {
        posEstimator.baro.alt = 0;
        posEstimator.baro.vel = 0;
        posEstimator.baro.lastUpdateTime = 0;
    }
}
#endif

#if defined(SONAR)
/**
 * Read sonar and update alt/vel topic
 *  Function is called at main loop rate, updates happen at reduced rate
 */
static void updateSonarTopic(uint32_t currentTime)
{
    static float previousSonarAlt = -1;
    static uint32_t previousTimeUpdate = 0;
    uint32_t deltaMicros = currentTime - previousTimeUpdate;

    if (deltaMicros < HZ2US(INAV_SONAR_UPDATE_RATE))
        return;

    previousTimeUpdate = currentTime;

    if (sensors(SENSOR_SONAR)) {
        /* Read sonar */
        float newSonarAlt = sonarRead();

        newSonarAlt = sonarCalculateAltitude(newSonarAlt, calculateCosTiltAngle());

        /* If we have two valid sonar readings in a row - update topic */
        if (newSonarAlt > 0 && previousSonarAlt > 0) {
            posEstimator.sonar.alt = (newSonarAlt + previousSonarAlt) * 0.5f;
            posEstimator.sonar.vel = (newSonarAlt - previousSonarAlt) / US2S(deltaMicros);
            posEstimator.sonar.epv = posControl.navConfig->inav.sonar_epv;
            posEstimator.sonar.lastUpdateTime = currentTime;
        }

        /* Store current reading as previous */
        previousSonarAlt = newSonarAlt;
    }
    else {
        /* No sonar */
        posEstimator.sonar.alt = 0;
        posEstimator.sonar.vel = 0;
        posEstimator.sonar.lastUpdateTime = 0;

        previousSonarAlt = -1;
    }
}
#endif

/**
 * Calculate next estimate using IMU and apply corrections from reference sensors (GPS, BARO etc)
 *  Function is called at main loop rate
 */
static void updateEstimatedTopic(uint32_t currentTime)
{
    float dt = US2S(currentTime - posEstimator.est.lastUpdateTime);
    posEstimator.est.lastUpdateTime = currentTime;

    /* In absence of accelerometer we can't estimate anything */
    if (!sensors(SENSOR_ACC)) {
        posEstimator.est.eph = posControl.navConfig->inav.max_eph_epv + 0.001f;
        posEstimator.est.epv = posControl.navConfig->inav.max_eph_epv + 0.001f;
        return;
    }

    /* increase EPH/EPV on each iteration */
    if (posEstimator.est.eph < posControl.navConfig->inav.max_eph_epv) {
        posEstimator.est.eph *= 1.0f + dt;
    }

    if (posEstimator.est.epv < posControl.navConfig->inav.max_eph_epv) {
        posEstimator.est.epv *= 1.0f + dt;
    }

    /* Figure out if we have valid position data from our data sources */
    bool isGPSValid = sensors(SENSOR_MAG) && persistentFlag(FLAG_MAG_CALIBRATION_DONE) && posControl.gpsOrigin.valid && 
                        sensors(SENSOR_GPS) && ((currentTime - posEstimator.gps.lastUpdateTime) <= MS2US(INAV_GPS_TIMEOUT_MS));
    bool isBaroValid = sensors(SENSOR_BARO) && ((currentTime - posEstimator.baro.lastUpdateTime) <= MS2US(INAV_BARO_TIMEOUT_MS));
    bool isSonarValid = sensors(SENSOR_SONAR) && ((currentTime - posEstimator.sonar.lastUpdateTime) <= MS2US(INAV_SONAR_TIMEOUT_MS));
#if defined(INAV_ENABLE_DEAD_RECKONING)
    bool useDeadReckoning = posControl.navConfig->inav.enable_dead_reckoning && (!isGPSValid);
#endif

    /* Apply GPS altitude corrections only on fixed wing aircrafts */
    bool useGpsZ = STATE(FIXED_WING) && isGPSValid;

    /* Pre-calculate history index for GPS & baro delay compensation */
    int gpsHistoryIndex = (posEstimator.history.index - 1) - constrain(((int)posControl.navConfig->inav.gps_delay_ms / (1000 / INAV_POSITION_PUBLISH_RATE_HZ)), 0, INAV_HISTORY_BUF_SIZE - 1);
    if (gpsHistoryIndex < 0) {
        gpsHistoryIndex += INAV_HISTORY_BUF_SIZE;
    }

    /* Estimate Z-axis */
    if ((posEstimator.est.epv < posControl.navConfig->inav.max_eph_epv) || useGpsZ || isBaroValid || isSonarValid) {
        /* Predict position/velocity based on acceleration */
        inavFilterPredict(&posEstimator.est.pos.V.Z, &posEstimator.est.vel.V.Z, dt, imuAverageAcceleration.V.Z);

        /* Compute sonar/baro transition factor */
#if defined(BARO) && defined(SONAR)
        if (isSonarValid && isBaroValid) {
            float sonarTransitionFactor;
            /* Sonar and baro */
            if (posEstimator.sonar.alt <= (SONAR_MAX_RANGE * 2 / 3)) {
                /* If within 2/3 sonar range - use only sonar */
                sonarTransitionFactor = 1.0f;
            }
            else if (posEstimator.sonar.alt <= SONAR_MAX_RANGE) {
                /* Squeze difference between sonar and baro into upper 1/3 sonar range.
                 * FIXME: this will give us totally wrong altitude in the upper
                 * 1/3 sonar range but will allow graceful transition from SONAR to BARO */
                sonarTransitionFactor = constrainf((SONAR_MAX_RANGE - posEstimator.sonar.alt) / (SONAR_MAX_RANGE / 3.0f), 0, 1);
            }
            else {
                /* Sonar driver returned a value > SONAR_MAX_RANGE, ignore it, rely on baro altitude */
                sonarTransitionFactor = 0.0f;
            }

            /* Fuse velocity and altitude */
            float fusedAlt = posEstimator.sonar.alt * sonarTransitionFactor + posEstimator.baro.alt * (1.0f - sonarTransitionFactor);
            float fusedVel = posEstimator.sonar.vel * sonarTransitionFactor + posEstimator.baro.vel * (1.0f - sonarTransitionFactor);
            float fusedWeightP = posControl.navConfig->inav.w_z_sonar_p * sonarTransitionFactor + 
                                 posControl.navConfig->inav.w_z_baro_p * (1.0f - sonarTransitionFactor);
            float fusedWeightV = posControl.navConfig->inav.w_z_sonar_v * sonarTransitionFactor + 
                                 posControl.navConfig->inav.w_z_baro_v * (1.0f - sonarTransitionFactor);

            /* Apply fused velocity and altitude correction */
            inavFilterCorrectPos(&posEstimator.est.pos.V.Z, &posEstimator.est.vel.V.Z, dt, fusedAlt - posEstimator.est.pos.V.Z, fusedWeightP);
            inavFilterCorrectVel(&posEstimator.est.vel.V.Z, dt, fusedVel - posEstimator.est.vel.V.Z, fusedWeightV);

            /* Adjust EPV */
            float fusedEPV = posEstimator.sonar.epv * sonarTransitionFactor + posEstimator.baro.epv * (1.0f - sonarTransitionFactor);
            posEstimator.est.epv = MIN(posEstimator.est.epv, fusedEPV);
        }
        else if (isSonarValid) {
            /* Apply only sonar correction, no baro */
            inavFilterCorrectPos(&posEstimator.est.pos.V.Z, &posEstimator.est.vel.V.Z, dt, posEstimator.sonar.alt - posEstimator.est.pos.V.Z, posControl.navConfig->inav.w_z_sonar_p);
            inavFilterCorrectVel(&posEstimator.est.vel.V.Z, dt, posEstimator.sonar.vel - posEstimator.est.vel.V.Z, posControl.navConfig->inav.w_z_sonar_v);

            /* Adjust EPV */
            posEstimator.est.epv = MIN(posEstimator.est.epv, posEstimator.sonar.epv);
        }
        else if (isBaroValid) {
            /* Apply only baro correction, no sonar */
            inavFilterCorrectPos(&posEstimator.est.pos.V.Z, &posEstimator.est.vel.V.Z, dt, posEstimator.baro.alt - posEstimator.est.pos.V.Z, posControl.navConfig->inav.w_z_baro_p);
            inavFilterCorrectVel(&posEstimator.est.vel.V.Z, dt, posEstimator.baro.vel - posEstimator.est.vel.V.Z, posControl.navConfig->inav.w_z_baro_v);

            /* Adjust EPV */
            posEstimator.est.epv = MIN(posEstimator.est.epv, posEstimator.baro.epv);
        }
#elif defined(BARO)
        if (isBaroValid) {
            inavFilterCorrectPos(&posEstimator.est.pos.V.Z, &posEstimator.est.vel.V.Z, dt, posEstimator.baro.alt - posEstimator.est.pos.V.Z, posControl.navConfig->inav.w_z_baro_p);
            inavFilterCorrectVel(&posEstimator.est.vel.V.Z, dt, posEstimator.baro.vel - posEstimator.est.vel.V.Z, posControl.navConfig->inav.w_z_baro_v);

            /* Adjust EPV */
            posEstimator.est.epv = MIN(posEstimator.est.epv, posEstimator.baro.epv);
        }
#elif defined(SONAR)
        if (isSonarValid) {
            /* Apply only sonar correction, no baro */
            inavFilterCorrectPos(&posEstimator.est.pos.V.Z, &posEstimator.est.vel.V.Z, dt, posEstimator.sonar.alt - posEstimator.est.pos.V.Z, posControl.navConfig->inav.w_z_sonar_p);
            inavFilterCorrectVel(&posEstimator.est.vel.V.Z, dt, posEstimator.sonar.vel - posEstimator.est.vel.V.Z, posControl.navConfig->inav.w_z_sonar_v);

            /* Adjust EPV */
            posEstimator.est.epv = MIN(posEstimator.est.epv, posEstimator.sonar.epv);
        }
#endif

        /* Apply GPS correction to altitude */
        if (useGpsZ) {
            inavFilterCorrectPos(&posEstimator.est.pos.V.Z, &posEstimator.est.vel.V.Z, dt, posEstimator.gps.pos.V.Z - posEstimator.history.pos[gpsHistoryIndex].V.Z, posControl.navConfig->inav.w_z_gps_p);
            inavFilterCorrectVel(&posEstimator.est.vel.V.Z, dt, posEstimator.gps.vel.V.Z - posEstimator.history.vel[gpsHistoryIndex].V.Z, posControl.navConfig->inav.w_z_gps_v);

            /* Adjust EPV */
            posEstimator.est.epv = MIN(posEstimator.est.epv, posEstimator.gps.epv);
        }
    }
    else {
        inavFilterCorrectVel(&posEstimator.est.vel.V.Z, dt, 0.0f - posEstimator.est.vel.V.Z, posControl.navConfig->inav.w_z_res_v);
    }

    /* Estimate XY-axis */
    if ((posEstimator.est.eph < posControl.navConfig->inav.max_eph_epv) || isGPSValid || useDeadReckoning) {
        /* Predict position */
        inavFilterPredict(&posEstimator.est.pos.V.X, &posEstimator.est.vel.V.X, dt, imuAverageAcceleration.V.X);
        inavFilterPredict(&posEstimator.est.pos.V.Y, &posEstimator.est.vel.V.Y, dt, imuAverageAcceleration.V.Y);

        /* Correct position */
        if (isGPSValid) {
            inavFilterCorrectPos(&posEstimator.est.pos.V.X, &posEstimator.est.vel.V.X, dt, posEstimator.gps.pos.V.X - posEstimator.history.pos[gpsHistoryIndex].V.X, posControl.navConfig->inav.w_xy_gps_p);
            inavFilterCorrectPos(&posEstimator.est.pos.V.Y, &posEstimator.est.vel.V.Y, dt, posEstimator.gps.pos.V.Y - posEstimator.history.pos[gpsHistoryIndex].V.Y, posControl.navConfig->inav.w_xy_gps_p);

            inavFilterCorrectVel(&posEstimator.est.vel.V.X, dt, posEstimator.gps.vel.V.X - posEstimator.history.vel[gpsHistoryIndex].V.X, posControl.navConfig->inav.w_xy_gps_v);
            inavFilterCorrectVel(&posEstimator.est.vel.V.Y, dt, posEstimator.gps.vel.V.Y - posEstimator.history.vel[gpsHistoryIndex].V.Y, posControl.navConfig->inav.w_xy_gps_v);

            /* Adjust EPH */
            posEstimator.est.eph = MIN(posEstimator.est.eph, posEstimator.gps.eph);
        }

#if defined(INAV_ENABLE_DEAD_RECKONING)
        /* Use dead reckoning */
        if (useDeadReckoning) {
            inavFilterCorrectPos(&posEstimator.est.pos.V.X, &posEstimator.est.vel.V.X, dt, 0.0f - posEstimator.est.pos.V.X, posControl.navConfig->inav.w_xy_dr_p);
            inavFilterCorrectPos(&posEstimator.est.pos.V.Y, &posEstimator.est.vel.V.Y, dt, 0.0f - posEstimator.est.pos.V.Y, posControl.navConfig->inav.w_xy_dr_p);

            inavFilterCorrectVel(&posEstimator.est.vel.V.X, dt, 0.0f - posEstimator.est.vel.V.X, posControl.navConfig->inav.w_xy_dr_v);
            inavFilterCorrectVel(&posEstimator.est.vel.V.Y, dt, 0.0f - posEstimator.est.vel.V.Y, posControl.navConfig->inav.w_xy_dr_v);

            /* Adjust EPH to just below max_epe */
            posEstimator.est.eph = posControl.navConfig->inav.max_eph_epv / (1.0f + dt);
        }
#endif
    }
    else {
        inavFilterCorrectVel(&posEstimator.est.vel.V.X, dt, 0.0f - posEstimator.est.vel.V.X, posControl.navConfig->inav.w_xy_res_v);
        inavFilterCorrectVel(&posEstimator.est.vel.V.Y, dt, 0.0f - posEstimator.est.vel.V.Y, posControl.navConfig->inav.w_xy_res_v);
    }
}

/**
 * Examine estimation error and update navigation system if estimate is good enough
 *  Function is called at main loop rate, but updates happen less frequently - at a fixed rate
 */
static void publishEstimatedTopic(uint32_t currentTime)
{
    static bool lastPositionPublishTime;

    /* Publish position update and set position validity, do this at a fixed but reduced rate */
    if ((currentTime - lastPositionPublishTime) >= HZ2US(INAV_POSITION_PUBLISH_RATE_HZ)) {
        lastPositionPublishTime = currentTime;

        if (posEstimator.est.eph < posControl.navConfig->inav.max_eph_epv) {
            updateActualHorizontalPositionAndVelocity(posEstimator.est.pos.V.X, posEstimator.est.pos.V.Y, posEstimator.est.vel.V.X, posEstimator.est.vel.V.Y);
            posControl.flags.hasValidPositionSensor = true;
        }
        else {
            posControl.flags.hasValidPositionSensor = false;
        }

        /* Publish altitude update and set altitude validity */
        if (posEstimator.est.epv < posControl.navConfig->inav.max_eph_epv) {
            updateActualAltitudeAndClimbRate(posEstimator.est.pos.V.Z, posEstimator.est.vel.V.Z);
            posControl.flags.hasValidAltitudeSensor = true;
        }
        else {
            posControl.flags.hasValidAltitudeSensor = false;
        }

        /* Store history data */
        posEstimator.history.pos[posEstimator.history.index] = posEstimator.est.pos;
        posEstimator.history.vel[posEstimator.history.index] = posEstimator.est.vel;
        posEstimator.history.index++;
        if (posEstimator.history.index >= INAV_HISTORY_BUF_SIZE) {
            posEstimator.history.index = 0;
        }
    }

    updateActualHeading((int32_t)heading * 100);
}

/**
 * Initialize position estimator
 *  Should be called once before any update occurs
 */
void initializePositionEstimator(void)
{
    posEstimator.est.eph = posControl.navConfig->inav.max_eph_epv + 0.001f;
    posEstimator.est.epv = posControl.navConfig->inav.max_eph_epv + 0.001f;

    posEstimator.gps.lastUpdateTime = 0;
    posEstimator.baro.lastUpdateTime = 0;
    posEstimator.sonar.lastUpdateTime = 0;

    posEstimator.history.index = 0;

    memset(&posEstimator.history.pos[0], 0, sizeof(posEstimator.history.pos));
    memset(&posEstimator.history.vel[0], 0, sizeof(posEstimator.history.vel));
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

    uint32_t currentTime = micros();

    /* Periodic sensor updates */
#if defined(BARO)
    updateBaroTopic(currentTime);
#endif

#if defined(SONAR)
    updateSonarTopic(currentTime);
#endif

    /* Update estimate */
    updateEstimatedTopic(currentTime);

    /* Publish estimate */
    publishEstimatedTopic(currentTime);
}
