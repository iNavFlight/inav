#include "gtest/gtest.h"
#include "unittest_macros.h"

#include <iostream>
#include <string>

extern "C" {
#include "io/osd.h"
};


TEST(OSDTest, TestCentiNumber)
{
   //bool osdFormatCentiNumber(char *buff, int32_t centivalue, uint32_t scale, int maxDecimals, int maxScaledDecimals, int length);
   char buf[10] = "";

   osdFormatCentiNumber(buf, 10000, 1, 2, 2, 5);

   std::cout << buf << std::endl;

   EXPECT_EQ(1, 1);

}