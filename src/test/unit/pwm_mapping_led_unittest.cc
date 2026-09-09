/*
 * Unit test: timerHardwareOverride() must not falsely flag sibling timer
 * channels as TIM_USE_LED when OUTPUT_MODE_LED is applied to a shared timer.
 *
 * Bug (pre-fix, same bug class as the BEEPER bug fixed by commits
 * 8c16aeed85 / 31bb99ea0b, see pwm_mapping_beeper_unittest.cc):
 *   timerHardwareOverride() applied TIM_USE_LED to every pad sharing a timer
 *   set to OUTPUT_MODE_LED, but the WS2811 LED strip driver
 *   (light_ws2811strip.c) only ever wires up the FIRST matching pad, found
 *   via timerGetByUsageFlag(TIM_USE_LED). All channels of one timer share a
 *   single period register (fixed at the WS2811 bit rate), so only one pad
 *   can actually drive the LED strip. This caused sibling pads on the same
 *   timer to be falsely reported/claimed as LED outputs while being
 *   functionally dead (or worse, stealing a pad that could have been used
 *   for something else).
 *
 * Fix (mirrors the BEEPER fix exactly):
 *   timerHardwareOverride() now takes an `isCanonicalLedPad` bool. The
 *   OUTPUT_MODE_LED case always clears TIM_USE_MOTOR|TIM_USE_SERVO|
 *   TIM_USE_BEEPER|TIM_USE_LED, and sets TIM_USE_PINIO as a GPIO-only
 *   fallback (binary on/off, no PWM — doesn't need the timer's fixed
 *   WS2811-bitrate period). Only if isCanonicalLedPad is true (i.e. this is
 *   the first timerHardware[] entry, in ascending scan order, seen for this
 *   physical timer with outputMode == OUTPUT_MODE_LED) does it clear
 *   TIM_USE_PINIO and set TIM_USE_LED instead.
 *
 *   pwmBuildTimerOutputList() computes isCanonicalLedPad via a
 *   `ledClaimed[HARDWARE_TIMER_DEFINITION_COUNT]` array, parallel to the
 *   existing `beeperClaimed[]` array used for OUTPUT_MODE_BEEPER.
 *
 * This file contains:
 *   1. A minimal inline reproduction of both the BUGGY and FIXED versions of
 *      timerHardwareOverride() (LED case only, plus MOTORS/SERVOS/BEEPER/
 *      PINIO for regression coverage) so the test is fully self-contained.
 *      pwm_mapping.c is excluded from the SITL/unit build by
 *      `#if !defined(SITL_BUILD)`, and the real function depends on PG
 *      (parameter-group) infrastructure not available in a host unit build.
 *   2. TEST BugReproduction — demonstrates that the old, unconditional-LED
 *      code gives TIM_USE_LED to every pad sharing an LED-mode timer.
 *   3. TEST FixVerification — only the canonical (first) pad on a shared
 *      timer gets TIM_USE_LED; sibling pads fall back to TIM_USE_PINIO.
 *   4. Multi-pad (3 siblings) test — only the first scanned pad is canonical.
 *   5. Regression tests for MOTORS/SERVOS/BEEPER/PINIO overrides on the
 *      fixed function, including timers mixed with LED-mode timers.
 *   6. TEST SUITE SourceSync — reads the ACTUAL LIVE
 *      src/main/drivers/pwm_mapping.c off disk at test time and asserts the
 *      real timerHardwareOverride() source text still matches the logic the
 *      inline reproduction above assumes. This exists specifically because
 *      the sibling pwm_mapping_beeper_unittest.cc was found to have drifted:
 *      it kept passing for months after commits 8c16aeed85/31bb99ea0b
 *      changed the real BEEPER logic, because it only ever exercised its own
 *      frozen, hand-copied logic and never looked at the live file again.
 *      An inline reproduction is unavoidable here (see point 1), but it
 *      should not be allowed to silently go stale — if pwm_mapping.c's real
 *      OUTPUT_MODE_LED case (or the canonical-pad tracking loop in
 *      pwmBuildTimerOutputList()) changes without this file being updated to
 *      match, SourceSync must fail loudly instead of the rest of the suite
 *      quietly passing against dead logic.
 */

#include <stdint.h>
#include <stdbool.h>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>

#include "gtest/gtest.h"
#include "unittest_macros.h"

/* -------------------------------------------------------------------------
 * Minimal type/flag definitions — mirrors the real firmware headers on this
 * branch (src/main/drivers/timer.h, src/main/flight/mixer.h). Unlike
 * release/9.1, this branch has TIM_USE_PINIO / TIM_USE_BEEPER and
 * OUTPUT_MODE_PINIO / OUTPUT_MODE_BEEPER.
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
    TIM_USE_PINIO   = (1 << 26),
} timerUsageFlag_e;

/* outputMode_e — must match src/main/flight/mixer.h exactly */
typedef enum {
    OUTPUT_MODE_AUTO    = 0,
    OUTPUT_MODE_MOTORS,
    OUTPUT_MODE_SERVOS,
    OUTPUT_MODE_LED,
    OUTPUT_MODE_PINIO,
    OUTPUT_MODE_BEEPER
} outputMode_e;

/* Minimal timer hardware entry — only the fields exercised by the function.
 * `timId` stands in for the real code's timer2id(timer->tim) — in the real
 * driver, multiple timerHardware[] pads sharing the same physical timer
 * compare equal on timer2id(); here we just store the id directly. */
typedef struct timerHardware_s {
    uint32_t usageFlags;
    uint8_t timId;
} timerHardware_t;

/* -------------------------------------------------------------------------
 * Inline reproductions of timerHardwareOverride().
 *
 * We inline these rather than linking pwm_mapping.c because:
 *  a) pwm_mapping.c is wrapped in #if !defined(SITL_BUILD) and the unit-test
 *     target.h defines SITL_BUILD, so the real file compiles to nothing.
 *  b) The function depends on timerOverrides() (parameter-group accessor) and
 *     timer2id(), which are platform / PG infrastructure that is not present
 *     in a host unit-test build.
 *
 * The functions below are literal translations of the current C source
 * (post BEEPER-fix, post LED-fix) with the PG indirection collapsed into a
 * simple `outputMode` parameter, and canonical-pad tracking passed in
 * explicitly as bool parameters (mirroring isCanonicalBeeperPad /
 * isCanonicalLedPad computed by pwmBuildTimerOutputList()).
 *
 * See the SourceSync test suite at the bottom of this file for a mechanism
 * that catches this reproduction going out of sync with the real source.
 * ------------------------------------------------------------------------- */

/*
 * BUGGY version — reproduces the pre-fix LED behaviour: OUTPUT_MODE_LED
 * unconditionally sets TIM_USE_LED on every pad sharing the timer, with no
 * canonical-pad tracking at all (same bug class as the pre-fix BEEPER code
 * in commit history before 8c16aeed85). MOTORS/SERVOS/PINIO/BEEPER cases are
 * included only for completeness / contrast; this reproduction focuses on
 * the LED bug.
 */
static void timerHardwareOverride_buggy(timerHardware_t *timer,
                                         outputMode_e outputMode)
{
    switch (outputMode) {
        case OUTPUT_MODE_MOTORS:
            timer->usageFlags &= ~(uint32_t)(TIM_USE_SERVO | TIM_USE_LED | TIM_USE_BEEPER);
            timer->usageFlags |= (uint32_t)TIM_USE_MOTOR;
            break;
        case OUTPUT_MODE_SERVOS:
            timer->usageFlags &= ~(uint32_t)(TIM_USE_MOTOR | TIM_USE_LED | TIM_USE_BEEPER);
            timer->usageFlags |= (uint32_t)TIM_USE_SERVO;
            break;
        case OUTPUT_MODE_LED:
            /* BUG: unconditionally sets TIM_USE_LED on every pad of this
             * timer, even though only one pad is actually wired up by
             * ws2811LedStripInit(). No canonical-pad tracking whatsoever. */
            timer->usageFlags &= ~(uint32_t)(TIM_USE_MOTOR | TIM_USE_SERVO | TIM_USE_BEEPER);
            timer->usageFlags |= (uint32_t)TIM_USE_LED;
            break;
        case OUTPUT_MODE_PINIO:
            timer->usageFlags &= ~(uint32_t)(TIM_USE_MOTOR | TIM_USE_SERVO | TIM_USE_LED | TIM_USE_BEEPER);
            timer->usageFlags |= (uint32_t)TIM_USE_PINIO;
            break;
        case OUTPUT_MODE_BEEPER:
            timer->usageFlags &= ~(uint32_t)(TIM_USE_MOTOR | TIM_USE_SERVO | TIM_USE_LED);
            timer->usageFlags |= (uint32_t)TIM_USE_BEEPER;
            break;
        default:
            break;
    }
}

/*
 * FIXED version — literal translation of the current
 * src/main/drivers/pwm_mapping.c timerHardwareOverride() on this branch.
 * Kept in sync with the real source by the SourceSync tests below.
 */
static void timerHardwareOverride_fixed(timerHardware_t *timer,
                                         outputMode_e outputMode,
                                         bool isCanonicalBeeperPad,
                                         bool isCanonicalLedPad)
{
    switch (outputMode) {
        case OUTPUT_MODE_MOTORS:
            timer->usageFlags &= ~(uint32_t)(TIM_USE_SERVO | TIM_USE_LED | TIM_USE_BEEPER);
            timer->usageFlags |= (uint32_t)TIM_USE_MOTOR;
            break;
        case OUTPUT_MODE_SERVOS:
            timer->usageFlags &= ~(uint32_t)(TIM_USE_MOTOR | TIM_USE_LED | TIM_USE_BEEPER);
            timer->usageFlags |= (uint32_t)TIM_USE_SERVO;
            break;
        case OUTPUT_MODE_LED:
            // Other channels of this timer share its period register, so they can't
            // be repurposed as motor/servo/BEEPER either. Only the first (canonical)
            // pad is the one ws2811LedStripInit() actually wires up; siblings can still
            // drive a GPIO-only PINIO (binary on/off, no PWM), since that doesn't need
            // the timer's fixed WS2811-bitrate period.
            timer->usageFlags &= ~(uint32_t)(TIM_USE_MOTOR | TIM_USE_SERVO | TIM_USE_BEEPER | TIM_USE_LED);
            timer->usageFlags |= (uint32_t)TIM_USE_PINIO;
            if (isCanonicalLedPad) {
                timer->usageFlags &= ~(uint32_t)TIM_USE_PINIO;
                timer->usageFlags |= (uint32_t)TIM_USE_LED;
            }
            break;
        case OUTPUT_MODE_PINIO:
            timer->usageFlags &= ~(uint32_t)(TIM_USE_MOTOR | TIM_USE_SERVO | TIM_USE_LED | TIM_USE_BEEPER);
            timer->usageFlags |= (uint32_t)TIM_USE_PINIO;
            break;
        case OUTPUT_MODE_BEEPER:
            // Other channels of this timer share its period register, so they can't
            // be repurposed as motor/servo/LED either. They can still drive a GPIO-only
            // PINIO (binary on/off, no PWM), since that doesn't need the timer's period.
            timer->usageFlags &= ~(uint32_t)(TIM_USE_MOTOR | TIM_USE_SERVO | TIM_USE_LED | TIM_USE_BEEPER);
            timer->usageFlags |= (uint32_t)TIM_USE_PINIO;
            if (isCanonicalBeeperPad) {
                timer->usageFlags &= ~(uint32_t)TIM_USE_PINIO;
                timer->usageFlags |= (uint32_t)TIM_USE_BEEPER;
            }
            break;
        default:
            break;
    }
}

/*
 * Helper — reproduces the canonical-pad-tracking loop in
 * pwmBuildTimerOutputList(): scans pads in ascending array order and applies
 * timerHardwareOverride_fixed() with isCanonicalBeeperPad / isCanonicalLedPad
 * computed exactly as the real code does (first pad seen per physical timer,
 * for a timer whose overridden outputMode matches, wins canonical status).
 */
static void applyFixedOverridesToArray(timerHardware_t *pads, int count,
                                        const outputMode_e *outputModeByPad,
                                        int maxTimId)
{
    bool beeperClaimed[32] = { false };
    bool ledClaimed[32] = { false };
    ASSERT_LT(maxTimId, 32);

    for (int idx = 0; idx < count; idx++) {
        timerHardware_t *timHw = &pads[idx];
        uint8_t timId = timHw->timId;
        outputMode_e mode = outputModeByPad[idx];

        bool isCanonicalBeeperPad = false;
        if (mode == OUTPUT_MODE_BEEPER && !beeperClaimed[timId]) {
            beeperClaimed[timId] = true;
            isCanonicalBeeperPad = true;
        }
        bool isCanonicalLedPad = false;
        if (mode == OUTPUT_MODE_LED && !ledClaimed[timId]) {
            ledClaimed[timId] = true;
            isCanonicalLedPad = true;
        }
        timerHardwareOverride_fixed(timHw, mode, isCanonicalBeeperPad, isCanonicalLedPad);
    }
}

/* =========================================================================
 * TEST SUITE: LedTimerSiblingPads
 * ========================================================================= */

/*
 * TEST 1 — BugReproduction
 *
 * Two pads share timer id 5, both overridden to OUTPUT_MODE_LED. On the
 * BUGGY code path (no canonical-pad tracking at all), BOTH pads get
 * TIM_USE_LED, even though only one of them will ever actually be wired up
 * by ws2811LedStripInit()/timerGetByUsageFlag(TIM_USE_LED). This test
 * PASSES (i.e. the bug reproduces) on the buggy code — confirming the
 * observable symptom of the bug.
 */
TEST(LedTimerSiblingPads, BugReproduction_BothSiblingsGetLedFlag)
{
    timerHardware_t padA; padA.usageFlags = 0; padA.timId = 5;
    timerHardware_t padB; padB.usageFlags = 0; padB.timId = 5;

    timerHardwareOverride_buggy(&padA, OUTPUT_MODE_LED);
    timerHardwareOverride_buggy(&padB, OUTPUT_MODE_LED);

    EXPECT_TRUE(padA.usageFlags & (uint32_t)TIM_USE_LED)
        << "First pad should get TIM_USE_LED (expected)";
    EXPECT_TRUE(padB.usageFlags & (uint32_t)TIM_USE_LED)
        << "BUG CONFIRMED: sibling pad on the SAME timer also got TIM_USE_LED, "
           "but light_ws2811strip.c only wires up the first pad found via "
           "timerGetByUsageFlag(TIM_USE_LED). This sibling would be falsely "
           "reported as an LED output while being functionally dead.";
}

/*
 * TEST 2 — FixVerification (two sibling pads)
 *
 * Same scenario as above, but using the FIXED function with proper
 * canonical-pad computation (as pwmBuildTimerOutputList() does). Only the
 * first-scanned pad on the shared timer should get TIM_USE_LED; the sibling
 * must fall back to TIM_USE_PINIO (GPIO-only), NOT TIM_USE_LED, and NOT be
 * left flagless (this branch has the PINIO fallback, unlike release/9.1).
 */
TEST(LedTimerSiblingPads, FixVerification_OnlyCanonicalPadGetsLed)
{
    timerHardware_t pads[2];
    pads[0].usageFlags = 0; pads[0].timId = 5;
    pads[1].usageFlags = 0; pads[1].timId = 5;
    outputMode_e modes[2] = { OUTPUT_MODE_LED, OUTPUT_MODE_LED };

    applyFixedOverridesToArray(pads, 2, modes, 6);

    EXPECT_TRUE(pads[0].usageFlags & (uint32_t)TIM_USE_LED)
        << "Canonical (first-scanned) pad must get TIM_USE_LED";
    EXPECT_FALSE(pads[0].usageFlags & (uint32_t)TIM_USE_PINIO)
        << "Canonical pad must not also carry the PINIO fallback flag";

    EXPECT_FALSE(pads[1].usageFlags & (uint32_t)TIM_USE_LED)
        << "FIX VERIFIED: sibling pad on the same timer must NOT get TIM_USE_LED";
    EXPECT_TRUE(pads[1].usageFlags & (uint32_t)TIM_USE_PINIO)
        << "Sibling pad must fall back to TIM_USE_PINIO (GPIO-only), not be left flagless";
}

/*
 * TEST 3 — Three-pad case: only the first scanned pad is canonical
 *
 * Confirms the canonical tracking generalizes beyond two pads: with three
 * pads sharing a timer, only pads[0] should be canonical; pads[1] and
 * pads[2] must both fall back to PINIO.
 */
TEST(LedTimerSiblingPads, ThreePads_OnlyFirstScannedIsCanonical)
{
    timerHardware_t pads[3];
    for (int i = 0; i < 3; i++) {
        pads[i].usageFlags = 0;
        pads[i].timId = 7;
    }
    outputMode_e modes[3] = { OUTPUT_MODE_LED, OUTPUT_MODE_LED, OUTPUT_MODE_LED };

    applyFixedOverridesToArray(pads, 3, modes, 8);

    EXPECT_TRUE(pads[0].usageFlags & (uint32_t)TIM_USE_LED)
        << "pads[0] (first scanned) must be canonical and get TIM_USE_LED";
    EXPECT_FALSE(pads[0].usageFlags & (uint32_t)TIM_USE_PINIO);

    for (int i = 1; i < 3; i++) {
        EXPECT_FALSE(pads[i].usageFlags & (uint32_t)TIM_USE_LED)
            << "pads[" << i << "] must NOT be canonical, so must not get TIM_USE_LED";
        EXPECT_TRUE(pads[i].usageFlags & (uint32_t)TIM_USE_PINIO)
            << "pads[" << i << "] must fall back to TIM_USE_PINIO";
    }
}

/*
 * TEST 4 — Two independent LED timers each get their own canonical pad
 *
 * Pads on timer id 5 and pads on timer id 9 are independent: each timer
 * should get exactly one canonical (LED) pad, tracked separately via the
 * per-timer ledClaimed[] array.
 */
TEST(LedTimerSiblingPads, IndependentTimers_EachGetsOwnCanonicalPad)
{
    timerHardware_t pads[4];
    pads[0].usageFlags = 0; pads[0].timId = 5; // timer A, pad 1 -> canonical
    pads[1].usageFlags = 0; pads[1].timId = 5; // timer A, pad 2 -> sibling
    pads[2].usageFlags = 0; pads[2].timId = 9; // timer B, pad 1 -> canonical
    pads[3].usageFlags = 0; pads[3].timId = 9; // timer B, pad 2 -> sibling
    outputMode_e modes[4] = { OUTPUT_MODE_LED, OUTPUT_MODE_LED, OUTPUT_MODE_LED, OUTPUT_MODE_LED };

    applyFixedOverridesToArray(pads, 4, modes, 10);

    EXPECT_TRUE(pads[0].usageFlags & (uint32_t)TIM_USE_LED);
    EXPECT_TRUE(pads[1].usageFlags & (uint32_t)TIM_USE_PINIO);
    EXPECT_FALSE(pads[1].usageFlags & (uint32_t)TIM_USE_LED);

    EXPECT_TRUE(pads[2].usageFlags & (uint32_t)TIM_USE_LED)
        << "Timer B's first pad must independently become canonical";
    EXPECT_TRUE(pads[3].usageFlags & (uint32_t)TIM_USE_PINIO);
    EXPECT_FALSE(pads[3].usageFlags & (uint32_t)TIM_USE_LED);
}

/* =========================================================================
 * Negative / regression tests — MOTORS/SERVOS/BEEPER overrides must still
 * work correctly with the fixed function, including when mixed with an
 * LED-mode timer in the same scan.
 * ========================================================================= */

TEST(LedTimerRegression, MotorOverride_Unaffected)
{
    timerHardware_t timer;
    timer.usageFlags = (uint32_t)(TIM_USE_MOTOR | TIM_USE_SERVO);
    timer.timId = 1;

    timerHardwareOverride_fixed(&timer, OUTPUT_MODE_MOTORS,
                                 /*isCanonicalBeeperPad=*/false,
                                 /*isCanonicalLedPad=*/false);

    EXPECT_TRUE(timer.usageFlags & (uint32_t)TIM_USE_MOTOR);
    EXPECT_FALSE(timer.usageFlags & (uint32_t)TIM_USE_SERVO);
    EXPECT_FALSE(timer.usageFlags & (uint32_t)TIM_USE_LED);
    EXPECT_FALSE(timer.usageFlags & (uint32_t)TIM_USE_BEEPER);
}

TEST(LedTimerRegression, ServoOverride_Unaffected)
{
    timerHardware_t timer;
    timer.usageFlags = (uint32_t)(TIM_USE_MOTOR | TIM_USE_SERVO);
    timer.timId = 2;

    timerHardwareOverride_fixed(&timer, OUTPUT_MODE_SERVOS,
                                 /*isCanonicalBeeperPad=*/false,
                                 /*isCanonicalLedPad=*/false);

    EXPECT_TRUE(timer.usageFlags & (uint32_t)TIM_USE_SERVO);
    EXPECT_FALSE(timer.usageFlags & (uint32_t)TIM_USE_MOTOR);
    EXPECT_FALSE(timer.usageFlags & (uint32_t)TIM_USE_LED);
}

TEST(LedTimerRegression, BeeperCanonicalPad_StillGetsBeeperFlag)
{
    /* Regression: the BEEPER canonical-pad logic (isCanonicalBeeperPad) must
     * be completely unaffected by the addition of isCanonicalLedPad. */
    timerHardware_t timer;
    timer.usageFlags = 0;
    timer.timId = 3;

    timerHardwareOverride_fixed(&timer, OUTPUT_MODE_BEEPER,
                                 /*isCanonicalBeeperPad=*/true,
                                 /*isCanonicalLedPad=*/false);

    EXPECT_TRUE(timer.usageFlags & (uint32_t)TIM_USE_BEEPER);
    EXPECT_FALSE(timer.usageFlags & (uint32_t)TIM_USE_PINIO);
}

TEST(LedTimerRegression, BeeperSiblingPad_FallsBackToPinio)
{
    timerHardware_t timer;
    timer.usageFlags = 0;
    timer.timId = 3;

    timerHardwareOverride_fixed(&timer, OUTPUT_MODE_BEEPER,
                                 /*isCanonicalBeeperPad=*/false,
                                 /*isCanonicalLedPad=*/false);

    EXPECT_FALSE(timer.usageFlags & (uint32_t)TIM_USE_BEEPER);
    EXPECT_TRUE(timer.usageFlags & (uint32_t)TIM_USE_PINIO)
        << "Non-canonical BEEPER pad must fall back to PINIO (regression check "
           "for the pre-existing BEEPER fix, unaffected by the LED fix)";
}

TEST(LedTimerRegression, PinioOverride_Unaffected)
{
    timerHardware_t timer;
    timer.usageFlags = (uint32_t)(TIM_USE_MOTOR | TIM_USE_SERVO | TIM_USE_LED);
    timer.timId = 4;

    timerHardwareOverride_fixed(&timer, OUTPUT_MODE_PINIO,
                                 /*isCanonicalBeeperPad=*/false,
                                 /*isCanonicalLedPad=*/false);

    EXPECT_FALSE(timer.usageFlags & (uint32_t)TIM_USE_MOTOR);
    EXPECT_FALSE(timer.usageFlags & (uint32_t)TIM_USE_SERVO);
    EXPECT_FALSE(timer.usageFlags & (uint32_t)TIM_USE_LED);
    EXPECT_TRUE(timer.usageFlags & (uint32_t)TIM_USE_PINIO);
}

TEST(LedTimerRegression, AutoMode_LeavesTimerUnchanged)
{
    timerHardware_t timer;
    timer.usageFlags = (uint32_t)(TIM_USE_MOTOR | TIM_USE_SERVO);
    timer.timId = 6;
    const uint32_t originalFlags = timer.usageFlags;

    timerHardwareOverride_fixed(&timer, OUTPUT_MODE_AUTO,
                                 /*isCanonicalBeeperPad=*/false,
                                 /*isCanonicalLedPad=*/false);

    EXPECT_EQ(timer.usageFlags, originalFlags)
        << "OUTPUT_MODE_AUTO (default case) must not change flags";
}

/*
 * TEST — Mixed scan: MOTOR timer, LED timer (2 pads), BEEPER timer (canonical
 * pad only) all processed together via applyFixedOverridesToArray(), matching
 * a realistic pwmBuildTimerOutputList() scenario where different timers on
 * the board have different output-mode overrides. Confirms none of the
 * per-timer canonical tracking (ledClaimed[] vs beeperClaimed[]) interferes
 * across timer ids or across output modes.
 */
TEST(LedTimerRegression, MixedScan_MotorLedBeeperTimersDoNotInterfere)
{
    timerHardware_t pads[5];
    for (int i = 0; i < 5; i++) pads[i].usageFlags = 0;

    pads[0].timId = 1; // MOTOR timer
    pads[1].timId = 2; // LED timer, canonical
    pads[2].timId = 2; // LED timer, sibling
    pads[3].timId = 3; // BEEPER timer, canonical
    pads[4].timId = 3; // BEEPER timer, sibling

    outputMode_e modes[5] = {
        OUTPUT_MODE_MOTORS,
        OUTPUT_MODE_LED,
        OUTPUT_MODE_LED,
        OUTPUT_MODE_BEEPER,
        OUTPUT_MODE_BEEPER,
    };

    applyFixedOverridesToArray(pads, 5, modes, 4);

    EXPECT_TRUE(pads[0].usageFlags & (uint32_t)TIM_USE_MOTOR);

    EXPECT_TRUE(pads[1].usageFlags & (uint32_t)TIM_USE_LED);
    EXPECT_FALSE(pads[1].usageFlags & (uint32_t)TIM_USE_PINIO);

    EXPECT_FALSE(pads[2].usageFlags & (uint32_t)TIM_USE_LED);
    EXPECT_TRUE(pads[2].usageFlags & (uint32_t)TIM_USE_PINIO);

    EXPECT_TRUE(pads[3].usageFlags & (uint32_t)TIM_USE_BEEPER);
    EXPECT_FALSE(pads[3].usageFlags & (uint32_t)TIM_USE_PINIO);

    EXPECT_FALSE(pads[4].usageFlags & (uint32_t)TIM_USE_BEEPER);
    EXPECT_TRUE(pads[4].usageFlags & (uint32_t)TIM_USE_PINIO);
}

/* =========================================================================
 * TEST SUITE: SourceSync
 *
 * Reads the ACTUAL LIVE src/main/drivers/pwm_mapping.c off disk (not a
 * cached copy, not something computed at CMake-configure time) and checks
 * that specific, logic-relevant fragments of the real timerHardwareOverride()
 * and pwmBuildTimerOutputList() are still present verbatim (modulo
 * whitespace and comments). If a future change to pwm_mapping.c alters this
 * logic without updating the inline reproduction above, these tests fail —
 * loudly, in `make check` / CI — instead of the rest of the file silently
 * continuing to pass against a stale copy.
 *
 * This does not (and structurally cannot, given the SITL_BUILD guard around
 * the entire file — see block comment above) substitute for actually linking
 * and executing the real function. It is a drift-detector, not a full
 * integration test. Comments are stripped and whitespace is collapsed before
 * comparison so that pure reformatting/typo-fixes in comments do not cause
 * false failures; only the executable-statement tokens are checked.
 * ========================================================================= */

namespace {

// Strips "//...\n" line comments. Deliberately does not handle /* */ block
// comments or comments inside string/char literals — pwm_mapping.c does not
// use either within the functions we check, and keeping this simple avoids
// this drift-detector itself becoming a source of drift.
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

// Collapses any run of whitespace (including newlines) to a single space,
// and trims leading/trailing whitespace, so indentation/line-wrap changes
// don't cause false failures.
std::string normalizeWhitespace(const std::string &text)
{
    std::string result;
    result.reserve(text.size());
    bool lastWasSpace = true; // trims leading whitespace
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

// Locates the live pwm_mapping.c relative to this test file's own path.
// __FILE__ is emitted as an absolute path by this project's CMake/Makefiles
// (verified: `c++ ... -c /abs/path/to/pwm_mapping_led_unittest.cc`), so this
// resolves correctly regardless of the build directory or CWD the test is
// invoked from.
std::string pwmMappingSourcePath()
{
    std::string thisFile = __FILE__; // .../src/test/unit/pwm_mapping_led_unittest.cc
    size_t lastSlash = thisFile.find_last_of("/\\");
    std::string testUnitDir = (lastSlash == std::string::npos) ? "." : thisFile.substr(0, lastSlash);
    return testUnitDir + "/../../main/drivers/pwm_mapping.c";
}

// Reads and normalizes pwm_mapping.c. Fails the calling test LOUDLY (not
// silently returning empty / skipping) if the file can't be found, since a
// missing file here means this drift-detector isn't detecting anything.
::testing::AssertionResult loadNormalizedPwmMappingSource(std::string *out)
{
    const std::string path = pwmMappingSourcePath();
    std::ifstream file(path);
    if (!file.is_open()) {
        return ::testing::AssertionFailure()
            << "Could not open live source file at computed path: " << path
            << " -- SourceSync tests cannot verify anything against dead/absent "
               "source. Check that src/test/unit/ and src/main/drivers/ still "
               "have their expected relative layout.";
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    if (buffer.str().empty()) {
        return ::testing::AssertionFailure()
            << "Live source file at " << path << " opened but read as empty.";
    }
    *out = normalizeSource(buffer.str());
    return ::testing::AssertionSuccess();
}

} // namespace

TEST(SourceSync, TimerHardwareOverrideSignatureMatchesLiveSource)
{
    std::string normalizedSource;
    ASSERT_TRUE(loadNormalizedPwmMappingSource(&normalizedSource));

    const std::string expectedSignature = normalizeSource(
        "static void timerHardwareOverride(timerHardware_t * timer, "
        "bool isCanonicalBeeperPad, bool isCanonicalLedPad) {");

    EXPECT_NE(normalizedSource.find(expectedSignature), std::string::npos)
        << "timerHardwareOverride()'s signature in the LIVE "
           "src/main/drivers/pwm_mapping.c no longer matches what this test's "
           "inline reproduction assumes. If the signature legitimately "
           "changed, update timerHardwareOverride_fixed()/_buggy() in "
           "pwm_mapping_led_unittest.cc (and pwm_mapping_beeper_unittest.cc) "
           "to match, then update this expected string too.";
}

TEST(SourceSync, LedCaseMatchesLiveSource)
{
    std::string normalizedSource;
    ASSERT_TRUE(loadNormalizedPwmMappingSource(&normalizedSource));

    // Comments intentionally omitted here (stripLineComments() removes them
    // from the live source before comparison too) so that comment wording
    // changes don't cause false failures; only the executable statements are
    // checked, which is exactly the logic the FixVerification tests above
    // depend on.
    const std::string expectedLedCase = normalizeSource(
        "case OUTPUT_MODE_LED: "
        "timer->usageFlags &= ~(TIM_USE_MOTOR|TIM_USE_SERVO|TIM_USE_BEEPER|TIM_USE_LED); "
        "timer->usageFlags |= TIM_USE_PINIO; "
        "if (isCanonicalLedPad) { "
        "timer->usageFlags &= ~TIM_USE_PINIO; "
        "timer->usageFlags |= TIM_USE_LED; "
        "} "
        "break;");

    EXPECT_NE(normalizedSource.find(expectedLedCase), std::string::npos)
        << "The OUTPUT_MODE_LED case in the LIVE src/main/drivers/pwm_mapping.c "
           "no longer matches timerHardwareOverride_fixed()'s LED case in this "
           "test file. This is exactly the kind of drift that let "
           "pwm_mapping_beeper_unittest.cc silently stop verifying real "
           "behavior after commits 8c16aeed85/31bb99ea0b. Update the inline "
           "reproduction (and the FixVerification/BugReproduction tests, if "
           "the fix strategy itself changed) to match the live source.";
}

TEST(SourceSync, CanonicalPadTrackingLoopMatchesLiveSource)
{
    std::string normalizedSource;
    ASSERT_TRUE(loadNormalizedPwmMappingSource(&normalizedSource));

    const std::string expectedLoop = normalizeSource(
        "bool beeperClaimed[HARDWARE_TIMER_DEFINITION_COUNT] = { false }; "
        "bool ledClaimed[HARDWARE_TIMER_DEFINITION_COUNT] = { false }; "
        "for (int idx = 0; idx < timerHardwareCount; idx++) { "
        "timerHardware_t *timHw = &timerHardware[idx]; "
        "uint8_t timId = timer2id(timHw->tim); "
        "bool isCanonicalBeeperPad = false; "
        "if (timerOverrides(timId)->outputMode == OUTPUT_MODE_BEEPER && !beeperClaimed[timId]) { "
        "beeperClaimed[timId] = true; "
        "isCanonicalBeeperPad = true; "
        "} "
        "bool isCanonicalLedPad = false; "
        "if (timerOverrides(timId)->outputMode == OUTPUT_MODE_LED && !ledClaimed[timId]) { "
        "ledClaimed[timId] = true; "
        "isCanonicalLedPad = true; "
        "}");

    EXPECT_NE(normalizedSource.find(expectedLoop), std::string::npos)
        << "pwmBuildTimerOutputList()'s canonical-pad-tracking loop in the LIVE "
           "src/main/drivers/pwm_mapping.c no longer matches what "
           "applyFixedOverridesToArray() in this test file reproduces. Update "
           "applyFixedOverridesToArray() to match the live source (e.g. if the "
           "ledClaimed[]/beeperClaimed[] scanning strategy changes).";
}
