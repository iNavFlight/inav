/*
 * This file is part of INAV.
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


#include "gtest/gtest.h"
#include "unittest_macros.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#include <stdio.h>

#include "io/gps_ublox_utils.h"

void dumpCfg(const ubx_config_data8_t *cfg, int valuesAdded)
{
    printf("%02x %02x %02x %02x %04x\n", cfg->header.preamble1, cfg->header.preamble2, cfg->header.msg_class, cfg->header.msg_id, cfg->header.length);

    printf("%02x %02x %02x %02x\n", cfg->configHeader.version, cfg->configHeader.layers, cfg->configHeader.transaction, cfg->configHeader.reserved);

    for(int i =0; i < valuesAdded; ++i) {
        printf("%i: %08x %02x\n", i+1, cfg->data.payload[i].key, cfg->data.payload[i].value);
    }

    uint8_t *chksums = (uint8_t *)&cfg->data.payload[valuesAdded];

    printf("%02x %02x\n", chksums[0], chksums[1]);
}

void dumpMemory(uint8_t *mem, int size)
{
    for(int i =0; i < size; ++i) {
        printf("%02x ", mem[i]);
    }
    printf("\n");
}

TEST(GPSUbloxTest, TestUbloxCfgFillBytes)
{
    ubx_config_data8_t cfg = {};
    ubx_config_data8_payload_t kvPairs[] = {
        { 0x10310025, 0x1},
        { 0x42, 0x69},
        { 0x04, 0x20},
        { 0x42, 0x69},
        { 0x04, 0x20},
        { 0x42, 0x69},
        { 0x04, 0x20},
        { 0x42, 0x69},
        { 0x04, 0x20},
        { 0x42, 0x69},
        { 0x04, 0x20},
        { 0x42, 0x69}
    };

    int valuesAdded = ubloxCfgFillBytes(&cfg, kvPairs, 12);

    EXPECT_TRUE(valuesAdded == 12);

    dumpCfg(&cfg, valuesAdded);

    valuesAdded = ubloxCfgFillBytes(&cfg, kvPairs, 1);
    EXPECT_TRUE(1 == valuesAdded);

    // Set glonass enabled, from u-center 2
    const uint8_t expected[] = {0xB5, 0x62, 0x06, 0x8A, 0x09, 0x00, 0x01, 0x01, 0x00, 0x00, 0x25, 0x00, 0x31, 0x10, 0x01, 0x02, 0xA7};
    EXPECT_FALSE(memcmp((void *)expected, (void *)&cfg, 17));

    printf("Expected:\n");
    dumpMemory((uint8_t *)expected, 17);
    printf("Actual:\n");
    dumpMemory((uint8_t *)&cfg, 17);

    // osdFormatCentiNumber(buf, 12345, 1, 2, 3, 7);
    // std::cout << "'" << buf << "'" << std::endl;
    //EXPECT_FALSE(strcmp(buf, " 123.45"));
}

TEST(GPSUbloxTest, navSigStructureSizes) {
    EXPECT_TRUE(sizeof(ubx_nav_sig_info) == 16);

    EXPECT_TRUE(sizeof(ubx_nav_sig) == (8 + (16 * UBLOX_MAX_SIGNALS)));

    EXPECT_TRUE(sizeof(ubx_nav_svinfo_channel) == 12);

    EXPECT_TRUE(sizeof(ubx_nav_svinfo) == (8 + (12 * UBLOX_MAX_SIGNALS)));
}

TEST(GPSUbloxTest, DecodeHardwareVersion)
{
    const struct {
        const char *hwVersion;
        uint8_t expected;
    } cases[] = {
        { "00040005",   UBX_HW_VERSION_UBLOX5 },
        { "00040007",   UBX_HW_VERSION_UBLOX6 },
        { "00070000",   UBX_HW_VERSION_UBLOX7 },
        { "00080000",   UBX_HW_VERSION_UBLOX8 },
        { "00190000",   UBX_HW_VERSION_UBLOX9 },    // also reported by the ZED-F9P
        { "000A0000",   UBX_HW_VERSION_UBLOX10 },
        { "000B0000",   UBX_HW_VERSION_UBLOX20 },
        { "000C0000",   UBX_HW_VERSION_UNKNOWN },
        { "000a0000",   UBX_HW_VERSION_UNKNOWN },
        { "0008000",    UBX_HW_VERSION_UNKNOWN },
        { "00080000XY", UBX_HW_VERSION_UNKNOWN },
        { "",           UBX_HW_VERSION_UNKNOWN },
    };

    for (const auto &c : cases) {
        SCOPED_TRACE(c.hwVersion);
        ubx_mon_ver ver = {};
        memcpy(ver.hwVersion, c.hwVersion, std::min(strlen(c.hwVersion), sizeof(ver.hwVersion)));
        EXPECT_EQ(c.expected, ubloxDecodeHardwareVersion(ver.hwVersion, sizeof(ver.hwVersion)));
    }
}

TEST(GPSUbloxTest, ParseProtocolVersion)
{
    const size_t extensionLength = 30;
    const struct {
        const char *extension;
        size_t length;
        bool valid;
        uint8_t major;
        uint8_t minor;
    } cases[] = {
        { "PROTVER=18.00", extensionLength, true, 18, 0 },
        { "PROTVER 14.00", extensionLength, true, 14, 0 },
        { "PROTVER=34.10", extensionLength, true, 34, 10 },
        { "PROTVER=27.31", extensionLength, true, 27, 31 },
        { "PROTVER=27.50", extensionLength, true, 27, 50 },
        { "PROTVER=50.11", extensionLength, true, 50, 11 },
        { "PROTVER=18.00", 14, true, 18, 0 },
        { "PROTVER=18.00", 13, false, 0, 0 },
        { "PROTVER=18.00ABCDEFGHIJKLMNOPQ", extensionLength, false, 0, 0 },
        { "PROTVER=18.000", extensionLength, false, 0, 0 },
        { "PROTVER=18.", extensionLength, false, 0, 0 },
        { "PROTVER=18", extensionLength, false, 0, 0 },
        { "PROTVER=18.0", extensionLength, false, 0, 0 },
        { "PROTVER=8.00", extensionLength, false, 0, 0 },
        { "PROTVER=50.xx", extensionLength, false, 0, 0 },
        { "PROTVER:18.00", extensionLength, false, 0, 0 },
        { "protver=18.00", extensionLength, false, 0, 0 },
        { "FWVER=HPG 2.10", extensionLength, false, 0, 0 },
        { "", extensionLength, false, 0, 0 },
        { "", 0, false, 0, 0 },
    };

    for (const auto &c : cases) {
        SCOPED_TRACE(testing::Message() << "'" << c.extension << "' length " << c.length);
        char extension[extensionLength] = {};
        memcpy(extension, c.extension, std::min(strlen(c.extension), sizeof(extension)));
        uint8_t major = 0;
        uint8_t minor = 0;
        EXPECT_EQ(c.valid, ubloxParseProtocolVersion(extension, c.length, &major, &minor));
        if (c.valid) {
            EXPECT_EQ(c.major, major);
            EXPECT_EQ(c.minor, minor);
        }
    }
}

TEST(GPSUbloxTest, CapabilitiesFollowHardwareOrProtocol)
{
    const struct {
        const char *receiver;
        uint8_t hwVersion;
        uint8_t protMajor;
        uint8_t protMinor;
        bool canConfigureGnss;
        bool useM10GnssKeys;
        uint8_t navHz;
    } cases[] = {
        { "M5",                 UBX_HW_VERSION_UBLOX5,   0,  0, false, false,  5 },
        { "M6",                 UBX_HW_VERSION_UBLOX6,   0,  0, false, false,  5 },
        { "M7",                 UBX_HW_VERSION_UBLOX7,   0,  0, false, false, 10 },
        { "M7 14.00",           UBX_HW_VERSION_UBLOX7,  14,  0, false, false, 10 },
        { "M8",                 UBX_HW_VERSION_UBLOX8,   0,  0, true,  false, 10 },
        { "M8 18.00",           UBX_HW_VERSION_UBLOX8,  18,  0, true,  false, 10 },
        { "M9 32.01",           UBX_HW_VERSION_UBLOX9,  32,  1, true,  false, 10 },
        { "F9P 27.50",          UBX_HW_VERSION_UBLOX9,  27, 50, true,  false, 10 },
        { "M10",                UBX_HW_VERSION_UBLOX10,  0,  0, true,  false, 10 },
        { "M10 34.10",          UBX_HW_VERSION_UBLOX10, 34, 10, true,  true,  10 },
        { "X20 50.11",          UBX_HW_VERSION_UBLOX20, 50, 11, true,  true,  10 },
        { "unknown 50.10",      UBX_HW_VERSION_UNKNOWN, 50, 10, true,  true,  10 },
        { "unknown 34.00",      UBX_HW_VERSION_UNKNOWN, 34,  0, true,  true,  10 },
        { "unknown 33.99",      UBX_HW_VERSION_UNKNOWN, 33, 99, true,  false, 10 },
        { "unknown 24.00",      UBX_HW_VERSION_UNKNOWN, 24,  0, true,  false, 10 },
        { "unknown 23.01",      UBX_HW_VERSION_UNKNOWN, 23,  1, true,  false, 10 },
        { "unknown 15.00",      UBX_HW_VERSION_UNKNOWN, 15,  0, true,  false, 10 },
        { "unknown 14.99",      UBX_HW_VERSION_UNKNOWN, 14, 99, false, false,  5 },
        { "unknown 0.00",       UBX_HW_VERSION_UNKNOWN,  0,  0, false, false,  5 },
    };

    for (const auto &c : cases) {
        SCOPED_TRACE(c.receiver);
        EXPECT_EQ(c.canConfigureGnss, ubloxCanConfigureGnss(c.hwVersion, c.protMajor, c.protMinor));
        EXPECT_EQ(c.useM10GnssKeys, ubloxUseM10GnssKeys(c.hwVersion, c.protMajor, c.protMinor));
        EXPECT_EQ(c.navHz, ubloxNavHzFor(c.hwVersion, c.protMajor, c.protMinor, 10));
        EXPECT_EQ(c.navHz == 10, ubloxCanConfigureNavRate(c.hwVersion, c.protMajor, c.protMinor));
    }
}

TEST(GPSUbloxTest, NavHzFollowsMinimumMeasurementPeriod)
{
    const struct {
        const char *receiver;
        uint8_t hwVersion;
        uint8_t protMajor;
        uint8_t protMinor;
        uint8_t configuredHz;
        uint8_t navHz;
    } cases[] = {
        { "M5 50 Hz",            UBX_HW_VERSION_UBLOX5,   0,  0,  50,  5 },
        { "M8 18.00 10 Hz",      UBX_HW_VERSION_UBLOX8,  18,  0,  10, 10 },
        { "M8 18.00 20 Hz",      UBX_HW_VERSION_UBLOX8,  18,  0,  20, 20 },
        { "M8 18.00 21 Hz",      UBX_HW_VERSION_UBLOX8,  18,  0,  21, 20 },
        { "M8 18.00 50 Hz",      UBX_HW_VERSION_UBLOX8,  18,  0,  50, 20 },
        { "unknown 23.01 25 Hz", UBX_HW_VERSION_UNKNOWN, 23,  1,  25, 20 },
        { "unknown 24.00 25 Hz", UBX_HW_VERSION_UNKNOWN, 24,  0,  25, 25 },
        { "M10 34.10 10 Hz",     UBX_HW_VERSION_UBLOX10, 34, 10,  10, 10 },
        { "M10 34.10 40 Hz",     UBX_HW_VERSION_UBLOX10, 34, 10,  40, 40 },
        { "M10 34.10 50 Hz",     UBX_HW_VERSION_UBLOX10, 34, 10,  50, 40 },
        { "X20 50.11 200 Hz",    UBX_HW_VERSION_UBLOX20, 50, 11, 200, 40 },
    };

    for (const auto &c : cases) {
        SCOPED_TRACE(c.receiver);
        EXPECT_EQ(c.navHz, ubloxNavHzFor(c.hwVersion, c.protMajor, c.protMinor, c.configuredHz));
    }
}