/*
 * Unit test: pinioInit()'s guard against configuring a PINIO pad as
 * timer-PWM when it shares its physical timer with a fixed-period output
 * (WS2811 LED strip or BEEPER tone).
 *
 * Bug context (follow-on issue surfaced by the sibling-pad fix in
 * src/main/drivers/pwm_mapping.c, see pwm_mapping_led_unittest.cc /
 * pwm_mapping_beeper_unittest.cc):
 *
 *   Once pwm_mapping.c correctly falls sibling pads on an LED/BEEPER timer
 *   back to TIM_USE_PINIO (instead of falsely claiming TIM_USE_LED), those
 *   pads reach pinioInit(). pinioInit() calls pinioInitTimerPWM() on any
 *   TIM_USE_PINIO-flagged pad it finds a TCH for, and pinioInitTimerPWM()
 *   calls timerConfigBase(tch, PINIO_PWM_PERIOD, PINIO_PWM_BASE_HZ) — but the
 *   period/prescaler register that timerConfigBase() programs is SHARED by
 *   every channel on that physical timer. timerGetTCH() has no per-timer
 *   exclusivity check of its own: it will happily hand back a TCH for the
 *   PINIO pad regardless of what else (LED strip / BEEPER) is already
 *   running on that timer. Without a guard, configuring the PINIO sibling as
 *   PWM would silently reprogram the LED-strip's WS2811 bit-rate period (or
 *   the BEEPER's tone period), corrupting that output.
 *
 * Fix: src/main/drivers/pinio.c adds timerHasFixedPeriodSibling(), which
 * scans timerHardware[] for any OTHER entry on the SAME physical timer
 * (`.tim ==`) carrying TIM_USE_LED or TIM_USE_BEEPER, and gates both
 * pinioInitTimerPWM() call sites in pinioInit() on
 * `!timerHasFixedPeriodSibling(timHw) && ...`, so such a pad falls back to
 * the plain-GPIO path instead.
 *
 * This file contains:
 *   1. A minimal inline reproduction of timerHasFixedPeriodSibling() (and the
 *      minimal timerHardware_t / usage-flag definitions it needs), since
 *      pinio.c cannot be linked into the host unit-test binary: it is gated
 *      by `#ifdef USE_PINIO` and depends on live hardware/timer/IO driver
 *      infrastructure (IOInit, timerGetTCH, timerConfigBase, etc.) that is
 *      not mocked for unit tests.
 *   2. TEST SUITE TimerHasFixedPeriodSibling — covers: no sibling on the
 *      timer, LED sibling, BEEPER sibling, a sibling with a matching flag
 *      but on a DIFFERENT physical timer (verifies the `.tim ==` comparison
 *      itself is doing real work, not just the flag test), and the pad's own
 *      entry carrying TIM_USE_PINIO while scanning past it.
 *   3. TEST SUITE SourceSync — reads the ACTUAL LIVE
 *      src/main/drivers/pinio.c off disk at test time and verifies (a) the
 *      real timerHasFixedPeriodSibling() body still matches this file's
 *      inline reproduction, and (b) both call sites inside pinioInit() still
 *      gate on `timerHasFixedPeriodSibling(...)`, so this test cannot
 *      silently drift from the real implementation and cannot stay green if
 *      the guard is later removed. This is the same drift-detection pattern
 *      used by pwm_mapping_led_unittest.cc / pwm_mapping_beeper_unittest.cc
 *      after pwm_mapping_beeper_unittest.cc was found to have gone stale.
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
 * Minimal type/flag definitions — mirrors src/main/drivers/timer.h exactly
 * for the bits this function cares about. `tim` stands in for the real
 * code's `tmr_type *tim` / `TIM_TypeDef *tim` pointer — in the real driver,
 * multiple timerHardware[] pads sharing the same physical timer compare
 * equal on pointer identity of `.tim`; here we just use a plain int id and
 * compare that, which exercises identical logic.
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

/* Minimal timer hardware entry — only the fields timerHasFixedPeriodSibling()
 * actually touches (`.tim`, `.usageFlags`). */
typedef struct timerHardware_s {
    int tim; // stands in for the real tmr_type*/TIM_TypeDef* pointer identity
    uint32_t usageFlags;
} timerHardware_t;

/* -------------------------------------------------------------------------
 * Inline reproduction of timerHasFixedPeriodSibling().
 *
 * We inline this rather than linking pinio.c because pinio.c is gated by
 * `#ifdef USE_PINIO` and depends on IOInit/IOConfigGPIOAF/timerGetTCH/
 * timerConfigBase/timerPWMConfigChannel/timerPWMStart/timerEnable — none of
 * which exist in the host unit-test build.
 *
 * This is a literal translation of the current C source. It operates against
 * test-local `timerHardware[]` / `timerHardwareCount` globals (declared
 * below, mirroring the real extern globals from timer.h) rather than the
 * real firmware ones, so each test can set up its own hardware table.
 *
 * See the SourceSync test suite at the bottom of this file for a mechanism
 * that catches this reproduction going out of sync with the real source.
 * ------------------------------------------------------------------------- */

static timerHardware_t *g_timerHardware = nullptr;
static int g_timerHardwareCount = 0;

static bool timerHasFixedPeriodSibling(const timerHardware_t *timHw)
{
    for (int i = 0; i < g_timerHardwareCount; i++) {
        if (g_timerHardware[i].tim == timHw->tim && (g_timerHardware[i].usageFlags & (TIM_USE_LED | TIM_USE_BEEPER))) {
            return true;
        }
    }
    return false;
}

/* =========================================================================
 * TEST SUITE: TimerHasFixedPeriodSibling
 * ========================================================================= */

/*
 * TEST 1 — No other pad on the timer at all: must return false.
 */
TEST(TimerHasFixedPeriodSibling, NoSibling_ReturnsFalse)
{
    timerHardware_t pads[1];
    pads[0].tim = 1; pads[0].usageFlags = (uint32_t)TIM_USE_PINIO;

    g_timerHardware = pads;
    g_timerHardwareCount = 1;

    EXPECT_FALSE(timerHasFixedPeriodSibling(&pads[0]));
}

/*
 * TEST 2 — A sibling pad on the SAME physical timer carries TIM_USE_LED:
 * must return true.
 */
TEST(TimerHasFixedPeriodSibling, LedSiblingOnSameTimer_ReturnsTrue)
{
    timerHardware_t pads[2];
    pads[0].tim = 5; pads[0].usageFlags = (uint32_t)TIM_USE_PINIO; // pad under test
    pads[1].tim = 5; pads[1].usageFlags = (uint32_t)TIM_USE_LED;   // sibling: LED strip

    g_timerHardware = pads;
    g_timerHardwareCount = 2;

    EXPECT_TRUE(timerHasFixedPeriodSibling(&pads[0]))
        << "A PINIO pad sharing its timer with an LED-strip pad must be "
           "reported as having a fixed-period sibling, so pinioInit() does "
           "not reprogram the LED strip's WS2811 period.";
}

/*
 * TEST 3 — A sibling pad on the SAME physical timer carries TIM_USE_BEEPER:
 * must return true.
 */
TEST(TimerHasFixedPeriodSibling, BeeperSiblingOnSameTimer_ReturnsTrue)
{
    timerHardware_t pads[2];
    pads[0].tim = 7; pads[0].usageFlags = (uint32_t)TIM_USE_PINIO; // pad under test
    pads[1].tim = 7; pads[1].usageFlags = (uint32_t)TIM_USE_BEEPER; // sibling: buzzer

    g_timerHardware = pads;
    g_timerHardwareCount = 2;

    EXPECT_TRUE(timerHasFixedPeriodSibling(&pads[0]))
        << "A PINIO pad sharing its timer with a BEEPER pad must be reported "
           "as having a fixed-period sibling, so pinioInit() does not "
           "reprogram the buzzer's tone period.";
}

/*
 * TEST 4 — A DIFFERENT physical timer has an LED pad, but the timer under
 * test does not. Must return false. This specifically verifies the `.tim ==`
 * comparison is doing real work (i.e. the function is not just checking
 * "does ANY entry anywhere have TIM_USE_LED|TIM_USE_BEEPER", which would be
 * a much easier, but wrong, bug to accidentally introduce).
 */
TEST(TimerHasFixedPeriodSibling, LedOnDifferentTimer_ReturnsFalse)
{
    timerHardware_t pads[2];
    pads[0].tim = 1; pads[0].usageFlags = (uint32_t)TIM_USE_PINIO; // pad under test, timer 1
    pads[1].tim = 2; pads[1].usageFlags = (uint32_t)TIM_USE_LED;   // LED pad, timer 2 (different!)

    g_timerHardware = pads;
    g_timerHardwareCount = 2;

    EXPECT_FALSE(timerHasFixedPeriodSibling(&pads[0]))
        << "An LED/BEEPER pad on a DIFFERENT physical timer must not cause a "
           "false positive -- the comparison must be scoped to the SAME "
           "timer (`.tim ==`), not just a global flag scan.";
}

/*
 * TEST 5 — Same as TEST 4 but with BEEPER instead of LED, for symmetry.
 */
TEST(TimerHasFixedPeriodSibling, BeeperOnDifferentTimer_ReturnsFalse)
{
    timerHardware_t pads[2];
    pads[0].tim = 3; pads[0].usageFlags = (uint32_t)TIM_USE_PINIO;
    pads[1].tim = 9; pads[1].usageFlags = (uint32_t)TIM_USE_BEEPER;

    g_timerHardware = pads;
    g_timerHardwareCount = 2;

    EXPECT_FALSE(timerHasFixedPeriodSibling(&pads[0]));
}

/*
 * TEST 6 — The pad's own entry (in timerHardware[]) carries TIM_USE_PINIO;
 * scanning must correctly pass over it (a pad with only TIM_USE_PINIO set
 * must not itself be mistaken for a fixed-period sibling) while still
 * finding the real LED sibling elsewhere on the same timer. Also exercises
 * a 3-pad table so the loop must not stop early.
 */
TEST(TimerHasFixedPeriodSibling, OwnPinioEntryIgnored_SiblingStillFound)
{
    timerHardware_t pads[3];
    pads[0].tim = 4; pads[0].usageFlags = (uint32_t)TIM_USE_PINIO; // pad under test (itself in the table)
    pads[1].tim = 4; pads[1].usageFlags = (uint32_t)TIM_USE_PINIO; // another PINIO pad, same timer, not fixed-period
    pads[2].tim = 4; pads[2].usageFlags = (uint32_t)TIM_USE_LED;   // the real fixed-period sibling

    g_timerHardware = pads;
    g_timerHardwareCount = 3;

    EXPECT_TRUE(timerHasFixedPeriodSibling(&pads[0]))
        << "Own TIM_USE_PINIO entry and a plain-PINIO sibling must not mask "
           "the presence of the real LED sibling further down the table.";
    EXPECT_TRUE(timerHasFixedPeriodSibling(&pads[1]))
        << "Same check from the second PINIO pad's perspective.";
}

/*
 * TEST 7 — Multiple pads on the timer, none of them fixed-period (all
 * MOTOR/SERVO/PINIO): must return false. Guards against a trivial "returns
 * true if timer has >1 pad" mis-implementation.
 */
TEST(TimerHasFixedPeriodSibling, MultipleNonFixedPeriodSiblings_ReturnsFalse)
{
    timerHardware_t pads[3];
    pads[0].tim = 6; pads[0].usageFlags = (uint32_t)TIM_USE_PINIO;
    pads[1].tim = 6; pads[1].usageFlags = (uint32_t)TIM_USE_MOTOR;
    pads[2].tim = 6; pads[2].usageFlags = (uint32_t)TIM_USE_SERVO;

    g_timerHardware = pads;
    g_timerHardwareCount = 3;

    EXPECT_FALSE(timerHasFixedPeriodSibling(&pads[0]));
}

/* =========================================================================
 * TEST SUITE: SourceSync
 *
 * Reads the ACTUAL LIVE src/main/drivers/pinio.c off disk (not a cached
 * copy, not something computed at CMake-configure time) and checks that:
 *   (a) timerHasFixedPeriodSibling()'s body still matches this file's inline
 *       reproduction, and
 *   (b) both pinioInitTimerPWM() call sites inside pinioInit() still gate on
 *       `timerHasFixedPeriodSibling(...)`.
 *
 * If a future change to pinio.c alters this logic (or removes the guard)
 * without updating this test file, these checks fail loudly in `make check`
 * / CI instead of this file silently continuing to pass against dead logic
 * -- exactly the class of drift that was previously found in
 * pwm_mapping_beeper_unittest.cc (see its header comment for history).
 *
 * This does not (and structurally cannot, given the USE_PINIO / hardware-
 * driver dependencies described above) substitute for actually linking and
 * executing the real function. It is a drift-detector, not a full
 * integration test. Comments are stripped and whitespace is collapsed
 * before comparison so that pure reformatting/typo-fixes in comments do not
 * cause false failures; only the executable-statement tokens are checked.
 * ========================================================================= */

namespace {

// Strips "//...\n" line comments. Deliberately does not handle /* */ block
// comments or comments inside string/char literals -- the functions we
// check in pinio.c don't use either, and keeping this simple avoids this
// drift-detector itself becoming a source of drift.
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

// Locates the live pinio.c relative to this test file's own path. __FILE__
// is emitted as an absolute path by this project's CMake/Makefiles
// (verified by the equivalent helper in pwm_mapping_led_unittest.cc /
// pwm_mapping_beeper_unittest.cc), so this resolves correctly regardless of
// the build directory or CWD the test is invoked from.
std::string pinioSourcePath()
{
    std::string thisFile = __FILE__; // .../src/test/unit/pinio_timer_sibling_unittest.cc
    size_t lastSlash = thisFile.find_last_of("/\\");
    std::string testUnitDir = (lastSlash == std::string::npos) ? "." : thisFile.substr(0, lastSlash);
    return testUnitDir + "/../../main/drivers/pinio.c";
}

// Reads and normalizes pinio.c. Fails the calling test LOUDLY (not silently
// returning empty / skipping) if the file can't be found, since a missing
// file here means this drift-detector isn't detecting anything.
::testing::AssertionResult loadNormalizedPinioSource(std::string *out)
{
    const std::string path = pinioSourcePath();
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

TEST(SourceSync, TimerHasFixedPeriodSiblingBodyMatchesLiveSource)
{
    std::string normalizedSource;
    ASSERT_TRUE(loadNormalizedPinioSource(&normalizedSource));

    const std::string expectedFunction = normalizeSource(
        "static bool timerHasFixedPeriodSibling(const timerHardware_t *timHw) "
        "{ "
        "for (int i = 0; i < timerHardwareCount; i++) { "
        "if (timerHardware[i].tim == timHw->tim && (timerHardware[i].usageFlags & (TIM_USE_LED | TIM_USE_BEEPER))) { "
        "return true; "
        "} "
        "} "
        "return false; "
        "}");

    EXPECT_NE(normalizedSource.find(expectedFunction), std::string::npos)
        << "timerHasFixedPeriodSibling() in the LIVE src/main/drivers/pinio.c "
           "no longer matches this test's inline reproduction. If the "
           "function legitimately changed, update the inline "
           "timerHasFixedPeriodSibling() in this test file to match, then "
           "update this expected string too. THIS is the exact class of "
           "drift that made pwm_mapping_beeper_unittest.cc stale for months "
           "-- do not let it happen again.";
}

TEST(SourceSync, Pass1CallSiteGatesOnFixedPeriodSiblingGuard)
{
    std::string normalizedSource;
    ASSERT_TRUE(loadNormalizedPinioSource(&normalizedSource));

    // Pass 1 (pinioHardware[] loop): the condition guarding pinioInitTimerPWM().
    const std::string expectedGuard = normalizeSource(
        "if (timHw && IOGetOwner(io) == OWNER_FREE && !timerHasFixedPeriodSibling(timHw) && pinioInitTimerPWM(runtimeCount, io, timHw, inverted)) {");

    EXPECT_NE(normalizedSource.find(expectedGuard), std::string::npos)
        << "pinioInit()'s Pass 1 (pinioHardware[] target-defined pins) call "
           "site no longer gates pinioInitTimerPWM() on "
           "!timerHasFixedPeriodSibling(timHw). If someone removes this "
           "guard, a PINIO pad sharing a timer with an LED strip or BEEPER "
           "would silently corrupt that output's period again -- this test "
           "must fail loudly if that happens.";
}

TEST(SourceSync, Pass2CallSiteGatesOnFixedPeriodSiblingGuard)
{
    std::string normalizedSource;
    ASSERT_TRUE(loadNormalizedPinioSource(&normalizedSource));

    // Pass 2 (timerHardware[] loop, TIM_USE_PINIO pads assigned via mixer):
    // the condition guarding pinioInitTimerPWM().
    const std::string expectedGuard = normalizeSource(
        "if (!timerHasFixedPeriodSibling(timHw) && pinioInitTimerPWM(runtimeCount, io, timHw, false)) {");

    EXPECT_NE(normalizedSource.find(expectedGuard), std::string::npos)
        << "pinioInit()'s Pass 2 (timerHardware[] TIM_USE_PINIO pads assigned "
           "via the mixer) call site no longer gates pinioInitTimerPWM() on "
           "!timerHasFixedPeriodSibling(timHw). If someone removes this "
           "guard, a mixer-assigned PINIO pad sharing a timer with an LED "
           "strip or BEEPER would silently corrupt that output's period "
           "again -- this test must fail loudly if that happens.";
}
