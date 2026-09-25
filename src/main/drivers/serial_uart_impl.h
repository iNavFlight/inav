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

#pragma once

// device specific uart implementation is defined here

#include "build/atomic.h"

#include "drivers/nvic.h"

extern const struct serialPortVTable uartVTable[];

void uartStartTxDMA(uartPort_t *s);

#ifdef USE_UART_RX_DMA
#include "common/utils.h"
#include "drivers/dma.h"
#include "drivers/timer.h"

#if defined(STM32F4) || defined(STM32F7)
// Here each UART's receiver is wired to fixed streams on a fixed channel (the DMA request
// mapping tables of the F4 and F7 reference manuals), so a target naming any other gets a
// build error rather than a port that quietly receives nothing
#define UART_RX_DMA_IS(tag, dma, stream, channel) \
    (DMATAG_GET_DMA(tag) == (dma) && DMATAG_GET_STREAM(tag) == (stream) && DMATAG_GET_CHANNEL(tag) == (channel))
#ifdef UART1_RX_DMA
STATIC_ASSERT(UART_RX_DMA_IS(UART1_RX_DMA, 2, 2, 4) || UART_RX_DMA_IS(UART1_RX_DMA, 2, 5, 4), UART1_RX_DMA_is_DMA2_stream_2_or_5_channel_4);
#endif
#ifdef UART2_RX_DMA
STATIC_ASSERT(UART_RX_DMA_IS(UART2_RX_DMA, 1, 5, 4), UART2_RX_DMA_is_DMA1_stream_5_channel_4);
#endif
#ifdef UART3_RX_DMA
STATIC_ASSERT(UART_RX_DMA_IS(UART3_RX_DMA, 1, 1, 4), UART3_RX_DMA_is_DMA1_stream_1_channel_4);
#endif
#ifdef UART4_RX_DMA
STATIC_ASSERT(UART_RX_DMA_IS(UART4_RX_DMA, 1, 2, 4), UART4_RX_DMA_is_DMA1_stream_2_channel_4);
#endif
#ifdef UART5_RX_DMA
STATIC_ASSERT(UART_RX_DMA_IS(UART5_RX_DMA, 1, 0, 4), UART5_RX_DMA_is_DMA1_stream_0_channel_4);
#endif
#ifdef UART6_RX_DMA
STATIC_ASSERT(UART_RX_DMA_IS(UART6_RX_DMA, 2, 1, 5) || UART_RX_DMA_IS(UART6_RX_DMA, 2, 2, 5), UART6_RX_DMA_is_DMA2_stream_1_or_2_channel_5);
#endif
#ifdef UART7_RX_DMA
STATIC_ASSERT(UART_RX_DMA_IS(UART7_RX_DMA, 1, 3, 5), UART7_RX_DMA_is_DMA1_stream_3_channel_5);
#endif
#ifdef UART8_RX_DMA
STATIC_ASSERT(UART_RX_DMA_IS(UART8_RX_DMA, 1, 6, 5), UART8_RX_DMA_is_DMA1_stream_6_channel_5);
#endif
#endif

// A UART takes a stream only if nothing has it, or it is this UART's own from an earlier
// open. A stream any of the target's timer outputs is mapped to is left to them even
// before they claim it, because the serial ports open first: a target naming one by
// mistake gets a UART on its byte interrupt, not a motor without DSHOT
static inline bool uartRxDmaAvailable(DMA_t dma, UARTDevice_e device)
{
    for (int i = 0; i < timerHardwareCount; i++) {
        if (dmaGetByTag(timerHardware[i].dmaTag) == dma) {
            return false;
        }
    }
    return dmaGetOwner(dma) == OWNER_FREE || (dmaGetOwner(dma) == OWNER_SERIAL && dma->resourceIndex == RESOURCE_INDEX(device));
}

// Hands the port's reception to the DMA stream its target names. False leaves it on the
// byte interrupt: no stream named, the stream taken by something else, a port opened
// without RX, or one whose owner wants each byte as it lands (rxCallback)
bool uartRxDmaStart(uartPort_t *s);

static inline bool uartRxDmaRunning(const uartPort_t *s)
{
    return s->rxDma != NULL;
}

// The stream counts down what is left of its lap round the ring, which says where it has got to
static inline uint32_t uartRxBufferHead(const uartPort_t *s)
{
    if (s->rxDma) {
#if defined(AT32F43x)
        const uint32_t left = s->rxDma->ref->dtcnt;
#else
        const uint32_t left = s->rxDma->ref->NDTR;
#endif
        return (s->port.rxBufferSize - left) % s->port.rxBufferSize;
    }
    return s->port.rxBufferHead;
}
#else
static inline bool uartRxDmaStart(uartPort_t *s) { (void)s; return false; }
static inline bool uartRxDmaRunning(const uartPort_t *s) { (void)s; return false; }
static inline uint32_t uartRxBufferHead(const uartPort_t *s) { return s->port.rxBufferHead; }
#endif

// A stream keeps writing where it was started, so it is started again on the new ring
static inline void uartSetRxBuffer(serialPort_t *instance, volatile uint8_t *buffer, uint32_t size)
{
    uartPort_t *s = (uartPort_t *)instance;

    ATOMIC_BLOCK(NVIC_PRIO_MAX) {
        s->port.rxBuffer = buffer;
        s->port.rxBufferSize = size;
        s->port.rxBufferHead = 0;
        s->port.rxBufferTail = 0;
    }
    if (uartRxDmaRunning(s)) {
        uartRxDmaStart(s);
    }
}

uartPort_t *serialUART1(uint32_t baudRate, portMode_t mode, portOptions_t options);
uartPort_t *serialUART2(uint32_t baudRate, portMode_t mode, portOptions_t options);
uartPort_t *serialUART3(uint32_t baudRate, portMode_t mode, portOptions_t options);
uartPort_t *serialUART4(uint32_t baudRate, portMode_t mode, portOptions_t options);
uartPort_t *serialUART5(uint32_t baudRate, portMode_t mode, portOptions_t options);
uartPort_t *serialUART6(uint32_t baudRate, portMode_t mode, portOptions_t options);
uartPort_t *serialUART7(uint32_t baudRate, portMode_t mode, portOptions_t options);
uartPort_t *serialUART8(uint32_t baudRate, portMode_t mode, portOptions_t options);

