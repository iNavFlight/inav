#include <gtest/gtest.h>

extern "C" {
#include "navigation/navigation_fixedwing_flight_tally_logic.h"
}

// Same shape as the launch controller's detection (navigation_fw_launch.c):
// every launch path involves acceleration toward the nose (body +X).

static const float G = 981.0f;          // cm/s^2
static const float THRESH = 1900.0f;    // launch_accel_thresh default, cm/s^2
static const float VEL_THRESH = 300.0f; // launch_velocity_thresh default, cm/s

// Measured specific force: forward (x), vertical (z) components.
static bool signature(float accelX, float accelZ, bool almostLevel = true,
                      float swingVelocity = 0, bool gpsHeading = false, float groundSpeed = 0,
                      bool requireAccelExcess = true)
{
    const float normSq = accelX * accelX + accelZ * accelZ;
    return fwFlightTakeoffSignature(accelX, normSq, G, THRESH, almostLevel,
                                    swingVelocity, VEL_THRESH, gpsHeading, groundSpeed,
                                    requireAccelExcess);
}

TEST(FwTakeoffSignature, StationaryLevelIsNotALaunch)
{
    EXPECT_FALSE(signature(0, G));
}

TEST(FwTakeoffSignature, StationaryNoseUpIsNotALaunch)
{
    // Gravity alone puts +X on the forward axis when nose-up (#11911).
    EXPECT_FALSE(signature(G, 0, false));
    EXPECT_FALSE(signature(G * 0.7f, G * 0.7f, false));
}

TEST(FwTakeoffSignature, LevelBungeePushIsALaunch)
{
    EXPECT_TRUE(signature(THRESH + 100, G));
}

TEST(FwTakeoffSignature, VerticalBumpWhileLevelIsNotALaunch)
{
    // Large magnitude excess, but no forward acceleration.
    EXPECT_FALSE(signature(0, G + 3000));
    EXPECT_FALSE(signature(50, G + 3000));
}

TEST(FwTakeoffSignature, BackwardPushIsNotALaunch)
{
    EXPECT_FALSE(signature(-(THRESH + 500), G));
}

TEST(FwTakeoffSignature, BungeeNeedsAlmostLevel)
{
    EXPECT_FALSE(signature(THRESH + 100, G, false));
}

TEST(FwTakeoffSignature, SteepOverheadThrowTowardTheNoseIsALaunch)
{
    // Nose straight up: gravity on +X plus the throw, no level requirement
    // on the swing/forward paths.
    const float x = G + 3000.0f;
    EXPECT_TRUE(signature(x, 0, false, VEL_THRESH + 50));
}

TEST(FwTakeoffSignature, SwingNeedsForwardAcceleration)
{
    // High swing velocity and magnitude excess but acceleration not toward the nose.
    EXPECT_FALSE(signature(0, G + 3000, false, VEL_THRESH + 50));
}

TEST(FwTakeoffSignature, ForwardLaunchNeedsGpsSpeedAndForwardAcceleration)
{
    EXPECT_TRUE(signature(THRESH + 100, G, false, 0, true, VEL_THRESH + 50));
    EXPECT_FALSE(signature(0, G + 3000, false, 0, true, VEL_THRESH + 50));
    EXPECT_FALSE(signature(THRESH + 100, G, false, 0, false, VEL_THRESH + 50));
    EXPECT_FALSE(signature(THRESH + 100, G, false, 0, true, VEL_THRESH));
}

TEST(FwTakeoffSignature, SmallForwardAccelerationBelowThresholdIsNotALaunch)
{
    EXPECT_FALSE(signature(500, G));
}

// The launch controller runs only after the pilot has committed to a launch,
// so it does not require the magnitude excess over g that the always-running
// background detector adds on the swing and forward-GPS paths.

TEST(FwTakeoffSignature, ControllerSwingNeedsOnlyPositiveForwardAcceleration)
{
    EXPECT_TRUE(signature(100, G, false, VEL_THRESH + 50, false, 0, false));
    EXPECT_FALSE(signature(100, G, false, VEL_THRESH + 50, false, 0, true));
}

TEST(FwTakeoffSignature, ControllerForwardLaunchNeedsOnlyPositiveForwardAcceleration)
{
    EXPECT_TRUE(signature(100, G, false, 0, true, VEL_THRESH + 50, false));
    EXPECT_FALSE(signature(100, G, false, 0, true, VEL_THRESH + 50, true));
}

TEST(FwTakeoffSignature, ControllerStillRejectsNoForwardAcceleration)
{
    EXPECT_FALSE(signature(0, G, false, VEL_THRESH + 50, false, 0, false));
    EXPECT_FALSE(signature(-100, G, false, VEL_THRESH + 50, false, 0, false));
    EXPECT_FALSE(signature(0, G, false, 0, true, VEL_THRESH + 50, false));
}
