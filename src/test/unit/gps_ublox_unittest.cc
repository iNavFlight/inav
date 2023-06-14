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
#include <stdio.h>

#include "io/gps_ublox_utils.h"

void dumpCfg(const ubx_config_data8_t *cfg, int valuesAdded)
{
    printf("%02x %02x %02x %02x %04x\n", cfg->header.preamble1, cfg->header.preamble2, cfg->header.msg_class, cfg->header.msg_id, cfg->header.length);

    printf("%02x %02x %02x %02x\n", cfg->configHeader.version, cfg->configHeader.layers, cfg->configHeader.transation, cfg->configHeader.reserved);

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