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
#include "io_impl.h"

#include "inverter.h"

#if defined(INVERTER_PIN_UART1) && !defined(INVERTER_PIN_UART1_RX)
#define INVERTER_PIN_UART1_RX INVERTER_PIN_UART1
#endif

#if defined(INVERTER_PIN_UART2) && !defined(INVERTER_PIN_UART2_RX)
#define INVERTER_PIN_UART2_RX INVERTER_PIN_UART2
#endif

#if defined(INVERTER_PIN_UART3) && !defined(INVERTER_PIN_UART3_RX)
#define INVERTER_PIN_UART3_RX INVERTER_PIN_UART3
#endif

#if defined(INVERTER_PIN_UART4) && !defined(INVERTER_PIN_UART4_RX)
#define INVERTER_PIN_UART4_RX INVERTER_PIN_UART4
#endif

#if defined(INVERTER_PIN_UART5) && !defined(INVERTER_PIN_UART5_RX)
#define INVERTER_PIN_UART5_RX INVERTER_PIN_UART5
#endif

#if defined(INVERTER_PIN_UART6) && !defined(INVERTER_PIN_UART6_RX)
#define INVERTER_PIN_UART6_RX INVERTER_PIN_UART6
#endif


#ifdef USE_INVERTER
static void inverterSet(IO_t pin, bool on)
{
    IOWrite(pin, on);
}

static void initInverter(ioTag_t ioTag)
{
    IO_t pin = IOGetByTag(ioTag);
    IOInit(pin, OWNER_INVERTER, RESOURCE_OUTPUT, 0);
    IOConfigGPIO(pin, IOCFG_OUT_PP);

    inverterSet(pin, false);
}
#endif

void initInverters(void)
{
#ifdef INVERTER_PIN_UART1_TX
    initInverter(IO_TAG(INVERTER_PIN_UART1_TX));
#endif

#ifdef INVERTER_PIN_UART1_RX
    initInverter(IO_TAG(INVERTER_PIN_UART1_RX));
#endif

#ifdef INVERTER_PIN_UART2_TX
    initInverter(IO_TAG(INVERTER_PIN_UART2_TX));
#endif

#ifdef INVERTER_PIN_UART2_RX
    initInverter(IO_TAG(INVERTER_PIN_UART2_RX));
#endif

#ifdef INVERTER_PIN_UART3_TX
    initInverter(IO_TAG(INVERTER_PIN_UART3_TX));
#endif

#ifdef INVERTER_PIN_UART3_RX
    initInverter(IO_TAG(INVERTER_PIN_UART3_RX));
#endif

#ifdef INVERTER_PIN_UART4_TX
    initInverter(IO_TAG(INVERTER_PIN_UART4_TX));
#endif

#ifdef INVERTER_PIN_UART4_RX
    initInverter(IO_TAG(INVERTER_PIN_UART4_RX));
#endif

#ifdef INVERTER_PIN_UART5_TX
    initInverter(IO_TAG(INVERTER_PIN_UART5_TX));
#endif

#ifdef INVERTER_PIN_UART5_RX
    initInverter(IO_TAG(INVERTER_PIN_UART5_RX));
#endif

#ifdef INVERTER_PIN_UART6_TX
    initInverter(IO_TAG(INVERTER_PIN_UART6_TX));
#endif

#ifdef INVERTER_PIN_UART6_RX
    initInverter(IO_TAG(INVERTER_PIN_UART6_RX));
#endif
}

void enableInverter(USART_TypeDef *USARTx, bool on)
{
#ifdef USE_INVERTER
    // TX path
    IO_t pin = IO_NONE;

#ifdef INVERTER_PIN_UART1_TX
    if (USARTx == USART1) {
        pin = IOGetByTag(IO_TAG(INVERTER_PIN_UART1_TX));
    }
#endif

#ifdef INVERTER_PIN_UART2_TX
    if (USARTx == USART2) {
        pin = IOGetByTag(IO_TAG(INVERTER_PIN_UART2_TX));
    }
#endif

#ifdef INVERTER_PIN_UART3_TX
    if (USARTx == USART3) {
        pin = IOGetByTag(IO_TAG(INVERTER_PIN_UART3_TX));
    }
#endif

#ifdef INVERTER_PIN_USART4_TX
    if (USARTx == USART4) {
        pin = IOGetByTag(IO_TAG(INVERTER_PIN_USART4_TX));
    }
#endif

#ifdef INVERTER_PIN_USART5_TX
    if (USARTx == USART5) {
        pin = IOGetByTag(IO_TAG(INVERTER_PIN_USART5_TX));
    }
#endif

#ifdef INVERTER_PIN_UART6_TX
    if (USARTx == USART6) {
        pin = IOGetByTag(IO_TAG(INVERTER_PIN_UART6_TX));
    }
#endif

    if (pin != IO_NONE) {
        inverterSet(pin, on);
    }

    // RX path
    pin = IO_NONE;

#ifdef INVERTER_PIN_UART1_RX
    if (USARTx == USART1) {
        pin = IOGetByTag(IO_TAG(INVERTER_PIN_UART1_RX));
    }
#endif

#ifdef INVERTER_PIN_UART2_RX
    if (USARTx == USART2) {
        pin = IOGetByTag(IO_TAG(INVERTER_PIN_UART2_RX));
    }
#endif

#ifdef INVERTER_PIN_UART3_RX
    if (USARTx == USART3) {
        pin = IOGetByTag(IO_TAG(INVERTER_PIN_UART3_RX));
    }
#endif

#ifdef INVERTER_PIN_USART4_RX
    if (USARTx == USART4) {
        pin = IOGetByTag(IO_TAG(INVERTER_PIN_USART4_RX));
    }
#endif

#ifdef INVERTER_PIN_USART5_RX
    if (USARTx == USART5) {
        pin = IOGetByTag(IO_TAG(INVERTER_PIN_USART5_RX));
    }
#endif

#ifdef INVERTER_PIN_UART6_RX
    if (USARTx == USART6) {
        pin = IOGetByTag(IO_TAG(INVERTER_PIN_UART6_RX));
    }
#endif

    if (pin != IO_NONE) {
        inverterSet(pin, on);
    }

#else
    UNUSED(USARTx);
    UNUSED(on);
#endif
}
