/**************************************************************
**************************************************************
**************************************************************
**************************************************************
**                                                          **
**                                                          **
**                  REMOVE IN INAV 11                       **
**                                                          **
**                                                          **
**************************************************************
**************************************************************
**************************************************************
**************************************************************/
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
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "platform.h"

#if defined(USE_TELEMETRY) && defined(USE_TELEMETRY_SMARTPORT)

#include "smartport_legacy.h"
#include "common/maths.h"
#include "common/utils.h"

#include "config/parameter_group_ids.h"

#include "drivers/time.h"

#include "sensors/battery.h"

#include "io/gps.h"
#include "io/serial.h"

#include "rx/frsky_crc.h"

#include "telemetry/telemetry.h"
#include "telemetry/smartport.h"
#include "telemetry/smartport_legacy.h"
#include "telemetry/msp_shared.h"

#include "common/axis.h"
#include "common/color.h"
#include "common/maths.h"
#include "common/utils.h"

#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/accgyro/accgyro.h"
#include "drivers/compass/compass.h"
#include "drivers/sensor.h"
#include "drivers/time.h"

#include "fc/config.h"
#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"

#include "flight/failsafe.h"
#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/pid.h"

#include "io/beeper.h"
#include "io/gps.h"
#include "io/serial.h"

#include "navigation/navigation.h"

#include "rx/frsky_crc.h"

#include "sensors/boardalignment.h"
#include "sensors/sensors.h"
#include "sensors/battery.h"
#include "sensors/acceleration.h"
#include "sensors/barometer.h"
#include "sensors/compass.h"
#include "sensors/gyro.h"
#include "sensors/pitotmeter.h"

#include "rx/rx.h"

#include "telemetry/telemetry.h"
#include "telemetry/smartport.h"
#include "telemetry/msp_shared.h"

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
static uint32_t frskyGetFlightMode(void)
{
    uint32_t tmpi = 0;

    // ones column (G)
    if (!isArmingDisabled())
        tmpi += 1;
    else
        tmpi += 2;
    if (ARMING_FLAG(ARMED))
        tmpi += 4;

    // tens column (F)
    if (FLIGHT_MODE(ANGLE_MODE))
        tmpi += 10;
    if (FLIGHT_MODE(HORIZON_MODE))
        tmpi += 20;
    if (FLIGHT_MODE(MANUAL_MODE))
        tmpi += 40;

    // hundreds column (E)
    if (FLIGHT_MODE(HEADING_MODE))
        tmpi += 100;
    if (FLIGHT_MODE(NAV_ALTHOLD_MODE))
        tmpi += 200;
    if (FLIGHT_MODE(NAV_POSHOLD_MODE) && !STATE(AIRPLANE))
        tmpi += 400;

    // thousands column (D)
    if (FLIGHT_MODE(NAV_RTH_MODE) && !isWaypointMissionRTHActive())
        tmpi += 1000;
    if (FLIGHT_MODE(NAV_COURSE_HOLD_MODE)) // intentionally out of order and 'else-ifs' to prevent column overflow
        tmpi += 8000;
    else if (FLIGHT_MODE(NAV_WP_MODE))
        tmpi += 2000;
    else if (FLIGHT_MODE(HEADFREE_MODE))
        tmpi += 4000;

    // ten thousands column (C)
    if (FLIGHT_MODE(FLAPERON))
        tmpi += 10000;
    if (FLIGHT_MODE(FAILSAFE_MODE))
        tmpi += 40000;
    else if (FLIGHT_MODE(AUTO_TUNE)) // intentionally reverse order and 'else-if' to prevent 16-bit overflow
        tmpi += 20000;

    // hundred thousands column (B)
    if (FLIGHT_MODE(NAV_FW_AUTOLAND))
        tmpi += 100000;
    if (FLIGHT_MODE(TURTLE_MODE))
        tmpi += 200000;
    else if (FLIGHT_MODE(NAV_POSHOLD_MODE) && STATE(AIRPLANE))
        tmpi += 800000;
    if (FLIGHT_MODE(NAV_SEND_TO))
        tmpi += 400000;

    // million column (A)
    if (FLIGHT_MODE(NAV_RTH_MODE) && isWaypointMissionRTHActive())
        tmpi += 1000000;
    if (FLIGHT_MODE(ANGLEHOLD_MODE))
        tmpi += 2000000;

    return tmpi;
}

static uint16_t frskyGetGPSState(void)
{
    uint16_t tmpi = 0;

    // ones and tens columns (# of satellites 0 - 99)
    tmpi += constrain(gpsSol.numSat, 0, 99);

    // hundreds column (satellite accuracy HDOP: 0 = worst [HDOP > 5.5], 9 = best [HDOP <= 1.0])
    tmpi += (9 - constrain((gpsSol.hdop - 51) / 50, 0, 9)) * 100;

    // thousands column (GPS fix status)
    if (STATE(GPS_FIX))
        tmpi += 1000;
    if (STATE(GPS_FIX_HOME))
        tmpi += 2000;
    if (ARMING_FLAG(ARMED) && IS_RC_MODE_ACTIVE(BOXHOMERESET) && !FLIGHT_MODE(NAV_RTH_MODE) && !FLIGHT_MODE(NAV_WP_MODE))
        tmpi += 4000;

    return tmpi;
}

static void smartPortSensorEncodeINT(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload) {
    payload->data = sensor->value;
}


static void smartPortSensorEncodeVFAS(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    payload->data = telemetryConfig()->report_cell_voltage ? getBatteryAverageCellVoltage() : getBatteryVoltage();;
}

static void smartPortSensorEncodeCurrent(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    payload->data = getAmperage() / 10;
}

static void smartPortSensorEncodeAltitude(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    payload->data = (uint32_t)getEstimatedActualPosition(Z);
}

static void smartPortSensorEncodeFuel(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    if (telemetryConfig()->smartportFuelUnit == SMARTPORT_FUEL_UNIT_PERCENT) {
        payload->data = calculateBatteryPercentage(); // Show remaining battery % if smartport_fuel_percent=ON
    } else if (isAmperageConfigured()) {
        payload->data = telemetryConfig()->smartportFuelUnit == SMARTPORT_FUEL_UNIT_MAH ? getMAhDrawn() : getMWhDrawn();
    }
}

static void smartPortSensorEncodeVario(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    payload->data = lrintf(getEstimatedActualVelocity(Z));
}

static void smartPortSensorEncodeHeading(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    payload->data = attitude.values.yaw * 10;
}

static void smartPortSensorEncodePitch(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    payload->data = attitude.values.pitch;
}

static void smartPortSensorEncodeRoll(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    payload->data = attitude.values.roll;
}

static void smartPortSensorEncodeACCX(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    payload->data = lrintf(100 * acc.accADCf[X]);
}

static void smartPortSensorEncodeACCY(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    payload->data = lrintf(100 * acc.accADCf[Y]);
}

static void smartPortSensorEncodeACCZ(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    payload->data = lrintf(100 * acc.accADCf[Z]);
}

static void smartPortSensorEncodeModes(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    payload->data = frskyGetFlightMode();
}

#ifdef USE_GPS
static void smartPortSensorEncodeGNSS(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    payload->data = frskyGetGPSState();
}

static void smartPortSensorEncodeSpeed(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    //convert to knots: 1cm/s = 0.0194384449 knots
    //Speed should be sent in knots/1000 (GPS speed is in cm/s)
    payload->data = gpsSol.groundSpeed * 1944 / 100;
}

static void smartPortSensorEncodeLat(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    uint32_t tmpui = abs(gpsSol.llh.lon);  // now we have unsigned value and one bit to spare
    tmpui = (tmpui + tmpui / 2) / 25 | 0x80000000;  // 6/100 = 1.5/25, division by power of 2 is fast
    if (gpsSol.llh.lon < 0) tmpui |= 0x40000000;

    payload->data = tmpui;
}

static void smartPortSensorEncodeLon(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    uint32_t tmpui = abs(gpsSol.llh.lat);  // now we have unsigned value and one bit to spare
    tmpui = (tmpui + tmpui / 2) / 25;  // 6/100 = 1.5/25, division by power of 2 is fast
    if (gpsSol.llh.lat < 0) tmpui |= 0x40000000;

    payload->data = tmpui;
}

static void smartPortSensorEncodeHomeDist(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    payload->data = GPS_distanceToHome;
}

static void smartPortSensorEncodeGpsAlt(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    payload->data = gpsSol.llh.alt;
}

static void smartPortSensorEncodeFpv(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    payload->data = gpsSol.groundCourse;
}

static void smartPortSensorEncodeAzimuth(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    int16_t h = GPS_directionToHome;
    if (h < 0) {
        h += 360;
    }
    if(h >= 180)
        h = h - 180;
    else
        h = h + 180;

    payload->data = h;
}
#endif
static void smartPortSensorEncodeA4(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    payload->data = getBatteryAverageCellVoltage();
}

#ifdef USE_PITOT
static void smartPortSensorEncodeASPD(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    payload->data = (uint32_t)(getAirspeedEstimate() * 0.194384449f);
}
#endif


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
#define TLM_SENSOR(NAME, APPID, FAST, SLOW, WF, WS, DENOM, ENC) \
{ \
    .sensor_id = TELEM_##NAME, \
    .app_id = (APPID), \
    .fast_interval = (FAST), \
    .slow_interval = (SLOW), \
    .fast_weight = (WF), \
    .slow_weight = (WS), \
    .ratio_num = 1, \
    .ratio_den = (DENOM), \
    .value = 0, \
    .bucket = 0, \
    .update = 0, \
    .active = false, \
    .encode = (telemetryEncode_f)smartPortSensorEncode##ENC, \
}

static telemetrySensor_t smartportTelemetrySensorsLegacy[] =
{
        TLM_SENSOR(HEARTBEAT        , 0x5100, 1000, 1000, 0, 0, 0, INT),
        TLM_SENSOR(LEGACY_VFAS      , 0x0210,  200,  200, 0, 0, 0, VFAS),
        TLM_SENSOR(LEGACY_CURRENT   , 0x0200,  200,  200, 0, 0, 0, Current),
        TLM_SENSOR(LEGACY_ALTITUDE  , 0x0100,  200,  200, 0, 0, 0, Altitude),
        TLM_SENSOR(LEGACY_FUEL      , 0x0600,  200,  200, 0, 0, 0, Fuel),
        TLM_SENSOR(LEGACY_VARIO     , 0x0110,  200,  200, 0, 0, 0, Vario),
        TLM_SENSOR(LEGACY_HEADING   , 0x0840,  200,  200, 0, 0, 0, Heading),
        TLM_SENSOR(LEGACY_PITCH     , 0x0430,  200,  200, 0, 0, 0, Pitch),
        TLM_SENSOR(LEGACY_ROLL      , 0x0440,  200,  200, 0, 0, 0, Roll),
        TLM_SENSOR(LEGACY_ACCX      , 0x0700,  200,  200, 0, 0, 0, ACCX),
        TLM_SENSOR(LEGACY_ACCY      , 0x0710,  200,  200, 0, 0, 0, ACCY),
        TLM_SENSOR(LEGACY_ACCZ      , 0x0720,  200,  200, 0, 0, 0, ACCZ),
        TLM_SENSOR(LEGACY_MODES     , 0x0470,  200,  200, 0, 0, 0, Modes),
#ifdef USE_GPS
        TLM_SENSOR(LEGACY_GNSS      , 0x0480,  200,  200, 0, 0, 0, GNSS),
        TLM_SENSOR(LEGACY_SPEED     , 0x0830,  200,  200, 0, 0, 0, Speed),
        TLM_SENSOR(LEGACY_LAT       , 0x0800,  200,  200, 0, 0, 0, Lat),
        TLM_SENSOR(LEGACY_LON       , 0x0800,  200,  200, 0, 0, 0, Lon),
        TLM_SENSOR(LEGACY_HOME_DIST , 0x0420,  200,  200, 0, 0, 0, HomeDist),
        TLM_SENSOR(LEGACY_GPS_ALT   , 0x0820,  200,  200, 0, 0, 0, GpsAlt),
        TLM_SENSOR(LEGACY_FPV       , 0x0450,  200,  200, 0, 0, 0, Fpv),
        TLM_SENSOR(LEGACY_AZIMUTH   , 0x0460,  200,  200, 0, 0, 0, Azimuth),
#endif
        TLM_SENSOR(LEGACY_A4        , 0x0910,  200,  200, 0, 0, 0, A4),
#ifdef USE_PITOT
        TLM_SENSOR(LEGACY_ASPD      , 0x0A00,  200,  200, 0, 0, 0, ASPD),
#endif
};

void initSmartPortSensorsLegacy(void) {
    telemetryScheduleInit(smartportTelemetrySensorsLegacy, ARRAYLEN(smartportTelemetrySensorsLegacy));

    for(size_t i = 0; i < ARRAYLEN(smartportTelemetrySensorsLegacy); i++) {
        if(telemetrySensorAllowed(smartportTelemetrySensorsLegacy[i].sensor_id)) {
            telemetryScheduleAdd(&smartportTelemetrySensorsLegacy[i]);
        }
    }

}


#endif