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
 * RP2350 hardware UART driver for INAV — Milestone 6
 *
 * Maps INAV serial ports to RP2350 hardware UARTs:
 *   UART1 (INAV) → uart0 (RP2350 UART0)
 *   UART2 (INAV) → uart1 (RP2350 UART1)
 *
 * Pin assignments are defined in target.h:
 *   UART1_TX_PIN / UART1_RX_PIN
 *   UART2_TX_PIN / UART2_RX_PIN
 *
 * RX is interrupt-driven (IRQ on FIFO not-empty).
 * TX is interrupt-driven (IRQ on FIFO not-full, enabled on first write).
 *
 * Signal inversion (e.g. SBUS) is done in the GPIO controller via
 * gpio_set_inover() — no external inverter circuit required.
 */

#include "platform.h"

#if defined(USE_UART1) || defined(USE_UART2) || defined(USE_UART3) || defined(USE_UART4)

#include <stdbool.h>
#include <stdint.h>

#include "build/build_config.h"

#include "common/utils.h"

#include "drivers/io.h"
#include "drivers/serial.h"
#include "drivers/serial_uart.h"
#include "drivers/serial_uart_impl.h"

#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/uart.h"

#define RP2350_UART_RX_BUFFER_SIZE 256
#define RP2350_UART_TX_BUFFER_SIZE 256

/* Convert INAV ioTag_t to RP2350 raw GPIO number. */
static inline uint ioTagToGpioNum(ioTag_t tag)
{
    return (uint)(DEFIO_TAG_GPIOID(tag) * 16 + DEFIO_TAG_PIN(tag));
}

/*
 * Per-UART device state.
 *
 * uartPort_t is the first member so that a pointer to this struct is also
 * a valid pointer to uartPort_t (and, by extension, serialPort_t), satisfying
 * C99 §6.7.2.1 (first-member address rule).  The vtable functions receive
 * serialPort_t * and cast directly to rp2350UartDevice_t *.
 */
typedef struct {
    uartPort_t  port;
    uart_inst_t *dev;
    uint        tx_pin;
    uint        rx_pin;
    volatile uint8_t rxBuffer[RP2350_UART_RX_BUFFER_SIZE];
    volatile uint8_t txBuffer[RP2350_UART_TX_BUFFER_SIZE];
} rp2350UartDevice_t;

/* ── Forward declarations ────────────────────────────────────────────────── */

static void     rp2350UartWrite(serialPort_t *instance, uint8_t ch);
static uint32_t rp2350UartTotalRxBytesWaiting(const serialPort_t *instance);
static uint32_t rp2350UartTotalTxBytesFree(const serialPort_t *instance);
static uint8_t  rp2350UartRead(serialPort_t *instance);
static void     rp2350UartSetBaudRate(serialPort_t *s, uint32_t baudRate);
static bool     rp2350UartIsTransmitBufferEmpty(const serialPort_t *s);
static void     rp2350UartSetMode(serialPort_t *instance, portMode_t mode);
static void     rp2350UartSetOptions(serialPort_t *instance, portOptions_t options);

/* Single shared vtable — identical for UART0 and UART1 */
static const struct serialPortVTable rp2350UartVTable = {
    .serialWrite                 = rp2350UartWrite,
    .serialTotalRxWaiting        = rp2350UartTotalRxBytesWaiting,
    .serialTotalTxFree           = rp2350UartTotalTxBytesFree,
    .serialRead                  = rp2350UartRead,
    .serialSetBaudRate           = rp2350UartSetBaudRate,
    .isSerialTransmitBufferEmpty = rp2350UartIsTransmitBufferEmpty,
    .setMode                     = rp2350UartSetMode,
    .setOptions                  = rp2350UartSetOptions,
    .isConnected                 = NULL,
    .writeBuf                    = NULL,
    .beginWrite                  = NULL,
    .endWrite                    = NULL,
    .isIdle                      = NULL,
};

/* ── Hardware configure ──────────────────────────────────────────────────── */

static void rp2350UartHwConfigure(rp2350UartDevice_t *s)
{
    portOptions_t options = s->port.port.options;
    uart_parity_t parity  = (options & SERIAL_PARITY_EVEN) ? UART_PARITY_EVEN
                                                            : UART_PARITY_NONE;
    uint stop_bits        = (options & SERIAL_STOPBITS_2) ? 2 : 1;

    uart_set_baudrate(s->dev, s->port.port.baudRate);
    uart_set_format(s->dev, 8, stop_bits, parity);

    /*
     * RP2350 GPIO signal inversion — replaces the external inverter circuit
     * that STM32 targets require for SBUS (100000 baud, 8E2, inverted).
     */
    if (options & SERIAL_INVERTED) {
        gpio_set_inover(s->rx_pin, GPIO_OVERRIDE_INVERT);
        gpio_set_outover(s->tx_pin, GPIO_OVERRIDE_INVERT);
    } else {
        gpio_set_inover(s->rx_pin, GPIO_OVERRIDE_NORMAL);
        gpio_set_outover(s->tx_pin, GPIO_OVERRIDE_NORMAL);
    }
}

/* ── Shared IRQ handler ──────────────────────────────────────────────────── */

static void rp2350UartIrqHandler(rp2350UartDevice_t *s)
{
    /* RX: drain hardware FIFO into the software ring buffer */
    while (uart_is_readable(s->dev)) {
        uint8_t ch = (uint8_t)uart_get_hw(s->dev)->dr;
        if (s->port.port.rxCallback) {
            s->port.port.rxCallback(ch, s->port.port.rxCallbackData);
        } else {
            s->port.port.rxBuffer[s->port.port.rxBufferHead] = ch;
            s->port.port.rxBufferHead =
                (s->port.port.rxBufferHead + 1) % s->port.port.rxBufferSize;
        }
    }

    /* TX: push from software ring buffer into hardware FIFO */
    while (uart_is_writable(s->dev)) {
        if (s->port.port.txBufferTail != s->port.port.txBufferHead) {
            uart_get_hw(s->dev)->dr =
                s->port.port.txBuffer[s->port.port.txBufferTail];
            s->port.port.txBufferTail =
                (s->port.port.txBufferTail + 1) % s->port.port.txBufferSize;
        } else {
            /* Buffer empty — disable TX interrupt until next write */
            uart_set_irqs_enabled(s->dev,
                                  (s->port.port.mode & MODE_RX) != 0,
                                  false);
            break;
        }
    }
}

/* ── UART1 (INAV) → RP2350 uart0 ────────────────────────────────────────── */

#ifdef USE_UART1

static rp2350UartDevice_t uart0Device;

static void uart0IrqHandler(void)
{
    rp2350UartIrqHandler(&uart0Device);
}

uartPort_t *serialUART1(uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    rp2350UartDevice_t *s = &uart0Device;

    s->dev    = uart0;
    s->tx_pin = ioTagToGpioNum(IO_TAG(UART1_TX_PIN));
    s->rx_pin = ioTagToGpioNum(IO_TAG(UART1_RX_PIN));

    s->port.port.vTable       = &rp2350UartVTable;
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

    uart_init(uart0, baudRate);
    if (mode & MODE_TX) { gpio_set_function(s->tx_pin, UART_FUNCSEL_NUM(uart0, s->tx_pin)); }
    if (mode & MODE_RX) { gpio_set_function(s->rx_pin, UART_FUNCSEL_NUM(uart0, s->rx_pin)); }

    rp2350UartHwConfigure(s);

    irq_set_exclusive_handler(UART0_IRQ, uart0IrqHandler);
    irq_set_enabled(UART0_IRQ, true);

    /* Enable RX interrupt; TX interrupt is enabled on demand by rp2350UartWrite() */
    uart_set_irqs_enabled(uart0, (mode & MODE_RX) != 0, false);

    return &s->port;
}

#endif /* USE_UART1 */

/* ── UART2 (INAV) → RP2350 uart1 ────────────────────────────────────────── */

#ifdef USE_UART2

static rp2350UartDevice_t uart1Device;

static void uart1IrqHandler(void)
{
    rp2350UartIrqHandler(&uart1Device);
}

uartPort_t *serialUART2(uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    rp2350UartDevice_t *s = &uart1Device;

    s->dev    = uart1;
    s->tx_pin = ioTagToGpioNum(IO_TAG(UART2_TX_PIN));
    s->rx_pin = ioTagToGpioNum(IO_TAG(UART2_RX_PIN));

    s->port.port.vTable       = &rp2350UartVTable;
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

    uart_init(uart1, baudRate);
    if (mode & MODE_TX) { gpio_set_function(s->tx_pin, UART_FUNCSEL_NUM(uart1, s->tx_pin)); }
    if (mode & MODE_RX) { gpio_set_function(s->rx_pin, UART_FUNCSEL_NUM(uart1, s->rx_pin)); }

    rp2350UartHwConfigure(s);

    irq_set_exclusive_handler(UART1_IRQ, uart1IrqHandler);
    irq_set_enabled(UART1_IRQ, true);

    uart_set_irqs_enabled(uart1, (mode & MODE_RX) != 0, false);

    return &s->port;
}

#endif /* USE_UART2 */

/* ── Vtable implementations ──────────────────────────────────────────────── */

static void rp2350UartWrite(serialPort_t *instance, uint8_t ch)
{
    rp2350UartDevice_t *s = (rp2350UartDevice_t *)instance;

    s->port.port.txBuffer[s->port.port.txBufferHead] = ch;
    s->port.port.txBufferHead =
        (s->port.port.txBufferHead + 1) % s->port.port.txBufferSize;

    /* Enable TX FIFO-empty interrupt to drain the ring buffer */
    uart_set_irqs_enabled(s->dev,
                          (s->port.port.mode & MODE_RX) != 0,
                          true);
}

static uint32_t rp2350UartTotalRxBytesWaiting(const serialPort_t *instance)
{
    const rp2350UartDevice_t *s = (const rp2350UartDevice_t *)instance;
    if (s->port.port.rxBufferHead >= s->port.port.rxBufferTail) {
        return s->port.port.rxBufferHead - s->port.port.rxBufferTail;
    }
    return s->port.port.rxBufferSize + s->port.port.rxBufferHead
           - s->port.port.rxBufferTail;
}

static uint32_t rp2350UartTotalTxBytesFree(const serialPort_t *instance)
{
    const rp2350UartDevice_t *s = (const rp2350UartDevice_t *)instance;
    uint32_t bytesUsed;
    if (s->port.port.txBufferHead >= s->port.port.txBufferTail) {
        bytesUsed = s->port.port.txBufferHead - s->port.port.txBufferTail;
    } else {
        bytesUsed = s->port.port.txBufferSize + s->port.port.txBufferHead
                    - s->port.port.txBufferTail;
    }
    return (s->port.port.txBufferSize - 1) - bytesUsed;
}

static uint8_t rp2350UartRead(serialPort_t *instance)
{
    rp2350UartDevice_t *s = (rp2350UartDevice_t *)instance;
    uint8_t ch = s->port.port.rxBuffer[s->port.port.rxBufferTail];
    s->port.port.rxBufferTail =
        (s->port.port.rxBufferTail + 1) % s->port.port.rxBufferSize;
    return ch;
}

static void rp2350UartSetBaudRate(serialPort_t *instance, uint32_t baudRate)
{
    rp2350UartDevice_t *s = (rp2350UartDevice_t *)instance;
    s->port.port.baudRate = baudRate;
    rp2350UartHwConfigure(s);
}

static bool rp2350UartIsTransmitBufferEmpty(const serialPort_t *instance)
{
    const rp2350UartDevice_t *s = (const rp2350UartDevice_t *)instance;
    return s->port.port.txBufferTail == s->port.port.txBufferHead;
}

static void rp2350UartSetMode(serialPort_t *instance, portMode_t mode)
{
    rp2350UartDevice_t *s = (rp2350UartDevice_t *)instance;
    s->port.port.mode = mode;
    uart_set_irqs_enabled(s->dev,
                          (mode & MODE_RX) != 0,
                          !rp2350UartIsTransmitBufferEmpty(instance));
}

static void rp2350UartSetOptions(serialPort_t *instance, portOptions_t options)
{
    rp2350UartDevice_t *s = (rp2350UartDevice_t *)instance;
    s->port.port.options = options;
    rp2350UartHwConfigure(s);
}

/* ── Stubs required by serial_uart.h / pwm_mapping.c ────────────────────── */

void uartGetPortPins(UARTDevice_e device, serialPortPins_t *pins)
{
    switch (device) {
#ifdef USE_UART1
    case UARTDEV_1:
        pins->txPin = IO_TAG(UART1_TX_PIN);
        pins->rxPin = IO_TAG(UART1_RX_PIN);
        break;
#endif
#ifdef USE_UART2
    case UARTDEV_2:
        pins->txPin = IO_TAG(UART2_TX_PIN);
        pins->rxPin = IO_TAG(UART2_RX_PIN);
        break;
#endif
#ifdef USE_UART3
    case UARTDEV_3:
        pins->txPin = IO_TAG(UART3_TX_PIN);
        pins->rxPin = IO_TAG(UART3_RX_PIN);
        break;
#endif
#ifdef USE_UART4
    case UARTDEV_4:
        pins->txPin = IO_TAG(UART4_TX_PIN);
        pins->rxPin = IO_TAG(UART4_RX_PIN);
        break;
#endif
    default:
        pins->txPin = IO_TAG(NONE);
        pins->rxPin = IO_TAG(NONE);
        break;
    }
}

void uartClearIdleFlag(uartPort_t *s)
{
    /* PL011 UART clears the idle flag automatically — nothing to do */
    UNUSED(s);
}

/*
 * uartVTable[] — required by serial_uart_impl.h's extern declaration.
 * The RP2350 UART ports use rp2350UartVTable instead; this symbol exists
 * only to satisfy the linker.
 */
const struct serialPortVTable uartVTable[1] = { { 0 } };

#endif /* USE_UART1 || USE_UART2 || USE_UART3 || USE_UART4 */
