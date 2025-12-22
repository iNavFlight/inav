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

#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

#include "platform.h"

#ifdef USE_TELEMETRY

#include "build/debug.h"

#include "common/utils.h"
#include "common/maths.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/serial.h"

#include "fc/config.h"
#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "io/serial.h"

#include "rx/rx.h"

#include "telemetry/telemetry.h"
#include "telemetry/hott.h"
#include "telemetry/smartport.h"
#include "telemetry/ltm.h"
#include "telemetry/mavlink.h"
#include "telemetry/jetiexbus.h"
#include "telemetry/ibus.h"
#include "telemetry/crsf.h"
#include "telemetry/srxl.h"
#include "telemetry/sim.h"
#include "telemetry/ghst.h"
#include "telemetry/sbus2.h"



PG_REGISTER_WITH_RESET_TEMPLATE(telemetryConfig_t, telemetryConfig, PG_TELEMETRY_CONFIG, /*version*/9);

PG_RESET_TEMPLATE(telemetryConfig_t, telemetryConfig,
    .telemetry_switch = SETTING_TELEMETRY_SWITCH_DEFAULT,
    .telemetry_inverted = SETTING_TELEMETRY_INVERTED_DEFAULT,
    .frsky_pitch_roll = SETTING_FRSKY_PITCH_ROLL_DEFAULT,
    .report_cell_voltage = SETTING_REPORT_CELL_VOLTAGE_DEFAULT,
    .hottAlarmSoundInterval = SETTING_HOTT_ALARM_SOUND_INTERVAL_DEFAULT,
    .halfDuplex = SETTING_TELEMETRY_HALFDUPLEX_DEFAULT,
#if !defined(SETTING_SMARTPORT_FUEL_UNIT_DEFAULT)  // SITL
    .smartportFuelUnit = 1,
#else
    .smartportFuelUnit = SETTING_SMARTPORT_FUEL_UNIT_DEFAULT,
#endif
    .ibusTelemetryType = SETTING_IBUS_TELEMETRY_TYPE_DEFAULT,
    .ltmUpdateRate = SETTING_LTM_UPDATE_RATE_DEFAULT,

#ifdef USE_TELEMETRY_SIM
    .simTransmitInterval = SETTING_SIM_TRANSMIT_INTERVAL_DEFAULT,
    .simTransmitFlags = SETTING_SIM_TRANSMIT_FLAGS_DEFAULT,
    .simLowAltitude = SETTING_SIM_LOW_ALTITUDE_DEFAULT,
    .simPin = SETTING_SIM_PIN_DEFAULT,
    .simGroundStationNumber = SETTING_SIM_GROUND_STATION_NUMBER_DEFAULT,

    .accEventThresholdHigh = SETTING_ACC_EVENT_THRESHOLD_HIGH_DEFAULT,
    .accEventThresholdLow = SETTING_ACC_EVENT_THRESHOLD_LOW_DEFAULT,
    .accEventThresholdNegX = SETTING_ACC_EVENT_THRESHOLD_NEG_X_DEFAULT,
#endif

    .mavlink = {
        .autopilot_type = SETTING_MAVLINK_AUTOPILOT_TYPE_DEFAULT,
        .extended_status_rate = SETTING_MAVLINK_EXT_STATUS_RATE_DEFAULT,
        .rc_channels_rate = SETTING_MAVLINK_RC_CHAN_RATE_DEFAULT,
        .position_rate = SETTING_MAVLINK_POS_RATE_DEFAULT,
        .extra1_rate = SETTING_MAVLINK_EXTRA1_RATE_DEFAULT,
        .extra2_rate = SETTING_MAVLINK_EXTRA2_RATE_DEFAULT,
        .extra3_rate = SETTING_MAVLINK_EXTRA3_RATE_DEFAULT,
        .version = SETTING_MAVLINK_VERSION_DEFAULT,
        .min_txbuff = SETTING_MAVLINK_MIN_TXBUFFER_DEFAULT,
        .radio_type = SETTING_MAVLINK_RADIO_TYPE_DEFAULT,
        .sysid = SETTING_MAVLINK_SYSID_DEFAULT
    },
#ifdef USE_TELEMETRY_CRSF
    .crsf_telemetry_link_rate = SETTING_CRSF_TELEMETRY_LINK_RATE_DEFAULT,
    .crsf_telemetry_link_ratio = SETTING_CRSF_TELEMETRY_LINK_RATIO_DEFAULT,
#endif //USE_TELEMETRY_CRSF

#if defined(USE_CUSTOM_TELEMETRY)
    .telemetry_mode = SETTING_TELEMETRY_MODE_DEFAULT,
    .telemetry_sensors =  { 0x0,  }, // all sensors enabled by default
#endif

);

void telemetryInit(void)
{

#if defined(USE_TELEMETRY_HOTT)
    initHoTTTelemetry();
#endif

#if defined(USE_TELEMETRY_SMARTPORT)
    initSmartPortTelemetry();
#endif

#if defined(USE_TELEMETRY_LTM)
    initLtmTelemetry();
#endif

#if defined(USE_TELEMETRY_MAVLINK)
    initMAVLinkTelemetry();
#endif

#if defined(USE_TELEMETRY_JETIEXBUS)
    initJetiExBusTelemetry();
#endif

#if defined(USE_TELEMETRY_IBUS)
    initIbusTelemetry();
#endif

#if defined(USE_TELEMETRY_SIM)
    initSimTelemetry();
#endif

#if defined(USE_SERIALRX_CRSF) && defined(USE_TELEMETRY_CRSF)
    initCrsfTelemetry();
#endif

#ifdef USE_TELEMETRY_SRXL
    initSrxlTelemetry();
#endif

#ifdef USE_TELEMETRY_GHST
    initGhstTelemetry();
#endif

    telemetryCheckState();
}

bool telemetryDetermineEnabledState(portSharing_e portSharing)
{
    bool enabled = portSharing == PORTSHARING_NOT_SHARED;

    if (portSharing == PORTSHARING_SHARED) {
        if (telemetryConfig()->telemetry_switch)
            enabled = IS_RC_MODE_ACTIVE(BOXTELEMETRY);
        else
            enabled = ARMING_FLAG(ARMED);
    }

    return enabled;
}

bool telemetryCheckRxPortShared(const serialPortConfig_t *portConfig)
{
    return portConfig->functionMask & FUNCTION_RX_SERIAL && portConfig->functionMask & TELEMETRY_SHAREABLE_PORT_FUNCTIONS_MASK;
}

serialPort_t *telemetrySharedPort = NULL;

void telemetryCheckState(void)
{

#if defined(USE_TELEMETRY_HOTT)
    checkHoTTTelemetryState();
#endif

#if defined(USE_TELEMETRY_SMARTPORT)
    checkSmartPortTelemetryState();
#endif

#if defined(USE_TELEMETRY_LTM)
    checkLtmTelemetryState();
#endif

#if defined(USE_TELEMETRY_MAVLINK)
    checkMAVLinkTelemetryState();
#endif

#if defined(USE_TELEMETRY_JETIEXBUS)
    checkJetiExBusTelemetryState();
#endif

#if defined(USE_TELEMETRY_IBUS)
    checkIbusTelemetryState();
#endif

#if defined(USE_TELEMETRY_SIM)
    checkSimTelemetryState();
#endif

#if defined(USE_SERIALRX_CRSF) && defined(USE_TELEMETRY_CRSF)
    checkCrsfTelemetryState();
#endif

#ifdef USE_TELEMETRY_SRXL
    checkSrxlTelemetryState();
#endif
#ifdef USE_TELEMETRY_GHST
    checkGhstTelemetryState();
#endif
}

void telemetryProcess(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs); // since not used by all the telemetry protocols

#if defined(USE_TELEMETRY_HOTT)
    handleHoTTTelemetry(currentTimeUs);
#endif

#if defined(USE_TELEMETRY_SMARTPORT)
    handleSmartPortTelemetry(currentTimeUs);
#endif

#if defined(USE_TELEMETRY_LTM)
    handleLtmTelemetry();
#endif

#if defined(USE_TELEMETRY_MAVLINK)
    handleMAVLinkTelemetry(currentTimeUs);
#endif

#if defined(USE_TELEMETRY_JETIEXBUS)
    handleJetiExBusTelemetry();
#endif

#if defined(USE_SERIALRX_IBUS) && defined(USE_TELEMETRY_IBUS)
    handleIbusTelemetry();
#endif

#if defined(USE_TELEMETRY_SIM)
    handleSimTelemetry();
#endif

#if defined(USE_SERIALRX_CRSF) && defined(USE_TELEMETRY_CRSF)
    handleCrsfTelemetry(currentTimeUs);
#endif

#ifdef USE_TELEMETRY_SRXL
    handleSrxlTelemetry(currentTimeUs);
#endif
#ifdef USE_TELEMETRY_GHST
    handleGhstTelemetry(currentTimeUs);
#endif

#ifdef USE_TELEMETRY_SBUS2
    handleSbus2Telemetry(currentTimeUs);
#endif
}


/** Telemetry scheduling framework **/
static telemetryScheduler_t sch = { 0, };

void telemetryScheduleUpdate(timeUs_t currentTime)
{
    timeDelta_t delta = cmpTimeUs(currentTime, sch.update_time);

    for (int i = 0; i < sch.sensor_count; i++) {
        telemetrySensor_t * sensor = &sch.sensors[i];
        if (sensor->active) {
            int value = telemetrySensorValue(sensor->sensor_id);
            if (sensor->ratio_den)
                value = value * sensor->ratio_num / sensor->ratio_den;
            sensor->update |= (value != sensor->value);
            sensor->value = value;

            const int interval = (sensor->update) ? sensor->fast_interval : sensor->slow_interval;
            sensor->bucket += delta * 1000 / interval;
            sensor->bucket = constrain(sensor->bucket, sch.min_level, sch.max_level);
        }
    }

    sch.update_time = currentTime;
}

telemetrySensor_t * telemetryScheduleNext(void)
{
    int index = sch.start_index;

    for (int i = 0; i < sch.sensor_count; i++) {
        index = (index + 1) % sch.sensor_count;
        telemetrySensor_t * sensor = &sch.sensors[index];
        if (sensor->active && sensor->bucket >= 0)
            return sensor;
    }

    return NULL;
}

void telemetryScheduleAdd(telemetrySensor_t * sensor)
{
    if (sensor) {
        sensor->bucket = 0;
        sensor->value = 0;
        sensor->update = true;
        sensor->active = true;
    }
}

void telemetryScheduleCommit(telemetrySensor_t * sensor)
{
    if (sensor) {
        sensor->bucket = constrain(sensor->bucket - sch.quanta, sch.min_level, sch.max_level);
        sensor->update = false;

        sch.start_index = sensor->index;
    }
}

void telemetryScheduleInit(telemetrySensor_t * sensors, size_t count)
{
    sch.sensors = sensors;
    sch.sensor_count = count;

    sch.update_time = 0;
    sch.start_index = 0;

    sch.quanta = 1000000;
    sch.max_level = 500000;
    sch.min_level = -1500000;

    for (unsigned int i = 0; i < count; i++) {
        telemetrySensor_t * sensor = &sch.sensors[i];
        sensor->index = i;
    }
}

#endif


