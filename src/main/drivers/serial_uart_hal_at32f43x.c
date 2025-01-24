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

/*
 * Authors:
 * Dominic Clifton - Serial port abstraction, Separation of common STM32 code for cleanflight, various cleanups.
 * Hamasaki/Timecop - Initial baseflight code
*/
#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#include "build/build_config.h"

#include "common/utils.h"

#include "drivers/uart_inverter.h"

#include "serial.h"
#include "serial_uart.h"
#include "serial_uart_impl.h"

static void usartConfigurePinInversion(uartPort_t *uartPort) {
#if !defined(USE_UART_INVERTER) 
    UNUSED(uartPort);
#else
    bool inverted = uartPort->port.options & SERIAL_INVERTED;

#ifdef USE_UART_INVERTER
    uartInverterLine_e invertedLines = UART_INVERTER_LINE_NONE;
    if (uartPort->port.mode & MODE_RX) {
        invertedLines |= UART_INVERTER_LINE_RX;
    }
    if (uartPort->port.mode & MODE_TX) {
        invertedLines |= UART_INVERTER_LINE_TX;
    }
    uartInverterSet(uartPort->USARTx, invertedLines, inverted);
#endif

#endif
}

static void uartReconfigure(uartPort_t *uartPort)
{ 
    usart_enable(uartPort->USARTx, FALSE);
    uint32_t baud_rate =  115200;
    usart_data_bit_num_type data_bit = USART_DATA_8BITS;
    usart_stop_bit_num_type stop_bit = USART_STOP_1_BIT;
    usart_parity_selection_type parity_type = USART_PARITY_EVEN;

    baud_rate = uartPort->port.baudRate;
    stop_bit = (uartPort->port.options & SERIAL_STOPBITS_2) ? USART_STOP_2_BIT : USART_STOP_1_BIT;

    // according to the stm32 documentation wordlen has to be 9 for parity bits
    // this does not seem to matter for rx but will give bad data on tx!
    if (uartPort->port.options & SERIAL_PARITY_EVEN) {
        data_bit = USART_DATA_9BITS;
    } else {
        data_bit = USART_DATA_8BITS;
    }
    usart_init(uartPort->USARTx, baud_rate, data_bit, stop_bit);

    parity_type   = (uartPort->port.options & SERIAL_PARITY_EVEN) ? USART_PARITY_EVEN : USART_PARITY_NONE;
    usart_parity_selection_config(uartPort->USARTx, parity_type);
    usart_hardware_flow_control_set (uartPort->USARTx, USART_HARDWARE_FLOW_NONE);

    if (uartPort->port.mode & MODE_RX)
          usart_receiver_enable(uartPort->USARTx, TRUE);
    if (uartPort->port.mode & MODE_TX)
          usart_transmitter_enable(uartPort->USARTx, TRUE);

    usartConfigurePinInversion(uartPort);
    uartConfigurePinSwap(uartPort);

    if (uartPort->port.options & SERIAL_BIDIR)
        usart_single_line_halfduplex_select(uartPort->USARTx, TRUE);
    else
        usart_single_line_halfduplex_select(uartPort->USARTx, FALSE);

     usart_enable(uartPort->USARTx, TRUE);
}

serialPort_t *uartOpen(usart_type *USARTx, serialReceiveCallbackPtr rxCallback, void *rxCallbackData, uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    uartPort_t *s = NULL;

    if (false) {
#ifdef USE_UART1
    } else if (USARTx == USART1) {
        s = serialUART1(baudRate, mode, options);

#endif
#ifdef USE_UART2
    } else if (USARTx == USART2) {
        s = serialUART2(baudRate, mode, options);
#endif
#ifdef USE_UART3
    } else if (USARTx == USART3) {
        s = serialUART3(baudRate, mode, options);
#endif
#ifdef USE_UART4
    } else if (USARTx == UART4) {
        s = serialUART4(baudRate, mode, options);
#endif
#ifdef USE_UART5
    } else if (USARTx == UART5) {
        s = serialUART5(baudRate, mode, options);
#endif
#ifdef USE_UART6
    } else if (USARTx == USART6) {
        s = serialUART6(baudRate, mode, options);
#endif
#ifdef USE_UART7
    } else if (USARTx == UART7) {
        s = serialUART7(baudRate, mode, options);
#endif
#ifdef USE_UART8
    } else if (USARTx == UART8) {
        s = serialUART8(baudRate, mode, options);
#endif

    } else {
        return (serialPort_t *)s;
    }

    // common serial initialisation code should move to serialPort::init()
    s->port.rxBufferHead = s->port.rxBufferTail = 0;
    s->port.txBufferHead = s->port.txBufferTail = 0;
    // callback works for IRQ-based RX ONLY
    s->port.rxCallback = rxCallback;
    s->port.rxCallbackData = rxCallbackData;
    s->port.mode = mode;
    s->port.baudRate = baudRate;
    s->port.options = options;

    uartReconfigure(s);

    if (mode & MODE_RX) {
        usart_flag_clear(s->USARTx, USART_RDBF_FLAG);
        usart_interrupt_enable (s->USARTx, USART_RDBF_INT, TRUE);

    }

    if (mode & MODE_TX) {
        usart_interrupt_enable (s->USARTx, USART_TDBE_INT, TRUE);

    }
    usart_enable(s->USARTx, TRUE);

    return (serialPort_t *)s;
}

void uartSetBaudRate(serialPort_t *instance, uint32_t baudRate)
{
    uartPort_t *uartPort = (uartPort_t *)instance;
    uartPort->port.baudRate = baudRate;
    uartReconfigure(uartPort);
}

void uartSetMode(serialPort_t *instance, portMode_t mode)
{
    uartPort_t *uartPort = (uartPort_t *)instance;
    uartPort->port.mode = mode;
    uartReconfigure(uartPort);
}

void uartSetOptions(serialPort_t *instance, portOptions_t options)
{
    uartPort_t *uartPort = (uartPort_t *)instance;
    uartPort->port.options = options;
    uartReconfigure(uartPort);
}

uint32_t uartTotalRxBytesWaiting(const serialPort_t *instance)
{
    const uartPort_t *s = (const uartPort_t*)instance;

    if (s->port.rxBufferHead >= s->port.rxBufferTail) {
        return s->port.rxBufferHead - s->port.rxBufferTail;
    } else {
        return s->port.rxBufferSize + s->port.rxBufferHead - s->port.rxBufferTail;
    }
}

uint32_t uartTotalTxBytesFree(const serialPort_t *instance)
{
    const uartPort_t *s = (const uartPort_t*)instance;

    uint32_t bytesUsed;

    if (s->port.txBufferHead >= s->port.txBufferTail) {
        bytesUsed = s->port.txBufferHead - s->port.txBufferTail;
    } else {
        bytesUsed = s->port.txBufferSize + s->port.txBufferHead - s->port.txBufferTail;
    }

    return (s->port.txBufferSize - 1) - bytesUsed;
}

bool isUartTransmitBufferEmpty(const serialPort_t *instance)
{
    const uartPort_t *s = (const uartPort_t *)instance;
    return s->port.txBufferTail == s->port.txBufferHead;
}

uint8_t uartRead(serialPort_t *instance)
{
    uint8_t ch;
    uartPort_t *s = (uartPort_t *)instance;

    ch = s->port.rxBuffer[s->port.rxBufferTail];
    if (s->port.rxBufferTail + 1 >= s->port.rxBufferSize) {
        s->port.rxBufferTail = 0;
    } else {
        s->port.rxBufferTail++;
    }

    return ch;
}

void uartWrite(serialPort_t *instance, uint8_t ch)
{
    uartPort_t *s = (uartPort_t *)instance;
    s->port.txBuffer[s->port.txBufferHead] = ch;
    if (s->port.txBufferHead + 1 >= s->port.txBufferSize) {
        s->port.txBufferHead = 0;
    } else {
        s->port.txBufferHead++;
    }

    usart_interrupt_enable (s->USARTx, USART_TDBE_INT, TRUE);

}

bool isUartIdle(serialPort_t *instance)
{
    uartPort_t *s = (uartPort_t *)instance;
    if(usart_flag_get(s->USARTx, USART_IDLEF_FLAG)) {

        uartClearIdleFlag(s);
        return true;
    } else {
        return false;
    }
}

const struct serialPortVTable uartVTable[] = {
    {
        .serialWrite = uartWrite,
        .serialTotalRxWaiting = uartTotalRxBytesWaiting,
        .serialTotalTxFree = uartTotalTxBytesFree,
        .serialRead = uartRead,
        .serialSetBaudRate = uartSetBaudRate,
        .isSerialTransmitBufferEmpty = isUartTransmitBufferEmpty,
        .setMode = uartSetMode,
        .setOptions = uartSetOptions,
        .isConnected = NULL,
        .writeBuf = NULL,
        .beginWrite = NULL,
        .endWrite = NULL,
        .isIdle = isUartIdle,
    }
};
