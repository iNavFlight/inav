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

TEST(NavigationVtolMissionLogicTest, SuccessfulMissionToFixedWingAdvancesAcceptedPlainWaypoint)
{
    const bool acceptedForAdvance = navMissionTransitionWaypointAcceptedForAdvance(
        true,
        true,
        100,
        12000.0f,
        12000.0f);

    EXPECT_TRUE(navMissionShouldAdvanceWaypointAfterTransition(
        acceptedForAdvance,
        true));
}

TEST(NavigationVtolMissionLogicTest, SuccessfulMissionToMultirotorAdvancesAcceptedPlainWaypoint)
{
    const bool acceptedForAdvance = navMissionTransitionWaypointAcceptedForAdvance(
        true,
        true,
        100,
        12000.0f,
        12000.0f);

    EXPECT_TRUE(navMissionShouldAdvanceWaypointAfterTransition(
        acceptedForAdvance,
        true));
}

TEST(NavigationVtolMissionLogicTest, AcceptedWaypointAdvancesDespiteAltitudeLossDuringTransition)
{
    const bool acceptedForAdvance = navMissionTransitionWaypointAcceptedForAdvance(
        true,
        true,
        100,
        12000.0f,
        12000.0f);

    EXPECT_TRUE(navMissionShouldAdvanceWaypointAfterTransition(
        acceptedForAdvance,
        true));
}

TEST(NavigationVtolMissionLogicTest, MissionTransitionStartedBelowEnforcedAltitudeDoesNotLatchWaypoint)
{
    const bool acceptedForAdvance = navMissionTransitionWaypointAcceptedForAdvance(
        true,
        true,
        100,
        11899.0f,
        12000.0f);

    EXPECT_FALSE(acceptedForAdvance);
    EXPECT_FALSE(navMissionShouldAdvanceWaypointAfterTransition(
        acceptedForAdvance,
        true));
}

TEST(NavigationVtolMissionLogicTest, MissionTransitionKeepsLegacyAdvanceWhenAltitudeEnforcementIsDisabled)
{
    const bool acceptedForAdvance = navMissionTransitionWaypointAcceptedForAdvance(
        true,
        true,
        0,
        11000.0f,
        12000.0f);

    EXPECT_TRUE(navMissionShouldAdvanceWaypointAfterTransition(
        acceptedForAdvance,
        true));
}

TEST(NavigationVtolMissionLogicTest, MissionTransitionDoesNotAdvanceHoldWaypointOrAbort)
{
    const bool holdWaypointAccepted = navMissionTransitionWaypointAcceptedForAdvance(
        true,
        false,
        100,
        12000.0f,
        12000.0f);

    EXPECT_FALSE(navMissionShouldAdvanceWaypointAfterTransition(
        holdWaypointAccepted,
        true));

    EXPECT_FALSE(navMissionShouldAdvanceWaypointAfterTransition(
        true,
        false));
}

TEST(NavigationVtolMissionLogicTest, ForcedFwToMcProfileSwitchAdvancesAcceptedWaypoint)
{
    const bool acceptedForAdvance = navMissionTransitionWaypointAcceptedForAdvance(
        true,
        true,
        300,
        12042.0f,
        12000.0f);

    EXPECT_TRUE(navMissionTransitionReachedTargetProfile(true, true));
    EXPECT_TRUE(navMissionShouldAdvanceWaypointAfterTransition(
        acceptedForAdvance,
        navMissionTransitionReachedTargetProfile(true, true)));
}

TEST(NavigationVtolMissionLogicTest, AbortedTransitionWithoutProfileSwitchDoesNotAdvanceWaypoint)
{
    EXPECT_FALSE(navMissionTransitionReachedTargetProfile(true, false));
    EXPECT_FALSE(navMissionShouldAdvanceWaypointAfterTransition(
        true,
        navMissionTransitionReachedTargetProfile(true, false)));
}

TEST(NavigationVtolMissionLogicTest, FwToMcHoldWaypointCapturesBeforeMissionResume)
{
    EXPECT_TRUE(navMissionTransitionShouldCaptureBeforeWaypointResume(
        true,
        true,
        true,
        true,
        false));

    EXPECT_FALSE(navMissionTransitionShouldRebaseWaypointBeforeResume(
        true,
        true,
        false,
        true));
}

TEST(NavigationVtolMissionLogicTest, McToFwHoldWaypointRebasesWithoutMultirotorCapture)
{
    EXPECT_FALSE(navMissionTransitionShouldCaptureBeforeWaypointResume(
        true,
        true,
        false,
        false,
        false));

    EXPECT_TRUE(navMissionTransitionShouldRebaseWaypointBeforeResume(
        true,
        true,
        false,
        false));
}

TEST(NavigationVtolMissionLogicTest, WaypointAdvanceAndAbortedTransitionSkipResumeHandling)
{
    EXPECT_FALSE(navMissionTransitionShouldCaptureBeforeWaypointResume(
        true,
        true,
        true,
        true,
        true));
    EXPECT_FALSE(navMissionTransitionShouldRebaseWaypointBeforeResume(
        true,
        true,
        true,
        false));

    EXPECT_FALSE(navMissionTransitionShouldCaptureBeforeWaypointResume(
        true,
        false,
        true,
        true,
        false));
    EXPECT_FALSE(navMissionTransitionShouldRebaseWaypointBeforeResume(
        true,
        false,
        false,
        false));
}

TEST(NavigationVtolMissionLogicTest, MissionTransitionSuppressesWaypointFallbackOnlyWhileInProgress)
{
    EXPECT_TRUE(navMissionVtolTransitionSuppressesWaypointRestartGuard(true));
    EXPECT_TRUE(navMissionVtolTransitionSuppressesWaypointFallbackToRth(true));
    EXPECT_TRUE(navMissionVtolTransitionHoldsWaypointSelector(true, true));

    EXPECT_FALSE(navMissionVtolTransitionSuppressesWaypointRestartGuard(false));
    EXPECT_FALSE(navMissionVtolTransitionSuppressesWaypointFallbackToRth(false));
    EXPECT_FALSE(navMissionVtolTransitionHoldsWaypointSelector(false, true));
    EXPECT_FALSE(navMissionVtolTransitionHoldsWaypointSelector(true, false));
}

TEST(NavigationVtolMissionLogicTest, MixerATOwnerKeepsMatchingNavigationMode)
{
    EXPECT_TRUE(navVtolMixerATModeRequestIsOwned(
        true,
        NAV_VTOL_MIXERAT_MODE_WAYPOINT,
        NAV_VTOL_MIXERAT_MODE_WAYPOINT));
    EXPECT_TRUE(navVtolMixerATModeRequestIsOwned(
        true,
        NAV_VTOL_MIXERAT_MODE_RTH,
        NAV_VTOL_MIXERAT_MODE_RTH));
    EXPECT_TRUE(navVtolMixerATModeRequestIsOwned(
        true,
        NAV_VTOL_MIXERAT_MODE_LAND,
        NAV_VTOL_MIXERAT_MODE_LAND));

    EXPECT_FALSE(navVtolMixerATModeRequestShouldPreempt(
        true,
        NAV_VTOL_MIXERAT_MODE_RTH,
        NAV_VTOL_MIXERAT_MODE_RTH));
}

TEST(NavigationVtolMissionLogicTest, DifferentNavigationModePreemptsMixerAT)
{
    EXPECT_TRUE(navVtolMixerATModeRequestShouldPreempt(
        true,
        NAV_VTOL_MIXERAT_MODE_WAYPOINT,
        NAV_VTOL_MIXERAT_MODE_RTH));
    EXPECT_TRUE(navVtolMixerATModeRequestShouldPreempt(
        true,
        NAV_VTOL_MIXERAT_MODE_RTH,
        NAV_VTOL_MIXERAT_MODE_LAND));
    EXPECT_TRUE(navVtolMixerATModeRequestShouldPreempt(
        true,
        NAV_VTOL_MIXERAT_MODE_LAND,
        NAV_VTOL_MIXERAT_MODE_POSHOLD));
    EXPECT_TRUE(navVtolMixerATModeRequestShouldPreempt(
        true,
        NAV_VTOL_MIXERAT_MODE_WAYPOINT,
        NAV_VTOL_MIXERAT_MODE_EMERGENCY_LANDING));
}

TEST(NavigationVtolMissionLogicTest, NavigationModePreemptsUnownedManualMixerAT)
{
    EXPECT_TRUE(navVtolMixerATModeRequestShouldPreempt(
        true,
        NAV_VTOL_MIXERAT_MODE_NONE,
        NAV_VTOL_MIXERAT_MODE_RTH));
}

TEST(NavigationVtolMissionLogicTest, InactiveMixerATNeverOwnsOrPreemptsNavigation)
{
    EXPECT_FALSE(navVtolMixerATModeRequestIsOwned(
        false,
        NAV_VTOL_MIXERAT_MODE_RTH,
        NAV_VTOL_MIXERAT_MODE_RTH));
    EXPECT_FALSE(navVtolMixerATModeRequestShouldPreempt(
        false,
        NAV_VTOL_MIXERAT_MODE_WAYPOINT,
        NAV_VTOL_MIXERAT_MODE_LAND));
    EXPECT_FALSE(navVtolMixerATModeRequestShouldPreempt(
        true,
        NAV_VTOL_MIXERAT_MODE_WAYPOINT,
        NAV_VTOL_MIXERAT_MODE_NONE));
}

TEST(NavigationVtolMissionLogicTest, MultirotorWaypointHoldsForEnforcedAltitudeInsideRadius)
{
    EXPECT_TRUE(navMissionShouldHoldMultirotorWaypointForAltitude(
        true,
        false,
        100,
        250.0f,
        250.0f,
        500,
        12000.0f));
}

TEST(NavigationVtolMissionLogicTest, MultirotorWaypointKeepsAltitudeHoldWhenStartedInsideRadius)
{
    EXPECT_TRUE(navMissionShouldHoldMultirotorWaypointForAltitude(
        true,
        false,
        100,
        650.0f,
        250.0f,
        500,
        12000.0f));
}

TEST(NavigationVtolMissionLogicTest, MultirotorWaypointDoesNotHoldWhenAltitudeReached)
{
    EXPECT_FALSE(navMissionShouldHoldMultirotorWaypointForAltitude(
        true,
        false,
        100,
        250.0f,
        250.0f,
        500,
        99.0f));
}

TEST(NavigationVtolMissionLogicTest, MultirotorWaypointAltitudeHoldIsDisabledWithoutEnforceAltitude)
{
    EXPECT_FALSE(navMissionShouldHoldMultirotorWaypointForAltitude(
        true,
        false,
        0,
        250.0f,
        250.0f,
        500,
        12000.0f));
}

TEST(NavigationVtolMissionLogicTest, MultirotorWaypointAltitudeHoldDoesNotAffectFixedWing)
{
    EXPECT_FALSE(navMissionShouldHoldMultirotorWaypointForAltitude(
        false,
        false,
        100,
        250.0f,
        250.0f,
        500,
        12000.0f));
}

TEST(NavigationVtolMissionLogicTest, MultirotorWaypointAltitudeHoldWaitsForHorizontalRadius)
{
    EXPECT_FALSE(navMissionShouldHoldMultirotorWaypointForAltitude(
        true,
        false,
        100,
        501.0f,
        501.0f,
        500,
        12000.0f));
}

TEST(NavigationVtolMissionLogicTest, MultirotorWaypointKeepsAltitudeHoldAfterEnteringRadius)
{
    EXPECT_TRUE(navMissionUpdateMultirotorWaypointAltitudeEnforceActive(
        true,
        false,
        false,
        100,
        250.0f,
        650.0f,
        500));

    EXPECT_TRUE(navMissionShouldHoldMultirotorWaypointForAltitude(
        true,
        true,
        100,
        650.0f,
        650.0f,
        500,
        12000.0f));
}

TEST(NavigationVtolMissionLogicTest, FirstMultirotorWaypointCanEnforceAltitudeBeforeRadius)
{
    EXPECT_TRUE(navMissionUpdateMultirotorWaypointAltitudeEnforceActive(
        true,
        false,
        true,
        100,
        1200.0f,
        1200.0f,
        500));

    EXPECT_TRUE(navMissionShouldHoldMultirotorWaypointForAltitude(
        true,
        true,
        100,
        1200.0f,
        1200.0f,
        500,
        12000.0f));
}

TEST(NavigationVtolMissionLogicTest, NonFirstMultirotorWaypointStillWaitsForRadius)
{
    EXPECT_FALSE(navMissionUpdateMultirotorWaypointAltitudeEnforceActive(
        true,
        false,
        false,
        100,
        1200.0f,
        1200.0f,
        500));
}

TEST(NavigationVtolMissionLogicTest, FirstGeoWaypointAltitudeForceRequiresMultirotor)
{
    EXPECT_TRUE(navMissionShouldForceFirstGeoWaypointAltitudeFromStart(
        true,
        true));

    EXPECT_FALSE(navMissionShouldForceFirstGeoWaypointAltitudeFromStart(
        true,
        false));

    EXPECT_FALSE(navMissionShouldForceFirstGeoWaypointAltitudeFromStart(
        false,
        true));
}

TEST(NavigationVtolMissionLogicTest, FirstMultirotorWaypointHoldsXyUntilAltitudeReached)
{
    EXPECT_TRUE(navMissionShouldHoldFirstMultirotorWaypointXyForAltitude(
        true,
        true,
        100,
        12000.0f));

    EXPECT_FALSE(navMissionShouldHoldFirstMultirotorWaypointXyForAltitude(
        true,
        true,
        100,
        99.0f));

    EXPECT_FALSE(navMissionShouldHoldFirstMultirotorWaypointXyForAltitude(
        true,
        false,
        100,
        12000.0f));

    EXPECT_FALSE(navMissionShouldHoldFirstMultirotorWaypointXyForAltitude(
        false,
        true,
        100,
        12000.0f));
}

TEST(NavigationVtolMissionLogicTest, ActiveMultirotorAltitudeEnforcementKeepsWaypointAltitude)
{
    EXPECT_FLOAT_EQ(12000.0f, navMissionMultirotorWaypointAltitudeTarget(
        true,
        true,
        12000.0f,
        250.0f));

    EXPECT_FLOAT_EQ(250.0f, navMissionMultirotorWaypointAltitudeTarget(
        true,
        false,
        12000.0f,
        250.0f));

    EXPECT_FLOAT_EQ(250.0f, navMissionMultirotorWaypointAltitudeTarget(
        false,
        true,
        12000.0f,
        250.0f));
}
