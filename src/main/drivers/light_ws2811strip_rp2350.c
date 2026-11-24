/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

/*
 * RP2350 WS2812 LED strip driver — hardware PWM + DMA.
 *
 * Replaces the generic light_ws2811strip.c (excluded from RP2350 builds in
 * cmake/rp2350.cmake) and provides all functions declared in
 * drivers/light_ws2811strip.h.
 *
 * Same principle as the STM32 driver: the PWM slice of WS2811_PIN runs at
 * 800 kHz and a DMA channel, paced by the wrap of that slice, writes the
 * compare value of every bit. It takes no PIO state machine, so the PIO blocks
 * stay free for DSHOT, the PIO UARTs and the OSD.
 *
 * The slice is shared by two GPIOs (and by the GPIO 16 above/below). When a
 * motor or servo output already runs on it, the strip is not started.
 *
 * The compare register is latched at the wrap, so a value written after wrap
 * N is on the pin from wrap N+1.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#ifdef USE_LED_STRIP

#include "common/color.h"
#include "common/colorconversion.h"
#include "common/log.h"
#include "common/maths.h"
#include "common/time.h"
#include "common/utils.h"

#include "drivers/io.h"
#include "drivers/light_ws2811strip.h"
#include "drivers/time.h"

#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

// One leading slot with the line low: a wrap request that was pending when the
// DMA channel starts must not eat the first data bit. One trailing slot leaves
// the compare value at zero, the line stays low after the frame.
#define WS2811_LEAD_SLOTS       1
#define WS2811_TAIL_SLOTS       1
#define WS2811_BUFFER_SIZE      (WS2811_LEAD_SLOTS + WS2811_LED_STRIP_LENGTH * WS2811_BITS_PER_LED + WS2811_TAIL_SLOTS)

// Low time that latches the frame. 50 us for the first WS2812, 280 us for the current WS2812B.
#define WS2811_RESET_US         300

/* ─── State ───────────────────────────────────────────────────────────────── */

static hsvColor_t   ledColorBuffer[WS2811_LED_STRIP_LENGTH];
static uint32_t     ledPwmBuffer[WS2811_BUFFER_SIZE];   // compare register value per bit

static int          ledStripDmaCh = -1;
static bool         ledStripInitialised = false;
static uint16_t     ledBitPeriod;       // PWM counts per bit
static uint32_t     ledCompareBit0;     // compare register values: the level sits in the half of the strip channel
static uint32_t     ledCompareBit1;
static uint32_t     ledPwmClockHz;
static const char   *ledStripFailure;
static timeUs_t     ledStripReadyAtUs;  // end of the last frame including the reset time

/* ─── Color buffer helpers (identical to generic light_ws2811strip.c) ─────── */

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
    ledColorBuffer[index].v = (uint8_t)((uint16_t)ledColorBuffer[index].v * scalePercent / 100);
}

void setStripColor(const hsvColor_t *color)
{
    for (uint16_t i = 0; i < WS2811_LED_STRIP_LENGTH; i++) {
        setLedHsv(i, color);
    }
}

void setStripColors(const hsvColor_t *colors)
{
    for (uint16_t i = 0; i < WS2811_LED_STRIP_LENGTH; i++) {
        setLedHsv(i, colors++);
    }
}

/* ─── Status ─────────────────────────────────────────────────────────────── */

// Why the strip was not started, for the status command. NULL when it runs or was never enabled.
const char *ws2811LedStripFailure(void)
{
    return ledStripFailure;
}

bool isWS2811LedStripReady(void)
{
    if (!ledStripInitialised) {
        return false;
    }
    return !dma_channel_is_busy((uint)ledStripDmaCh) && cmpTimeUs(micros(), ledStripReadyAtUs) >= 0;
}

/* ─── Hardware init ──────────────────────────────────────────────────────── */

void ws2811LedStripInit(void)
{
    if (ledStripInitialised) {
        return;
    }

    /* DEFIO_TAG_GPIOID gives the port index (0=PA, 1=PB, ...), DEFIO_TAG_PIN
     * the bit within the port. GPIO number = gpioid*16 + pin. */
    const ioTag_t tag = IO_TAG(WS2811_PIN);
    const uint gpio   = (uint)(DEFIO_TAG_GPIOID(tag) * 16 + DEFIO_TAG_PIN(tag));
    const uint slice  = pwm_gpio_to_slice_num(gpio);
    const uint chan   = pwm_gpio_to_channel(gpio);

    /* The strip needs the whole slice: its period, and the DMA writes the
     * compare values of both channels. Motor and servo outputs are set up
     * earlier in init() (pwmMotorAndServoInit), so a slice that already runs
     * belongs to one of them. */
    if (pwm_hw->slice[slice].csr & PWM_CH0_CSR_EN_BITS) {
        ledStripFailure = "PWM slice used by a motor or servo output";
        LOG_ERROR(SYSTEM, "LED strip: PWM slice %u is used by a motor or servo output", slice);
        return;
    }

    const int dmaCh = dma_claim_unused_channel(false);
    if (dmaCh < 0) {
        ledStripFailure = "no free DMA channel";
        LOG_ERROR(SYSTEM, "LED strip: no free DMA channel");
        return;
    }
    ledStripDmaCh = dmaCh;

    /* clk_sys is 150, 192 or 240 MHz (cpu_overclock): 188, 240 or 300 counts per bit */
    ledPwmClockHz  = clock_get_hz(clk_sys);
    ledBitPeriod   = (uint16_t)((ledPwmClockHz + WS2811_CARRIER_HZ / 2) / WS2811_CARRIER_HZ);
    /* Channel A is the low half of the compare register, channel B the high
     * half. The other channel is written as zero. */
    const uint shift = (chan == PWM_CHAN_B) ? PWM_CH0_CC_B_LSB : PWM_CH0_CC_A_LSB;
    ledCompareBit0 = (uint32_t)(ledBitPeriod * 8 / 25) << shift;    // 0.4 us high
    ledCompareBit1 = (uint32_t)(ledBitPeriod * 16 / 25) << shift;   // 0.8 us high

    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&cfg, 1);
    pwm_config_set_wrap(&cfg, (uint16_t)(ledBitPeriod - 1));
    pwm_init(slice, &cfg, false);
    pwm_set_both_levels(slice, 0, 0);
    pwm_set_enabled(slice, true);

    gpio_set_function(gpio, GPIO_FUNC_PWM);
#ifdef WS2811_PIN_INVERTED
    // For boards whose level shifter inverts. After gpio_set_function(), which resets the override.
    gpio_set_outover(gpio, GPIO_OVERRIDE_INVERT);
#endif

    /* Whole-register writes: a narrow write would be repeated in both halves
     * of the register and put the strip data on the other channel too. */
    dma_channel_config dc = dma_channel_get_default_config((uint)ledStripDmaCh);
    channel_config_set_transfer_data_size(&dc, DMA_SIZE_32);
    channel_config_set_read_increment(&dc, true);
    channel_config_set_write_increment(&dc, false);
    channel_config_set_dreq(&dc, pwm_get_dreq(slice));

    dma_channel_configure(
        (uint)ledStripDmaCh,
        &dc,
        &pwm_hw->slice[slice].cc,
        NULL,                            /* read: set at start time */
        WS2811_BUFFER_SIZE,
        false
    );

    memset(ledColorBuffer, 0, sizeof(ledColorBuffer));
    memset(ledPwmBuffer, 0, sizeof(ledPwmBuffer));

    ledStripReadyAtUs = micros();
    ledStripInitialised = true;

    /* Send an all-zeros frame to reset any previously lit strip */
    ws2811UpdateStrip(WS2811_LED_STRIP_LENGTH);
}

/* ─── Strip update ───────────────────────────────────────────────────────── */

void ws2811UpdateStrip(uint16_t usedLedCount)
{
    /* Don't start a new frame while the previous one is still in flight or
     * the strip has not latched it yet */
    if (!isWS2811LedStripReady()) {
        return;
    }

    /* Bound the transfer to the configured LED count rather than always
     * streaming the full 128 slots. */
    const uint16_t ledCount = MIN(usedLedCount, (uint16_t)WS2811_LED_STRIP_LENGTH);
    if (ledCount == 0) {
        return;
    }

    /* WS2812 bit order: G[7:0] R[7:0] B[7:0], MSB first */
    uint32_t *slot = &ledPwmBuffer[WS2811_LEAD_SLOTS];
    for (uint16_t i = 0; i < ledCount; i++) {
        const rgbColor24bpp_t *rgb = hsvToRgb24(&ledColorBuffer[i]);
        const uint32_t grb = ((uint32_t)rgb->rgb.g << 16) | ((uint32_t)rgb->rgb.r << 8) | rgb->rgb.b;
        for (uint32_t mask = 1u << (WS2811_BITS_PER_LED - 1); mask; mask >>= 1) {
            *slot++ = (grb & mask) ? ledCompareBit1 : ledCompareBit0;
        }
    }
    for (unsigned i = 0; i < WS2811_TAIL_SLOTS; i++) {
        *slot++ = 0;
    }

    const uint32_t slots = (uint32_t)(slot - ledPwmBuffer);
    /* One more period until the last value is on the pin */
    const uint32_t frameUs = (uint32_t)((uint64_t)(slots + 1) * ledBitPeriod * 1000000u / ledPwmClockHz) + 1;
    ledStripReadyAtUs = micros() + frameUs + WS2811_RESET_US;

    dma_channel_set_read_addr((uint)ledStripDmaCh, ledPwmBuffer, false);
    dma_channel_set_trans_count((uint)ledStripDmaCh, slots, false);
    dma_channel_start((uint)ledStripDmaCh);
}

/* ─── Stubs for BF-only / timer-based API ────────────────────────────────── */

/* Called only when a plain-LED (non-WS2812) PWM pin mode is selected.
 * This driver does not support that mode; leave as no-ops. */
void ledPinStartPWM(uint16_t value) { UNUSED(value); }
void ledPinStopPWM(void) { }
// PINIO channel 0 (the LED pad as an on/off output) is not available: the pad belongs to the PWM slice
void ws2811SetIdleHigh(bool high) { UNUSED(high); }

/* Declared in the header for completeness; not called by INAV at runtime. */
void ws2811LedStripHardwareInit(void) { }
void ws2811LedStripDMAEnable(void)   { }
bool ws2811LedStripDMAInProgress(void) { return !isWS2811LedStripReady(); }

#endif /* USE_LED_STRIP */
