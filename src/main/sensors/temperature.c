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

#include "stdbool.h"
#include "stdint.h"

#include "platform.h"

#include "common/maths.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "fc/runtime_config.h"

#include "sensors/sensors.h"
#include "sensors/temperature.h"
#include "sensors/gyro.h"
#include "sensors/barometer.h"

static bool     tempSensorValid[TEMP_COUNT];
static int16_t  tempSensorValue[TEMP_COUNT];

bool isTemperatureSensorValid(tempSensor_e sensor)
{
    return tempSensorValid[sensor];
}

int16_t getTemperature(tempSensor_e sensor)
{
    return tempSensorValue[sensor];
}

void temperatureUpdate(void)
{
    // TEMP_GYRO: Update gyro temperature in decidegrees
    if (gyroReadTemperature()) {
        tempSensorValid[TEMP_GYRO] = true;
        tempSensorValue[TEMP_GYRO] = gyroGetTemperature();
    }
    // TEMP_BARO: Update baro temperature in decidegrees
    if(sensors(SENSOR_BARO)){
        tempSensorValid[TEMP_BARO] = true;
        tempSensorValue[TEMP_BARO] = baroGetTemperature();
    }
}
