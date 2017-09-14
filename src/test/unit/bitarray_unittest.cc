#include <cstdint>

extern "C" {
#include "common/bitarray.h"
}

#include "gtest/gtest.h"

TEST(BitArrayTest, TestGetSet)
{
    uint32_t array[1] = {0};
    void *p = array;

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
    uint32_t array[1] = {0};
    void *p = array;

    bitArraySet(p, 31);
    EXPECT_EQ(bitArrayGet(p, 31), true);
    EXPECT_EQ(bitArrayGet(p, 30), false);
    EXPECT_EQ(bitArrayGet(p, 0), false);
    bitArrayClr(p, 31);
    EXPECT_EQ(bitArrayGet(p, 31), false);
}

TEST(BitArrayTest, TestFind)
{
    uint32_t array[4] = {0, 0, 0, 0};
    void *p = array;

    EXPECT_EQ(bitArrayFindFirstSet(p, 0, sizeof(array)), -1);

    bitArraySet(p, 17);
    EXPECT_EQ(bitArrayFindFirstSet(p, 0, sizeof(array)), 17);
    EXPECT_EQ(bitArrayFindFirstSet(p, 16, sizeof(array)), 17);
    EXPECT_EQ(bitArrayFindFirstSet(p, 17, sizeof(array)), 17);
    EXPECT_EQ(bitArrayFindFirstSet(p, 18, sizeof(array)), -1);

    bitArraySet(p, 44);
    EXPECT_EQ(bitArrayFindFirstSet(p, 0, sizeof(array)), 17);
    EXPECT_EQ(bitArrayFindFirstSet(p, 16, sizeof(array)), 17);
    EXPECT_EQ(bitArrayFindFirstSet(p, 17, sizeof(array)), 17);
    EXPECT_EQ(bitArrayFindFirstSet(p, 18, sizeof(array)), 44);

    bitArrayClr(p, 17);
    EXPECT_EQ(bitArrayFindFirstSet(p, 0, sizeof(array)), 44);
    EXPECT_EQ(bitArrayFindFirstSet(p, 16, sizeof(array)), 44);
    EXPECT_EQ(bitArrayFindFirstSet(p, 17, sizeof(array)), 44);
    EXPECT_EQ(bitArrayFindFirstSet(p, 18, sizeof(array)), 44);
    EXPECT_EQ(bitArrayFindFirstSet(p, 45, sizeof(array)), -1);

    bitArrayClr(p, 44);
    EXPECT_EQ(bitArrayFindFirstSet(p, 0, sizeof(array)), -1);
    EXPECT_EQ(bitArrayFindFirstSet(p, 64, sizeof(array)), -1);
}
