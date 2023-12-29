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

#pragma once

#include "config/parameter_group.h"

#include "drivers/barometer/barometer.h"

typedef enum {
    BARO_NONE = 0,
    BARO_AUTODETECT = 1,
    BARO_BMP085 = 2,
    BARO_MS5611 = 3,
    BARO_BMP280 = 4,
    BARO_MS5607 = 5,
    BARO_LPS25H = 6,
    BARO_SPL06  = 7,
    BARO_BMP388 = 8,
    BARO_DPS310 = 9,
    BARO_B2SMPB = 10,
    BARO_MSP    = 11,
    BARO_FAKE   = 12,
    BARO_MAX    = BARO_FAKE
} baroSensor_e;

typedef struct baro_s {
    baroDev_t dev;
    int32_t BaroAlt;
    int32_t baroTemperature;            // Use temperature for telemetry
    int32_t baroPressure;               // Use pressure for telemetry
} baro_t;

extern baro_t baro;
extern baro_t multiBaro[2];

#ifdef USE_BARO

typedef struct barometerConfig_s {
    uint8_t baro_hardware;                  // Barometer hardware to use
    uint16_t baro_calibration_tolerance;    // Baro calibration tolerance (cm at sea level)
} barometerConfig_t;

PG_DECLARE(barometerConfig_t, barometerConfig);

typedef struct barometerMultiConfig_s {
    uint8_t multi_baro_count;                       // Total Barometers count to use
    uint8_t multi_baro_hardware_1;                  // First Barometer hardware to use
    uint16_t multi_baro_calibration_tolerance_1;    // First Baro calibration tolerance (cm at sea level)
    uint8_t multi_baro_hardware_2;                  // Second Barometer hardware to use
    uint16_t multi_baro_calibration_tolerance_2;    // Second Baro calibration tolerance (cm at sea level)
} barometerMultiConfig_t;

PG_DECLARE(barometerMultiConfig_t, barometerMultiConfig);

bool baroInit(void);
bool baroIsCalibrationComplete(void);
void baroStartCalibration(void);
uint32_t baroUpdate(void);
int32_t baroCalculateAltitude(void);
int32_t baroGetLatestAltitude(void);
int16_t baroGetTemperature(void);
int32_t baroMultiGetLatestAltitude(uint8_t baroIndex);
int16_t baroMultiGetTemperature(uint8_t baroIndex);
bool baroIsHealthy(void);

#if defined(SITL_BUILD)
float altitudeToPressure(const float altCm);
#endif

#endif
