/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

/*
 * Coverage for https://github.com/iNavFlight/inav/issues/11704
 *
 * "Swap Roll & Yaw" (LOGIC_CONDITION_SWAP_ROLL_YAW, which sets the global
 * flag LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_SWAP_ROLL_YAW) is honoured by
 * the PID path (flight/pid.c calls
 * programming/logic_condition.c:getRcCommandOverride() for both angle and
 * rate targets) and by flight/servos.c's servoMixer(), which routes its
 * FLIGHT_MODE(MANUAL_MODE) stabilized-input reads and its INPUT_RC_ROLL/
 * PITCH/YAW reads through getRcCommandOverride() as well, so tailsitter
 * VTOL servos swap consistently with the motors.
 *
 * This test exercises the REAL, unmodified flight/servos.c servoMixer()
 * function (linked directly into this unit test, not re-implemented) and
 * checks that INPUT_STABILIZED_ROLL / INPUT_STABILIZED_YAW end up swapped
 * once LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_SWAP_ROLL_YAW is set.
 *
 * The "expected" (correct/swapped) values mirror the trivial 2-line swap
 * performed by getRcCommandOverride() in programming/logic_condition.c
 * (lines ~1147-1154):
 *
 *     if (SWAP_ROLL_YAW flag set && axis == FD_ROLL) outputValue = command[FD_YAW];
 *     else if (SWAP_ROLL_YAW flag set && axis == FD_YAW) outputValue = command[FD_ROLL];
 */

#include <cstdint>
#include <cstring>

extern "C" {
    #include "platform.h"

    #include "common/axis.h"
    #include "common/maths.h"

    #include "fc/cli.h"
    #include "fc/config.h"
    #include "fc/fc_core.h"
    #include "fc/rc_controls.h"
    #include "fc/rc_modes.h"
    #include "fc/runtime_config.h"

    #include "build/debug.h"

    #include "drivers/gimbal_common.h"
    #include "drivers/headtracker_common.h"
    #include "drivers/io_port_expander.h"
    #include "drivers/light_ws2811strip.h"
    #include "drivers/vtx_common.h"

    #include "flight/failsafe.h"
    #include "flight/imu.h"
    #include "flight/mixer.h"
    #include "flight/mixer_profile.h"
    #include "flight/pid.h"
    #include "flight/servos.h"
    #include "flight/wind_estimator.h"

    #include "io/gps.h"
    #include "io/osd_common.h"
    #include "io/vtx.h"

    #include "navigation/navigation.h"
    // navigation_private.h is intentionally NOT included: it uses
    // _Static_assert (a C11-only keyword, not valid in this C++ TU) and
    // declares symbols (posControl, navGetCurrentStateFlags(),
    // calculateDistanceToDestination()) that are private to navigation.c /
    // logic_condition.c and never referenced by anything this test's TU
    // declares or calls -- they only need to exist as *linker* symbols to
    // satisfy the rest of logic_condition.c's translation unit, so this
    // file provides simplified stand-in declarations for them below
    // instead. Since C linking matches extern symbols by name only (no
    // cross-TU type checking), this is safe as long as the real
    // implementations in that translation unit are never actually invoked
    // by this test -- and they are not (only getRcCommandOverride() is
    // called).

    #include "programming/global_variables.h"
    #include "programming/logic_condition.h"
    #include "programming/pid.h"

    #include "rx/rx.h"
    #include "rx/msp_override.h"

    #include "sensors/battery.h"
    #include "sensors/diagnostics.h"
    #include "sensors/pitotmeter.h"
    #include "sensors/rangefinder.h"

    // ---- Globals servos.c references but that this test does not link
    // ---- the "real" owning .c file for. These are plain extern data
    // ---- (not accessor functions), so defining them here is equivalent
    // ---- to linking rc_controls.c / runtime_config.c / pid.c / imu.c /
    // ---- mixer_profile.c / mixer.c for just these fields.
    int16_t rcCommand[4];
    uint32_t armingFlags;
    uint32_t flightModeFlags;
    uint32_t stateFlags;
    int16_t axisPID[3];
    attitudeEulerAngles_t attitude;
    mixerConfig_t currentMixerConfig;
    bool isMixerTransitionMixing;
    int mixerThrottleCommand;
    // logicConditionsGlobalFlags is NOT defined here: it is a real global
    // owned by programming/logic_condition.c (which this test now links,
    // to exercise the real getRcCommandOverride()), declared extern in
    // programming/logic_condition.h (already included above).

    // customServoMixers()/customServoMixersMutable() (used by
    // loadCustomServoMixer() in servos.c) resolve, via macros in
    // flight/mixer_profile.h, through mixerProfiles(systemConfig()->
    // current_mixer_profile_index)->ServoMixers. Provide the backing
    // storage for those two PG-registered globals instead of linking
    // fc/config.c and flight/mixer_profile.c.
    systemConfig_t systemConfig_System;
    mixerProfile_t mixerProfiles_SystemArray[MAX_MIXER_PROFILE_COUNT];

    // ---- Minimal stand-ins for FC subsystems this test does not link.
    // servoMixer() calls these but none of the branches guarded by them
    // are relevant to the roll/yaw-swap bug being reproduced.
    bool feature(uint32_t mask)
    {
        (void)mask;
        return false;
    }

    bool IS_RC_MODE_ACTIVE(boxId_e boxId)
    {
        (void)boxId;
        return false;
    }

    // Per-channel raw RX values, settable by tests that need to distinguish
    // e.g. ROLL from YAW (path #3 / INPUT_RC_ROLL,YAW reproduction below).
    // Defaults to PWM_RANGE_MIDDLE (matches old fixed-return stub) for any
    // channel a test does not explicitly set.
    int16_t g_rxChannelValues[MAX_SUPPORTED_RC_CHANNEL_COUNT];

    void resetRxChannelValues(void)
    {
        for (unsigned i = 0; i < MAX_SUPPORTED_RC_CHANNEL_COUNT; i++) {
            g_rxChannelValues[i] = PWM_RANGE_MIDDLE;
        }
    }

    int16_t rxGetChannelValue(unsigned channelNumber)
    {
        if (channelNumber < MAX_SUPPORTED_RC_CHANNEL_COUNT) {
            return g_rxChannelValues[channelNumber];
        }
        return PWM_RANGE_MIDDLE;
    }

    uint32_t getLooptime(void)
    {
        return 4000; // 250Hz, arbitrary - servo_lowpass_freq is left at 0 so this is unused
    }

    // The following are only reached from processServoAutotrimMode() /
    // processContinuousServoAutotrim(), neither of which this test calls
    // (the bug under test is entirely within servoMixer()). Stubbed only
    // to satisfy the linker for the rest of servos.c's object file.
    fpVector3_t imuMeasuredRotationBF;
    int32_t debug[DEBUG32_VALUE_COUNT];
    uint8_t debugMode;
    int32_t axisPID_I[3];

    bool isFwAutoModeActive(boxId_e mode) { (void)mode; return false; }
    timeMs_t millis(void) { return 0; }
    void pidResetErrorAccumulators(void) {}
    void pidReduceErrorAccumulators(int8_t delta, uint8_t axis) { (void)delta; (void)axis; }
    float getAxisIterm(uint8_t axis) { (void)axis; return 0.0f; }
    float getTotalRateTarget(void) { return 0.0f; }
    float getFixedWingLevelTrim(void) { return 0.0f; }
    bool areSticksDeflected(void) { return false; }
    void saveConfigAndNotify(void) {}
    bool isGPSHeadingValid(void) { return false; }

    // ---- Additional stand-ins required to link programming/logic_condition.c
    // (needed for the REAL getRcCommandOverride(), which lives in that file).
    // None of these are reachable from getRcCommandOverride() itself -- it
    // only touches the LOGIC_CONDITION_GLOBAL_FLAG() macro / logicConditionsGlobalFlags
    // (already stubbed above) -- they are only needed to satisfy the linker
    // for the rest of logic_condition.c's translation unit.
    pidProfile_t pidProfile_Instance;
    pidProfile_t *pidProfile_ProfileCurrent = &pidProfile_Instance;
    navConfig_t navConfig_System;
    gpsSolutionData_t gpsSol;
    navSystemStatus_t NAV_Status;
    int currentMixerProfileIndex;
    bool cliMode;

    // GPS_distanceToHome is plain extern data (declared in navigation.h,
    // which this file includes), not an accessor function -- type must
    // match navigation.h's "extern uint32_t GPS_distanceToHome;" exactly.
    uint32_t GPS_distanceToHome;

    // posControl (navigationPosControl_t, from the deliberately-not-included
    // navigation_private.h -- see comment above) is opaque storage here:
    // large enough that it's implausible any accidental single-word write
    // from elsewhere in the real logic_condition.c object (code that is
    // linked in but never invoked by this test) could run past the end,
    // and never accessed as a real struct from this TU.
    uint8_t posControl[4096];

    // navigationFSMStateFlags_t / calculateDistanceToDestination's
    // parameter type are also only declared in navigation_private.h; give
    // simplified stand-in types/signatures here purely so the linker has a
    // symbol to resolve against (see comment above: safe because this
    // test's TU declares -- and calls -- nothing else that depends on
    // their real definitions).
    typedef uint32_t stubNavigationFSMStateFlags_t;

    void gvSet(uint8_t index, int32_t value) { (void)index; (void)value; }
    int32_t gvGet(uint8_t index) { (void)index; return 0; }
    void updateHeadingHoldTarget(int16_t heading) { (void)heading; }
    uint8_t getConfigProfile(void) { return 0; }
    bool setConfigProfile(uint8_t profileIndex) { (void)profileIndex; return false; }
    void pidInit(void) {}
    bool pidInitFilters(void) { return false; }
    void schedulePidGainsUpdate(void) {}
    void navigationUsePIDs(void) {}
    void ledPinStartPWM(uint16_t value) { (void)value; }
    void ledPinStopPWM(void) {}
    stubNavigationFSMStateFlags_t navGetCurrentStateFlags(void) { return 0; }
    bool geoConvertGeodeticToLocal(fpVector3_t *pos, const gpsOrigin_t *origin, const gpsLocation_t *llh, geoAltitudeConversionMode_e altConv)
    {
        (void)pos; (void)origin; (void)llh; (void)altConv;
        return false;
    }
    uint32_t calculateDistanceToDestination(const fpVector3_t *destinationPos) { (void)destinationPos; return 0; }
    float getFlightTime(void) { return 0.0f; }
    uint32_t getTotalTravelDistance(void) { return 0; }
    uint16_t getRSSI(void) { return 0; }
    uint16_t getBatteryVoltage(void) { return 0; }
    uint16_t getBatteryAverageCellVoltage(void) { return 0; }
    uint8_t getBatteryCellCount(void) { return 0; }
    int16_t getAmperage(void) { return 0; }
    int32_t getMAhDrawn(void) { return 0; }
    hardwareSensorStatus_e getHwGPSStatus(void) { return HW_SENSOR_NONE; }
    int16_t osdGet3DSpeed(void) { return 0; }
    float getEstimatedActualPosition(int axis) { (void)axis; return 0.0f; }
    float getEstimatedActualVelocity(int axis) { (void)axis; return 0.0f; }
    uint8_t getConfigBatteryProfile(void) { return 0; }
    uint16_t getFlownLoiterRadius(void) { return 0; }
    bool isEstimatedAglTrusted(void) { return false; }
    float getEstimatedAglPosition(void) { return 0.0f; }
    int32_t rangefinderGetLatestRawAltitude(void) { return 0; }
    int32_t programmingPidGetOutput(uint8_t i) { (void)i; return 0; }
    failsafePhase_e failsafePhase(void) { return FAILSAFE_IDLE; }

    // ---- Additional stand-ins required after rebasing this branch onto a
    // newer upstream/maintenance-10.x: servos.c and logic_condition.c now
    // reference these symbols from unrelated code paths (VTX control,
    // headtracker, simulator/HITL passthrough, I/O port expanders, gimbal
    // sensitivity, RTH/forced-landing activation, waypoint mission queries,
    // airspeed/wind estimation, RC link statistics, and MSP RC override).
    // None of them are reachable from servoMixer()'s MANUAL_MODE/RC-input
    // branches or from getRcCommandOverride() -- they only need to exist
    // as inert linker symbols to satisfy the rest of those two
    // translation units.
    int16_t mixerATGetTransitionServoInput(void) { return 0; }

    headTrackerDevice_t *headTrackerCommonDevice(void) { return NULL; }
    bool headTrackerCommonIsValid(const headTrackerDevice_t *headtrackerDevice) { (void)headtrackerDevice; return false; }
    int headTrackerCommonGetPanPWM(const headTrackerDevice_t *headTrackerDevice) { (void)headTrackerDevice; return PWM_RANGE_MIDDLE; }
    int headTrackerCommonGetTiltPWM(const headTrackerDevice_t *headTrackerDevice) { (void)headTrackerDevice; return PWM_RANGE_MIDDLE; }
    int headTrackerCommonGetRollPWM(const headTrackerDevice_t *headTrackerDevice) { (void)headTrackerDevice; return PWM_RANGE_MIDDLE; }

    // simulatorData is plain extern data (simulatorData_t, declared in
    // fc/runtime_config.h), not an accessor function.
    simulatorData_t simulatorData;

    // vtxSettingsConfig is a PG_REGISTER'd config; PG_DECLARE's accessors
    // (vtxSettingsConfig()/vtxSettingsConfig_Mutable()) resolve to this
    // backing storage, matching the systemConfig_System pattern above.
    vtxSettingsConfig_t vtxSettingsConfig_System;

    vtxDevice_t *vtxCommonDevice(void) { return NULL; }
    bool vtxCommonGetDeviceCapability(vtxDevice_t *vtxDevice, vtxDeviceCapability_t *pDeviceCapability)
    {
        (void)vtxDevice; (void)pDeviceCapability;
        return false;
    }
    void vtxCommonSetPowerByIndex(vtxDevice_t *vtxDevice, uint8_t index) { (void)vtxDevice; (void)index; }
    void vtxCommonSetBandAndChannel(vtxDevice_t *vtxDevice, uint8_t band, uint8_t channel) { (void)vtxDevice; (void)band; (void)channel; }

    void setGimbalSensitivity(int16_t sensitivity) { (void)sensitivity; }
    void ioPortExpanderSet(uint8_t pin, uint8_t value) { (void)pin; (void)value; }
    void ioPortExpanderSync(void) {}

    bool navigationSetAltitudeTargetWithDatum(geoAltitudeDatumFlag_e datumFlag, int32_t targetAltitudeCm)
    {
        (void)datumFlag; (void)targetAltitudeCm;
        return false;
    }
    bool activateRTHMode(void) { return false; }
    bool activateForcedLanding(void) { return false; }
    bool navGetMissionWaypointByRelativeIndex(int16_t relativeIndex, navWaypoint_t *wpData) { (void)relativeIndex; (void)wpData; return false; }
    geoAltitudeConversionMode_e waypointMissionAltConvMode(geoAltitudeDatumFlag_e datumFlag) { (void)datumFlag; return GEO_ALT_ABSOLUTE; }
    bool navigationIsExecutingAnEmergencyLanding(void) { return false; }

    float getAirspeedEstimate(void) { return 0.0f; }
    bool isEstimatedWindSpeedValid(void) { return false; }
    float getEstimatedHorizontalWindSpeed(uint16_t *angle) { (void)angle; return 0.0f; }

    rxLinkStatistics_t rxLinkStatistics;

    bool mspOverrideFlightAxisAngleActive(uint8_t axis, int *target) { (void)axis; (void)target; return false; }
    bool mspOverrideFlightAxisRateActive(uint8_t axis, int *target) { (void)axis; (void)target; return false; }
}

#include "unittest_macros.h"
#include "gtest/gtest.h"

namespace {

// Servo output channels used by this test's mixer table.
constexpr int SERVO_ROLL_OUT = 0;
constexpr int SERVO_YAW_OUT = 1;

class SwapRollYawServoTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        memset(&systemConfig_System, 0, sizeof(systemConfig_System));
        systemConfig_System.current_mixer_profile_index = 0;
        memset(mixerProfiles_SystemArray, 0, sizeof(mixerProfiles_SystemArray));

        memset(rcCommand, 0, sizeof(rcCommand));
        armingFlags = 0;
        flightModeFlags = 0;
        stateFlags = 0;
        memset(axisPID, 0, sizeof(axisPID));
        memset(&attitude, 0, sizeof(attitude));
        memset(&currentMixerConfig, 0, sizeof(currentMixerConfig));
        isMixerTransitionMixing = false;
        mixerThrottleCommand = 1500;
        logicConditionsGlobalFlags = 0;
        resetRxChannelValues();

        // MANUAL_MODE is the flight mode in which servoMixer() reads
        // rcCommand[] directly instead of the PID-stabilized axisPID[].
        // This is exactly the code path affected by the bug.
        flightModeFlags = MANUAL_MODE;

        // Simple 1:1 passthrough servo mixer table:
        //   servo[SERVO_ROLL_OUT] <- INPUT_STABILIZED_ROLL
        //   servo[SERVO_YAW_OUT]  <- INPUT_STABILIZED_YAW
        memset(customServoMixersMutable(0), 0, sizeof(servoMixer_t) * MAX_SERVO_RULES);
        customServoMixersMutable(0)->targetChannel = SERVO_ROLL_OUT;
        customServoMixersMutable(0)->inputSource = INPUT_STABILIZED_ROLL;
        customServoMixersMutable(0)->rate = 100;
        customServoMixersMutable(0)->speed = 0;
        // conditionId defaults to 0 (an all-zero, i.e. falsy, real logic
        // condition slot) when memset above zeroes the rule; production
        // code (servos.c's default servoMixer_t initializer) uses -1
        // ("no condition attached" -> logicConditionGetValue() treats any
        // negative id as always-true) for unconditional rules. Set that
        // explicitly so this rule is not gated off by
        // logicConditionGetValue(0) returning false.
        customServoMixersMutable(0)->conditionId = -1;

        customServoMixersMutable(1)->targetChannel = SERVO_YAW_OUT;
        customServoMixersMutable(1)->inputSource = INPUT_STABILIZED_YAW;
        customServoMixersMutable(1)->rate = 100;
        customServoMixersMutable(1)->speed = 0;
        customServoMixersMutable(1)->conditionId = -1;

        loadCustomServoMixer();

        // Neutral, symmetric servo endpoints so servo[i] == input + 1500
        // (scaleMax == scaleMin == 1.0, rate == 100%).
        for (int i = 0; i < MAX_SUPPORTED_SERVOS; i++) {
            servoParamsMutable(i)->min = 1000;
            servoParamsMutable(i)->max = 2000;
            servoParamsMutable(i)->middle = 1500;
            servoParamsMutable(i)->rate = 100;
            servoComputeScalingFactors(i);
        }
    }
};

// getRcCommandOverride()'s swap (programming/logic_condition.c ~line 1147)
// mirrored here only to compute the *expected* value for the assertions.
// The swap itself is a trivial, stable 2-line operation (see block comment
// above) so hand-mirroring it carries negligible drift risk, unlike more
// complex algorithms.
int16_t expectedOverriddenValue(int16_t rcCommandRoll, int16_t rcCommandYaw, uint8_t axis)
{
    if (axis == FD_ROLL) {
        return rcCommandYaw;
    }
    if (axis == FD_YAW) {
        return rcCommandRoll;
    }
    return 0;
}

TEST_F(SwapRollYawServoTest, ServoOutputsAreSwappedWhenSwapFlagSet)
{
    const int16_t rollStick = 350;
    const int16_t yawStick = -220;

    rcCommand[ROLL] = rollStick;
    rcCommand[YAW] = yawStick;

    LOGIC_CONDITION_GLOBAL_FLAG_ENABLE(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_SWAP_ROLL_YAW);

    servoMixer(0.001f);

    const int16_t expectedRollServoInput = expectedOverriddenValue(rollStick, yawStick, FD_ROLL); // == yawStick
    const int16_t expectedYawServoInput = expectedOverriddenValue(rollStick, yawStick, FD_YAW);   // == rollStick

    const int16_t expectedRollServo = 1500 + expectedRollServoInput;
    const int16_t expectedYawServo = 1500 + expectedYawServoInput;

    // This is what the fix should produce: servo roll output reflects the
    // (swapped-in) yaw stick, and servo yaw output reflects the
    // (swapped-in) roll stick -- matching what getRcCommandOverride()
    // already does for the PID/motor path.
    EXPECT_EQ(expectedRollServo, servo[SERVO_ROLL_OUT])
        << "servo roll output should reflect swapped (yaw) stick input when "
           "LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_SWAP_ROLL_YAW is set";
    EXPECT_EQ(expectedYawServo, servo[SERVO_YAW_OUT])
        << "servo yaw output should reflect swapped (roll) stick input when "
           "LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_SWAP_ROLL_YAW is set";
}

// Negative/control case: with the flag OFF, servo outputs must NOT be
// swapped -- this should pass both before and after the fix, guarding
// against an over-eager fix that swaps unconditionally.
TEST_F(SwapRollYawServoTest, ServoOutputsAreNotSwappedWhenFlagClear)
{
    const int16_t rollStick = 350;
    const int16_t yawStick = -220;

    rcCommand[ROLL] = rollStick;
    rcCommand[YAW] = yawStick;

    // Flag intentionally left clear (LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_SWAP_ROLL_YAW not set).

    servoMixer(0.001f);

    const int16_t expectedRollServo = 1500 + rollStick;
    const int16_t expectedYawServo = 1500 + yawStick;

    EXPECT_EQ(expectedRollServo, servo[SERVO_ROLL_OUT]);
    EXPECT_EQ(expectedYawServo, servo[SERVO_YAW_OUT]);
}

// getRcCommandOverride() also applies the per-axis "invert" override flags
// (LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_INVERT_ROLL/PITCH/YAW,
// programming/logic_condition.c ~line 1213), which negate outputValue for
// the matching axis *after* any swap has already been applied. This test
// exercises the invert flags in isolation (SWAP_ROLL_YAW left clear) so the
// expected value is simply the negated stick input, with no interaction
// between swap and invert to reason about. This is new coverage: before
// the roll/yaw-swap fix routed the stabilized-input path through
// getRcCommandOverride(), these invert flags had zero effect here (the
// code read rcCommand[] directly), so this proves that side effect of the
// fix reaches the servo output as well.
TEST_F(SwapRollYawServoTest, ServoOutputsAreInvertedWhenInvertFlagsSetWithoutSwap)
{
    const int16_t rollStick = 350;
    const int16_t yawStick = -220;

    rcCommand[ROLL] = rollStick;
    rcCommand[YAW] = yawStick;

    // SWAP_ROLL_YAW intentionally left clear so outputValue == command[axis]
    // going into the invert step, keeping the expected value a plain negation.
    LOGIC_CONDITION_GLOBAL_FLAG_ENABLE(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_INVERT_ROLL);
    LOGIC_CONDITION_GLOBAL_FLAG_ENABLE(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_INVERT_YAW);

    servoMixer(0.001f);

    const int16_t expectedRollServo = 1500 - rollStick;
    const int16_t expectedYawServo = 1500 - yawStick;

    EXPECT_EQ(expectedRollServo, servo[SERVO_ROLL_OUT])
        << "servo roll output should reflect the inverted roll stick when "
           "LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_INVERT_ROLL is set";
    EXPECT_EQ(expectedYawServo, servo[SERVO_YAW_OUT])
        << "servo yaw output should reflect the inverted yaw stick when "
           "LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_INVERT_YAW is set";
}

// ---------------------------------------------------------------------
// Coverage for "RC Roll"/"RC Yaw" mixer inputs (INPUT_RC_ROLL /
// INPUT_RC_YAW), a THIRD, separate servo-mixer input source from
// "Stabilized Roll/Yaw" above. servoMixer() computes these unconditionally
// on every call (not gated by FLIGHT_MODE(MANUAL_MODE)) from the raw RX
// channel value via rxGetChannelValue()/GET_RX_CHANNEL_INPUT(), and routes
// them through getRcCommandOverride() the same way the stabilized-input
// path does, so LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_SWAP_ROLL_YAW applies
// to servos mixed from INPUT_RC_ROLL/INPUT_RC_YAW regardless of flight mode.
// ---------------------------------------------------------------------

class SwapRollYawRcInputServoTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        memset(&systemConfig_System, 0, sizeof(systemConfig_System));
        systemConfig_System.current_mixer_profile_index = 0;
        memset(mixerProfiles_SystemArray, 0, sizeof(mixerProfiles_SystemArray));

        memset(rcCommand, 0, sizeof(rcCommand));
        armingFlags = 0;
        flightModeFlags = 0;
        stateFlags = 0;
        memset(axisPID, 0, sizeof(axisPID));
        memset(&attitude, 0, sizeof(attitude));
        memset(&currentMixerConfig, 0, sizeof(currentMixerConfig));
        isMixerTransitionMixing = false;
        mixerThrottleCommand = 1500;
        logicConditionsGlobalFlags = 0;
        resetRxChannelValues();

        // Deliberately NOT MANUAL_MODE: INPUT_RC_ROLL/INPUT_RC_YAW are
        // computed unconditionally regardless of flight mode, so this
        // test uses a plain stabilized/assisted flight mode to make clear
        // this bug is independent of the MANUAL_MODE path fixed above.
        flightModeFlags = 0;

        // Servo mixer table using "RC Roll"/"RC Yaw" inputs instead of
        // "Stabilized Roll"/"Stabilized Yaw":
        //   servo[SERVO_ROLL_OUT] <- INPUT_RC_ROLL
        //   servo[SERVO_YAW_OUT]  <- INPUT_RC_YAW
        memset(customServoMixersMutable(0), 0, sizeof(servoMixer_t) * MAX_SERVO_RULES);
        customServoMixersMutable(0)->targetChannel = SERVO_ROLL_OUT;
        customServoMixersMutable(0)->inputSource = INPUT_RC_ROLL;
        customServoMixersMutable(0)->rate = 100;
        customServoMixersMutable(0)->speed = 0;
        // See conditionId comment in SwapRollYawServoTest::SetUp() above.
        customServoMixersMutable(0)->conditionId = -1;

        customServoMixersMutable(1)->targetChannel = SERVO_YAW_OUT;
        customServoMixersMutable(1)->inputSource = INPUT_RC_YAW;
        customServoMixersMutable(1)->rate = 100;
        customServoMixersMutable(1)->speed = 0;
        customServoMixersMutable(1)->conditionId = -1;

        loadCustomServoMixer();

        // Neutral, symmetric servo endpoints so servo[i] == input + 1500
        // (scaleMax == scaleMin == 1.0, rate == 100%).
        for (int i = 0; i < MAX_SUPPORTED_SERVOS; i++) {
            servoParamsMutable(i)->min = 1000;
            servoParamsMutable(i)->max = 2000;
            servoParamsMutable(i)->middle = 1500;
            servoParamsMutable(i)->rate = 100;
            servoComputeScalingFactors(i);
        }
    }
};

TEST_F(SwapRollYawRcInputServoTest, RcInputServoOutputsAreSwappedWhenSwapFlagSet)
{
    // Distinguishable raw RX stick values (pre-center-subtraction, i.e. the
    // values rxGetChannelValue() itself returns) for ROLL and YAW.
    const int16_t rollRxValue = PWM_RANGE_MIDDLE + 350;
    const int16_t yawRxValue = PWM_RANGE_MIDDLE - 220;

    g_rxChannelValues[ROLL] = rollRxValue;
    g_rxChannelValues[YAW] = yawRxValue;

    LOGIC_CONDITION_GLOBAL_FLAG_ENABLE(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_SWAP_ROLL_YAW);

    servoMixer(0.001f);

    // What the fix should produce: with roll/yaw swapped, the ROLL servo
    // output should reflect the (swapped-in) raw YAW stick, and the YAW
    // servo output should reflect the (swapped-in) raw ROLL stick --
    // mirroring exactly what getRcCommandOverride() already does for the
    // INPUT_STABILIZED_ROLL/YAW path.
    const int16_t expectedRollServo = yawRxValue;
    const int16_t expectedYawServo = rollRxValue;

    EXPECT_EQ(expectedRollServo, servo[SERVO_ROLL_OUT])
        << "servoMixer() should route INPUT_RC_ROLL through "
           "getRcCommandOverride() so LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_SWAP_ROLL_YAW "
           "swaps RC-Roll-driven servos too.";
    EXPECT_EQ(expectedYawServo, servo[SERVO_YAW_OUT])
        << "servoMixer() should route INPUT_RC_YAW through "
           "getRcCommandOverride() so LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_SWAP_ROLL_YAW "
           "swaps RC-Yaw-driven servos too.";
}

// Negative/control case: with the flag OFF, RC-Roll/RC-Yaw servo outputs
// must NOT be swapped -- mirrors SwapRollYawServoTest.ServoOutputsAreNotSwappedWhenFlagClear
// above, but for the INPUT_RC_ROLL/INPUT_RC_YAW path. This should pass both
// before and after the fix, guarding against an over-eager fix that swaps
// unconditionally.
TEST_F(SwapRollYawRcInputServoTest, RcInputServoOutputsAreNotSwappedWhenFlagClear)
{
    const int16_t rollRxValue = PWM_RANGE_MIDDLE + 350;
    const int16_t yawRxValue = PWM_RANGE_MIDDLE - 220;

    g_rxChannelValues[ROLL] = rollRxValue;
    g_rxChannelValues[YAW] = yawRxValue;

    // Flag intentionally left clear (LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_SWAP_ROLL_YAW not set).

    servoMixer(0.001f);

    const int16_t expectedRollServo = rollRxValue;
    const int16_t expectedYawServo = yawRxValue;

    EXPECT_EQ(expectedRollServo, servo[SERVO_ROLL_OUT]);
    EXPECT_EQ(expectedYawServo, servo[SERVO_YAW_OUT]);
}

// getRcCommandOverride() also applies the per-axis "invert" override flags
// (LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_INVERT_ROLL/PITCH/YAW,
// programming/logic_condition.c ~line 1213) to the RC-input path as well,
// negating outputValue for the matching axis *after* any swap has already
// been applied. This test exercises the invert flags in isolation
// (SWAP_ROLL_YAW left clear) so the expected value is simply the negated
// raw RX stick input relative to center, with no interaction between swap
// and invert to reason about. This is new coverage: before the roll/yaw-swap
// fix routed the RC-input path through getRcCommandOverride(), these invert
// flags had zero effect here (the code read the raw RX value directly), so
// this proves that side effect of the fix reaches the servo output as well.
TEST_F(SwapRollYawRcInputServoTest, RcInputServoOutputsAreInvertedWhenInvertFlagsSetWithoutSwap)
{
    const int16_t rollRxValue = PWM_RANGE_MIDDLE + 350;
    const int16_t yawRxValue = PWM_RANGE_MIDDLE - 220;

    g_rxChannelValues[ROLL] = rollRxValue;
    g_rxChannelValues[YAW] = yawRxValue;

    // SWAP_ROLL_YAW intentionally left clear so outputValue == command[axis]
    // going into the invert step, keeping the expected value a plain negation
    // of the RC input relative to center.
    LOGIC_CONDITION_GLOBAL_FLAG_ENABLE(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_INVERT_ROLL);
    LOGIC_CONDITION_GLOBAL_FLAG_ENABLE(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_INVERT_YAW);

    servoMixer(0.001f);

    // getRcCommandOverride() negates the *centered* RC command
    // (rcCommand[axis] == rxValue - PWM_RANGE_MIDDLE at the point it is
    // passed in via GET_RX_CHANNEL_INPUT()), so the servo output mirrors
    // the stick about center rather than about zero.
    const int16_t expectedRollServo = PWM_RANGE_MIDDLE - (rollRxValue - PWM_RANGE_MIDDLE);
    const int16_t expectedYawServo = PWM_RANGE_MIDDLE - (yawRxValue - PWM_RANGE_MIDDLE);

    EXPECT_EQ(expectedRollServo, servo[SERVO_ROLL_OUT])
        << "servoMixer() should route INPUT_RC_ROLL through "
           "getRcCommandOverride() so LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_INVERT_ROLL "
           "inverts RC-Roll-driven servos too.";
    EXPECT_EQ(expectedYawServo, servo[SERVO_YAW_OUT])
        << "servoMixer() should route INPUT_RC_YAW through "
           "getRcCommandOverride() so LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_INVERT_YAW "
           "inverts RC-Yaw-driven servos too.";
}

} // namespace
