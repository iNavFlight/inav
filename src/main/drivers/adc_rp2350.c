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
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
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
 * RP2350 ADC driver — round-robin DMA ring buffer — Milestone 8
 *
 * The ADC inputs are hardware-fixed, the package decides where they are:
 *   RP2350A: ADC0–3 = GPIO26–29
 *   RP2350B: ADC0–7 = GPIO40–47
 *
 * A target lists the inputs it uses with ADC_CHANNEL_n_PIN (n = 1..4) and
 * INAV refers to them as ADC_CHN_n.  Example (RP2350_PICO):
 *   ADC_CHANNEL_1_PIN PB10 → GPIO26 → ADC0 → ADC_CHN_1  (VBAT)
 *   ADC_CHANNEL_2_PIN PB11 → GPIO27 → ADC1 → ADC_CHN_2  (CURRENT)
 *
 * All external ADC inputs of the package are sampled in round-robin, not
 * only the listed ones: the DMA ring buffer must be a power-of-2 size and
 * 4 (A) or 8 (B) inputs give exactly that, with the buffer slot equal to
 * the ADC input number.  Only the listed pins are switched to analog; the
 * others keep their digital function and their slots are never read.
 *
 * ADC sample rate: clkdiv=65535 → 48 MHz / 65536 ≈ 730 conversions/sec,
 * shared by all inputs.  Battery voltage changes are slow — this rate is
 * more than adequate.
 *
 * adcGetChannel() is always non-blocking: it reads the DMA-cached buffer.
 */

#include "platform.h"

#if defined(USE_ADC) && defined(RP2350)

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "common/utils.h"
#include "drivers/adc.h"
#include "drivers/io.h"
#include "drivers/io_def.h"

#include "hardware/adc.h"
#include "hardware/dma.h"

/* ── Channel layout ─────────────────────────────────────────────────────── */

/* Number of ADC inputs sampled in round-robin: every external input of the
 * package (the last SDK channel is the internal temperature sensor). */
#define ADC_RR_COUNT   (NUM_ADC_CHANNELS - 1u)

/* DMA ring size bits: log2(ADC_RR_COUNT × sizeof(uint16_t)) */
#if PICO_RP2350A
#define ADC_RING_BITS  3u
#else
#define ADC_RING_BITS  4u
#endif

_Static_assert((1u << ADC_RING_BITS) == ADC_RR_COUNT * sizeof(uint16_t), "ADC DMA ring size mismatch");

/* DMA result buffer — one slot per round-robin channel, filled continuously.
 * Must be volatile (DMA writes behind the compiler's back) and aligned to a
 * power-of-2 boundary ≥ buffer size for the DMA ring mode (2^ADC_RING_BITS bytes). */
static volatile uint16_t adcDmaBuf[ADC_RR_COUNT] __attribute__((aligned(1u << ADC_RING_BITS)));

/* Target pin of ADC_CHN_1..ADC_CHN_4, IOTAG_NONE when the target has no such channel */
static const ioTag_t adcChannelTag[] = {
#ifdef ADC_CHANNEL_1_PIN
    IO_TAG(ADC_CHANNEL_1_PIN),
#else
    IOTAG_NONE,
#endif
#ifdef ADC_CHANNEL_2_PIN
    IO_TAG(ADC_CHANNEL_2_PIN),
#else
    IOTAG_NONE,
#endif
#ifdef ADC_CHANNEL_3_PIN
    IO_TAG(ADC_CHANNEL_3_PIN),
#else
    IOTAG_NONE,
#endif
#ifdef ADC_CHANNEL_4_PIN
    IO_TAG(ADC_CHANNEL_4_PIN),
#else
    IOTAG_NONE,
#endif
};

static adcChannel_e adcFunctionMap[ADC_FUNCTION_COUNT];

static bool adcReady = false;

/* ── Helper ──────────────────────────────────────────────────────────────── */

/*
 * Convert an adcChannel_e to the 0-based RP2350 ADC input its target pin is
 * wired to.  Returns -1 for ADC_CHN_NONE, a channel the target does not
 * define, or a pin that is not an ADC input of this package.
 */
static int chnToRp2350Ch(adcChannel_e chn)
{
    if (chn < ADC_CHN_1 || (unsigned)(chn - ADC_CHN_1) >= ARRAYLEN(adcChannelTag)) {
        return -1;
    }
    const ioTag_t tag = adcChannelTag[chn - ADC_CHN_1];
    if (tag == IOTAG_NONE) {
        return -1;
    }
    const int ch = DEFIO_TAG_GPIOID(tag) * 16 + DEFIO_TAG_PIN(tag) - (int)ADC_BASE_PIN;
    if (ch < 0 || ch >= (int)ADC_RR_COUNT) {
        return -1;
    }
    return ch;
}

/* ── INAV ADC API ────────────────────────────────────────────────────────── */

bool adcIsFunctionAssigned(uint8_t function)
{
    if (function >= ADC_FUNCTION_COUNT) {
        return false;
    }
    return adcFunctionMap[function] != ADC_CHN_NONE;
}

int adcGetFunctionChannelAllocation(uint8_t function)
{
    if (function >= ADC_FUNCTION_COUNT) {
        return ADC_CHN_NONE;
    }
    return (int)adcFunctionMap[function];
}

uint16_t adcGetChannel(uint8_t function)
{
    if (!adcReady || function >= ADC_FUNCTION_COUNT) {
        return 0;
    }
    int ch = chnToRp2350Ch(adcFunctionMap[function]);
    if (ch < 0) {
        return 0;
    }
    return adcDmaBuf[ch];
}

void adcInit(drv_adc_config_t *init)
{
    for (int i = 0; i < ADC_FUNCTION_COUNT; i++) {
        adcFunctionMap[i] = (adcChannel_e)init->adcFunctionChannel[i];
    }

    adc_init();

    /* Switch only the pins the target declares to analog; the remaining ADC
     * capable GPIOs may carry digital functions (SPI, UART, …). */
    for (int chn = ADC_CHN_1; chn < ADC_CHN_1 + (int)ARRAYLEN(adcChannelTag); chn++) {
        const int ch = chnToRp2350Ch((adcChannel_e)chn);
        if (ch >= 0) {
            adc_gpio_init(ADC_BASE_PIN + (uint)ch);
        }
    }

    /* Round-robin over all external inputs, starting at input 0 (selected by
     * adc_init()) so that input n always lands in adcDmaBuf[n]. */
    adc_set_round_robin((1u << ADC_RR_COUNT) - 1u);

    /* Very slow: ~10 samples/sec per channel — ADC clkdiv max value */
    adc_set_clkdiv(65535.0f);

    /*
     * Enable the ADC FIFO and its DREQ signal so DMA can pace transfers.
     * Parameters: en=true, dreq_en=true, dreq_thresh=1, err_in_fifo=false,
     *             byte_shift=false (keep full 12-bit result in 16-bit entry).
     * Without this call, DREQ_ADC is never asserted and DMA stalls immediately.
     */
    adc_fifo_setup(true, true, 1, false, false);

    /* ── DMA ring buffer setup ────────────────────────────────────────── */

    int dma_ch = dma_claim_unused_channel(false);
    if (dma_ch < 0) {
        /* No free DMA channel — ADC driver unavailable */
        return;
    }

    dma_channel_config cfg = dma_channel_get_default_config((uint)dma_ch);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&cfg, false);   /* always read adc_hw->fifo */
    channel_config_set_write_increment(&cfg, true);   /* advance through buffer */
    channel_config_set_dreq(&cfg, DREQ_ADC);

    /*
     * Ring mode: write pointer wraps at a 2^ADC_RING_BITS byte boundary.
     * adcDmaBuf is ADC_RR_COUNT × 2 bytes = 2^ADC_RING_BITS.
     * transfer_count=0xFFFFFFFF gives effectively endless transfers.
     */
    channel_config_set_ring(&cfg, true, ADC_RING_BITS);

    dma_channel_configure(
        (uint)dma_ch,
        &cfg,
        (void *)adcDmaBuf,  /* write destination (cast away volatile for SDK) */
        &adc_hw->fifo,      /* read source: ADC FIFO output register (not result) */
        0xFFFFFFFFu,        /* endless — wraps in ring mode */
        true);              /* start immediately */

    adc_run(true);
    adcReady = true;
}

#endif /* USE_ADC && RP2350 */
