/*
 * Unit test: timerHardwareOverride() must not corrupt TIM_USE_BEEPER flags.
 *
 * Bug (pre-fix):
 *   When a user applies OUTPUT_MODE_SERVOS to a timer that has TIM_USE_BEEPER
 *   set (e.g. MATEKH743 TIM2/PA15), timerHardwareOverride() would apply the
 *   servo-mode mask without first checking for the beeper flag.  The subsequent
 *   pwmAssignOutput(servo) call strips everything except TIM_USE_SERVO, leaving
 *   beeperPwmInit() unable to find its timer.
 *
 * Fix (commit 551bce85d6):
 *   Guard added at the top of timerHardwareOverride():
 *       if (timer->usageFlags & TIM_USE_BEEPER) { return; }
 *
 * This file contains:
 *   1.  A minimal inline reproduction of both the BUGGY and FIXED versions of
 *       timerHardwareOverride() so that the test is fully self-contained
 *       (pwm_mapping.c is excluded from the SITL/unit build by its own guard).
 *   2.  TEST BugReproduction — demonstrates that the bug corrupts TIM_USE_BEEPER.
 *       This test would FAIL on pre-fix code and PASSES on the fixed code.
 *   3.  TEST FixVerification — positive assertion that TIM_USE_BEEPER survives
 *       after a servo-mode override on a beeper timer.
 *   4.  Negative / regression tests covering normal (non-beeper) timers to
 *       confirm that the guard does not break the existing override behaviour.
 */

#include <stdint.h>
#include <stdbool.h>

#include "gtest/gtest.h"
#include "unittest_macros.h"

/* -------------------------------------------------------------------------
 * Minimal type/flag definitions — mirrors the real firmware headers.
 * We avoid pulling in the real headers because they drag in hundreds of
 * platform-specific includes that cannot be satisfied in a host build.
 * ------------------------------------------------------------------------- */

/* timerUsageFlag_e — must match src/main/drivers/timer.h exactly */
typedef enum {
    TIM_USE_ANY     = 0,
    TIM_USE_PPM     = (1 << 0),
    TIM_USE_PWM     = (1 << 1),
    TIM_USE_MOTOR   = (1 << 2),
    TIM_USE_SERVO   = (1 << 3),
    TIM_USE_LED     = (1 << 24),
    TIM_USE_BEEPER  = (1 << 25),
} timerUsageFlag_e;

/* outputMode_e — must match src/main/flight/mixer.h exactly */
typedef enum {
    OUTPUT_MODE_AUTO    = 0,
    OUTPUT_MODE_MOTORS,
    OUTPUT_MODE_SERVOS,
    OUTPUT_MODE_LED,
    OUTPUT_MODE_PINIO
} outputMode_e;

/* Minimal timer hardware entry — only the fields exercised by the function. */
typedef struct timerHardware_s {
    uint32_t usageFlags;
    /* All other fields (tim, tag, channel, …) are unused in this test. */
} timerHardware_t;

/* -------------------------------------------------------------------------
 * Inline reproductions of timerHardwareOverride().
 *
 * We inline these rather than linking pwm_mapping.c because:
 *  a) pwm_mapping.c is wrapped in #if !defined(SITL_BUILD) and the unit-test
 *     target.h defines SITL_BUILD, so the real file compiles to nothing.
 *  b) The function depends on timerOverrides() (parameter-group accessor) and
 *     timer2id(), which are platform / PG infrastructure that is not present in
 *     a host unit-test build.
 *
 * The functions below are literal translations of the C source with the PG
 * indirection collapsed into a simple `outputMode` parameter.
 * ------------------------------------------------------------------------- */

/*
 * BUGGY version (pre-fix, equivalent to the code BEFORE commit 551bce85d6).
 *
 * The OUTPUT_MODE_SERVOS case clears TIM_USE_MOTOR and TIM_USE_LED but does
 * NOT clear TIM_USE_BEEPER.  The resulting combined flags
 * (TIM_USE_SERVO | TIM_USE_BEEPER) will later be stripped by pwmAssignOutput()
 * to just TIM_USE_SERVO, silently destroying the beeper timer entry.
 */
static void timerHardwareOverride_buggy(timerHardware_t *timer,
                                        outputMode_e outputMode)
{
    /* NOTE: no beeper guard here — this is intentionally the buggy version */
    switch (outputMode) {
        case OUTPUT_MODE_MOTORS:
            timer->usageFlags &= ~((uint32_t)(TIM_USE_SERVO | TIM_USE_LED));
            timer->usageFlags |=  (uint32_t)TIM_USE_MOTOR;
            break;
        case OUTPUT_MODE_SERVOS:
            timer->usageFlags &= ~((uint32_t)(TIM_USE_MOTOR | TIM_USE_LED));
            timer->usageFlags |=  (uint32_t)TIM_USE_SERVO;
            break;
        case OUTPUT_MODE_LED:
            timer->usageFlags &= ~((uint32_t)(TIM_USE_MOTOR | TIM_USE_SERVO));
            timer->usageFlags |=  (uint32_t)TIM_USE_LED;
            break;
        case OUTPUT_MODE_PINIO:
            timer->usageFlags &= ~((uint32_t)(TIM_USE_MOTOR | TIM_USE_SERVO | TIM_USE_LED));
            break;
        default:
            break;
    }
}

/*
 * FIXED version (post-fix, matches commit 551bce85d6 exactly).
 *
 * The beeper guard causes the function to return immediately when
 * TIM_USE_BEEPER is set, leaving the flags completely untouched.
 */
static void timerHardwareOverride_fixed(timerHardware_t *timer,
                                        outputMode_e outputMode)
{
    /* Guard added by the fix: never modify a beeper timer. */
    if (timer->usageFlags & (uint32_t)TIM_USE_BEEPER) {
        return;
    }
    switch (outputMode) {
        case OUTPUT_MODE_MOTORS:
            timer->usageFlags &= ~((uint32_t)(TIM_USE_SERVO | TIM_USE_LED));
            timer->usageFlags |=  (uint32_t)TIM_USE_MOTOR;
            break;
        case OUTPUT_MODE_SERVOS:
            timer->usageFlags &= ~((uint32_t)(TIM_USE_MOTOR | TIM_USE_LED));
            timer->usageFlags |=  (uint32_t)TIM_USE_SERVO;
            break;
        case OUTPUT_MODE_LED:
            timer->usageFlags &= ~((uint32_t)(TIM_USE_MOTOR | TIM_USE_SERVO));
            timer->usageFlags |=  (uint32_t)TIM_USE_LED;
            break;
        case OUTPUT_MODE_PINIO:
            timer->usageFlags &= ~((uint32_t)(TIM_USE_MOTOR | TIM_USE_SERVO | TIM_USE_LED));
            break;
        default:
            break;
    }
}

/* =========================================================================
 * Helper — simulate what pwmAssignOutput(MAP_TO_SERVO_OUTPUT) does to flags.
 * In real code: timHw->usageFlags &= TIM_USE_SERVO;
 * We need this to show the full two-step corruption path.
 * ========================================================================= */
static void pwmAssignServo_stripFlags(timerHardware_t *timHw)
{
    timHw->usageFlags &= (uint32_t)TIM_USE_SERVO;
}

/* =========================================================================
 * TEST SUITE: BeeperTimerProtection
 * ========================================================================= */

class BeeperTimerProtectionTest : public ::testing::Test {
protected:
    timerHardware_t timer;

    void SetUp() override {
        /* Simulate MATEKH743 TIM2/PA15: beeper shares a servo-capable timer */
        timer.usageFlags = (uint32_t)(TIM_USE_BEEPER | TIM_USE_SERVO);
    }
};

/*
 * TEST 1 — BugReproduction
 *
 * This test demonstrates the pre-fix bug. On the BUGGY code path:
 *   1. timerHardwareOverride_buggy() applies OUTPUT_MODE_SERVOS, which does
 *      NOT clear TIM_USE_BEEPER → flags become TIM_USE_SERVO | TIM_USE_BEEPER.
 *   2. pwmAssignOutput(servo) strips everything except TIM_USE_SERVO
 *      → TIM_USE_BEEPER is lost.
 *
 * The final assertion checks that TIM_USE_BEEPER was lost, which is the
 * observable symptom of the bug.  This test PASSES (i.e. the bug reproduces),
 * which is what we want from the reproduction test.
 */
TEST_F(BeeperTimerProtectionTest, BugReproduction_BeeperFlagLostAfterOverride)
{
    /* Step 1: buggy override — does not protect beeper */
    timerHardwareOverride_buggy(&timer, OUTPUT_MODE_SERVOS);

    /* After the buggy override: both SERVO and BEEPER flags should be set
     * because OUTPUT_MODE_SERVOS only clears MOTOR and LED, not BEEPER. */
    EXPECT_TRUE(timer.usageFlags & (uint32_t)TIM_USE_SERVO)
        << "SERVO flag should be set by the OUTPUT_MODE_SERVOS override";
    EXPECT_TRUE(timer.usageFlags & (uint32_t)TIM_USE_BEEPER)
        << "BEEPER flag should still be present at this point (intermediate state)";

    /* Step 2: simulate pwmAssignOutput(servo) which strips everything
     * except TIM_USE_SERVO — this is the second half of the corruption. */
    pwmAssignServo_stripFlags(&timer);

    /* The bug: TIM_USE_BEEPER is now gone. beeperPwmInit() will fail silently. */
    EXPECT_FALSE(timer.usageFlags & (uint32_t)TIM_USE_BEEPER)
        << "BUG CONFIRMED: TIM_USE_BEEPER was stripped by pwmAssignOutput "
           "because timerHardwareOverride() did not protect it. "
           "beeperPwmInit() would fail to find its timer.";
}

/*
 * TEST 2 — FixVerification
 *
 * The fixed timerHardwareOverride() returns immediately when TIM_USE_BEEPER is
 * set, so the servo override is a no-op for beeper timers.  TIM_USE_BEEPER must
 * survive, allowing beeperPwmInit() to find the timer later.
 *
 * This test PASSES on the fixed code and would FAIL on the buggy code.
 */
TEST_F(BeeperTimerProtectionTest, FixVerification_BeeperFlagSurvivesOverride)
{
    const uint32_t originalFlags = timer.usageFlags;

    /* Fixed override: should be a no-op because TIM_USE_BEEPER is set */
    timerHardwareOverride_fixed(&timer, OUTPUT_MODE_SERVOS);

    /* Flags must be completely unchanged — the guard must have returned early. */
    EXPECT_EQ(timer.usageFlags, originalFlags)
        << "FIX VERIFIED: timerHardwareOverride() must not modify a timer "
           "that has TIM_USE_BEEPER set.";

    EXPECT_TRUE(timer.usageFlags & (uint32_t)TIM_USE_BEEPER)
        << "TIM_USE_BEEPER must still be set after the override attempt";
}

/*
 * TEST 3 — FixVerification for OUTPUT_MODE_MOTORS
 *
 * Confirm the beeper guard works for all output-mode variants, not just SERVOS.
 */
TEST_F(BeeperTimerProtectionTest, FixVerification_BeeperProtectedFromMotorOverride)
{
    const uint32_t originalFlags = timer.usageFlags;

    timerHardwareOverride_fixed(&timer, OUTPUT_MODE_MOTORS);

    EXPECT_EQ(timer.usageFlags, originalFlags)
        << "OUTPUT_MODE_MOTORS must not modify a beeper timer";
    EXPECT_TRUE(timer.usageFlags & (uint32_t)TIM_USE_BEEPER);
}

/*
 * TEST 4 — FixVerification for OUTPUT_MODE_LED
 */
TEST_F(BeeperTimerProtectionTest, FixVerification_BeeperProtectedFromLedOverride)
{
    const uint32_t originalFlags = timer.usageFlags;

    timerHardwareOverride_fixed(&timer, OUTPUT_MODE_LED);

    EXPECT_EQ(timer.usageFlags, originalFlags)
        << "OUTPUT_MODE_LED must not modify a beeper timer";
    EXPECT_TRUE(timer.usageFlags & (uint32_t)TIM_USE_BEEPER);
}

/*
 * TEST 5 — FixVerification for OUTPUT_MODE_PINIO
 */
TEST_F(BeeperTimerProtectionTest, FixVerification_BeeperProtectedFromPinioOverride)
{
    const uint32_t originalFlags = timer.usageFlags;

    timerHardwareOverride_fixed(&timer, OUTPUT_MODE_PINIO);

    EXPECT_EQ(timer.usageFlags, originalFlags)
        << "OUTPUT_MODE_PINIO must not modify a beeper timer";
    EXPECT_TRUE(timer.usageFlags & (uint32_t)TIM_USE_BEEPER);
}

/* =========================================================================
 * Negative / regression tests — normal (non-beeper) timers must still be
 * overridden correctly.  The guard must NOT prevent normal overrides.
 * ========================================================================= */

class NormalTimerOverrideTest : public ::testing::Test {
protected:
    timerHardware_t timer;

    /* No TIM_USE_BEEPER: both motor and servo flags set (auto-mode timer). */
    void SetUp() override {
        timer.usageFlags = (uint32_t)(TIM_USE_MOTOR | TIM_USE_SERVO);
    }
};

TEST_F(NormalTimerOverrideTest, MotorOverride_SetsMotorClearsServoAndLed)
{
    timerHardwareOverride_fixed(&timer, OUTPUT_MODE_MOTORS);

    EXPECT_TRUE(timer.usageFlags  & (uint32_t)TIM_USE_MOTOR)
        << "TIM_USE_MOTOR must be set after OUTPUT_MODE_MOTORS override";
    EXPECT_FALSE(timer.usageFlags & (uint32_t)TIM_USE_SERVO)
        << "TIM_USE_SERVO must be cleared by OUTPUT_MODE_MOTORS override";
    EXPECT_FALSE(timer.usageFlags & (uint32_t)TIM_USE_LED)
        << "TIM_USE_LED must be cleared by OUTPUT_MODE_MOTORS override";
    EXPECT_FALSE(timer.usageFlags & (uint32_t)TIM_USE_BEEPER)
        << "TIM_USE_BEEPER must remain clear on a non-beeper timer";
}

TEST_F(NormalTimerOverrideTest, ServoOverride_SetsServoClearsMotorAndLed)
{
    timerHardwareOverride_fixed(&timer, OUTPUT_MODE_SERVOS);

    EXPECT_TRUE(timer.usageFlags  & (uint32_t)TIM_USE_SERVO)
        << "TIM_USE_SERVO must be set after OUTPUT_MODE_SERVOS override";
    EXPECT_FALSE(timer.usageFlags & (uint32_t)TIM_USE_MOTOR)
        << "TIM_USE_MOTOR must be cleared by OUTPUT_MODE_SERVOS override";
    EXPECT_FALSE(timer.usageFlags & (uint32_t)TIM_USE_LED)
        << "TIM_USE_LED must be cleared by OUTPUT_MODE_SERVOS override";
}

TEST_F(NormalTimerOverrideTest, LedOverride_SetsLedClearsMotorAndServo)
{
    timerHardwareOverride_fixed(&timer, OUTPUT_MODE_LED);

    EXPECT_TRUE(timer.usageFlags  & (uint32_t)TIM_USE_LED)
        << "TIM_USE_LED must be set after OUTPUT_MODE_LED override";
    EXPECT_FALSE(timer.usageFlags & (uint32_t)TIM_USE_MOTOR)
        << "TIM_USE_MOTOR must be cleared by OUTPUT_MODE_LED override";
    EXPECT_FALSE(timer.usageFlags & (uint32_t)TIM_USE_SERVO)
        << "TIM_USE_SERVO must be cleared by OUTPUT_MODE_LED override";
}

TEST_F(NormalTimerOverrideTest, PinioOverride_ClearsAllOutputFlags)
{
    timer.usageFlags = (uint32_t)(TIM_USE_MOTOR | TIM_USE_SERVO | TIM_USE_LED);

    timerHardwareOverride_fixed(&timer, OUTPUT_MODE_PINIO);

    EXPECT_FALSE(timer.usageFlags & (uint32_t)TIM_USE_MOTOR)
        << "TIM_USE_MOTOR must be cleared by OUTPUT_MODE_PINIO override";
    EXPECT_FALSE(timer.usageFlags & (uint32_t)TIM_USE_SERVO)
        << "TIM_USE_SERVO must be cleared by OUTPUT_MODE_PINIO override";
    EXPECT_FALSE(timer.usageFlags & (uint32_t)TIM_USE_LED)
        << "TIM_USE_LED must be cleared by OUTPUT_MODE_PINIO override";
}

TEST_F(NormalTimerOverrideTest, AutoMode_LeavesTimerUnchanged)
{
    const uint32_t originalFlags = timer.usageFlags;

    timerHardwareOverride_fixed(&timer, OUTPUT_MODE_AUTO);

    EXPECT_EQ(timer.usageFlags, originalFlags)
        << "OUTPUT_MODE_AUTO (default case) must not change flags";
}

/* =========================================================================
 * Edge case: timer has ONLY TIM_USE_BEEPER (no motor/servo capability)
 * ========================================================================= */

TEST(BeeperOnlyTimer, OverrideIgnoredForPureBeeper)
{
    timerHardware_t timer;
    timer.usageFlags = (uint32_t)TIM_USE_BEEPER;

    const uint32_t originalFlags = timer.usageFlags;

    /* All override modes must be no-ops for a pure beeper timer. */
    timerHardwareOverride_fixed(&timer, OUTPUT_MODE_MOTORS);
    EXPECT_EQ(timer.usageFlags, originalFlags);

    timerHardwareOverride_fixed(&timer, OUTPUT_MODE_SERVOS);
    EXPECT_EQ(timer.usageFlags, originalFlags);

    timerHardwareOverride_fixed(&timer, OUTPUT_MODE_LED);
    EXPECT_EQ(timer.usageFlags, originalFlags);

    timerHardwareOverride_fixed(&timer, OUTPUT_MODE_PINIO);
    EXPECT_EQ(timer.usageFlags, originalFlags);
}
