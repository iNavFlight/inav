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


//#include "build/debug.h"


#ifdef USE_PITOT

extern baro_t baro;

pitot_t pitot = {.lastMeasurementUs = 0, .lastSeenHealthyMs = 0};

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

    pt1FilterInit(&pitot.lpfState, pitotmeterConfig()->pitot_lpf_milli_hz / 1000.0f, 0.0f);

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
            pitot.pressure = pt1FilterApply3(&pitot.lpfState, pitotPressureTmp, US2S(currentTimeUs - pitot.lastMeasurementUs));
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
 */
float getAirspeedEstimate(void)
{
    return pitot.airSpeed;
}

bool pitotIsHealthy(void)
{
    return (millis() - pitot.lastSeenHealthyMs) < PITOT_HARDWARE_TIMEOUT_MS;
}

#endif /* PITOT */
