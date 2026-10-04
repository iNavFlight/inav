#include <string>
#include <vector>
#include "gtest/gtest.h"
#include "fc/cli_string.h"

static std::string output;
static void append(uint8_t ch) { output += static_cast<char>(ch); }
TEST(CliString, DumpAndRestorePreserveHashesQuotesBackslashesAndSpaces) {
    for (const char *value : {"RACE #1", "  PROFILE  ", "A\"#B", "A\\\"#B", "", "#", "C:\\FLIGHT"}) {
        output.clear();
        cliWriteQuotedString(value, append);
        const auto quotedLength = output.size();
        output += " # trailing comment";
        EXPECT_EQ(quotedLength + 1, cliUncommentedLength(output.data(), output.size()));
        std::vector<char> restored(output.begin(), output.begin() + quotedLength);
        restored.push_back(0);
        EXPECT_STREQ(value, cliUnquoteString(restored.data()));
    }
}
TEST(CliString, OrdinaryCommentsAndUnquotedSettingsKeepTheirMeaning) {
    EXPECT_EQ(0u, cliUncommentedLength("#default set name = X", 21));
    EXPECT_EQ(13u, cliUncommentedLength("set name = X # note", 19));
    char raw[] = "A\\B";
    EXPECT_STREQ("A\\B", cliUnquoteString(raw));
}
