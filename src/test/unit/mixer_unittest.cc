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
 * 3D/reversible motors, both while ARMED (normal flight) and while DISARMED
 * (motor-testing via the MSP_SET_MOTOR path used by the configurator's
 * motor-test UI).
 *
 * writeMotors()'s FEATURE_REVERSIBLE_MOTORS DShot branch starts with an
 * `if (!ARMING_FLAG(ARMED))` check. While disarmed, direction is inferred
 * directly from motor[i] against the ESC's real 3D deadband
 * (reversibleMotorsConfig()->deadband_high / ->deadband_low) rather than
 * from `reversibleMotorsThrottleState`/`throttleRangeMin`/`throttleRangeMax`
 * (those statics are only ever updated by mixTable()'s armed-flight
 * direction-switching logic, which never runs while disarmed):
 *   - motor[i] above deadband_high -> scaled into
 *     [DSHOT_3D_DEADBAND_HIGH, DSHOT_MAX_THROTTLE] from
 *     [deadband_high, getMaxThrottle()] (forward thrust).
 *   - motor[i] below deadband_low -> scaled into
 *     [DSHOT_MIN_THROTTLE, DSHOT_3D_DEADBAND_LOW] from
 *     [motorConfig()->mincommand, deadband_low] (reverse thrust).
 *   - motor[i] inside [deadband_low, deadband_high] -> DSHOT_DISARM_COMMAND.
 * While armed, the dispatch is based on
 * reversibleMotorsThrottleState/throttleRangeMin/throttleRangeMax via
 * handleOutputScaling(), in the `else if` chain after the disarmed check.
 *
 * Why this file hand-reproduces mixer.c's logic instead of linking it
 * -------------------------------------------------------------------
 * writeMotors()'s entire body (and its helper handleOutputScaling()) is
 * wrapped in `#if !defined(SITL_BUILD)`. Every unit test in this directory
 * is compiled against src/test/unit/target.h, which unconditionally
 * `#define`s SITL_BUILD, so the DShot-scaling code under test can never
 * actually be *compiled* by this test harness - it is structurally excluded.
 * This file hand-reproduces the relevant functions verbatim (labelled
 * `_CurrentSource`) and adds a "SourceSync" test suite at the bottom that
 * reads the ACTUAL LIVE src/main/flight/mixer.c off disk at test time and
 * asserts the key fragments this reproduction depends on - including the
 * presence of the `ARMING_FLAG(ARMED)` check - are still present. If
 * mixer.c changes again in the future, the SourceSync tests will fail,
 * flagging that this file's inline reproduction (and the expectations built
 * on top of it) need to be updated to match.
 *
 * This file is fully self-contained (no CMakeLists.txt `depends` entry
 * needed - it is picked up purely by the `*_unittest.cc` glob): even
 * scaleRangef()/constrain() (used both inside the reproduction and to
 * independently compute "should be" expected values) are hand-reproduced
 * from common/maths.c below, with their own SourceSync check.
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
 * Verbatim reproduction of the FEATURE_REVERSIBLE_MOTORS + isMotorProtocolDigital
 * branch inside writeMotors() (mixer.c, roughly lines 377-419), AS IT EXISTS
 * TODAY (post-fix). Takes an explicit `isArmed` parameter mirroring the live
 * `!ARMING_FLAG(ARMED)` check, plus the ESC 3D deadband/mincommand/maxThrottle
 * values needed by the disarmed branch. `reversibleMotorsThrottleState` /
 * `throttleRangeMin` / `throttleRangeMax` are still taken as plain parameters
 * (mixer.c holds them as file-scope statics) - they are only consulted, and
 * only meaningful, when `isArmed` is true, exactly as in the live code.
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
        if (motorValue > deadbandHigh) {
            motorOutput = scaleRangef(motorValue,
                deadbandHigh, maxThrottle,
                DSHOT_3D_DEADBAND_HIGH, DSHOT_MAX_THROTTLE);
            motorOutput = constrain(motorOutput, DSHOT_3D_DEADBAND_HIGH, DSHOT_MAX_THROTTLE);
        } else if (motorValue < deadbandLow) {
            motorOutput = scaleRangef(motorValue,
                mincommand, deadbandLow,
                DSHOT_MIN_THROTTLE, DSHOT_3D_DEADBAND_LOW);
            motorOutput = constrain(motorOutput, DSHOT_MIN_THROTTLE, DSHOT_3D_DEADBAND_LOW);
        } else {
            motorOutput = DSHOT_DISARM_COMMAND;
        }
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
 * writeMotors() (mixer.c, roughly lines 421-431). This path does not depend
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
 * above (never calls handleOutputScaling_CurrentSource or the disarmed-branch
 * logic directly), using the same scaleRangef()/constrain() helpers so this
 * test never embeds a *second*, possibly-diverging copy of the interpolation
 * math. This encodes exactly the mapping the fix implements:
 *   - forward: motor above deadband_high scales into
 *              [DSHOT_3D_DEADBAND_HIGH, DSHOT_MAX_THROTTLE] from
 *              [deadband_high, maxThrottle].
 *   - reverse: motor below deadband_low scales into
 *              [DSHOT_MIN_THROTTLE, DSHOT_3D_DEADBAND_LOW] from
 *              [mincommand, deadband_low].
 *   - deadband zone (between deadband_low and deadband_high): disarm command.
 * ------------------------------------------------------------------------- */
static uint16_t expectedScaledOutput(int16_t input, int16_t inMin, int16_t inMax, int16_t outMin, int16_t outMax)
{
    int16_t value = (int16_t)scaleRangef(input, inMin, inMax, outMin, outMax);
    value = constrain(value, outMin, outMax);
    return (uint16_t)value;
}

// Matches the live code's strict `>`/`<` comparisons: exact equality with
// deadbandHigh/deadbandLow falls through to the disarm case, it does not scale.
static uint16_t expectedForwardDshot(int16_t motorValue, int16_t deadbandHigh, int16_t maxThrottle)
{
    if (motorValue <= deadbandHigh) {
        return DSHOT_DISARM_COMMAND;
    }
    return expectedScaledOutput(motorValue, deadbandHigh, maxThrottle, DSHOT_3D_DEADBAND_HIGH, DSHOT_MAX_THROTTLE);
}

static uint16_t expectedReverseDshot(int16_t motorValue, int16_t mincommand, int16_t deadbandLow)
{
    if (motorValue >= deadbandLow) {
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
    // motor value squarely inside [deadband_low, deadband_high] must still
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

TEST(MixerWriteMotorsDisarmed, ExactlyAtDeadbandHighIsDisarmedNotScaled)
{
    // motor[i] == deadband_high must NOT scale: the live code's `>` comparison
    // is strict, so equality falls through to the deadband/disarm case.
    const int16_t motorValue = REV_DEADBAND_HIGH;

    ASSERT_EQ((uint16_t)DSHOT_DISARM_COMMAND, expectedForwardDshot(motorValue, REV_DEADBAND_HIGH, MAX_THROTTLE_VAL));

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

TEST(MixerWriteMotorsDisarmed, ExactlyAtDeadbandLowIsDisarmedNotScaled)
{
    // motor[i] == deadband_low must NOT scale: the live code's `<` comparison
    // is strict, so equality falls through to the deadband/disarm case.
    const int16_t motorValue = REV_DEADBAND_LOW;

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

    // Covers both the new disarmed branch (direction inferred from motor[i]
    // against the ESC's real 3D deadband) and the still-unchanged armed
    // FORWARD/BACKWARD dispatch that now follows it in the else-if chain.
    const std::string expectedReversibleDshotBranch = normalizeSource(
        "if (!ARMING_FLAG(ARMED)) { "
        "if (motor[i] > reversibleMotorsConfig()->deadband_high) { "
        "motorValue = scaleRangef(motor[i], reversibleMotorsConfig()->deadband_high, getMaxThrottle(), "
        "DSHOT_3D_DEADBAND_HIGH, DSHOT_MAX_THROTTLE); "
        "motorValue = constrain(motorValue, DSHOT_3D_DEADBAND_HIGH, DSHOT_MAX_THROTTLE); "
        "} else if (motor[i] < reversibleMotorsConfig()->deadband_low) { "
        "motorValue = scaleRangef(motor[i], motorConfig()->mincommand, reversibleMotorsConfig()->deadband_low, "
        "DSHOT_MIN_THROTTLE, DSHOT_3D_DEADBAND_LOW); "
        "motorValue = constrain(motorValue, DSHOT_MIN_THROTTLE, DSHOT_3D_DEADBAND_LOW); "
        "} else { "
        "motorValue = DSHOT_DISARM_COMMAND; "
        "} "
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
           "Update the reproduction (and, if the disarmed-branch formula itself changed, "
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
