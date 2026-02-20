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
 * RP2350 WS2812 LED strip driver — PIO2 + DMA.
 *
 * Replaces the STM32 timer/DMA implementation in light_ws2811strip.c for
 * RP2350 targets.  Uses PIO2 (PIO_LEDSTRIP_INDEX = 2) with a 4-instruction
 * WS2812 program and a DMA channel to stream GRB words to the PIO TX FIFO.
 *
 * Replaces the generic light_ws2811strip.c (excluded from RP2350 builds in
 * cmake/rp2350.cmake) and provides all functions declared in
 * drivers/light_ws2811strip.h.
 *
 * Pin: WS2811_PIN (GP22 = PB6 in target.h) — free from flash/I2C/UART/SPI.
 * PIO: PIO2 SM0, loaded once at ws2811LedStripInit().
 * DMA: one channel, polled for completion — no IRQ needed.
 *
 * GRB word layout in led_data[] (WS2812 native order, left-shift OSR, 24-bit
 * autopull):
 *   bits 31:24 = G[7:0]   — transmitted first
 *   bits 23:16 = R[7:0]
 *   bits 15: 8 = B[7:0]   — transmitted last
 *   bits  7: 0 = 0x00     — discarded after 24-bit autopull fires
 *
 * Reference: Betaflight src/platform/PICO/light_ws2811strip_pico.c
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#ifdef USE_LED_STRIP

#include "common/color.h"
#include "common/colorconversion.h"
#include "common/utils.h"

#include "config/parameter_group_ids.h"
#include "drivers/io.h"
#include "drivers/light_ws2811strip.h"
#include "fc/settings.h"

#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/pio.h"

/* ─── PIO program ─────────────────────────────────────────────────────────── */
/*
 * 4-instruction WS2812 program.  Timing (10 cycles/bit at 800 kHz):
 *   T1=3, T2=3, T3=4  → period = 10 cycles
 *   0-bit: high=3, low=7  1-bit: high=6, low=4
 * Identical to BF's light_ws2811strip_pico.c.
 */
#define WS2812_WRAP_TARGET  0
#define WS2812_WRAP         3
#define WS2812_PIO_VERSION  0
#define WS2812_T1 3
#define WS2812_T2 3
#define WS2812_T3 4

static const uint16_t ws2812_program_instructions[] = {
    //                .wrap_target
    0x6321,  /* 0: out  x, 1           side 0 [3] */
    0x1223,  /* 1: jmp  !x, 3          side 1 [2] */
    0x1200,  /* 2: jmp  0              side 1 [2] */
    0xa242,  /* 3: nop                 side 0 [2] */
    //                .wrap
};

static const struct pio_program ws2812_program = {
    .instructions  = ws2812_program_instructions,
    .length        = 4,
    .origin        = -1,
    .pio_version   = WS2812_PIO_VERSION,
    .used_gpio_ranges = 0x0,
};

/* ─── State ───────────────────────────────────────────────────────────────── */

static hsvColor_t   ledColorBuffer[WS2811_LED_STRIP_LENGTH];
static uint32_t     led_data[WS2811_LED_STRIP_LENGTH];   /* GRB words for PIO */

static PIO          ledStripPio   = NULL;
static uint         ledStripSm    = 0;
static int          ledStripDmaCh = -1;
static bool         ledStripInitialised = false;

/* ─── PG registration ─────────────────────────────────────────────────────── */

PG_REGISTER_WITH_RESET_TEMPLATE(ledPinConfig_t, ledPinConfig, PG_LEDPIN_CONFIG, 0);

PG_RESET_TEMPLATE(ledPinConfig_t, ledPinConfig,
    .led_pin_pwm_mode = SETTING_LED_PIN_PWM_MODE_DEFAULT
);

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

bool isWS2811LedStripReady(void)
{
    if (!ledStripInitialised || ledStripDmaCh < 0) {
        return false;
    }
    return !dma_channel_is_busy((uint)ledStripDmaCh);
}

/* ─── Hardware init ──────────────────────────────────────────────────────── */

void ws2811LedStripInit(void)
{
    /* WS2811_PIN is defined in target.h (e.g. PB6 = GP22).
     * DEFIO_TAG_GPIOID gives the port index (0=PA,1=PB); DEFIO_TAG_PIN
     * gives the bit within the port.  GPIO number = gpioid*16 + pin. */
    const ioTag_t tag = IO_TAG(WS2811_PIN);
    const uint gpio   = (uint)(DEFIO_TAG_GPIOID(tag) * 16 + DEFIO_TAG_PIN(tag));

    ledStripPio = PIO_INSTANCE(PIO_LEDSTRIP_INDEX);   /* PIO2 */

    /* For pins 32..47 the GPIO base must be shifted (RP2350B only).
     * GP22 is < 32 so this is a no-op on Pico 2, but keep it for portability. */
    if (gpio >= 32) {
        pio_set_gpio_base(ledStripPio, 16u);
    }

    if (!pio_can_add_program(ledStripPio, &ws2812_program)) {
        return;
    }
    const uint offset = (uint)pio_add_program(ledStripPio, &ws2812_program);

    const int sm = pio_claim_unused_sm(ledStripPio, false);
    if (sm < 0) {
        return;
    }
    ledStripSm = (uint)sm;

    /* Configure PIO state machine */
    pio_gpio_init(ledStripPio, gpio);
    pio_sm_set_consecutive_pindirs(ledStripPio, ledStripSm, gpio, 1, true);

    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + WS2812_WRAP_TARGET, offset + WS2812_WRAP);
    sm_config_set_sideset(&c, 1, false, false);
    sm_config_set_sideset_pins(&c, gpio);
    /* Left shift (MSB first), autopull after 24 bits (RGB; not RGBW) */
    sm_config_set_out_shift(&c, false, true, 24);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);

    const int cycles_per_bit = WS2812_T1 + WS2812_T2 + WS2812_T3;
    const float div = (float)clock_get_hz(clk_sys) / ((float)WS2811_CARRIER_HZ * (float)cycles_per_bit);
    sm_config_set_clkdiv(&c, div);

    pio_sm_init(ledStripPio, ledStripSm, offset, &c);
    pio_sm_set_enabled(ledStripPio, ledStripSm, true);

    /* Claim and configure DMA channel */
    const int dma_ch = dma_claim_unused_channel(false);
    if (dma_ch < 0) {
        return;
    }
    ledStripDmaCh = dma_ch;

    dma_channel_config dc = dma_channel_get_default_config((uint)ledStripDmaCh);
    channel_config_set_transfer_data_size(&dc, DMA_SIZE_32);
    channel_config_set_read_increment(&dc, true);
    channel_config_set_write_increment(&dc, false);
    channel_config_set_dreq(&dc, pio_get_dreq(ledStripPio, ledStripSm, true));

    dma_channel_configure(
        (uint)ledStripDmaCh,
        &dc,
        &ledStripPio->txf[ledStripSm],  /* write: PIO TX FIFO      */
        NULL,                            /* read: set at start time */
        WS2811_LED_STRIP_LENGTH,
        false
    );

    memset(ledColorBuffer, 0, sizeof(ledColorBuffer));
    memset(led_data, 0, sizeof(led_data));

    ledStripInitialised = true;

    /* Send an all-zeros frame to reset any previously lit strip */
    ws2811UpdateStrip();
}

/* ─── Strip update ───────────────────────────────────────────────────────── */

void ws2811UpdateStrip(void)
{
    if (!ledStripInitialised || ledStripDmaCh < 0) {
        return;
    }
    /* Don't start a new frame while the previous one is still in flight */
    if (dma_channel_is_busy((uint)ledStripDmaCh)) {
        return;
    }

    /* Convert HSV → GRB.  WS2812 bit order: G[7:0] R[7:0] B[7:0], MSB first.
     * Pack into bits 31:8 of the 32-bit word; the PIO left-shifts 24 bits out
     * then autopulls the next word (bottom 8 bits are discarded). */
    for (uint16_t i = 0; i < WS2811_LED_STRIP_LENGTH; i++) {
        const rgbColor24bpp_t *rgb = hsvToRgb24(&ledColorBuffer[i]);
        led_data[i] = ((uint32_t)rgb->rgb.g << 24)
                    | ((uint32_t)rgb->rgb.r << 16)
                    | ((uint32_t)rgb->rgb.b <<  8);
    }

    /* Start DMA transfer */
    dma_channel_set_read_addr((uint)ledStripDmaCh, led_data, false);
    dma_channel_set_trans_count((uint)ledStripDmaCh, WS2811_LED_STRIP_LENGTH, false);
    dma_channel_start((uint)ledStripDmaCh);
}

/* ─── Stubs for BF-only / timer-based API ────────────────────────────────── */

/* Called only when a plain-LED (non-WS2812) PWM pin mode is selected.
 * PIO-based WS2812 does not support this mode; leave as no-ops. */
void ledPinStartPWM(uint16_t value) { UNUSED(value); }
void ledPinStopPWM(void) { }

/* Declared in the header for completeness; not called by INAV at runtime. */
void ws2811LedStripHardwareInit(void) { }
void ws2811LedStripDMAEnable(void)   { }
bool ws2811LedStripDMAInProgress(void) { return !isWS2811LedStripReady(); }

#endif /* USE_LED_STRIP */
