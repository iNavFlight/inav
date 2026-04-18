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

#pragma once

#include "common/time.h"

#include "io/serial.h"

#include "target/common.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#ifndef MAVLINK_COMM_NUM_BUFFERS
#define MAVLINK_COMM_NUM_BUFFERS MAX_MAVLINK_PORTS
#endif
#include "common/mavlink.h"
#pragma GCC diagnostic pop

#define TELEMETRY_MAVLINK_PORT_MODE MODE_RXTX
#define TELEMETRY_MAVLINK_MAXRATE 50
#define TELEMETRY_MAVLINK_DELAY ((1000 * 1000) / TELEMETRY_MAVLINK_MAXRATE)
#define TELEMETRY_MAVLINK_HIGH_LATENCY_INTERVAL_US (5 * 1000 * 1000)
#define MAV_DATA_STREAM_EXTENDED_SYS_STATE (MAV_DATA_STREAM_EXTRA3 + 1)
#define MAV_DATA_STREAM_HEARTBEAT (MAV_DATA_STREAM_EXTENDED_SYS_STATE + 1)
#define MAVLINK_STREAM_COUNT (MAV_DATA_STREAM_HEARTBEAT + 1)
#define ARDUPILOT_VERSION_MAJOR 4
#define ARDUPILOT_VERSION_MINOR 7
#define ARDUPILOT_VERSION_PATCH 0
#define MAVLINK_MAX_ROUTES 32
#define MAVLINK_PORT_MASK(portIndex) (1U << (portIndex))
#define MAXSTREAMS MAVLINK_STREAM_COUNT

typedef enum {
    MAVLINK_PERIODIC_MESSAGE_HEARTBEAT,
    MAVLINK_PERIODIC_MESSAGE_SYS_STATUS,
    MAVLINK_PERIODIC_MESSAGE_EXTENDED_SYS_STATE,
    MAVLINK_PERIODIC_MESSAGE_RC_CHANNELS,
    MAVLINK_PERIODIC_MESSAGE_GPS_RAW_INT,
    MAVLINK_PERIODIC_MESSAGE_GLOBAL_POSITION_INT,
    MAVLINK_PERIODIC_MESSAGE_GPS_GLOBAL_ORIGIN,
    MAVLINK_PERIODIC_MESSAGE_ATTITUDE,
    MAVLINK_PERIODIC_MESSAGE_VFR_HUD,
    MAVLINK_PERIODIC_MESSAGE_BATTERY_STATUS,
    MAVLINK_PERIODIC_MESSAGE_SCALED_PRESSURE,
    MAVLINK_PERIODIC_MESSAGE_SYSTEM_TIME,
    MAVLINK_PERIODIC_MESSAGE_COUNT
} mavlinkPeriodicMessage_e;

/**
 * MAVLink requires angles to be in the range -Pi..Pi.
 * This converts angles from a range of 0..Pi to -Pi..Pi
 */
#define RADIANS_TO_MAVLINK_RANGE(angle) (((angle) > M_PIf) ? ((angle) - (2 * M_PIf)) : (angle))

typedef struct mavlinkRouteEntry_s {
    uint8_t sysid;
    uint8_t compid;
    uint8_t ingressPortIndex;
} mavlinkRouteEntry_t;

typedef struct mavlinkPortRuntime_s {
    serialPort_t *port;
    const serialPortConfig_t *portConfig;
    portSharing_e portSharing;
    bool telemetryEnabled;
    bool txbuffValid;
    uint8_t txbuffFree;
    timeUs_t lastMavlinkMessageUs;
    timeUs_t lastRxFrameUs;
    timeUs_t lastHighLatencyMessageUs;
    bool highLatencyEnabled;
    uint8_t mavRates[MAVLINK_STREAM_COUNT];
    uint8_t mavRatesConfigured[MAVLINK_STREAM_COUNT];
    timeUs_t mavStreamNextDue[MAVLINK_STREAM_COUNT];
    int32_t mavMessageOverrideIntervalsUs[MAVLINK_PERIODIC_MESSAGE_COUNT];
    timeUs_t mavMessageNextDue[MAVLINK_PERIODIC_MESSAGE_COUNT];
    uint8_t txSeq;
    uint32_t txDroppedFrames;
    mavlink_message_t mavRecvMsg;
    mavlink_status_t mavRecvStatus;
} mavlinkPortRuntime_t;
