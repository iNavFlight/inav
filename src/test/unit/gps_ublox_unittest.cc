#include "gtest/gtest.h"
#include "unittest_macros.h"

#include <iostream>
#include <string>

extern "C" {
#include "io/gps_ublox.h"
};


TEST(GPSUbloxTest, TestUbloxCfgFillBytes)
{
    ubx_config_data8_t cfg = {};
    ubx_config_data8_payload_t kvPairs = {
        { 0x10, 0x1},
        { 0x42, 0x69}
    };

    ubloxCfgFillBytes(&cfg, &kvPairs, 2);

    // osdFormatCentiNumber(buf, 12345, 1, 2, 3, 7);
    // std::cout << "'" << buf << "'" << std::endl;
    //EXPECT_FALSE(strcmp(buf, " 123.45"));
}