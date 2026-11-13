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

#if defined(USE_UART_RX_DMA) || defined(USE_UART_TX_DMA)
#include "common/utils.h"
#include "drivers/dma.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"

#if defined(STM32F4) || defined(STM32F7)
// Fixed streams and channel per UART receiver and transmitter (reference manual request
// tables): a target naming another gets a build error instead of a silent port
#define UART_DMA_IS(tag, dma, stream, channel)     (DMATAG_GET_DMA(tag) == (dma) && DMATAG_GET_STREAM(tag) == (stream) && DMATAG_GET_CHANNEL(tag) == (channel))
#ifdef UART1_RX_DMA
STATIC_ASSERT(UART_DMA_IS(UART1_RX_DMA, 2, 2, 4) || UART_DMA_IS(UART1_RX_DMA, 2, 5, 4), UART1_RX_DMA_is_DMA2_stream_2_or_5_channel_4);
#endif
#ifdef UART2_RX_DMA
STATIC_ASSERT(UART_DMA_IS(UART2_RX_DMA, 1, 5, 4), UART2_RX_DMA_is_DMA1_stream_5_channel_4);
#endif
#ifdef UART3_RX_DMA
STATIC_ASSERT(UART_DMA_IS(UART3_RX_DMA, 1, 1, 4), UART3_RX_DMA_is_DMA1_stream_1_channel_4);
#endif
#ifdef UART4_RX_DMA
STATIC_ASSERT(UART_DMA_IS(UART4_RX_DMA, 1, 2, 4), UART4_RX_DMA_is_DMA1_stream_2_channel_4);
#endif
#ifdef UART5_RX_DMA
STATIC_ASSERT(UART_DMA_IS(UART5_RX_DMA, 1, 0, 4), UART5_RX_DMA_is_DMA1_stream_0_channel_4);
#endif
#ifdef UART6_RX_DMA
STATIC_ASSERT(UART_DMA_IS(UART6_RX_DMA, 2, 1, 5) || UART_DMA_IS(UART6_RX_DMA, 2, 2, 5), UART6_RX_DMA_is_DMA2_stream_1_or_2_channel_5);
#endif
#ifdef UART7_RX_DMA
STATIC_ASSERT(UART_DMA_IS(UART7_RX_DMA, 1, 3, 5), UART7_RX_DMA_is_DMA1_stream_3_channel_5);
#endif
#ifdef UART8_RX_DMA
STATIC_ASSERT(UART_DMA_IS(UART8_RX_DMA, 1, 6, 5), UART8_RX_DMA_is_DMA1_stream_6_channel_5);
#endif
#ifdef UART1_TX_DMA
STATIC_ASSERT(UART_DMA_IS(UART1_TX_DMA, 2, 7, 4), UART1_TX_DMA_is_DMA2_stream_7_channel_4);
#endif
#ifdef UART2_TX_DMA
STATIC_ASSERT(UART_DMA_IS(UART2_TX_DMA, 1, 6, 4), UART2_TX_DMA_is_DMA1_stream_6_channel_4);
#endif
#ifdef UART3_TX_DMA
STATIC_ASSERT(UART_DMA_IS(UART3_TX_DMA, 1, 3, 4) || UART_DMA_IS(UART3_TX_DMA, 1, 4, 7), UART3_TX_DMA_is_DMA1_stream_3_channel_4_or_stream_4_channel_7);
#endif
#ifdef UART4_TX_DMA
STATIC_ASSERT(UART_DMA_IS(UART4_TX_DMA, 1, 4, 4), UART4_TX_DMA_is_DMA1_stream_4_channel_4);
#endif
#ifdef UART5_TX_DMA
STATIC_ASSERT(UART_DMA_IS(UART5_TX_DMA, 1, 7, 4), UART5_TX_DMA_is_DMA1_stream_7_channel_4);
#endif
#ifdef UART6_TX_DMA
STATIC_ASSERT(UART_DMA_IS(UART6_TX_DMA, 2, 6, 5) || UART_DMA_IS(UART6_TX_DMA, 2, 7, 5), UART6_TX_DMA_is_DMA2_stream_6_or_7_channel_5);
#endif
#ifdef UART7_TX_DMA
STATIC_ASSERT(UART_DMA_IS(UART7_TX_DMA, 1, 1, 5), UART7_TX_DMA_is_DMA1_stream_1_channel_5);
#endif
#ifdef UART8_TX_DMA
STATIC_ASSERT(UART_DMA_IS(UART8_TX_DMA, 1, 0, 5), UART8_TX_DMA_is_DMA1_stream_0_channel_5);
#endif
#endif

// Free, or this UART's own from an earlier open, and not one a timer output claims later in init
static inline bool uartDmaStreamAvailable(DMA_t dma, UARTDevice_e device)
{
    if (pwmIsDmaStreamReserved(dma)) {
        return false;
    }
    return dmaGetOwner(dma) == OWNER_FREE || (dmaGetOwner(dma) == OWNER_SERIAL && dma->resourceIndex == RESOURCE_INDEX(device));
}
#endif

#ifdef USE_UART_RX_DMA
// False leaves the port on the byte interrupt: no stream named or free, no RX, or an
// rxCallback that wants each byte as it lands
bool uartRxDmaStart(uartPort_t *s);

static inline bool uartRxDmaRunning(const uartPort_t *s)
{
    return s->rxDma != NULL;
}

// NDTR counts down the rest of the lap round the ring
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

#ifdef USE_UART_TX_DMA
// False leaves the port on the byte interrupt: no stream named or free, or no TX
bool uartTxDmaStart(uartPort_t *s);
// Stops the stream, dropping any transfer on its way: for a port about to be reset
void uartTxDmaStop(uartPort_t *s);
// Sends what is queued, unless a transfer is already on its way: its end starts the next
void uartStartTxDMA(uartPort_t *s);

static inline bool uartTxDmaRunning(const uartPort_t *s)
{
    return s->txDma != NULL;
}

#if defined(STM32F7) || defined(STM32H7)
// The stream reads RAM, and a buffer a feature swaps in (MSP DisplayPort, gimbal) can still have its bytes in the D-cache
static inline void uartTxDmaCleanCache(const volatile uint8_t *data, uint32_t count)
{
    const uint32_t start = (uint32_t)data & ~31U;  // this CMSIS does not align to the 32 byte line
    SCB_CleanDCache_by_Addr((uint32_t *)start, (uint32_t)data + count - start);
}
#endif
#else
static inline bool uartTxDmaStart(uartPort_t *s) { (void)s; return false; }
static inline void uartTxDmaStop(uartPort_t *s) { (void)s; }
static inline void uartStartTxDMA(uartPort_t *s) { (void)s; }
static inline bool uartTxDmaRunning(const uartPort_t *s) { (void)s; return false; }
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

