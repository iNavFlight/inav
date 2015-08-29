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

#if defined(NAV)

/* latest update for GPS local position and velocity*/
static t_fp_vector newGPSPos;
static t_fp_vector newGPSVel;
static bool newGPSDataAvailable;

/*-----------------------------------------------------------
 * NAV data collection and pre-processing code
 * This is the largest sensor-dependent part of nav-rewrite.
 * Adding new sensors, implementing EKF, etc. should modify
 * this part of code and do not touch the above code (if possible)
 *-----------------------------------------------------------*/
// Why is this here: Because GPS will be sending at quiet a nailed rate (if not overloaded by junk tasks at the brink of its specs)
// but we might read out with timejitter because Irq might be off by a few us so we do a +-10% margin around the time between GPS
// datasets representing the most common Hz-rates today. You might want to extend the list or find a smarter way.
// Don't overload your GPS in its config with trash, choose a Hz rate that it can deliver at a sustained rate.
// (c) CrashPilot1000
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
 * Calculate estimated heading and feed it to NAV Position Controller
 *  Update rate: loop/imu
 *  Params:
 *      newLat, newLon - new coordinates
 *      newAlt - new MSL altitude (cm)
 */
void onNewGPSData(int32_t newLat, int32_t newLon, int32_t newAlt)
{
    static uint32_t previousTime;
    static bool isFirstUpdate = true;
    static int32_t previousLat;
    static int32_t previousLon;
    static int32_t previousAlt;

    gpsLocation_t newLLH;

    // Don't have a valid GPS 3D fix, do nothing and restart
    if (!(STATE(GPS_FIX) && GPS_numSat >= 5)) {
        isFirstUpdate = true;
        return;
    }

    uint32_t currentTime = micros();

    // Convert to local coordinates
    newLLH.lat = newLat;
    newLLH.lon = newLon;
    newLLH.alt = newAlt;
    gpsConvertGeodeticToLocal(&posControl.gpsOrigin, &newLLH, &newGPSPos);

    navDebug[3] = newAlt;

    // If not first update - calculate velocities
    if (!isFirstUpdate) {
        float dT = US2S(getGPSDeltaTimeFilter(currentTime - previousTime));
        float gpsScaleLonDown = constrainf(cos_approx((ABS(newLat) / 10000000.0f) * 0.0174532925f), 0.01f, 1.0f);

        // Calculate NEU velocities
        newGPSVel.V.X = (newGPSVel.V.X + (DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR * (newLat - previousLat) / dT)) / 2.0f;
        newGPSVel.V.Y = (newGPSVel.V.Y + (gpsScaleLonDown * DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR * (newLon - previousLon) / dT)) / 2.0f;
        newGPSVel.V.Z = (newGPSVel.V.Z + (newAlt - previousAlt) / dT) / 2.0f;

        // Update IMU velocities with complementary filter to keep them close to real velocities (as given by GPS)
        imuApplyFilterToActualVelocity(X, posControl.navProfile->nav_gps_cf, newGPSVel.V.X);
        imuApplyFilterToActualVelocity(Y, posControl.navProfile->nav_gps_cf, newGPSVel.V.Y);

        updateActualHorizontalPositionAndVelocity(newGPSPos.V.X, newGPSPos.V.Y, imuAverageVelocity.V.X, imuAverageVelocity.V.Y);
        newGPSDataAvailable = true;
    }
    else {
        // Initialize GPS velocity
        newGPSVel.V.X = 0.0f;
        newGPSVel.V.Y = 0.0f;
        newGPSVel.V.Z = 0.0f;

        updateActualHorizontalPositionAndVelocity(newGPSPos.V.X, newGPSPos.V.Y, 0, 0);
        newGPSDataAvailable = false;
    }

    previousLat = newLat;
    previousLon = newLon;
    previousAlt = newAlt;

    isFirstUpdate = false;
    previousTime = currentTime;

    updateHomePosition();
}

/**
 * Calculate estimated heading and feed it to NAV Position Controller
 *  Update rate: ALTITUDE_UPDATE_FREQUENCY_HZ (10Hz)
 */
void updateEstimatedHeading(void)
{
    // NAV uses heading in centidegrees
    updateActualHeading((int32_t)heading * 100);
}

/**
 * Calculate estimated altitude and climb rate and update NAV Position Controller
 *  Update rate: loop/imu
 */
void updateAltitudeAndClimbRate(void)
{
    float estimatedAlt, estimatedClimbRate, imuFilterWeight;

#if defined(BARO)
    float newBaroAlt, baroClimbRate;
#endif

#if defined(SONAR)
    float newSonarAlt, sonarClimbRate;
#endif

#if defined(BARO)
    static filterWithBufferSample_t baroClimbRateFilterBuffer[NAV_BARO_CLIMB_RATE_FILTER_SIZE];
    static filterWithBufferState_t baroClimbRateFilter;
#endif
    static bool climbRateFiltersInitialized = false;

    static uint32_t previousTimeUpdate = 0;
    uint32_t currentTime = micros();
    uint32_t deltaMicros = currentTime - previousTimeUpdate;

    // too fast, likely no new data available
    if (deltaMicros < HZ2US(ALTITUDE_UPDATE_FREQUENCY_HZ))
        return;

    previousTimeUpdate = currentTime;

    if (sensors(SENSOR_BARO) || sensors(SENSOR_SONAR)) {
        // Initialize climb rate filter
        if (!climbRateFiltersInitialized) {
#if defined(BARO)
            // If BARO compiled in - initialize filter
            filterWithBufferInit(&baroClimbRateFilter, &baroClimbRateFilterBuffer[0], NAV_BARO_CLIMB_RATE_FILTER_SIZE);
#endif
            climbRateFiltersInitialized = true;
        }

#if defined(BARO)
        // Calculate barometric altitude and climb rate
        // For NAV to work good baro altitude must not be delayed much. Large delay means slow response, means low PID gains to avoid oscillations
        // One should keep baro_tab_size small, but this will lead to high noise. NAV is OK with noisy measures, LPFs in altitude control
        // code and new CLT fusion will handle that just fine
        newBaroAlt = baroCalculateAltitude();

        if (sensors(SENSOR_BARO) && isBaroCalibrationComplete()) {
            filterWithBufferUpdate(&baroClimbRateFilter, newBaroAlt, currentTime);
            baroClimbRate = filterWithBufferApply_Derivative(&baroClimbRateFilter) * 1e6f;

            baroClimbRate = constrainf(baroClimbRate, -1500, 1500);  // constrain baro velocity +/- 1500cm/s
            baroClimbRate = applyDeadband(baroClimbRate, 10);       // to reduce noise near zero

            posControl.flags.hasValidAltitudeSensor = true;
        }
        else {
            newBaroAlt = 0;
            baroClimbRate = 0.0f;
        }
#endif

#if defined(SONAR)
        // Calculate sonar altitude above surface and climb rate above surface
        if (sensors(SENSOR_SONAR)) {
            static uint32_t lastValidSonarUpdateTime = 0;
            static float lastValidSonarAlt;

            // Read sonar
            newSonarAlt = sonarRead();

            // FIXME: Add sonar tilt compensation

            if (newSonarAlt > 0) {
                // We have a valid reading

                if ((currentTime - lastValidSonarUpdateTime) < HZ2US(MIN_SONAR_UPDATE_FREQUENCY_HZ)) {
                    // Sonar updated within valid time, use this measurement to calculate climb rate
                    sonarClimbRate = (newSonarAlt - lastValidSonarAlt) / ((currentTime - lastValidSonarUpdateTime) * 1e-6f);
                }
                else {
                    // Previous sonar update was delayed too much - we can't trust sonarClimbRate yet
                    sonarClimbRate = 0.0f;
                }

                lastValidSonarAlt = newSonarAlt;
                lastValidSonarUpdateTime = currentTime;

                posControl.flags.hasValidAltitudeSensor = true;
            }
        }
        else {
            // No sonar
            newSonarAlt = -1;
            sonarClimbRate = 0.0f;
        }
#endif

#if defined(BARO) && defined(SONAR)
        if (newSonarAlt <= 0) {
            // Can't trust sonar - rely on baro
            estimatedAlt = newBaroAlt;
            estimatedClimbRate = baroClimbRate;
            imuFilterWeight = posControl.barometerConfig->baro_cf_vel;
        }
        else {
            // Fuse altitude
            if (newSonarAlt <= (SONAR_MAX_RANGE * 2 / 3)) {
                // If within 2/3 sonar range - use only sonar
                estimatedAlt = newSonarAlt;
            }
            else if (newSonarAlt <= SONAR_MAX_RANGE) {
                // Squeze difference between sonar and baro into upper 1/3 sonar range.
                // FIXME: this will give us totally wrong altitude in the upper
                //        1/3 sonar range but will allow graceful transition from SONAR to BARO
                float sonarToBaroTransition = constrainf((SONAR_MAX_RANGE - newSonarAlt) / (SONAR_MAX_RANGE / 3.0f), 0, 1);
                estimatedAlt = newSonarAlt * sonarToBaroTransition + newBaroAlt * (1.0f - sonarToBaroTransition);
            }
            else {
                // Sonar driver returned a value > SONAR_MAX_RANGE, ignore it, rely on baro altitude
                estimatedAlt = newBaroAlt;
            }

            // FIXME: Make all this configurable
            // Fuse velocity - trust sonar more and baro less
            estimatedClimbRate = 0.333f * baroClimbRate + 0.667f * sonarClimbRate;
            imuFilterWeight = posControl.barometerConfig->baro_cf_vel;
        }
#elif defined(BARO)
        estimatedAlt = newBaroAlt;
        estimatedClimbRate = baroClimbRate;
        imuFilterWeight = posControl.barometerConfig->baro_cf_vel;
#elif defined(SONAR)
        // If sonar reading is not valid, assume that we are outside valid sonar range and set measured altitude to SONAR_MAX_RANGE
        // This will allow us to enable althold above sonar range and not hit the ground when going within range
        if (newSonarAlt <= 0)
            estimatedAlt = SONAR_MAX_RANGE;
        else
            estimatedAlt = newSonarAlt;

        estimatedClimbRate = sonarClimbRate;
        imuFilterWeight = 0.900f; // FIXME: make configurable
#else
        #error "FIXME - Shouldn't happen (no baro or sonar)"
#endif
    }
    else if (sensors(SENSOR_GPS) && newGPSDataAvailable) {
        // It is possible that we have GPS but don't have a BARO or SONAR. In such rare case rely on GPS
        //  This is the worst case scenario, GPS does not provide reliable climb rate and GPS Z-position is very noisy
        //  This CAN NOT be used on a multicopter
        static float gpsAltFilterLPFState = 0.0f;
        static float gpsVelFilterLPFState = 0.0f;

        // Apply LPF to GPS altitude (posControl will also add it's own LPFs, so we shouldn't go too low in frequency here)
        estimatedAlt = navApplyFilter(newGPSPos.V.Z, 2.0f, deltaMicros * 1e-6, &gpsAltFilterLPFState);
        estimatedClimbRate = navApplyFilter(newGPSVel.V.Z, 2.0f, deltaMicros * 1e-6, &gpsVelFilterLPFState);
        imuFilterWeight = posControl.barometerConfig->baro_cf_vel;  // FIXME

        newGPSDataAvailable = false;
        posControl.flags.hasValidAltitudeSensor = true;
    }

    // By using CF it's possible to correct the drift of integrated accZ (velocity) without loosing the phase, i.e without delay
    imuApplyFilterToActualVelocity(Z, imuFilterWeight, estimatedClimbRate);

    updateActualAltitudeAndClimbRate(estimatedAlt, imuAverageVelocity.V.Z);
}

#endif  // NAV
