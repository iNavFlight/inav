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
#include "drivers/time.h"

#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "sensors/barometer.h"
#include "sensors/sensors.h"

#include "io/beeper.h"

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
    BAROMETER_NEEDS_SAMPLES = 0,
    BAROMETER_NEEDS_CALCULATION
} barometerState_e;

uint32_t baroUpdate(void)
{
    static barometerState_e state = BAROMETER_NEEDS_SAMPLES;

#ifdef USE_SIMULATOR
    if (ARMING_FLAG(SIMULATOR_MODE_HITL)) {
        return 0;
    }
#endif

    switch (state) {
        default:
        case BAROMETER_NEEDS_SAMPLES:
            if (baro.dev.get_ut) {
                baro.dev.get_ut(&baro.dev);
            }
            if (baro.dev.start_up) {
                baro.dev.start_up(&baro.dev);
            }
            state = BAROMETER_NEEDS_CALCULATION;
            return baro.dev.up_delay;
        break;

        case BAROMETER_NEEDS_CALCULATION:
            if (baro.dev.get_up) {
                baro.dev.get_up(&baro.dev);
            }
            if (baro.dev.start_ut) {
                baro.dev.start_ut(&baro.dev);
            }
            //output: baro.baroPressure, baro.baroTemperature
            baro.dev.calculate(&baro.dev, &baro.baroPressure, &baro.baroTemperature);
            state = BAROMETER_NEEDS_SAMPLES;
            return baro.dev.ut_delay;
        break;
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
