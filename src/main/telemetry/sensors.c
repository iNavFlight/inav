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

#include "platform.h"

#ifdef USE_TELEMETRY

#include "sensors.h"
#include "drivers/time.h"
#include "sensors/battery.h"
#include "navigation/navigation.h"
#include "flight/imu.h"
#include "common/crc.h"
#include "scheduler/scheduler.h"
#include "fc/runtime_config.h"
#include "sensors//acceleration.h"

static uint32_t getTupleHash(uint32_t a, uint32_t b)
{
    uint32_t data[2] = { a, b };
    return fnv_update(0x42424242, data, sizeof(data));
}

int telemetrySensorValue(sensor_id_e id)
{
    switch (id) {
        case TELEM_NONE:
            return 0;

        case TELEM_BATTERY:
            return millis();
        case TELEM_BATTERY_VOLTAGE:
            if (isBatteryVoltageConfigured()) {
                return getBatteryVoltage();
            }
            return 0;
        case TELEM_BATTERY_CURRENT:
            if (isAmperageConfigured()) {
                return getAmperage();
            }
            return 0;
        case TELEM_BATTERY_CONSUMPTION:
            return getMAhDrawn();
        case TELEM_BATTERY_CHARGE_LEVEL:
            return calculateBatteryPercentage();
        case TELEM_BATTERY_CELL_COUNT:
            if (isBatteryVoltageConfigured()) {
                return getBatteryCellCount();
            }
            return 0;
        case TELEM_BATTERY_CELL_VOLTAGE:
            return getBatteryAverageCellVoltage();
        case TELEM_BATTERY_CELL_VOLTAGES:
            return 0;
        case TELEM_BATTERY_LEGACY_FUEL:
            return millis();
            
        case TELEM_HEADING:
            return DECIDEGREES_TO_DEGREES(attitude.values.yaw);
        case TELEM_ALTITUDE:
            return (int)(getEstimatedActualPosition(Z)); // cm

#ifdef USE_GPS
        case TELEM_GPS_SATS:
            return gpsSol.numSat;
        case TELEM_GPS_HDOP:
            return gpsSol.hdop;
        case TELEM_GPS_COORD:
            return getTupleHash(gpsSol.llh.lat, gpsSol.llh.lon);
        case TELEM_GPS_ALTITUDE:
            return gpsSol.llh.alt;
        case TELEM_GPS_HEADING:
            return gpsSol.groundCourse;
        case TELEM_GPS_GROUNDSPEED:
            return gpsSol.groundSpeed;
        case TELEM_GPS_HOME_DISTANCE:
            return (int)GPS_distanceToHome;
        case TELEM_GPS_HOME_DIRECTION:
            return GPS_directionToHome;
#endif
        case TELEM_CPU_LOAD:
            return averageSystemLoadPercent * 10;

        case TELEM_FLIGHT_MODE:
            return (int)flightModeFlags;
        case TELEM_ARMING_FLAGS:
            return (int)armingFlags;

        case TELEM_ATTITUDE:
            return millis();
        case TELEM_ATTITUDE_PITCH:
            return attitude.values.pitch;
        case TELEM_ATTITUDE_ROLL:
            return attitude.values.roll;
        case TELEM_ATTITUDE_YAW:
            return attitude.values.yaw;

        case TELEM_ACCEL_X:
            return (int)(acc.accADCf[X] * 1000);
        case TELEM_ACCEL_Y:
            return (int)(acc.accADCf[Y] * 1000);
        case TELEM_ACCEL_Z:
            return (int)(acc.accADCf[Z] * 1000);
        default:
            return 0;
    }
}

bool telemetrySensorAllowed(sensor_id_e id)
{
    switch (id) {
        case TELEM_NONE:
            return true;
        case TELEM_BATTERY:
            return true;
        case TELEM_BATTERY_VOLTAGE:
            return isBatteryVoltageConfigured();
        case TELEM_BATTERY_CURRENT:
            return isAmperageConfigured();
        case TELEM_BATTERY_CONSUMPTION:
            return true;
        case TELEM_BATTERY_CHARGE_LEVEL:
            return true;
        case TELEM_BATTERY_CELL_COUNT:
            return isBatteryVoltageConfigured();
        case TELEM_BATTERY_CELL_VOLTAGE:
            return isBatteryVoltageConfigured();
        case TELEM_BATTERY_CELL_VOLTAGES:
            return 0;

        case TELEM_BATTERY_LEGACY_FUEL:
            return true;

        case TELEM_HEADING:
            return true;
        case TELEM_ALTITUDE:
            return sensors(SENSOR_BARO);

#ifdef USE_GPS
        case TELEM_GPS_SATS:
            return true;
        case TELEM_GPS_HDOP:
            return true;
        case TELEM_GPS_COORD:
            return true;
        case TELEM_GPS_ALTITUDE:
            return true;
        case TELEM_GPS_HEADING:
            return true;
        case TELEM_GPS_GROUNDSPEED:
            return true;
        case TELEM_GPS_HOME_DISTANCE:
            return true;
        case TELEM_GPS_HOME_DIRECTION:
            return true;
#endif

        case TELEM_CPU_LOAD:
            return true;

        case TELEM_FLIGHT_MODE:
            return true;
        case TELEM_ARMING_FLAGS:
            return true;

        case TELEM_ATTITUDE:
            return true;
        case TELEM_ATTITUDE_PITCH:
            return true;
        case TELEM_ATTITUDE_ROLL:
            return true;
        case TELEM_ATTITUDE_YAW:
            return true;

        case TELEM_ACCEL_X:
            return true;
        case TELEM_ACCEL_Y:
            return true;
        case TELEM_ACCEL_Z:
            return true;
        default:
            return false;
    }
}

////////////////////////////////////////////////////////////
#endif  // USE_TELEMETRY