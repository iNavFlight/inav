/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include <algorithm>
#include <cstring>
#include <deque>
#include <iterator>
#include <vector>
#include "gtest/gtest.h"

extern "C" {
#include "platform.h"
#include "build/debug.h"
#include "io/gps.h"
#include "io/gps_private.h"
#include "io/gps_ublox.h"

int32_t debug[DEBUG32_VALUE_COUNT];
uint8_t debugMode;
gpsReceiverData_t gpsState;
gpsSolutionData_t gpsSolDRV;
gpsStatistics_t gpsStats;
gpsConfig_t gpsConfig_System;
extern const uint32_t baudRates[] = {115200};
baudRate_e gpsToSerialBaudRate[GPS_BAUDRATE_COUNT] = {};
}

static std::deque<uint8_t> rx;
static std::vector<std::vector<uint8_t>> tx;
static std::vector<uint8_t> versionPayload;
static std::vector<uint8_t> gnssPayload;
static timeMs_t nowMs;
static timeMs_t maxSilenceMs;
static unsigned solutions;
static bool modernOnly;
static serialPort_t port;

static void receive(uint8_t msgClass, uint8_t id, const std::vector<uint8_t>& payload)
{
    std::vector<uint8_t> packet = {0xb5, 0x62, msgClass, id,
        static_cast<uint8_t>(payload.size()), static_cast<uint8_t>(payload.size() >> 8)};
    packet.insert(packet.end(), payload.begin(), payload.end());
    uint8_t a = 0, b = 0;
    for (size_t i = 2; i < packet.size(); ++i) {
        a += packet[i];
        b += a;
    }
    packet.push_back(a);
    packet.push_back(b);
    rx.insert(rx.end(), packet.begin(), packet.end());
}

extern "C" {
#ifndef __APPLE__
// strnstr is provided by newlib on the flight controller and libc on macOS.
char *strnstr(const char *haystack, const char *needle, size_t length)
{
    const size_t needleLength = strlen(needle);
    if (!needleLength) {
        return const_cast<char *>(haystack);
    }
    for (size_t i = 0; i + needleLength <= length && haystack[i]; ++i) {
        if (!strncmp(haystack + i, needle, needleLength)) {
            return const_cast<char *>(haystack + i);
        }
    }
    return nullptr;
}
#endif

timeMs_t millis(void) { return nowMs; }
uint32_t serialRxBytesWaiting(const serialPort_t *) { return rx.size(); }
uint8_t serialRead(serialPort_t *)
{
    const uint8_t value = rx.front();
    rx.pop_front();
    return value;
}
bool isSerialTransmitBufferEmpty(const serialPort_t *) { return true; }
void serialSetBaudRate(serialPort_t *, uint32_t) {}
void serialPrint(serialPort_t *, const char *) {}
int gpsBaudRateToInt(gpsBaudRate_e) { return 115200; }
void gpsSetState(gpsState_e state) { gpsState.state = state; }
// Both refresh the timestamp that the GPS timeout in gps.c checks.
void gpsSetProtocolTimeout(timeMs_t timeout)
{
    gpsState.lastMessageMs = nowMs;
    gpsState.timeoutMs = timeout;
}
void gpsProcessNewDriverData(void) {}
void gpsProcessNewSolutionData(bool)
{
    ++solutions;
    gpsState.lastMessageMs = nowMs;
}
uint16_t gpsConstrainEPE(uint32_t value) { return std::min(value, uint32_t(UINT16_MAX)); }
uint16_t gpsConstrainHDOP(uint32_t value) { return std::min(value, uint32_t(UINT16_MAX)); }
void serialWriteBuf(serialPort_t *, const uint8_t *data, int count)
{
    tx.emplace_back(data, data + count);
    if (data[2] == CLASS_MON && data[3] == MSG_VER) {
        receive(CLASS_MON, MSG_VER, versionPayload);
    } else if (data[2] == CLASS_MON && data[3] == MSG_MON_GNSS) {
        if (!gnssPayload.empty()) {
            receive(CLASS_MON, MSG_MON_GNSS, gnssPayload);
        }
    } else if (data[2] == CLASS_CFG) {
        // New receivers do not acknowledge removed legacy CFG messages.
        if (!modernOnly || data[3] == 0x8a) {
            receive(CLASS_ACK, MSG_ACK_ACK, {CLASS_CFG, data[3]});
        }
    }
}
}

class GPSUbloxProtocolTest : public testing::Test {
protected:
    void SetUp() override
    {
        rx.clear();
        tx.clear();
        versionPayload.clear();
        gnssPayload.clear();
        nowMs = 0;
        maxSilenceMs = 0;
        solutions = 0;
        modernOnly = true;
        gpsState = {};
        gpsSolDRV = {};
        gpsStats = {};
        gpsConfig_System = {};
        gpsConfig_System.autoConfig = GPS_AUTOCONFIG_ON;
        gpsConfig_System.autoBaud = GPS_AUTOBAUD_OFF;
        gpsConfig_System.ubloxNavHz = 8;
        gpsState.gpsConfig = &gpsConfig_System;
        gpsState.gpsPort = &port;
        gpsState.baseTimeoutMs = 1000;
        gpsRestartUBLOX();
    }

    void version(const char *hardware, const char *protocol)
    {
        versionPayload.assign(70, 0);
        memcpy(versionPayload.data(), "EXT HPG 2.02", 12);
        memcpy(versionPayload.data() + 30, hardware, strlen(hardware));
        memcpy(versionPayload.data() + 40, protocol, strlen(protocol));
    }

    void run(unsigned duration = 2000)
    {
        for (unsigned i = 0; i < duration; i += 10) {
            if (i % 100 == 0) {
                receive(CLASS_NAV, MSG_PVT, std::vector<uint8_t>(sizeof(ubx_nav_pvt), 0));
            }
            gpsHandleUBLOX();
            maxSilenceMs = std::max(maxSilenceMs, nowMs - gpsState.lastMessageMs);
            nowMs += 10;
        }
    }

    unsigned countMessages(uint8_t msgClass, uint8_t id)
    {
        return std::count_if(tx.begin(), tx.end(), [=](const std::vector<uint8_t>& p) {
            return p[2] == msgClass && p[3] == id;
        });
    }

    bool sentValset(uint32_t key, uint8_t value)
    {
        const uint8_t keyValue[] = {uint8_t(key), uint8_t(key >> 8), uint8_t(key >> 16), uint8_t(key >> 24), value};
        return std::any_of(tx.begin(), tx.end(), [&](const std::vector<uint8_t>& p) {
            return p[2] == CLASS_CFG && p[3] == 0x8a &&
                std::search(p.begin(), p.end(), std::begin(keyValue), std::end(keyValue)) != p.end();
        });
    }
};

TEST_F(GPSUbloxProtocolTest, UnknownHardwareUsesAdvertisedProtocolAndCompletesConfiguration)
{
    // Synthetic MON-VER, not a capture from the reported receiver.
    version("UNKNOWN", "PROTVER=50.10");
    run();
    EXPECT_EQ(UBX_HW_VERSION_UNKNOWN, gpsState.hwVersion);
    EXPECT_EQ(50, gpsState.swVersionMajor);
    EXPECT_EQ(10, gpsState.swVersionMinor);
    EXPECT_EQ(1u, countMessages(CLASS_MON, MSG_VER));
    EXPECT_EQ(0u, countMessages(CLASS_CFG, MSG_CFG_NAV_SETTINGS));
    EXPECT_GE(countMessages(CLASS_CFG, 0x8a), 4u);
    EXPECT_GT(solutions, 0u);
    EXPECT_EQ(GPS_NO_FIX, gpsSolDRV.fixType);
    EXPECT_EQ(8, gpsConfig_System.ubloxNavHz);
    bool requestedRate = false;
    for (const auto& p : tx) {
        // CFG-RATE-MEAS (0x30210001), 125 ms = 8 Hz, little endian.
        const uint8_t keyValue[] = {0x01, 0x00, 0x21, 0x30, 125, 0};
        requestedRate |= std::search(p.begin(), p.end(), std::begin(keyValue), std::end(keyValue)) != p.end();
    }
    EXPECT_TRUE(requestedRate);
}

TEST_F(GPSUbloxProtocolTest, KnownM8RetainsLegacyConfiguration)
{
    modernOnly = false;
    version("00080000", "PROTVER=18.00");
    run(4000);
    EXPECT_EQ(UBX_HW_VERSION_UBLOX8, gpsState.hwVersion);
    EXPECT_EQ(18, gpsState.swVersionMajor);
    EXPECT_GT(countMessages(CLASS_CFG, MSG_CFG_NAV_SETTINGS), 0u);
    EXPECT_EQ(0u, countMessages(CLASS_CFG, 0x8a));
    EXPECT_GT(solutions, 0u);
}

TEST_F(GPSUbloxProtocolTest, KnownM10RetainsModernConfiguration)
{
    version("000A0000", "PROTVER=34.10");
    run(4000);
    EXPECT_EQ(UBX_HW_VERSION_UBLOX10, gpsState.hwVersion);
    EXPECT_EQ(34, gpsState.swVersionMajor);
    EXPECT_EQ(10, gpsState.swVersionMinor);
    EXPECT_EQ(0u, countMessages(CLASS_CFG, MSG_CFG_NAV_SETTINGS));
    EXPECT_GT(solutions, 0u);
}

// MON-GNSS captured from a ZED-X20P running HPG 2.10. It is message version 1
// (signal plans), which the driver does not parse.
static const uint8_t x20MonGnss[] = {
    0x01, 0x04, 0x02, 0x01, 0x01, 0x53, 0x50, 0x31, 0x00, 0x00, 0x0d, 0x00, 0x0b, 0x00, 0x1b, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x3b, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x53, 0x50, 0x32, 0x00, 0x00, 0x0d, 0x00, 0x0b, 0x00, 0x1a, 0x00, 0x03, 0x00, 0x01, 0x00,
    0x3b, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x53, 0x50, 0x33,
    0x00, 0x00, 0x0d, 0x00, 0x0b, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x01, 0x00, 0x3b, 0x00, 0x01, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x53, 0x50, 0x36, 0x00, 0x00, 0x0d, 0x00,
    0x03, 0x00, 0x0b, 0x00, 0x03, 0x00, 0x01, 0x00, 0x3b, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
};

static void x20Receiver(void)
{
    // MON-VER captured from the same receiver.
    const char *extensions[] = {"ROM BASE 0x00A9D329", "FWVER=HPG 2.10", "PROTVER=50.11", "MOD=ZED-X20P",
        "GPS;GLO;GAL;BDS", "SBAS;QZSS", "NAVIC;LBAND"};
    versionPayload.assign(250, 0);
    memcpy(versionPayload.data(), "EXT HPG 2.10 (b0eda3)", 21);
    memcpy(versionPayload.data() + 30, "000B0000", 8);
    for (size_t i = 0; i < sizeof(extensions) / sizeof(extensions[0]); ++i) {
        memcpy(versionPayload.data() + 40 + i * 30, extensions[i], strlen(extensions[i]));
    }
    gnssPayload.assign(std::begin(x20MonGnss), std::end(x20MonGnss));
}

TEST_F(GPSUbloxProtocolTest, X20StaysWithinGpsTimeout)
{
    x20Receiver();
    run(4000);
    EXPECT_EQ(UBX_HW_VERSION_UBLOX20, gpsState.hwVersion);
    // gps.c restarts the driver when this gap exceeds the base timeout.
    EXPECT_LE(maxSilenceMs, gpsState.baseTimeoutMs);
    EXPECT_GT(solutions, 0u);
}

TEST_F(GPSUbloxProtocolTest, X20AppliesConstellationSettings)
{
    x20Receiver();
    gpsConfig_System.ubloxUseGalileo = true;
    gpsConfig_System.ubloxUseBeidou = true;
    gpsConfig_System.ubloxUseGlonass = false;
    run(4000);
    EXPECT_EQ(UBX_HW_VERSION_UBLOX20, gpsState.hwVersion);
    EXPECT_EQ(50, gpsState.swVersionMajor);
    EXPECT_EQ(11, gpsState.swVersionMinor);
    EXPECT_EQ(0u, countMessages(CLASS_CFG, MSG_CFG_NAV_SETTINGS));
    EXPECT_TRUE(sentValset(UBLOX_CFG_SIGNAL_GAL_ENA, 1));
    EXPECT_TRUE(sentValset(UBLOX_CFG_SIGNAL_BDS_ENA, 1));
    EXPECT_TRUE(sentValset(UBLOX_CFG_GLO_ENA, 0));
    EXPECT_GT(solutions, 0u);
}

TEST_F(GPSUbloxProtocolTest, TruncatedExtensionDoesNotReusePreviousPayload)
{
    version("UNKNOWN", "PROTVER=50.10");
    run();
    gpsRestartUBLOX();
    versionPayload.resize(45);
    run();
    EXPECT_EQ(0, gpsState.swVersionMajor);
    EXPECT_EQ(0, gpsState.swVersionMinor);
}

TEST_F(GPSUbloxProtocolTest, ShortHeaderDoesNotReuseHardwareOrVersion)
{
    version("00080000", "PROTVER=18.00");
    run();
    gpsRestartUBLOX();
    versionPayload.resize(30);
    run();
    EXPECT_EQ(UBX_HW_VERSION_UNKNOWN, gpsState.hwVersion);
    EXPECT_EQ(0, gpsState.swVersionMajor);
}

TEST_F(GPSUbloxProtocolTest, MalformedProtocolIsIgnored)
{
    version("UNKNOWN", "PROTVER=50.xx");
    run();
    EXPECT_EQ(0, gpsState.swVersionMajor);
    EXPECT_EQ(0, gpsState.swVersionMinor);
}

TEST_F(GPSUbloxProtocolTest, SpaceSeparatedProtocolIsAccepted)
{
    version("UNKNOWN", "PROTVER 50.10");
    run();
    EXPECT_EQ(50, gpsState.swVersionMajor);
    EXPECT_EQ(10, gpsState.swVersionMinor);
    EXPECT_GT(solutions, 0u);
}

TEST_F(GPSUbloxProtocolTest, ProtocolAfterOtherExtensionsIsParsed)
{
    version("UNKNOWN", "FWVER=HPG 2.02");
    versionPayload.resize(130, 0);
    memcpy(versionPayload.data() + 70, "MOD=ZED-X20P", 12);
    memcpy(versionPayload.data() + 100, "PROTVER=50.10", 13);
    run();
    EXPECT_EQ(50, gpsState.swVersionMajor);
    EXPECT_EQ(10, gpsState.swVersionMinor);
    EXPECT_GT(solutions, 0u);
}

TEST_F(GPSUbloxProtocolTest, AutoConfigurationDisabledStillAcceptsPvt)
{
    gpsConfig_System.autoConfig = GPS_AUTOCONFIG_OFF;
    run();
    EXPECT_TRUE(tx.empty());
    EXPECT_GT(solutions, 0u);
}
