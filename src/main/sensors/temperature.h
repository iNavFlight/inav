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

typedef enum {
    TEMP_GYRO = 0,
    TEMP_BARO = 1,
#ifdef USE_TEMPERATURE_SENSOR
    TEMP_LM75 = 2,
#endif
    TEMP_COUNT
} tempSensor_e;

// Temperature is returned in degC*10
int16_t getTemperature(tempSensor_e sensor);
float getCurrentTemperature(void);
tempSensor_e getCurrentTemperatureSensorUsed(void);
void temperatureUpdate(void);

#ifdef USE_TEMPERATURE_SENSOR
void temperatureInit(void);
#endif
