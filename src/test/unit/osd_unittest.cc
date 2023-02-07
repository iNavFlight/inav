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
   char buf[11] = "0123456789";

   osdFormatCentiNumber(buf, 12345, 1, 2, 3, 7);
   std::cout << "'" << buf << "'" << std::endl;
   EXPECT_FALSE(strcmp(buf, " 123.45"));

   memset(buf, 0, 11);
   osdFormatCentiNumber(buf, 12345, 1, 2, 2, 6);
   std::cout << "'" << buf << "'" << std::endl;
   EXPECT_FALSE(strcmp(buf, "123.45"));

   memset(buf, 0, 11);
   osdFormatCentiNumber(buf, 12345, 1, 2, 2, 5);
   std::cout << "'" << buf << "'" << std::endl;
   EXPECT_FALSE(strcmp(buf, "123.4"));

   memset(buf, 0, 11);
   osdFormatCentiNumber(buf, 12345, 1, 2, 2, 4);
   std::cout << "'" << buf << "'" << std::endl;
   EXPECT_FALSE(strcmp(buf, " 123")); // this should be causing #8769

   memset(buf, 0, 11);
   osdFormatCentiNumber(buf, 12345, 1, 2, 2, 3);
   std::cout << "'" << buf << "'" << std::endl;
   EXPECT_FALSE(strcmp(buf, "123"));
   std::cout << "'" << buf << "'" << std::endl;

   memset(buf, 0, 11);
   osdFormatCentiNumber(buf, -12345, 1, 2, 2, 8);
   std::cout << "'" << buf << "'" << std::endl;
   EXPECT_FALSE(strcmp(buf, " -123.45"));



   memset(buf, 0, 11);
   osdFormatCentiNumber(buf, -12345, 1, 2, 2, 7);
   std::cout << "'" << buf << "'" << std::endl;
   EXPECT_FALSE(strcmp(buf, "-123.45"));

   memset(buf, 0, 11);
   osdFormatCentiNumber(buf, -12345, 1, 2, 2, 6);
   std::cout << "'" << buf << "'" << std::endl;
   EXPECT_FALSE(strcmp(buf, "-123.4"));

   memset(buf, 0, 11);
   osdFormatCentiNumber(buf, -12345, 1, 2, 2, 5);
   std::cout << "'" << buf << "'" << std::endl;
   EXPECT_FALSE(strcmp(buf, " -123"));

   memset(buf, 0, 11);
   osdFormatCentiNumber(buf, -12345, 1, 2, 2, 4);
   std::cout << "'" << buf << "'" << std::endl;
   EXPECT_FALSE(strcmp(buf, "-123"));

   EXPECT_EQ(1, 1);

}