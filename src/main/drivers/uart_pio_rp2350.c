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
 * RP2350 PIO software UART driver for INAV — Milestone 6
 *
 * Implements UART3 and UART4 using PIO state machines on PIO1:
 *
 *   UART3 (INAV) → PIO1 SM0 (TX) + SM1 (RX)  — PIO1_IRQ0
 *   UART4 (INAV) → PIO1 SM2 (TX) + SM3 (RX)  — PIO1_IRQ1
 *
 * PIO program budget (32 instructions per block):
 *   TX program: 4 instructions  (shared by SM0 and SM2)
 *   RX program: 9 instructions  (shared by SM1 and SM3)
 *   Total used: 13 of 32 — well within budget.
 *
 * Each SM has its own clock divider, so UART3 and UART4 can run at
 * different baud rates simultaneously (e.g. 420000 baud CRSF + 57600 baud
 * telemetry).
 *
 * PIO programs are pre-assembled from pico-examples/pio/uart_tx.pio and
 * uart_rx.pio (BSD-3-Clause, copyright Raspberry Pi Trading Ltd).
 * Embedding pre-assembled instruction arrays avoids a pioasm build step.
 *
 * RX FIFO data layout: the RX SM shifts in bits MSB-first, so each received
 * byte lands in bits [31:24] of the 32-bit FIFO word. Read with >> 24.
 *
 * Signal inversion (SBUS) is done via gpio_set_inover/outover — no external
 * inverter required.
 *
 * Pin assignments are defined in target.h:
 *   UART3_TX_PIN / UART3_RX_PIN
 *   UART4_TX_PIN / UART4_RX_PIN
 */

#include "platform.h"

#if defined(USE_UART3) || defined(USE_UART4)

#include <stdbool.h>
#include <stdint.h>

#include "build/build_config.h"
#include "common/utils.h"

#include "drivers/io.h"
#include "drivers/serial.h"
#include "drivers/serial_uart.h"
#include "drivers/serial_uart_impl.h"

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/pio.h"

/* ── Pre-assembled PIO programs ─────────────────────────────────────────────
 *
 * Source: pico-examples/pio/uart_tx.pio and uart_rx.pio
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd. BSD-3-Clause.
 */

/* uart_tx — 4 instructions, uses side-set on pin_tx ──────────────────────── */
static const uint16_t uart_tx_instructions[] = {
    0x9fa0, //  0: pull   block           side 1 [7]  — pull & idle high
    0xf727, //  1: set    x, 7            side 0 [7]  — start bit
    0x6001, //  2: out    pins, 1                      — data bit
    0x0642, //  3: jmp    x--, 2                 [6]  — loop
};
static const struct pio_program uart_tx_program = {
    .instructions = uart_tx_instructions,
    .length       = 4,
    .origin       = -1,
    .pio_version  = 0,
#if PICO_PIO_VERSION > 0
    .used_gpio_ranges = 0x0,
#endif
};

/* uart_rx — 9 instructions ──────────────────────────────────────────────── */
static const uint16_t uart_rx_instructions[] = {
    0x2020, //  0: wait   0 pin, 0          — wait for start bit
    0xea27, //  1: set    x, 7       [10]   — init bit counter + half-bit delay
    0x4001, //  2: in     pins, 1           — sample bit
    0x0642, //  3: jmp    x--, 2     [6]    — loop 8 bits
    0x00c8, //  4: jmp    pin, 8            — check stop bit
    0xc014, //  5: irq    nowait 4 rel      — signal framing error
    0x20a0, //  6: wait   1 pin, 0          — wait for line idle
    0x0000, //  7: jmp    0                 — restart
    0x8020, //  8: push   block             — push byte to FIFO
};
static const struct pio_program uart_rx_program = {
    .instructions = uart_rx_instructions,
    .length       = 9,
    .origin       = -1,
    .pio_version  = 0,
#if PICO_PIO_VERSION > 0
    .used_gpio_ranges = 0x0,
#endif
};

/* ── Program init helpers ────────────────────────────────────────────────── */

/* Convert INAV ioTag_t to RP2350 raw GPIO number. */
static inline uint ioTagToGpioNum(ioTag_t tag)
{
    return (uint)(DEFIO_TAG_GPIOID(tag) * 16 + DEFIO_TAG_PIN(tag));
}

static void uart_tx_program_init(PIO pio, uint sm, uint offset, uint pin, uint baud)
{
    pio_sm_set_pins_with_mask64(pio, sm, 1ull << pin, 1ull << pin);
    pio_sm_set_pindirs_with_mask64(pio, sm, 1ull << pin, 1ull << pin);
    pio_gpio_init(pio, pin);

    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + 0, offset + 3);
    sm_config_set_sideset(&c, 2, true, false);
    sm_config_set_out_shift(&c, true, false, 32);
    sm_config_set_out_pins(&c, pin, 1);
    sm_config_set_sideset_pins(&c, pin);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
    float div = (float)clock_get_hz(clk_sys) / (8.0f * (float)baud);
    sm_config_set_clkdiv(&c, div);
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}

static void uart_rx_program_init(PIO pio, uint sm, uint offset, uint pin, uint baud)
{
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, false);
    pio_gpio_init(pio, pin);
    gpio_pull_up(pin);

    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + 0, offset + 8);
    sm_config_set_in_pins(&c, pin);
    sm_config_set_jmp_pin(&c, pin);
    sm_config_set_in_shift(&c, true, false, 32);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
    float div = (float)clock_get_hz(clk_sys) / (8.0f * (float)baud);
    sm_config_set_clkdiv(&c, div);
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}

/* ── Device struct ───────────────────────────────────────────────────────── */

/*
 * piouartDevice_t — PIO UART port state.
 *
 * uartPort_t is the first member so that (piouartDevice_t *) ↔ (uartPort_t *)
 * ↔ (serialPort_t *) casts are valid (C99 §6.7.2.1 first-member rule).
 */
typedef struct {
    uartPort_t  port;
    uint        sm_tx;          /* state machine index for TX (0..3) */
    uint        sm_rx;          /* state machine index for RX (0..3) */
    uint        tx_pin;
    uint        rx_pin;
    int         irq_index;      /* 0 = PIO1_IRQ0, 1 = PIO1_IRQ1 */
    uint32_t    rx_intr_bit;    /* PIO_INTR_SMx_RXNEMPTY_BITS for sm_rx */
    uint32_t    tx_intr_bit;    /* PIO_INTR_SMx_TXNFULL_BITS  for sm_tx */
    int         tx_offset;      /* pio_add_program offset for TX program */
    int         rx_offset;      /* pio_add_program offset for RX program */
    volatile uint8_t rxBuffer[256];
    volatile uint8_t txBuffer[256];
} piouartDevice_t;

/* ── Forward declarations ────────────────────────────────────────────────── */

static void     pioUartWrite(serialPort_t *instance, uint8_t ch);
static uint32_t pioUartTotalRxBytesWaiting(const serialPort_t *instance);
static uint32_t pioUartTotalTxBytesFree(const serialPort_t *instance);
static uint8_t  pioUartRead(serialPort_t *instance);
static void     pioUartSetBaudRate(serialPort_t *s, uint32_t baudRate);
static bool     pioUartIsTransmitBufferEmpty(const serialPort_t *s);
static void     pioUartSetMode(serialPort_t *instance, portMode_t mode);
static void     pioUartSetOptions(serialPort_t *instance, portOptions_t options);

static const struct serialPortVTable pioUartVTable = {
    .serialWrite                 = pioUartWrite,
    .serialTotalRxWaiting        = pioUartTotalRxBytesWaiting,
    .serialTotalTxFree           = pioUartTotalTxBytesFree,
    .serialRead                  = pioUartRead,
    .serialSetBaudRate           = pioUartSetBaudRate,
    .isSerialTransmitBufferEmpty = pioUartIsTransmitBufferEmpty,
    .setMode                     = pioUartSetMode,
    .setOptions                  = pioUartSetOptions,
    .isConnected                 = NULL,
    .writeBuf                    = NULL,
    .beginWrite                  = NULL,
    .endWrite                    = NULL,
    .isIdle                      = NULL,
};

/* Program offsets — loaded once into PIO1, shared by both UARTs */
static int txProgramOffset = -1;
static int rxProgramOffset = -1;

/* Lookup tables for PIO interrupt register bits by SM index */
static const uint32_t sm_rxnempty_bits[4] = {
    PIO_INTR_SM0_RXNEMPTY_BITS,
    PIO_INTR_SM1_RXNEMPTY_BITS,
    PIO_INTR_SM2_RXNEMPTY_BITS,
    PIO_INTR_SM3_RXNEMPTY_BITS,
};
static const uint32_t sm_txnfull_bits[4] = {
    PIO_INTR_SM0_TXNFULL_BITS,
    PIO_INTR_SM1_TXNFULL_BITS,
    PIO_INTR_SM2_TXNFULL_BITS,
    PIO_INTR_SM3_TXNFULL_BITS,
};

/* ── Shared IRQ handler ──────────────────────────────────────────────────── */

static void piouartIrqHandler(piouartDevice_t *s)
{
    io_rw_32 *inte = s->irq_index == 0 ? &pio1->inte0 : &pio1->inte1;
    io_ro_32 *ints = s->irq_index == 0 ? &pio1->ints0 : &pio1->ints1;

    /* RX: drain FIFO into ring buffer while not empty */
    if (*ints & s->rx_intr_bit) {
        while (!pio_sm_is_rx_fifo_empty(pio1, s->sm_rx)) {
            /*
             * With shift_right=true and push_threshold=32, 8 bits shift into
             * the top of the ISR so the byte sits in bits [31:24] — left-
             * justified.  Use >> 24 to extract.
             */
            uint8_t ch = (uint8_t)(pio1->rxf[s->sm_rx] >> 24);
            if (s->port.port.rxCallback) {
                s->port.port.rxCallback(ch, s->port.port.rxCallbackData);
            } else {
                s->port.port.rxBuffer[s->port.port.rxBufferHead] = ch;
                s->port.port.rxBufferHead =
                    (s->port.port.rxBufferHead + 1) % s->port.port.rxBufferSize;
            }
        }
    }

    /*
     * TX: push ring buffer bytes into PIO TX FIFO.
     *
     * We check *inte (enable register) rather than *ints because the TX
     * FIFO-not-full condition is almost always true — using ints would fire
     * on every IRQ entry even when there's nothing to send.
     */
    if (*inte & s->tx_intr_bit) {
        while (!pio_sm_is_tx_fifo_full(pio1, s->sm_tx)) {
            if (s->port.port.txBufferTail != s->port.port.txBufferHead) {
                /* PIO TX program reads the low 8 bits of the FIFO word */
                pio_sm_put(pio1, s->sm_tx,
                           s->port.port.txBuffer[s->port.port.txBufferTail]);
                s->port.port.txBufferTail =
                    (s->port.port.txBufferTail + 1) % s->port.port.txBufferSize;
            } else {
                /* Buffer empty — disable TX interrupt until next write */
                pio_set_irqn_source_enabled(pio1, (uint)s->irq_index,
                    pio_get_tx_fifo_not_full_interrupt_source(s->sm_tx), false);
                break;
            }
        }
    }
}

/* ── piouartInit — shared init for UART3 and UART4 ──────────────────────── */

/*
 * piouartInit is NOT inside any #ifdef USE_UARTn guard so that it compiles
 * whenever USE_UART3 or USE_UART4 (or both) are defined.  Placing it inside
 * the USE_UART3 guard would cause a linker error if only USE_UART4 is defined.
 */
static uartPort_t *piouartInit(piouartDevice_t *s, uint sm_tx, uint sm_rx,
                                uint tx_pin, uint rx_pin, int irq_index,
                                uint32_t baudRate, portMode_t mode,
                                portOptions_t options)
{
    s->sm_tx       = sm_tx;
    s->sm_rx       = sm_rx;
    s->tx_pin      = tx_pin;
    s->rx_pin      = rx_pin;
    s->irq_index   = irq_index;
    s->rx_intr_bit = sm_rxnempty_bits[sm_rx];
    s->tx_intr_bit = sm_txnfull_bits[sm_tx];

    s->port.port.vTable       = &pioUartVTable;
    s->port.port.baudRate     = baudRate;
    s->port.port.mode         = mode;
    s->port.port.options      = options;
    s->port.port.rxBuffer     = s->rxBuffer;
    s->port.port.txBuffer     = s->txBuffer;
    s->port.port.rxBufferSize = sizeof(s->rxBuffer);
    s->port.port.txBufferSize = sizeof(s->txBuffer);
    s->port.port.rxBufferHead = 0;
    s->port.port.rxBufferTail = 0;
    s->port.port.txBufferHead = 0;
    s->port.port.txBufferTail = 0;

    /* Load programs once — each subsequent call reuses the same offset */
    if (txProgramOffset < 0) {
        txProgramOffset = pio_add_program(pio1, &uart_tx_program);
    }
    if (rxProgramOffset < 0) {
        rxProgramOffset = pio_add_program(pio1, &uart_rx_program);
    }

    uart_tx_program_init(pio1, sm_tx, (uint)txProgramOffset, tx_pin, baudRate);
    uart_rx_program_init(pio1, sm_rx, (uint)rxProgramOffset, rx_pin, baudRate);

    /*
     * GPIO signal inversion — replaces the external inverter that STM32 targets
     * require for SBUS (100000 baud, 8E2, inverted).  RP2350 can invert any GPIO
     * input/output in hardware.
     */
    if (options & SERIAL_INVERTED) {
        gpio_set_inover(rx_pin, GPIO_OVERRIDE_INVERT);
        gpio_set_outover(tx_pin, GPIO_OVERRIDE_INVERT);
    }

    return &s->port;
}

/* ── UART3 (INAV) → PIO1 SM0 (TX) + SM1 (RX) ───────────────────────────── */

#ifdef USE_UART3

static piouartDevice_t piouart0Device;

static void pio1Irq0Handler(void)
{
    piouartIrqHandler(&piouart0Device);
}

uartPort_t *serialUART3(uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    uartPort_t *p = piouartInit(&piouart0Device,
                                0 /* sm_tx */, 1 /* sm_rx */,
                                ioTagToGpioNum(IO_TAG(UART3_TX_PIN)),
                                ioTagToGpioNum(IO_TAG(UART3_RX_PIN)),
                                0 /* irq_index = PIO1_IRQ0 */,
                                baudRate, mode, options);

    irq_set_exclusive_handler(PIO1_IRQ_0, pio1Irq0Handler);
    irq_set_enabled(PIO1_IRQ_0, true);

    /* Enable RX interrupt; TX is enabled on demand in pioUartWrite() */
    if (mode & MODE_RX) {
        pio_set_irqn_source_enabled(pio1, 0,
            pio_get_rx_fifo_not_empty_interrupt_source(1 /* sm_rx */), true);
    }

    return p;
}

#endif /* USE_UART3 */

/* ── UART4 (INAV) → PIO1 SM2 (TX) + SM3 (RX) ───────────────────────────── */

#ifdef USE_UART4

static piouartDevice_t piouart1Device;

static void pio1Irq1Handler(void)
{
    piouartIrqHandler(&piouart1Device);
}

uartPort_t *serialUART4(uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    uartPort_t *p = piouartInit(&piouart1Device,
                                2 /* sm_tx */, 3 /* sm_rx */,
                                ioTagToGpioNum(IO_TAG(UART4_TX_PIN)),
                                ioTagToGpioNum(IO_TAG(UART4_RX_PIN)),
                                1 /* irq_index = PIO1_IRQ1 */,
                                baudRate, mode, options);

    irq_set_exclusive_handler(PIO1_IRQ_1, pio1Irq1Handler);
    irq_set_enabled(PIO1_IRQ_1, true);

    if (mode & MODE_RX) {
        pio_set_irqn_source_enabled(pio1, 1,
            pio_get_rx_fifo_not_empty_interrupt_source(3 /* sm_rx */), true);
    }

    return p;
}

#endif /* USE_UART4 */

/* ── Vtable implementations ──────────────────────────────────────────────── */

static void pioUartWrite(serialPort_t *instance, uint8_t ch)
{
    piouartDevice_t *s = (piouartDevice_t *)instance;

    s->port.port.txBuffer[s->port.port.txBufferHead] = ch;
    s->port.port.txBufferHead =
        (s->port.port.txBufferHead + 1) % s->port.port.txBufferSize;

    pio_set_irqn_source_enabled(pio1, (uint)s->irq_index,
        pio_get_tx_fifo_not_full_interrupt_source(s->sm_tx), true);
}

static uint32_t pioUartTotalRxBytesWaiting(const serialPort_t *instance)
{
    const piouartDevice_t *s = (const piouartDevice_t *)instance;
    if (s->port.port.rxBufferHead >= s->port.port.rxBufferTail) {
        return s->port.port.rxBufferHead - s->port.port.rxBufferTail;
    }
    return s->port.port.rxBufferSize + s->port.port.rxBufferHead
           - s->port.port.rxBufferTail;
}

static uint32_t pioUartTotalTxBytesFree(const serialPort_t *instance)
{
    const piouartDevice_t *s = (const piouartDevice_t *)instance;
    uint32_t bytesUsed;
    if (s->port.port.txBufferHead >= s->port.port.txBufferTail) {
        bytesUsed = s->port.port.txBufferHead - s->port.port.txBufferTail;
    } else {
        bytesUsed = s->port.port.txBufferSize + s->port.port.txBufferHead
                    - s->port.port.txBufferTail;
    }
    return (s->port.port.txBufferSize - 1) - bytesUsed;
}

static uint8_t pioUartRead(serialPort_t *instance)
{
    piouartDevice_t *s = (piouartDevice_t *)instance;
    uint8_t ch = s->port.port.rxBuffer[s->port.port.rxBufferTail];
    s->port.port.rxBufferTail =
        (s->port.port.rxBufferTail + 1) % s->port.port.rxBufferSize;
    return ch;
}

static void pioUartSetBaudRate(serialPort_t *instance, uint32_t baudRate)
{
    piouartDevice_t *s = (piouartDevice_t *)instance;
    s->port.port.baudRate = baudRate;

    /*
     * Reinitialise both SMs with new clock divider.  pio_sm_clear_fifos() is
     * called internally by uart_tx/rx_program_init → pio_sm_init, so any bytes
     * queued in the TX ring buffer but not yet pushed to the PIO FIFO will still
     * be sent, but bytes already in the hardware FIFO are discarded.  Callers
     * that need a clean switch (e.g. GPS auto-baud) should drain the port first.
     */
    pio_sm_set_enabled(pio1, s->sm_tx, false);
    pio_sm_set_enabled(pio1, s->sm_rx, false);
    uart_tx_program_init(pio1, s->sm_tx, (uint)txProgramOffset, s->tx_pin, baudRate);
    uart_rx_program_init(pio1, s->sm_rx, (uint)rxProgramOffset, s->rx_pin, baudRate);
}

static bool pioUartIsTransmitBufferEmpty(const serialPort_t *instance)
{
    const piouartDevice_t *s = (const piouartDevice_t *)instance;
    return s->port.port.txBufferTail == s->port.port.txBufferHead;
}

static void pioUartSetMode(serialPort_t *instance, portMode_t mode)
{
    piouartDevice_t *s = (piouartDevice_t *)instance;
    s->port.port.mode = mode;
    pio_set_irqn_source_enabled(pio1, (uint)s->irq_index,
        pio_get_rx_fifo_not_empty_interrupt_source(s->sm_rx),
        (mode & MODE_RX) != 0);
}

static void pioUartSetOptions(serialPort_t *instance, portOptions_t options)
{
    piouartDevice_t *s = (piouartDevice_t *)instance;
    s->port.port.options = options;

    if (options & SERIAL_INVERTED) {
        gpio_set_inover(s->rx_pin, GPIO_OVERRIDE_INVERT);
        gpio_set_outover(s->tx_pin, GPIO_OVERRIDE_INVERT);
    } else {
        gpio_set_inover(s->rx_pin, GPIO_OVERRIDE_NORMAL);
        gpio_set_outover(s->tx_pin, GPIO_OVERRIDE_NORMAL);
    }
}

#endif /* USE_UART3 || USE_UART4 */
