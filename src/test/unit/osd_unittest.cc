#include "gtest/gtest.h"
#include "unittest_macros.h"

#include <iostream>
#include <string>

extern "C" {
#include "io/osd.h"
};

extern "C" {
uint16_t osdGlideTimeFromValues(int32_t altitude, float verticalSpeed);
}


TEST(OSDTest, TestCentiNumber)
{
   //bool osdFormatCentiNumber(char *buff, int32_t centivalue, uint32_t scale, int maxDecimals, int maxScaledDecimals, int length);
   char buf[11] = "0123456789";

   osdFormatCentiNumber(buf, 12345, 1, 2, 3, 7, false);
   std::cout << "'" << buf << "'" << std::endl;
   EXPECT_FALSE(strcmp(buf, " 123.45"));

   memset(buf, 0, 11);
   osdFormatCentiNumber(buf, 12345, 1, 2, 2, 6, false);
   std::cout << "'" << buf << "'" << std::endl;
   EXPECT_FALSE(strcmp(buf, "123.45"));

   memset(buf, 0, 11);
   osdFormatCentiNumber(buf, 12345, 1, 2, 2, 5, false);
   std::cout << "'" << buf << "'" << std::endl;
   EXPECT_FALSE(strcmp(buf, "123.4"));

   memset(buf, 0, 11);
   osdFormatCentiNumber(buf, 12345, 1, 2, 2, 4, false);
   std::cout << "'" << buf << "'" << std::endl;
   EXPECT_FALSE(strcmp(buf, " 123")); // this should be causing #8769

   memset(buf, 0, 11);
   osdFormatCentiNumber(buf, 12345, 1, 2, 2, 3, false);
   std::cout << "'" << buf << "'" << std::endl;
   EXPECT_FALSE(strcmp(buf, "123"));
   std::cout << "'" << buf << "'" << std::endl;

   memset(buf, 0, 11);
   osdFormatCentiNumber(buf, -12345, 1, 2, 2, 8, false);
   std::cout << "'" << buf << "'" << std::endl;
   EXPECT_FALSE(strcmp(buf, " -123.45"));



   memset(buf, 0, 11);
   osdFormatCentiNumber(buf, -12345, 1, 2, 2, 7, false);
   std::cout << "'" << buf << "'" << std::endl;
   EXPECT_FALSE(strcmp(buf, "-123.45"));

   memset(buf, 0, 11);
   osdFormatCentiNumber(buf, -12345, 1, 2, 2, 6, false);
   std::cout << "'" << buf << "'" << std::endl;
   EXPECT_FALSE(strcmp(buf, "-123.4"));

   memset(buf, 0, 11);
   osdFormatCentiNumber(buf, -12345, 1, 2, 2, 5, false);
   std::cout << "'" << buf << "'" << std::endl;
   EXPECT_FALSE(strcmp(buf, " -123"));

   memset(buf, 0, 11);
   osdFormatCentiNumber(buf, -12345, 1, 2, 2, 4, false);
   std::cout << "'" << buf << "'" << std::endl;
   EXPECT_FALSE(strcmp(buf, "-123"));

   EXPECT_EQ(1, 1);

}

TEST(OSDTest, TestGlideTimeDivisorGuard)
{
    // Regression: a vertical speed between -1 and 0 cm/s used to produce a
    // zero integer divisor and crash SITL with SIGFPE.
    EXPECT_EQ(0, osdGlideTimeFromValues(10000, -0.5f));
    EXPECT_EQ(0, osdGlideTimeFromValues(10000, 0.0f));
    EXPECT_EQ(0, osdGlideTimeFromValues(10000, 0.5f));
    EXPECT_EQ(10000, osdGlideTimeFromValues(10000, -1.0f));
    EXPECT_EQ(2000, osdGlideTimeFromValues(10000, -5.0f));
    EXPECT_EQ(3333, osdGlideTimeFromValues(10000, -3.0f));
}
