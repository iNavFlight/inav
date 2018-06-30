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

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#include "drivers/io.h"
#include "drivers/io_impl.h"
#include "drivers/uart_inverter.h"

#if defined(USE_UART_INVERTER)

static void inverterPinSet(IO_t pin, bool on)
{
    IOWrite(pin, on);
}

static void initInverter(ioTag_t ioTag)
{
    IO_t pin = IOGetByTag(ioTag);
    IOInit(pin, OWNER_INVERTER, RESOURCE_OUTPUT, 0);
    IOConfigGPIO(pin, IOCFG_OUT_PP);

    inverterPinSet(pin, false);
}

void uartInverterInit(void)
{

// UART1
#ifdef INVERTER_PIN_UART1_RX
    initInverter(IO_TAG(INVERTER_PIN_UART1_RX));
#endif

#ifdef INVERTER_PIN_UART1_TX
    initInverter(IO_TAG(INVERTER_PIN_UART1_TX));
#endif

// UART2
#ifdef INVERTER_PIN_UART2_RX
    initInverter(IO_TAG(INVERTER_PIN_UART2_RX));
#endif

#ifdef INVERTER_PIN_UART2_TX
    initInverter(IO_TAG(INVERTER_PIN_UART2_TX));
#endif

// UART3
#ifdef INVERTER_PIN_UART3_RX
    initInverter(IO_TAG(INVERTER_PIN_UART3_RX));
#endif

#ifdef INVERTER_PIN_UART3_TX
    initInverter(IO_TAG(INVERTER_PIN_UART3_TX));
#endif

// UART4
#ifdef INVERTER_PIN_UART4_RX
    initInverter(IO_TAG(INVERTER_PIN_UART4_RX));
#endif

#ifdef INVERTER_PIN_UART4_TX
    initInverter(IO_TAG(INVERTER_PIN_UART4_TX));
#endif

// UART5
#ifdef INVERTER_PIN_UART5_RX
    initInverter(IO_TAG(INVERTER_PIN_UART5_RX));
#endif

#ifdef INVERTER_PIN_UART5_TX
    initInverter(IO_TAG(INVERTER_PIN_UART5_TX));
#endif

// UART6
#ifdef INVERTER_PIN_UART6_RX
    initInverter(IO_TAG(INVERTER_PIN_UART6_RX));
#endif

#ifdef INVERTER_PIN_UART6_TX
    initInverter(IO_TAG(INVERTER_PIN_UART6_TX));
#endif

}

void uartInverterSet(USART_TypeDef *USARTx, uartInverterLine_e line, bool enable)
{
    IO_t rx_pin = IO_NONE;
    IO_t tx_pin = IO_NONE;

// UART1
#if defined(INVERTER_PIN_UART1_RX) || defined(INVERTER_PIN_UART1_TX)
    if (USARTx == USART1) {
#if defined(INVERTER_PIN_UART1_RX)
        rx_pin = IOGetByTag(IO_TAG(INVERTER_PIN_UART1_RX));
#endif
#if defined(INVERTER_PIN_UART1_TX)
        tx_pin = IOGetByTag(IO_TAG(INVERTER_PIN_UART1_TX));
#endif
    }
#endif

// UART2
#if defined(INVERTER_PIN_UART2_RX) || defined(INVERTER_PIN_UART2_TX)
    if (USARTx == USART2) {
#if defined(INVERTER_PIN_UART2_RX)
        rx_pin = IOGetByTag(IO_TAG(INVERTER_PIN_UART2_RX));
#endif
#if defined(INVERTER_PIN_UART2_TX)
        tx_pin = IOGetByTag(IO_TAG(INVERTER_PIN_UART2_TX));
#endif
    }
#endif

// UART3
#if defined(INVERTER_PIN_UART3_RX) || defined(INVERTER_PIN_UART3_TX)
    if (USARTx == USART3) {
#if defined(INVERTER_PIN_UART3_RX)
        rx_pin = IOGetByTag(IO_TAG(INVERTER_PIN_UART3_RX));
#endif
#if defined(INVERTER_PIN_UART3_TX)
        tx_pin = IOGetByTag(IO_TAG(INVERTER_PIN_UART3_TX));
#endif
    }
#endif

// UART4
#if defined(INVERTER_PIN_UART4_RX) || defined(INVERTER_PIN_UART4_TX)
    if (USARTx == USART4) {
#if defined(INVERTER_PIN_UART4_RX)
        rx_pin = IOGetByTag(IO_TAG(INVERTER_PIN_UART4_RX));
#endif
#if defined(INVERTER_PIN_UART4_TX)
        tx_pin = IOGetByTag(IO_TAG(INVERTER_PIN_UART4_TX));
#endif
    }
#endif

// UART5
#if defined(INVERTER_PIN_UART5_RX) || defined(INVERTER_PIN_UART5_TX)
    if (USARTx == USART5) {
#if defined(INVERTER_PIN_UART5_RX)
        rx_pin = IOGetByTag(IO_TAG(INVERTER_PIN_UART5_RX));
#endif
#if defined(INVERTER_PIN_UART5_TX)
        tx_pin = IOGetByTag(IO_TAG(INVERTER_PIN_UART5_TX));
#endif
    }
#endif

// UART6
#if defined(INVERTER_PIN_UART6_RX) || defined(INVERTER_PIN_UART6_TX)
    if (USARTx == USART6) {
#if defined(INVERTER_PIN_UART6_RX)
        rx_pin = IOGetByTag(IO_TAG(INVERTER_PIN_UART6_RX));
#endif
#if defined(INVERTER_PIN_UART6_TX)
        tx_pin = IOGetByTag(IO_TAG(INVERTER_PIN_UART6_TX));
#endif
    }
#endif

    // Now do the actual work
    if (rx_pin != IO_NONE && (line & UART_INVERTER_LINE_RX)) {
        inverterPinSet(rx_pin, enable);
    }
    if (tx_pin != IO_NONE && (line & UART_INVERTER_LINE_TX)) {
        inverterPinSet(tx_pin, enable);
    }
}

#endif
