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

#include "drivers/time.h"
#include "drivers/io.h"

#include "serial.h"
#include "serial_uart.h"
#include "serial_uart_impl.h"
#include "serial_uart_device_at32.h"


 
void uartIrqHandler(uartPort_t *s)
{
    if (usart_flag_get(s->USARTx, USART_RDBF_FLAG) == SET) {
        if (s->port.rxCallback) {
            s->port.rxCallback(s->USARTx->dt, s->port.rxCallbackData);
        } else {
            s->port.rxBuffer[s->port.rxBufferHead] = s->USARTx->dt;
            s->port.rxBufferHead = (s->port.rxBufferHead + 1) % s->port.rxBufferSize;
        }
    }

    if (usart_flag_get(s->USARTx, USART_TDBE_FLAG) == SET) {
        if (s->port.txBufferTail != s->port.txBufferHead) {
            usart_data_transmit(s->USARTx, s->port.txBuffer[s->port.txBufferTail]);
            s->port.txBufferTail = (s->port.txBufferTail + 1) % s->port.txBufferSize;
        } else {
            usart_interrupt_enable (s->USARTx, USART_TDBE_INT, FALSE);
        }
    }

    if (usart_flag_get(s->USARTx, USART_ROERR_FLAG) == SET)
    {
        usart_flag_clear(s->USARTx, USART_ROERR_FLAG);
    }
}

void uartGetPortPins(UARTDevice_e device, serialPortPins_t * pins)
{
    uartDevice_t *uart = uartHardwareMap[device];

    if (uart) {
        pins->txPin = uart->tx;
        pins->rxPin = uart->rx;
    }
    else {
        pins->txPin = IO_TAG(NONE);
        pins->rxPin = IO_TAG(NONE);
    }
}

void uartClearIdleFlag(uartPort_t *s)
{
    (void) s->USARTx->sts;
    (void) s->USARTx->dt;
}

uartPort_t *serialUART(UARTDevice_e device, uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    uartPort_t *s;

    uartDevice_t *uart = uartHardwareMap[device];
    if (!uart) return NULL;

    s = &(uart->port);
    s->port.vTable = uartVTable;

    s->port.baudRate = baudRate;

    s->port.rxBuffer = uart->rxBuffer;
    s->port.txBuffer = uart->txBuffer;
    s->port.rxBufferSize = sizeof(uart->rxBuffer);
    s->port.txBufferSize = sizeof(uart->txBuffer);

    s->USARTx = uart->dev;

    IO_t tx = IOGetByTag(uart->tx);
    IO_t rx = IOGetByTag(uart->rx);

    if (uart->rcc_apb2)
        RCC_ClockCmd(uart->rcc_apb2, ENABLE);

    if (uart->rcc_apb1)
        RCC_ClockCmd(uart->rcc_apb1, ENABLE);
    
    if (uart->rcc_ahb1)
        RCC_ClockCmd(uart->rcc_apb1, ENABLE);

    if (options & SERIAL_BIDIR) {
        IOInit(tx, OWNER_SERIAL, RESOURCE_UART_TXRX, RESOURCE_INDEX(device));
        if (options & SERIAL_BIDIR_PP) {
            IOConfigGPIOAF(tx, IOCFG_AF_PP, uart->af);
        } else {
            IOConfigGPIOAF(tx,
                    (options & SERIAL_BIDIR_NOPULL) ? IOCFG_AF_OD : IOCFG_AF_OD_UP,
                    uart->af);
        }
    }
    else {
        if (mode & MODE_TX) {
            IOInit(tx, OWNER_SERIAL, RESOURCE_UART_TX, RESOURCE_INDEX(device));
            IOConfigGPIOAF(tx, IOCFG_AF_PP, uart->af);
        }

        if (mode & MODE_RX) {
            IOInit(rx, OWNER_SERIAL, RESOURCE_UART_RX, RESOURCE_INDEX(device));
            IOConfigGPIOAF(rx, IOCFG_AF_PP, uart->af);
        }
    } 

     nvic_irq_enable(uart->irq, uart->irqPriority, 0);
 

    return s;
}

#ifdef USE_UART1
uartPort_t *serialUART1(uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    return serialUART(UARTDEV_1, baudRate, mode, options);
}

// USART1 Rx/Tx IRQ Handler
void USART1_IRQHandler(void)
{
    uartPort_t *s = &(uartHardwareMap[UARTDEV_1]->port);
    uartIrqHandler(s);
}

#endif

#ifdef USE_UART2
// USART2 (RX + TX by IRQ)
uartPort_t *serialUART2(uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    return serialUART(UARTDEV_2, baudRate, mode, options);
}

void USART2_IRQHandler(void)
{
    uartPort_t *s = &(uartHardwareMap[UARTDEV_2]->port);
    uartIrqHandler(s);
}
#endif

#ifdef USE_UART3
// USART3
uartPort_t *serialUART3(uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    return serialUART(UARTDEV_3, baudRate, mode, options);
}

void USART3_IRQHandler(void)
{
    uartPort_t *s = &(uartHardwareMap[UARTDEV_3]->port);
    uartIrqHandler(s);
}
#endif

#ifdef USE_UART4
// USART4
uartPort_t *serialUART4(uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    return serialUART(UARTDEV_4, baudRate, mode, options);
}

void UART4_IRQHandler(void)
{
    uartPort_t *s = &(uartHardwareMap[UARTDEV_4]->port);
    uartIrqHandler(s);
}
#endif

#ifdef USE_UART5
// USART5
uartPort_t *serialUART5(uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    return serialUART(UARTDEV_5, baudRate, mode, options);
}

void UART5_IRQHandler(void)
{
    uartPort_t *s = &(uartHardwareMap[UARTDEV_5]->port);
    uartIrqHandler(s);
}
#endif

#ifdef USE_UART6
// USART6
uartPort_t *serialUART6(uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    return serialUART(UARTDEV_6, baudRate, mode, options);
}

void USART6_IRQHandler(void)
{
    uartPort_t *s = &(uartHardwareMap[UARTDEV_6]->port);
    uartIrqHandler(s);
}
#endif

#ifdef USE_UART7
uartPort_t *serialUART7(uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    return serialUART(UARTDEV_7, baudRate, mode, options);
}

// UART7 Rx/Tx IRQ Handler
void UART7_IRQHandler(void)
{
    uartPort_t *s = &(uartHardwareMap[UARTDEV_7]->port);
    uartIrqHandler(s);
}
#endif

#ifdef USE_UART8
uartPort_t *serialUART8(uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    return serialUART(UARTDEV_8, baudRate, mode, options);
}

// UART8 Rx/Tx IRQ Handler
void UART8_IRQHandler(void)
{
    uartPort_t *s = &(uartHardwareMap[UARTDEV_8]->port);
    uartIrqHandler(s);
}
#endif
