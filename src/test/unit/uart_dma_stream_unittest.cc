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
bool available(const board_t &b, int stream, owner_e owner)
{
    if (reserved(b, stream)) {
        return false;
    }
    return owner == OWNER_FREE;
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
    EXPECT_FALSE(available(b, 0, OWNER_TIMER));
    EXPECT_TRUE(available(b, 3, OWNER_FREE));
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
    EXPECT_TRUE(available(b, 6, OWNER_FREE));
    // A port holds only the streams it runs on, this one's included: every stop gives its stream back
    EXPECT_FALSE(available(b, 6, OWNER_SERIAL));
    EXPECT_FALSE(available(b, 6, OWNER_TIMER));
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
static inline bool uartDmaStreamAvailable(DMA_t dma)
{
    if (pwmIsDmaStreamReserved(dma)) {
        return false;
    }
    return dmaGetOwner(dma) == OWNER_FREE;
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

/*
 * uartDmaPick() (serial_uart_impl.h): the stream a port starts on. Mirrored on plain tags: a stream is "unavailable"
 * when uartDmaStreamAvailable() would refuse it, "used elsewhere" when it is the SD card's (or the AT32 ADC's).
 */
namespace {

const uint32_t TAG_NONE = 0;
const uint32_t TAG_AUTO = 0xFFFF0000U;

uint32_t tag(int dma, int stream, int channel)
{
    return ((dma & 0x03) << 12) | ((stream & 0x0F) << 8) | (channel & 0xFF);
}

// dmaGetByTag() matches controller and stream; a tag with no controller has no stream
int streamOf(uint32_t t)
{
    return ((t >> 12) & 0x03) ? (int)((t >> 8) & 0x3F) : -1;
}

struct dmaWorld_t {
    int unavailable[4];
    int usedElsewhere[2];
    int named[2];           // streams a target names for some port
};

bool listed(const int *list, int count, int stream)
{
    for (int i = 0; i < count; i++) {
        if (list[i] == stream) {
            return true;
        }
    }
    return false;
}

const uint32_t rxCandidates[8][2] = {
    { tag(2, 2, 4), tag(2, 5, 4) }, { tag(1, 5, 4) }, { tag(1, 1, 4) }, { tag(1, 2, 4) },
    { tag(1, 0, 4) }, { tag(2, 1, 5), tag(2, 2, 5) }, { tag(1, 3, 5) }, { tag(1, 6, 5) },
};
const uint32_t txCandidates[8][2] = {
    { tag(2, 7, 4) }, { tag(1, 6, 4) }, { tag(1, 3, 4), tag(1, 4, 7) }, { tag(1, 4, 4) },
    { tag(1, 7, 4) }, { tag(2, 6, 5), tag(2, 7, 5) }, { tag(1, 1, 5) }, { tag(1, 0, 5) },
};

// Mirror of uartDmaPick() for F4/F7; bursts is SERIAL_RX_BURSTS
uint32_t pick(uint32_t named, const uint32_t (*candidates)[2], int device, bool rxCallback, bool halfDuplex,
              int otherDirection, const dmaWorld_t &w, bool bursts = false)
{
    if (named != TAG_AUTO) {
        const int dma = streamOf(named);
        return (named != TAG_NONE && dma >= 0 && !listed(w.unavailable, 4, dma)) ? named : TAG_NONE;
    }
    if ((rxCallback && !(bursts && !halfDuplex)) || halfDuplex) {
        return TAG_NONE;
    }
    for (int i = 0; i < 2; i++) {
        const uint32_t t = candidates[device][i];
        const int dma = streamOf(t);
        if (t != TAG_NONE && dma >= 0 && dma != otherDirection && !listed(w.usedElsewhere, 2, dma) && !listed(w.named, 2, dma) &&
            !listed(w.unavailable, 4, dma)) {
            return t;
        }
    }
    return TAG_NONE;
}

const dmaWorld_t freeWorld = { { -1, -1, -1, -1 }, { -1, -1 }, { -1, -1 } };

int s(int dma, int stream)
{
    return streamOf(tag(dma, stream, 0));
}

} // namespace

TEST(UartDmaPick, NamedStreamIsUsedOnlyWhenFree)
{
    EXPECT_EQ(tag(1, 5, 4), pick(tag(1, 5, 4), rxCandidates, 1, false, false, -1, freeWorld));
    const dmaWorld_t taken = { { s(1, 5), -1, -1, -1 }, { -1, -1 }, { -1, -1 } };
    EXPECT_EQ(TAG_NONE, pick(tag(1, 5, 4), rxCandidates, 1, false, false, -1, taken));
    EXPECT_EQ(TAG_NONE, pick(TAG_NONE, rxCandidates, 1, false, false, -1, freeWorld));
}

TEST(UartDmaPick, NamedStreamAlsoServesReceiversAndHalfDuplex)
{
    EXPECT_EQ(tag(1, 6, 4), pick(tag(1, 6, 4), txCandidates, 1, true, true, -1, freeWorld));
}

TEST(UartDmaPick, AutoLeavesReceiversAndHalfDuplexOnTheInterrupt)
{
    EXPECT_EQ(TAG_NONE, pick(TAG_AUTO, rxCandidates, 0, true, false, -1, freeWorld));
    EXPECT_EQ(TAG_NONE, pick(TAG_AUTO, txCandidates, 0, false, true, -1, freeWorld));
}

TEST(UartDmaPick, AutoGivesAReceiverTakingBurstsAStream)
{
    EXPECT_EQ(tag(2, 2, 4), pick(TAG_AUTO, rxCandidates, 0, true, false, -1, freeWorld, true));
    EXPECT_EQ(tag(2, 7, 4), pick(TAG_AUTO, txCandidates, 0, true, false, -1, freeWorld, true));
}

TEST(UartDmaPick, AutoLeavesAHalfDuplexReceiverOnTheInterruptEvenTakingBursts)
{
    EXPECT_EQ(TAG_NONE, pick(TAG_AUTO, rxCandidates, 0, true, true, -1, freeWorld, true));
}

TEST(UartDmaPick, AutoTakesTheFirstFreeCandidate)
{
    EXPECT_EQ(tag(2, 2, 4), pick(TAG_AUTO, rxCandidates, 0, false, false, -1, freeWorld));
    const dmaWorld_t firstTaken = { { s(2, 2), -1, -1, -1 }, { -1, -1 }, { -1, -1 } };
    EXPECT_EQ(tag(2, 5, 4), pick(TAG_AUTO, rxCandidates, 0, false, false, -1, firstTaken));
    const dmaWorld_t bothTaken = { { s(2, 2), s(2, 5), -1, -1 }, { -1, -1 }, { -1, -1 } };
    EXPECT_EQ(TAG_NONE, pick(TAG_AUTO, rxCandidates, 0, false, false, -1, bothTaken));
}

TEST(UartDmaPick, AutoNeverTakesAnEmptySecondCandidate)
{
    const dmaWorld_t taken = { { s(1, 5), -1, -1, -1 }, { -1, -1 }, { -1, -1 } };
    EXPECT_EQ(TAG_NONE, pick(TAG_AUTO, rxCandidates, 1, false, false, -1, taken));
}

TEST(UartDmaPick, AutoNeverTakesAStreamAnotherPortNames)
{
    // UART6 names DMA2 stream 2 for its receiver: UART1 moves on to stream 5 even when it opens first
    const dmaWorld_t named = { { -1, -1, -1, -1 }, { -1, -1 }, { s(2, 2), -1 } };
    EXPECT_EQ(tag(2, 5, 4), pick(TAG_AUTO, rxCandidates, 0, false, false, -1, named));
}

TEST(UartDmaPick, AutoSkipsTheOtherDirectionAndTheSdCard)
{
    EXPECT_EQ(tag(2, 7, 5), pick(TAG_AUTO, txCandidates, 5, false, false, s(2, 6), freeWorld));
    const dmaWorld_t sdcard = { { -1, -1, -1, -1 }, { s(2, 6), -1 }, { -1, -1 } };
    EXPECT_EQ(tag(2, 7, 5), pick(TAG_AUTO, txCandidates, 5, false, false, -1, sdcard));
}

namespace {

const char *expectedPick = R"(
static inline dmaTag_t uartDmaPick(dmaTag_t named, const dmaTag_t (*candidates)[UART_DMA_CANDIDATES], UARTDevice_e device,
                                   const serialPort_t *port, DMA_t otherDirection)
{
    if (named != DMA_TAG_AUTO) {
        const DMA_t dma = dmaGetByTag(named);
        return (named != DMA_NONE && dma && uartDmaStreamAvailable(dma)) ? named : DMA_NONE;
    }
    if ((port->rxCallback && !uartRxTakesBursts(port)) || (port->options & SERIAL_BIDIR)) {
        return DMA_NONE;
    }
    for (int i = 0; i < UART_DMA_CANDIDATES; i++) {
        const dmaTag_t tag = uartDmaCandidate(candidates, device, i);
        const DMA_t dma = dmaGetByTag(tag);
        if (tag != DMA_NONE && dma && dma != otherDirection && !uartDmaStreamUsedElsewhere(dma) && !uartDmaStreamNamed(dma) &&
            uartDmaStreamAvailable(dma)) {
            return tag;
        }
    }
    UNUSED(candidates);
    UNUSED(device);
    return DMA_NONE;
}
)";

const char *expectedCandidates = R"(
static const dmaTag_t uartRxDmaCandidates[UARTDEV_MAX][UART_DMA_CANDIDATES] = {
    [UARTDEV_1] = { DMA_TAG(2, 2, 4), DMA_TAG(2, 5, 4) },
    [UARTDEV_2] = { DMA_TAG(1, 5, 4) },
    [UARTDEV_3] = { DMA_TAG(1, 1, 4) },
    [UARTDEV_4] = { DMA_TAG(1, 2, 4) },
    [UARTDEV_5] = { DMA_TAG(1, 0, 4) },
    [UARTDEV_6] = { DMA_TAG(2, 1, 5), DMA_TAG(2, 2, 5) },
    [UARTDEV_7] = { DMA_TAG(1, 3, 5) },
    [UARTDEV_8] = { DMA_TAG(1, 6, 5) },
};
static const dmaTag_t uartTxDmaCandidates[UARTDEV_MAX][UART_DMA_CANDIDATES] = {
    [UARTDEV_1] = { DMA_TAG(2, 7, 4) },
    [UARTDEV_2] = { DMA_TAG(1, 6, 4) },
    [UARTDEV_3] = { DMA_TAG(1, 3, 4), DMA_TAG(1, 4, 7) },
    [UARTDEV_4] = { DMA_TAG(1, 4, 4) },
    [UARTDEV_5] = { DMA_TAG(1, 7, 4) },
    [UARTDEV_6] = { DMA_TAG(2, 6, 5), DMA_TAG(2, 7, 5) },
    [UARTDEV_7] = { DMA_TAG(1, 1, 5) },
    [UARTDEV_8] = { DMA_TAG(1, 0, 5) },
};
)";

const char *expectedTakesBursts = R"(
#define uartRxTakesBursts(port)     (((port)->options & (SERIAL_RX_BURSTS | SERIAL_BIDIR)) == SERIAL_RX_BURSTS)
)";

// Not mirrored: checked so that a change to them shows up here
const char *expectedStreamList = R"(
static const dmaTag_t uartDmaStreams[UART_DMA_CANDIDATES] = {
    DMA_TAG(1, 0, 0), DMA_TAG(1, 1, 0), DMA_TAG(1, 2, 0), DMA_TAG(1, 3, 0), DMA_TAG(1, 4, 0), DMA_TAG(1, 5, 0),
    DMA_TAG(1, 6, 0), DMA_TAG(1, 7, 0), DMA_TAG(2, 3, 0), DMA_TAG(2, 4, 0), DMA_TAG(2, 5, 0), DMA_TAG(2, 6, 0),
    DMA_TAG(2, 7, 0),
};
)";

const char *expectedNamed = R"(
static inline bool uartDmaStreamNamed(DMA_t dma)
{
    for (unsigned i = 0; i < ARRAYLEN(uartNamedDmaTags); i++) {
        if (uartNamedDmaTags[i] != DMA_NONE && dma == dmaGetByTag(uartNamedDmaTags[i])) {
            return true;
        }
    }
    return false;
}
)";

} // namespace

TEST(UartDmaStreamSourceSync, H7StreamListMatches)
{
    EXPECT_TRUE(liveSourceContains("serial_uart_impl.h", expectedStreamList));
}

TEST(UartDmaStreamSourceSync, NamedMatches)
{
    EXPECT_TRUE(liveSourceContains("serial_uart_impl.h", expectedNamed));
}

// Without it a closed or reopened port would keep a stream the pick above counts as taken
TEST(UartDmaStreamSourceSync, EveryStopGivesItsStreamBack)
{
    for (const char *file : { "serial_uart_stm32f4xx.c", "serial_uart_stm32f7xx.c", "serial_uart_stm32h7xx.c", "serial_uart_at32f43x.c" }) {
        EXPECT_TRUE(liveSourceContains(file, "uartDmaRelease(s->rxDma); s->rxDma = NULL;"));
        EXPECT_TRUE(liveSourceContains(file, "uartDmaRelease(s->txDma); s->txDma = NULL;"));
    }
}

TEST(UartDmaStreamSourceSync, PickMatchesMirror)
{
    EXPECT_TRUE(liveSourceContains("serial_uart_impl.h", expectedPick));
    EXPECT_TRUE(liveSourceContains("serial_uart_impl.h", expectedTakesBursts));
}

TEST(UartDmaStreamSourceSync, CandidateTablesMatchMirror)
{
    EXPECT_TRUE(liveSourceContains("serial_uart_impl.h", expectedCandidates));
}

namespace {

const char *expectedUsedElsewhere = R"(
static inline bool uartDmaStreamUsedElsewhere(DMA_t dma)
{
#if (defined(STM32F4) || defined(STM32F7)) && defined(USE_SDCARD_SDIO)
#ifdef SDCARD_SDIO_DMA
    if (dma == dmaGetByTag(SDCARD_SDIO_DMA)) {
        return true;
    }
#else
    if (dma == dmaGetByTag(DMA_TAG(2, 3, 0)) || dma == dmaGetByTag(DMA_TAG(2, 6, 0))) {
        return true;
    }
#endif
#endif
#if defined(AT32F43x)
#ifdef ADC1_DMA_STREAM
    if (dma == dmaGetByRef(ADC1_DMA_STREAM)) {
#else
    if (dma == dmaGetByRef(DMA2_CHANNEL1)) {
#endif
        return true;
    }
#endif
    UNUSED(dma);
    return false;
}
)";

const char *expectedAt32StreamList = R"(
static const dmaTag_t uartDmaStreams[UART_DMA_CANDIDATES] = {
    DMA_TAG(1, 1, 0), DMA_TAG(1, 2, 0), DMA_TAG(1, 3, 0), DMA_TAG(1, 4, 0), DMA_TAG(1, 5, 0), DMA_TAG(1, 6, 0),
    DMA_TAG(1, 7, 0), DMA_TAG(2, 1, 0), DMA_TAG(2, 2, 0), DMA_TAG(2, 3, 0), DMA_TAG(2, 4, 0), DMA_TAG(2, 5, 0),
    DMA_TAG(2, 6, 0), DMA_TAG(2, 7, 0),
};
)";

// Built rather than pasted, so a UART or a direction left out shows up
std::string streamName(int uart, const char *direction)
{
    return "UART" + std::to_string(uart) + "_" + direction + "_DMA";
}

std::string expectedNamedList()
{
    std::string text = "static const dmaTag_t uartNamedDmaTags[] = {\n";
    for (int uart = 1; uart <= 8; uart++) {
        for (const char *direction : { "RX", "TX" }) {
            const std::string name = streamName(uart, direction);
            text += "#if defined(" + name + ") && (" + name + " != DMA_TAG_AUTO) && (" + name + " != DMA_NONE)\n    " + name + ",\n#endif\n";
        }
    }
    return text + "    DMA_NONE,\n};\n";   // keeps the array from being empty
}

std::string expectedAutoDefaults()
{
    std::string text = "#if defined(STM32F4) || defined(STM32F7) || defined(STM32H7) || defined(AT32F43x)\n";
    for (int uart = 1; uart <= 8; uart++) {
        for (const char *direction : { "RX", "TX" }) {
            const std::string name = streamName(uart, direction);
            text += "#if defined(USE_UART" + std::to_string(uart) + ") && !defined(" + name + ")\n#define " + name + " DMA_TAG_AUTO\n#endif\n";
        }
    }
    return text + "#endif\n";
}

// The first uartDmaPick() call after the anchor must be the expected one: catches a direction given the other's table
::testing::AssertionResult livePickAfter(const char *relativePath, const char *anchor, const char *call)
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
    const std::string text = normalize(buffer.str());
    const size_t at = text.find(normalize(anchor));
    if (at == std::string::npos) {
        return ::testing::AssertionFailure() << relativePath << ": no " << anchor;
    }
    const size_t next = text.find("uartDmaPick(", at);
    const std::string expected = normalize(call);
    if (next == std::string::npos || text.compare(next, expected.size(), expected) != 0) {
        return ::testing::AssertionFailure() << relativePath << ": " << anchor << " no longer calls " << call;
    }
    return ::testing::AssertionSuccess();
}

} // namespace

TEST(UartDmaStreamSourceSync, UsedElsewhereMatches)
{
    EXPECT_TRUE(liveSourceContains("serial_uart_impl.h", expectedUsedElsewhere));
}

TEST(UartDmaStreamSourceSync, At32StreamListMatches)
{
    EXPECT_TRUE(liveSourceContains("serial_uart_impl.h", expectedAt32StreamList));
}

TEST(UartDmaStreamSourceSync, NamedListHasEveryPortAndDirection)
{
    EXPECT_TRUE(liveSourceContains("serial_uart_impl.h", expectedNamedList().c_str()));
}

TEST(UartDmaStreamSourceSync, EveryPortAndDirectionDefaultsToAuto)
{
    EXPECT_TRUE(liveSourceContains("../target/common_post.h", expectedAutoDefaults().c_str()));
}

TEST(UartDmaStreamSourceSync, EachDirectionPicksFromItsOwnTable)
{
    const char *rxTables = "uartDmaPick(uartRxDmaTag[device], uartRxDmaCandidates, device, &s->port, uartTxDmaOf(s))";
    const char *txTables = "uartDmaPick(uartTxDmaTag[device], uartTxDmaCandidates, device, &s->port, uartRxDmaOf(s))";
    const char *rxConfig = "uartDmaPick(uartRxDmaConfig[device].tag, NULL, device, &s->port, uartTxDmaOf(s))";
    const char *txConfig = "uartDmaPick(uartTxDmaConfig[device].tag, NULL, device, &s->port, uartRxDmaOf(s))";
    for (const char *driver : { "serial_uart_stm32f4xx.c", "serial_uart_stm32f7xx.c" }) {
        EXPECT_TRUE(livePickAfter(driver, "bool uartRxDmaStart(uartPort_t *s)", rxTables));
        EXPECT_TRUE(livePickAfter(driver, "bool uartTxDmaStart(uartPort_t *s)", txTables));
    }
    for (const char *driver : { "serial_uart_stm32h7xx.c", "serial_uart_at32f43x.c" }) {
        EXPECT_TRUE(livePickAfter(driver, "bool uartRxDmaStart(uartPort_t *s)", rxConfig));
        EXPECT_TRUE(livePickAfter(driver, "bool uartTxDmaStart(uartPort_t *s)", txConfig));
    }
}

/*
 * uartCharNs() and uartRxDmaDeliver() (serial_uart_impl.h): which bytes a burst hands over, and the time each is given.
 * Mirrored on a plain ring; the source check below keeps the mirror honest.
 */
namespace {

uint32_t charNs(uint32_t baudRate, bool parityEven, bool stopBits2)
{
    const uint32_t bits = 10 + (parityEven ? 1 : 0) + (stopBits2 ? 1 : 0);
    return bits * (1000000000U / baudRate);
}

struct handed_t {
    uint8_t byte;
    uint32_t timeUs;
};

// Mirror of uartRxDmaDeliver(): appends what the callback would get to out, returns the new tail
uint32_t deliver(const uint8_t *ring, uint32_t size, uint32_t tail, uint32_t head, uint32_t lastEndUs, uint32_t cns,
                 handed_t *out, uint32_t *count)
{
    for (uint32_t left = (head + size - tail) % size; left > 0; left--) {
        out[*count].timeUs = lastEndUs - ((left - 1) * cns) / 1000;
        out[*count].byte = ring[tail];
        (*count)++;
        tail = (tail + 1) % size;
    }
    return tail;
}

const char *expectedCharNs = R"(
static inline uint32_t uartCharNs(const uartPort_t *s)
{
    const uint32_t bits = 10 + ((s->port.options & SERIAL_PARITY_EVEN) ? 1 : 0) + ((s->port.options & SERIAL_STOPBITS_2) ? 1 : 0);
    return bits * (1000000000U / s->port.baudRate);
}
)";

const char *expectedDeliver = R"(
    const uint32_t size = s->port.rxBufferSize;
    const uint32_t charNs = uartCharNs(s);
    uint32_t tail = s->port.rxBufferTail;

    for (uint32_t left = (uartRxBufferHead(s) + size - tail) % size; left > 0; left--) {
        s->port.rxByteTimeUs = lastEndUs - ((left - 1) * charNs) / 1000;
        s->port.rxCallback(s->port.rxBuffer[tail], s->port.rxCallbackData);
        tail = (tail + 1) % size;
    }
    s->port.rxBufferTail = tail;
)";

} // namespace

TEST(UartRxDmaDeliver, CharacterTimesOfCrsfAndSbus)
{
    EXPECT_EQ(23800U, charNs(420000, false, false));    // 8N1, 23.8 us
    EXPECT_EQ(120000U, charNs(100000, true, true));     // 8E2, 120 us
}

TEST(UartRxDmaDeliver, BurstIsTimedBackFromItsEnd)
{
    uint8_t ring[256];
    for (int i = 0; i < 256; i++) {
        ring[i] = (uint8_t)i;
    }
    handed_t out[256];
    uint32_t count = 0;

    // A 26 byte CRSF frame at 420000 baud, its last byte ending at 10000 us
    EXPECT_EQ(26U, deliver(ring, 256, 0, 26, 10000, charNs(420000, false, false), out, &count));
    ASSERT_EQ(26U, count);
    EXPECT_EQ(0, out[0].byte);
    EXPECT_EQ(10000U - 595U, out[0].timeUs);            // 25 characters before the last
    EXPECT_EQ(10000U - 23U, out[24].timeUs);
    EXPECT_EQ(10000U, out[25].timeUs);
}

TEST(UartRxDmaDeliver, BurstWrapsRoundTheRing)
{
    uint8_t ring[256];
    for (int i = 0; i < 256; i++) {
        ring[i] = (uint8_t)i;
    }
    handed_t out[256];
    uint32_t count = 0;

    EXPECT_EQ(4U, deliver(ring, 256, 250, 4, 5000, charNs(100000, true, true), out, &count));
    ASSERT_EQ(10U, count);
    EXPECT_EQ(250, out[0].byte);
    EXPECT_EQ(255, out[5].byte);
    EXPECT_EQ(0, out[6].byte);
    EXPECT_EQ(3, out[9].byte);
    EXPECT_EQ(5000U - 9U * 120U, out[0].timeUs);
}

TEST(UartRxDmaDeliver, NothingNewHandsNothing)
{
    uint8_t ring[256] = { 0 };
    handed_t out[1];
    uint32_t count = 0;

    EXPECT_EQ(17U, deliver(ring, 256, 17, 17, 5000, charNs(420000, false, false), out, &count));
    EXPECT_EQ(0U, count);
}

TEST(UartDmaStreamSourceSync, DeliverMatchesMirror)
{
    EXPECT_TRUE(liveSourceContains("serial_uart_impl.h", expectedCharNs));
    EXPECT_TRUE(liveSourceContains("serial_uart_impl.h", expectedDeliver));
}
