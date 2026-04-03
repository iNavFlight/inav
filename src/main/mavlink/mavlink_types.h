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
#define ARDUPILOT_VERSION_MINOR 6
#define ARDUPILOT_VERSION_PATCH 3
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

typedef enum {
    MAV_FRAME_SUPPORTED_NONE = 0,
    MAV_FRAME_SUPPORTED_GLOBAL = (1 << 0),
    MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT = (1 << 1),
    MAV_FRAME_SUPPORTED_GLOBAL_INT = (1 << 2),
    MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT_INT = (1 << 3),
} mavFrameSupportMask_e;

/**
 * MAVLink requires angles to be in the range -Pi..Pi.
 * This converts angles from a range of 0..Pi to -Pi..Pi
 */
#define RADIANS_TO_MAVLINK_RANGE(angle) (((angle) > M_PIf) ? ((angle) - (2 * M_PIf)) : (angle))

/** @brief A mapping of plane flight modes for custom_mode field of heartbeat. */
typedef enum APM_PLANE_MODE
{
   PLANE_MODE_MANUAL=0,
   PLANE_MODE_CIRCLE=1,
   PLANE_MODE_STABILIZE=2,
   PLANE_MODE_TRAINING=3,
   PLANE_MODE_ACRO=4,
   PLANE_MODE_FLY_BY_WIRE_A=5,
   PLANE_MODE_FLY_BY_WIRE_B=6,
   PLANE_MODE_CRUISE=7,
   PLANE_MODE_AUTOTUNE=8,
   PLANE_MODE_AUTO=10,
   PLANE_MODE_RTL=11,
   PLANE_MODE_LOITER=12,
   PLANE_MODE_TAKEOFF=13,
   PLANE_MODE_AVOID_ADSB=14,
   PLANE_MODE_GUIDED=15,
   PLANE_MODE_INITIALIZING=16,
   PLANE_MODE_QSTABILIZE=17,
   PLANE_MODE_QHOVER=18,
   PLANE_MODE_QLOITER=19,
   PLANE_MODE_QLAND=20,
   PLANE_MODE_QRTL=21,
   PLANE_MODE_QAUTOTUNE=22,
   PLANE_MODE_QACRO=23,
   PLANE_MODE_THERMAL=24,
   PLANE_MODE_LOITER_ALT_QLAND=25,
   PLANE_MODE_AUTOLAND=26,
   PLANE_MODE_ENUM_END=27,
} APM_PLANE_MODE;

/** @brief A mapping of copter flight modes for custom_mode field of heartbeat. */
typedef enum APM_COPTER_MODE
{
   COPTER_MODE_STABILIZE=0,
   COPTER_MODE_ACRO=1,
   COPTER_MODE_ALT_HOLD=2,
   COPTER_MODE_AUTO=3,
   COPTER_MODE_GUIDED=4,
   COPTER_MODE_LOITER=5,
   COPTER_MODE_RTL=6,
   COPTER_MODE_CIRCLE=7,
   COPTER_MODE_LAND=9,
   COPTER_MODE_DRIFT=11,
   COPTER_MODE_SPORT=13,
   COPTER_MODE_FLIP=14,
   COPTER_MODE_AUTOTUNE=15,
   COPTER_MODE_POSHOLD=16,
   COPTER_MODE_BRAKE=17,
   COPTER_MODE_THROW=18,
   COPTER_MODE_AVOID_ADSB=19,
   COPTER_MODE_GUIDED_NOGPS=20,
   COPTER_MODE_SMART_RTL=21,
   COPTER_MODE_FLOWHOLD=22,
   COPTER_MODE_FOLLOW=23,
   COPTER_MODE_ZIGZAG=24,
   COPTER_MODE_SYSTEMID=25,
   COPTER_MODE_AUTOROTATE=26,
   COPTER_MODE_AUTO_RTL=27,
   COPTER_MODE_TURTLE=28,
   COPTER_MODE_ENUM_END=29,
} APM_COPTER_MODE;

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

typedef struct mavlinkModeDescriptor_s {
    uint8_t customMode;
    const char *name;
} mavlinkModeDescriptor_t;

extern const mavlinkModeDescriptor_t planeModes[];
extern const uint8_t planeModesCount;
extern const mavlinkModeDescriptor_t copterModes[];
extern const uint8_t copterModesCount;

typedef struct mavlinkMissionItemData_s {
    uint8_t frame;
    uint16_t command;
    float param1;
    float param2;
    float param3;
    float param4;
    int32_t lat;
    int32_t lon;
    float alt;
} mavlinkMissionItemData_t;
