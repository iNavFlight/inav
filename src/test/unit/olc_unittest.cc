#include <string>

extern "C" {
#include "common/olc.h"
#include "common/utils.h"
}

#include "gtest/gtest.h"

using std::string;

struct EncodeCase {
    double lat;
    double lon;
    size_t length;
    string result;
};

struct EncodeCase encodeCases[] = {
    {47.0000625, 8.0000625, 10, "8FVC2222+22"},
    {47.0000625, 8.0000625, 13, "8FVC2222+22GCC"},
};

TEST(OLCTest, TestEncode)
{
    char buf[20];

    for (unsigned ii = 0; ii < ARRAYLEN(encodeCases); ii++) {
        struct EncodeCase c = encodeCases[ii];
        int32_t lat = c.lat * OLC_DEG_MULTIPLIER;
        int32_t lon = c.lon * OLC_DEG_MULTIPLIER;
        EXPECT_GT(olc_encode(lat, lon, c.length, buf, sizeof(buf)), 0);
        EXPECT_EQ(c.result, (string)buf);
    }
}
