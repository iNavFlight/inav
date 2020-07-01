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

#define TEMPERATURE_LABEL_LEN 4
#define MAX_TEMP_SENSORS 8

#define TEMPERATURE_INVALID_VALUE -1250

typedef enum {
    TEMP_SENSOR_NONE = 0,
    TEMP_SENSOR_LM75,
    TEMP_SENSOR_DS18B20
} tempSensorType_e;

typedef struct {
    tempSensorType_e type;
    uint64_t address;
    int16_t alarm_min;
    int16_t alarm_max;
    uint8_t osdSymbol;
    char label[TEMPERATURE_LABEL_LEN];
} tempSensorConfig_t;

PG_DECLARE_ARRAY(tempSensorConfig_t, MAX_TEMP_SENSORS, tempSensorConfig);

// Temperature is returned in degC*10
bool getIMUTemperature(int16_t *temperature);
bool getBaroTemperature(int16_t *temperature);
void temperatureUpdate(void);

#ifdef USE_TEMPERATURE_SENSOR
void temperatureInit(void);

bool getSensorTemperature(uint8_t sensorIndex, int16_t *temperature);

void tempSensorAddressToString(uint64_t address, char *hex_address);
bool tempSensorStringToAddress(const char *hex_address, uint64_t *address);
#endif

void resetTempSensorConfig(void);
