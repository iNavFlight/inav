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

#include <iostream>
#include <string>
#include <vector>
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

static void appendPair(std::vector<uint8_t> &payload, uint32_t key, int valueBytes)
{
    for (int i = 0; i < 4; i++) {
        payload.push_back((key >> (8 * i)) & 0xff);
    }
    for (int i = 0; i < valueBytes; i++) {
        payload.push_back(1);
    }
}

TEST(GPSUbloxTest, ValgetGnss10Keys)
{
    // version 1, RAM layer, position 0
    std::vector<uint8_t> payload = { 0x01, 0x00, 0x00, 0x00 };
    appendPair(payload, UBLOX_CFG_SIGNAL_SBAS_ENA, 1);
    appendPair(payload, UBLOX_CFG_SIGNAL_GAL_ENA, 1);
    appendPair(payload, 0x30210001, 2);     // another group, two bytes: skipped
    appendPair(payload, UBLOX_CFG_SIGNAL_BDS_B1_ENA, 1);
    appendPair(payload, 0x40520001, 4);     // another group, four bytes: skipped
    appendPair(payload, UBLOX_CFG_QZSS_L1CA_ENA, 1);
    appendPair(payload, UBLOX_CFG_SIGNAL_GAL_E5A_ENA, 1);

    // Their places in INAV's key table: SBAS_ENA, GAL_ENA, BDS_B1_ENA, QZSS_L1CA_ENA, GAL_E5A_ENA
    const uint16_t expected = (1 << 0) | (1 << 1) | (1 << 7) | (1 << 9) | (1 << 12);
    EXPECT_EQ(expected, ubloxGnss10KeysFromValget(payload.data(), payload.size()));

    // A pair cut short is left out
    appendPair(payload, UBLOX_CFG_GLO_ENA, 1);
    EXPECT_EQ(expected, ubloxGnss10KeysFromValget(payload.data(), payload.size() - 1));
}

static int pairValue(const ubx_config_data8_payload_t *pairs, int count, uint32_t key)
{
    for (int i = 0; i < count; i++) {
        if (pairs[i].key == key) {
            return pairs[i].value;
        }
    }
    return -1;
}

static const uint8_t allGnss = UBX_MON_GNSS_GPS_MASK | UBX_MON_GNSS_GALILEO_MASK | UBX_MON_GNSS_BEIDOU_MASK | UBX_MON_GNSS_GLONASS_MASK;

// A single band M10 has no second band keys
static const uint16_t m10Keys = UBLOX_GNSS10_ALL_KEYS & ~UBLOX_GNSS10_L5_KEYS;

TEST(GPSUbloxTest, Gnss10PairsM10)
{
    ubx_config_data8_payload_t pairs[UBLOX_GNSS10_KEY_COUNT];
    const int count = ubloxGnss10Pairs(pairs, allGnss, true, m10Keys, false);

    EXPECT_EQ(12, count);
    EXPECT_EQ(-1, pairValue(pairs, count, UBLOX_CFG_SIGNAL_GAL_E5A_ENA));
    EXPECT_EQ(1, pairValue(pairs, count, UBLOX_CFG_SIGNAL_BDS_B1C_ENA));     // next to Glonass
    EXPECT_EQ(0, pairValue(pairs, count, UBLOX_CFG_SIGNAL_BDS_B1_ENA));
    EXPECT_EQ(1, pairValue(pairs, count, UBLOX_CFG_QZSS_L1S_ENA));
    EXPECT_EQ(1, pairValue(pairs, count, UBLOX_CFG_GLO_L1_ENA));

    // Without Glonass an M10 stays on B1I
    const uint8_t noGlonass = allGnss & ~UBX_MON_GNSS_GLONASS_MASK;
    const int count2 = ubloxGnss10Pairs(pairs, noGlonass, true, m10Keys, false);
    EXPECT_EQ(1, pairValue(pairs, count2, UBLOX_CFG_SIGNAL_BDS_B1_ENA));
    EXPECT_EQ(0, pairValue(pairs, count2, UBLOX_CFG_SIGNAL_BDS_B1C_ENA));
}

TEST(GPSUbloxTest, Gnss10PairsM10Spg500)
{
    // SPG 5.00 has neither BDS_B1C_ENA nor QZSS_L1S_ENA (#12174)
    const uint16_t known = m10Keys & ~(1 << UBLOX_GNSS10_BDS_B1C) & ~(1 << UBLOX_GNSS10_QZSS_L1S);
    ubx_config_data8_payload_t pairs[UBLOX_GNSS10_KEY_COUNT];
    const int count = ubloxGnss10Pairs(pairs, allGnss, true, known, false);

    EXPECT_EQ(10, count);
    EXPECT_EQ(-1, pairValue(pairs, count, UBLOX_CFG_SIGNAL_BDS_B1C_ENA));
    EXPECT_EQ(1, pairValue(pairs, count, UBLOX_CFG_SIGNAL_BDS_B1_ENA));
    EXPECT_EQ(-1, pairValue(pairs, count, UBLOX_CFG_QZSS_L1S_ENA));
    EXPECT_EQ(1, pairValue(pairs, count, UBLOX_CFG_GLO_ENA));
}

// No Glonass group at all, dual band
static const uint16_t f10Keys = UBLOX_GNSS10_ALL_KEYS & ~(1 << UBLOX_GNSS10_GLO) & ~(1 << UBLOX_GNSS10_GLO_L1);

TEST(GPSUbloxTest, Gnss10PairsF10)
{
    const uint8_t gnss = UBX_MON_GNSS_GPS_MASK | UBX_MON_GNSS_GALILEO_MASK | UBX_MON_GNSS_BEIDOU_MASK;
    ubx_config_data8_payload_t pairs[UBLOX_GNSS10_KEY_COUNT];
    const int count = ubloxGnss10Pairs(pairs, gnss, false, f10Keys, false);

    EXPECT_EQ(13, count);
    EXPECT_EQ(-1, pairValue(pairs, count, UBLOX_CFG_GLO_ENA));
    EXPECT_EQ(1, pairValue(pairs, count, UBLOX_CFG_SIGNAL_BDS_B1C_ENA));
    EXPECT_EQ(0, pairValue(pairs, count, UBLOX_CFG_SIGNAL_BDS_B1_ENA));
    EXPECT_EQ(0, pairValue(pairs, count, UBLOX_CFG_SIGNAL_SBAS_ENA));
    EXPECT_EQ(1, pairValue(pairs, count, UBLOX_CFG_SIGNAL_GAL_E5A_ENA));
    EXPECT_EQ(1, pairValue(pairs, count, UBLOX_CFG_SIGNAL_BDS_B2A_ENA));
    EXPECT_EQ(1, pairValue(pairs, count, UBLOX_CFG_QZSS_L5_ENA));
}

TEST(GPSUbloxTest, Gnss10PairsF10BandsTogether)
{
    // Both bands of a constellation go off with it, so that turning it on again sends both
    ubx_config_data8_payload_t pairs[UBLOX_GNSS10_KEY_COUNT];
    const int count = ubloxGnss10Pairs(pairs, UBX_MON_GNSS_GPS_MASK, false, f10Keys, false);

    EXPECT_EQ(13, count);
    EXPECT_EQ(0, pairValue(pairs, count, UBLOX_CFG_SIGNAL_GAL_E1_ENA));
    EXPECT_EQ(0, pairValue(pairs, count, UBLOX_CFG_SIGNAL_GAL_E5A_ENA));
    EXPECT_EQ(0, pairValue(pairs, count, UBLOX_CFG_SIGNAL_BDS_B1C_ENA));
    EXPECT_EQ(0, pairValue(pairs, count, UBLOX_CFG_SIGNAL_BDS_B2A_ENA));
    EXPECT_EQ(1, pairValue(pairs, count, UBLOX_CFG_QZSS_L1CA_ENA));
    EXPECT_EQ(1, pairValue(pairs, count, UBLOX_CFG_QZSS_L5_ENA));
}

TEST(GPSUbloxTest, Gnss10PairsConstellationsOnly)
{
    ubx_config_data8_payload_t pairs[UBLOX_GNSS10_KEY_COUNT];
    const int count = ubloxGnss10Pairs(pairs, allGnss, true, UBLOX_GNSS10_ALL_KEYS, true);

    EXPECT_EQ(5, count);
    EXPECT_EQ(1, pairValue(pairs, count, UBLOX_CFG_SIGNAL_BDS_ENA));
    EXPECT_EQ(-1, pairValue(pairs, count, UBLOX_CFG_SIGNAL_BDS_B1_ENA));
    EXPECT_EQ(-1, pairValue(pairs, count, UBLOX_CFG_SIGNAL_BDS_B1C_ENA));
}
