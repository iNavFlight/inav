/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Verifies writeMotors() (src/main/flight/mixer.c) DShot output for
 * 3D/reversible motors, both ARMED and DISARMED (motor-testing via
 * MSP_SET_MOTOR). While disarmed, direction can't come from
 * reversibleMotorsThrottleState/throttleRangeMin/Max - mixTable() is the only
 * thing that updates those, and it never runs while disarmed - so the
 * disarmed case is a separate, always-compiled helper,
 * calculateDisarmedReversibleMotorsDshotValue().
 *
 * Why this file hand-reproduces mixer.c's logic instead of linking it:
 * writeMotors() and its helpers are `static`, and writeMotors() itself is
 * `#if !defined(SITL_BUILD)`-gated; every test here is built with SITL_BUILD
 * defined, so the real functions can't be linked in (and linking mixer.c
 * wholesale would require stubbing motorConfig(), feature(), ARMING_FLAG(),
 * etc.). Functions are hand-reproduced verbatim (`_CurrentSource` suffix);
 * the "SourceSync" suite at the bottom reads the live mixer.c at test time
 * and fails if it drifts from these reproductions.
 */

#include <stdint.h>
#include <stdbool.h>

#include <cctype>
#include <fstream>
#include <sstream>
#include <string>

#include "gtest/gtest.h"
#include "unittest_macros.h"

/*
 * Verbatim reproductions of src/main/common/maths.c's scaleRangef()/
 * constrain() (not SITL_BUILD-gated - only kept as local copies here so this
 * whole file stays self-contained with no CMakeLists.txt `depends` entry).
 * Checked against the live source by SourceSync test
 * MathsHelpersMatchLiveSource below.
 */
static float scaleRangef(float x, float srcMin, float srcMax, float destMin, float destMax)
{
    float a = (destMax - destMin) * (x - srcMin);
    float b = srcMax - srcMin;
    return ((a / b) + destMin);
}

static int32_t constrain(int32_t amt, int32_t low, int32_t high)
{
    if (amt < low)
        return low;
    else if (amt > high)
        return high;
    else
        return amt;
}

/* -------------------------------------------------------------------------
 * Mirrors of src/main/flight/mixer.h constants/enum used by the reproduction
 * below. Checked against the live header by SourceSync test
 * DshotConstantsMatchLiveMixerHeader.
 * ------------------------------------------------------------------------- */
#define DSHOT_DISARM_COMMAND      0
#define DSHOT_MIN_THROTTLE       48
#define DSHOT_MAX_THROTTLE     2047
#define DSHOT_3D_DEADBAND_LOW  1047
#define DSHOT_3D_DEADBAND_HIGH 1048

typedef enum {
    MOTOR_DIRECTION_FORWARD,
    MOTOR_DIRECTION_BACKWARD,
    MOTOR_DIRECTION_DEADBAND
} reversibleMotorsThrottleState_e;

/* -------------------------------------------------------------------------
 * Verbatim reproduction of handleOutputScaling() from mixer.c (as of this
 * writing, roughly lines 268-291). See SourceSync test
 * HandleOutputScalingMatchesLiveSource below.
 * ------------------------------------------------------------------------- */
static uint16_t handleOutputScaling_CurrentSource(
    int16_t input,          // Input value from the mixer
    int16_t stopThreshold,  // Threshold value to check if motor should be rotating or not
    int16_t onStopValue,    // Value sent when min rotation is required
    int16_t inputScaleMin,  // Input range - min value
    int16_t inputScaleMax,  // Input range - max value
    int16_t outputScaleMin, // Output range - min value
    int16_t outputScaleMax, // Output range - max value
    bool moveForward        // If motor should be rotating FORWARD or BACKWARD
)
{
    int16_t value;
    if ((moveForward && input < stopThreshold) || (!moveForward && input > stopThreshold)) {
        value = onStopValue;
    }
    else {
        value = scaleRangef(input, inputScaleMin, inputScaleMax, outputScaleMin, outputScaleMax);
        value = constrain(value, outputScaleMin, outputScaleMax);
    }

    return value;
}

/*
 * Verbatim reproduction of calculateDisarmedReversibleMotorsDshotValue() from
 * mixer.c (as of this writing, immediately before writeMotors()). This is the
 * function that infers forward/reverse/deadband direction for a disarmed
 * reversible motor directly from motorValue against the ESC's configured 3D
 * deadband (since mixTable() never runs while disarmed, so
 * reversibleMotorsThrottleState/throttleRangeMin/Max are stale). See
 * SourceSync test CalculateDisarmedReversibleMotorsDshotValueMatchesLiveSource
 * below.
 *
 * Boundary values (motorValue == deadbandHigh or == deadbandLow) belong to
 * the thrust side (inclusive `>=`/`<=`), matching mixTable()'s direction
 * switch and handleOutputScaling()'s stop check. The two degenerate-config
 * guards (`maxThrottle <= deadbandHigh`, `deadbandLow <= mincommand`) prevent
 * a zero/negative-width scaleRangef() denominator - both are reachable via
 * valid-looking CLI configuration (see the two
 * *DegenerateConfig*ReturnsClamped* tests below).
 */
static uint16_t calculateDisarmedReversibleMotorsDshotValue_CurrentSource(
    int16_t motorValue,
    int16_t deadbandLow,
    int16_t deadbandHigh,
    int16_t mincommand,
    int16_t maxThrottle)
{
    if (motorValue >= deadbandHigh) {
        if (maxThrottle <= deadbandHigh) {
            return DSHOT_MAX_THROTTLE;
        }
        int32_t scaled = (int32_t)scaleRangef(motorValue, deadbandHigh, maxThrottle, DSHOT_3D_DEADBAND_HIGH, DSHOT_MAX_THROTTLE);
        return constrain(scaled, DSHOT_3D_DEADBAND_HIGH, DSHOT_MAX_THROTTLE);
    }
    if (motorValue <= deadbandLow) {
        if (deadbandLow <= mincommand) {
            return DSHOT_MIN_THROTTLE;
        }
        int32_t scaled = (int32_t)scaleRangef(motorValue, mincommand, deadbandLow, DSHOT_MIN_THROTTLE, DSHOT_3D_DEADBAND_LOW);
        return constrain(scaled, DSHOT_MIN_THROTTLE, DSHOT_3D_DEADBAND_LOW);
    }
    return DSHOT_DISARM_COMMAND;
}

/*
 * Verbatim reproduction of the FEATURE_REVERSIBLE_MOTORS + isMotorProtocolDigital
 * branch inside writeMotors() (mixer.c, roughly lines 397-437), AS IT EXISTS
 * TODAY. Takes an explicit `isArmed` parameter mirroring the live
 * `!ARMING_FLAG(ARMED)` check, plus the ESC 3D deadband/mincommand/maxThrottle
 * values needed by the disarmed branch (forwarded straight into
 * calculateDisarmedReversibleMotorsDshotValue_CurrentSource(), exactly as the
 * live writeMotors() forwards them into the real function).
 * `reversibleMotorsThrottleState` / `throttleRangeMin` / `throttleRangeMax`
 * are still taken as plain parameters (mixer.c holds them as file-scope
 * statics) - they are only consulted, and only meaningful, when `isArmed` is
 * true, exactly as in the live code.
 */
static uint16_t writeMotorsReversibleDshot_CurrentSource(
    int16_t motorValue,        // motor[i]
    bool isArmed,
    reversibleMotorsThrottleState_e reversibleMotorsThrottleState,
    int16_t throttleRangeMin,
    int16_t throttleRangeMax,
    int16_t deadbandLow,       // reversibleMotorsConfig()->deadband_low
    int16_t deadbandHigh,      // reversibleMotorsConfig()->deadband_high
    int16_t mincommand,        // motorConfig()->mincommand
    int16_t maxThrottle)       // getMaxThrottle()
{
    uint16_t motorOutput;

    if (!isArmed) {
        // Disarmed (motor test via MSP_SET_MOTOR): mixTable() never runs
        // while disarmed, so reversibleMotorsThrottleState and
        // throttleRangeMin/Max are stale. Infer direction directly from
        // motor[i] against the ESC's real 3D deadband instead.
        motorOutput = calculateDisarmedReversibleMotorsDshotValue_CurrentSource(
            motorValue, deadbandLow, deadbandHigh, mincommand, maxThrottle);
    } else if (reversibleMotorsThrottleState == MOTOR_DIRECTION_FORWARD) {
        motorOutput = handleOutputScaling_CurrentSource(
            motorValue,
            throttleRangeMin,
            DSHOT_DISARM_COMMAND,
            throttleRangeMin,
            throttleRangeMax,
            DSHOT_3D_DEADBAND_HIGH,
            DSHOT_MAX_THROTTLE,
            true
        );
    } else {
        motorOutput = handleOutputScaling_CurrentSource(
            motorValue,
            throttleRangeMax,
            DSHOT_DISARM_COMMAND,
            throttleRangeMin,
            throttleRangeMax,
            DSHOT_MIN_THROTTLE,
            DSHOT_3D_DEADBAND_LOW,
            false
        );
    }

    return motorOutput;
}

/*
 * Verbatim reproduction of the non-reversible-motors DShot branch inside
 * writeMotors() (mixer.c, roughly lines 439-449). This path does not depend
 * on reversibleMotorsThrottleState at all, and was not touched by the fix -
 * used here purely as a regression guard (item 5 in the task spec).
 */
static uint16_t writeMotorsNonReversibleDshot_CurrentSource(
    int16_t motorValue,
    int16_t throttleIdleValue,
    int16_t mincommand,
    int16_t maxThrottle)
{
    return handleOutputScaling_CurrentSource(
        motorValue,
        throttleIdleValue,
        DSHOT_DISARM_COMMAND,
        mincommand,
        maxThrottle,
        DSHOT_MIN_THROTTLE,
        DSHOT_MAX_THROTTLE,
        true
    );
}

/* -------------------------------------------------------------------------
 * "Should be" expected values - computed independently of the reproduction
 * above (never calls handleOutputScaling_CurrentSource or
 * calculateDisarmedReversibleMotorsDshotValue_CurrentSource directly), using
 * the same scaleRangef()/constrain() helpers so this test never embeds a
 * *second*, possibly-diverging copy of the interpolation math. This encodes
 * exactly the mapping the fix implements, for the ordinary (non-degenerate-
 * config) case:
 *   - forward: motor at or above deadband_high scales into
 *              [DSHOT_3D_DEADBAND_HIGH, DSHOT_MAX_THROTTLE] from
 *              [deadband_high, maxThrottle].
 *   - reverse: motor at or below deadband_low scales into
 *              [DSHOT_MIN_THROTTLE, DSHOT_3D_DEADBAND_LOW] from
 *              [mincommand, deadband_low].
 *   - strictly inside (deadband_low, deadband_high) -> disarm command.
 * ------------------------------------------------------------------------- */
static uint16_t expectedScaledOutput(int16_t input, int16_t inMin, int16_t inMax, int16_t outMin, int16_t outMax)
{
    int16_t value = (int16_t)scaleRangef(input, inMin, inMax, outMin, outMax);
    value = constrain(value, outMin, outMax);
    return (uint16_t)value;
}

// Matches the live code's inclusive `>=`/`<=` comparisons: exact equality
// with deadbandHigh/deadbandLow belongs to the thrust side and scales; only
// strictly-inside values disarm.
static uint16_t expectedForwardDshot(int16_t motorValue, int16_t deadbandHigh, int16_t maxThrottle)
{
    if (motorValue < deadbandHigh) {
        return DSHOT_DISARM_COMMAND;
    }
    return expectedScaledOutput(motorValue, deadbandHigh, maxThrottle, DSHOT_3D_DEADBAND_HIGH, DSHOT_MAX_THROTTLE);
}

static uint16_t expectedReverseDshot(int16_t motorValue, int16_t mincommand, int16_t deadbandLow)
{
    if (motorValue > deadbandLow) {
        return DSHOT_DISARM_COMMAND;
    }
    return expectedScaledOutput(motorValue, mincommand, deadbandLow, DSHOT_MIN_THROTTLE, DSHOT_3D_DEADBAND_LOW);
}

namespace {

/* Realistic configuration values, as requested in the task:
 *  - reversibleMotorsConfig()->deadband_low/deadband_high/neutral default to
 *    SETTING_3D_DEADBAND_LOW_DEFAULT/SETTING_3D_DEADBAND_HIGH_DEFAULT/
 *    SETTING_3D_NEUTRAL_DEFAULT (1406 / 1514 / 1460, per src/main/fc/settings.yaml).
 *  - motorConfig()->mincommand defaults to SETTING_MIN_COMMAND_DEFAULT (1000).
 *  - getMaxThrottle() is MAX_THROTTLE (2000) for non-rover/boat platforms.
 *  - throttleDeadbandHigh/Low are PWM_RANGE_MIDDLE (1500) +/-
 *    rcControlsConfig()->mid_throttle_deadband, default
 *    SETTING_3D_DEADBAND_THROTTLE_DEFAULT (50) -> 1550 / 1450. This is the
 *    value mixerResetDisarmedMotors() stores into throttleRangeMin for
 *    reversible motors - no longer consulted while disarmed post-fix, but
 *    still used here to exercise the armed-flight regression tests.
 */
constexpr int16_t REV_DEADBAND_LOW = 1406;
constexpr int16_t REV_DEADBAND_HIGH = 1514;
constexpr int16_t MOTOR_MINCOMMAND = 1000;
constexpr int16_t MAX_THROTTLE_VAL = 2000;
constexpr int16_t THROTTLE_DEADBAND_HIGH = 1550; // PWM_RANGE_MIDDLE(1500) + mid_throttle_deadband(50)
constexpr int16_t THROTTLE_DEADBAND_LOW = 1450;  // PWM_RANGE_MIDDLE(1500) - mid_throttle_deadband(50)

/* ==========================================================================
 * Disarmed reversible-motor DShot output: direction is inferred directly
 * from motor[i] against the ESC's real 3D deadband, independent of
 * reversibleMotorsThrottleState/throttleRangeMin/Max (which are not
 * consulted while disarmed).
 * ========================================================================== */

TEST(MixerWriteMotorsDisarmed, ForwardValueAboveDeadbandHighIsScaledCorrectly)
{
    // motor value is above the ESC's real 3D deadband_high (1514) - a real
    // forward-thrust command - and below the stale throttleRangeMin (1550)
    // that writeMotors() used to (incorrectly) consult while disarmed. The
    // fixed code ignores throttleRangeMin entirely while disarmed.
    const int16_t motorValue = 1520;

    const uint16_t expected = expectedForwardDshot(motorValue, REV_DEADBAND_HIGH, MAX_THROTTLE_VAL);
    ASSERT_NE(expected, (uint16_t)DSHOT_DISARM_COMMAND)
        << "test setup error: expected value should be a real forward thrust command";

    const uint16_t actual = writeMotorsReversibleDshot_CurrentSource(
        motorValue,
        false,                    // disarmed
        MOTOR_DIRECTION_FORWARD,  // stale/irrelevant while disarmed post-fix
        THROTTLE_DEADBAND_HIGH,   // stale/irrelevant while disarmed post-fix
        MAX_THROTTLE_VAL,
        REV_DEADBAND_LOW,
        REV_DEADBAND_HIGH,
        MOTOR_MINCOMMAND,
        MAX_THROTTLE_VAL);

    EXPECT_EQ(expected, actual)
        << "writeMotors() must scale motor=" << motorValue << " (above deadband_high="
        << REV_DEADBAND_HIGH << ") into a valid forward DShot value using deadband_high, "
           "not disarm it because it's below the (no-longer-consulted-while-disarmed) "
           "RC-stick deadband (throttleRangeMin=" << THROTTLE_DEADBAND_HIGH << ").";
}

TEST(MixerWriteMotorsDisarmed, ReverseValueProducesReverseThrust)
{
    // motor value is below the ESC's real 3D deadband_low (1406) - a real
    // reverse-thrust command. Even though reversibleMotorsThrottleState is
    // passed in as stuck FORWARD (as it would be while disarmed, since
    // mixTable() never ran to flip it to BACKWARD), the fixed code no longer
    // consults it while disarmed - direction comes from motor[i] itself.
    const int16_t motorValue = 1200;

    const uint16_t expected = expectedReverseDshot(motorValue, MOTOR_MINCOMMAND, REV_DEADBAND_LOW);
    ASSERT_NE(expected, (uint16_t)DSHOT_DISARM_COMMAND)
        << "test setup error: expected value should be a real reverse thrust command";

    const uint16_t actual = writeMotorsReversibleDshot_CurrentSource(
        motorValue,
        false,                    // disarmed
        MOTOR_DIRECTION_FORWARD,  // stale/irrelevant while disarmed post-fix
        THROTTLE_DEADBAND_HIGH,
        MAX_THROTTLE_VAL,
        REV_DEADBAND_LOW,
        REV_DEADBAND_HIGH,
        MOTOR_MINCOMMAND,
        MAX_THROTTLE_VAL);

    EXPECT_EQ(expected, actual)
        << "writeMotors() must produce a reverse-thrust DShot value for motor=" << motorValue
        << " (below deadband_low=" << REV_DEADBAND_LOW << ") while disarmed, regardless of "
           "the stale reversibleMotorsThrottleState value.";
}

TEST(MixerWriteMotorsDisarmed, DeadbandZoneStillProducesDisarmCommand)
{
    // motor value squarely inside (deadband_low, deadband_high) must still
    // disarm the motor regardless of which direction it's evaluated from.
    // This held both before and after the fix.
    const int16_t motorValue = 1460; // == reversibleMotorsConfig()->neutral

    ASSERT_EQ((uint16_t)DSHOT_DISARM_COMMAND, expectedForwardDshot(motorValue, REV_DEADBAND_HIGH, MAX_THROTTLE_VAL));
    ASSERT_EQ((uint16_t)DSHOT_DISARM_COMMAND, expectedReverseDshot(motorValue, MOTOR_MINCOMMAND, REV_DEADBAND_LOW));

    const uint16_t actual = writeMotorsReversibleDshot_CurrentSource(
        motorValue,
        false,                    // disarmed
        MOTOR_DIRECTION_FORWARD,
        THROTTLE_DEADBAND_HIGH,
        MAX_THROTTLE_VAL,
        REV_DEADBAND_LOW,
        REV_DEADBAND_HIGH,
        MOTOR_MINCOMMAND,
        MAX_THROTTLE_VAL);

    EXPECT_EQ((uint16_t)DSHOT_DISARM_COMMAND, actual);
}

TEST(MixerWriteMotorsDisarmed, ExactlyAtDeadbandHighScalesToMinimalForwardThrust)
{
    // motor[i] == deadband_high must SCALE, not disarm: the live code's `>=`
    // comparison is inclusive, so exact equality belongs to the thrust side
    // (matching mixTable()'s >=/<= direction switch and
    // handleOutputScaling()'s strict-inequality stop check, where exact
    // equality never stops the motor). At this exact boundary the scaled
    // fractional position is zero, so the result is exactly
    // DSHOT_3D_DEADBAND_HIGH (1048) - the minimal forward-thrust DShot value.
    const int16_t motorValue = REV_DEADBAND_HIGH;

    const uint16_t expected = expectedForwardDshot(motorValue, REV_DEADBAND_HIGH, MAX_THROTTLE_VAL);
    ASSERT_EQ((uint16_t)DSHOT_3D_DEADBAND_HIGH, expected) << "test setup sanity check";

    const uint16_t actual = writeMotorsReversibleDshot_CurrentSource(
        motorValue,
        false,                    // disarmed
        MOTOR_DIRECTION_FORWARD,
        THROTTLE_DEADBAND_HIGH,
        MAX_THROTTLE_VAL,
        REV_DEADBAND_LOW,
        REV_DEADBAND_HIGH,
        MOTOR_MINCOMMAND,
        MAX_THROTTLE_VAL);

    EXPECT_EQ((uint16_t)DSHOT_3D_DEADBAND_HIGH, actual)
        << "motor[i] exactly at deadband_high must scale to the minimal forward-thrust DShot "
           "value (DSHOT_3D_DEADBAND_HIGH), not fall through to DSHOT_DISARM_COMMAND.";
}

TEST(MixerWriteMotorsDisarmed, ExactlyAtDeadbandLowScalesToMinimalReverseThrust)
{
    // motor[i] == deadband_low must SCALE, not disarm: the live code's `<=`
    // comparison is inclusive, so exact equality belongs to the thrust side.
    // At this exact boundary the scaled fractional position is at its
    // maximum (denominator == numerator), so the result is exactly
    // DSHOT_3D_DEADBAND_LOW (1047) - the minimal reverse-thrust DShot value.
    const int16_t motorValue = REV_DEADBAND_LOW;

    const uint16_t expected = expectedReverseDshot(motorValue, MOTOR_MINCOMMAND, REV_DEADBAND_LOW);
    ASSERT_EQ((uint16_t)DSHOT_3D_DEADBAND_LOW, expected) << "test setup sanity check";

    const uint16_t actual = writeMotorsReversibleDshot_CurrentSource(
        motorValue,
        false,                    // disarmed
        MOTOR_DIRECTION_FORWARD,
        THROTTLE_DEADBAND_HIGH,
        MAX_THROTTLE_VAL,
        REV_DEADBAND_LOW,
        REV_DEADBAND_HIGH,
        MOTOR_MINCOMMAND,
        MAX_THROTTLE_VAL);

    EXPECT_EQ((uint16_t)DSHOT_3D_DEADBAND_LOW, actual)
        << "motor[i] exactly at deadband_low must scale to the minimal reverse-thrust DShot "
           "value (DSHOT_3D_DEADBAND_LOW), not fall through to DSHOT_DISARM_COMMAND.";
}

/* ==========================================================================
 * Degenerate-config guards inside calculateDisarmedReversibleMotorsDshotValue:
 * reachable via valid-looking CLI configuration (min_command/3d_deadband_low/
 * 3d_deadband_high/max_throttle are independently settable), these guards
 * prevent scaleRangef()'s denominator (srcMax - srcMin) from becoming zero
 * or negative. Calls the reproduction function directly with the degenerate
 * parameter combinations, exactly as the task spec requests.
 * ========================================================================== */

TEST(MixerWriteMotorsDisarmed, ForwardDegenerateConfigMaxThrottleAtDeadbandHighReturnsMaxThrottle)
{
    // Degenerate config: max_throttle <= deadband_high (here, equal), so the
    // forward-scaling source range [deadbandHigh, maxThrottle] has zero
    // width. Must clamp to DSHOT_MAX_THROTTLE instead of dividing by zero.
    const int16_t deadbandHigh = 2000;
    const int16_t maxThrottle = 2000;
    const int16_t motorValue = 2000; // >= deadbandHigh

    const uint16_t actual = calculateDisarmedReversibleMotorsDshotValue_CurrentSource(
        motorValue, REV_DEADBAND_LOW, deadbandHigh, MOTOR_MINCOMMAND, maxThrottle);

    EXPECT_EQ((uint16_t)DSHOT_MAX_THROTTLE, actual)
        << "Degenerate config (deadband_high >= max_throttle) must not divide by a "
           "zero/negative-width range - it must clamp to DSHOT_MAX_THROTTLE instead.";
}

TEST(MixerWriteMotorsDisarmed, ReverseDegenerateConfigDeadbandLowAtMincommandReturnsMinThrottle)
{
    // Degenerate config: deadband_low <= mincommand (here, equal), so the
    // reverse-scaling source range [mincommand, deadbandLow] has zero width.
    // Must clamp to DSHOT_MIN_THROTTLE instead of dividing by zero.
    const int16_t deadbandLow = 1000;
    const int16_t mincommand = 1000;
    const int16_t motorValue = 1000; // <= deadbandLow

    const uint16_t actual = calculateDisarmedReversibleMotorsDshotValue_CurrentSource(
        motorValue, deadbandLow, REV_DEADBAND_HIGH, mincommand, MAX_THROTTLE_VAL);

    EXPECT_EQ((uint16_t)DSHOT_MIN_THROTTLE, actual)
        << "Degenerate config (deadband_low <= mincommand) must not divide by a "
           "zero/negative-width range - it must clamp to DSHOT_MIN_THROTTLE instead.";
}

/* ==========================================================================
 * REGRESSION - armed-flight DShot output path must be completely unchanged.
 *
 * While ARMED, mixTable() runs every loop and keeps
 * reversibleMotorsThrottleState/throttleRangeMin/throttleRangeMax correctly
 * up to date; these tests pin down the correct behaviour for that case,
 * confirming the fix (scoped to the disarmed branch) left it untouched.
 * ========================================================================== */

TEST(MixerWriteMotorsArmedRegression, ForwardPathUnaffected)
{
    // As mixTable() would set them for a FORWARD armed throttle command:
    // throttleRangeMin = throttleDeadbandHigh, throttleRangeMax = getMaxThrottle().
    const int16_t motorValue = 1800;

    const uint16_t expected = expectedScaledOutput(
        motorValue, THROTTLE_DEADBAND_HIGH, MAX_THROTTLE_VAL, DSHOT_3D_DEADBAND_HIGH, DSHOT_MAX_THROTTLE);

    const uint16_t actual = writeMotorsReversibleDshot_CurrentSource(
        motorValue,
        true,  // armed
        MOTOR_DIRECTION_FORWARD,
        THROTTLE_DEADBAND_HIGH,
        MAX_THROTTLE_VAL,
        REV_DEADBAND_LOW,
        REV_DEADBAND_HIGH,
        MOTOR_MINCOMMAND,
        MAX_THROTTLE_VAL);

    EXPECT_EQ(expected, actual);
}

TEST(MixerWriteMotorsArmedRegression, BackwardPathUnaffected)
{
    // As mixTable() would set them for a BACKWARD armed throttle command:
    // throttleRangeMax = throttleDeadbandLow, throttleRangeMin = motorConfig()->mincommand.
    const int16_t motorValue = THROTTLE_DEADBAND_LOW; // right at the boundary

    const uint16_t expected = expectedScaledOutput(
        motorValue, MOTOR_MINCOMMAND, THROTTLE_DEADBAND_LOW, DSHOT_MIN_THROTTLE, DSHOT_3D_DEADBAND_LOW);
    ASSERT_EQ((uint16_t)DSHOT_3D_DEADBAND_LOW, expected) << "test setup sanity check";

    const uint16_t actual = writeMotorsReversibleDshot_CurrentSource(
        motorValue,
        true,  // armed
        MOTOR_DIRECTION_BACKWARD,
        MOTOR_MINCOMMAND,
        THROTTLE_DEADBAND_LOW,
        REV_DEADBAND_LOW,
        REV_DEADBAND_HIGH,
        MOTOR_MINCOMMAND,
        MAX_THROTTLE_VAL);

    EXPECT_EQ(expected, actual);
}

/* ==========================================================================
 * REGRESSION - non-reversible (non-3D) DShot motor testing while disarmed.
 * This branch does not use reversibleMotorsThrottleState at all and was not
 * touched by the fix (which is scoped to the FEATURE_REVERSIBLE_MOTORS
 * branch only).
 * ========================================================================== */

TEST(MixerWriteMotorsNonReversibleRegression, BelowThrottleIdleIsDisarmed)
{
    const int16_t throttleIdleValue = 1150; // mincommand(1000) + 15% of (maxThrottle-mincommand)
    const int16_t motorValue = MOTOR_MINCOMMAND;

    const uint16_t actual = writeMotorsNonReversibleDshot_CurrentSource(
        motorValue, throttleIdleValue, MOTOR_MINCOMMAND, MAX_THROTTLE_VAL);

    EXPECT_EQ((uint16_t)DSHOT_DISARM_COMMAND, actual);
}

TEST(MixerWriteMotorsNonReversibleRegression, AboveThrottleIdleIsScaledNormally)
{
    const int16_t throttleIdleValue = 1150;
    const int16_t motorValue = 1500;

    const uint16_t expected = expectedScaledOutput(
        motorValue, MOTOR_MINCOMMAND, MAX_THROTTLE_VAL, DSHOT_MIN_THROTTLE, DSHOT_MAX_THROTTLE);

    const uint16_t actual = writeMotorsNonReversibleDshot_CurrentSource(
        motorValue, throttleIdleValue, MOTOR_MINCOMMAND, MAX_THROTTLE_VAL);

    EXPECT_EQ(expected, actual);
}

/* ==========================================================================
 * SourceSync - reads the ACTUAL LIVE src/main/flight/mixer.c (and
 * common/maths.c) off disk at test time (not a cached copy) and asserts
 * that the fragments this file's `_CurrentSource` reproduction depends on
 * are still present, INCLUDING the presence of the `ARMING_FLAG(ARMED)`
 * guard in writeMotors() - which is the fix. If mixer.c changes again, some
 * of these will (by design) start failing; that is the signal to update
 * this test file's reproduction and expectations to match the change, not
 * necessarily a sign anything is wrong.
 * ========================================================================== */

std::string stripLineComments(const std::string &text)
{
    std::string result;
    result.reserve(text.size());
    for (size_t i = 0; i < text.size();) {
        if (text[i] == '/' && i + 1 < text.size() && text[i + 1] == '/') {
            while (i < text.size() && text[i] != '\n') {
                i++;
            }
            continue;
        }
        result += text[i];
        i++;
    }
    return result;
}

std::string normalizeWhitespace(const std::string &text)
{
    std::string result;
    result.reserve(text.size());
    bool lastWasSpace = true;
    for (char c : text) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!lastWasSpace) {
                result += ' ';
                lastWasSpace = true;
            }
        } else {
            result += c;
            lastWasSpace = false;
        }
    }
    while (!result.empty() && result.back() == ' ') {
        result.pop_back();
    }
    return result;
}

std::string normalizeSource(const std::string &text)
{
    return normalizeWhitespace(stripLineComments(text));
}

// Resolves a path relative to this test file's own directory. __FILE__ is
// emitted as an absolute path by this project's CMake/Makefiles, so this
// resolves correctly regardless of the build directory or CWD the test is
// invoked from.
std::string pathRelativeToThisFile(const std::string &relative)
{
    std::string thisFile = __FILE__; // .../src/test/unit/mixer_unittest.cc
    size_t lastSlash = thisFile.find_last_of("/\\");
    std::string testUnitDir = (lastSlash == std::string::npos) ? "." : thisFile.substr(0, lastSlash);
    return testUnitDir + "/" + relative;
}

::testing::AssertionResult loadNormalizedFile(const std::string &path, std::string *out)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        return ::testing::AssertionFailure()
            << "Could not open live source file at computed path: " << path
            << " -- SourceSync tests cannot verify anything against dead/absent source.";
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    if (buffer.str().empty()) {
        return ::testing::AssertionFailure() << "Live source file at " << path << " opened but read as empty.";
    }
    *out = normalizeSource(buffer.str());
    return ::testing::AssertionSuccess();
}

// Extracts the substring of `normalizedSource` starting at `startMarker`
// (inclusive) and ending just before the next occurrence of `endMarker`.
::testing::AssertionResult extractRegion(
    const std::string &normalizedSource,
    const std::string &startMarker,
    const std::string &endMarker,
    std::string *out)
{
    size_t start = normalizedSource.find(startMarker);
    if (start == std::string::npos) {
        return ::testing::AssertionFailure()
            << "Could not find start marker in live source: \"" << startMarker << "\"";
    }
    size_t end = normalizedSource.find(endMarker, start + startMarker.size());
    if (end == std::string::npos) {
        return ::testing::AssertionFailure()
            << "Could not find end marker in live source (after start marker): \"" << endMarker << "\"";
    }
    *out = normalizedSource.substr(start, end - start);
    return ::testing::AssertionSuccess();
}

TEST(SourceSync, DshotConstantsMatchLiveMixerHeader)
{
    std::string normalizedHeader;
    ASSERT_TRUE(loadNormalizedFile(pathRelativeToThisFile("../../main/flight/mixer.h"), &normalizedHeader));

    EXPECT_NE(normalizedHeader.find(normalizeSource("#define DSHOT_DISARM_COMMAND 0")), std::string::npos);
    EXPECT_NE(normalizedHeader.find(normalizeSource("#define DSHOT_MIN_THROTTLE 48")), std::string::npos);
    EXPECT_NE(normalizedHeader.find(normalizeSource("#define DSHOT_MAX_THROTTLE 2047")), std::string::npos);
    EXPECT_NE(normalizedHeader.find(normalizeSource("#define DSHOT_3D_DEADBAND_LOW 1047")), std::string::npos);
    EXPECT_NE(normalizedHeader.find(normalizeSource("#define DSHOT_3D_DEADBAND_HIGH 1048")), std::string::npos)
        << "DSHOT_* constants in the live mixer.h no longer match the values hard-coded "
           "in this test file - update both this file's #defines and the comments above.";
}

TEST(SourceSync, MathsHelpersMatchLiveSource)
{
    std::string normalizedSource;
    ASSERT_TRUE(loadNormalizedFile(pathRelativeToThisFile("../../main/common/maths.c"), &normalizedSource));

    const std::string expectedScaleRangef = normalizeSource(
        "float a = (destMax - destMin) * (x - srcMin); "
        "float b = srcMax - srcMin; "
        "return ((a / b) + destMin);");
    EXPECT_NE(normalizedSource.find(expectedScaleRangef), std::string::npos)
        << "scaleRangef() in the live common/maths.c no longer matches the copy in this test "
           "file - update it to match.";

    const std::string expectedConstrain = normalizeSource(
        "if (amt < low) return low; else if (amt > high) return high; else return amt;");
    EXPECT_NE(normalizedSource.find(expectedConstrain), std::string::npos)
        << "constrain() in the live common/maths.c no longer matches the copy in this test "
           "file - update it to match.";
}

TEST(SourceSync, HandleOutputScalingMatchesLiveSource)
{
    std::string normalizedSource;
    ASSERT_TRUE(loadNormalizedFile(pathRelativeToThisFile("../../main/flight/mixer.c"), &normalizedSource));

    const std::string expectedBody = normalizeSource(
        "if ((moveForward && input < stopThreshold) || (!moveForward && input > stopThreshold)) { "
        "value = onStopValue; "
        "} "
        "else { "
        "value = scaleRangef(input, inputScaleMin, inputScaleMax, outputScaleMin, outputScaleMax); "
        "value = constrain(value, outputScaleMin, outputScaleMax); "
        "}");

    EXPECT_NE(normalizedSource.find(expectedBody), std::string::npos)
        << "handleOutputScaling() in the LIVE src/main/flight/mixer.c no longer matches "
           "handleOutputScaling_CurrentSource() in this test file. Update the reproduction "
           "(and the expected-value helpers, if the scaling formula itself changed) to match.";
}

TEST(SourceSync, CalculateDisarmedReversibleMotorsDshotValueMatchesLiveSource)
{
    std::string normalizedSource;
    ASSERT_TRUE(loadNormalizedFile(pathRelativeToThisFile("../../main/flight/mixer.c"), &normalizedSource));

    const std::string expectedBody = normalizeSource(
        "static uint16_t calculateDisarmedReversibleMotorsDshotValue( "
        "int16_t motorValue, "
        "int16_t deadbandLow, "
        "int16_t deadbandHigh, "
        "int16_t mincommand, "
        "int16_t maxThrottle) "
        "{ "
        "if (motorValue >= deadbandHigh) { "
        "if (maxThrottle <= deadbandHigh) { "
        "return DSHOT_MAX_THROTTLE; "
        "} "
        "int32_t scaled = (int32_t)scaleRangef(motorValue, deadbandHigh, maxThrottle, DSHOT_3D_DEADBAND_HIGH, DSHOT_MAX_THROTTLE); "
        "return constrain(scaled, DSHOT_3D_DEADBAND_HIGH, DSHOT_MAX_THROTTLE); "
        "} "
        "if (motorValue <= deadbandLow) { "
        "if (deadbandLow <= mincommand) { "
        "return DSHOT_MIN_THROTTLE; "
        "} "
        "int32_t scaled = (int32_t)scaleRangef(motorValue, mincommand, deadbandLow, DSHOT_MIN_THROTTLE, DSHOT_3D_DEADBAND_LOW); "
        "return constrain(scaled, DSHOT_MIN_THROTTLE, DSHOT_3D_DEADBAND_LOW); "
        "} "
        "return DSHOT_DISARM_COMMAND; "
        "}");

    EXPECT_NE(normalizedSource.find(expectedBody), std::string::npos)
        << "calculateDisarmedReversibleMotorsDshotValue() in the LIVE src/main/flight/mixer.c "
           "no longer matches calculateDisarmedReversibleMotorsDshotValue_CurrentSource() in "
           "this test file. Update the reproduction (and the boundary/degenerate-config guard "
           "tests, if the guard conditions or comparison operators themselves changed) to "
           "match.";
}

TEST(SourceSync, WriteMotorsReversibleDshotBranchMatchesLiveSource)
{
    std::string normalizedSource;
    ASSERT_TRUE(loadNormalizedFile(pathRelativeToThisFile("../../main/flight/mixer.c"), &normalizedSource));

    std::string writeMotorsBody;
    ASSERT_TRUE(extractRegion(
        normalizedSource,
        normalizeSource("void FAST_CODE writeMotors(void) {"),
        normalizeSource("void writeAllMotors(int16_t mc) {"),
        &writeMotorsBody));

    // Covers both the disarmed branch (now just a call into
    // calculateDisarmedReversibleMotorsDshotValue(), direction inferred from
    // motor[i] against the ESC's real 3D deadband) and the still-unchanged
    // armed FORWARD/BACKWARD dispatch that follows it in the else-if chain.
    const std::string expectedReversibleDshotBranch = normalizeSource(
        "if (!ARMING_FLAG(ARMED)) { "
        "motorValue = calculateDisarmedReversibleMotorsDshotValue( "
        "motor[i], "
        "reversibleMotorsConfig()->deadband_low, "
        "reversibleMotorsConfig()->deadband_high, "
        "motorConfig()->mincommand, "
        "getMaxThrottle() "
        "); "
        "} else if (reversibleMotorsThrottleState == MOTOR_DIRECTION_FORWARD) { "
        "motorValue = handleOutputScaling( "
        "motor[i], "
        "throttleRangeMin, "
        "DSHOT_DISARM_COMMAND, "
        "throttleRangeMin, "
        "throttleRangeMax, "
        "DSHOT_3D_DEADBAND_HIGH, "
        "DSHOT_MAX_THROTTLE, "
        "true "
        "); "
        "} else { "
        "motorValue = handleOutputScaling( "
        "motor[i], "
        "throttleRangeMax, "
        "DSHOT_DISARM_COMMAND, "
        "throttleRangeMin, "
        "throttleRangeMax, "
        "DSHOT_MIN_THROTTLE, "
        "DSHOT_3D_DEADBAND_LOW, "
        "false "
        "); "
        "}");

    EXPECT_NE(writeMotorsBody.find(expectedReversibleDshotBranch), std::string::npos)
        << "The FEATURE_REVERSIBLE_MOTORS DShot branch inside the LIVE writeMotors() no "
           "longer matches writeMotorsReversibleDshot_CurrentSource() in this test file. "
           "Update the reproduction (and, if the disarmed-branch call site itself changed, "
           "the expected-value helpers) to match.";
}

TEST(SourceSync, WriteMotorsNowHasArmedCheck)
{
    // This is the fix: writeMotors() must gate its reversible-motors DShot
    // branch on `!ARMING_FLAG(ARMED)` before falling back to the
    // reversibleMotorsThrottleState-based dispatch. If a future change
    // removes this check again (regressing back to the original bug), this
    // test will (by design) fail - that is the signal to re-investigate
    // rather than a sign this test itself is broken.
    std::string normalizedSource;
    ASSERT_TRUE(loadNormalizedFile(pathRelativeToThisFile("../../main/flight/mixer.c"), &normalizedSource));

    std::string writeMotorsBody;
    ASSERT_TRUE(extractRegion(
        normalizedSource,
        normalizeSource("void FAST_CODE writeMotors(void) {"),
        normalizeSource("void writeAllMotors(int16_t mc) {"),
        &writeMotorsBody));

    EXPECT_NE(writeMotorsBody.find(normalizeSource("if (!ARMING_FLAG(ARMED)) {")), std::string::npos)
        << "writeMotors() in the live mixer.c no longer contains the `if (!ARMING_FLAG(ARMED))` "
           "check inside its FEATURE_REVERSIBLE_MOTORS DShot branch. This looks like the "
           "disarmed-motor-testing fix has been reverted or altered - this test file's "
           "reproduction (writeMotorsReversibleDshot_CurrentSource) and the "
           "MixerWriteMotorsDisarmed test expectations need to be re-checked against the new "
           "behaviour.";
}

TEST(SourceSync, MixerResetDisarmedMotorsStillUsesThrottleDeadbandHighNotConfigDeadbandHigh)
{
    // Confirms the surrounding code this fix did NOT touch:
    // mixerResetDisarmedMotors() still seeds throttleRangeMin with
    // throttleDeadbandHigh (the RC-stick deadband), not
    // reversibleMotorsConfig()->deadband_high (the ESC 3D deadband), for the
    // reversible-motors case. This value is no longer consulted while
    // disarmed post-fix, but the armed-flight regression tests above still
    // rely on this fact holding for the pre-fix statics they exercise.
    std::string normalizedSource;
    ASSERT_TRUE(loadNormalizedFile(pathRelativeToThisFile("../../main/flight/mixer.c"), &normalizedSource));

    std::string resetBody;
    ASSERT_TRUE(extractRegion(
        normalizedSource,
        normalizeSource("void mixerResetDisarmedMotors(void) {"),
        normalizeSource("static uint16_t handleOutputScaling("),
        &resetBody));

    const std::string expectedFragment = normalizeSource(
        "if (feature(FEATURE_REVERSIBLE_MOTORS)) { "
        "motorZeroCommand = reversibleMotorsConfig()->neutral; "
        "throttleRangeMin = throttleDeadbandHigh; "
        "throttleRangeMax = getMaxThrottle(); "
        "}");

    EXPECT_NE(resetBody.find(expectedFragment), std::string::npos)
        << "mixerResetDisarmedMotors() in the live mixer.c no longer matches this test's "
           "assumption. Update THROTTLE_DEADBAND_HIGH/LOW and the armed-regression test "
           "expectations in this file to match the new logic.";

    // And confirm the state is unconditionally reset to FORWARD, every time -
    // no longer load-bearing for the disarmed branch post-fix (which ignores
    // this static entirely), but still what mixTable() would see if it were
    // ever consulted.
    EXPECT_NE(resetBody.find(normalizeSource("reversibleMotorsThrottleState = MOTOR_DIRECTION_FORWARD;")),
        std::string::npos);
}

TEST(SourceSync, MixTableStillEarlyReturnsBeforeSettingReversibleMotorsThrottleState)
{
    // Confirms mixTable() (the ONLY place that ever updates
    // reversibleMotorsThrottleState/throttleRangeMin/throttleRangeMax to
    // BACKWARD) still early-returns while disarmed, BEFORE reaching that
    // logic - i.e. those statics still structurally cannot be updated while
    // disarmed today. This is no longer load-bearing for correctness (the
    // fix stopped consulting them while disarmed), but documents why the
    // fix was necessary in the first place.
    std::string normalizedSource;
    ASSERT_TRUE(loadNormalizedFile(pathRelativeToThisFile("../../main/flight/mixer.c"), &normalizedSource));

    std::string mixTableBody;
    ASSERT_TRUE(extractRegion(
        normalizedSource,
        normalizeSource("void FAST_CODE mixTable(void)"),
        normalizeSource("int16_t getThrottlePercent(bool useScaled)"),
        &mixTableBody));

    size_t earlyReturnPos = mixTableBody.find(normalizeSource("if (isDisarmed || motorStopIsActive) {"));
    size_t backwardAssignPos = mixTableBody.find(
        normalizeSource("reversibleMotorsThrottleState = MOTOR_DIRECTION_BACKWARD;"));

    ASSERT_NE(earlyReturnPos, std::string::npos) << "Could not find the disarmed early-return guard in mixTable().";
    ASSERT_NE(backwardAssignPos, std::string::npos)
        << "Could not find the BACKWARD direction-switch assignment in mixTable().";

    EXPECT_LT(earlyReturnPos, backwardAssignPos)
        << "mixTable()'s disarmed/motor-stop early return must appear BEFORE the "
           "direction-switching logic that sets reversibleMotorsThrottleState - if this "
           "ordering changed, re-evaluate whether the fix in writeMotors() is still necessary "
           "and correct.";
}

} // namespace
