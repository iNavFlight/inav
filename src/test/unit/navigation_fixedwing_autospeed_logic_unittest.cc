#include <gtest/gtest.h>

extern "C" {
#include "navigation/navigation_fixedwing_autospeed_logic.h"
}

TEST(NavigationFixedWingAutoSpeedLogicTest, RunsOnlyInFixedWingWhenEnabled)
{
    EXPECT_TRUE(navFixedWingAutoSpeedMayControlThrottle(true, true, false));
    EXPECT_FALSE(navFixedWingAutoSpeedMayControlThrottle(false, true, false));
    EXPECT_FALSE(navFixedWingAutoSpeedMayControlThrottle(true, false, false));
}

TEST(NavigationFixedWingAutoSpeedLogicTest, TransitionControllerAlwaysOwnsThrottle)
{
    EXPECT_FALSE(navFixedWingAutoSpeedMayControlThrottle(true, true, true));
    EXPECT_FALSE(navFixedWingAutoSpeedMayControlThrottle(false, true, true));
}
