#include <gtest/gtest.h>

extern "C" {
#include "navigation/navigation_waypoint_logic.h"
}

TEST(NavigationWaypointLogicTest, ResolvesFirstAndLastWaypointInSingleMission)
{
    int16_t absoluteIndex = -1;

    EXPECT_TRUE(navMissionRelativeWaypointIndexToAbsolute(0, 0, 4, 60, &absoluteIndex));
    EXPECT_EQ(0, absoluteIndex);
    EXPECT_TRUE(navMissionRelativeWaypointIndexToAbsolute(3, 0, 4, 60, &absoluteIndex));
    EXPECT_EQ(3, absoluteIndex);
}

TEST(NavigationWaypointLogicTest, ResolvesRelativeIndexInsideSelectedMultiMission)
{
    int16_t absoluteIndex = -1;

    EXPECT_TRUE(navMissionRelativeWaypointIndexToAbsolute(0, 5, 4, 60, &absoluteIndex));
    EXPECT_EQ(5, absoluteIndex);
    EXPECT_TRUE(navMissionRelativeWaypointIndexToAbsolute(3, 5, 4, 60, &absoluteIndex));
    EXPECT_EQ(8, absoluteIndex);
}

TEST(NavigationWaypointLogicTest, RejectsIndexesOutsideSelectedMission)
{
    int16_t absoluteIndex = -1;

    EXPECT_FALSE(navMissionRelativeWaypointIndexToAbsolute(-1, 5, 4, 60, &absoluteIndex));
    EXPECT_FALSE(navMissionRelativeWaypointIndexToAbsolute(4, 5, 4, 60, &absoluteIndex));
    EXPECT_FALSE(navMissionRelativeWaypointIndexToAbsolute(0, -1, 4, 60, &absoluteIndex));
    EXPECT_FALSE(navMissionRelativeWaypointIndexToAbsolute(0, 5, 0, 60, &absoluteIndex));
    EXPECT_FALSE(navMissionRelativeWaypointIndexToAbsolute(0, 58, 4, 60, &absoluteIndex));
    EXPECT_FALSE(navMissionRelativeWaypointIndexToAbsolute(0, 5, 4, 60, nullptr));
}
