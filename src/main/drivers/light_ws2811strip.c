/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */


/*
 * "Note that the timing on the WS2812/WS2812B LEDs has changed as of batches from WorldSemi
 * manufactured made in October 2013, and timing tolerance for approx 10-30% of parts is very small.
 * Recommendation from WorldSemi is now: 0 = 400ns high/850ns low, and 1 = 850ns high, 400ns low"
 *
 * Currently the timings are 0 = 350ns high/800ns and 1 = 700ns high/650ns low.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <platform.h>

#ifdef USE_LED_STRIP

#include "build/build_config.h"
#include "build/debug.h"

#include "common/color.h"
#include "common/colorconversion.h"
#include "common/maths.h"
#include "common/time.h"

#include "drivers/dma.h"
#include "drivers/io.h"
#include "drivers/time.h"
#include "drivers/timer.h"
#include "drivers/timer_impl.h"
#include "drivers/light_ws2811strip.h"

#include "fc/runtime_config.h"

#define WS2811_PERIOD (WS2811_TIMER_HZ / WS2811_CARRIER_HZ)
#define WS2811_BIT_COMPARE_1 ((WS2811_PERIOD * 2) / 3)
#define WS2811_BIT_COMPARE_0 (WS2811_PERIOD / 3)

// Circular DMA buffer: 2 halves, 1 LED group each. ws2811DMARefillCallback
// refills whichever half DMA just finished sending, so this only needs to
// hold a couple of LEDs regardless of strip length.
#define WS2811_LEDS_PER_GROUP 4
#define WS2811_GROUP_BITS (WS2811_LEDS_PER_GROUP * WS2811_BITS_PER_LED)
#define WS2811_CHUNK_BUFFER_SIZE (2 * WS2811_GROUP_BITS)

// WS2812 reset/latch is a *minimum* low duration with no upper bound, so
// this is a threshold to check against, not a preamble to budget for.
#define WS2811_RESET_US 60

// CCR is a 16-bit register on TIM3/TIM4; DMA must write it at that width or
// the high byte is left stale.
static DMA_RAM uint16_t ledStripDMABuffer[WS2811_CHUNK_BUFFER_SIZE];

static IO_t ws2811IO = IO_NONE;
static TCH_t * ws2811TCH = NULL;
static bool ws2811Initialised = false;

static hsvColor_t ledColorBuffer[WS2811_LED_STRIP_LENGTH];

void setLedHsv(uint16_t index, const hsvColor_t *color)
{
    ledColorBuffer[index] = *color;
}

void getLedHsv(uint16_t index, hsvColor_t *color)
{
    *color = ledColorBuffer[index];
}

void setLedValue(uint16_t index, const uint8_t value)
{
    ledColorBuffer[index].v = value;
}

void scaleLedValue(uint16_t index, const uint8_t scalePercent)
{
    ledColorBuffer[index].v = ((uint16_t)ledColorBuffer[index].v * scalePercent / 100);
}

void setStripColor(const hsvColor_t *color)
{
    uint16_t index;
    for (index = 0; index < WS2811_LED_STRIP_LENGTH; index++) {
        setLedHsv(index, color);
    }
}

void setStripColors(const hsvColor_t *colors)
{
    uint16_t index;
    for (index = 0; index < WS2811_LED_STRIP_LENGTH; index++) {
        setLedHsv(index, colors++);
    }
}

static void ws2811DMARefillCallback(TCH_t * tch, bool transferComplete);

bool ledConfigureDMA(void) {
    /* Compute the prescaler value */
    uint8_t period = WS2811_TIMER_HZ / WS2811_CARRIER_HZ;

    timerConfigBase(ws2811TCH, period, WS2811_TIMER_HZ);
    timerPWMConfigChannel(ws2811TCH, 0);

    return timerPWMConfigChannelDMA(ws2811TCH, ledStripDMABuffer, sizeof(ledStripDMABuffer[0]), WS2811_CHUNK_BUFFER_SIZE);
}

// Number of LEDs in the most recent transfer; bounds the DMA transfer (and
// ws2811SetIdleHigh's target) to what's actually configured.
static uint16_t activeLedCount = WS2811_LED_STRIP_LENGTH;

// groupInHalf[i]: group index currently in half i, so the refill callback
// can tell when the group it just finished sending was the last one.
static uint16_t totalGroups;
static uint16_t nextGroupToAssign;
static uint16_t groupInHalf[2];

// Shared between normal transfer completion and ws2811SetIdleHigh (PINIO).
static bool lineIdleLow = true;
static timeUs_t lastLowAtUs = 0;

void ws2811LedStripInit(void)
{
    const timerHardware_t * timHw = timerGetByTag(IO_TAG(WS2811_PIN), TIM_USE_ANY);

    if (!(timHw->usageFlags & TIM_USE_LED)) { // Check if it has not been reassigned
        timHw = timerGetByUsageFlag(TIM_USE_LED); // Get first pin marked as LED
    }

    if (timHw == NULL) {
        return;
    }

    ws2811TCH = timerGetTCH(timHw);
    if (ws2811TCH == NULL) {
        return;
    }

    impl_timerPWMSetDMARefillCallback(ws2811TCH, ws2811DMARefillCallback);

    ws2811IO = IOGetByTag(timHw->tag); //IOGetByTag(IO_TAG(WS2811_PIN));
    IOInit(ws2811IO, OWNER_LED_STRIP, RESOURCE_OUTPUT, 0);
    IOConfigGPIOAF(ws2811IO, IOCFG_AF_PP_FAST, timHw->alternateFunction);

    if (!ledConfigureDMA()) {
        // If DMA failed - abort
        ws2811Initialised = false;
        return;
    }

    // Zero out DMA buffer — LED pin idles LOW between WS2812 bursts
    memset(&ledStripDMABuffer, 0, sizeof(ledStripDMABuffer));
    lineIdleLow = true;
    lastLowAtUs = micros();
    ws2811Initialised = true;

    ws2811UpdateStrip(WS2811_LED_STRIP_LENGTH);
}

bool isWS2811LedStripReady(void)
{
    return !timerPWMDMAInProgress(ws2811TCH);
}

static void writeLedBits(uint16_t *dest, const rgbColor24bpp_t *color)
{
    uint32_t grb = (color->rgb.g << 16) | (color->rgb.r << 8) | (color->rgb.b);

    for (int8_t index = 23; index >= 0; index--) {
        *dest++ = (grb & (1 << index)) ? WS2811_BIT_COMPARE_1 : WS2811_BIT_COMPARE_0;
    }
}

// Slots at or beyond activeLedCount get a direct "off" write instead of a
// color lookup, so a group is self-contained regardless of where the
// configured strip actually ends.
static void ws2811FillGroup(uint8_t halfIndex, uint16_t groupIndex)
{
    uint16_t *half = &ledStripDMABuffer[halfIndex * WS2811_GROUP_BITS];
    uint16_t baseLed = groupIndex * WS2811_LEDS_PER_GROUP;

    for (uint8_t slot = 0; slot < WS2811_LEDS_PER_GROUP; slot++) {
        uint16_t *dest = &half[slot * WS2811_BITS_PER_LED];
        uint16_t ledIdx = baseLed + slot;

        if (ledIdx < activeLedCount) {
            writeLedBits(dest, hsvToRgb24(&ledColorBuffer[ledIdx]));
        } else {
            for (uint8_t bit = 0; bit < WS2811_BITS_PER_LED; bit++) {
                dest[bit] = WS2811_BIT_COMPARE_0;
            }
        }
    }
}

static void ws2811RefillHalf(uint8_t halfIndex)
{
    ws2811FillGroup(halfIndex, nextGroupToAssign);
    groupInHalf[halfIndex] = nextGroupToAssign;
    nextGroupToAssign++;
}

// CCR is preload/shadow-buffered, so the direct write below takes effect
// cleanly at the next period boundary without needing further DMA.
static void ws2811StopTransfer(void)
{
    timerPWMStopDMA(ws2811TCH);
    *timerCCR(ws2811TCH) = 0;
    lineIdleLow = true;
    lastLowAtUs = micros();
}

// transferComplete: true = half 1 just finished (DMA wrapped to half 0),
// false = half 0 just finished (DMA moved on to half 1).
static void ws2811DMARefillCallback(TCH_t * tch, bool transferComplete)
{
    (void)tch;

    uint8_t finishedHalf = transferComplete ? 1 : 0;

    if (groupInHalf[finishedHalf] == totalGroups - 1) {
        ws2811StopTransfer();
        return;
    }

    ws2811RefillHalf(finishedHalf);
}

static void ws2811EnsureResetGap(void)
{
    // A gap before real data is always safe regardless of length — only a
    // mid-frame gap (prevented by true circular DMA) risks looking like a
    // premature reset — so usually this is just a compare, not a wait.
    if (!lineIdleLow || cmpTimeUs(micros(), lastLowAtUs) < WS2811_RESET_US) {
        *timerCCR(ws2811TCH) = 0;
        lineIdleLow = true;
        delayMicroseconds(WS2811_RESET_US);
        lastLowAtUs = micros();
    }
}

// Non-blocking except when the line was left idle-high by PINIO or updates
// are requested faster than the reset window allows. LEDs are transmitted
// in the background via the DMA refill callback.
void ws2811UpdateStrip(uint16_t usedLedCount)
{
    if (!ws2811Initialised || !ws2811TCH) {
        return;
    }

    // don't wait - risk of infinite block, just get an update next time round
    if (timerPWMDMAInProgress(ws2811TCH)) {
        return;
    }

    activeLedCount = MIN(usedLedCount, (uint16_t)WS2811_LED_STRIP_LENGTH);
    if (activeLedCount == 0) {
        return;
    }

    ws2811EnsureResetGap();

    totalGroups = (activeLedCount + WS2811_LEDS_PER_GROUP - 1) / WS2811_LEDS_PER_GROUP;
    nextGroupToAssign = 0;
    ws2811RefillHalf(0);
    ws2811RefillHalf(1);

    impl_timerPWMSetDMACircular(ws2811TCH, true, WS2811_CHUNK_BUFFER_SIZE);
}

void ws2811SetIdleHigh(bool high)
{
    lineIdleLow = !high;
    *timerCCR(ws2811TCH) = high ? 255 : 0;
    if (!high) {
        lastLowAtUs = micros();
    }
}

#endif
