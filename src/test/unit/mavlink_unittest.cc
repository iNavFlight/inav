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
#include <string.h>

#include <limits.h>

extern "C" {
    #include "platform.h"

    #include "build/debug.h"

    #include "common/axis.h"
    #include "common/maths.h"
    #include "common/time.h"

    #include "config/parameter_group.h"
    #include "config/parameter_group_ids.h"

    #include "drivers/display.h"
    #include "drivers/osd_symbols.h"
    #include "drivers/serial.h"

    #include "fc/config.h"
    #include "fc/rc_modes.h"
    #include "fc/runtime_config.h"
    #include "fc/settings.h"

    #include "flight/failsafe.h"
    #include "flight/imu.h"
    #include "flight/mixer.h"
    #include "flight/mixer_profile.h"

    #include "io/adsb.h"
    #include "io/gps.h"
    #include "io/osd.h"

    #include "navigation/navigation.h"

    #include "rx/rx.h"

    #include "sensors/barometer.h"
    #include "sensors/battery.h"
    #include "sensors/diagnostics.h"
    #include "sensors/gyro.h"
    #include "sensors/pitotmeter.h"
    #include "sensors/temperature.h"

    #include "telemetry/mavlink.h"
    #include "telemetry/telemetry.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
    #define MAVLINK_COMM_NUM_BUFFERS 1
    #include "common/mavlink.h"
#pragma GCC diagnostic pop

    void mavlinkSendAttitude(void);
    void mavlinkSendBatteryTemperatureStatusText(void);

    PG_REGISTER(telemetryConfig_t, telemetryConfig, PG_TELEMETRY_CONFIG, 0);
    PG_REGISTER(rxConfig_t, rxConfig, PG_RX_CONFIG, 0);
    PG_REGISTER(systemConfig_t, systemConfig, PG_SYSTEM_CONFIG, 0);
    PG_REGISTER_ARRAY(mixerProfile_t, MAX_MIXER_PROFILE_COUNT, mixerProfiles, PG_MIXER_PROFILE, 0);
}

#include "unittest_macros.h"
#include "gtest/gtest.h"

static serialPort_t testSerialPort;
static serialPortConfig_t testPortConfig;
static uint8_t serialRxBuffer[2048];
static uint8_t serialTxBuffer[2048];
static size_t serialRxLen;
static size_t serialRxPos;
static size_t serialTxLen;

const uint32_t baudRates[] = {
    0, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 250000,
    460800, 921600, 1000000, 1500000, 2000000, 2470000
};

static navWaypoint_t lastWaypoint;
static int setWaypointCalls;
static int resetWaypointCalls;
static int mavlinkRxHandleCalls;
static bool gcsValid;
static int waypointCount;
static navWaypoint_t waypointStore[4];

static void resetSerialBuffers(void)
{
    serialRxLen = 0;
    serialRxPos = 0;
    serialTxLen = 0;
}

static void pushRxMessage(const mavlink_message_t *msg)
{
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    int length = mavlink_msg_to_send_buffer(buffer, msg);
    memcpy(&serialRxBuffer[serialRxLen], buffer, (size_t)length);
    serialRxLen += (size_t)length;
}

static bool popTxMessage(mavlink_message_t *msg)
{
    mavlink_status_t status;
    memset(&status, 0, sizeof(status));
    for (size_t i = 0; i < serialTxLen; i++) {
        if (mavlink_parse_char(0, serialTxBuffer[i], msg, &status) == MAVLINK_FRAMING_OK) {
            return true;
        }
    }
    return false;
}

static void initMavlinkTestState(void)
{
    resetSerialBuffers();
    setWaypointCalls = 0;
    resetWaypointCalls = 0;
    mavlinkRxHandleCalls = 0;
    gcsValid = true;
    waypointCount = 0;
    memset(waypointStore, 0, sizeof(waypointStore));
    memset(&rxLinkStatistics, 0, sizeof(rxLinkStatistics));

    telemetryConfigMutable()->mavlink.sysid = 1;
    telemetryConfigMutable()->mavlink.autopilot_type = MAVLINK_AUTOPILOT_ARDUPILOT;
    telemetryConfigMutable()->mavlink.version = 2;
    telemetryConfigMutable()->mavlink.min_txbuff = 0;
    telemetryConfigMutable()->halfDuplex = 0;

    rxConfigMutable()->receiverType = RX_TYPE_NONE;
    rxConfigMutable()->serialrx_provider = SERIALRX_SBUS;
    rxConfigMutable()->halfDuplex = 0;

    systemConfigMutable()->current_mixer_profile_index = 0;
    mixerProfilesMutable(0)->mixer_config.platformType = PLATFORM_AIRPLANE;

    rxRuntimeConfig.channelCount = 8;

    initMAVLinkTelemetry();
    configureMAVLinkTelemetryPort();
}

TEST(MavlinkTelemetryTest, AttitudeUsesRadiansPerSecond)
{
    initMavlinkTestState();

    attitude.values.roll = 100;
    attitude.values.pitch = -200;
    attitude.values.yaw = 450;
    gyro.gyroADCf[FD_ROLL] = 90.0f;
    gyro.gyroADCf[FD_PITCH] = -45.0f;
    gyro.gyroADCf[FD_YAW] = 180.0f;

    mavlinkSendAttitude();

    mavlink_message_t msg;
    ASSERT_TRUE(popTxMessage(&msg));
    ASSERT_EQ(msg.msgid, MAVLINK_MSG_ID_ATTITUDE);

    mavlink_attitude_t att;
    mavlink_msg_attitude_decode(&msg, &att);

    EXPECT_NEAR(att.rollspeed, DEGREES_TO_RADIANS(gyro.gyroADCf[FD_ROLL]), 1e-6f);
    EXPECT_NEAR(att.pitchspeed, DEGREES_TO_RADIANS(gyro.gyroADCf[FD_PITCH]), 1e-6f);
    EXPECT_NEAR(att.yawspeed, DEGREES_TO_RADIANS(gyro.gyroADCf[FD_YAW]), 1e-6f);
}

TEST(MavlinkTelemetryTest, CommandLongRepositionUsesGlobalFrameAndParams)
{
    initMavlinkTestState();

    mavlink_message_t cmd;
    mavlink_msg_command_long_pack(
        42, 200, &cmd,
        1, MAV_COMP_ID_MISSIONPLANNER,
        MAV_CMD_DO_REPOSITION,
        0,
        0, 0, 0, 123.4f,
        37.5f, -122.25f, 12.3f);

    pushRxMessage(&cmd);
    handleMAVLinkTelemetry(1000);

    mavlink_message_t ackMsg;
    ASSERT_TRUE(popTxMessage(&ackMsg));
    ASSERT_EQ(ackMsg.msgid, MAVLINK_MSG_ID_COMMAND_ACK);

    mavlink_command_ack_t ack;
    mavlink_msg_command_ack_decode(&ackMsg, &ack);

    EXPECT_EQ(ack.command, MAV_CMD_DO_REPOSITION);
    EXPECT_EQ(ack.result, MAV_RESULT_ACCEPTED);
    EXPECT_EQ(setWaypointCalls, 1);
    EXPECT_EQ(lastWaypoint.lat, (int32_t)(37.5f * 1e7f));
    EXPECT_EQ(lastWaypoint.lon, (int32_t)(-122.25f * 1e7f));
    EXPECT_EQ(lastWaypoint.alt, (int32_t)(12.3f * 100.0f));
    EXPECT_EQ(lastWaypoint.p3, NAV_WP_ALTMODE);
    EXPECT_EQ(lastWaypoint.p1, 123);
}

TEST(MavlinkTelemetryTest, CommandIntUnsupportedFrameIsRejected)
{
    initMavlinkTestState();

    mavlink_message_t cmd;
    mavlink_msg_command_int_pack(
        42, 200, &cmd,
        1, MAV_COMP_ID_MISSIONPLANNER,
        MAV_FRAME_BODY_NED,
        MAV_CMD_DO_REPOSITION,
        0, 0,
        0, 0, 0, 0,
        100000000, 200000000, 10.0f);

    pushRxMessage(&cmd);
    handleMAVLinkTelemetry(1000);

    mavlink_message_t ackMsg;
    ASSERT_TRUE(popTxMessage(&ackMsg));
    ASSERT_EQ(ackMsg.msgid, MAVLINK_MSG_ID_COMMAND_ACK);

    mavlink_command_ack_t ack;
    mavlink_msg_command_ack_decode(&ackMsg, &ack);

    EXPECT_EQ(ack.command, MAV_CMD_DO_REPOSITION);
    EXPECT_EQ(ack.result, MAV_RESULT_UNSUPPORTED);
    EXPECT_EQ(setWaypointCalls, 0);
}

TEST(MavlinkTelemetryTest, CommandIntRepositionScalesCoordinates)
{
    initMavlinkTestState();

    mavlink_message_t cmd;
    mavlink_msg_command_int_pack(
        42, 200, &cmd,
        1, MAV_COMP_ID_MISSIONPLANNER,
        MAV_FRAME_GLOBAL_RELATIVE_ALT_INT,
        MAV_CMD_DO_REPOSITION,
        0, 0,
        0, 0, 0, 45.6f,
        375000000, -1222500000, 12.3f);

    pushRxMessage(&cmd);
    handleMAVLinkTelemetry(1000);

    mavlink_message_t ackMsg;
    ASSERT_TRUE(popTxMessage(&ackMsg));
    ASSERT_EQ(ackMsg.msgid, MAVLINK_MSG_ID_COMMAND_ACK);

    mavlink_command_ack_t ack;
    mavlink_msg_command_ack_decode(&ackMsg, &ack);

    EXPECT_EQ(ack.command, MAV_CMD_DO_REPOSITION);
    EXPECT_EQ(ack.result, MAV_RESULT_ACCEPTED);
    EXPECT_EQ(setWaypointCalls, 1);
    EXPECT_EQ(lastWaypoint.lat, 375000000);
    EXPECT_NEAR((double)lastWaypoint.lon, -1222500000.0, 100.0);
    EXPECT_EQ(lastWaypoint.alt, (int32_t)(12.3f * 100.0f));
    EXPECT_EQ(lastWaypoint.p3, 0);
    EXPECT_EQ(lastWaypoint.p1, 45);
}

TEST(MavlinkTelemetryTest, MissionClearAllAcksAndResets)
{
    initMavlinkTestState();

    mavlink_message_t msg;
    mavlink_msg_mission_clear_all_pack(
        42, 200, &msg,
        1, MAV_COMP_ID_MISSIONPLANNER, MAV_MISSION_TYPE_MISSION);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    EXPECT_EQ(resetWaypointCalls, 1);

    mavlink_message_t ackMsg;
    ASSERT_TRUE(popTxMessage(&ackMsg));
    ASSERT_EQ(ackMsg.msgid, MAVLINK_MSG_ID_MISSION_ACK);

    mavlink_mission_ack_t ack;
    mavlink_msg_mission_ack_decode(&ackMsg, &ack);

    EXPECT_EQ(ack.type, MAV_MISSION_ACCEPTED);
    EXPECT_EQ(ack.mission_type, MAV_MISSION_TYPE_MISSION);
}

TEST(MavlinkTelemetryTest, MissionCountRequestsFirstItem)
{
    initMavlinkTestState();

    mavlink_message_t msg;
    mavlink_msg_mission_count_pack(
        42, 200, &msg,
        1, MAV_COMP_ID_MISSIONPLANNER, 1, MAV_MISSION_TYPE_MISSION, 0);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    mavlink_message_t reqMsg;
    ASSERT_TRUE(popTxMessage(&reqMsg));
    ASSERT_EQ(reqMsg.msgid, MAVLINK_MSG_ID_MISSION_REQUEST_INT);

    mavlink_mission_request_int_t req;
    mavlink_msg_mission_request_int_decode(&reqMsg, &req);

    EXPECT_EQ(req.seq, 0);
    EXPECT_EQ(req.mission_type, MAV_MISSION_TYPE_MISSION);
}

TEST(MavlinkTelemetryTest, MissionItemIntSingleItemAcksAccepted)
{
    initMavlinkTestState();

    mavlink_message_t countMsg;
    mavlink_msg_mission_count_pack(
        42, 200, &countMsg,
        1, MAV_COMP_ID_MISSIONPLANNER, 1, MAV_MISSION_TYPE_MISSION, 0);

    pushRxMessage(&countMsg);
    handleMAVLinkTelemetry(1000);

    serialTxLen = 0;

    mavlink_message_t itemMsg;
    mavlink_msg_mission_item_int_pack(
        42, 200, &itemMsg,
        1, MAV_COMP_ID_MISSIONPLANNER, 0,
        MAV_FRAME_GLOBAL_RELATIVE_ALT_INT,
        MAV_CMD_NAV_WAYPOINT, 0, 1,
        0, 0, 0, 0,
        375000000, -1222500000, 12.3f,
        MAV_MISSION_TYPE_MISSION);

    pushRxMessage(&itemMsg);
    handleMAVLinkTelemetry(1000);

    mavlink_message_t ackMsg;
    ASSERT_TRUE(popTxMessage(&ackMsg));
    ASSERT_EQ(ackMsg.msgid, MAVLINK_MSG_ID_MISSION_ACK);

    mavlink_mission_ack_t ack;
    mavlink_msg_mission_ack_decode(&ackMsg, &ack);

    EXPECT_EQ(ack.type, MAV_MISSION_ACCEPTED);
    EXPECT_EQ(setWaypointCalls, 1);
    EXPECT_EQ(lastWaypoint.lat, 375000000);
    EXPECT_EQ(lastWaypoint.lon, -1222500000);
    EXPECT_EQ(lastWaypoint.alt, (int32_t)(12.3f * 100.0f));
}

TEST(MavlinkTelemetryTest, MissionRequestListSendsCount)
{
    initMavlinkTestState();
    waypointCount = 2;

    mavlink_message_t msg;
    mavlink_msg_mission_request_list_pack(
        42, 200, &msg,
        1, MAV_COMP_ID_MISSIONPLANNER, MAV_MISSION_TYPE_MISSION);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    mavlink_message_t countMsg;
    ASSERT_TRUE(popTxMessage(&countMsg));
    ASSERT_EQ(countMsg.msgid, MAVLINK_MSG_ID_MISSION_COUNT);

    mavlink_mission_count_t count;
    mavlink_msg_mission_count_decode(&countMsg, &count);

    EXPECT_EQ(count.count, 2);
    EXPECT_EQ(count.mission_type, MAV_MISSION_TYPE_MISSION);
}

TEST(MavlinkTelemetryTest, MissionRequestSendsWaypoint)
{
    initMavlinkTestState();
    waypointCount = 1;
    waypointStore[0].action = NAV_WP_ACTION_WAYPOINT;
    waypointStore[0].lat = 375000000;
    waypointStore[0].lon = -1222500000;
    waypointStore[0].alt = 1234;
    waypointStore[0].p3 = 0;

    mavlink_message_t msg;
    mavlink_msg_mission_request_pack(
        42, 200, &msg,
        1, MAV_COMP_ID_MISSIONPLANNER, 0, MAV_MISSION_TYPE_MISSION);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    mavlink_message_t itemMsg;
    ASSERT_TRUE(popTxMessage(&itemMsg));
    ASSERT_EQ(itemMsg.msgid, MAVLINK_MSG_ID_MISSION_ITEM);

    mavlink_mission_item_t item;
    mavlink_msg_mission_item_decode(&itemMsg, &item);

    EXPECT_EQ(item.seq, 0);
    EXPECT_EQ(item.command, MAV_CMD_NAV_WAYPOINT);
    EXPECT_EQ(item.frame, MAV_FRAME_GLOBAL_RELATIVE_ALT);
    EXPECT_NEAR(item.x, 37.5f, 1e-4f);
    EXPECT_NEAR(item.y, -122.25f, 1e-4f);
    EXPECT_NEAR(item.z, 12.34f, 1e-4f);
}

TEST(MavlinkTelemetryTest, LegacyGuidedMissionItemUsesAbsoluteAltitude)
{
    initMavlinkTestState();
    ENABLE_ARMING_FLAG(ARMED);

    mavlink_message_t msg;
    mavlink_msg_mission_item_pack(
        42, 200, &msg,
        1, MAV_COMP_ID_MISSIONPLANNER, 0,
        MAV_FRAME_GLOBAL,
        MAV_CMD_NAV_WAYPOINT, 2, 1,
        0, 0, 0, 0,
        37.5f, -122.25f, 12.3f,
        MAV_MISSION_TYPE_MISSION);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    EXPECT_EQ(setWaypointCalls, 1);
    EXPECT_EQ(lastWaypoint.p3, NAV_WP_ALTMODE);
}

TEST(MavlinkTelemetryTest, ParamRequestListRespondsWithEmptyParam)
{
    initMavlinkTestState();

    mavlink_message_t msg;
    mavlink_msg_param_request_list_pack(
        42, 200, &msg,
        1, MAV_COMP_ID_MISSIONPLANNER);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    mavlink_message_t paramMsg;
    ASSERT_TRUE(popTxMessage(&paramMsg));
    ASSERT_EQ(paramMsg.msgid, MAVLINK_MSG_ID_PARAM_VALUE);

    mavlink_param_value_t param;
    mavlink_msg_param_value_decode(&paramMsg, &param);

    EXPECT_EQ(param.param_count, 0);
    EXPECT_EQ(param.param_index, 0);
}

TEST(MavlinkTelemetryTest, SetPositionTargetGlobalIntSetsWaypoint)
{
    initMavlinkTestState();

    mavlink_message_t msg;
    mavlink_msg_set_position_target_global_int_pack(
        42, 200, &msg,
        0, 1, MAV_COMP_ID_MISSIONPLANNER,
        MAV_FRAME_GLOBAL_RELATIVE_ALT_INT, 0,
        375000000, -1222500000, 12.3f,
        0, 0, 0, 0, 0, 0, 0, 0);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    EXPECT_EQ(setWaypointCalls, 1);
    EXPECT_EQ(lastWaypoint.lat, 375000000);
    EXPECT_EQ(lastWaypoint.lon, -1222500000);
    EXPECT_EQ(lastWaypoint.alt, (int32_t)(12.3f * 100.0f));
    EXPECT_EQ(lastWaypoint.p3, 0);
}

TEST(MavlinkTelemetryTest, SetPositionTargetGlobalIntUsesAbsoluteAltitude)
{
    initMavlinkTestState();

    mavlink_message_t msg;
    mavlink_msg_set_position_target_global_int_pack(
        42, 200, &msg,
        0, 1, MAV_COMP_ID_MISSIONPLANNER,
        MAV_FRAME_GLOBAL_INT, 0,
        375000000, -1222500000, 12.3f,
        0, 0, 0, 0, 0, 0, 0, 0);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    EXPECT_EQ(setWaypointCalls, 1);
    EXPECT_EQ(lastWaypoint.p3, NAV_WP_ALTMODE);
}

TEST(MavlinkTelemetryTest, RequestDataStreamStopsStream)
{
    initMavlinkTestState();

    mavlink_message_t setMsg;
    mavlink_msg_request_data_stream_pack(
        42, 200, &setMsg,
        1, MAV_COMP_ID_MISSIONPLANNER,
        MAV_DATA_STREAM_RC_CHANNELS, 10, 1);

    pushRxMessage(&setMsg);
    handleMAVLinkTelemetry(1000);

    serialTxLen = 0;

    mavlink_message_t getMsg;
    mavlink_msg_command_long_pack(
        42, 200, &getMsg,
        1, MAV_COMP_ID_MISSIONPLANNER,
        MAV_CMD_GET_MESSAGE_INTERVAL,
        0,
        (float)MAVLINK_MSG_ID_RC_CHANNELS,
        0, 0, 0, 0, 0, 0);

    pushRxMessage(&getMsg);
    handleMAVLinkTelemetry(1000);

    mavlink_message_t intervalMsg;
    ASSERT_TRUE(popTxMessage(&intervalMsg));
    ASSERT_EQ(intervalMsg.msgid, MAVLINK_MSG_ID_MESSAGE_INTERVAL);

    mavlink_message_interval_t interval;
    mavlink_msg_message_interval_decode(&intervalMsg, &interval);

    EXPECT_EQ(interval.message_id, MAVLINK_MSG_ID_RC_CHANNELS);
    EXPECT_EQ(interval.interval_us, 100000);

    serialTxLen = 0;

    mavlink_message_t stopMsg;
    mavlink_msg_request_data_stream_pack(
        42, 200, &stopMsg,
        1, MAV_COMP_ID_MISSIONPLANNER,
        MAV_DATA_STREAM_RC_CHANNELS, 0, 0);

    pushRxMessage(&stopMsg);
    handleMAVLinkTelemetry(1000);

    serialTxLen = 0;

    pushRxMessage(&getMsg);
    handleMAVLinkTelemetry(1000);

    ASSERT_TRUE(popTxMessage(&intervalMsg));
    ASSERT_EQ(intervalMsg.msgid, MAVLINK_MSG_ID_MESSAGE_INTERVAL);

    mavlink_msg_message_interval_decode(&intervalMsg, &interval);

    EXPECT_EQ(interval.message_id, MAVLINK_MSG_ID_RC_CHANNELS);
    EXPECT_EQ(interval.interval_us, -1);
}

TEST(MavlinkTelemetryTest, RequestProtocolVersionUsesConfiguredVersion)
{
    initMavlinkTestState();

    mavlink_message_t msg;
    mavlink_msg_command_long_pack(
        42, 200, &msg,
        1, MAV_COMP_ID_MISSIONPLANNER,
        MAV_CMD_REQUEST_PROTOCOL_VERSION,
        0,
        0, 0, 0, 0, 0, 0, 0);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    mavlink_message_t versionMsg;
    ASSERT_TRUE(popTxMessage(&versionMsg));
    ASSERT_EQ(versionMsg.msgid, MAVLINK_MSG_ID_PROTOCOL_VERSION);

    mavlink_protocol_version_t version;
    mavlink_msg_protocol_version_decode(&versionMsg, &version);

    EXPECT_EQ(version.version, 200);
    EXPECT_EQ(version.min_version, 200);
    EXPECT_EQ(version.max_version, 200);
}

TEST(MavlinkTelemetryTest, BatteryStatusDoesNotSendExtendedSysState)
{
    initMavlinkTestState();

    mavlinkSendBatteryTemperatureStatusText();

    mavlink_status_t status;
    memset(&status, 0, sizeof(status));
    mavlink_message_t msg;
    bool sawExtSysState = false;

    for (size_t i = 0; i < serialTxLen; i++) {
        if (mavlink_parse_char(0, serialTxBuffer[i], &msg, &status) == MAVLINK_FRAMING_OK) {
            if (msg.msgid == MAVLINK_MSG_ID_EXTENDED_SYS_STATE) {
                sawExtSysState = true;
                break;
            }
        }
    }

    EXPECT_FALSE(sawExtSysState);
}

TEST(MavlinkTelemetryTest, RadioStatusUpdatesRxLinkStats)
{
    initMavlinkTestState();
    rxConfigMutable()->receiverType = RX_TYPE_SERIAL;
    rxConfigMutable()->serialrx_provider = SERIALRX_MAVLINK;
    telemetryConfigMutable()->mavlink.radio_type = MAVLINK_RADIO_ELRS;

    mavlink_message_t msg;
    mavlink_msg_radio_status_pack(
        42, 200, &msg,
        200, 150, 255, 7, 3, 0, 0);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    EXPECT_EQ(rxLinkStatistics.uplinkRSSI, -150);
    EXPECT_EQ(rxLinkStatistics.uplinkSNR, 7);
    EXPECT_EQ(rxLinkStatistics.uplinkLQ, scaleRange(200, 0, 255, 0, 100));
}

TEST(MavlinkTelemetryTest, RcChannelsOverrideIsForwarded)
{
    initMavlinkTestState();

    mavlink_message_t msg;
    mavlink_msg_rc_channels_override_pack(
        42, 200, &msg,
        1, MAV_COMP_ID_MISSIONPLANNER,
        1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    EXPECT_EQ(mavlinkRxHandleCalls, 1);
}

extern "C" {

int32_t debug[DEBUG32_VALUE_COUNT];

uint32_t armingFlags;
uint32_t stateFlags;

attitudeEulerAngles_t attitude;
gyro_t gyro;
gpsSolutionData_t gpsSol;
gpsLocation_t GPS_home;
navSystemStatus_t NAV_Status;
rxRuntimeConfig_t rxRuntimeConfig;
rxLinkStatistics_t rxLinkStatistics;
uint16_t averageSystemLoadPercent;

uint32_t micros(void)
{
    return 0;
}

uint32_t millis(void)
{
    return 0;
}

serialPortConfig_t *findSerialPortConfig(serialPortFunction_e function)
{
    UNUSED(function);
    testPortConfig.identifier = SERIAL_PORT_USART1;
    testPortConfig.telemetry_baudrateIndex = BAUD_115200;
    return &testPortConfig;
}

portSharing_e determinePortSharing(const serialPortConfig_t *portConfig, serialPortFunction_e function)
{
    UNUSED(portConfig);
    UNUSED(function);
    return PORTSHARING_NOT_SHARED;
}

serialPort_t *openSerialPort(serialPortIdentifier_e identifier, serialPortFunction_e function,
                             serialReceiveCallbackPtr rxCallback, void *rxCallbackData,
                             uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    UNUSED(identifier);
    UNUSED(function);
    UNUSED(rxCallback);
    UNUSED(rxCallbackData);
    UNUSED(baudRate);
    UNUSED(mode);
    UNUSED(options);
    return &testSerialPort;
}

void closeSerialPort(serialPort_t *serialPort)
{
    UNUSED(serialPort);
}

uint32_t serialRxBytesWaiting(const serialPort_t *instance)
{
    UNUSED(instance);
    return (uint32_t)(serialRxLen - serialRxPos);
}

uint32_t serialTxBytesFree(const serialPort_t *instance)
{
    UNUSED(instance);
    return 1024;
}

uint8_t serialRead(serialPort_t *instance)
{
    UNUSED(instance);
    return serialRxBuffer[serialRxPos++];
}

void serialWrite(serialPort_t *instance, uint8_t ch)
{
    UNUSED(instance);
    serialTxBuffer[serialTxLen++] = ch;
}

void serialSetMode(serialPort_t *instance, portMode_t mode)
{
    UNUSED(instance);
    UNUSED(mode);
}

bool telemetryDetermineEnabledState(portSharing_e portSharing)
{
    UNUSED(portSharing);
    return true;
}

bool sensors(uint32_t mask)
{
    UNUSED(mask);
    return false;
}

bool isAmperageConfigured(void)
{
    return false;
}

bool feature(uint32_t mask)
{
    UNUSED(mask);
    return false;
}

int16_t getAmperage(void)
{
    return 0;
}

int32_t getMAhDrawn(void)
{
    return 0;
}

int32_t getMWhDrawn(void)
{
    return 0;
}

uint8_t getBatteryCellCount(void)
{
    return 0;
}

uint16_t getBatteryAverageCellVoltage(void)
{
    return 0;
}

uint16_t getBatteryVoltage(void)
{
    return 0;
}

int16_t getThrottlePercent(bool scaled)
{
    UNUSED(scaled);
    return 0;
}

bool osdUsingScaledThrottle(void)
{
    return false;
}

float getEstimatedActualPosition(int axis)
{
    UNUSED(axis);
    return 0.0f;
}

float getEstimatedActualVelocity(int axis)
{
    UNUSED(axis);
    return 0.0f;
}

float getAirspeedEstimate(void)
{
    return 0.0f;
}

bool pitotIsHealthy(void)
{
    return false;
}

bool rxIsReceivingSignal(void)
{
    return false;
}

bool rxAreFlightChannelsValid(void)
{
    return false;
}

uint16_t getRSSI(void)
{
    return 0;
}

int16_t rxGetChannelValue(unsigned channel)
{
    UNUSED(channel);
    return 1500;
}

hardwareSensorStatus_e getHwGyroStatus(void) { return HW_SENSOR_NONE; }
hardwareSensorStatus_e getHwAccelerometerStatus(void) { return HW_SENSOR_NONE; }
hardwareSensorStatus_e getHwCompassStatus(void) { return HW_SENSOR_NONE; }
hardwareSensorStatus_e getHwBarometerStatus(void) { return HW_SENSOR_NONE; }
hardwareSensorStatus_e getHwGPSStatus(void) { return HW_SENSOR_NONE; }
hardwareSensorStatus_e getHwRangefinderStatus(void) { return HW_SENSOR_NONE; }
hardwareSensorStatus_e getHwPitotmeterStatus(void) { return HW_SENSOR_NONE; }
hardwareSensorStatus_e getHwOpticalFlowStatus(void) { return HW_SENSOR_NONE; }

bool getBaroTemperature(int16_t *temperature)
{
    *temperature = 0;
    return false;
}

bool getIMUTemperature(int16_t *temperature)
{
    *temperature = 0;
    return false;
}

bool areSensorsCalibrating(void)
{
    return false;
}

bool failsafeIsActive(void)
{
    return false;
}

failsafePhase_e failsafePhase(void)
{
    return FAILSAFE_IDLE;
}

int isGCSValid(void)
{
    return gcsValid;
}

void setWaypoint(uint8_t wpNumber, const navWaypoint_t *wp)
{
    UNUSED(wpNumber);
    lastWaypoint = *wp;
    setWaypointCalls++;
}

int getWaypointCount(void)
{
    return waypointCount;
}

void getWaypoint(uint8_t wpNumber, navWaypoint_t *wp)
{
    if (wpNumber == 0 || wpNumber > ARRAYLEN(waypointStore)) {
        memset(wp, 0, sizeof(*wp));
        return;
    }
    *wp = waypointStore[wpNumber - 1];
}

bool isWaypointListValid(void)
{
    return true;
}

void resetWaypointList(void)
{
    resetWaypointCalls++;
    waypointCount = 0;
    memset(waypointStore, 0, sizeof(waypointStore));
}

flightModeForTelemetry_e getFlightModeForTelemetry(void)
{
    return FLM_MANUAL;
}

bool isModeActivationConditionPresent(boxId_e modeId)
{
    UNUSED(modeId);
    return false;
}

textAttributes_t osdGetSystemMessage(char *message, size_t length, bool remove)
{
    UNUSED(length);
    UNUSED(remove);
    message[0] = 0x20;
    message[1] = '\0';
    return 0;
}

void mavlinkRxHandleMessage(const mavlink_rc_channels_override_t *msg)
{
    UNUSED(msg);
    mavlinkRxHandleCalls++;
}

adsbVehicleValues_t* getVehicleForFill(void)
{
    return NULL;
}

void adsbNewVehicle(adsbVehicleValues_t *vehicle)
{
    UNUSED(vehicle);
}

bool adsbHeartbeat(void)
{
    return false;
}

uint8_t calculateBatteryPercentage(void)
{
    return 0;
}

}
