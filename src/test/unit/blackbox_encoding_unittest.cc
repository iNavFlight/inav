#include <algorithm>
#include <cstdarg>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <limits>
#include <string>
#include <vector>

#include "gtest/gtest.h"

extern "C" {
#include "blackbox/blackbox_encoding.h"

int32_t blackboxHeaderBudget;
void blackboxWrite(uint8_t value);
int tfp_format(void *, void (*)(void *, char), const char *, va_list);
}

static std::vector<uint8_t> encoded;

TEST(BlackboxEncoding, SlowFrameTerrainAndDroneCanOrderMatchesHeader)
{
    // Source-level contract: these optional fields share a frame but have
    // different signedness. A merge must preserve their declared wire order.
    const std::string testPath = __FILE__;
    const auto directoryEnd = testPath.find_last_of("/\\");
    ASSERT_NE(std::string::npos, directoryEnd);
    std::ifstream file(testPath.substr(0, directoryEnd) + "/../../main/blackbox/blackbox.c");
    ASSERT_TRUE(file.is_open());
    const std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    const auto headerStart = source.find("blackboxSlowFields[]");
    ASSERT_NE(std::string::npos, headerStart);
    const auto headerEnd = source.find("};", headerStart);
    ASSERT_NE(std::string::npos, headerEnd);
    const auto writerStart = source.find("static void writeSlowFrame(void)\n{");
    ASSERT_NE(std::string::npos, writerStart);
    const auto writerEnd = source.find("\n}", writerStart);
    ASSERT_NE(std::string::npos, writerEnd);

    const char *fields[] = {"droneCANBusOffCount", "terrainAGL", "terrainAMSL"};
    std::vector<std::pair<size_t, std::string>> headerOrder, writerOrder;
    for (const char *field : fields) {
        const auto declared = source.find(std::string("\"") + field + "\"", headerStart);
        const auto written = source.find(std::string("slowHistory.") + field, writerStart);
        ASSERT_LT(declared, headerEnd) << field;
        ASSERT_LT(written, writerEnd) << field;
        headerOrder.emplace_back(declared, field);
        writerOrder.emplace_back(written, field);
    }
    std::sort(headerOrder.begin(), headerOrder.end());
    std::sort(writerOrder.begin(), writerOrder.end());
    for (size_t i = 0; i < headerOrder.size(); i++) {
        EXPECT_EQ(headerOrder[i].second, writerOrder[i].second) << "Slow-frame field " << i;
    }
}

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

static std::vector<uint8_t> writeTag8_8SVB(std::vector<int32_t> values)
{
    encoded.clear();
    blackboxWriteTag8_8SVB(values.data(), (int)values.size());
    return encoded;
}

// Small values zigzag-encode to one byte: 0 -> 0, -1 -> 1, 1 -> 2, 2 -> 4, ...

TEST(BlackboxEncodingTest, Tag8_8SVBNothing)
{
    EXPECT_EQ(writeTag8_8SVB({}), std::vector<uint8_t>({}));
}

TEST(BlackboxEncodingTest, Tag8_8SVBOneFieldHasNoHeader)
{
    EXPECT_EQ(writeTag8_8SVB({0}), std::vector<uint8_t>({0x00}));
    EXPECT_EQ(writeTag8_8SVB({5}), std::vector<uint8_t>({0x0A}));
}

TEST(BlackboxEncodingTest, Tag8_8SVBHeaderMarksNonZeroFields)
{
    EXPECT_EQ(writeTag8_8SVB({1, 0, -1}), std::vector<uint8_t>({0x05, 0x02, 0x01}));
    EXPECT_EQ(writeTag8_8SVB({1, 2, 3, 4, 5, 6, 7, 8}),
              std::vector<uint8_t>({0xFF, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E, 0x10}));
}

// Decoders read a run longer than 8 as groups of 8, and a lone last field as a plain signed VB
TEST(BlackboxEncodingTest, Tag8_8SVBNinthFieldIsAPlainVB)
{
    EXPECT_EQ(writeTag8_8SVB({1, 2, 3, 4, 5, 6, 7, 8, 0}),
              std::vector<uint8_t>({0xFF, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x00}));
    EXPECT_EQ(writeTag8_8SVB({0, 0, 0, 0, 0, 0, 0, 0, 3}),
              std::vector<uint8_t>({0x00, 0x06}));
}

TEST(BlackboxEncodingTest, Tag8_8SVBSeventeenFields)
{
    EXPECT_EQ(writeTag8_8SVB({1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, -1,  2}),
              std::vector<uint8_t>({0x01, 0x02, 0x80, 0x01, 0x04}));
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
