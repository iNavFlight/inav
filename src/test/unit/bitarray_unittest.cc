#include <cstdint>
#include <cstring>

extern "C" {
#include "common/bitarray.h"
}

#include "gtest/gtest.h"

TEST(BitArrayTest, TestGetSet)
{
    BITARRAY_DECLARE(p, 32);
    memset(p, 0, sizeof(p));

    bitArraySet(p, 14);
    EXPECT_EQ(bitArrayGet(p, 14), true);
    EXPECT_EQ(bitArrayGet(p, 13), false);
    EXPECT_EQ(bitArrayGet(p, 15), false);

    EXPECT_EQ(bitArrayGet(p, 0), false);
    bitArraySet(p, 0);
    EXPECT_EQ(bitArrayGet(p, 0), true);
}

TEST(BitArrayTest, TestClr)
{
    BITARRAY_DECLARE(p, 32);
    memset(p, 0, sizeof(p));

    bitArraySet(p, 31);
    EXPECT_EQ(bitArrayGet(p, 31), true);
    EXPECT_EQ(bitArrayGet(p, 30), false);
    EXPECT_EQ(bitArrayGet(p, 0), false);
    bitArrayClr(p, 31);
    EXPECT_EQ(bitArrayGet(p, 31), false);
}

TEST(BitArrayTest, TestFind)
{
    BITARRAY_DECLARE(p, 32*4);
    memset(p, 0, sizeof(p));

    EXPECT_EQ(bitArrayFindFirstSet(p, 0, sizeof(p)), -1);

    bitArraySet(p, 17);
    EXPECT_EQ(bitArrayFindFirstSet(p, 0, sizeof(p)), 17);
    EXPECT_EQ(bitArrayFindFirstSet(p, 16, sizeof(p)), 17);
    EXPECT_EQ(bitArrayFindFirstSet(p, 17, sizeof(p)), 17);
    EXPECT_EQ(bitArrayFindFirstSet(p, 18, sizeof(p)), -1);

    bitArraySet(p, 44);
    EXPECT_EQ(bitArrayFindFirstSet(p, 0, sizeof(p)), 17);
    EXPECT_EQ(bitArrayFindFirstSet(p, 16, sizeof(p)), 17);
    EXPECT_EQ(bitArrayFindFirstSet(p, 17, sizeof(p)), 17);
    EXPECT_EQ(bitArrayFindFirstSet(p, 18, sizeof(p)), 44);

    bitArrayClr(p, 17);
    EXPECT_EQ(bitArrayFindFirstSet(p, 0, sizeof(p)), 44);
    EXPECT_EQ(bitArrayFindFirstSet(p, 16, sizeof(p)), 44);
    EXPECT_EQ(bitArrayFindFirstSet(p, 17, sizeof(p)), 44);
    EXPECT_EQ(bitArrayFindFirstSet(p, 18, sizeof(p)), 44);
    EXPECT_EQ(bitArrayFindFirstSet(p, 43, sizeof(p)), 44);
    EXPECT_EQ(bitArrayFindFirstSet(p, 44, sizeof(p)), 44);
    EXPECT_EQ(bitArrayFindFirstSet(p, 45, sizeof(p)), -1);

    bitArrayClr(p, 44);
    EXPECT_EQ(bitArrayFindFirstSet(p, 0, sizeof(p)), -1);
    EXPECT_EQ(bitArrayFindFirstSet(p, 64, sizeof(p)), -1);
}

TEST(BitArrayTest, TestSetClrAll)
{
    const int bits = 32 * 4;

    BITARRAY_DECLARE(p, bits);
    BITARRAY_CLR_ALL(p);

    EXPECT_EQ(-1, BITARRAY_FIND_FIRST_SET(p, 0));

    BITARRAY_SET_ALL(p);

    for (int ii = 0; ii < bits; ii++) {
        EXPECT_EQ(ii, BITARRAY_FIND_FIRST_SET(p, ii));
    }
}
