/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdarg>
#include <cstdint>
#include <vector>

extern "C" {
#include "blackbox/blackbox_encoding.h"
}

#include "gtest/gtest.h"

static std::vector<uint8_t> written;

extern "C" {
int32_t blackboxHeaderBudget;

void blackboxWrite(uint8_t value)
{
    written.push_back(value);
}

int blackboxPrint(const char *s)
{
    int n = 0;
    for (; *s; s++, n++) {
        blackboxWrite(*s);
    }
    return n;
}

int tfp_format(void *putp, void (*putf)(void *, char), const char *fmt, va_list va)
{
    (void)putp;
    (void)putf;
    (void)fmt;
    (void)va;
    return 0;
}
}

static std::vector<uint8_t> writeTag8_8SVB(std::vector<int32_t> values)
{
    written.clear();
    blackboxWriteTag8_8SVB(values.data(), (int)values.size());
    return written;
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
