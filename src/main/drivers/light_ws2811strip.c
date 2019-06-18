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

#include "drivers/dma.h"
#include "drivers/io.h"
#include "drivers/timer.h"
#include "drivers/light_ws2811strip.h"

#define WS2811_PERIOD (WS2811_TIMER_HZ / WS2811_CARRIER_HZ)
#define WS2811_BIT_COMPARE_1 ((WS2811_PERIOD * 2) / 3)
#define WS2811_BIT_COMPARE_0 (WS2811_PERIOD / 3)

static timerDMASafeType_t ledStripDMABuffer[WS2811_DMA_BUFFER_SIZE];

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

void ws2811LedStripInit(void)
{
    const timerHardware_t * timHw = timerGetByTag(IO_TAG(WS2811_PIN), TIM_USE_ANY);

    if (timHw == NULL) {
        return;
    }

    ws2811TCH = timerGetTCH(timHw);
    if (ws2811TCH == NULL) {
        return;
    }

    /* Compute the prescaler value */
    uint8_t period = WS2811_TIMER_HZ / WS2811_CARRIER_HZ;

    ws2811IO = IOGetByTag(IO_TAG(WS2811_PIN));
    IOInit(ws2811IO, OWNER_LED_STRIP, RESOURCE_OUTPUT, 0);
    IOConfigGPIOAF(ws2811IO, IOCFG_AF_PP_FAST, timHw->alternateFunction);

    timerConfigBase(ws2811TCH, period, WS2811_TIMER_HZ);
    timerPWMConfigChannel(ws2811TCH, 0);

    // If DMA failed - abort
    if (!timerPWMConfigChannelDMA(ws2811TCH, ledStripDMABuffer, sizeof(ledStripDMABuffer[0]), WS2811_DMA_BUFFER_SIZE)) {
        ws2811Initialised = false;
        return;
    }

    // Zero out DMA buffer
    memset(&ledStripDMABuffer, 0, sizeof(ledStripDMABuffer));
    ws2811Initialised = true;

    ws2811UpdateStrip();
}

bool isWS2811LedStripReady(void)
{
    return !timerPWMDMAInProgress(ws2811TCH);
}

STATIC_UNIT_TESTED uint16_t dmaBufferOffset;
static int16_t ledIndex;

STATIC_UNIT_TESTED void fastUpdateLEDDMABuffer(rgbColor24bpp_t *color)
{
    uint32_t grb = (color->rgb.g << 16) | (color->rgb.r << 8) | (color->rgb.b);

    for (int8_t index = 23; index >= 0; index--) {
        ledStripDMABuffer[dmaBufferOffset++] = (grb & (1 << index)) ? WS2811_BIT_COMPARE_1 : WS2811_BIT_COMPARE_0;
    }
}

/*
 * This method is non-blocking unless an existing LED update is in progress.
 * it does not wait until all the LEDs have been updated, that happens in the background.
 */
void ws2811UpdateStrip(void)
{
    static rgbColor24bpp_t *rgb24;

    // don't wait - risk of infinite block, just get an update next time round
    if (timerPWMDMAInProgress(ws2811TCH)) {
        return;
    }

    dmaBufferOffset = 0;                // reset buffer memory index
    ledIndex = 0;                       // reset led index

    // fill transmit buffer with correct compare values to achieve
    // correct pulse widths according to color values
    while (ledIndex < WS2811_LED_STRIP_LENGTH)
    {
        rgb24 = hsvToRgb24(&ledColorBuffer[ledIndex]);
        fastUpdateLEDDMABuffer(rgb24);
        ledIndex++;
    }

    // Initiate hardware transfer
    if (!ws2811Initialised || !ws2811TCH) {
        return;
    }

    timerPWMPrepareDMA(ws2811TCH, WS2811_DMA_BUFFER_SIZE);
    timerPWMStartDMA(ws2811TCH);
}

#endif
