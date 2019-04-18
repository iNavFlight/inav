#include <string>

extern "C" {
#include "common/olc.h"
#include "common/utils.h"
}

#include "gtest/gtest.h"

using std::string;

struct EncodeCase {
    string result;
    double lat;
    double lon;

    int length();
};

int EncodeCase::length()
{
    int n = 0;
    for (size_t ii = 0; ii < this->result.length(); ii++) {
        if (result[ii] == '0') {
            break;
        }
        if (result[ii] != '+') {
            n++;
        }
    }
    return n;
}

// Tests cases from https://github.com/google/open-location-code/blob/master/test_data/encodingTests.csv
struct EncodeCase encodeCases[] = {
    {"7FG49Q00+",       20.375, 2.775},
    {"7FG49QCJ+2V",     20.3700625, 2.7821875},
    {"7FG49QCJ+2VX",    20.3701125, 2.782234375},
    {"7FG49QCJ+2VXGJ",  20.3701135, 2.78223535156},
    {"8FVC2222+22",     47.0000625, 8.0000625},
    {"4VCPPQGP+Q9",     -41.2730625, 174.7859375},
    {"62G20000+",       0.5, -179.5},
    {"22220000+",       -89.5, -179.5},
    {"7FG40000+",       20.5, 2.5},
    {"22222222+22",     -89.9999375, -179.9999375},
    {"6VGX0000+",       0.5, 179.5},
    {"6FH32222+222",    1, 1},
    // Special cases over 90 latitude and 180 longitude
    {"CFX30000+",       90, 1},
    {"CFX30000+",       92, 1},
    {"62H20000+",       1, 180},
    {"62H30000+",       1, 181},
    {"CFX3X2X2+X2",     90, 1},
    // Test non-precise latitude/longitude value
    {"6FH56C22+22",     1.2, 3.4},
};

TEST(OLCTest, TestEncode)
{
    char buf[20];

    for (unsigned ii = 0; ii < ARRAYLEN(encodeCases); ii++) {
        struct EncodeCase c = encodeCases[ii];
        int32_t lat = c.lat * OLC_DEG_MULTIPLIER;
        int32_t lon = c.lon * OLC_DEG_MULTIPLIER;
        EXPECT_GT(olc_encode(lat, lon, c.length(), buf, sizeof(buf)), 0);
        EXPECT_EQ(c.result, (string)buf);
    }
}
