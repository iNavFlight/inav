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
 * RP2350A ADC channels (GPIO → ADC ch → INAV adcChannel_e):
 *   GPIO26 → ADC0 → ADC_CHN_1  (VBAT)
 *   GPIO27 → ADC1 → ADC_CHN_2  (CURRENT)
 *   GPIO28 → ADC2 → ADC_CHN_3  (RSSI)
 *   GPIO29 → ADC3 → not assigned (padding / VSYS on Pico 2)
 *
 * The RP2350 DMA ring buffer must be a power-of-2 size.  With 3 real
 * channels we pad to 4 by adding ADC3 (GPIO29) so the buffer is
 * 4 × 2 bytes = 8 bytes = 2^3.  GPIO29 is the on-board VSYS/3 divider
 * on Pico 2 and causes no harm as an unassigned ADC slot.
 *
 * ADC sample rate: clkdiv=65535 → ~10 samples/sec per channel.
 * Battery voltage changes are slow — this rate is more than adequate.
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

#include "hardware/adc.h"
#include "hardware/dma.h"

/* ── Channel layout ─────────────────────────────────────────────────────── */

/* Number of ADC channels sampled in round-robin (must be power-of-2) */
#define ADC_RR_COUNT   4u   /* ch0=GPIO26, ch1=GPIO27, ch2=GPIO28, ch3=GPIO29 */

/* GPIO pin of the first ADC channel (ADC0 = GPIO26 on RP2350A) */
#define ADC_BASE_GPIO  26u

/* DMA ring size bits: log2(ADC_RR_COUNT × sizeof(uint16_t)) = log2(8) = 3 */
#define ADC_RING_BITS  3u

/* DMA result buffer — one slot per round-robin channel, filled continuously.
 * Must be volatile (DMA writes behind the compiler's back) and aligned to a
 * power-of-2 boundary ≥ buffer size for the DMA ring mode (2^ADC_RING_BITS = 8 bytes). */
static volatile uint16_t adcDmaBuf[ADC_RR_COUNT] __attribute__((aligned(1u << ADC_RING_BITS)));

/* Function → adcChannel_e mapping; ADC_CHN_NONE if not assigned */
static adcChannel_e adcFunctionMap[ADC_FUNCTION_COUNT];

static bool adcReady = false;

/* ── Helper ──────────────────────────────────────────────────────────────── */

/*
 * Convert an adcChannel_e (ADC_CHN_1..ADC_CHN_4) to a 0-based RP2350 ADC
 * channel index.  Returns -1 for ADC_CHN_NONE or out-of-range values.
 */
static int chnToRp2350Ch(adcChannel_e chn)
{
    if (chn < ADC_CHN_1 || chn > ADC_CHN_4) {
        return -1;
    }
    return (int)(chn - ADC_CHN_1);  /* ADC_CHN_1 → 0, ADC_CHN_2 → 1, … */
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
    if (ch < 0 || (uint)ch >= ADC_RR_COUNT) {
        return 0;
    }
    return adcDmaBuf[ch];
}

void adcInit(drv_adc_config_t *init)
{
    /* Store function → channel mapping */
    for (int i = 0; i < ADC_FUNCTION_COUNT; i++) {
        adcFunctionMap[i] = (adcChannel_e)init->adcFunctionChannel[i];
    }

    adc_init();

    /* Enable all four ADC input GPIOs (ch0–ch3 = GPIO26–29) */
    for (uint i = 0; i < ADC_RR_COUNT; i++) {
        adc_gpio_init(ADC_BASE_GPIO + i);
    }

    /* Round-robin: channels 0–3 (bit mask 0x0F) */
    adc_set_round_robin(0x0Fu);

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
     * adcDmaBuf is 8 bytes (ADC_RR_COUNT × 2), so ring_size_bits=3.
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
