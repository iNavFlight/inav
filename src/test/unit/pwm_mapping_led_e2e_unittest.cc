/*
 * End-to-end unit test: pwmBuildTimerOutputList()'s FULL two-loop pipeline
 * (canonical-pad override loop + priority-ordered assignment loop) must not
 * let a sibling pad on a shared LED timer end up flagged TIM_USE_LED.
 *
 * This is a companion to pwm_mapping_led_unittest.cc, which only exercises
 * timerHardwareOverride() in isolation (the first loop). That is NOT enough
 * to catch the real bug found via hardware bootlog tracing on a SPEDIXF405
 * (TIM4, S5+S6 shared): timerHardwareOverride() correctly set
 * S5=TIM_USE_LED, S6=TIM_USE_PINIO after the first (canonical-claim) loop,
 * but pwmAssignOutput()'s MAP_TO_LED_OUTPUT case, run later in the SECOND
 * (assignment) loop, used to call pwmClaimTimer(timHw->tim, timHw->usageFlags)
 * after assigning the canonical pad. pwmClaimTimer() propagates the
 * just-assigned pad's usageFlags to EVERY sibling pad sharing the same
 * physical timer — correct for MOTOR/SERVO (a multi-channel motor timer
 * really should offer multiple motor outputs), but wrong for LED, since it
 * silently overwrote the sibling's correct TIM_USE_PINIO back to
 * TIM_USE_LED. This reintroduced the "sibling pad falsely reported as LED"
 * bug via a different, later code path than the one originally fixed in
 * timerHardwareOverride().
 *
 * Fix (already applied to src/main/drivers/pwm_mapping.c, NOT modified by
 * this test file): pwmAssignOutput()'s MAP_TO_LED_OUTPUT case no longer
 * calls pwmClaimTimer() — it only masks timHw->usageFlags &= TIM_USE_LED.
 * The MAP_TO_MOTOR_OUTPUT / MAP_TO_SERVO_OUTPUT cases are UNCHANGED and
 * still call pwmClaimTimer() to propagate flags to sibling pads.
 *
 * This file contains:
 *   1. Inline, faithful reproductions of ALL THREE pieces working together,
 *      end to end:
 *        - timerHardwareOverride() (fixed version, with isCanonicalBeeperPad
 *          / isCanonicalLedPad canonical-pad tracking)
 *        - pwmClaimTimer() (exact real logic: propagate-usageFlags-to-
 *          siblings-sharing-a-timer)
 *        - pwmAssignOutput() in BOTH its FIXED form (current source: LED
 *          case does NOT call pwmClaimTimer) and its BUGGY/pre-fix form (LED
 *          case DOES call pwmClaimTimer, exactly like MOTOR/SERVO), so the
 *          test can demonstrate the bug reproduces against the buggy form
 *          and is fixed against the current form.
 *      pwm_mapping.c is excluded from the SITL/host unit-test build (guarded
 *      by `#if !defined(SITL_BUILD)`), and depends on PG (parameter-group)
 *      infrastructure not available in a host unit build, so it cannot be
 *      linked directly — same constraint as pwm_mapping_led_unittest.cc and
 *      pwm_mapping_beeper_unittest.cc.
 *   2. A minimal simulateBuildTimerOutputList() that reproduces
 *      pwmBuildTimerOutputList()'s two-loop structure: the canonical-claim
 *      loop, then the priority-ordered (dedicated pass, then auto pass)
 *      assignment loop, with checkPwmTimerConflicts() stubbed to always
 *      return false (matching the real hardware trace where
 *      FEATURE_LED_STRIP was off).
 *   3. TEST SUITE LedSiblingE2E — the primary regression test. Two pads
 *      share one physical timer set to LED mode. After running the FULL
 *      simulated pipeline (both loops):
 *        - with the BUGGY pwmAssignOutput (pre-fix): the sibling pad ends up
 *          incorrectly flagged TIM_USE_LED too (bug reproduction).
 *        - with the FIXED pwmAssignOutput (current source): the canonical
 *          pad keeps TIM_USE_LED and the sibling correctly falls back to
 *          TIM_USE_PINIO, NOT TIM_USE_LED.
 *   4. TEST SUITE MotorSiblingPropagationRegression — the inverse/regression
 *      guard: pwmClaimTimer()'s propagation-to-siblings behavior must still
 *      happen for MAP_TO_MOTOR_OUTPUT (this fix must not have been
 *      accidentally broadened to remove propagation for motor/servo too).
 *   5. TEST SUITE SourceSync — reads the ACTUAL LIVE
 *      src/main/drivers/pwm_mapping.c off disk at test time and asserts that
 *      pwmAssignOutput()'s MAP_TO_LED_OUTPUT case does NOT contain a
 *      pwmClaimTimer() call, while MAP_TO_MOTOR_OUTPUT / MAP_TO_SERVO_OUTPUT
 *      still do. This exists because pwm_mapping_beeper_unittest.cc was
 *      previously found to have drifted silently for months after upstream
 *      logic changed (see pwm_mapping_led_unittest.cc's SourceSync suite) —
 *      an inline reproduction is unavoidable here, but it must not be
 *      allowed to silently go stale.
 */

#include <stdint.h>
#include <stdbool.h>
#include <cctype>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

#include "gtest/gtest.h"
#include "unittest_macros.h"

/* -------------------------------------------------------------------------
 * Minimal type/flag definitions — mirrors the real firmware headers on this
 * branch (src/main/drivers/timer.h, src/main/flight/mixer.h,
 * src/main/drivers/pwm_mapping.c's internal MAP_TO_* enum).
 * ------------------------------------------------------------------------- */

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

#define TIM_IS_MOTOR(flags) ((flags) & (uint32_t)TIM_USE_MOTOR)
#define TIM_IS_SERVO(flags) ((flags) & (uint32_t)TIM_USE_SERVO)
#define TIM_IS_LED(flags)   ((flags) & (uint32_t)TIM_USE_LED)

typedef enum {
    OUTPUT_MODE_AUTO    = 0,
    OUTPUT_MODE_MOTORS,
    OUTPUT_MODE_SERVOS,
    OUTPUT_MODE_LED,
    OUTPUT_MODE_PINIO,
    OUTPUT_MODE_BEEPER
} outputMode_e;

/* Mirrors the anonymous enum at the top of pwm_mapping.c */
enum {
    MAP_TO_NONE,
    MAP_TO_MOTOR_OUTPUT,
    MAP_TO_SERVO_OUTPUT,
    MAP_TO_LED_OUTPUT
};

/* Minimal timer hardware entry. `timId` stands in for the real code's
 * timer2id(timer->tim) / the `tim` pointer identity itself — in the real
 * driver, multiple timerHardware[] pads sharing the same physical timer
 * compare equal on timer2id(tim); here we just store the id directly and use
 * it as the "same physical timer" key everywhere the real code would compare
 * `->tim` pointers (including inside the simulated pwmClaimTimer()). */
typedef struct timerHardware_s {
    uint32_t usageFlags;
    uint8_t timId;
} timerHardware_t;

static const int MAX_PADS = 16;
static const int MAX_TIMID = 16;

/* -------------------------------------------------------------------------
 * Piece 1: timerHardwareOverride() — literal translation of the current
 * (fixed) src/main/drivers/pwm_mapping.c. Identical to the copy in
 * pwm_mapping_led_unittest.cc / pwm_mapping_beeper_unittest.cc; duplicated
 * here because each *_unittest.cc becomes its own standalone executable
 * (see src/test/unit/CMakeLists.txt's `file(GLOB TEST_PROGRAMS *_unittest.cc)`
 * + one add_executable() per file), so statics cannot be shared across files.
 * ------------------------------------------------------------------------- */
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

/* -------------------------------------------------------------------------
 * Piece 2: pwmClaimTimer() — exact real logic (propagate usageFlags to every
 * pad sharing the same physical timer). Real signature is
 * pwmClaimTimer(HAL_Timer_t *tim, uint32_t usageFlags) and it iterates the
 * GLOBAL timerHardware[]/timerHardwareCount; here `pads`/`count` stand in for
 * that global array, passed explicitly since this is a host unit test with
 * no global timer table.
 * ------------------------------------------------------------------------- */
static uint8_t pwmClaimTimer_sim(timerHardware_t *pads, int count, uint8_t timId, uint32_t usageFlags)
{
    uint8_t changed = 0;
    for (int idx = 0; idx < count; idx++) {
        timerHardware_t *timHw = &pads[idx];
        if (timHw->timId == timId && timHw->usageFlags != usageFlags) {
            timHw->usageFlags = usageFlags;
            changed++;
        }
    }
    return changed;
}

/* Output-collection bookkeeping — stands in for timMotorServoHardware_t's
 * timMotors[]/timServos[] arrays (pointers into timerHardware[] in the real
 * code; here indices into `pads[]`). Note: the real timOutputs struct does
 * NOT track LED pads at all — the LED strip driver finds its pad later via
 * timerGetByUsageFlag(TIM_USE_LED) — so we don't track LED assignments here
 * either; LED "assignment" is verified purely via the pad's final usageFlags,
 * matching real-world observability. */
struct SimOutputs {
    int motorPadIdx[MAX_PADS];
    int motorCount;
    int servoPadIdx[MAX_PADS];
    int servoCount;
};

static bool pwmHasMotorOnTimer_sim(const timerHardware_t *pads, const SimOutputs *outputs, uint8_t timId)
{
    for (int i = 0; i < outputs->motorCount; i++) {
        if (pads[outputs->motorPadIdx[i]].timId == timId) {
            return true;
        }
    }
    return false;
}

static bool pwmHasServoOnTimer_sim(const timerHardware_t *pads, const SimOutputs *outputs, uint8_t timId)
{
    for (int i = 0; i < outputs->servoCount; i++) {
        if (pads[outputs->servoPadIdx[i]].timId == timId) {
            return true;
        }
    }
    return false;
}

/* -------------------------------------------------------------------------
 * Piece 3: pwmAssignOutput() — provided in TWO forms:
 *
 *   pwmAssignOutput_fixed()  — literal translation of the CURRENT (fixed)
 *                              src/main/drivers/pwm_mapping.c: MOTOR/SERVO
 *                              cases call pwmClaimTimer_sim() to propagate
 *                              flags to sibling pads; the LED case does NOT.
 *
 *   pwmAssignOutput_buggy()  — reproduces the PRE-FIX behavior: identical
 *                              except the LED case ALSO calls
 *                              pwmClaimTimer_sim(), exactly like MOTOR/SERVO
 *                              used to (and still do). This is the exact bug
 *                              found via hardware bootlog tracing.
 * ------------------------------------------------------------------------- */
static void pwmAssignOutput_fixed(SimOutputs *outputs, timerHardware_t *pads, int count,
                                   timerHardware_t *timHw, int type)
{
    int padIdx = (int)(timHw - pads);
    switch (type) {
        case MAP_TO_MOTOR_OUTPUT:
            timHw->usageFlags &= (uint32_t)TIM_USE_MOTOR;
            outputs->motorPadIdx[outputs->motorCount++] = padIdx;
            pwmClaimTimer_sim(pads, count, timHw->timId, timHw->usageFlags);
            break;
        case MAP_TO_SERVO_OUTPUT:
            timHw->usageFlags &= (uint32_t)TIM_USE_SERVO;
            outputs->servoPadIdx[outputs->servoCount++] = padIdx;
            pwmClaimTimer_sim(pads, count, timHw->timId, timHw->usageFlags);
            break;
        case MAP_TO_LED_OUTPUT:
            // FIX: no pwmClaimTimer_sim() call here — see file header comment.
            timHw->usageFlags &= (uint32_t)TIM_USE_LED;
            break;
        default:
            break;
    }
}

static void pwmAssignOutput_buggy(SimOutputs *outputs, timerHardware_t *pads, int count,
                                   timerHardware_t *timHw, int type)
{
    int padIdx = (int)(timHw - pads);
    switch (type) {
        case MAP_TO_MOTOR_OUTPUT:
            timHw->usageFlags &= (uint32_t)TIM_USE_MOTOR;
            outputs->motorPadIdx[outputs->motorCount++] = padIdx;
            pwmClaimTimer_sim(pads, count, timHw->timId, timHw->usageFlags);
            break;
        case MAP_TO_SERVO_OUTPUT:
            timHw->usageFlags &= (uint32_t)TIM_USE_SERVO;
            outputs->servoPadIdx[outputs->servoCount++] = padIdx;
            pwmClaimTimer_sim(pads, count, timHw->timId, timHw->usageFlags);
            break;
        case MAP_TO_LED_OUTPUT:
            // BUG (pre-fix): propagates this pad's masked flags to every
            // sibling sharing the timer, overwriting a correctly-computed
            // TIM_USE_PINIO sibling back to TIM_USE_LED.
            timHw->usageFlags &= (uint32_t)TIM_USE_LED;
            pwmClaimTimer_sim(pads, count, timHw->timId, timHw->usageFlags);
            break;
        default:
            break;
    }
}

typedef void (*pwmAssignOutputFn)(SimOutputs *, timerHardware_t *, int, timerHardware_t *, int);

/* checkPwmTimerConflicts() stub — the task scenario matches the real
 * hardware trace where FEATURE_LED_STRIP was off, so this always returns
 * false (never skip a pad due to a conflicting port/feature). */
static bool checkPwmTimerConflicts_stub(const timerHardware_t * /*timHw*/)
{
    return false;
}

/* -------------------------------------------------------------------------
 * Piece 4: simulateBuildTimerOutputList() — minimal reproduction of
 * pwmBuildTimerOutputList()'s two-loop structure:
 *   Loop 1: canonical-pad tracking (beeperClaimed[]/ledClaimed[]) +
 *           timerHardwareOverride() for every pad.
 *   Loop 2: priority-ordered assignment (priority 0 = dedicated pass,
 *           priority 1 = auto pass), motor branch then servo branch then
 *           LED branch (auto-pass only), exactly mirroring the real
 *           function's branch order and conditions.
 * `assignFn` selects pwmAssignOutput_fixed or pwmAssignOutput_buggy.
 * ------------------------------------------------------------------------- */
static void simulateBuildTimerOutputList(timerHardware_t *pads, int count,
                                          const outputMode_e *modeByTimId,
                                          int motorCount, int servoCount,
                                          pwmAssignOutputFn assignFn,
                                          SimOutputs *outputs)
{
    outputs->motorCount = 0;
    outputs->servoCount = 0;

    // Loop 1: apply all timerOverrides upfront (canonical-claim tracking).
    bool beeperClaimed[MAX_TIMID] = { false };
    bool ledClaimed[MAX_TIMID] = { false };
    for (int idx = 0; idx < count; idx++) {
        timerHardware_t *timHw = &pads[idx];
        uint8_t timId = timHw->timId;
        ASSERT_LT(timId, MAX_TIMID);
        outputMode_e mode = modeByTimId[timId];

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

    // Loop 2: assign outputs in priority order (dedicated, then auto).
    for (int priority = 0; priority < 2; priority++) {
        int motorIdx = outputs->motorCount;

        for (int idx = 0; idx < count; idx++) {
            timerHardware_t *timHw = &pads[idx];
            outputMode_e mode = modeByTimId[timHw->timId];
            bool isDedicated = (priority == 0);

            if (checkPwmTimerConflicts_stub(timHw)) {
                continue;
            }

            // Motors: dedicated (OUTPUT_MODE_MOTORS) first, then auto
            if (TIM_IS_MOTOR(timHw->usageFlags) && motorIdx < motorCount
                    && !pwmHasServoOnTimer_sim(pads, outputs, timHw->timId)
                    && (isDedicated ? mode == OUTPUT_MODE_MOTORS : mode != OUTPUT_MODE_MOTORS)) {
                assignFn(outputs, pads, count, timHw, MAP_TO_MOTOR_OUTPUT);
                motorIdx++;
                continue;
            }

            // Servos: dedicated (OUTPUT_MODE_SERVOS) first, then auto
            if (TIM_IS_SERVO(timHw->usageFlags) && outputs->servoCount < servoCount
                    && !pwmHasMotorOnTimer_sim(pads, outputs, timHw->timId)
                    && (isDedicated ? mode == OUTPUT_MODE_SERVOS : mode != OUTPUT_MODE_SERVOS)) {
                assignFn(outputs, pads, count, timHw, MAP_TO_SERVO_OUTPUT);
                continue;
            }

            // LEDs: only on the auto pass, and only if timer is uncontested
            if (!isDedicated && TIM_IS_LED(timHw->usageFlags)
                    && !pwmHasMotorOnTimer_sim(pads, outputs, timHw->timId)
                    && !pwmHasServoOnTimer_sim(pads, outputs, timHw->timId)) {
                assignFn(outputs, pads, count, timHw, MAP_TO_LED_OUTPUT);
            }
        }
    }
}

/* =========================================================================
 * TEST SUITE: LedSiblingE2E
 *
 * Two pads share one physical timer (timId=4), overridden to
 * OUTPUT_MODE_LED. No motors/servos in play (motorCount=servoCount=0), so
 * only the LED auto-pass branch is exercised in loop 2 — the exact path
 * that exposed the real bug (FEATURE_LED_STRIP off -> checkPwmTimerConflicts
 * always false -> LED pad reached via the uncontested-timer auto pass).
 * ========================================================================= */

TEST(LedSiblingE2E, BugReproduction_FullPipeline_SiblingIncorrectlyGetsLed)
{
    timerHardware_t pads[2];
    pads[0].usageFlags = 0; pads[0].timId = 4;
    pads[1].usageFlags = 0; pads[1].timId = 4;

    outputMode_e modeByTimId[MAX_TIMID];
    for (int i = 0; i < MAX_TIMID; i++) modeByTimId[i] = OUTPUT_MODE_AUTO;
    modeByTimId[4] = OUTPUT_MODE_LED;

    SimOutputs outputs;
    simulateBuildTimerOutputList(pads, 2, modeByTimId, /*motorCount=*/0, /*servoCount=*/0,
                                  pwmAssignOutput_buggy, &outputs);

    EXPECT_TRUE(pads[0].usageFlags & (uint32_t)TIM_USE_LED)
        << "Canonical (first-scanned) pad should get TIM_USE_LED";
    EXPECT_TRUE(pads[1].usageFlags & (uint32_t)TIM_USE_LED)
        << "BUG REPRODUCED: with the pre-fix pwmAssignOutput() (LED case also "
           "calling pwmClaimTimer()), the sibling pad — correctly computed as "
           "TIM_USE_PINIO by timerHardwareOverride() in loop 1 — gets silently "
           "overwritten back to TIM_USE_LED when the canonical pad is assigned "
           "in loop 2. This is the real hardware-observed bug (SPEDIXF405 "
           "TIM4 S5+S6).";
    EXPECT_FALSE(pads[1].usageFlags & (uint32_t)TIM_USE_PINIO)
        << "Sibling's correct PINIO flag was clobbered by propagation (bug symptom)";
}

TEST(LedSiblingE2E, FixVerification_FullPipeline_SiblingKeepsPinio_NotLed)
{
    timerHardware_t pads[2];
    pads[0].usageFlags = 0; pads[0].timId = 4;
    pads[1].usageFlags = 0; pads[1].timId = 4;

    outputMode_e modeByTimId[MAX_TIMID];
    for (int i = 0; i < MAX_TIMID; i++) modeByTimId[i] = OUTPUT_MODE_AUTO;
    modeByTimId[4] = OUTPUT_MODE_LED;

    SimOutputs outputs;
    simulateBuildTimerOutputList(pads, 2, modeByTimId, /*motorCount=*/0, /*servoCount=*/0,
                                  pwmAssignOutput_fixed, &outputs);

    EXPECT_TRUE(pads[0].usageFlags & (uint32_t)TIM_USE_LED)
        << "Canonical (first-scanned) pad must have TIM_USE_LED after the full pipeline";
    EXPECT_FALSE(pads[0].usageFlags & (uint32_t)TIM_USE_PINIO)
        << "Canonical pad must not also carry the PINIO fallback flag";

    EXPECT_FALSE(pads[1].usageFlags & (uint32_t)TIM_USE_LED)
        << "FIX VERIFIED: sibling pad on the same timer must NOT end up with "
           "TIM_USE_LED after the full two-loop pipeline (this is the "
           "assertion that would have FAILED against the pre-fix "
           "pwmAssignOutput(), which called pwmClaimTimer() for LED too)";
    EXPECT_TRUE(pads[1].usageFlags & (uint32_t)TIM_USE_PINIO)
        << "Sibling pad must retain TIM_USE_PINIO (GPIO-only fallback) set by "
           "timerHardwareOverride() in loop 1, undisturbed by loop 2's "
           "assignment of the canonical pad";
}

TEST(LedSiblingE2E, FixVerification_ThreeSiblingPads_OnlyCanonicalKeepsLed)
{
    timerHardware_t pads[3];
    for (int i = 0; i < 3; i++) { pads[i].usageFlags = 0; pads[i].timId = 9; }

    outputMode_e modeByTimId[MAX_TIMID];
    for (int i = 0; i < MAX_TIMID; i++) modeByTimId[i] = OUTPUT_MODE_AUTO;
    modeByTimId[9] = OUTPUT_MODE_LED;

    SimOutputs outputs;
    simulateBuildTimerOutputList(pads, 3, modeByTimId, /*motorCount=*/0, /*servoCount=*/0,
                                  pwmAssignOutput_fixed, &outputs);

    EXPECT_TRUE(pads[0].usageFlags & (uint32_t)TIM_USE_LED);
    for (int i = 1; i < 3; i++) {
        EXPECT_FALSE(pads[i].usageFlags & (uint32_t)TIM_USE_LED)
            << "pads[" << i << "] must not end up LED-flagged";
        EXPECT_TRUE(pads[i].usageFlags & (uint32_t)TIM_USE_PINIO)
            << "pads[" << i << "] must remain PINIO";
    }
}

/* =========================================================================
 * TEST SUITE: MotorSiblingPropagationRegression
 *
 * Regression guard: pwmClaimTimer()'s propagate-to-siblings behavior must
 * still happen for MAP_TO_MOTOR_OUTPUT (and, by the same code path, for
 * MAP_TO_SERVO_OUTPUT). The fix removed the pwmClaimTimer() call ONLY from
 * the LED case; it must NOT have been broadened to also drop it from
 * motor/servo.
 * ========================================================================= */

/*
 * Direct-call test: proves pwmAssignOutput_fixed()'s MOTOR case still
 * propagates via pwmClaimTimer_sim(). This is the test that actually
 * *distinguishes* "propagation happened" from "propagation didn't happen" —
 * pads[1] never has pwmAssignOutput_fixed() called on it directly; its
 * flags can ONLY change as a side effect of pads[0]'s assignment call
 * invoking pwmClaimTimer_sim() for the shared timId. Pre-existing
 * (non-MOTOR) bits on pads[1] must be overwritten (not merely OR'd/left
 * alone), matching pwmClaimTimer()'s real "usageFlags = usageFlags" (full
 * overwrite, not `|=`) semantics.
 */
TEST(MotorSiblingPropagationRegression, DirectCall_PropagationOverwritesSiblingFlags)
{
    timerHardware_t pads[2];
    pads[0].usageFlags = (uint32_t)(TIM_USE_MOTOR | TIM_USE_PWM);
    pads[0].timId = 7;
    pads[1].usageFlags = (uint32_t)(TIM_USE_MOTOR | TIM_USE_PPM); // different pre-existing bit
    pads[1].timId = 7;

    SimOutputs outputs;
    outputs.motorCount = 0;
    outputs.servoCount = 0;

    // Assign ONLY pads[0] — pads[1] is never passed to pwmAssignOutput_fixed().
    pwmAssignOutput_fixed(&outputs, pads, 2, &pads[0], MAP_TO_MOTOR_OUTPUT);

    EXPECT_EQ(pads[0].usageFlags, (uint32_t)TIM_USE_MOTOR)
        << "Assigned pad's own flags must be masked down to TIM_USE_MOTOR only";
    EXPECT_EQ(pads[1].usageFlags, (uint32_t)TIM_USE_MOTOR)
        << "REGRESSION CHECK: sibling pad's flags must be overwritten to "
           "TIM_USE_MOTOR by pwmClaimTimer() propagation alone, even though "
           "pwmAssignOutput() was never called on it directly. If this "
           "propagation were accidentally removed for MOTOR too, pads[1] "
           "would incorrectly retain TIM_USE_PPM here.";
    EXPECT_EQ(outputs.motorCount, 1);
}

/*
 * Full-pipeline sanity check: with the current fixed source, a two-pad
 * shared MOTOR timer still ends up with both pads assigned as motor outputs.
 * (Note: unlike the direct-call test above, this alone would still pass even
 * without propagation, since each pad also gets individually masked by its
 * own pwmAssignOutput() call in the auto/dedicated pass — the direct-call
 * test above is what actually isolates the propagation behavior. This test
 * is included for overall pipeline sanity / non-regression of the motor
 * assignment path as a whole.)
 */
TEST(MotorSiblingPropagationRegression, FullPipeline_BothMotorPadsAssigned)
{
    timerHardware_t pads[2];
    pads[0].usageFlags = 0; pads[0].timId = 7;
    pads[1].usageFlags = 0; pads[1].timId = 7;

    outputMode_e modeByTimId[MAX_TIMID];
    for (int i = 0; i < MAX_TIMID; i++) modeByTimId[i] = OUTPUT_MODE_AUTO;
    modeByTimId[7] = OUTPUT_MODE_MOTORS;

    SimOutputs outputs;
    simulateBuildTimerOutputList(pads, 2, modeByTimId, /*motorCount=*/2, /*servoCount=*/0,
                                  pwmAssignOutput_fixed, &outputs);

    EXPECT_EQ(outputs.motorCount, 2) << "Both pads on the shared MOTOR timer should be assigned";
    EXPECT_EQ(pads[0].usageFlags, (uint32_t)TIM_USE_MOTOR);
    EXPECT_EQ(pads[1].usageFlags, (uint32_t)TIM_USE_MOTOR);
}

/* =========================================================================
 * TEST SUITE: SourceSync
 *
 * Reads the ACTUAL LIVE src/main/drivers/pwm_mapping.c off disk and checks
 * that pwmAssignOutput()'s MAP_TO_LED_OUTPUT case does NOT call
 * pwmClaimTimer(), while MAP_TO_MOTOR_OUTPUT/MAP_TO_SERVO_OUTPUT still do.
 * Mirrors the SourceSync pattern in pwm_mapping_led_unittest.cc /
 * pwm_mapping_beeper_unittest.cc. This is a drift-detector, not a substitute
 * for the behavioral tests above (see those files' header comments for why
 * pwm_mapping.c can't be linked directly into a host unit test).
 * ========================================================================= */

namespace {

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

std::string pwmMappingSourcePath()
{
    std::string thisFile = __FILE__; // .../src/test/unit/pwm_mapping_led_e2e_unittest.cc
    size_t lastSlash = thisFile.find_last_of("/\\");
    std::string testUnitDir = (lastSlash == std::string::npos) ? "." : thisFile.substr(0, lastSlash);
    return testUnitDir + "/../../main/drivers/pwm_mapping.c";
}

::testing::AssertionResult loadNormalizedPwmMappingSource(std::string *out)
{
    const std::string path = pwmMappingSourcePath();
    std::ifstream file(path);
    if (!file.is_open()) {
        return ::testing::AssertionFailure()
            << "Could not open live source file at computed path: " << path
            << " -- SourceSync tests cannot verify anything against dead/absent "
               "source.";
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

// Extracts the substring of `normalizedSource` starting at the (already
// normalized) needle `funcSignature` through its matching closing brace,
// assuming brace-balanced C code with no braces inside string/char literals
// (true for pwmAssignOutput()). Returns empty string if not found or
// unbalanced.
std::string extractBraceBlock(const std::string &normalizedSource, const std::string &funcSignature)
{
    size_t start = normalizedSource.find(funcSignature);
    if (start == std::string::npos) {
        return "";
    }
    size_t bracePos = normalizedSource.find('{', start);
    if (bracePos == std::string::npos) {
        return "";
    }
    int depth = 0;
    size_t i = bracePos;
    for (; i < normalizedSource.size(); i++) {
        if (normalizedSource[i] == '{') depth++;
        else if (normalizedSource[i] == '}') {
            depth--;
            if (depth == 0) {
                return normalizedSource.substr(bracePos, i - bracePos + 1);
            }
        }
    }
    return ""; // unbalanced -- shouldn't happen for valid C
}

} // namespace

TEST(SourceSync, PwmAssignOutputSignatureMatchesLiveSource)
{
    std::string normalizedSource;
    ASSERT_TRUE(loadNormalizedPwmMappingSource(&normalizedSource));

    const std::string expectedSignature = normalizeSource(
        "static void pwmAssignOutput(timMotorServoHardware_t *timOutputs, "
        "timerHardware_t *timHw, int type)");

    EXPECT_NE(normalizedSource.find(expectedSignature), std::string::npos)
        << "pwmAssignOutput()'s signature in the LIVE src/main/drivers/pwm_mapping.c "
           "no longer matches what this test assumes. Update "
           "pwm_mapping_led_e2e_unittest.cc's inline reproduction and this "
           "expected string to match.";
}

TEST(SourceSync, LedCaseDoesNotCallPwmClaimTimer)
{
    std::string normalizedSource;
    ASSERT_TRUE(loadNormalizedPwmMappingSource(&normalizedSource));

    std::string body = extractBraceBlock(normalizedSource,
        normalizeSource("static void pwmAssignOutput(timMotorServoHardware_t *timOutputs, "
                         "timerHardware_t *timHw, int type)"));
    ASSERT_FALSE(body.empty())
        << "Could not locate/extract pwmAssignOutput()'s body from the live source "
           "-- check PwmAssignOutputSignatureMatchesLiveSource for details.";

    size_t ledCasePos = body.find("case MAP_TO_LED_OUTPUT:");
    ASSERT_NE(ledCasePos, std::string::npos)
        << "Could not find 'case MAP_TO_LED_OUTPUT:' inside pwmAssignOutput()'s body "
           "in the live source.";

    size_t nextCasePos = body.find("case", ledCasePos + 1);
    size_t defaultPos = body.find("default:", ledCasePos + 1);
    size_t ledCaseEnd = std::min(
        nextCasePos == std::string::npos ? body.size() : nextCasePos,
        defaultPos == std::string::npos ? body.size() : defaultPos);
    std::string ledCaseBody = body.substr(ledCasePos, ledCaseEnd - ledCasePos);

    EXPECT_EQ(ledCaseBody.find("pwmClaimTimer("), std::string::npos)
        << "REGRESSION: the live MAP_TO_LED_OUTPUT case in "
           "src/main/drivers/pwm_mapping.c now calls pwmClaimTimer() again. "
           "This reintroduces the exact bug this test file guards against: "
           "propagating this pad's flags to sibling pads on the same timer, "
           "overwriting their correct TIM_USE_PINIO back to TIM_USE_LED. "
           "LED case body found: " << ledCaseBody;
}

TEST(SourceSync, MotorAndServoCasesStillCallPwmClaimTimer)
{
    std::string normalizedSource;
    ASSERT_TRUE(loadNormalizedPwmMappingSource(&normalizedSource));

    std::string body = extractBraceBlock(normalizedSource,
        normalizeSource("static void pwmAssignOutput(timMotorServoHardware_t *timOutputs, "
                         "timerHardware_t *timHw, int type)"));
    ASSERT_FALSE(body.empty());

    size_t motorCasePos = body.find("case MAP_TO_MOTOR_OUTPUT:");
    size_t servoCasePos = body.find("case MAP_TO_SERVO_OUTPUT:");
    ASSERT_NE(motorCasePos, std::string::npos);
    ASSERT_NE(servoCasePos, std::string::npos);
    ASSERT_LT(motorCasePos, servoCasePos)
        << "Expected MAP_TO_MOTOR_OUTPUT case to appear before MAP_TO_SERVO_OUTPUT "
           "in source order, as in the current file. If this legitimately "
           "changed, adjust this test's slicing logic.";

    size_t ledCasePos = body.find("case MAP_TO_LED_OUTPUT:", servoCasePos);
    ASSERT_NE(ledCasePos, std::string::npos);

    std::string motorAndServoBody = body.substr(motorCasePos, ledCasePos - motorCasePos);

    // Expect exactly two pwmClaimTimer( calls in the motor+servo span (one each).
    int count = 0;
    size_t pos = 0;
    while ((pos = motorAndServoBody.find("pwmClaimTimer(", pos)) != std::string::npos) {
        count++;
        pos += std::strlen("pwmClaimTimer(");
    }
    EXPECT_EQ(count, 2)
        << "REGRESSION: expected exactly 2 pwmClaimTimer() calls across the "
           "MAP_TO_MOTOR_OUTPUT and MAP_TO_SERVO_OUTPUT cases (one each) in the "
           "live source, found " << count << ". This fix must only remove "
           "propagation for LED, not for motor/servo. Span checked: "
           << motorAndServoBody;
}

TEST(SourceSync, PwmClaimTimerBodyMatchesLiveSource)
{
    std::string normalizedSource;
    ASSERT_TRUE(loadNormalizedPwmMappingSource(&normalizedSource));

    const std::string expectedBody = normalizeSource(
        "uint8_t pwmClaimTimer(HAL_Timer_t *tim, uint32_t usageFlags) { "
        "uint8_t changed = 0; "
        "for (int idx = 0; idx < timerHardwareCount; idx++) { "
        "timerHardware_t *timHw = &timerHardware[idx]; "
        "if (timHw->tim == tim && timHw->usageFlags != usageFlags) { "
        "timHw->usageFlags = usageFlags; "
        "changed++; "
        "} "
        "} "
        "return changed; "
        "}");

    EXPECT_NE(normalizedSource.find(expectedBody), std::string::npos)
        << "pwmClaimTimer()'s body in the LIVE src/main/drivers/pwm_mapping.c no "
           "longer matches pwmClaimTimer_sim()'s reproduction in this test file "
           "(propagate-usageFlags-to-siblings-sharing-a-timer). Update "
           "pwmClaimTimer_sim() to match, then update this expected string.";
}
