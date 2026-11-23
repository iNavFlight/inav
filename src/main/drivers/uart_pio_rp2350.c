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
 *   UART3 (INAV) → PIO1, one TX + one RX state machine — PIO1_IRQ0
 *   UART4 (INAV) → PIO1, one TX + one RX state machine — PIO1_IRQ1
 *
 * A port takes one state machine per direction it uses (MODE_RX, MODE_TX), with
 * pio_claim_unused_sm(), so a receive-only port (ESC telemetry) costs one and
 * PIO1 can be shared with another driver (LED strip). A port that cannot get
 * the state machines for its mode fails to open. A direction added later with
 * serialSetMode() claims its state machine then, or is dropped from the mode.
 * A re-open gives back the state machine of a direction the new mode has not.
 * The TX pin of a receive-only port is left alone, as on the STM32 targets.
 *
 * PIO program budget (32 instructions per block):
 *   TX program: 4 instructions  (shared by both ports)
 *   RX program: 9 instructions  (shared by both ports)
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
#include "common/log.h"
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

/*
 * GPIO signal inversion: the RP2350 can invert any GPIO input/output in
 * hardware, so an inverted protocol needs no external inverter. It is set right
 * after pio_gpio_init(), which resets the pin overrides, and before the state
 * machine starts: an RX state machine on a still non-inverted line would take
 * the idle level for a start bit.
 */
static void uart_tx_program_init(PIO pio, uint sm, uint offset, uint pin, uint baud, bool inverted)
{
    pio_sm_set_pins_with_mask64(pio, sm, 1ull << pin, 1ull << pin);
    pio_sm_set_pindirs_with_mask64(pio, sm, 1ull << pin, 1ull << pin);
    pio_gpio_init(pio, pin);
    gpio_set_outover(pin, inverted ? GPIO_OVERRIDE_INVERT : GPIO_OVERRIDE_NORMAL);

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

static void uart_rx_program_init(PIO pio, uint sm, uint offset, uint pin, uint baud, bool inverted)
{
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, false);
    pio_gpio_init(pio, pin);
    gpio_set_inover(pin, inverted ? GPIO_OVERRIDE_INVERT : GPIO_OVERRIDE_NORMAL);
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
    bool        txClaimed;      /* sm_tx is valid; kept across re-opens */
    bool        rxClaimed;      /* sm_rx is valid; kept across re-opens */
    uint        sm_tx;          /* state machine index for TX (0..3) */
    uint        sm_rx;          /* state machine index for RX (0..3) */
    uint        tx_pin;
    uint        rx_pin;
    int         irq_index;      /* 0 = PIO1_IRQ0, 1 = PIO1_IRQ1 */
    uint8_t     uartNumber;     /* INAV UART number, for the log */
    uint32_t    rx_intr_bit;    /* PIO_INTR_SMx_RXNEMPTY_BITS for sm_rx, 0 without RX */
    uint32_t    tx_intr_bit;    /* PIO_INTR_SMx_TXNFULL_BITS  for sm_tx, 0 without TX */
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

/* ── State machines per direction ────────────────────────────────────────── */

/* Directions a port needs state machines for. */
static portMode_t piouartWantedDirections(portMode_t mode, portOptions_t options)
{
    portMode_t dirs = (portMode_t)(mode & MODE_RXTX);
    if (dirs == 0) {
        dirs = MODE_RXTX;
    }
    if (options & SERIAL_BIDIR) {
        dirs = (portMode_t)(dirs | MODE_TX);
    }
    return dirs;
}

/*
 * Claim the state machines for the directions in dirs that the port does not
 * have yet. Returns the directions it could not get.
 */
static portMode_t piouartClaimDirections(piouartDevice_t *s, portMode_t dirs)
{
    portMode_t missing = (portMode_t)0;

    if ((dirs & MODE_TX) && !s->txClaimed) {
        const int sm = pio_claim_unused_sm(pio1, false);
        if (sm < 0) {
            missing = (portMode_t)(missing | MODE_TX);
        } else {
            /* Programs are loaded once and shared by both ports */
            if (txProgramOffset < 0) {
                txProgramOffset = pio_add_program(pio1, &uart_tx_program);
            }
            s->sm_tx       = (uint)sm;
            s->tx_intr_bit = sm_txnfull_bits[sm];
            s->txClaimed   = true;
        }
    }

    if ((dirs & MODE_RX) && !s->rxClaimed) {
        const int sm = pio_claim_unused_sm(pio1, false);
        if (sm < 0) {
            missing = (portMode_t)(missing | MODE_RX);
        } else {
            if (rxProgramOffset < 0) {
                rxProgramOffset = pio_add_program(pio1, &uart_rx_program);
            }
            s->sm_rx       = (uint)sm;
            s->rx_intr_bit = sm_rxnempty_bits[sm];
            s->rxClaimed   = true;
        }
    }

    return missing;
}

/* Stop a direction and give its state machine back. */
static void piouartReleaseTx(piouartDevice_t *s)
{
    if (!s->txClaimed) {
        return;
    }
    pio_set_irqn_source_enabled(pio1, (uint)s->irq_index,
        pio_get_tx_fifo_not_full_interrupt_source(s->sm_tx), false);
    pio_sm_set_enabled(pio1, s->sm_tx, false);
    pio_sm_unclaim(pio1, s->sm_tx);
    /* Off the PIO, so that it does not stay driven at whatever level it stopped */
    gpio_deinit(s->tx_pin);
    s->tx_intr_bit = 0;
    s->txClaimed   = false;
}

static void piouartReleaseRx(piouartDevice_t *s)
{
    if (!s->rxClaimed) {
        return;
    }
    pio_set_irqn_source_enabled(pio1, (uint)s->irq_index,
        pio_get_rx_fifo_not_empty_interrupt_source(s->sm_rx), false);
    pio_sm_set_enabled(pio1, s->sm_rx, false);
    pio_sm_unclaim(pio1, s->sm_rx);
    gpio_deinit(s->rx_pin);
    s->rx_intr_bit = 0;
    s->rxClaimed   = false;
}

/* Inversion of a running port, see uart_tx_program_init() */
static void piouartApplyInversion(const piouartDevice_t *s)
{
    const uint override = (s->port.port.options & SERIAL_INVERTED) ? GPIO_OVERRIDE_INVERT
                                                                   : GPIO_OVERRIDE_NORMAL;
    if (s->rxClaimed) {
        gpio_set_inover(s->rx_pin, override);
    }
    if (s->txClaimed) {
        gpio_set_outover(s->tx_pin, override);
    }
}

/*
 * (Re)start the state machines of the directions in dirs at the port's baud
 * rate. pio_sm_init() clears the FIFOs of the state machine it restarts.
 */
static void piouartStartDirections(piouartDevice_t *s, portMode_t dirs)
{
    const uint baudRate = (uint)s->port.port.baudRate;
    const bool inverted = (s->port.port.options & SERIAL_INVERTED) != 0;

    if ((dirs & MODE_TX) && s->txClaimed) {
        pio_sm_set_enabled(pio1, s->sm_tx, false);
        uart_tx_program_init(pio1, s->sm_tx, (uint)txProgramOffset, s->tx_pin, baudRate, inverted);
    }
    if ((dirs & MODE_RX) && s->rxClaimed) {
        pio_sm_set_enabled(pio1, s->sm_rx, false);
        uart_rx_program_init(pio1, s->sm_rx, (uint)rxProgramOffset, s->rx_pin, baudRate, inverted);
    }
}

/* ── piouartInit — shared init for UART3 and UART4 ──────────────────────── */

/*
 * piouartInit is NOT inside any #ifdef USE_UARTn guard so that it compiles
 * whenever USE_UART3 or USE_UART4 (or both) are defined.  Placing it inside
 * the USE_UART3 guard would cause a linker error if only USE_UART4 is defined.
 */
static uartPort_t *piouartInit(piouartDevice_t *s, uint8_t uartNumber,
                                uint tx_pin, uint rx_pin, int irq_index,
                                uint32_t baudRate, portMode_t mode,
                                portOptions_t options)
{
    s->uartNumber  = uartNumber;
    s->tx_pin      = tx_pin;
    s->rx_pin      = rx_pin;
    s->irq_index   = irq_index;

    /*
     * The programs are 8N1. Refuse the port rather than run a protocol with
     * parity or two stop bits (SBUS is 8E2) on it and deliver garbage.
     */
    bool usable = !(options & (SERIAL_PARITY_EVEN | SERIAL_STOPBITS_2));
    if (!usable) {
        LOG_ERROR(SYSTEM, "UART%d: PIO UART is 8N1 only, port not opened", uartNumber);
    }

    /* PIO1 reaches 32 GPIOs from its base; another driver on it may have moved it */
    const uint gpioBase = pio_get_gpio_base(pio1);
    if (usable && (tx_pin < gpioBase || tx_pin >= gpioBase + 32 || rx_pin < gpioBase || rx_pin >= gpioBase + 32)) {
        LOG_ERROR(SYSTEM, "UART%d: pins outside the PIO1 GPIO range, port not opened", uartNumber);
        usable = false;
    }

    /*
     * One state machine per direction of the mode. Nothing tells the driver that
     * a port was closed, so a re-open is where a direction the new mode does not
     * use gives its state machine back. A port that cannot get what its mode
     * needs is left stopped, with no state machine.
     */
    const portMode_t wanted = usable ? piouartWantedDirections(mode, options) : (portMode_t)0;
    if (!(wanted & MODE_TX)) {
        piouartReleaseTx(s);
    }
    if (!(wanted & MODE_RX)) {
        piouartReleaseRx(s);
    }
    if (!usable) {
        return NULL;
    }
    if (piouartClaimDirections(s, wanted) != 0) {
        piouartReleaseTx(s);
        piouartReleaseRx(s);
        LOG_ERROR(SYSTEM, "UART%d: no free PIO1 state machine, port not opened", uartNumber);
        return NULL;
    }

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

    piouartStartDirections(s, MODE_RXTX);

    return &s->port;
}

/* ── UART3 (INAV) → PIO1, IRQ0 ─────────────────────────────────────────── */

#ifdef USE_UART3

static piouartDevice_t piouart0Device;

static void pio1Irq0Handler(void)
{
    piouartIrqHandler(&piouart0Device);
}

uartPort_t *serialUART3(uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    uartPort_t *p = piouartInit(&piouart0Device, 3,
                                ioTagToGpioNum(IO_TAG(UART3_TX_PIN)),
                                ioTagToGpioNum(IO_TAG(UART3_RX_PIN)),
                                0 /* irq_index = PIO1_IRQ0 */,
                                baudRate, mode, options);
    if (!p) {
        return NULL;
    }

    irq_set_exclusive_handler(PIO1_IRQ_0, pio1Irq0Handler);
    irq_set_enabled(PIO1_IRQ_0, true);

    /* Enable RX interrupt; TX is enabled on demand in pioUartWrite() */
    if (mode & MODE_RX) {
        pio_set_irqn_source_enabled(pio1, 0,
            pio_get_rx_fifo_not_empty_interrupt_source(piouart0Device.sm_rx), true);
    }

    return p;
}

#endif /* USE_UART3 */

/* ── UART4 (INAV) → PIO1, IRQ1 ─────────────────────────────────────────── */

#ifdef USE_UART4

static piouartDevice_t piouart1Device;

static void pio1Irq1Handler(void)
{
    piouartIrqHandler(&piouart1Device);
}

uartPort_t *serialUART4(uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    uartPort_t *p = piouartInit(&piouart1Device, 4,
                                ioTagToGpioNum(IO_TAG(UART4_TX_PIN)),
                                ioTagToGpioNum(IO_TAG(UART4_RX_PIN)),
                                1 /* irq_index = PIO1_IRQ1 */,
                                baudRate, mode, options);
    if (!p) {
        return NULL;
    }

    irq_set_exclusive_handler(PIO1_IRQ_1, pio1Irq1Handler);
    irq_set_enabled(PIO1_IRQ_1, true);

    if (mode & MODE_RX) {
        pio_set_irqn_source_enabled(pio1, 1,
            pio_get_rx_fifo_not_empty_interrupt_source(piouart1Device.sm_rx), true);
    }

    return p;
}

#endif /* USE_UART4 */

/* ── Vtable implementations ──────────────────────────────────────────────── */

static void pioUartWrite(serialPort_t *instance, uint8_t ch)
{
    piouartDevice_t *s = (piouartDevice_t *)instance;

    if (!s->txClaimed) {
        return;     /* receive-only port: nothing drains the buffer */
    }

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
     * Reinitialise the port's SMs with new clock divider.  pio_sm_clear_fifos() is
     * called internally by uart_tx/rx_program_init → pio_sm_init, so any bytes
     * queued in the TX ring buffer but not yet pushed to the PIO FIFO will still
     * be sent, but bytes already in the hardware FIFO are discarded.  Callers
     * that need a clean switch (e.g. GPS auto-baud) should drain the port first.
     */
    piouartStartDirections(s, MODE_RXTX);
}

static bool pioUartIsTransmitBufferEmpty(const serialPort_t *instance)
{
    const piouartDevice_t *s = (const piouartDevice_t *)instance;
    return s->port.port.txBufferTail == s->port.port.txBufferHead;
}

static void pioUartSetMode(serialPort_t *instance, portMode_t mode)
{
    piouartDevice_t *s = (piouartDevice_t *)instance;

    /*
     * A direction the port was not opened with gets its state machine now. If
     * none is free, the direction is dropped from the mode.
     */
    portMode_t added = (portMode_t)0;
    if ((mode & MODE_TX) && !s->txClaimed) {
        added = (portMode_t)(added | MODE_TX);
    }
    if ((mode & MODE_RX) && !s->rxClaimed) {
        added = (portMode_t)(added | MODE_RX);
    }
    if (added) {
        const portMode_t missing = piouartClaimDirections(s, added);
        if (missing) {
            LOG_ERROR(SYSTEM, "UART%d: no free PIO1 state machine, direction not added", s->uartNumber);
        }
        mode = (portMode_t)(mode & ~missing);
        piouartStartDirections(s, (portMode_t)(added & ~missing));
    }

    s->port.port.mode = mode;
    if (s->rxClaimed) {
        pio_set_irqn_source_enabled(pio1, (uint)s->irq_index,
            pio_get_rx_fifo_not_empty_interrupt_source(s->sm_rx),
            (mode & MODE_RX) != 0);
    }
}

static void pioUartSetOptions(serialPort_t *instance, portOptions_t options)
{
    piouartDevice_t *s = (piouartDevice_t *)instance;
    s->port.port.options = options;

    piouartApplyInversion(s);
}

#endif /* USE_UART3 || USE_UART4 */
