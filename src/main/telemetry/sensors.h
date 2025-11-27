/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <stddef.h>
#include <stdbool.h>

#include "common/utils.h"

typedef enum
{
    TELEM_NONE                          = 0,

    TELEM_HEARTBEAT                     = 1,

    TELEM_BATTERY                       = 2,
    TELEM_BATTERY_VOLTAGE               = 3,
    TELEM_BATTERY_CURRENT               = 4,
    TELEM_BATTERY_CONSUMPTION           = 5,
    TELEM_BATTERY_CHARGE_LEVEL          = 6,
    TELEM_BATTERY_CELL_COUNT            = 7,
    TELEM_BATTERY_CELL_VOLTAGE          = 8,
    TELEM_BATTERY_CELL_VOLTAGES         = 9,

    TELEM_BATTERY_LEGACY_FUEL           = 10,

    TELEM_HEADING                       = 11,
    TELEM_ALTITUDE                      = 12,
    TELEM_VARIOMETER                    = 13,

    TELEM_ATTITUDE                      = 14,
    TELEM_ATTITUDE_PITCH                = 15,
    TELEM_ATTITUDE_ROLL                 = 16,
    TELEM_ATTITUDE_YAW                  = 17,

    TELEM_ACCEL_X                       = 18,
    TELEM_ACCEL_Y                       = 19,
    TELEM_ACCEL_Z                       = 20,

    TELEM_GPS                           = 21,
    TELEM_GPS_SATS                      = 22,
    TELEM_GPS_HDOP                      = 23,
    TELEM_GPS_COORD                     = 24,
    TELEM_GPS_ALTITUDE                  = 25,
    TELEM_GPS_HEADING                   = 26,
    TELEM_GPS_GROUNDSPEED               = 27,
    TELEM_GPS_HOME_DISTANCE             = 28,
    TELEM_GPS_HOME_DIRECTION            = 29,

    TELEM_CPU_LOAD                      = 30,

    TELEM_FLIGHT_MODE                   = 31,
    TELEM_ARMING_FLAGS                  = 32,
    TELEM_ARMING_DISABLE_FLAGS          = 33,

    TELEM_SENSOR_COUNT

} sensor_id_e;

typedef struct telemetrySensor_s telemetrySensor_t;

typedef void (*telemetryEncode_f)(telemetrySensor_t *sensor, void *ptr);

struct telemetrySensor_s {

    uint16_t                index;

    uint16_t                sensor_id;
    uint32_t                app_id;

    uint16_t                fast_weight;
    uint16_t                slow_weight;
    uint16_t                fast_interval;
    uint16_t                slow_interval;

    int                     ratio_num;
    int                     ratio_den;

    int                     value;

    bool                    active;
    bool                    update;

    int                     bucket;

    telemetryEncode_f       encode;
};

int telemetrySensorValue(sensor_id_e id);
bool telemetrySensorActive(sensor_id_e id);

void legacySensorInit(void);

bool telemetrySensorAllowed(sensor_id_e id);