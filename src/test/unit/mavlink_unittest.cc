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
#include <vector>

extern "C" {
    #include "platform.h"

    #include "build/debug.h"

    #include "common/axis.h"
    #include "common/maths.h"
    #include "common/streambuf.h"
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

    #include "msp/msp.h"
    #include "msp/msp_protocol.h"
    #include "msp/msp_serial.h"

    #include "mavlink/mavlink_runtime.h"
    #include "mavlink/mavlink_streams.h"
    #include "navigation/navigation.h"
#ifdef __cplusplus
    #define _Static_assert static_assert
#endif
    #include "navigation/navigation_private.h"
#ifdef __cplusplus
    #undef _Static_assert
#endif

    #include "rx/rx.h"

    #include "sensors/barometer.h"
    #include "sensors/battery.h"
    #include "sensors/diagnostics.h"
    #include "sensors/gyro.h"
    #include "sensors/pitotmeter.h"
    #include "sensors/sensors.h"
    #include "sensors/temperature.h"

    #include "mavlink/mavlink_types.h"
    #include "telemetry/mavlink.h"
    #include "telemetry/telemetry.h"

    void mavlinkSendAttitude(void);
    void mavlinkSendHeartbeat(void);
    void mavlinkSendBatteryTemperatureStatusText(void);
    bool mavlinkSendStatusText(void);
    void mavlinkSendPosition(timeUs_t currentTimeUs);

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
static const uint8_t testTargetComponent = MAV_COMP_ID_AUTOPILOT1;
static const uint8_t testTunnelSourceSystem = 42;
static const uint8_t testTunnelSourceComponent = 200;
static const uint8_t testSimpleMspCommand = 90;
static const uint8_t testLargeReplyMspCommand = 91;
static const size_t testMspFrameBufSize = MSP_PORT_OUTBUF_SIZE + 16;
static const timeMs_t testMissionUploadRetryMs = 1500;
static timeMs_t fakeMillis;
static timeUs_t fakeMicros;
static int mspCommandCallCount;
static int mspPassthroughDispatchCount;
static int mspRebootPostProcessCount;
static int waitForSerialPortToFinishTransmittingCalls;
static serialPort_t *lastPostProcessPort;

const uint32_t baudRates[] = {
    0, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 250000,
    460800, 921600, 1000000, 1500000, 2000000, 2470000
};

static navWaypoint_t lastWaypoint;
static uint8_t lastWaypointNumber;
static int setWaypointCalls;
static int resetWaypointCalls;
static int saveWaypointCalls;
static bool saveWaypointResult;
static int mavlinkRxHandleCalls;
static bool gcsValid;
static int waypointCount;
static navWaypoint_t waypointStore[4];
static float estimatedPosition[XYZ_AXIS_COUNT];
static float estimatedVelocity[XYZ_AXIS_COUNT];
static int altitudeTargetSetCalls;
static bool altitudeTargetSetResult;
static geoAltitudeDatumFlag_e lastAltitudeTargetDatum;
static int32_t lastAltitudeTargetCm;
static flightModeForTelemetry_e testFlightMode;
static uint32_t testSensorsMask;
static int setArmStateCalls;
static bool requestedArmState;
static bool setArmStateResult;
static int activateRthCalls;
static rthState_e forcedRthState;
static int activatePositionHoldCalls;
static bool activatePositionHoldResult;
static uint32_t lastLoiterRadiusOverride;
static int activateLandingCalls;
static bool activateLandingResult;
static bool canSetHome;
static bool testModeActivationConditions[CHECKBOX_ITEM_COUNT];
static char testOsdSystemMessage[MAVLINK_MSG_STATUSTEXT_FIELD_TEXT_LEN + 1];
static textAttributes_t testOsdSystemMessageAttributes;

static void resetSerialBuffers(void)
{
    serialRxLen = 0;
    serialRxPos = 0;
    serialTxLen = 0;
}

static std::vector<uint8_t> makeMspV1Request(uint8_t cmd, const std::vector<uint8_t> &payload = {})
{
    std::vector<uint8_t> frame = {
        '$', 'M', '<',
        static_cast<uint8_t>(payload.size()),
        cmd
    };
    uint8_t checksum = frame[3] ^ frame[4];
    for (const uint8_t byte : payload) {
        frame.push_back(byte);
        checksum ^= byte;
    }
    frame.push_back(checksum);
    return frame;
}

static void pushRxMessage(const mavlink_message_t *msg)
{
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    int length = mavlink_msg_to_send_buffer(buffer, msg);
    memcpy(&serialRxBuffer[serialRxLen], buffer, (size_t)length);
    serialRxLen += (size_t)length;
}

static void handleMavlinkUntilRxEmpty(timeUs_t currentTimeUs)
{
    while (serialRxPos < serialRxLen) {
        handleMAVLinkTelemetry(currentTimeUs);
    }
}

static void pushTunnelPayload(uint8_t payloadLength, const std::vector<uint8_t> &payload, uint8_t targetComponent = testTargetComponent)
{
    uint8_t tunnelPayload[MAVLINK_MSG_TUNNEL_FIELD_PAYLOAD_LEN] = { 0 };
    size_t copyLength = payload.size();
    if (copyLength > MAVLINK_MSG_TUNNEL_FIELD_PAYLOAD_LEN) {
        copyLength = MAVLINK_MSG_TUNNEL_FIELD_PAYLOAD_LEN;
    }
    if (copyLength > 0) {
        memcpy(tunnelPayload, payload.data(), copyLength);
    }

    mavlink_message_t msg;
    mavlink_msg_tunnel_pack(
        testTunnelSourceSystem,
        testTunnelSourceComponent,
        &msg,
        telemetryConfig()->mavlink_common.sysid,
        targetComponent,
        0x8001,
        payloadLength,
        tunnelPayload);
    pushRxMessage(&msg);
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

static bool findTxMessageById(uint32_t msgid, mavlink_message_t *match)
{
    mavlink_status_t status;
    memset(&status, 0, sizeof(status));

    mavlink_message_t msg;
    for (size_t i = 0; i < serialTxLen; i++) {
        if (mavlink_parse_char(0, serialTxBuffer[i], &msg, &status) == MAVLINK_FRAMING_OK && msg.msgid == msgid) {
            *match = msg;
            return true;
        }
    }

    return false;
}

static std::vector<mavlink_message_t> parseTxMessages(void)
{
    std::vector<mavlink_message_t> messages;
    mavlink_status_t status;
    memset(&status, 0, sizeof(status));

    mavlink_message_t msg;
    for (size_t i = 0; i < serialTxLen; i++) {
        if (mavlink_parse_char(0, serialTxBuffer[i], &msg, &status) == MAVLINK_FRAMING_OK) {
            messages.push_back(msg);
        }
    }

    return messages;
}

static std::vector<uint8_t> collectTunnelPayload(const std::vector<mavlink_message_t> &messages)
{
    std::vector<uint8_t> payload;

    for (const mavlink_message_t &msg : messages) {
        EXPECT_EQ(msg.msgid, MAVLINK_MSG_ID_TUNNEL);

        mavlink_tunnel_t tunnel;
        mavlink_msg_tunnel_decode(&msg, &tunnel);

        EXPECT_EQ(tunnel.payload_type, 0x8001);
        EXPECT_EQ(tunnel.target_system, testTunnelSourceSystem);
        EXPECT_EQ(tunnel.target_component, testTunnelSourceComponent);

        payload.insert(payload.end(), tunnel.payload, tunnel.payload + tunnel.payload_length);
    }

    return payload;
}

static void initMavlinkTestState(void)
{
    resetSerialBuffers();
    fakeMillis = 0;
    fakeMicros = 0;
    setWaypointCalls = 0;
    lastWaypointNumber = 0;
    resetWaypointCalls = 0;
    saveWaypointCalls = 0;
    saveWaypointResult = true;
    mavlinkRxHandleCalls = 0;
    mspCommandCallCount = 0;
    mspPassthroughDispatchCount = 0;
    mspRebootPostProcessCount = 0;
    waitForSerialPortToFinishTransmittingCalls = 0;
    lastPostProcessPort = NULL;
    gcsValid = true;
    waypointCount = 0;
    memset(estimatedPosition, 0, sizeof(estimatedPosition));
    memset(estimatedVelocity, 0, sizeof(estimatedVelocity));
    altitudeTargetSetCalls = 0;
    altitudeTargetSetResult = true;
    lastAltitudeTargetDatum = NAV_WP_TAKEOFF_DATUM;
    lastAltitudeTargetCm = 0;
    testFlightMode = FLM_MANUAL;
    testSensorsMask = 0;
    setArmStateCalls = 0;
    requestedArmState = false;
    setArmStateResult = true;
    activateRthCalls = 0;
    forcedRthState = RTH_IDLE;
    activatePositionHoldCalls = 0;
    activatePositionHoldResult = true;
    lastLoiterRadiusOverride = 0;
    activateLandingCalls = 0;
    activateLandingResult = true;
    canSetHome = true;
    memset(testModeActivationConditions, 0, sizeof(testModeActivationConditions));
    strcpy(testOsdSystemMessage, " ");
    testOsdSystemMessageAttributes = TEXT_ATTRIBUTES_NONE;
    armingFlags = 0;
    stateFlags = 0;
    flightModeFlags = 0;
    memset(&gpsSol, 0, sizeof(gpsSol));
    memset(&GPS_home, 0, sizeof(GPS_home));
    memset(waypointStore, 0, sizeof(waypointStore));
    memset(&rxLinkStatistics, 0, sizeof(rxLinkStatistics));

    telemetryConfigMutable()->mavlink_common.sysid = 1;
    telemetryConfigMutable()->mavlink_common.autopilot_type = MAVLINK_AUTOPILOT_ARDUPILOT;
    telemetryConfigMutable()->mavlink_common.version = 2;
    telemetryConfigMutable()->mavlink[0].extended_status_rate = 2;
    telemetryConfigMutable()->mavlink[0].rc_channels_rate = 1;
    telemetryConfigMutable()->mavlink[0].position_rate = 2;
    telemetryConfigMutable()->mavlink[0].extra1_rate = 2;
    telemetryConfigMutable()->mavlink[0].extra2_rate = 2;
    telemetryConfigMutable()->mavlink[0].extra3_rate = 1;
    telemetryConfigMutable()->mavlink[0].min_txbuff = 0;
    telemetryConfigMutable()->halfDuplex = 0;

    rxConfigMutable()->receiverType = RX_TYPE_NONE;
    rxConfigMutable()->serialrx_provider = SERIALRX_SBUS;
    rxConfigMutable()->halfDuplex = 0;

    systemConfigMutable()->current_mixer_profile_index = 0;
    mixerProfilesMutable(0)->mixer_config.platformType = PLATFORM_AIRPLANE;

    rxRuntimeConfig.channelCount = 8;

    initMAVLinkTelemetry();
    checkMAVLinkTelemetryState();
}






TEST(MavlinkTelemetryTest, MlrsRadioLinkStatsUpdateRxStatisticsOnMavlinkSerialRxPort)
{
    initMavlinkTestState();

    telemetryConfigMutable()->mavlink[0].radio_type = MAVLINK_RADIO_MLRS;
    rxConfigMutable()->receiverType = RX_TYPE_SERIAL;
    rxConfigMutable()->serialrx_provider = SERIALRX_MAVLINK;
    testPortConfig.functionMask |= FUNCTION_RX_SERIAL;

    mavlink_message_t msg;
    mavlink_msg_mlrs_radio_link_stats_pack(
        testTunnelSourceSystem,
        MAV_COMP_ID_TELEMETRY_RADIO,
        &msg,
        telemetryConfig()->mavlink_common.sysid,
        testTargetComponent,
        MLRS_RADIO_LINK_STATS_FLAGS_RSSI_DBM | MLRS_RADIO_LINK_STATS_FLAGS_RX_RECEIVE_ANTENNA2,
        91,
        77,
        100,
        7,
        55,
        120,
        9,
        111,
        11,
        130,
        13,
        0.0f,
        0.0f);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    const mavlinkMlrsPortRuntime_t *mlrs = mavlinkGetPortMlrsRuntime(0);
    ASSERT_NE(mlrs, nullptr);
    EXPECT_TRUE(mlrs->stats.valid);
    EXPECT_TRUE(mlrs->stats.rssiIsDbm);
    EXPECT_EQ(mlrs->stats.rxLinkQualityRc, 91);
    EXPECT_EQ(mlrs->stats.rxLinkQualitySerial, 77);
    EXPECT_EQ(mlrs->stats.activeAntenna, 1);
    EXPECT_EQ(mlrs->stats.rxRssi, -111);
    EXPECT_EQ(mlrs->stats.rxSnr, 11);
    EXPECT_EQ(rxLinkStatistics.uplinkLQ, 91);
    EXPECT_EQ(rxLinkStatistics.downlinkLQ, 77);
    EXPECT_EQ(rxLinkStatistics.uplinkRSSI, -111);
    EXPECT_EQ(rxLinkStatistics.uplinkSNR, 11);
    EXPECT_EQ(rxLinkStatistics.activeAntenna, 1);
}

TEST(MavlinkTelemetryTest, MlrsRadioLinkStatsStayCachedOffReceiverPort)
{
    initMavlinkTestState();

    telemetryConfigMutable()->mavlink[0].radio_type = MAVLINK_RADIO_MLRS;

    mavlink_message_t msg;
    mavlink_msg_mlrs_radio_link_stats_pack(
        testTunnelSourceSystem,
        MAV_COMP_ID_TELEMETRY_RADIO,
        &msg,
        telemetryConfig()->mavlink_common.sysid,
        testTargetComponent,
        MLRS_RADIO_LINK_STATS_FLAGS_RSSI_DBM,
        65,
        44,
        101,
        6,
        33,
        121,
        8,
        0,
        INT8_MAX,
        0,
        INT8_MAX,
        0.0f,
        0.0f);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    const mavlinkMlrsPortRuntime_t *mlrs = mavlinkGetPortMlrsRuntime(0);
    ASSERT_NE(mlrs, nullptr);
    EXPECT_TRUE(mlrs->stats.valid);
    EXPECT_EQ(mlrs->stats.rxLinkQualityRc, 65);
    EXPECT_EQ(mlrs->stats.rxLinkQualitySerial, 44);
    EXPECT_EQ(mlrs->stats.rxRssi, -101);
    EXPECT_EQ(mlrs->stats.rxSnr, 6);
    EXPECT_EQ(rxLinkStatistics.uplinkLQ, 0);
    EXPECT_EQ(rxLinkStatistics.downlinkLQ, 0);
    EXPECT_EQ(rxLinkStatistics.uplinkRSSI, 0);
    EXPECT_EQ(rxLinkStatistics.uplinkSNR, 0);
}

TEST(MavlinkTelemetryTest, MlrsRadioLinkInformationUpdatesReceiverFacingMetadata)
{
    initMavlinkTestState();

    telemetryConfigMutable()->mavlink[0].radio_type = MAVLINK_RADIO_MLRS;
    rxConfigMutable()->receiverType = RX_TYPE_SERIAL;
    rxConfigMutable()->serialrx_provider = SERIALRX_MAVLINK;
    testPortConfig.functionMask |= FUNCTION_RX_SERIAL;

    mavlink_message_t msg;
    mavlink_msg_mlrs_radio_link_information_pack(
        testTunnelSourceSystem,
        MAV_COMP_ID_TELEMETRY_RADIO,
        &msg,
        telemetryConfig()->mavlink_common.sysid,
        testTargetComponent,
        MLRS_RADIO_LINK_TYPE_MLRS,
        3,
        20,
        10,
        50,
        50,
        "50HZ",
        "915",
        9600,
        4800,
        115,
        117);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    const mavlinkMlrsPortRuntime_t *mlrs = mavlinkGetPortMlrsRuntime(0);
    ASSERT_NE(mlrs, nullptr);
    EXPECT_TRUE(mlrs->info.valid);
    EXPECT_STREQ(mlrs->info.modeStr, "50HZ");
    EXPECT_STREQ(mlrs->info.bandStr, "915");
    EXPECT_EQ(mlrs->info.txPowerMw, 100);
    EXPECT_EQ(mlrs->info.rxPowerMw, 10);
    EXPECT_EQ(mlrs->info.txReceiveSensitivityDbm, -115);
    EXPECT_EQ(mlrs->info.rxReceiveSensitivityDbm, -117);
    EXPECT_EQ(rxLinkStatistics.uplinkTXPower, 100);
    EXPECT_EQ(rxLinkStatistics.downlinkTXPower, 10);
    EXPECT_STREQ(rxLinkStatistics.band, "915");
    EXPECT_STREQ(rxLinkStatistics.mode, "50HZ");
}

TEST(MavlinkTelemetryTest, MlrsFlowControlUsesIngressPortAndAcceptsZeroTxbuf)
{
    initMavlinkTestState();

    mavlink_message_t msg;
    mavlink_msg_mlrs_radio_link_flow_control_pack(
        testTunnelSourceSystem,
        MAV_COMP_ID_TELEMETRY_RADIO,
        &msg,
        9600,
        4800,
        90,
        40,
        0);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    const mavlinkMlrsPortRuntime_t *mlrs = mavlinkGetPortMlrsRuntime(0);
    ASSERT_NE(mlrs, nullptr);
    EXPECT_TRUE(mlrs->flowControl.valid);
    EXPECT_EQ(mlrs->flowControl.packet.txbuf, 0);
    EXPECT_TRUE(mavlinkPortTxBufferIsValid(0));
    EXPECT_EQ(mavlinkPortTxBufferFree(0), 0);
}

TEST(MavlinkTelemetryTest, MlrsMessagesRequireTelemetryRadioComponent)
{
    initMavlinkTestState();

    telemetryConfigMutable()->mavlink[0].radio_type = MAVLINK_RADIO_MLRS;
    rxConfigMutable()->receiverType = RX_TYPE_SERIAL;
    rxConfigMutable()->serialrx_provider = SERIALRX_MAVLINK;
    testPortConfig.functionMask |= FUNCTION_RX_SERIAL;

    mavlink_message_t statsMsg;
    mavlink_msg_mlrs_radio_link_stats_pack(
        testTunnelSourceSystem,
        testTunnelSourceComponent,
        &statsMsg,
        telemetryConfig()->mavlink_common.sysid,
        testTargetComponent,
        MLRS_RADIO_LINK_STATS_FLAGS_RSSI_DBM,
        91,
        77,
        100,
        7,
        55,
        120,
        9,
        0,
        INT8_MAX,
        0,
        INT8_MAX,
        0.0f,
        0.0f);
    pushRxMessage(&statsMsg);

    mavlink_message_t flowControlMsg;
    mavlink_msg_mlrs_radio_link_flow_control_pack(
        testTunnelSourceSystem,
        testTunnelSourceComponent,
        &flowControlMsg,
        9600,
        4800,
        90,
        40,
        10);
    pushRxMessage(&flowControlMsg);

    handleMAVLinkTelemetry(1000);

    const mavlinkMlrsPortRuntime_t *mlrs = mavlinkGetPortMlrsRuntime(0);
    ASSERT_NE(mlrs, nullptr);
    EXPECT_FALSE(mlrs->stats.valid);
    EXPECT_FALSE(mlrs->flowControl.valid);
    EXPECT_EQ(rxLinkStatistics.uplinkLQ, 0);
    EXPECT_FALSE(mavlinkPortTxBufferIsValid(0));
    EXPECT_EQ(mavlinkPortTxBufferFree(0), 100);
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
        1, testTargetComponent,
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













TEST(MavlinkTelemetryTest, BroadcastCommandLongRepositionExecutesLocally)
{
    initMavlinkTestState();

    mavlink_message_t cmd;
    mavlink_msg_command_long_pack(
        42, 200, &cmd,
        0, 0,
        MAV_CMD_DO_REPOSITION,
        0,
        0, 0, 0, 123.4f,
        37.5f, -122.25f, 12.3f);

    pushRxMessage(&cmd);
    handleMAVLinkTelemetry(1000);

    mavlink_message_t ackMsg;
    ASSERT_TRUE(findTxMessageById(MAVLINK_MSG_ID_COMMAND_ACK, &ackMsg));

    mavlink_command_ack_t ack;
    mavlink_msg_command_ack_decode(&ackMsg, &ack);

    EXPECT_EQ(ack.command, MAV_CMD_DO_REPOSITION);
    EXPECT_EQ(ack.result, MAV_RESULT_ACCEPTED);
    EXPECT_EQ(setWaypointCalls, 1);
}

TEST(MavlinkTelemetryTest, SameSystemDifferentComponentIsNotDroppedAsLocalIdentity)
{
    initMavlinkTestState();

    mavlink_message_t cmd;
    mavlink_msg_command_long_pack(
        1, 42, &cmd,
        1, testTargetComponent,
        MAV_CMD_DO_REPOSITION,
        0,
        0, 0, 0, 0,
        37.5f, -122.25f, 12.3f);

    pushRxMessage(&cmd);
    handleMAVLinkTelemetry(1000);

    mavlink_message_t ackMsg;
    ASSERT_TRUE(findTxMessageById(MAVLINK_MSG_ID_COMMAND_ACK, &ackMsg));

    mavlink_command_ack_t ack;
    mavlink_msg_command_ack_decode(&ackMsg, &ack);

    EXPECT_EQ(ack.command, MAV_CMD_DO_REPOSITION);
    EXPECT_EQ(ack.result, MAV_RESULT_ACCEPTED);
    EXPECT_EQ(setWaypointCalls, 1);
}

TEST(MavlinkTelemetryTest, CommandIntUnsupportedFrameIsRejected)
{
    initMavlinkTestState();

    mavlink_message_t cmd;
    mavlink_msg_command_int_pack(
        42, 200, &cmd,
        1, testTargetComponent,
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
        1, testTargetComponent,
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

















TEST(MavlinkTelemetryTest, ParamRequestListRespondsWithEmptyParam)
{
    initMavlinkTestState();

    mavlink_message_t msg;
    mavlink_msg_param_request_list_pack(
        42, 200, &msg,
        1, testTargetComponent);

    pushRxMessage(&msg);
    handleMavlinkUntilRxEmpty(1000);

    mavlink_message_t paramMsg;
    ASSERT_TRUE(popTxMessage(&paramMsg));
    ASSERT_EQ(paramMsg.msgid, MAVLINK_MSG_ID_PARAM_VALUE);

    mavlink_param_value_t param;
    mavlink_msg_param_value_decode(&paramMsg, &param);

    EXPECT_EQ(param.param_count, 0);
    EXPECT_EQ(param.param_index, 0);
}

TEST(MavlinkTelemetryTest, BroadcastParamRequestListRespondsWithEmptyParam)
{
    initMavlinkTestState();

    mavlink_message_t msg;
    mavlink_msg_param_request_list_pack(
        42, 200, &msg,
        0, 0);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    mavlink_message_t paramMsg;
    ASSERT_TRUE(findTxMessageById(MAVLINK_MSG_ID_PARAM_VALUE, &paramMsg));

    mavlink_param_value_t param;
    mavlink_msg_param_value_decode(&paramMsg, &param);

    EXPECT_EQ(param.param_count, 0);
    EXPECT_EQ(param.param_index, 0);
}

















TEST(MavlinkTelemetryTest, HeartbeatGuidedFlagRequiresValidGcsInPoshold)
{
    initMavlinkTestState();
    testFlightMode = FLM_POSITION_HOLD;
    gcsValid = false;

    mavlinkSendHeartbeat();

    mavlink_message_t msg;
    ASSERT_TRUE(popTxMessage(&msg));
    ASSERT_EQ(msg.msgid, MAVLINK_MSG_ID_HEARTBEAT);

    mavlink_heartbeat_t heartbeat;
    mavlink_msg_heartbeat_decode(&msg, &heartbeat);
    EXPECT_EQ((heartbeat.base_mode & MAV_MODE_FLAG_GUIDED_ENABLED), 0);

    serialTxLen = 0;
    gcsValid = true;

    mavlinkSendHeartbeat();

    ASSERT_TRUE(popTxMessage(&msg));
    ASSERT_EQ(msg.msgid, MAVLINK_MSG_ID_HEARTBEAT);

    mavlink_msg_heartbeat_decode(&msg, &heartbeat);
    EXPECT_NE((heartbeat.base_mode & MAV_MODE_FLAG_GUIDED_ENABLED), 0);
}

TEST(MavlinkTelemetryTest, PositionReportsPositiveDownVelocity)
{
    initMavlinkTestState();
    testSensorsMask = SENSOR_GPS;
    gpsSol.fixType = GPS_FIX_3D;
    gpsSol.llh.lat = 375000000;
    gpsSol.llh.lon = -1222500000;
    gpsSol.llh.alt = 12345;
    estimatedVelocity[Z] = 321.0f;

    mavlinkSendPosition(1000);

    mavlink_status_t status;
    memset(&status, 0, sizeof(status));
    mavlink_message_t msg;
    bool sawGlobalPosition = false;

    for (size_t i = 0; i < serialTxLen; i++) {
        if (mavlink_parse_char(0, serialTxBuffer[i], &msg, &status) == MAVLINK_FRAMING_OK) {
            if (msg.msgid == MAVLINK_MSG_ID_GLOBAL_POSITION_INT) {
                mavlink_global_position_int_t position;
                mavlink_msg_global_position_int_decode(&msg, &position);
                EXPECT_EQ(position.vz, -321);
                sawGlobalPosition = true;
            }
        }
    }

    EXPECT_TRUE(sawGlobalPosition);
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
    telemetryConfigMutable()->mavlink[0].radio_type = MAVLINK_RADIO_ELRS;

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
        1, testTargetComponent,
        1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    EXPECT_EQ(mavlinkRxHandleCalls, 1);
}

TEST(MavlinkTelemetryTest, RcChannelsOverrideIgnoresTargetSystemMismatch)
{
    initMavlinkTestState();

    mavlink_message_t msg;
    mavlink_msg_rc_channels_override_pack(
        42, 200, &msg,
        99, testTargetComponent,
        1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    EXPECT_EQ(mavlinkRxHandleCalls, 1);
}




#ifdef USE_MAVLINK_STANDARD_MODES

#else
#endif












extern "C" {

int32_t debug[DEBUG32_VALUE_COUNT];

uint32_t armingFlags;
uint32_t stateFlags;
uint32_t flightModeFlags;
bool cliMode;
const char *armingDisableFlagNames[] = {
    "GEOZONE", "FS", "ANGLE", "CAL", "OVRLD", "NAV", "COMPASS",
    "ACC", "ARMSW", "HWFAIL", "BOXFS", "PLACEHOLDER", "RX",
    "THR", "CLI", "CMS", "OSD", "ROLL/PITCH", "AUTOTRIM", "OOM",
    "SETTINGFAIL", "PWMOUT", "NOPREARM", "DSHOTBEEPER", "LANDED"
};

attitudeEulerAngles_t attitude;
gyro_t gyro;
gpsSolutionData_t gpsSol;
gpsLocation_t GPS_home;
navSystemStatus_t NAV_Status;
navigationPosControl_t posControl;
rxRuntimeConfig_t rxRuntimeConfig;
rxLinkStatistics_t rxLinkStatistics;
uint16_t averageSystemLoadPercent;

timeUs_t micros(void)
{
    return fakeMicros;
}

uint32_t millis(void)
{
    return fakeMillis;
}

bool rtcGet(rtcTime_t *t)
{
    UNUSED(t);
    return false;
}

serialPortConfig_t *findSerialPortConfig(serialPortFunction_e function)
{
    UNUSED(function);
    testPortConfig.functionMask = FUNCTION_TELEMETRY_MAVLINK;
    testPortConfig.identifier = SERIAL_PORT_USART1;
    testPortConfig.telemetry_baudrateIndex = BAUD_115200;
    return &testPortConfig;
}

serialPortConfig_t *findNextSerialPortConfig(serialPortFunction_e function)
{
    UNUSED(function);
    return NULL;
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

void serialWriteBuf(serialPort_t *instance, const uint8_t *data, int count)
{
    UNUSED(instance);
    memcpy(&serialTxBuffer[serialTxLen], data, (size_t)count);
    serialTxLen += (size_t)count;
}

void serialBeginWrite(serialPort_t *instance)
{
    UNUSED(instance);
}

void serialEndWrite(serialPort_t *instance)
{
    UNUSED(instance);
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

bool serialIsConnected(const serialPort_t *instance)
{
    UNUSED(instance);
    return true;
}

bool isSerialTransmitBufferEmpty(const serialPort_t *instance)
{
    UNUSED(instance);
    return true;
}

void waitForSerialPortToFinishTransmitting(serialPort_t *serialPort)
{
    waitForSerialPortToFinishTransmittingCalls++;
    lastPostProcessPort = serialPort;
}

void cliEnter(serialPort_t *serialPort)
{
    UNUSED(serialPort);
}

bool sensors(uint32_t mask)
{
    return (testSensorsMask & mask) != 0;
}

bool isAmperageConfigured(void)
{
    return false;
}

bool isBlackboxDeviceFull(void)
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

batteryState_e getBatteryState(void)
{
    return BATTERY_OK;
}

bool isEstimatedWindSpeedValid(void)
{
    return false;
}

float getEstimatedHorizontalWindSpeed(uint16_t *angle)
{
    if (angle) {
        *angle = 0;
    }
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
    return estimatedPosition[axis];
}

float getEstimatedActualVelocity(int axis)
{
    return estimatedVelocity[axis];
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

bool fcSetArmState(bool arm)
{
    setArmStateCalls++;
    requestedArmState = arm;
    return setArmStateResult;
}

void activateForcedRTH(void)
{
    activateRthCalls++;
}

bool activateRTHMode(void)
{
    activateRthCalls++;
    return forcedRthState != RTH_IDLE;
}

bool activatePositionHoldMode(void)
{
    activatePositionHoldCalls++;
    return activatePositionHoldResult;
}

void navigationSetLoiterRadiusOverride(uint32_t loiterRadiusCm)
{
    lastLoiterRadiusOverride = loiterRadiusCm;
}

uint32_t navigationGetLoiterRadiusOverride(void)
{
    return lastLoiterRadiusOverride;
}

uint32_t navigationGetLoiterRadius(void)
{
    return lastLoiterRadiusOverride;
}

rthState_e getStateOfForcedRTH(void)
{
    return forcedRthState;
}

bool activateForcedLanding(void)
{
    activateLandingCalls++;
    return activateLandingResult;
}

bool navCanSetHome(void)
{
    return canSetHome;
}

bool navigationSetAltitudeTargetWithDatum(geoAltitudeDatumFlag_e datumFlag, int32_t targetAltitudeCm)
{
    altitudeTargetSetCalls++;
    lastAltitudeTargetDatum = datumFlag;
    lastAltitudeTargetCm = targetAltitudeCm;
    return altitudeTargetSetResult;
}

navigationFSMStateFlags_t navGetCurrentStateFlags(void)
{
    return (navigationFSMStateFlags_t)0;
}

void updateHeadingHoldTarget(int16_t heading)
{
    UNUSED(heading);
}

void setWaypoint(uint8_t wpNumber, const navWaypoint_t *wp)
{
    lastWaypointNumber = wpNumber;
    lastWaypoint = *wp;
    setWaypointCalls++;
    if (wpNumber > 0 && wpNumber <= ARRAYLEN(waypointStore)) {
        waypointStore[wpNumber - 1] = *wp;
        if (wpNumber > waypointCount) {
            waypointCount = wpNumber;
        }
    }
}

int getWaypointCount(void)
{
    return waypointCount;
}

uint8_t getActiveWpNumber(void)
{
    return NAV_Status.activeWpNumber;
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

bool saveNonVolatileWaypointList(void)
{
    saveWaypointCalls++;
    return saveWaypointResult;
}

void resetWaypointList(void)
{
    resetWaypointCalls++;
    waypointCount = 0;
    memset(waypointStore, 0, sizeof(waypointStore));
}

flightModeForTelemetry_e getFlightModeForTelemetry(void)
{
    return testFlightMode;
}

bool isModeActivationConditionPresent(boxId_e modeId)
{
    return testModeActivationConditions[modeId];
}

textAttributes_t osdGetSystemMessage(char *message, size_t length, bool remove)
{
    UNUSED(remove);
    strncpy(message, testOsdSystemMessage, length - 1);
    message[length - 1] = '\0';
    return testOsdSystemMessageAttributes;
}

void mavlinkRxHandleMessage(const mavlink_rc_channels_override_t *msg)
{
    UNUSED(msg);
    mavlinkRxHandleCalls++;
}

static void testMspRebootPostProcess(serialPort_t *serialPort)
{
    mspRebootPostProcessCount++;
    lastPostProcessPort = serialPort;
}

mspResult_e mspFcProcessCommand(mspPacket_t *cmd, mspPacket_t *reply, mspPostProcessFnPtr *mspPostProcessFn)
{
    mspCommandCallCount++;
    reply->cmd = cmd->cmd;
    reply->flags = 0;
    reply->result = 0;

    switch (cmd->cmd) {
    case MSP_SET_PASSTHROUGH:
        mspPassthroughDispatchCount++;
        if (mspPostProcessFn) {
            *mspPostProcessFn = testMspRebootPostProcess;
        }
        return MSP_RESULT_ACK;
    case MSP_REBOOT:
        if (mspPostProcessFn) {
            *mspPostProcessFn = testMspRebootPostProcess;
        }
        return MSP_RESULT_ACK;
    case testLargeReplyMspCommand:
        for (uint16_t i = 0; i < 300; i++) {
            sbufWriteU8(&reply->buf, (uint8_t)i);
        }
        return MSP_RESULT_ACK;
    default:
        return MSP_RESULT_ACK;
    }
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
