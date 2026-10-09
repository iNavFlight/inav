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
#include "build/debug.h"

#include "common/calibration.h"
#include "common/log.h"
#include "common/maths.h"
#include "common/time.h"
#include "common/utils.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/barometer/barometer.h"
#include "drivers/barometer/barometer_bmp085.h"
#include "drivers/barometer/barometer_bmp280.h"
#include "drivers/barometer/barometer_bmp388.h"
#include "drivers/barometer/barometer_lps25h.h"
#include "drivers/barometer/barometer_fake.h"
#include "drivers/barometer/barometer_ms56xx.h"
#include "drivers/barometer/barometer_spl06.h"
#include "drivers/barometer/barometer_dps310.h"
#include "drivers/barometer/barometer_2smpb_02b.h"
#include "drivers/barometer/barometer_msp.h"
#include "drivers/barometer/barometer_crsf.h"
#include "drivers/time.h"

#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "sensors/barometer.h"
#include "sensors/sensors.h"

#ifdef USE_HARDWARE_REVISION_DETECTION
#include "hardware_revision.h"
#endif

baro_t baro;                        // barometer access functions

#ifdef USE_BARO

PG_REGISTER_WITH_RESET_TEMPLATE(barometerConfig_t, barometerConfig, PG_BAROMETER_CONFIG, 5);

PG_RESET_TEMPLATE(barometerConfig_t, barometerConfig,
    .baro_hardware = SETTING_BARO_HARDWARE_DEFAULT,
    .baro_calibration_tolerance = SETTING_BARO_CAL_TOLERANCE_DEFAULT,
    .baro_temp_correction = SETTING_BARO_TEMP_CORRECTION_DEFAULT,
);

static zeroCalibrationScalar_t zeroCalibration;
static float baroGroundAltitude = 0;
static float baroGroundPressure = 101325.0f; // 101325 pascal, 1 standard atmosphere

bool baroDetect(baroDev_t *dev, baroSensor_e baroHardwareToUse)
{
    // Detect what pressure sensors are available. baro->update() is set to sensor-specific update function

    baroSensor_e baroHardware = BARO_NONE;
    requestedSensors[SENSOR_INDEX_BARO] = baroHardwareToUse;

    switch (baroHardwareToUse) {
    case BARO_AUTODETECT:
    case BARO_BMP085:
#ifdef USE_BARO_BMP085
        if (bmp085Detect(dev)) {
            baroHardware = BARO_BMP085;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (baroHardwareToUse != BARO_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case BARO_MS5607:
#ifdef USE_BARO_MS5607
        if (ms5607Detect(dev)) {
            baroHardware = BARO_MS5607;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (baroHardwareToUse != BARO_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case BARO_MS5611:
#ifdef USE_BARO_MS5611
        if (ms5611Detect(dev)) {
            baroHardware = BARO_MS5611;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (baroHardwareToUse != BARO_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case BARO_BMP280:
#if defined(USE_BARO_BMP280) || defined(USE_BARO_SPI_BMP280)
        if (bmp280Detect(dev)) {
            baroHardware = BARO_BMP280;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (baroHardwareToUse != BARO_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case BARO_BMP388:
#if defined(USE_BARO_BMP388) || defined(USE_BARO_SPI_BMP388)
        if (bmp388Detect(dev)) {
            baroHardware = BARO_BMP388;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (baroHardwareToUse != BARO_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case BARO_SPL06:
#if defined(USE_BARO_SPL06) || defined(USE_BARO_SPI_SPL06)
        if (spl06Detect(dev)) {
            baroHardware = BARO_SPL06;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (baroHardwareToUse != BARO_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case BARO_LPS25H:
#if defined(USE_BARO_LPS25H)
        if (lps25hDetect(dev)) {
            baroHardware = BARO_LPS25H;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (baroHardwareToUse != BARO_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case BARO_DPS310:
#if defined(USE_BARO_DPS310)
        if (baroDPS310Detect(dev)) {
            baroHardware = BARO_DPS310;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (baroHardwareToUse != BARO_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case BARO_B2SMPB:
#if defined(USE_BARO_B2SMPB)
        if (baro2SMPB02BDetect(dev)) {
            baroHardware = BARO_B2SMPB;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (baroHardwareToUse != BARO_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case BARO_MSP:
#ifdef USE_BARO_MSP
        // Skip autodetection for MSP baro, only allow manual config
        if (baroHardwareToUse != BARO_AUTODETECT && mspBaroDetect(dev)) {
            baroHardware = BARO_MSP;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (baroHardwareToUse != BARO_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case BARO_CRSF:
#ifdef USE_BARO_CRSF
        // Skip autodetection for CRSF baro, only allow manual config
        if (baroHardwareToUse != BARO_AUTODETECT && crsfBaroDetect(dev)) {
            baroHardware = BARO_CRSF;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (baroHardwareToUse != BARO_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case BARO_FAKE:
#ifdef USE_FAKE_BARO
        if (fakeBaroDetect(dev)) {
            baroHardware = BARO_FAKE;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (baroHardwareToUse != BARO_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case BARO_NONE:
        baroHardware = BARO_NONE;
        break;
    }

    if (baroHardware == BARO_NONE) {
        sensorsClear(SENSOR_BARO);
        return false;
    }

    detectedSensors[SENSOR_INDEX_BARO] = baroHardware;
    sensorsSet(SENSOR_BARO);
    return true;
}

bool baroInit(void)
{
    if (!baroDetect(&baro.dev, barometerConfig()->baro_hardware)) {
        return false;
    }
    return true;
}

typedef enum {
    BARO_STATE_TEMPERATURE_START = 0,
    BARO_STATE_TEMPERATURE_READ,
    BARO_STATE_TEMPERATURE_SAMPLE,
    BARO_STATE_PRESSURE_START,
    BARO_STATE_PRESSURE_READ,
    BARO_STATE_PRESSURE_SAMPLE,
} barometerState_e;

// Delay before looking at a non-blocking bus transfer again
#define BARO_STATE_STEP_DELAY_US    1000

/*
 * Runs the measurement cycle described in drivers/barometer/barometer.h.
 * Returns the delay until the next call (0 keeps the current task period). *newSampleReady is set when
 * baro.baroPressure / baro.baroTemperature were updated in this call.
 */
uint32_t baroUpdate(bool *newSampleReady)
{
    static barometerState_e state = BARO_STATE_TEMPERATURE_START;
    baroDev_t *dev = &baro.dev;
    bool busError = false;

    *newSampleReady = false;

#ifdef USE_SIMULATOR
    if (ARMING_FLAG(SIMULATOR_MODE_HITL)) {
        // Pressure and temperature are injected over MSP, keep publishing them at the sensor's pace. The two delays
        // together are one measurement cycle; up_delay alone is a 1 ms scheduler step for some drivers (DPS310).
        // Without a physical sensor both are 0 and the task keeps its default period
        *newSampleReady = true;
        return dev->ut_delay + dev->up_delay;
    }
#endif

    // States that need no wait on the bus run back to back within one call
    for (;;) {
        switch (state) {
            default:
            case BARO_STATE_TEMPERATURE_START:
                if (dev->start_ut && !dev->start_ut(dev) && dev->read_ut) {
                    return BARO_STATE_STEP_DELAY_US;    // non-blocking start refused, bus is busy
                }
                state = BARO_STATE_TEMPERATURE_READ;
                return dev->ut_delay;

            case BARO_STATE_TEMPERATURE_READ:
                if (dev->read_ut) {
                    // The start_ut write has to have reached the sensor, otherwise the data registers still hold the
                    // previous conversion and a successful read would publish it as a new sample
                    if (dev->start_ut && dev->busDev) {
                        if (busIsBusy(dev->busDev, &busError)) {
                            return BARO_STATE_STEP_DELAY_US;    // trigger write still in progress
                        }
                        if (busError) {
                            state = BARO_STATE_TEMPERATURE_START;   // trigger write failed, redo the phase
                            break;
                        }
                    }
                    if (!dev->read_ut(dev)) {
                        return BARO_STATE_STEP_DELAY_US;    // bus is busy, try again shortly
                    }
                    state = BARO_STATE_TEMPERATURE_SAMPLE;
                    return BARO_STATE_STEP_DELAY_US;
                }
                state = BARO_STATE_TEMPERATURE_SAMPLE;      // blocking driver, sample right away
                break;

            case BARO_STATE_TEMPERATURE_SAMPLE:
                if (dev->read_ut) {
                    if (dev->busDev && busIsBusy(dev->busDev, &busError) && !busError) {
                        return BARO_STATE_STEP_DELAY_US;    // transfer still in progress
                    }
                    if (busError || (dev->get_ut && !dev->get_ut(dev))) {
                        state = BARO_STATE_TEMPERATURE_START;   // transfer failed or sample unusable, redo the phase
                        break;
                    }
                }
                else if (dev->get_ut) {
                    dev->get_ut(dev);
                }
                state = BARO_STATE_PRESSURE_START;
                break;

            case BARO_STATE_PRESSURE_START:
                if (dev->start_up && !dev->start_up(dev) && dev->read_up) {
                    return BARO_STATE_STEP_DELAY_US;    // non-blocking start refused, bus is busy
                }
                state = BARO_STATE_PRESSURE_READ;
                return dev->up_delay;

            case BARO_STATE_PRESSURE_READ:
                if (dev->read_up) {
                    if (dev->start_up && dev->busDev) {
                        if (busIsBusy(dev->busDev, &busError)) {
                            return BARO_STATE_STEP_DELAY_US;    // trigger write still in progress
                        }
                        if (busError) {
                            state = BARO_STATE_PRESSURE_START;  // trigger write failed, redo the phase
                            break;
                        }
                    }
                    if (!dev->read_up(dev)) {
                        return BARO_STATE_STEP_DELAY_US;    // bus is busy, try again shortly
                    }
                    state = BARO_STATE_PRESSURE_SAMPLE;
                    return BARO_STATE_STEP_DELAY_US;
                }
                state = BARO_STATE_PRESSURE_SAMPLE;         // blocking driver, sample right away
                break;

            case BARO_STATE_PRESSURE_SAMPLE:
                if (dev->read_up) {
                    if (dev->busDev && busIsBusy(dev->busDev, &busError) && !busError) {
                        return BARO_STATE_STEP_DELAY_US;    // transfer still in progress
                    }
                    if (busError || (dev->get_up && !dev->get_up(dev))) {
                        state = BARO_STATE_PRESSURE_START;  // transfer failed or sample unusable, redo the phase
                        break;
                    }
                }
                else if (dev->get_up) {
                    dev->get_up(dev);
                }

                //output: baro.baroPressure, baro.baroTemperature
                dev->calculate(dev, &baro.baroPressure, &baro.baroTemperature);
                *newSampleReady = true;
                state = dev->combined_read ? BARO_STATE_PRESSURE_START : BARO_STATE_TEMPERATURE_START;
                break;
        }
    }
}

static float pressureToAltitude(const float pressure)
{
    return (1.0f - powf(pressure / 101325.0f, 0.190295f)) * 4433000.0f;
}

float altitudeToPressure(const float altCm)
{
    return powf(1.0f - (altCm / 4433000.0f), 5.254999) * 101325.0f;
}

bool baroIsCalibrationComplete(void)
{
    return zeroCalibrationIsCompleteS(&zeroCalibration) && zeroCalibrationIsSuccessfulS(&zeroCalibration);
}

void baroStartCalibration(void)
{
    const float acceptedPressureVariance = (101325.0f - altitudeToPressure(barometerConfig()->baro_calibration_tolerance)); // max 30cm deviation during calibration (at sea level)
    zeroCalibrationStartS(&zeroCalibration, CALIBRATING_BARO_TIME_MS, acceptedPressureVariance, false);
}

int32_t baroCalculateAltitude(void)
{
    if (!baroIsCalibrationComplete()) {
        zeroCalibrationAddValueS(&zeroCalibration, baro.baroPressure);

        if (zeroCalibrationIsCompleteS(&zeroCalibration)) {
            zeroCalibrationGetZeroS(&zeroCalibration, &baroGroundPressure);
            baroGroundAltitude = pressureToAltitude(baroGroundPressure);
            LOG_DEBUG(BARO, "Barometer calibration complete (%d)", (int)lrintf(baroGroundAltitude));
        }

        baro.BaroAlt = 0;
    }
    else {
        // calculates height from ground via baro readings
        baro.BaroAlt = pressureToAltitude(baro.baroPressure) - baroGroundAltitude;
        baro.BaroAlt += applySensorTempCompensation(baro.baroTemperature, baro.BaroAlt, SENSOR_INDEX_BARO);
   }

    return baro.BaroAlt;
}

int32_t baroGetLatestAltitude(void)
{
    return baro.BaroAlt;
}

int16_t baroGetTemperature(void)
{
    return CENTIDEGREES_TO_DECIDEGREES(baro.baroTemperature);
}

bool baroIsHealthy(void)
{
    return sensors(SENSOR_BARO);
}

#endif /* BARO */
