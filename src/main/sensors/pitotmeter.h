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
#include "common/filter.h"
#include "common/calibration.h"

#include "drivers/pitotmeter.h"

typedef enum {
    PITOT_NONE = 0,
    PITOT_AUTODETECT = 1,
    PITOT_MS4525 = 2,
    PITOT_ADC = 3,
    PITOT_VIRTUAL = 4,
    PITOT_FAKE = 5,
} pitotSensor_e;

#define PITOT_MAX  PITOT_FAKE
#define PITOT_SAMPLE_COUNT_MAX   48

typedef struct pitotmeterConfig_s {
    uint8_t pitot_hardware;                 // Pitotmeter hardware to use
    uint16_t pitot_lpf_milli_hz;            // additional LPF to reduce pitot noise in [0.001Hz]
    float pitot_scale;                      // scale value
} pitotmeterConfig_t;

PG_DECLARE(pitotmeterConfig_t, pitotmeterConfig);

typedef struct pito_s {
    pitotDev_t dev;
    float airSpeed;

    zeroCalibrationScalar_t zeroCalibration;
    pt1Filter_t lpfState;
    timeUs_t lastMeasurementUs;
    timeMs_t lastSeenHealthyMs;

    float pressureZero;
    float pressure;
} pitot_t;

#ifdef USE_PITOT

#define AIR_DENSITY_SEA_LEVEL_15C   1.225f      // Air density at sea level and 15 degrees Celsius
#define P0                          101325.0f   // standard pressure [Pa]

extern pitot_t pitot;

bool pitotInit(void);
bool pitotIsCalibrationComplete(void);
void pitotStartCalibration(void);
void pitotUpdate(void);
int32_t pitotCalculateAirSpeed(void);
bool pitotIsHealthy(void);

#endif
