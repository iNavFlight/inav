#include "gtest/gtest.h"
#include "unittest_macros.h"

#include <iostream>
#include <string>

extern "C" {
#include "io/osd.h"
#include "io/osd_utils.h"
#include "config/profile_name.h"
#include "drivers/osd_symbols.h"
};


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

TEST(OSDTest, ShorterProfileNameOverwritesPreviousSuffix)
{
    char buffer[MAX_PROFILE_NAME_LENGTH + 1];
    char display[MAX_PROFILE_NAME_LENGTH + 1] = {};
    osdFormatProfileName(buffer, "LongName", SYM_PROFILE, 1);
    memcpy(display, buffer, strlen(buffer));
    osdFormatProfileName(buffer, "A", SYM_PROFILE, 2);
    memcpy(display, buffer, strlen(buffer));
    EXPECT_EQ(std::string("A") + std::string(MAX_PROFILE_NAME_LENGTH - 1, ' '), display);
}

TEST(OSDTest, UnnamedProfilesClearPreviousName)
{
    for (const char symbol : { (char)SYM_PROFILE, (char)SYM_BATT_FULL, 'M' }) {
        char buffer[MAX_PROFILE_NAME_LENGTH + 1];
        char display[MAX_PROFILE_NAME_LENGTH + 1] = {};
        osdFormatProfileName(buffer, "LongName", symbol, 1);
        memcpy(display, buffer, strlen(buffer));
        osdFormatProfileName(buffer, "", symbol, 3);
        memcpy(display, buffer, strlen(buffer));
        EXPECT_EQ(std::string(1, symbol) + "3" + std::string(MAX_PROFILE_NAME_LENGTH - 2, ' '), display);
    }
}

TEST(OSDTest, FullWidthProfileNameIsUppercasedAndTerminated)
{
    char buffer[MAX_PROFILE_NAME_LENGTH + 2];
    memset(buffer, '!', sizeof(buffer));
    const std::string name(MAX_PROFILE_NAME_LENGTH, 'x');
    osdFormatProfileName(buffer, name.c_str(), 'M', 1);
    EXPECT_EQ(std::string(MAX_PROFILE_NAME_LENGTH, 'X'), buffer);
    EXPECT_EQ('!', buffer[MAX_PROFILE_NAME_LENGTH + 1]);
}

TEST(OSDTest, ProfileNameReadIsBounded)
{
    char name[MAX_PROFILE_NAME_LENGTH];
    memset(name, 'z', sizeof(name));
    char buffer[MAX_PROFILE_NAME_LENGTH + 1];
    osdFormatProfileName(buffer, name, 'M', 1);
    EXPECT_EQ(std::string(MAX_PROFILE_NAME_LENGTH, 'Z'), buffer);
}
