#include <gtest/gtest.h>

extern "C" {
#include "navigation/navigation_vtol_mission_logic.h"
}

TEST(NavigationVtolMissionLogicTest, ReadyWhenAllPreconditionsAreMet)
{
    EXPECT_EQ(NAV_MISSION_VTOL_PRECONDITION_READY,
        navMissionVtolTransitionPreconditionDisposition(
            true,
            false,
            false,
            true,
            true,
            true,
            true,
            false));
}

TEST(NavigationVtolMissionLogicTest, TemporaryRuntimeConditionsWait)
{
    EXPECT_EQ(NAV_MISSION_VTOL_PRECONDITION_WAIT,
        navMissionVtolTransitionPreconditionDisposition(
            true,
            false,
            true,
            true,
            true,
            true,
            true,
            false));

    EXPECT_EQ(NAV_MISSION_VTOL_PRECONDITION_WAIT,
        navMissionVtolTransitionPreconditionDisposition(
            true,
            false,
            false,
            false,
            true,
            true,
            true,
            false));

    EXPECT_EQ(NAV_MISSION_VTOL_PRECONDITION_WAIT,
        navMissionVtolTransitionPreconditionDisposition(
            true,
            false,
            false,
            true,
            false,
            true,
            true,
            false));

    EXPECT_EQ(NAV_MISSION_VTOL_PRECONDITION_WAIT,
        navMissionVtolTransitionPreconditionDisposition(
            true,
            false,
            false,
            true,
            true,
            true,
            true,
            true));
}

TEST(NavigationVtolMissionLogicTest, HardSafetyAndConfigurationProblemsReject)
{
    EXPECT_EQ(NAV_MISSION_VTOL_PRECONDITION_REJECT,
        navMissionVtolTransitionPreconditionDisposition(
            false,
            false,
            false,
            true,
            true,
            true,
            true,
            false));

    EXPECT_EQ(NAV_MISSION_VTOL_PRECONDITION_REJECT,
        navMissionVtolTransitionPreconditionDisposition(
            true,
            true,
            false,
            true,
            true,
            true,
            true,
            false));

    EXPECT_EQ(NAV_MISSION_VTOL_PRECONDITION_REJECT,
        navMissionVtolTransitionPreconditionDisposition(
            true,
            false,
            false,
            true,
            true,
            false,
            true,
            false));

    EXPECT_EQ(NAV_MISSION_VTOL_PRECONDITION_REJECT,
        navMissionVtolTransitionPreconditionDisposition(
            true,
            false,
            false,
            true,
            true,
            true,
            false,
            false));
}

TEST(NavigationVtolMissionLogicTest, StartValidationRejectsBadTargetProfile)
{
    EXPECT_EQ(NAV_MISSION_VTOL_START_VALIDATION_REJECT,
        navMissionVtolTransitionStartValidation(
            false,
            true));
}

TEST(NavigationVtolMissionLogicTest, StartValidationUsesFailActionWhenRequestCannotStart)
{
    EXPECT_EQ(NAV_MISSION_VTOL_START_VALIDATION_FAIL_ACTION,
        navMissionVtolTransitionStartValidation(
            true,
            false));
}

TEST(NavigationVtolMissionLogicTest, StartValidationAllowsValidTransitionStart)
{
    EXPECT_EQ(NAV_MISSION_VTOL_START_VALIDATION_READY,
        navMissionVtolTransitionStartValidation(
            true,
            true));
}
