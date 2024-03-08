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

#ifdef USE_HARDWARE_REVISION_DETECTION
#include "hardware_revision.h"
#endif

baro_t baro;                        // barometer access functions
baro_t multiBaro[2];

#ifdef USE_BARO

PG_REGISTER_WITH_RESET_TEMPLATE(barometerConfig_t, barometerConfig, PG_BAROMETER_CONFIG, 4);

PG_RESET_TEMPLATE(barometerConfig_t, barometerConfig,
    .baro_hardware = SETTING_BARO_HARDWARE_DEFAULT,
    .baro_calibration_tolerance = SETTING_BARO_CAL_TOLERANCE_DEFAULT
);

#ifndef USE_BARO_MULTI
static zeroCalibrationScalar_t zeroCalibration;
static float baroGroundAltitude = 0;
static float baroGroundPressure = 101325.0f; // 101325 pascal, 1 standard atmosphere
#else
PG_REGISTER_WITH_RESET_TEMPLATE(barometerMultiConfig_t, barometerMultiConfig, PG_MULTI_BAROMETER_CONFIG, 4);

PG_RESET_TEMPLATE(barometerMultiConfig_t, barometerMultiConfig,
    .multi_baro_count = SETTING_MULTI_BARO_COUNT_DEFAULT,
    .multi_baro_hardware_1 = SETTING_MULTI_BARO_HARDWARE_1_DEFAULT,
    .multi_baro_calibration_tolerance_1 = SETTING_MULTI_BARO_CAL_TOLERANCE_1_DEFAULT,
    .multi_baro_hardware_2 = SETTING_MULTI_BARO_HARDWARE_2_DEFAULT,
    .multi_baro_calibration_tolerance_2 = SETTING_MULTI_BARO_CAL_TOLERANCE_2_DEFAULT
);

static zeroCalibrationScalar_t multiZeroCalibration[2];
static float baroMultiGroundAltitude[2] = {0, 0};
static float baroMultiGroundPressure[2] = {101325.0f, 101325.0f}; // 101325 pascal, 1 standard atmosphere
#endif

bool baroDetect(baroDev_t *dev, baroSensor_e baroHardwareToUse, int baro_index)
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

    if(baro_index == BARO_FAKE) {
        detectedSensors[SENSOR_INDEX_BARO] = baroHardware;
    } else if (baro_index == SENSOR_MULTI_INDEX_BARO_FIRST) {
        // Need to implement composite hardware type here for to assign into sensors
        // instead of using first one
        detectedSensors[SENSOR_INDEX_BARO] = baroHardware;
        detectedMultiSensors[SENSOR_MULTI_INDEX_BARO_FIRST] = baroHardware;
    } else if (baro_index == SENSOR_MULTI_INDEX_BARO_SECOND) {
        detectedMultiSensors[SENSOR_MULTI_INDEX_BARO_SECOND] = baroHardware;
    }

    sensorsSet(SENSOR_BARO);
    return true;
}

bool baroInit(void)
{
#ifdef USE_BARO_MULTI
    uint8_t detectedBaroHardware;
    for(int i = 0; i < barometerMultiConfig()->multi_baro_count; i++) {
        if(i == 0) {
            detectedBaroHardware = barometerMultiConfig()->multi_baro_hardware_1;
        }
        if(i == 1) {
            detectedBaroHardware = barometerMultiConfig()->multi_baro_hardware_2;
        }
        if(!baroDetect(&multiBaro[i].dev, detectedBaroHardware, i)) {
            return false;
        }
    }
    baro.dev = multiBaro[0].dev;
    return true;
#else    
    if (!baroDetect(&baro.dev, barometerConfig()->baro_hardware, BARO_FAKE)) {
        return false;
    }
    return true;
#endif   
}

typedef enum {
    BAROMETER_NEEDS_SAMPLES = 0,
    BAROMETER_NEEDS_CALCULATION
} barometerState_e;

uint32_t baroUpdate(void)
{
#ifdef USE_BARO_MULTI
    int hardwareCount = barometerMultiConfig()->multi_baro_count;
    int delays[4] = {0, 0, 0, 0};

    static barometerState_e multiState[] = {
        BAROMETER_NEEDS_SAMPLES,
        BAROMETER_NEEDS_SAMPLES
    }; //Here is multi state with maximum length of config value
    
#else
    static barometerState_e state = BAROMETER_NEEDS_SAMPLES;
#endif

#ifdef USE_SIMULATOR
    if (ARMING_FLAG(SIMULATOR_MODE_HITL)) {
        return 0;
    }
#endif

#ifdef USE_BARO_MULTI
    for(int i = 0; i < hardwareCount; i++) {
        switch (multiState[i]) {
            default:
            case BAROMETER_NEEDS_SAMPLES:
                
                if (multiBaro[i].dev.get_ut) {
                    multiBaro[i].dev.get_ut(&multiBaro[i].dev);
                }
                if (multiBaro[i].dev.start_up) {
                    multiBaro[i].dev.start_up(&multiBaro[i].dev);
                }
                multiState[i] = BAROMETER_NEEDS_CALCULATION;
                delays[i] = multiBaro[i].dev.up_delay;
                delays[i+2] = 0;
            break;

            case BAROMETER_NEEDS_CALCULATION:
                if (multiBaro[i].dev.get_up) {
                    multiBaro[i].dev.get_up(&multiBaro[i].dev);
                }
                if (multiBaro[i].dev.start_ut) {
                    multiBaro[i].dev.start_ut(&multiBaro[i].dev);
                }
                //output: baro.baroPressure, baro.baroTemperature
                multiBaro[i].dev.calculate(&multiBaro[i].dev, &multiBaro[i].baroPressure, &multiBaro[i].baroTemperature);
                multiState[i] = BAROMETER_NEEDS_SAMPLES;
                delays[i] = 0;
                delays[i+2] = multiBaro[i].dev.ut_delay;
            break;
        }
    }

    // return longest delay, must be refactored
    if(delays[0] != 0 || delays[1] != 0) {
        return (delays[0] >=  delays[1]) ? delays[0] : delays[1];
    } else if (delays[2] != 0 || delays[3] != 0) {
        return (delays[2] >=  delays[3]) ? delays[2] : delays[3];
    } else {
        return 0;
    }

#else
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
#endif
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
#ifdef USE_BARO_MULTI    
    return 
        zeroCalibrationIsCompleteS(&multiZeroCalibration[0]) && 
        zeroCalibrationIsSuccessfulS(&multiZeroCalibration[0]) && 
        zeroCalibrationIsCompleteS(&multiZeroCalibration[1]) &&
        zeroCalibrationIsSuccessfulS(&multiZeroCalibration[1]);
#else
    return zeroCalibrationIsCompleteS(&zeroCalibration) && zeroCalibrationIsSuccessfulS(&zeroCalibration);
#endif    
}

void baroStartCalibration(void)
{
#ifdef USE_BARO_MULTI    
    const float acceptedPressureVarianceFirst = (101325.0f - altitudeToPressure(barometerMultiConfig()->multi_baro_calibration_tolerance_1)); // max 30cm deviation during calibration (at sea level)
    const float acceptedPressureVarianceSecond = (101325.0f - altitudeToPressure(barometerMultiConfig()->multi_baro_calibration_tolerance_2)); // max 30cm deviation during calibration (at sea level)
    zeroCalibrationStartS(&multiZeroCalibration[0], CALIBRATING_BARO_TIME_MS, acceptedPressureVarianceFirst, false);
    zeroCalibrationStartS(&multiZeroCalibration[1], CALIBRATING_BARO_TIME_MS, acceptedPressureVarianceSecond, false);
#else
    const float acceptedPressureVariance = (101325.0f - altitudeToPressure(barometerConfig()->baro_calibration_tolerance)); // max 30cm deviation during calibration (at sea level)
    zeroCalibrationStartS(&zeroCalibration, CALIBRATING_BARO_TIME_MS, acceptedPressureVariance, false);
#endif    
}


int32_t baroCalculateAltitude(void)
{
    #ifdef USE_BARO_MULTI
        int32_t calcBaroAlt = 0;

        if (!baroIsCalibrationComplete()) {
            for(int i = 0; i < barometerMultiConfig()->multi_baro_count; i++) {
                zeroCalibrationAddValueS(&multiZeroCalibration[i], multiBaro[i].baroPressure);

                if (zeroCalibrationIsCompleteS(&multiZeroCalibration[i])) {
                    zeroCalibrationGetZeroS(&multiZeroCalibration[i], &baroMultiGroundPressure[i]);
                    baroMultiGroundAltitude[i] = pressureToAltitude(baroMultiGroundPressure[i]);
                    LOG_DEBUG( SYSTEM, "Barometer calibration complete (%d)", (int)lrintf(baroMultiGroundAltitude[i]));
                }

                multiBaro[i].BaroAlt = 0;
            }
        } else {
            // calculates height from ground via baro readings
            
            for (int i = 0; i < barometerMultiConfig()->multi_baro_count; i++) {
                multiBaro[i].BaroAlt = pressureToAltitude(multiBaro[i].baroPressure) - baroMultiGroundAltitude[i];
            }            
        }
        for(int i = 0; i < barometerMultiConfig()->multi_baro_count; i++) {
            calcBaroAlt += multiBaro[i].BaroAlt;
        }
        return calcBaroAlt / (int)barometerMultiConfig()->multi_baro_count;
    #else
        if (!baroIsCalibrationComplete()) {
            zeroCalibrationAddValueS(&zeroCalibration, baro.baroPressure);

            if (zeroCalibrationIsCompleteS(&zeroCalibration)) {
                zeroCalibrationGetZeroS(&zeroCalibration, &baroGroundPressure);
                baroGroundAltitude = pressureToAltitude(baroGroundPressure);
                LOG_DEBUG(BARO, "Barometer calibration complete (%d)", (int)lrintf(baroGroundAltitude));
            }
            baro.BaroAlt = 0;
        } else {
            // calculates height from ground via baro readings
            baro.BaroAlt = pressureToAltitude(baro.baroPressure) - baroGroundAltitude;
        }
        return baro.BaroAlt;
   #endif
}

int32_t baroGetLatestAltitude(void)
{
    #ifdef USE_BARO_MULTI
        int32_t calcMultiBaroAlt = 0;
        for(int i = 0; i < barometerMultiConfig()->multi_baro_count; i++) {
            calcMultiBaroAlt += multiBaro[i].BaroAlt;
        }
        LOG_DEBUG( SYSTEM, "MultiBaro:latestAlt %u", (unsigned int)(calcMultiBaroAlt) );

        return calcMultiBaroAlt / (int)barometerMultiConfig()->multi_baro_count;

    #else
        return baro.BaroAlt;
    #endif
}

int32_t baroMultiGetLatestAltitude(uint8_t baroIndex)
{   
    #ifdef USE_BARO_MULTI
        return multiBaro[baroIndex].BaroAlt;
    #else
        UNUSED(baroIndex);
        return 0;
    #endif
}

int16_t baroGetTemperature(void)
{   
#ifdef USE_BARO_MULTI
    int32_t calcMultiBaroTemperature = 0;
    for(int i = 0; i < barometerMultiConfig()->multi_baro_count; i++) {
        calcMultiBaroTemperature += CENTIDEGREES_TO_DECIDEGREES(multiBaro[i].baroTemperature);
    }
    return calcMultiBaroTemperature / (int)barometerMultiConfig()->multi_baro_count;
#else    
    return CENTIDEGREES_TO_DECIDEGREES(baro.baroTemperature);
#endif
}

int16_t baroMultiGetTemperature(uint8_t baroIndex)
{   
#ifdef USE_BARO_MULTI
    return CENTIDEGREES_TO_DECIDEGREES(multiBaro[baroIndex].baroTemperature);
#else    
    UNUSED(baroIndex);
    return 0;
#endif
}

bool baroIsHealthy(void)
{
    return sensors(SENSOR_BARO);
}

#endif /* BARO */
