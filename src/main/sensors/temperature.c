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

#include "build/debug.h"

#include "common/maths.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/logging.h"
#include "drivers/temperature/temperature.h"
#include "drivers/temperature/lm75.h"

#include "fc/runtime_config.h"

#include "sensors/sensors.h"
#include "sensors/temperature.h"
#include "sensors/gyro.h"
#include "sensors/barometer.h"

static int16_t  tempSensorValue[TEMP_COUNT];
static tempSensor_e tempSensorValid;

#ifdef USE_TEMPERATURE_SENSOR

static bool lm75Detected = false;
temperatureDev_t temperatureDev;

void temperatureInit(void)
{
    addBootlogEvent2(BOOT_EVENT_TEMP_SENSOR_DETECTION, BOOT_EVENT_FLAGS_NONE);
    lm75Detected = lm75Detect(&temperatureDev);
}
#endif

int16_t getTemperature(tempSensor_e sensor)
{
    return tempSensorValue[sensor];
}

float getCurrentTemperature(void)
{   //returns current temperature in degrees celsius
    return tempSensorValue[tempSensorValid]/10.0f;
}

tempSensor_e getCurrentTemperatureSensorUsed(void) {
    return tempSensorValid;
}

void temperatureUpdate(void)
{
    // TEMP_GYRO: Update gyro temperature in decidegrees
    if (gyroReadTemperature()) {
        tempSensorValue[TEMP_GYRO] = gyroGetTemperature();
        tempSensorValid = TEMP_GYRO;
    }

    #if defined(USE_BARO)
    // TEMP_BARO: Update baro temperature in decidegrees
    if(sensors(SENSOR_BARO)){
        tempSensorValue[TEMP_BARO] = baroGetTemperature();
        tempSensorValid = TEMP_BARO;
    }
    #endif

    #ifdef USE_TEMPERATURE_SENSOR
    if (lm75Detected && temperatureDev.read(&temperatureDev, &tempSensorValue[TEMP_LM75])) {
        tempSensorValid = TEMP_LM75;
    }
    #endif
}
