#include "gtest/gtest.h"
#include "unittest_macros.h"

#include <iostream>
#include <string>
#include <stdio.h>

#include "io/gps_ublox_utils.h"

TEST(GPSUbloxTest, TestUbloxCfgFillBytes)
{
    ubx_config_data8_t cfg = {};
    ubx_config_data8_payload_t kvPairs[] = {
        { 0x04, 0x20},
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

    printf("%02x %02x %02x %02x %04x\n", cfg.header.preamble1, cfg.header.preamble2, cfg.header.msg_class, cfg.header.msg_id, cfg.header.length);

    printf("%02x %02x %02x\n", cfg.configHeader.version, cfg.configHeader.layers, cfg.configHeader.reserved);

    for(int i =0; i < valuesAdded; ++i) {
        printf("%i: %08x %02x\n", i+1, cfg.data.payload[i].key, cfg.data.payload[i].value);
    }

    uint8_t *chksums = (uint8_t *)&cfg.data.payload[valuesAdded];

    printf("%02x %02x\n", chksums[0], chksums[1]);

    // osdFormatCentiNumber(buf, 12345, 1, 2, 3, 7);
    // std::cout << "'" << buf << "'" << std::endl;
    //EXPECT_FALSE(strcmp(buf, " 123.45"));
}