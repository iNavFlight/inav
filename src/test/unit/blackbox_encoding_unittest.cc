#include <algorithm>
#include <cstdarg>
#include <cstdint>
#include <limits>
#include <vector>

#include "gtest/gtest.h"

extern "C" {
#include "blackbox/blackbox_encoding.h"

int32_t blackboxHeaderBudget;
void blackboxWrite(uint8_t value);
int tfp_format(void *, void (*)(void *, char), const char *, va_list);
}

static std::vector<uint8_t> encoded;

extern "C" void blackboxWrite(uint8_t value)
{
    encoded.push_back(value);
}

extern "C" int blackboxPrint(const char *)
{
    return 0;
}

extern "C" int tfp_format(void *, void (*)(void *, char), const char *, va_list)
{
    return 0;
}

static int32_t readSignedVB(size_t &offset)
{
    uint32_t value = 0;
    for (unsigned shift = 0; shift < 35; shift += 7) {
        if (offset >= encoded.size()) {
            ADD_FAILURE() << "Truncated variable-byte value";
            return 0;
        }
        const uint8_t byte = encoded[offset++];
        value |= uint32_t(byte & 0x7f) << shift;
        if (!(byte & 0x80)) {
            return (value & 1) ? -1 - int32_t(value >> 1) : int32_t(value >> 1);
        }
    }
    ADD_FAILURE() << "Invalid variable-byte value";
    return 0;
}

static void expectRoundTrip(std::vector<int32_t> values)
{
    encoded.clear();
    blackboxWriteTag8_8SVB(values.data(), values.size());
    // A following field must remain aligned, even when the ninth delta is zero.
    constexpr int32_t followingField = -12345;
    blackboxWriteSignedVB(followingField);

    size_t offset = 0;
    for (size_t first = 0; first < values.size(); first += 8) {
        const size_t count = std::min(size_t(8), values.size() - first);
        if (count == 1) {
            EXPECT_EQ(values[first], readSignedVB(offset));
            continue;
        }
        ASSERT_LT(offset, encoded.size());
        const uint8_t header = encoded[offset++];
        for (size_t i = 0; i < count; i++) {
            const int32_t value = (header & (1U << i)) ? readSignedVB(offset) : 0;
            EXPECT_EQ(values[first + i], value) << "Field " << first + i;
        }
    }
    EXPECT_EQ(followingField, readSignedVB(offset));
    EXPECT_EQ(encoded.size(), offset);
}

TEST(BlackboxEncoding, ExistingGroupsKeepTheirWireFormat)
{
    int32_t values[] = {0, -1, 0, 64, 0, 0, 0, 1};
    encoded.clear();
    blackboxWriteTag8_8SVB(values, 8);
    EXPECT_EQ((std::vector<uint8_t>{0x8a, 0x01, 0x80, 0x01, 0x02}), encoded);

    encoded.clear();
    blackboxWriteTag8_8SVB(values + 7, 1);
    EXPECT_EQ((std::vector<uint8_t>{0x02}), encoded);
}

TEST(BlackboxEncoding, NinthOptionalFieldIsAlwaysWritten)
{
    for (const int32_t rssiDelta : {0, 1, -1, 1023, -1023}) {
        SCOPED_TRACE(rssiDelta);
        std::vector<int32_t> values(9, 0);
        values[7] = -500;
        values[8] = rssiDelta;
        expectRoundTrip(values);
    }
}

TEST(BlackboxEncoding, EveryOptionalSensorCombinationAndPresenceMask)
{
    // VBAT, current, three compass axes, barometer, pitot, rangefinder, RSSI.
    constexpr unsigned widths[] = {1, 1, 3, 1, 1, 1, 1};
    for (unsigned sensors = 0; sensors < (1U << 7); sensors++) {
        unsigned count = 0;
        for (unsigned i = 0; i < 7; i++) {
            if (sensors & (1U << i)) {
                count += widths[i];
            }
        }
        for (unsigned presence = 0; presence < (1U << count); presence++) {
            SCOPED_TRACE(sensors);
            SCOPED_TRACE(presence);
            std::vector<int32_t> values(count, 0);
            for (unsigned i = 0; i < count; i++) {
                if (presence & (1U << i)) {
                    values[i] = (i & 1) ? -int32_t(128 + i) : int32_t(128 + i);
                }
            }
            expectRoundTrip(values);
        }
    }
}

TEST(BlackboxEncoding, MultipleGroupsAndIntegerLimits)
{
    for (unsigned count = 0; count <= 25; count++) {
        SCOPED_TRACE(count);
        std::vector<int32_t> values(count);
        for (unsigned i = 0; i < count; i++) {
            values[i] = (i & 1) ? std::numeric_limits<int32_t>::min() : std::numeric_limits<int32_t>::max();
        }
        expectRoundTrip(values);
    }
}
