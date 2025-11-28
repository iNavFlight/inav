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

#include "sensors/battery.h"
#include "sensors//acceleration.h"
#include "sensors/esc_sensor.h"
#include "sensors/temperature.h"

#include "drivers/time.h"

#include "navigation/navigation.h"

#include "flight/imu.h"
#include "flight/mixer.h"

#include "common/crc.h"

#include "scheduler/scheduler.h"

#include "fc/runtime_config.h"


static uint32_t getTupleHash(uint32_t a, uint32_t b)
{
    uint32_t data[2] = { a, b };
    return fnv_update(0x42424242, data, sizeof(data));
}

int telemetrySensorValue(sensor_id_e id)
{
    switch (id) {
        /////////////////
        //// BATTERY ////
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

        /////////////////
        //// GPS     ////
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
        /////////////////
        //// SYSTEM  ////
        case TELEM_FLIGHT_MODE:
            return (int)flightModeFlags;
        case TELEM_ARMING_FLAGS:
            return (int)armingFlags;
        case TELEM_CPU_LOAD:
            return averageSystemLoadPercent * 10;

        /////////////////
        //// MOVING  ////
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

#ifdef USE_ESC_SENSOR
        /////////////////
        //// ESC  ////
        case TELEM_ESC_RPM:
            return millis();
        case TELEM_ESC1_RPM:
            return getMotorCount() > 0 ? (int)getEscTelemetry(0)->rpm : 0;
        case TELEM_ESC2_RPM:
            return getMotorCount() > 1 ? (int)getEscTelemetry(1)->rpm : 0;
        case TELEM_ESC3_RPM:
            return getMotorCount() > 2 ? (int)getEscTelemetry(2)->rpm : 0;
        case TELEM_ESC4_RPM:
            return getMotorCount() > 3 ? (int)getEscTelemetry(3)->rpm : 0;
#endif

#ifdef USE_TEMPERATURE_SENSOR
        case TELEM_ESC1_TEMPERATURE:
            return getMotorCount() > 0 ? (int)(getEscTelemetry(0)->temperature * 10) : 0;
        case TELEM_ESC2_TEMPERATURE:
            return getMotorCount() > 1 ? (int)(getEscTelemetry(1)->temperature * 10) : 0;
        case TELEM_ESC3_TEMPERATURE:
            return getMotorCount() > 2 ? (int)(getEscTelemetry(2)->temperature * 10) : 0;
        case TELEM_ESC4_TEMPERATURE:
            return getMotorCount() > 3 ? (int)(getEscTelemetry(3)->temperature * 10) : 0;
#endif
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
            return isAmperageConfigured();
        case TELEM_BATTERY_CHARGE_LEVEL:
            return isBatteryVoltageConfigured();
        case TELEM_BATTERY_CELL_COUNT:
            return isBatteryVoltageConfigured();
        case TELEM_BATTERY_CELL_VOLTAGE:
            return isBatteryVoltageConfigured();
        case TELEM_BATTERY_CELL_VOLTAGES:
            return isBatteryVoltageConfigured();

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

#ifdef USE_ESC_SENSOR
        case TELEM_ESC_RPM:
            return STATE(ESC_SENSOR_ENABLED);
        case TELEM_ESC1_RPM:
            return STATE(ESC_SENSOR_ENABLED) && getMotorCount() > 0;
        case TELEM_ESC2_RPM:
            return STATE(ESC_SENSOR_ENABLED) && getMotorCount() > 1;
        case TELEM_ESC3_RPM:
            return STATE(ESC_SENSOR_ENABLED) && getMotorCount() > 2;
        case TELEM_ESC4_RPM:
            return STATE(ESC_SENSOR_ENABLED) && getMotorCount() > 3;
#endif

#ifdef USE_TEMPERATURE_SENSOR
        case TELEM_ESC_TEMPERATURE:
            return STATE(ESC_SENSOR_ENABLED);
        case TELEM_ESC1_TEMPERATURE:
            return STATE(ESC_SENSOR_ENABLED) && getMotorCount() > 0;
        case TELEM_ESC2_TEMPERATURE:
            return STATE(ESC_SENSOR_ENABLED) && getMotorCount() > 1;
        case TELEM_ESC3_TEMPERATURE:
            return STATE(ESC_SENSOR_ENABLED) && getMotorCount() > 2;
        case TELEM_ESC4_TEMPERATURE:
            return STATE(ESC_SENSOR_ENABLED) && getMotorCount() > 3;
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