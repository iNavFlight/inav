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

/*
 * telemetry.h
 *
 *  Created on: 6 Apr 2014
 *      Author: Hydra
 */

#pragma once

#include "common/time.h"
#include "config/parameter_group.h"
#include "io/serial.h"
#include "telemetry/sensors.h"

#define TELEM_SENSOR_SLOT_COUNT 30

typedef enum {
    LTM_RATE_NORMAL,
    LTM_RATE_MEDIUM,
    LTM_RATE_SLOW
} ltmUpdateRate_e;

typedef enum {
    MAVLINK_AUTOPILOT_GENERIC,
    MAVLINK_AUTOPILOT_ARDUPILOT
} mavlinkAutopilotType_e;

typedef enum {
    MAVLINK_RADIO_GENERIC,
    MAVLINK_RADIO_ELRS,
    MAVLINK_RADIO_SIK,
} mavlinkRadio_e;

typedef enum {
    SMARTPORT_FUEL_UNIT_PERCENT,
    SMARTPORT_FUEL_UNIT_MAH,
    SMARTPORT_FUEL_UNIT_MWH
} smartportFuelUnit_e;

typedef enum {
    TELEMETRY_STATE_OFF = 0,
    TELEMETRY_STATE_LEGACY,
    TELEMETRY_STATE_CUSTOM,
    TELEMETRY_STATE_POPULATE,
} telemetryState_e;

typedef enum {
    TELEMETRY_MODE_LEGACY,
    TELEMETRY_MODE_CUSTOM,
} telemetryMode_e;

typedef struct telemetryConfig_s {
    uint8_t telemetry_switch;               // Use aux channel to change serial output & baudrate( MSP / Telemetry ). It disables automatic switching to Telemetry when armed.
    uint8_t telemetry_inverted;             // Flip the default inversion of the protocol - Same as serialrx_inverted in rx.c, but for telemetry.
    uint8_t frsky_pitch_roll;
    uint8_t report_cell_voltage;
    uint8_t hottAlarmSoundInterval;
    uint8_t halfDuplex;
    smartportFuelUnit_e smartportFuelUnit;
    uint8_t ibusTelemetryType;
    uint8_t ltmUpdateRate;

#ifdef USE_TELEMETRY_SIM
    int16_t simLowAltitude;
    char simGroundStationNumber[16];
    char simPin[8];
    uint16_t simTransmitInterval;
    uint8_t simTransmitFlags;

    uint16_t accEventThresholdHigh;
    uint16_t accEventThresholdLow;
    uint16_t accEventThresholdNegX;
#endif
    struct {
        uint8_t autopilot_type;
        uint8_t extended_status_rate;
        uint8_t rc_channels_rate;
        uint8_t position_rate;
        uint8_t extra1_rate;
        uint8_t extra2_rate;
        uint8_t extra3_rate;
        uint8_t version;
        uint8_t min_txbuff;
        uint8_t radio_type;
        uint8_t sysid;
    } mavlink;
#ifdef USE_TELEMETRY_CRSF
    uint16_t crsf_telemetry_link_rate;
    uint16_t crsf_telemetry_link_ratio;
#endif

#if  defined(USE_CUSTOM_TELEMETRY)
    uint8_t telemetry_mode;
    uint16_t telemetry_sensors[TELEM_SENSOR_SLOT_COUNT];
#endif

} telemetryConfig_t;

typedef struct {
    telemetrySensor_t *         sensors;
    uint16_t                    sensor_count;
    uint16_t                    start_index;
    timeUs_t                    update_time;
    int32_t                     max_level;
    int32_t                     min_level;
    int32_t                     quanta;
} telemetryScheduler_t;

PG_DECLARE(telemetryConfig_t, telemetryConfig);

#define TELEMETRY_SHAREABLE_PORT_FUNCTIONS_MASK (FUNCTION_TELEMETRY_LTM | FUNCTION_TELEMETRY_IBUS)
extern serialPort_t *telemetrySharedPort;

void telemetryInit(void);
bool telemetryCheckRxPortShared(const serialPortConfig_t *portConfig);

void telemetryCheckState(void);
void telemetryProcess(timeUs_t currentTimeUs);

bool telemetryDetermineEnabledState(portSharing_e portSharing);
void telemetryScheduleAdd(telemetrySensor_t * sensor);
void telemetryScheduleUpdate(timeUs_t currentTime);
void telemetryScheduleCommit(telemetrySensor_t * sensor);
void telemetryScheduleInit(telemetrySensor_t * sensors, size_t count);
telemetrySensor_t * telemetryScheduleNext(void);
