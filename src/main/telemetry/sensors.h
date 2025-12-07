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
    /////////////////////////////////////////
    ////// SPECIAL //////////////////////////
    TELEM_NONE                          = 0,
    TELEM_HEARTBEAT                     = 1,

    /////////////////////////////////////////
    ////// BATERY //////////////////////////
    TELEM_BATTERY                       = 2, // start from settings
    TELEM_BATTERY_VOLTAGE               = 3,
    TELEM_BATTERY_CURRENT               = 4,
    TELEM_BATTERY_CONSUMPTION           = 5,
    TELEM_BATTERY_CHARGE_LEVEL          = 6,
    TELEM_BATTERY_CELL_COUNT            = 7,
    TELEM_BATTERY_CELL_VOLTAGE          = 8,
    TELEM_BATTERY_CELL_VOLTAGES         = 9,

    TELEM_BATTERY_FUEL                  = 10,

    /////////////////////////////////////////
    ////// MOVING //////////////////////////
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

    /////////////////////////////////////////
    ////// GPS     //////////////////////////
    TELEM_GPS                           = 21,
    TELEM_GPS_SATS                      = 22,
    TELEM_GPS_HDOP                      = 23,
    TELEM_GPS_COORD                     = 24,
    TELEM_GPS_ALTITUDE                  = 25,
    TELEM_GPS_HEADING                   = 26,
    TELEM_GPS_GROUNDSPEED               = 27,
    TELEM_GPS_HOME_DISTANCE             = 28,
    TELEM_GPS_HOME_DIRECTION            = 29,
    TELEM_GPS_AZIMUTH                   = 30,

    /////////////////////////////////////////
    ////// ESC     //////////////////////////
    TELEM_ESC_RPM                       = 31,
    TELEM_ESC_TEMPERATURE               = 32,

    TELEM_ESC1_RPM                      = 33,
    TELEM_ESC1_TEMPERATURE              = 34,

    TELEM_ESC2_RPM                      = 35,
    TELEM_ESC2_TEMPERATURE              = 36,

    TELEM_ESC3_RPM                      = 37,
    TELEM_ESC3_TEMPERATURE              = 38,

    TELEM_ESC4_RPM                      = 39,
    TELEM_ESC4_TEMPERATURE              = 40,

    TELEM_TEMPERATURE                   = 41,

    /////////////////////////////////////////
    ////// SYSTEM  //////////////////////////
    TELEM_CPU_LOAD                      = 42,
    TELEM_FLIGHT_MODE                   = 43,
    TELEM_PROFILES                      = 44,
    TELEM_ARMING_FLAGS                  = 45,

/////////////////////////////////////////
////// LEGACY SMARTPORT  ////////////////
    TELEM_LEGACY_VFAS                   = 46,
    TELEM_LEGACY_CURRENT                = 47,
    TELEM_LEGACY_ALTITUDE               = 48,
    TELEM_LEGACY_FUEL                   = 49,
    TELEM_LEGACY_VARIO                  = 50,
    TELEM_LEGACY_HEADING                = 51,
    TELEM_LEGACY_PITCH                  = 52,
    TELEM_LEGACY_ROLL                   = 53,
    TELEM_LEGACY_ACCX                   = 54,
    TELEM_LEGACY_ACCY                   = 55,
    TELEM_LEGACY_ACCZ                   = 56,
    TELEM_LEGACY_MODES                  = 57,

    TELEM_LEGACY_GNSS                   = 58,
    TELEM_LEGACY_SPEED                  = 59,
    TELEM_LEGACY_LAT                    = 60,
    TELEM_LEGACY_LON                    = 61,
    TELEM_LEGACY_HOME_DIST              = 62,
    TELEM_LEGACY_GPS_ALT                = 63,
    TELEM_LEGACY_FPV                    = 64,
    TELEM_LEGACY_AZIMUTH                = 65,

    TELEM_LEGACY_A4                     = 66,
    TELEM_LEGACY_ASPD                   = 67,

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
bool telemetrySensorAllowed(sensor_id_e id);