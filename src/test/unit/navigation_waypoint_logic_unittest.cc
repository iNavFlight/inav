#include <gtest/gtest.h>

extern "C" {
#include "navigation/navigation_waypoint_logic.h"
}

TEST(NavigationWaypointLogicTest, DisabledAnticipationRejectsStalePlannerArrival)
{
    EXPECT_FALSE(navWaypointAllowsTurnAnticipation(-1));
    // Signed left turns must not be mistaken for the disabled sentinel.
    EXPECT_TRUE(navWaypointAllowsTurnAnticipation(-9000));
    EXPECT_TRUE(navWaypointAllowsTurnAnticipation(9000));
    EXPECT_TRUE(navWaypointAllowsTurnAnticipation(0));
}

TEST(NavigationWaypointLogicTest, JumpToNonAnticipatingWaypointCancelsFlyIntoAwayArc)
{
    // Independent of leg-bearing change: a jump farther down the same leg
    // must not pick up the second arc belonging to the previous waypoint.
    EXPECT_TRUE(navWaypointShouldCancelTurnAnticipation(true, true, -1));
    EXPECT_FALSE(navWaypointShouldCancelTurnAnticipation(true, true, -9000));
    EXPECT_FALSE(navWaypointShouldCancelTurnAnticipation(true, true, 9000));
}

TEST(NavigationWaypointLogicTest, FlyOverEntryAndCommittedFlyIntoArcArePreserved)
{
    EXPECT_FALSE(navWaypointShouldCancelTurnAnticipation(false, true, -1));
    // A normal FLY_INTO advances its waypoint at pickup and must finish the
    // committed main arc even when the next waypoint disables anticipation.
    EXPECT_FALSE(navWaypointShouldCancelTurnAnticipation(true, false, -1));
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
