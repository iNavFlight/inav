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
static timeMs_t fakeMillis;
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
static int setWaypointCalls;
static int resetWaypointCalls;
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

static std::vector<uint8_t> encodeMspV1Reply(uint8_t cmd, int16_t result, const std::vector<uint8_t> &payload = {})
{
    uint8_t payloadBuf[MSP_PORT_OUTBUF_SIZE];
    mspPacket_t reply = {
        .buf = { .ptr = payloadBuf, .end = ARRAYEND(payloadBuf), },
        .cmd = cmd,
        .flags = 0,
        .result = result,
    };
    uint8_t *payloadHead = reply.buf.ptr;
    if (!payload.empty()) {
        sbufWriteData(&reply.buf, payload.data(), (int)payload.size());
    }
    sbufSwitchToReader(&reply.buf, payloadHead);

    uint8_t frameBuf[testMspFrameBufSize];
    const int frameLength = mspSerialEncodePacket(&reply, MSP_V1, frameBuf, sizeof(frameBuf));
    return std::vector<uint8_t>(frameBuf, frameBuf + frameLength);
}

static void pushRxMessage(const mavlink_message_t *msg)
{
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    int length = mavlink_msg_to_send_buffer(buffer, msg);
    memcpy(&serialRxBuffer[serialRxLen], buffer, (size_t)length);
    serialRxLen += (size_t)length;
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
    setWaypointCalls = 0;
    resetWaypointCalls = 0;
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
    armingFlags = 0;
    stateFlags = 0;
    memset(&gpsSol, 0, sizeof(gpsSol));
    memset(&GPS_home, 0, sizeof(GPS_home));
    memset(waypointStore, 0, sizeof(waypointStore));
    memset(&rxLinkStatistics, 0, sizeof(rxLinkStatistics));
    posControl.wpReachedSeq = 0;
    posControl.wpReachedNotificationPending = false;

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

TEST(MavlinkTelemetryTest, TunnelMalformedPayloadLengthIsDroppedAndDoesNotPoisonState)
{
    initMavlinkTestState();

    std::vector<uint8_t> request = makeMspV1Request(testSimpleMspCommand);
    request.resize(MAVLINK_MSG_TUNNEL_FIELD_PAYLOAD_LEN, 0);

    pushTunnelPayload(MAVLINK_MSG_TUNNEL_FIELD_PAYLOAD_LEN + 1, request);
    handleMAVLinkTelemetry(1000);

    EXPECT_EQ(mspCommandCallCount, 0);
    EXPECT_TRUE(parseTxMessages().empty());

    resetSerialBuffers();
    pushTunnelPayload((uint8_t)makeMspV1Request(testSimpleMspCommand).size(), makeMspV1Request(testSimpleMspCommand));
    handleMAVLinkTelemetry(1000);

    EXPECT_EQ(mspCommandCallCount, 1);
    EXPECT_EQ(collectTunnelPayload(parseTxMessages()), encodeMspV1Reply(testSimpleMspCommand, MSP_RESULT_ACK));
}

TEST(MavlinkTelemetryTest, TunnelRejectsPassthroughBeforeDispatch)
{
    initMavlinkTestState();

    const std::vector<uint8_t> request = makeMspV1Request((uint8_t)MSP_SET_PASSTHROUGH);

    pushTunnelPayload((uint8_t)request.size(), request);
    handleMAVLinkTelemetry(1000);

    EXPECT_EQ(mspCommandCallCount, 0);
    EXPECT_EQ(mspPassthroughDispatchCount, 0);
    EXPECT_EQ(waitForSerialPortToFinishTransmittingCalls, 0);
    EXPECT_EQ(mspRebootPostProcessCount, 0);
    EXPECT_EQ(collectTunnelPayload(parseTxMessages()), encodeMspV1Reply((uint8_t)MSP_SET_PASSTHROUGH, MSP_RESULT_ERROR));
}

TEST(MavlinkTelemetryTest, TunnelRebootUsesIngressPortForPostProcess)
{
    initMavlinkTestState();

    const std::vector<uint8_t> request = makeMspV1Request((uint8_t)MSP_REBOOT);

    pushTunnelPayload((uint8_t)request.size(), request);
    handleMAVLinkTelemetry(1000);

    EXPECT_EQ(mspCommandCallCount, 1);
    EXPECT_EQ(waitForSerialPortToFinishTransmittingCalls, 1);
    EXPECT_EQ(mspRebootPostProcessCount, 1);
    EXPECT_EQ(lastPostProcessPort, &testSerialPort);
    EXPECT_EQ(collectTunnelPayload(parseTxMessages()), encodeMspV1Reply((uint8_t)MSP_REBOOT, MSP_RESULT_ACK));
}

TEST(MavlinkTelemetryTest, TunnelStalePartialFrameResetsBeforeNextRequest)
{
    initMavlinkTestState();

    pushTunnelPayload(3, {'$', 'M', '<'});
    handleMAVLinkTelemetry(1000);

    EXPECT_EQ(mspCommandCallCount, 0);
    EXPECT_TRUE(parseTxMessages().empty());

    resetSerialBuffers();
    fakeMillis = 2000;

    const std::vector<uint8_t> request = makeMspV1Request(testSimpleMspCommand);
    pushTunnelPayload((uint8_t)request.size(), request);
    handleMAVLinkTelemetry(1000);

    EXPECT_EQ(mspCommandCallCount, 1);
    EXPECT_EQ(collectTunnelPayload(parseTxMessages()), encodeMspV1Reply(testSimpleMspCommand, MSP_RESULT_ACK));
}

TEST(MavlinkTelemetryTest, TunnelLargeReplyFragmentsAcrossMultipleMessages)
{
    initMavlinkTestState();

    const std::vector<uint8_t> request = makeMspV1Request(testLargeReplyMspCommand);
    pushTunnelPayload((uint8_t)request.size(), request);
    handleMAVLinkTelemetry(1000);

    const std::vector<mavlink_message_t> messages = parseTxMessages();
    ASSERT_EQ(messages.size(), 3U);

    std::vector<uint8_t> expectedPayload(300);
    for (size_t i = 0; i < expectedPayload.size(); i++) {
        expectedPayload[i] = (uint8_t)i;
    }

    EXPECT_EQ(collectTunnelPayload(messages), encodeMspV1Reply(testLargeReplyMspCommand, MSP_RESULT_ACK, expectedPayload));
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

TEST(MavlinkTelemetryTest, MissionClearAllAcksAndResets)
{
    initMavlinkTestState();

    mavlink_message_t msg;
    mavlink_msg_mission_clear_all_pack(
        42, 200, &msg,
        1, testTargetComponent, MAV_MISSION_TYPE_MISSION);

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
        1, testTargetComponent, 1, MAV_MISSION_TYPE_MISSION, 0);

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

TEST(MavlinkTelemetryTest, MissionCountWhileArmedIsRejected)
{
    initMavlinkTestState();
    ENABLE_ARMING_FLAG(ARMED);

    mavlink_message_t msg;
    mavlink_msg_mission_count_pack(
        42, 200, &msg,
        1, testTargetComponent, 1, MAV_MISSION_TYPE_MISSION, 0);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    mavlink_status_t status;
    memset(&status, 0, sizeof(status));
    mavlink_message_t outMsg;
    bool sawAck = false;
    bool sawRequest = false;

    for (size_t i = 0; i < serialTxLen; i++) {
        if (mavlink_parse_char(0, serialTxBuffer[i], &outMsg, &status) == MAVLINK_FRAMING_OK) {
            if (outMsg.msgid == MAVLINK_MSG_ID_MISSION_ACK) {
                mavlink_mission_ack_t ack;
                mavlink_msg_mission_ack_decode(&outMsg, &ack);
                EXPECT_EQ(ack.type, MAV_MISSION_ERROR);
                sawAck = true;
            }
            if (outMsg.msgid == MAVLINK_MSG_ID_MISSION_REQUEST_INT) {
                sawRequest = true;
            }
        }
    }

    EXPECT_TRUE(sawAck);
    EXPECT_FALSE(sawRequest);
}

TEST(MavlinkTelemetryTest, MissionItemIntSingleItemAcksAccepted)
{
    initMavlinkTestState();

    mavlink_message_t countMsg;
    mavlink_msg_mission_count_pack(
        42, 200, &countMsg,
        1, testTargetComponent, 1, MAV_MISSION_TYPE_MISSION, 0);

    pushRxMessage(&countMsg);
    handleMAVLinkTelemetry(1000);

    serialTxLen = 0;

    mavlink_message_t itemMsg;
    mavlink_msg_mission_item_int_pack(
        42, 200, &itemMsg,
        1, testTargetComponent, 0,
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

TEST(MavlinkTelemetryTest, MissionItemIntSingleFinalItemAllowsAutocontinueZero)
{
    initMavlinkTestState();

    mavlink_message_t countMsg;
    mavlink_msg_mission_count_pack(
        42, 200, &countMsg,
        1, testTargetComponent, 1, MAV_MISSION_TYPE_MISSION, 0);

    pushRxMessage(&countMsg);
    handleMAVLinkTelemetry(1000);

    serialTxLen = 0;

    mavlink_message_t itemMsg;
    mavlink_msg_mission_item_int_pack(
        42, 200, &itemMsg,
        1, testTargetComponent, 0,
        MAV_FRAME_GLOBAL_RELATIVE_ALT_INT,
        MAV_CMD_NAV_WAYPOINT, 0, 0,
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
}

TEST(MavlinkTelemetryTest, MissionItemIntNonFinalAutocontinueZeroIsRejected)
{
    initMavlinkTestState();

    mavlink_message_t countMsg;
    mavlink_msg_mission_count_pack(
        42, 200, &countMsg,
        1, testTargetComponent, 2, MAV_MISSION_TYPE_MISSION, 0);

    pushRxMessage(&countMsg);
    handleMAVLinkTelemetry(1000);

    serialTxLen = 0;

    mavlink_message_t itemMsg;
    mavlink_msg_mission_item_int_pack(
        42, 200, &itemMsg,
        1, testTargetComponent, 0,
        MAV_FRAME_GLOBAL_RELATIVE_ALT_INT,
        MAV_CMD_NAV_WAYPOINT, 0, 0,
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

    EXPECT_EQ(ack.type, MAV_MISSION_UNSUPPORTED);
}

TEST(MavlinkTelemetryTest, MissionItemIntGuidedWhileArmedUpdatesWaypoint)
{
    initMavlinkTestState();
    ENABLE_ARMING_FLAG(ARMED);

    mavlink_message_t msg;
    mavlink_msg_mission_item_int_pack(
        42, 200, &msg,
        1, testTargetComponent, 0,
        MAV_FRAME_GLOBAL_RELATIVE_ALT_INT,
        MAV_CMD_NAV_WAYPOINT, 2, 1,
        0, 0, 0, 0,
        375000000, -1222500000, 12.3f,
        MAV_MISSION_TYPE_MISSION);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    EXPECT_EQ(setWaypointCalls, 1);
    EXPECT_EQ(lastWaypoint.lat, 375000000);
    EXPECT_EQ(lastWaypoint.lon, -1222500000);
    EXPECT_EQ(lastWaypoint.alt, (int32_t)(12.3f * 100.0f));
    EXPECT_EQ(lastWaypoint.p3, 0);
}

TEST(MavlinkTelemetryTest, MissionItemIntGuidedWhileArmedCurrentThreeChangesAltitude)
{
    initMavlinkTestState();
    ENABLE_ARMING_FLAG(ARMED);

    mavlink_message_t msg;
    mavlink_msg_mission_item_int_pack(
        42, 200, &msg,
        1, testTargetComponent, 0,
        MAV_FRAME_GLOBAL_RELATIVE_ALT_INT,
        MAV_CMD_NAV_WAYPOINT, 3, 1,
        0, 0, 0, 0,
        375000000, -1222500000, 12.3f,
        MAV_MISSION_TYPE_MISSION);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    mavlink_message_t ackMsg;
    ASSERT_TRUE(popTxMessage(&ackMsg));
    ASSERT_EQ(ackMsg.msgid, MAVLINK_MSG_ID_MISSION_ACK);

    mavlink_mission_ack_t ack;
    mavlink_msg_mission_ack_decode(&ackMsg, &ack);

    EXPECT_EQ(ack.type, MAV_MISSION_ACCEPTED);
    EXPECT_EQ(altitudeTargetSetCalls, 1);
    EXPECT_EQ(lastAltitudeTargetDatum, NAV_WP_TAKEOFF_DATUM);
    EXPECT_EQ(lastAltitudeTargetCm, 1230);
}

TEST(MavlinkTelemetryTest, MissionRequestListSendsCount)
{
    initMavlinkTestState();
    waypointCount = 2;

    mavlink_message_t msg;
    mavlink_msg_mission_request_list_pack(
        42, 200, &msg,
        1, testTargetComponent, MAV_MISSION_TYPE_MISSION);

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
        1, testTargetComponent, 0, MAV_MISSION_TYPE_MISSION);

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

TEST(MavlinkTelemetryTest, MissionItemReachedIsBroadcastOnceWhenPending)
{
    initMavlinkTestState();

    posControl.wpReachedSeq = 3;
    posControl.wpReachedNotificationPending = true;

    handleMAVLinkTelemetry(1000);

    mavlink_message_t reachedMsg;
    ASSERT_TRUE(findTxMessageById(MAVLINK_MSG_ID_MISSION_ITEM_REACHED, &reachedMsg));

    mavlink_mission_item_reached_t reached;
    mavlink_msg_mission_item_reached_decode(&reachedMsg, &reached);

    EXPECT_EQ(reached.seq, 3);

    resetSerialBuffers();
    handleMAVLinkTelemetry(1000);

    EXPECT_FALSE(findTxMessageById(MAVLINK_MSG_ID_MISSION_ITEM_REACHED, &reachedMsg));
}

TEST(MavlinkTelemetryTest, LegacyGuidedMissionItemUsesAbsoluteAltitude)
{
    initMavlinkTestState();
    ENABLE_ARMING_FLAG(ARMED);

    mavlink_message_t msg;
    mavlink_msg_mission_item_pack(
        42, 200, &msg,
        1, testTargetComponent, 0,
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
        1, testTargetComponent);

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

TEST(MavlinkTelemetryTest, SetPositionTargetGlobalIntSetsWaypoint)
{
    initMavlinkTestState();

    mavlink_message_t msg;
    mavlink_msg_set_position_target_global_int_pack(
        42, 200, &msg,
        0, 1, testTargetComponent,
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

TEST(MavlinkTelemetryTest, BroadcastSetPositionTargetGlobalIntSetsWaypoint)
{
    initMavlinkTestState();

    mavlink_message_t msg;
    mavlink_msg_set_position_target_global_int_pack(
        42, 200, &msg,
        0, 0, 0,
        MAV_FRAME_GLOBAL_RELATIVE_ALT_INT, 0,
        375000000, -1222500000, 12.3f,
        0, 0, 0, 0, 0, 0, 0, 0);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    EXPECT_EQ(setWaypointCalls, 1);
    EXPECT_EQ(lastWaypoint.lat, 375000000);
    EXPECT_EQ(lastWaypoint.lon, -1222500000);
}

TEST(MavlinkTelemetryTest, SetPositionTargetGlobalIntUsesAbsoluteAltitude)
{
    initMavlinkTestState();

    mavlink_message_t msg;
    mavlink_msg_set_position_target_global_int_pack(
        42, 200, &msg,
        0, 1, testTargetComponent,
        MAV_FRAME_GLOBAL_INT, 0,
        375000000, -1222500000, 12.3f,
        0, 0, 0, 0, 0, 0, 0, 0);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    EXPECT_EQ(setWaypointCalls, 1);
    EXPECT_EQ(lastWaypoint.p3, NAV_WP_ALTMODE);
}

TEST(MavlinkTelemetryTest, SetPositionTargetGlobalIntAltitudeOnlyRequiresValidGcs)
{
    initMavlinkTestState();
    gcsValid = false;

    mavlink_message_t msg;
    mavlink_msg_set_position_target_global_int_pack(
        42, 200, &msg,
        0, 1, testTargetComponent,
        MAV_FRAME_GLOBAL_RELATIVE_ALT_INT,
        POSITION_TARGET_TYPEMASK_X_IGNORE |
            POSITION_TARGET_TYPEMASK_Y_IGNORE |
            POSITION_TARGET_TYPEMASK_VX_IGNORE |
            POSITION_TARGET_TYPEMASK_VY_IGNORE |
            POSITION_TARGET_TYPEMASK_VZ_IGNORE |
            POSITION_TARGET_TYPEMASK_AX_IGNORE |
            POSITION_TARGET_TYPEMASK_AY_IGNORE |
            POSITION_TARGET_TYPEMASK_AZ_IGNORE |
            POSITION_TARGET_TYPEMASK_YAW_IGNORE |
            POSITION_TARGET_TYPEMASK_YAW_RATE_IGNORE,
        0, 0, 12.3f,
        0, 0, 0, 0, 0, 0, 0, 0);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    EXPECT_EQ(altitudeTargetSetCalls, 0);
}

TEST(MavlinkTelemetryTest, SetPositionTargetLocalNedAltitudeOnlySetsAltitudeTarget)
{
    initMavlinkTestState();
    estimatedPosition[Z] = 1000.0f;

    mavlink_message_t msg;
    mavlink_msg_set_position_target_local_ned_pack(
        42, 200, &msg,
        0, 1, testTargetComponent,
        MAV_FRAME_LOCAL_OFFSET_NED,
        POSITION_TARGET_TYPEMASK_X_IGNORE |
            POSITION_TARGET_TYPEMASK_Y_IGNORE |
            POSITION_TARGET_TYPEMASK_VX_IGNORE |
            POSITION_TARGET_TYPEMASK_VY_IGNORE |
            POSITION_TARGET_TYPEMASK_VZ_IGNORE |
            POSITION_TARGET_TYPEMASK_AX_IGNORE |
            POSITION_TARGET_TYPEMASK_AY_IGNORE |
            POSITION_TARGET_TYPEMASK_AZ_IGNORE |
            POSITION_TARGET_TYPEMASK_YAW_IGNORE |
            POSITION_TARGET_TYPEMASK_YAW_RATE_IGNORE,
        0.0f, 0.0f, -2.5f,
        0, 0, 0, 0, 0, 0, 0, 0);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    EXPECT_EQ(altitudeTargetSetCalls, 1);
    EXPECT_EQ(lastAltitudeTargetDatum, NAV_WP_TAKEOFF_DATUM);
    EXPECT_EQ(lastAltitudeTargetCm, 1250);
}

TEST(MavlinkTelemetryTest, BroadcastSetPositionTargetLocalNedAltitudeOnlySetsAltitudeTarget)
{
    initMavlinkTestState();
    estimatedPosition[Z] = 1000.0f;

    mavlink_message_t msg;
    mavlink_msg_set_position_target_local_ned_pack(
        42, 200, &msg,
        0, 0, 0,
        MAV_FRAME_LOCAL_OFFSET_NED,
        POSITION_TARGET_TYPEMASK_X_IGNORE |
            POSITION_TARGET_TYPEMASK_Y_IGNORE |
            POSITION_TARGET_TYPEMASK_VX_IGNORE |
            POSITION_TARGET_TYPEMASK_VY_IGNORE |
            POSITION_TARGET_TYPEMASK_VZ_IGNORE |
            POSITION_TARGET_TYPEMASK_AX_IGNORE |
            POSITION_TARGET_TYPEMASK_AY_IGNORE |
            POSITION_TARGET_TYPEMASK_AZ_IGNORE |
            POSITION_TARGET_TYPEMASK_YAW_IGNORE |
            POSITION_TARGET_TYPEMASK_YAW_RATE_IGNORE,
        0.0f, 0.0f, -2.5f,
        0, 0, 0, 0, 0, 0, 0, 0);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    EXPECT_EQ(altitudeTargetSetCalls, 1);
    EXPECT_EQ(lastAltitudeTargetCm, 1250);
}

TEST(MavlinkTelemetryTest, SetPositionTargetLocalNedIgnoresXyMotionRequests)
{
    initMavlinkTestState();
    estimatedPosition[Z] = 1000.0f;

    mavlink_message_t msg;
    mavlink_msg_set_position_target_local_ned_pack(
        42, 200, &msg,
        0, 1, testTargetComponent,
        MAV_FRAME_LOCAL_OFFSET_NED,
        POSITION_TARGET_TYPEMASK_VX_IGNORE |
            POSITION_TARGET_TYPEMASK_VY_IGNORE |
            POSITION_TARGET_TYPEMASK_VZ_IGNORE |
            POSITION_TARGET_TYPEMASK_AX_IGNORE |
            POSITION_TARGET_TYPEMASK_AY_IGNORE |
            POSITION_TARGET_TYPEMASK_AZ_IGNORE |
            POSITION_TARGET_TYPEMASK_YAW_IGNORE |
            POSITION_TARGET_TYPEMASK_YAW_RATE_IGNORE,
        1.0f, 0.0f, -2.5f,
        0, 0, 0, 0, 0, 0, 0, 0);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    EXPECT_EQ(altitudeTargetSetCalls, 0);
}

TEST(MavlinkTelemetryTest, RequestDataStreamStopsStream)
{
    initMavlinkTestState();

    mavlink_message_t setMsg;
    mavlink_msg_request_data_stream_pack(
        42, 200, &setMsg,
        1, testTargetComponent,
        MAV_DATA_STREAM_RC_CHANNELS, 10, 1);

    pushRxMessage(&setMsg);
    handleMAVLinkTelemetry(1000);

    serialTxLen = 0;

    mavlink_message_t getMsg;
    mavlink_msg_command_long_pack(
        42, 200, &getMsg,
        1, testTargetComponent,
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
        1, testTargetComponent,
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

TEST(MavlinkTelemetryTest, BroadcastRequestDataStreamAdjustsBaseInterval)
{
    initMavlinkTestState();

    mavlink_message_t setMsg;
    mavlink_msg_request_data_stream_pack(
        42, 200, &setMsg,
        0, 0,
        MAV_DATA_STREAM_RC_CHANNELS, 10, 1);

    pushRxMessage(&setMsg);
    handleMAVLinkTelemetry(1000);

    serialTxLen = 0;

    mavlink_message_t getMsg;
    mavlink_msg_command_long_pack(
        42, 200, &getMsg,
        1, testTargetComponent,
        MAV_CMD_GET_MESSAGE_INTERVAL,
        0,
        (float)MAVLINK_MSG_ID_RC_CHANNELS,
        0, 0, 0, 0, 0, 0);

    pushRxMessage(&getMsg);
    handleMAVLinkTelemetry(1000);

    mavlink_message_t intervalMsg;
    ASSERT_TRUE(findTxMessageById(MAVLINK_MSG_ID_MESSAGE_INTERVAL, &intervalMsg));

    mavlink_message_interval_t interval;
    mavlink_msg_message_interval_decode(&intervalMsg, &interval);

    EXPECT_EQ(interval.message_id, MAVLINK_MSG_ID_RC_CHANNELS);
    EXPECT_EQ(interval.interval_us, 100000);
}

TEST(MavlinkTelemetryTest, SetMessageIntervalOverridesOnlyRequestedPositionMessage)
{
    initMavlinkTestState();

    mavlink_message_t setMsg;
    mavlink_msg_command_long_pack(
        42, 200, &setMsg,
        1, testTargetComponent,
        MAV_CMD_SET_MESSAGE_INTERVAL,
        0,
        (float)MAVLINK_MSG_ID_GLOBAL_POSITION_INT,
        200000.0f, 0, 0, 0, 0, 0);

    pushRxMessage(&setMsg);
    handleMAVLinkTelemetry(1000);

    const uint16_t messageIds[] = {
        MAVLINK_MSG_ID_GLOBAL_POSITION_INT,
        MAVLINK_MSG_ID_GPS_RAW_INT,
        MAVLINK_MSG_ID_GPS_GLOBAL_ORIGIN
    };
    const int32_t expectedIntervals[] = {
        200000,
        500000,
        500000
    };

    for (size_t i = 0; i < ARRAYLEN(messageIds); i++) {
        serialTxLen = 0;

        mavlink_message_t getMsg;
        mavlink_msg_command_long_pack(
            42, 200, &getMsg,
            1, testTargetComponent,
            MAV_CMD_GET_MESSAGE_INTERVAL,
            0,
            (float)messageIds[i],
            0, 0, 0, 0, 0, 0);

        pushRxMessage(&getMsg);
        handleMAVLinkTelemetry(1000);

        mavlink_message_t intervalMsg;
        ASSERT_TRUE(findTxMessageById(MAVLINK_MSG_ID_MESSAGE_INTERVAL, &intervalMsg));

        mavlink_message_interval_t interval;
        mavlink_msg_message_interval_decode(&intervalMsg, &interval);

        EXPECT_EQ(interval.message_id, messageIds[i]);
        EXPECT_EQ(interval.interval_us, expectedIntervals[i]);
    }
}

TEST(MavlinkTelemetryTest, SetMessageIntervalCanDisableBatteryStatusWithoutAffectingScaledPressure)
{
    initMavlinkTestState();

    mavlink_message_t setMsg;
    mavlink_msg_command_long_pack(
        42, 200, &setMsg,
        1, testTargetComponent,
        MAV_CMD_SET_MESSAGE_INTERVAL,
        0,
        (float)MAVLINK_MSG_ID_BATTERY_STATUS,
        -1.0f, 0, 0, 0, 0, 0);

    pushRxMessage(&setMsg);
    handleMAVLinkTelemetry(1000);

    const uint16_t messageIds[] = {
        MAVLINK_MSG_ID_BATTERY_STATUS,
        MAVLINK_MSG_ID_SCALED_PRESSURE
    };
    const int32_t expectedIntervals[] = {
        -1,
        1000000
    };

    for (size_t i = 0; i < ARRAYLEN(messageIds); i++) {
        serialTxLen = 0;

        mavlink_message_t getMsg;
        mavlink_msg_command_long_pack(
            42, 200, &getMsg,
            1, testTargetComponent,
            MAV_CMD_GET_MESSAGE_INTERVAL,
            0,
            (float)messageIds[i],
            0, 0, 0, 0, 0, 0);

        pushRxMessage(&getMsg);
        handleMAVLinkTelemetry(1000);

        mavlink_message_t intervalMsg;
        ASSERT_TRUE(findTxMessageById(MAVLINK_MSG_ID_MESSAGE_INTERVAL, &intervalMsg));

        mavlink_message_interval_t interval;
        mavlink_msg_message_interval_decode(&intervalMsg, &interval);

        EXPECT_EQ(interval.message_id, messageIds[i]);
        EXPECT_EQ(interval.interval_us, expectedIntervals[i]);
    }
}

TEST(MavlinkTelemetryTest, SetMessageIntervalResetRevertsToCurrentGroupInterval)
{
    initMavlinkTestState();

    mavlink_message_t streamMsg;
    mavlink_msg_request_data_stream_pack(
        42, 200, &streamMsg,
        1, testTargetComponent,
        MAV_DATA_STREAM_POSITION, 10, 1);

    pushRxMessage(&streamMsg);
    handleMAVLinkTelemetry(1000);

    serialTxLen = 0;

    mavlink_message_t setMsg;
    mavlink_msg_command_long_pack(
        42, 200, &setMsg,
        1, testTargetComponent,
        MAV_CMD_SET_MESSAGE_INTERVAL,
        0,
        (float)MAVLINK_MSG_ID_GLOBAL_POSITION_INT,
        250000.0f, 0, 0, 0, 0, 0);

    pushRxMessage(&setMsg);
    handleMAVLinkTelemetry(1000);

    serialTxLen = 0;

    mavlink_message_t clearMsg;
    mavlink_msg_command_long_pack(
        42, 200, &clearMsg,
        1, testTargetComponent,
        MAV_CMD_SET_MESSAGE_INTERVAL,
        0,
        (float)MAVLINK_MSG_ID_GLOBAL_POSITION_INT,
        0.0f, 0, 0, 0, 0, 0);

    pushRxMessage(&clearMsg);
    handleMAVLinkTelemetry(1000);

    serialTxLen = 0;

    mavlink_message_t getMsg;
    mavlink_msg_command_long_pack(
        42, 200, &getMsg,
        1, testTargetComponent,
        MAV_CMD_GET_MESSAGE_INTERVAL,
        0,
        (float)MAVLINK_MSG_ID_GLOBAL_POSITION_INT,
        0, 0, 0, 0, 0, 0);

    pushRxMessage(&getMsg);
    handleMAVLinkTelemetry(1000);

    mavlink_message_t intervalMsg;
    ASSERT_TRUE(findTxMessageById(MAVLINK_MSG_ID_MESSAGE_INTERVAL, &intervalMsg));

    mavlink_message_interval_t interval;
    mavlink_msg_message_interval_decode(&intervalMsg, &interval);

    EXPECT_EQ(interval.message_id, MAVLINK_MSG_ID_GLOBAL_POSITION_INT);
    EXPECT_EQ(interval.interval_us, 100000);
}

TEST(MavlinkTelemetryTest, HeartbeatIntervalIsIndependentFromExtra2Stream)
{
    initMavlinkTestState();

    mavlink_message_t stopExtra2Msg;
    mavlink_msg_request_data_stream_pack(
        42, 200, &stopExtra2Msg,
        1, testTargetComponent,
        MAV_DATA_STREAM_EXTRA2, 0, 0);

    pushRxMessage(&stopExtra2Msg);
    handleMAVLinkTelemetry(1000);

    serialTxLen = 0;

    mavlink_message_t getMsg;
    mavlink_msg_command_long_pack(
        42, 200, &getMsg,
        1, testTargetComponent,
        MAV_CMD_GET_MESSAGE_INTERVAL,
        0,
        (float)MAVLINK_MSG_ID_HEARTBEAT,
        0, 0, 0, 0, 0, 0);

    pushRxMessage(&getMsg);
    handleMAVLinkTelemetry(1000);

    mavlink_message_t intervalMsg;
    ASSERT_TRUE(popTxMessage(&intervalMsg));
    ASSERT_EQ(intervalMsg.msgid, MAVLINK_MSG_ID_MESSAGE_INTERVAL);

    mavlink_message_interval_t interval;
    mavlink_msg_message_interval_decode(&intervalMsg, &interval);

    EXPECT_EQ(interval.message_id, MAVLINK_MSG_ID_HEARTBEAT);
    EXPECT_EQ(interval.interval_us, 1000000);
}

TEST(MavlinkTelemetryTest, RequestProtocolVersionUsesConfiguredVersion)
{
    initMavlinkTestState();

    mavlink_message_t msg;
    mavlink_msg_command_long_pack(
        42, 200, &msg,
        1, testTargetComponent,
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

TEST(MavlinkTelemetryTest, SystemTimeSupportsRequestPeriodicOutputAndIntervalQuery)
{
    initMavlinkTestState();
    fakeMillis = 1234;

    mavlink_message_t requestMsg;
    mavlink_msg_command_long_pack(
        42, 200, &requestMsg,
        1, testTargetComponent,
        MAV_CMD_REQUEST_MESSAGE,
        0,
        (float)MAVLINK_MSG_ID_SYSTEM_TIME,
        0, 0, 0, 0, 0, 0);

    pushRxMessage(&requestMsg);
    handleMAVLinkTelemetry(1000);

    mavlink_message_t systemTimeMsg;
    ASSERT_TRUE(findTxMessageById(MAVLINK_MSG_ID_SYSTEM_TIME, &systemTimeMsg));

    mavlink_system_time_t systemTime;
    mavlink_msg_system_time_decode(&systemTimeMsg, &systemTime);

    EXPECT_EQ(systemTime.time_unix_usec, 0U);
    EXPECT_EQ(systemTime.time_boot_ms, 1234U);

    serialTxLen = 0;
    handleMAVLinkTelemetry(1000000);

    ASSERT_TRUE(findTxMessageById(MAVLINK_MSG_ID_SYSTEM_TIME, &systemTimeMsg));
    mavlink_msg_system_time_decode(&systemTimeMsg, &systemTime);
    EXPECT_EQ(systemTime.time_boot_ms, 1234U);

    serialTxLen = 0;

    mavlink_message_t getMsg;
    mavlink_msg_command_long_pack(
        42, 200, &getMsg,
        1, testTargetComponent,
        MAV_CMD_GET_MESSAGE_INTERVAL,
        0,
        (float)MAVLINK_MSG_ID_SYSTEM_TIME,
        0, 0, 0, 0, 0, 0);

    pushRxMessage(&getMsg);
    handleMAVLinkTelemetry(1000);

    mavlink_message_t intervalMsg;
    ASSERT_TRUE(findTxMessageById(MAVLINK_MSG_ID_MESSAGE_INTERVAL, &intervalMsg));

    mavlink_message_interval_t interval;
    mavlink_msg_message_interval_decode(&intervalMsg, &interval);

    EXPECT_EQ(interval.message_id, MAVLINK_MSG_ID_SYSTEM_TIME);
    EXPECT_EQ(interval.interval_us, 1000000);
}

TEST(MavlinkTelemetryTest, RequestAutopilotCapabilitiesReportsLocalNedCapabilityAndPackedVersion)
{
    initMavlinkTestState();

    mavlink_message_t msg;
    mavlink_msg_command_long_pack(
        42, 200, &msg,
        1, testTargetComponent,
        MAV_CMD_REQUEST_AUTOPILOT_CAPABILITIES,
        0,
        1, 0, 0, 0, 0, 0, 0);

    pushRxMessage(&msg);
    handleMAVLinkTelemetry(1000);

    mavlink_status_t status;
    memset(&status, 0, sizeof(status));
    mavlink_message_t outMsg;
    bool sawAutopilotVersion = false;

    for (size_t i = 0; i < serialTxLen; i++) {
        if (mavlink_parse_char(0, serialTxBuffer[i], &outMsg, &status) == MAVLINK_FRAMING_OK) {
            if (outMsg.msgid == MAVLINK_MSG_ID_AUTOPILOT_VERSION) {
                mavlink_autopilot_version_t version;
                mavlink_msg_autopilot_version_decode(&outMsg, &version);
                EXPECT_NE((version.capabilities & MAV_PROTOCOL_CAPABILITY_SET_POSITION_TARGET_LOCAL_NED), 0U);
                EXPECT_EQ(version.flight_sw_version,
                    ((uint32_t)ARDUPILOT_VERSION_MAJOR << 24) |
                    ((uint32_t)ARDUPILOT_VERSION_MINOR << 16) |
                    ((uint32_t)ARDUPILOT_VERSION_PATCH << 8));
                EXPECT_EQ(version.middleware_sw_version, 0U);
                EXPECT_EQ(version.os_sw_version, 0U);
                sawAutopilotVersion = true;
            }
        }
    }

    EXPECT_TRUE(sawAutopilotVersion);
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

extern "C" {

int32_t debug[DEBUG32_VALUE_COUNT];

uint32_t armingFlags;
uint32_t stateFlags;
bool cliMode;

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
    return 0;
}

uint32_t millis(void)
{
    return fakeMillis;
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

bool navigationSetAltitudeTargetWithDatum(geoAltitudeDatumFlag_e datumFlag, int32_t targetAltitudeCm)
{
    altitudeTargetSetCalls++;
    lastAltitudeTargetDatum = datumFlag;
    lastAltitudeTargetCm = targetAltitudeCm;
    return altitudeTargetSetResult;
}

bool navigationConsumeWaypointReached(uint16_t *seq)
{
    if (!posControl.wpReachedNotificationPending) {
        return false;
    }

    *seq = posControl.wpReachedSeq;
    posControl.wpReachedNotificationPending = false;
    return true;
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
    return testFlightMode;
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
