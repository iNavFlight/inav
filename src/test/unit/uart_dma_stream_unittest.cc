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
 * Which DMA streams a UART may take: pwmIsDmaStreamReserved() (pwm_mapping.c) and uartDmaStreamAvailable()
 * (serial_uart_impl.h). pwm_mapping.c is not built for the host, so, as in pwm_mapping_beeper_unittest.cc, the
 * logic is mirrored on plain data and a SourceSync test checks that the live function still reads the same.
 */

#include <stdint.h>
#include <stdbool.h>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>

#include "gtest/gtest.h"
#include "unittest_macros.h"

namespace {

enum { TIM_USE_MOTOR = 1 << 2, TIM_USE_SERVO = 1 << 3, TIM_USE_LED = 1 << 24 };
enum { OUTPUT_MODE_AUTO = 0, OUTPUT_MODE_LED = 3 };
enum owner_e { OWNER_FREE, OWNER_TIMER, OWNER_SERIAL };

struct pad_t {
    int timer;          // index into the per-timer output modes
    int stream;         // the DMA stream the pad is mapped to, -1 for none
    uint32_t usageFlags;
};

struct board_t {
    const pad_t *pads;
    int padCount;
    uint8_t outputMode[4];
    bool dshot;
    bool ledStrip;
    bool motorsInitialised;
};

// Mirror of pwmIsDmaStreamReserved()
bool reserved(const board_t &b, int stream)
{
    const bool motorsWillClaim = !b.motorsInitialised && b.dshot;

    for (int i = 0; i < b.padCount; i++) {
        const pad_t *timHw = &b.pads[i];
        if (timHw->stream != stream) {
            continue;
        }
        if (motorsWillClaim) {
            return true;
        }
        if (b.ledStrip && ((timHw->usageFlags & TIM_USE_LED) || b.outputMode[timHw->timer] == OUTPUT_MODE_LED)) {
            return true;
        }
    }
    return false;
}

// Mirror of uartDmaStreamAvailable()
bool available(const board_t &b, int stream, owner_e owner, int ownerIndex, int device)
{
    if (reserved(b, stream)) {
        return false;
    }
    return owner == OWNER_FREE || (owner == OWNER_SERIAL && ownerIndex == device);
}

// Four motor-capable outputs on streams 0-3, a LED pad on stream 4 (timer 2), stream 6 unmapped
const pad_t pads[] = {
    { 0, 0, TIM_USE_MOTOR | TIM_USE_SERVO },
    { 0, 1, TIM_USE_MOTOR | TIM_USE_SERVO },
    { 1, 2, TIM_USE_MOTOR | TIM_USE_SERVO },
    { 1, 3, TIM_USE_MOTOR | TIM_USE_SERVO },
    { 2, 4, TIM_USE_LED },
};

board_t board(bool dshot, bool ledStrip, bool motorsInitialised)
{
    return board_t{ pads, (int)(sizeof(pads) / sizeof(pads[0])), { OUTPUT_MODE_AUTO, OUTPUT_MODE_AUTO, OUTPUT_MODE_AUTO, OUTPUT_MODE_AUTO },
                    dshot, ledStrip, motorsInitialised };
}

} // namespace

TEST(UartDmaStream, DshotMotorsNotStartedReserveEveryOutputStream)
{
    const board_t b = board(true, false, false);
    for (int s = 0; s <= 4; s++) {
        EXPECT_TRUE(reserved(b, s)) << "stream " << s;
    }
    EXPECT_FALSE(reserved(b, 6));
}

TEST(UartDmaStream, StartedMotorsLeaveTheRestToOwnership)
{
    const board_t b = board(true, false, true);
    for (int s = 0; s <= 4; s++) {
        EXPECT_FALSE(reserved(b, s)) << "stream " << s;
    }
    // The started motors own their streams; an unused output's stream is free
    EXPECT_FALSE(available(b, 0, OWNER_TIMER, 0, 5));
    EXPECT_TRUE(available(b, 3, OWNER_FREE, 0, 5));
}

TEST(UartDmaStream, MotorsOffDshotNeverReserveMotorStreams)
{
    const board_t b = board(false, false, false);
    for (int s = 0; s <= 4; s++) {
        EXPECT_FALSE(reserved(b, s)) << "stream " << s;
    }
}

TEST(UartDmaStream, LedStripKeepsItsStreamOnlyWhenEnabled)
{
    EXPECT_TRUE(reserved(board(false, true, false), 4));
    EXPECT_TRUE(reserved(board(true, true, true), 4));
    EXPECT_FALSE(reserved(board(false, false, false), 4));
    EXPECT_FALSE(reserved(board(true, false, true), 4));
}

TEST(UartDmaStream, LedOutputModeReservesTheTimersPads)
{
    board_t b = board(false, true, true);
    b.outputMode[1] = OUTPUT_MODE_LED;
    EXPECT_TRUE(reserved(b, 2));
    EXPECT_TRUE(reserved(b, 3));
    EXPECT_FALSE(reserved(b, 0));
}

TEST(UartDmaStream, OwnershipStillDecidesFreeStreams)
{
    const board_t b = board(false, false, true);
    EXPECT_TRUE(available(b, 6, OWNER_FREE, 0, 5));
    EXPECT_TRUE(available(b, 6, OWNER_SERIAL, 5, 5));
    EXPECT_FALSE(available(b, 6, OWNER_SERIAL, 2, 5));
    EXPECT_FALSE(available(b, 6, OWNER_TIMER, 0, 5));
}

/*
 * SourceSync: the live pwmIsDmaStreamReserved() must read as below, whitespace and line comments aside, so the
 * mirror above keeps testing the real logic.
 */
namespace {

std::string normalize(const std::string &text)
{
    std::string noComments;
    for (size_t i = 0; i < text.size();) {
        if (text[i] == '/' && i + 1 < text.size() && text[i + 1] == '/') {
            while (i < text.size() && text[i] != '\n') {
                i++;
            }
            continue;
        }
        noComments += text[i++];
    }
    std::string result;
    bool lastWasSpace = true;
    for (char c : noComments) {
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

const char *expectedFunction = R"(
bool pwmIsDmaStreamReserved(DMA_t dma)
{
    const bool motorsWillClaim = !motorsInitialised && getMotorProtocolProperties(motorConfig()->motorPwmProtocol)->isDSHOT;

    for (int i = 0; i < timerHardwareCount; i++) {
        const timerHardware_t *timHw = &timerHardware[i];
        if (dmaGetByTag(timHw->dmaTag) != dma) {
            continue;
        }
        if (motorsWillClaim) {
            return true;
        }
        if (feature(FEATURE_LED_STRIP) && (TIM_IS_LED(timHw->usageFlags) || timerOverrides(timer2id(timHw->tim))->outputMode == OUTPUT_MODE_LED)) {
            return true;
        }
    }
    return false;
}
)";

const char *expectedAvailable = R"(
static inline bool uartDmaStreamAvailable(DMA_t dma, UARTDevice_e device)
{
    if (pwmIsDmaStreamReserved(dma)) {
        return false;
    }
    return dmaGetOwner(dma) == OWNER_FREE || (dmaGetOwner(dma) == OWNER_SERIAL && dma->resourceIndex == RESOURCE_INDEX(device));
}
)";

::testing::AssertionResult liveSourceContains(const char *relativePath, const char *snippet)
{
    std::string thisFile = __FILE__;
    const size_t slash = thisFile.find_last_of("/\\");
    const std::string path = (slash == std::string::npos ? std::string(".") : thisFile.substr(0, slash))
        + "/../../main/drivers/" + relativePath;
    std::ifstream file(path);
    if (!file.is_open()) {
        return ::testing::AssertionFailure() << "cannot open " << path;
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    if (normalize(buffer.str()).find(normalize(snippet)) == std::string::npos) {
        return ::testing::AssertionFailure() << relativePath << " changed: update the mirror in this test";
    }
    return ::testing::AssertionSuccess();
}

} // namespace

TEST(UartDmaStreamSourceSync, ReservedMatchesMirror)
{
    EXPECT_TRUE(liveSourceContains("pwm_mapping.c", expectedFunction));
}

TEST(UartDmaStreamSourceSync, AvailableMatchesMirror)
{
    EXPECT_TRUE(liveSourceContains("serial_uart_impl.h", expectedAvailable));
}
