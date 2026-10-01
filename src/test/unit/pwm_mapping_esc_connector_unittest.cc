/*
 * A Smart ESC on a board's ESC connector (esc_srxl2_connector): after
 * pwmBuildTimerOutputList() the connector's pad has no servo, LED, beeper or
 * PINIO flag, so none of those takes the pin from the UART, and its motor slot
 * stays put, which keeps the servos where they were.
 *
 * pwm_mapping.c is empty in the host build (SITL_BUILD), so like the other
 * pwm_mapping tests this reproduces the override pass and the assignment loop;
 * SourceSync checks that the live source still has the connector's two hooks.
 */

#include <cctype>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>

#include "gtest/gtest.h"

namespace {

const uint32_t TIM_USE_MOTOR  = 1u << 2;
const uint32_t TIM_USE_SERVO  = 1u << 3;
const uint32_t TIM_USE_LED    = 1u << 24;
const uint32_t TIM_USE_BEEPER = 1u << 25;
const uint32_t TIM_USE_PINIO  = 1u << 26;
const uint32_t TIM_USE_OUTPUT_AUTO = TIM_USE_MOTOR | TIM_USE_SERVO;

enum outputMode_e { MODE_AUTO, MODE_MOTORS, MODE_SERVOS, MODE_LED, MODE_PINIO, MODE_BEEPER };

const int MAX_TIMERS = 16;
const int MAX_PADS = 16;

struct Pad {
    uint8_t tim;
    uint32_t usageFlags;
    bool connector;
};

struct Outputs {
    int motors[MAX_PADS];
    int motorCount;
    int servos[MAX_PADS];
    int servoCount;
};

void timerHardwareOverride(Pad *pad, outputMode_e mode, bool isCanonicalBeeperPad, bool isCanonicalLedPad)
{
    switch (mode) {
    case MODE_MOTORS:
        pad->usageFlags &= ~(TIM_USE_SERVO | TIM_USE_LED | TIM_USE_BEEPER);
        pad->usageFlags |= TIM_USE_MOTOR;
        break;
    case MODE_SERVOS:
        pad->usageFlags &= ~(TIM_USE_MOTOR | TIM_USE_LED | TIM_USE_BEEPER);
        pad->usageFlags |= TIM_USE_SERVO;
        break;
    case MODE_LED:
        pad->usageFlags &= ~(TIM_USE_MOTOR | TIM_USE_SERVO | TIM_USE_BEEPER | TIM_USE_LED);
        pad->usageFlags |= isCanonicalLedPad ? TIM_USE_LED : TIM_USE_PINIO;
        break;
    case MODE_PINIO:
        pad->usageFlags &= ~(TIM_USE_MOTOR | TIM_USE_SERVO | TIM_USE_LED | TIM_USE_BEEPER);
        pad->usageFlags |= TIM_USE_PINIO;
        break;
    case MODE_BEEPER:
        pad->usageFlags &= ~(TIM_USE_MOTOR | TIM_USE_SERVO | TIM_USE_LED | TIM_USE_BEEPER);
        pad->usageFlags |= isCanonicalBeeperPad ? TIM_USE_BEEPER : TIM_USE_PINIO;
        break;
    default:
        break;
    }
}

bool hasOutputOnTimer(const Pad *pads, const int *list, int count, uint8_t tim)
{
    for (int i = 0; i < count; i++) {
        if (pads[list[i]].tim == tim) {
            return true;
        }
    }
    return false;
}

// pwmAssignOutput() for motors and servos: pwmClaimTimer() hands the pad's flags to its timer's siblings
void assignOutput(Pad *pads, int padCount, int idx, uint32_t flag, int *list, int *count)
{
    pads[idx].usageFlags &= flag;
    list[(*count)++] = idx;
    for (int i = 0; i < padCount; i++) {
        if (pads[i].tim == pads[idx].tim) {
            pads[i].usageFlags = pads[idx].usageFlags;
        }
    }
}

void buildOutputList(Pad *pads, int padCount, const outputMode_e *modes, int motorCount, int servoCount,
                     bool connectorInUse, Outputs *out)
{
    bool beeperClaimed[MAX_TIMERS] = { false };
    bool ledClaimed[MAX_TIMERS] = { false };
    for (int i = 0; i < padCount; i++) {
        const uint8_t tim = pads[i].tim;
        const bool canonicalBeeper = modes[tim] == MODE_BEEPER && !beeperClaimed[tim];
        const bool canonicalLed = modes[tim] == MODE_LED && !ledClaimed[tim];
        beeperClaimed[tim] = beeperClaimed[tim] || canonicalBeeper;
        ledClaimed[tim] = ledClaimed[tim] || canonicalLed;
        timerHardwareOverride(&pads[i], modes[tim], canonicalBeeper, canonicalLed);
    }

    out->motorCount = 0;
    out->servoCount = 0;
    for (int priority = 0; priority < 2; priority++) {
        const bool isDedicated = (priority == 0);
        int motorIdx = out->motorCount;

        for (int i = 0; i < padCount; i++) {
            Pad *pad = &pads[i];
            const outputMode_e mode = modes[pad->tim];

            if ((pad->usageFlags & TIM_USE_MOTOR) && motorIdx < motorCount
                    && !hasOutputOnTimer(pads, out->servos, out->servoCount, pad->tim)
                    && (isDedicated ? mode == MODE_MOTORS : mode != MODE_MOTORS)) {
                assignOutput(pads, padCount, i, TIM_USE_MOTOR, out->motors, &out->motorCount);
                motorIdx++;
                continue;
            }

            if ((pad->usageFlags & TIM_USE_SERVO) && out->servoCount < servoCount
                    && !hasOutputOnTimer(pads, out->motors, out->motorCount, pad->tim)
                    && !(connectorInUse && pad->connector)
                    && (isDedicated ? mode == MODE_SERVOS : mode != MODE_SERVOS)) {
                assignOutput(pads, padCount, i, TIM_USE_SERVO, out->servos, &out->servoCount);
                continue;
            }

            if (!isDedicated && (pad->usageFlags & TIM_USE_LED)
                    && !hasOutputOnTimer(pads, out->motors, out->motorCount, pad->tim)
                    && !hasOutputOnTimer(pads, out->servos, out->servoCount, pad->tim)) {
                pad->usageFlags &= TIM_USE_LED;
            }
        }
    }

    for (int i = 0; i < padCount; i++) {
        if (connectorInUse && pads[i].connector) {
            pads[i].usageFlags &= ~(TIM_USE_SERVO | TIM_USE_LED | TIM_USE_BEEPER | TIM_USE_PINIO);
        }
    }
}

// NEXUS X/XR in target.c order; its config.c sets TIM1, the ESC connector's timer, to motors
const int NEXUSX_ESC = 4;
const Pad NEXUSX[] = {
    { 3, TIM_USE_OUTPUT_AUTO, false },  // S1
    { 3, TIM_USE_OUTPUT_AUTO, false },  // S2
    { 3, TIM_USE_OUTPUT_AUTO, false },  // S3
    { 2, TIM_USE_OUTPUT_AUTO, false },  // TAIL
    { 1, TIM_USE_OUTPUT_AUTO, true },   // ESC, UART1 TX with the connector in use
    { 2, TIM_USE_OUTPUT_AUTO, false },  // RPM
    { 2, TIM_USE_OUTPUT_AUTO, false },  // TLM
    { 4, TIM_USE_OUTPUT_AUTO, false },  // AUX
    { 4, TIM_USE_OUTPUT_AUTO, false },  // SBUS
};
const int NEXUSX_PADS = sizeof(NEXUSX) / sizeof(NEXUSX[0]);

struct Board {
    Pad pads[MAX_PADS];
    outputMode_e modes[MAX_TIMERS];
    Outputs out;
};

Board nexusX(outputMode_e escTimerMode)
{
    Board b = {};
    for (int i = 0; i < NEXUSX_PADS; i++) {
        b.pads[i] = NEXUSX[i];
    }
    b.modes[1] = escTimerMode;
    return b;
}

bool isServo(const Outputs &out, int pad)
{
    for (int i = 0; i < out.servoCount; i++) {
        if (out.servos[i] == pad) {
            return true;
        }
    }
    return false;
}

TEST(EscConnector, KeepsItsMotorSlotSoServosStay)
{
    Board off = nexusX(MODE_MOTORS);
    Board on = nexusX(MODE_MOTORS);
    buildOutputList(off.pads, NEXUSX_PADS, off.modes, 1, 7, false, &off.out);
    buildOutputList(on.pads, NEXUSX_PADS, on.modes, 1, 7, true, &on.out);

    ASSERT_EQ(1, on.out.motorCount);
    EXPECT_EQ(NEXUSX_ESC, on.out.motors[0]);
    EXPECT_EQ(TIM_USE_MOTOR, on.pads[NEXUSX_ESC].usageFlags);
    ASSERT_EQ(off.out.servoCount, on.out.servoCount);
    for (int i = 0; i < on.out.servoCount; i++) {
        EXPECT_EQ(off.out.servos[i], on.out.servos[i]) << "servo " << i + 1;
    }
}

TEST(EscConnector, BeeperOnItsTimerLeavesThePin)
{
    Board off = nexusX(MODE_BEEPER);
    Board on = nexusX(MODE_BEEPER);
    buildOutputList(off.pads, NEXUSX_PADS, off.modes, 1, 7, false, &off.out);
    buildOutputList(on.pads, NEXUSX_PADS, on.modes, 1, 7, true, &on.out);

    // beeperPwmInit() looks the pad up by TIM_USE_BEEPER
    EXPECT_TRUE(off.pads[NEXUSX_ESC].usageFlags & TIM_USE_BEEPER);
    EXPECT_FALSE(on.pads[NEXUSX_ESC].usageFlags & TIM_USE_BEEPER);
}

TEST(EscConnector, LedStripOnItsTimerLeavesThePin)
{
    Board off = nexusX(MODE_LED);
    Board on = nexusX(MODE_LED);
    buildOutputList(off.pads, NEXUSX_PADS, off.modes, 1, 7, false, &off.out);
    buildOutputList(on.pads, NEXUSX_PADS, on.modes, 1, 7, true, &on.out);

    EXPECT_TRUE(off.pads[NEXUSX_ESC].usageFlags & TIM_USE_LED);
    EXPECT_FALSE(on.pads[NEXUSX_ESC].usageFlags & TIM_USE_LED);
}

TEST(EscConnector, PinioOnItsTimerLeavesThePin)
{
    Board on = nexusX(MODE_PINIO);
    buildOutputList(on.pads, NEXUSX_PADS, on.modes, 1, 7, true, &on.out);

    EXPECT_FALSE(on.pads[NEXUSX_ESC].usageFlags & TIM_USE_PINIO);
}

TEST(EscConnector, TimerOnAutoGivesItNoServo)
{
    Board on = nexusX(MODE_AUTO);
    buildOutputList(on.pads, NEXUSX_PADS, on.modes, 1, 8, true, &on.out);

    EXPECT_FALSE(isServo(on.out, NEXUSX_ESC));
    EXPECT_FALSE(on.pads[NEXUSX_ESC].usageFlags & TIM_USE_SERVO);
}

TEST(EscConnector, SiblingServoLeavesItNoServoFlag)
{
    // A connector sharing its timer with a later pad: that pad's servo claim copies TIM_USE_SERVO onto it
    Pad pads[] = {
        { 8, TIM_USE_OUTPUT_AUTO, false },
        { 3, TIM_USE_OUTPUT_AUTO, false },
        { 2, TIM_USE_OUTPUT_AUTO, true },
        { 2, TIM_USE_OUTPUT_AUTO, false },
        { 5, TIM_USE_OUTPUT_AUTO, false },
    };
    outputMode_e modes[MAX_TIMERS] = {};
    Outputs out = {};
    buildOutputList(pads, 5, modes, 1, 4, true, &out);

    EXPECT_FALSE(isServo(out, 2));
    EXPECT_TRUE(isServo(out, 3));
    EXPECT_EQ(0u, pads[2].usageFlags & ~TIM_USE_MOTOR);
}

std::string normalizeWhitespace(const std::string &text)
{
    std::string result;
    bool lastWasSpace = true;
    for (char c : text) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!lastWasSpace) {
                result += ' ';
            }
            lastWasSpace = true;
        } else {
            result += c;
            lastWasSpace = false;
        }
    }
    return result;
}

std::string livePwmMappingSource()
{
    std::string thisFile = __FILE__;
    const size_t lastSlash = thisFile.find_last_of("/\\");
    const std::string dir = (lastSlash == std::string::npos) ? "." : thisFile.substr(0, lastSlash);
    std::ifstream file(dir + "/../../main/drivers/pwm_mapping.c");
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return normalizeWhitespace(buffer.str());
}

TEST(SourceSync, LiveSourceHasTheConnectorHooks)
{
    const std::string source = livePwmMappingSource();
    ASSERT_FALSE(source.empty()) << "src/main/drivers/pwm_mapping.c not found next to the tests";

    EXPECT_NE(std::string::npos, source.find(normalizeWhitespace(
        "&& !pwmHasMotorOnTimer(timOutputs, timHw->tim) && !isEscConnectorInUse(timHw)")))
        << "the servo assignment no longer skips the connector's pad";
    EXPECT_NE(std::string::npos, source.find(normalizeWhitespace(
        "if (isEscConnectorInUse(&timerHardware[idx])) { "
        "timerHardware[idx].usageFlags &= ~(TIM_USE_SERVO | TIM_USE_LED | TIM_USE_BEEPER | TIM_USE_PINIO); }")))
        << "the connector's pad no longer ends without its servo, LED, beeper and PINIO flags";
}

} // namespace
