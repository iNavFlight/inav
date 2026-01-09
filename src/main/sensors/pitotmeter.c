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

#include "platform.h"

#include "common/log.h"
#include "common/maths.h"
#include "common/time.h"
#include "common/utils.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/pitotmeter/pitotmeter.h"
#include "drivers/pitotmeter/pitotmeter_ms4525.h"
#include "drivers/pitotmeter/pitotmeter_dlvr_l10d.h"
#include "drivers/pitotmeter/pitotmeter_adc.h"
#include "drivers/pitotmeter/pitotmeter_msp.h"
#include "drivers/pitotmeter/pitotmeter_virtual.h"
#include "drivers/pitotmeter/pitotmeter_fake.h"
#include "drivers/time.h"

#include "fc/config.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "scheduler/protothreads.h"

#include "sensors/pitotmeter.h"
#include "sensors/barometer.h"
#include "sensors/sensors.h"

#include "io/gps.h"

#ifdef USE_WIND_ESTIMATOR
#include "flight/wind_estimator.h"
#include "navigation/navigation.h"
#include "navigation/navigation_private.h"
#endif

//#include "build/debug.h"


#ifdef USE_PITOT

extern baro_t baro;

pitot_t pitot = {.lastMeasurementUs = 0, .lastSeenHealthyMs = 0};

// Pitot sensor validation state
static bool pitotHardwareFailed = false;
static uint16_t pitotFailureCounter = 0;
static uint16_t pitotRecoveryCounter = 0;
#define PITOT_FAILURE_THRESHOLD 20   // 0.2 seconds at 100Hz - fast detection per LOG00002 analysis
#define PITOT_RECOVERY_THRESHOLD 200 // 2 seconds of consecutive good readings to recover

// Forward declaration for GPS-based airspeed fallback
static float getVirtualAirspeedEstimate(void);

PG_REGISTER_WITH_RESET_TEMPLATE(pitotmeterConfig_t, pitotmeterConfig, PG_PITOTMETER_CONFIG, 2);

#define PITOT_HARDWARE_TIMEOUT_MS   500     // Accept 500ms of non-responsive sensor, report HW failure otherwise

#ifdef USE_PITOT
#define PITOT_HARDWARE_DEFAULT    PITOT_AUTODETECT
#else
#define PITOT_HARDWARE_DEFAULT    PITOT_NONE
#endif

PG_RESET_TEMPLATE(pitotmeterConfig_t, pitotmeterConfig,
    .pitot_hardware = SETTING_PITOT_HARDWARE_DEFAULT,
    .pitot_lpf_milli_hz = SETTING_PITOT_LPF_MILLI_HZ_DEFAULT,
    .pitot_scale = SETTING_PITOT_SCALE_DEFAULT
);

bool pitotDetect(pitotDev_t *dev, uint8_t pitotHardwareToUse)
{
    pitotSensor_e pitotHardware = PITOT_NONE;
    requestedSensors[SENSOR_INDEX_PITOT] = pitotHardwareToUse;

    switch (pitotHardwareToUse) {
        case PITOT_AUTODETECT:
        case PITOT_MS4525:
#ifdef USE_PITOT_MS4525
            if (ms4525Detect(dev)) {
                pitotHardware = PITOT_MS4525;
                break;
            }
#endif
            /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
            if (pitotHardwareToUse != PITOT_AUTODETECT) {
                break;
            }
            FALLTHROUGH;

        case PITOT_DLVR:

			// Skip autodetection for DLVR (it is indistinguishable from MS4525) and allow only manual config
            if (pitotHardwareToUse != PITOT_AUTODETECT && dlvrDetect(dev)) {
                pitotHardware = PITOT_DLVR;
                break;
            }
            FALLTHROUGH;

        case PITOT_ADC:
#if defined(USE_ADC) && defined(USE_PITOT_ADC)
            if (adcPitotDetect(dev)) {
                pitotHardware = PITOT_ADC;
                break;
            }
#endif
            /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
            if (pitotHardwareToUse != PITOT_AUTODETECT) {
                break;
            }
            FALLTHROUGH;

        case PITOT_VIRTUAL:
#if defined(USE_WIND_ESTIMATOR) && defined(USE_PITOT_VIRTUAL) 
            if ((pitotHardwareToUse != PITOT_AUTODETECT) && virtualPitotDetect(dev)) {
                pitotHardware = PITOT_VIRTUAL;
                break;
            }
#endif
            /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
            if (pitotHardwareToUse != PITOT_AUTODETECT) {
                break;
            }
            FALLTHROUGH;

        case PITOT_MSP:
#ifdef USE_PITOT_MSP
            // Skip autodetection for MSP baro, only allow manual config
            if (pitotHardwareToUse != PITOT_AUTODETECT && mspPitotmeterDetect(dev)) {
                pitotHardware = PITOT_MSP;
                break;
            }
#endif
            /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
            if (pitotHardwareToUse != PITOT_AUTODETECT) {
                break;
            }
            FALLTHROUGH;

        case PITOT_FAKE:
#ifdef USE_PITOT_FAKE
            if (fakePitotDetect(dev)) {
                pitotHardware = PITOT_FAKE;
                break;
            }
#endif
            /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
            if (pitotHardwareToUse != PITOT_AUTODETECT) {
                break;
            }
            FALLTHROUGH;

        case PITOT_NONE:
            pitotHardware = PITOT_NONE;
            break;
    }

    if (pitotHardware == PITOT_NONE) {
        sensorsClear(SENSOR_PITOT);
        return false;
    }

    detectedSensors[SENSOR_INDEX_PITOT] = pitotHardware;
    sensorsSet(SENSOR_PITOT);
    return true;
}

bool pitotInit(void)
{
    if (!pitotDetect(&pitot.dev, pitotmeterConfig()->pitot_hardware)) {
        return false;
    }
    return true;
}

bool pitotIsCalibrationComplete(void)
{
    return zeroCalibrationIsCompleteS(&pitot.zeroCalibration) && zeroCalibrationIsSuccessfulS(&pitot.zeroCalibration);
}

void pitotStartCalibration(void)
{
    zeroCalibrationStartS(&pitot.zeroCalibration, CALIBRATING_PITOT_TIME_MS, SSL_AIR_PRESSURE * pitot.dev.calibThreshold, false);
}

static void performPitotCalibrationCycle(void)
{
    zeroCalibrationAddValueS(&pitot.zeroCalibration, pitot.pressure);

    if (zeroCalibrationIsCompleteS(&pitot.zeroCalibration)) {
        zeroCalibrationGetZeroS(&pitot.zeroCalibration, &pitot.pressureZero);
        LOG_DEBUG(PITOT, "Pitot calibration complete (%d)", (int)lrintf(pitot.pressureZero));
    }
}

STATIC_PROTOTHREAD(pitotThread)
{
    ptBegin(pitotThread);

    static float pitotPressureTmp;
    static float pitotTemperatureTmp;
    timeUs_t currentTimeUs;

    // Init filter
    pitot.lastMeasurementUs = micros();
    if(pitotmeterConfig()->pitot_lpf_milli_hz >0){
        pt1FilterInit(&pitot.lpfState, pitotmeterConfig()->pitot_lpf_milli_hz / 1000.0f, 0.0f);
    }
    while(1) {
#ifdef USE_SIMULATOR
    	while (SIMULATOR_HAS_OPTION(HITL_AIRSPEED) && SIMULATOR_HAS_OPTION(HITL_PITOT_FAILURE))
        {
            ptDelayUs(10000);
    	}
#endif

        if ( pitot.lastSeenHealthyMs == 0 ) {
            if (pitot.dev.start(&pitot.dev)) {
                pitot.lastSeenHealthyMs = millis();
            }        
        }

        if ( (millis() - pitot.lastSeenHealthyMs) >= US2MS(pitot.dev.delay)) {
            if (pitot.dev.get(&pitot.dev))          // read current data
                pitot.lastSeenHealthyMs = millis();

            if (pitot.dev.start(&pitot.dev))        // init for next read
                pitot.lastSeenHealthyMs = millis();        
        }


        pitot.dev.calculate(&pitot.dev, &pitotPressureTmp, &pitotTemperatureTmp);

#ifdef USE_SIMULATOR
        if (SIMULATOR_HAS_OPTION(HITL_AIRSPEED)) {
            pitotPressureTmp = sq(simulatorData.airSpeed) * SSL_AIR_DENSITY / 20000.0f + SSL_AIR_PRESSURE;     
        }
#endif
#if defined(USE_PITOT_FAKE)
        if (pitotmeterConfig()->pitot_hardware == PITOT_FAKE) { 
            pitotPressureTmp = sq(fakePitotGetAirspeed()) * SSL_AIR_DENSITY / 20000.0f + SSL_AIR_PRESSURE;     
        } 
#endif
        ptYield();

        // Calculate IAS
        if (pitotIsCalibrationComplete()) {
            // NOTE ::
            // https://en.wikipedia.org/wiki/Indicated_airspeed
            // Indicated airspeed (IAS) is the airspeed read directly from the airspeed indicator on an aircraft, driven by the pitot-static system.
            // The IAS is an important value for the pilot because it is the indicated speeds which are specified in the aircraft flight manual for
            // such important performance values as the stall speed. A typical aircraft will always stall at the same indicated airspeed (for the current configuration)
            // regardless of density, altitude or true airspeed.
            //
            // Therefore we shouldn't care about CAS/TAS and only calculate IAS since it's more indicative to the pilot and more useful in calculations
            // It also allows us to use pitot_scale to calibrate the dynamic pressure sensor scale

            // NOTE ::filter pressure - apply filter when NOT calibrating for zero !!!
            currentTimeUs = micros();
            if(pitotmeterConfig()->pitot_lpf_milli_hz >0){
                pitot.pressure = pt1FilterApply3(&pitot.lpfState, pitotPressureTmp, US2S(currentTimeUs - pitot.lastMeasurementUs));
            }else{
                pitot.pressure = pitotPressureTmp;
            }
            pitot.lastMeasurementUs = currentTimeUs;

            pitot.airSpeed = pitotmeterConfig()->pitot_scale * fast_fsqrtf(2.0f * fabsf(pitot.pressure - pitot.pressureZero) / SSL_AIR_DENSITY) * 100;  // cm/s
            pitot.temperature = pitotTemperatureTmp;   // Kelvin

        } else {
            pitot.pressure = pitotPressureTmp;
            performPitotCalibrationCycle();
            pitot.airSpeed = 0.0f;
        }

#if defined(USE_PITOT_FAKE)
        if (pitotmeterConfig()->pitot_hardware == PITOT_FAKE) { 
            pitot.airSpeed = fakePitotGetAirspeed();
        }
#endif
#ifdef USE_SIMULATOR
        if (SIMULATOR_HAS_OPTION(HITL_AIRSPEED)) {
            pitot.airSpeed = simulatorData.airSpeed;
        }
#endif
    }

    ptEnd(0);
}

void pitotUpdate(void)
{
    pitotThread();
}

/*
 * Airspeed estimate in cm/s
 * Returns hardware pitot if valid, GPS-based virtual airspeed if pitot failed,
 * or raw pitot value as last resort
 */
float getAirspeedEstimate(void)
{
    // If hardware pitot has failed validation, use GPS-based virtual airspeed
    if (pitotHardwareFailed) {
        float virtualAirspeed = getVirtualAirspeedEstimate();
        if (virtualAirspeed > 0.0f) {
            return virtualAirspeed;
        }
    }
    return pitot.airSpeed;
}

bool pitotIsHealthy(void)
{
    return (millis() - pitot.lastSeenHealthyMs) < PITOT_HARDWARE_TIMEOUT_MS;
}

/**
 * Calculate virtual airspeed estimate (same as virtual pitot)
 *
 * Uses GPS velocity with wind correction when available, providing a reference
 * airspeed that already accounts for wind conditions.
 *
 * @return virtual airspeed in cm/s, or 0 if GPS unavailable
 */
static float getVirtualAirspeedEstimate(void)
{
#if defined(USE_GPS) && defined(USE_WIND_ESTIMATOR)
    if (!STATE(GPS_FIX)) {
        return 0.0f;
    }

    float airSpeed = 0.0f;

    // Use wind estimator if available (matches virtual pitot logic)
    if (isEstimatedWindSpeedValid()) {
        uint16_t windHeading;  // centidegrees
        float windSpeed = getEstimatedHorizontalWindSpeed(&windHeading);  // cm/s
        float horizontalWindSpeed = windSpeed * cos_approx(CENTIDEGREES_TO_RADIANS(windHeading - posControl.actualState.yaw));
        airSpeed = posControl.actualState.velXY - horizontalWindSpeed;
        airSpeed = calc_length_pythagorean_2D(airSpeed, getEstimatedActualVelocity(Z) + getEstimatedWindSpeed(Z));
    } else {
        // Fall back to raw GPS velocity if no wind estimator
        airSpeed = calc_length_pythagorean_3D(gpsSol.velNED[X], gpsSol.velNED[Y], gpsSol.velNED[Z]);
    }

    return airSpeed;
#elif defined(USE_GPS)
    // No wind estimator, use raw GPS velocity
    if (!STATE(GPS_FIX)) {
        return 0.0f;
    }
    return calc_length_pythagorean_3D(gpsSol.velNED[X], gpsSol.velNED[Y], gpsSol.velNED[Z]);
#else
    return 0.0f;
#endif
}

/**
 * Pitot sensor sanity check against virtual airspeed
 *
 * Compares hardware pitot reading against virtual airspeed (GPS + wind estimator)
 * to detect gross sensor failures like blocked pitot tubes.
 *
 * Uses wide thresholds to catch implausible readings while avoiding false positives:
 * - Compares against wind-corrected virtual airspeed (not raw GPS groundspeed)
 * - Wide tolerance (30%-200%) catches gross failures only
 * - Detects: blocked pitot reading 25 km/h when virtual shows 85 km/h
 * - Avoids: false positives from sensor accuracy differences
 *
 * @return true if pitot reading appears plausible, false if likely failed
 */
static bool isPitotReadingPlausible(void)
{
#ifdef USE_GPS
    if (!STATE(GPS_FIX)) {
        return true;
    }

    const float virtualAirspeedCmS = getVirtualAirspeedEstimate();
    const float minValidationSpeed = 700.0f;  // 7 m/s

    if (virtualAirspeedCmS < minValidationSpeed) {
        return true;
    }

    const float pitotAirspeedCmS = pitot.airSpeed;

    // Wide thresholds to catch gross failures (blocked pitot) only
    const float minPlausibleAirspeed = virtualAirspeedCmS * 0.3f;  // 30% of virtual
    const float maxPlausibleAirspeed = virtualAirspeedCmS * 2.0f;  // 200% of virtual

    if (pitotAirspeedCmS < minPlausibleAirspeed || pitotAirspeedCmS > maxPlausibleAirspeed) {
        return false;
    }

    return true;
#else
    return true;
#endif
}

/**
 * Check if pitot sensor has failed validation
 *
 * @return true if pitot has failed sanity checks and should not be trusted
 */
bool pitotHasFailed(void)
{
    return pitotHardwareFailed;
}

bool pitotValidForAirspeed(void)
{
    bool ret = false;
    ret = pitotIsHealthy() && pitotIsCalibrationComplete();

    // For virtual pitot, we need GPS fix
    if (detectedSensors[SENSOR_INDEX_PITOT] == PITOT_VIRTUAL) {
        ret = ret && STATE(GPS_FIX);
    }

    // For hardware pitot sensors, validate readings against GPS when armed
    // This detects blocked or failed pitot tubes
    if (ret && detectedSensors[SENSOR_INDEX_PITOT] != PITOT_VIRTUAL &&
        detectedSensors[SENSOR_INDEX_PITOT] != PITOT_NONE) {

        if (ARMING_FLAG(ARMED)) {
            // Check if pitot reading is plausible
            if (!isPitotReadingPlausible()) {
                pitotFailureCounter++;
            } else if (pitotFailureCounter > 0) {
                // Decay counter if sensor appears healthy
                pitotFailureCounter--;
            }

            // Declare failure after sustained implausible readings
            if (pitotFailureCounter >= PITOT_FAILURE_THRESHOLD) {
                pitotHardwareFailed = true;
                pitotRecoveryCounter = 0;  // Start recovery tracking
            }

            // Recovery: require sustained consecutive good readings to clear failure
            if (pitotHardwareFailed) {
                if (isPitotReadingPlausible()) {
                    pitotRecoveryCounter++;
                    if (pitotRecoveryCounter >= PITOT_RECOVERY_THRESHOLD) {
                        pitotHardwareFailed = false;  // Sensor has recovered
                        pitotFailureCounter = 0;
                        pitotRecoveryCounter = 0;
                    }
                } else {
                    // Bad reading resets recovery progress
                    pitotRecoveryCounter = 0;
                }
            }
        } else {
            // Reset on disarm for next flight
            pitotHardwareFailed = false;
            pitotFailureCounter = 0;
            pitotRecoveryCounter = 0;
        }

        // If pitot has failed sanity checks, require GPS fix (like virtual pitot)
        if (pitotHardwareFailed) {
            ret = ret && STATE(GPS_FIX);
        }
    }

    return ret;
}
#endif /* PITOT */
