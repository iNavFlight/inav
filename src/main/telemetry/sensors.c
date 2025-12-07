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

#if defined(USE_TELEMETRY)

#include "sensors.h"
#include "telemetry.h"

#include "sensors/battery.h"
#include "sensors//acceleration.h"
#include "sensors/esc_sensor.h"
#include "sensors/temperature.h"
#include "sensors/pitotmeter.h"

#include "drivers/time.h"

#include "navigation/navigation.h"

#include "flight/imu.h"
#include "flight/mixer.h"

#include "common/crc.h"
#include "fc/config.h"

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

        case TELEM_NONE:
            return 0;

        /////////////////////////////////////////
        ////// BATERY //////////////////////////
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
        case TELEM_BATTERY_FUEL:
            return millis();

        /////////////////////////////////////////
        ////// MOVING //////////////////////////
        case TELEM_HEADING:
            return DECIDEGREES_TO_DEGREES(attitude.values.yaw);
        case TELEM_ALTITUDE:
            return (int)(getEstimatedActualPosition(Z)); // cm
        case TELEM_VARIOMETER:
            return (int)getEstimatedActualVelocity(Z);
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

        /////////////////////////////////////////
        ////// GPS     //////////////////////////
#ifdef USE_GPS
        case TELEM_GPS:
            return (int)millis();
        case TELEM_GPS_SATS:
            return gpsSol.numSat;
        case TELEM_GPS_HDOP:
            return gpsSol.hdop;
        case TELEM_GPS_COORD:
            return (int)getTupleHash(gpsSol.llh.lat, gpsSol.llh.lon);
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
        case TELEM_GPS_AZIMUTH:
            return ((GPS_directionToHome < 0 ? GPS_directionToHome + 360 : GPS_directionToHome) + 180) % 360;
#endif

#ifdef USE_ESC_SENSOR
        /////////////////////////////////////////
        ////// ESC     //////////////////////////
        case TELEM_ESC_RPM:
            return (int)millis();
        case TELEM_ESC1_RPM:
            return getMotorCount() > 0 ? (int)getEscTelemetry(0)->rpm : 0;
        case TELEM_ESC2_RPM:
            return getMotorCount() > 1 ? (int)getEscTelemetry(1)->rpm : 0;
        case TELEM_ESC3_RPM:
            return getMotorCount() > 2 ? (int)getEscTelemetry(2)->rpm : 0;
        case TELEM_ESC4_RPM:
            return getMotorCount() > 3 ? (int)getEscTelemetry(3)->rpm : 0;


        case TELEM_ESC_TEMPERATURE:
            return (int)millis();
        case TELEM_ESC1_TEMPERATURE:
            return getMotorCount() > 0 ? (int)(getEscTelemetry(0)->temperature * 10) : 0;
        case TELEM_ESC2_TEMPERATURE:
            return getMotorCount() > 1 ? (int)(getEscTelemetry(1)->temperature * 10) : 0;
        case TELEM_ESC3_TEMPERATURE:
            return getMotorCount() > 2 ? (int)(getEscTelemetry(2)->temperature * 10) : 0;
        case TELEM_ESC4_TEMPERATURE:
            return getMotorCount() > 3 ? (int)(getEscTelemetry(3)->temperature * 10) : 0;
#endif


        /////////////////////////////////////////
        ////// SYSTEM  //////////////////////////
        case TELEM_FLIGHT_MODE:
            return (int)flightModeFlags;
        case TELEM_PROFILES:
            return ((getConfigBatteryProfile() & 0xF) << 8) |
                   ((getConfigMixerProfile() & 0xF) << 4) |
                   ((getConfigProfile() & 0xF));
        case TELEM_ARMING_FLAGS:
            return (int)armingFlags;
        case TELEM_CPU_LOAD:
            return averageSystemLoadPercent * 10;

        ////////////////////////////////////////
        ////// LEGACY SMARTPORT  ////////////////
        case TELEM_LEGACY_VFAS:
            FALLTHROUGH;
        case TELEM_LEGACY_CURRENT:
            FALLTHROUGH;
        case TELEM_LEGACY_ALTITUDE:
            FALLTHROUGH;
        case TELEM_LEGACY_FUEL:
            FALLTHROUGH;
        case TELEM_LEGACY_VARIO:
            FALLTHROUGH;
        case TELEM_LEGACY_HEADING:
            FALLTHROUGH;
        case TELEM_LEGACY_PITCH:
            FALLTHROUGH;
        case TELEM_LEGACY_ROLL:
            FALLTHROUGH;
        case TELEM_LEGACY_ACCX:
            FALLTHROUGH;
        case TELEM_LEGACY_ACCY:
            FALLTHROUGH;
        case TELEM_LEGACY_ACCZ:
            FALLTHROUGH;
        case TELEM_LEGACY_MODES:
            FALLTHROUGH;
        case TELEM_LEGACY_GNSS:
            FALLTHROUGH;
        case TELEM_LEGACY_SPEED:
            FALLTHROUGH;
        case TELEM_LEGACY_LAT:
            FALLTHROUGH;
        case TELEM_LEGACY_LON:
            FALLTHROUGH;
        case TELEM_LEGACY_HOME_DIST:
            FALLTHROUGH;
        case TELEM_LEGACY_GPS_ALT:
            FALLTHROUGH;
        case TELEM_LEGACY_FPV:
            FALLTHROUGH;
        case TELEM_LEGACY_AZIMUTH:
            FALLTHROUGH;
        case TELEM_LEGACY_A4:
            FALLTHROUGH;
        case TELEM_LEGACY_ASPD:
            return millis();
        default:
            return 0;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_GPS
bool shouldSendGpsData(void) {
    // We send GPS data if the GPS is configured and we have a fix
    // or the craft has never been armed yet. This way if GPS stops working
    // while in flight, the user will easily notice because the sensor will stop
    // updating.
    return feature(FEATURE_GPS) && (STATE(GPS_FIX)
#ifdef USE_GPS_FIX_ESTIMATION
        || STATE(GPS_ESTIMATED_FIX)
#endif
        || !ARMING_FLAG(WAS_EVER_ARMED));
}
#endif

bool shouldSendDataForMotorIndex(uint8_t motorIndex)
{
#ifdef USE_ESC_SENSOR
    return STATE(ESC_SENSOR_ENABLED) && getMotorCount() > motorIndex;
#else
    UNUSED(motorIndex);
    return false;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_CUSTOM_TELEMETRY
bool telemetrySensorActive(sensor_id_e sid)
{
    // system sensors are always active
    if(sid == TELEM_NONE || sid == TELEM_HEARTBEAT) {
        return true;
    }

    for (int i = 0; i < TELEM_SENSOR_SLOT_COUNT; ++i) {
        if (telemetryConfig()->telemetry_sensors[i] == sid) {
            return true;
        }
    }

    return false;
}
#endif

bool telemetrySensorAllowed(sensor_id_e id)
{
    switch (id) {
        case TELEM_NONE:
            return true;

        case TELEM_HEARTBEAT:
            return true;

        /////////////////////////////////////////
        ////// BATERY //////////////////////////
        case TELEM_BATTERY:
            return true;
        case TELEM_BATTERY_VOLTAGE:
            return isBatteryVoltageConfigured();
        case TELEM_BATTERY_CURRENT:
            return isAmperageConfigured();
        case TELEM_BATTERY_CONSUMPTION:
            return isAmperageConfigured();
        case TELEM_BATTERY_CHARGE_LEVEL:
            FALLTHROUGH;
        case TELEM_BATTERY_CELL_COUNT:
            FALLTHROUGH;
        case TELEM_BATTERY_CELL_VOLTAGE:
            FALLTHROUGH;
        case TELEM_BATTERY_CELL_VOLTAGES:
            return isBatteryVoltageConfigured();
        case TELEM_BATTERY_FUEL:
            return true;

        /////////////////////////////////////////
        ////// MOVING //////////////////////////
        case TELEM_HEADING:
            return true;
        case TELEM_ALTITUDE:
            return sensors(SENSOR_BARO);
        case TELEM_ATTITUDE:
            return true;
        case TELEM_ATTITUDE_PITCH:
            return true;
        case TELEM_ATTITUDE_ROLL:
            return true;
        case TELEM_ATTITUDE_YAW:
            return true;
        case TELEM_VARIOMETER:
            return sensors(SENSOR_BARO);

        case TELEM_ACCEL_X:
            return true;
        case TELEM_ACCEL_Y:
            return true;
        case TELEM_ACCEL_Z:
            return true;

        /////////////////////////////////////////
        ////// GPS     //////////////////////////
#ifdef USE_GPS
        case TELEM_GPS:
            FALLTHROUGH;
        case TELEM_GPS_SATS:
            FALLTHROUGH;
        case TELEM_GPS_HDOP:
            FALLTHROUGH;
        case TELEM_GPS_COORD:
            FALLTHROUGH;
        case TELEM_GPS_ALTITUDE:
            FALLTHROUGH;
        case TELEM_GPS_HEADING:
            FALLTHROUGH;
        case TELEM_GPS_GROUNDSPEED:
            FALLTHROUGH;
        case TELEM_GPS_HOME_DISTANCE:
            FALLTHROUGH;
        case TELEM_GPS_HOME_DIRECTION:
            FALLTHROUGH;
        case TELEM_GPS_AZIMUTH:
            return shouldSendGpsData();
#endif

        /////////////////////////////////////////
        ////// ESC     //////////////////////////

        case TELEM_ESC_RPM:
            FALLTHROUGH;
        case TELEM_ESC1_RPM:
            return shouldSendDataForMotorIndex(0);
        case TELEM_ESC2_RPM:
            return shouldSendDataForMotorIndex(1);
        case TELEM_ESC3_RPM:
            return shouldSendDataForMotorIndex(2);
        case TELEM_ESC4_RPM:
            return shouldSendDataForMotorIndex(3);;

        case TELEM_TEMPERATURE:
            return true;
        case TELEM_ESC_TEMPERATURE:
            FALLTHROUGH;
        case TELEM_ESC1_TEMPERATURE:
            return shouldSendDataForMotorIndex(0);
        case TELEM_ESC2_TEMPERATURE:
            return shouldSendDataForMotorIndex(1);
        case TELEM_ESC3_TEMPERATURE:
            return shouldSendDataForMotorIndex(2);
        case TELEM_ESC4_TEMPERATURE:
            return shouldSendDataForMotorIndex(3);

        /////////////////////////////////////////
        ////// SYSTEM  //////////////////////////
        case TELEM_CPU_LOAD:
            return true;
        case TELEM_FLIGHT_MODE:
            return true;
        case TELEM_ARMING_FLAGS:
            return true;
        case TELEM_PROFILES:
            return true;

        ////////////////////////////////////////
        ////// LEGACY SMARTPORT  ////////////////
        case TELEM_LEGACY_VFAS:
            return isBatteryVoltageConfigured();
        case TELEM_LEGACY_CURRENT:
            return isAmperageConfigured();
        case TELEM_LEGACY_ALTITUDE:
            return sensors(SENSOR_BARO);
        case TELEM_LEGACY_FUEL:
            return true;
        case TELEM_LEGACY_VARIO:
            return sensors(SENSOR_BARO);
        case TELEM_LEGACY_HEADING:
            return true;
        case TELEM_LEGACY_PITCH:
            FALLTHROUGH;
        case TELEM_LEGACY_ROLL:
            return telemetryConfig()->frsky_pitch_roll;
        case TELEM_LEGACY_ACCX:
            FALLTHROUGH;
        case TELEM_LEGACY_ACCY:
            FALLTHROUGH;
        case TELEM_LEGACY_ACCZ:
            return sensors(SENSOR_ACC) && !telemetryConfig()->frsky_pitch_roll;
        case TELEM_LEGACY_MODES:
            return true;

#ifdef USE_GPS
        case TELEM_LEGACY_GNSS:
            FALLTHROUGH;
        case TELEM_LEGACY_SPEED:
            FALLTHROUGH;
        case TELEM_LEGACY_LAT:
            FALLTHROUGH;
        case TELEM_LEGACY_LON:
            FALLTHROUGH;
        case TELEM_LEGACY_HOME_DIST:
            FALLTHROUGH;
        case TELEM_LEGACY_GPS_ALT:
            FALLTHROUGH;
        case TELEM_LEGACY_FPV:
            FALLTHROUGH;
        case TELEM_LEGACY_AZIMUTH:
            return shouldSendGpsData();
        case TELEM_LEGACY_A4:
            return isBatteryVoltageConfigured();
#endif
#ifdef USE_PITOT
        case TELEM_LEGACY_ASPD:
            return sensors(SENSOR_PITOT) && pitotIsHealthy();
#endif
        default:
            return false;
    }
}

////////////////////////////////////////////////////////////
#endif  // USE_TELEMETRY