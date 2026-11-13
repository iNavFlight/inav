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
#include "drivers/time.h"

extern const struct serialPortVTable uartVTable[];

#if defined(USE_UART_RX_DMA) || defined(USE_UART_TX_DMA)
#include "common/utils.h"
#include "drivers/dma.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"

#if defined(STM32F4) || defined(STM32F7)
// Fixed streams and channel per UART receiver and transmitter (reference manual request
// tables): a target naming another gets a build error instead of a silent port
#define UART_DMA_IS(tag, dma, stream, channel)     ((tag) == DMA_TAG_AUTO || (tag) == DMA_NONE || \
    (DMATAG_GET_DMA(tag) == (dma) && DMATAG_GET_STREAM(tag) == (stream) && DMATAG_GET_CHANNEL(tag) == (channel)))
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

// Every stop gives its stream back, so a port holds only the streams it runs on
#define uartDmaRelease(dma)     dmaInit((dma), OWNER_FREE, 0)

// Free, and not one a timer output claims later in init
static inline bool uartDmaStreamAvailable(DMA_t dma)
{
    if (pwmIsDmaStreamReserved(dma)) {
        return false;
    }
    return dmaGetOwner(dma) == OWNER_FREE;
}

#ifdef USE_UART_RX_DMA
#define uartRxDmaOf(s)      ((s)->rxDma)
#else
#define uartRxDmaOf(s)      ((DMA_t)NULL)
#endif
#ifdef USE_UART_TX_DMA
#define uartTxDmaOf(s)      ((s)->txDma)
#else
#define uartTxDmaOf(s)      ((DMA_t)NULL)
#endif

// Streams tried, in order, for a port whose target names none (DMA_TAG_AUTO)
#if defined(STM32F4) || defined(STM32F7)
#define UART_DMA_CANDIDATES 2
static const dmaTag_t uartRxDmaCandidates[UARTDEV_MAX][UART_DMA_CANDIDATES] = {
    [UARTDEV_1] = { DMA_TAG(2, 2, 4), DMA_TAG(2, 5, 4) },
    [UARTDEV_2] = { DMA_TAG(1, 5, 4) },
    [UARTDEV_3] = { DMA_TAG(1, 1, 4) },
    [UARTDEV_4] = { DMA_TAG(1, 2, 4) },
    [UARTDEV_5] = { DMA_TAG(1, 0, 4) },
    [UARTDEV_6] = { DMA_TAG(2, 1, 5), DMA_TAG(2, 2, 5) },
    [UARTDEV_7] = { DMA_TAG(1, 3, 5) },
    [UARTDEV_8] = { DMA_TAG(1, 6, 5) },
};
static const dmaTag_t uartTxDmaCandidates[UARTDEV_MAX][UART_DMA_CANDIDATES] = {
    [UARTDEV_1] = { DMA_TAG(2, 7, 4) },
    [UARTDEV_2] = { DMA_TAG(1, 6, 4) },
    [UARTDEV_3] = { DMA_TAG(1, 3, 4), DMA_TAG(1, 4, 7) },
    [UARTDEV_4] = { DMA_TAG(1, 4, 4) },
    [UARTDEV_5] = { DMA_TAG(1, 7, 4) },
    [UARTDEV_6] = { DMA_TAG(2, 6, 5), DMA_TAG(2, 7, 5) },
    [UARTDEV_7] = { DMA_TAG(1, 1, 5) },
    [UARTDEV_8] = { DMA_TAG(1, 0, 5) },
};
#define uartDmaCandidate(table, device, i)      ((table)[device][i])
#elif defined(STM32H7)
// The DMAMUX routes any UART to any stream; DMA2 streams 0-2 are the ADCs'
#define UART_DMA_CANDIDATES 13
static const dmaTag_t uartDmaStreams[UART_DMA_CANDIDATES] = {
    DMA_TAG(1, 0, 0), DMA_TAG(1, 1, 0), DMA_TAG(1, 2, 0), DMA_TAG(1, 3, 0), DMA_TAG(1, 4, 0), DMA_TAG(1, 5, 0),
    DMA_TAG(1, 6, 0), DMA_TAG(1, 7, 0), DMA_TAG(2, 3, 0), DMA_TAG(2, 4, 0), DMA_TAG(2, 5, 0), DMA_TAG(2, 6, 0),
    DMA_TAG(2, 7, 0),
};
#define uartDmaCandidate(table, device, i)      (uartDmaStreams[i])
#elif defined(AT32F43x)
// The DMAMUX routes any UART to any channel; the ADC's is excluded below
#define UART_DMA_CANDIDATES 14
static const dmaTag_t uartDmaStreams[UART_DMA_CANDIDATES] = {
    DMA_TAG(1, 1, 0), DMA_TAG(1, 2, 0), DMA_TAG(1, 3, 0), DMA_TAG(1, 4, 0), DMA_TAG(1, 5, 0), DMA_TAG(1, 6, 0),
    DMA_TAG(1, 7, 0), DMA_TAG(2, 1, 0), DMA_TAG(2, 2, 0), DMA_TAG(2, 3, 0), DMA_TAG(2, 4, 0), DMA_TAG(2, 5, 0),
    DMA_TAG(2, 6, 0), DMA_TAG(2, 7, 0),
};
#define uartDmaCandidate(table, device, i)      (uartDmaStreams[i])
#endif

// Streams claimed after the first serial ports open: the SD card would give up on a taken one, the AT32 ADC takes it anyway
static inline bool uartDmaStreamUsedElsewhere(DMA_t dma)
{
#if (defined(STM32F4) || defined(STM32F7)) && defined(USE_SDCARD_SDIO)
#ifdef SDCARD_SDIO_DMA
    if (dma == dmaGetByTag(SDCARD_SDIO_DMA)) {
        return true;
    }
#else
    if (dma == dmaGetByTag(DMA_TAG(2, 3, 0)) || dma == dmaGetByTag(DMA_TAG(2, 6, 0))) {
        return true;
    }
#endif
#endif
#if defined(AT32F43x)
#ifdef ADC1_DMA_STREAM
    if (dma == dmaGetByRef(ADC1_DMA_STREAM)) {
#else
    if (dma == dmaGetByRef(DMA2_CHANNEL1)) {
#endif
        return true;
    }
#endif
    UNUSED(dma);
    return false;
}

// Streams a target names for some port: an automatic pick leaves them alone, whichever port opens first
static const dmaTag_t uartNamedDmaTags[] = {
#if defined(UART1_RX_DMA) && (UART1_RX_DMA != DMA_TAG_AUTO) && (UART1_RX_DMA != DMA_NONE)
    UART1_RX_DMA,
#endif
#if defined(UART1_TX_DMA) && (UART1_TX_DMA != DMA_TAG_AUTO) && (UART1_TX_DMA != DMA_NONE)
    UART1_TX_DMA,
#endif
#if defined(UART2_RX_DMA) && (UART2_RX_DMA != DMA_TAG_AUTO) && (UART2_RX_DMA != DMA_NONE)
    UART2_RX_DMA,
#endif
#if defined(UART2_TX_DMA) && (UART2_TX_DMA != DMA_TAG_AUTO) && (UART2_TX_DMA != DMA_NONE)
    UART2_TX_DMA,
#endif
#if defined(UART3_RX_DMA) && (UART3_RX_DMA != DMA_TAG_AUTO) && (UART3_RX_DMA != DMA_NONE)
    UART3_RX_DMA,
#endif
#if defined(UART3_TX_DMA) && (UART3_TX_DMA != DMA_TAG_AUTO) && (UART3_TX_DMA != DMA_NONE)
    UART3_TX_DMA,
#endif
#if defined(UART4_RX_DMA) && (UART4_RX_DMA != DMA_TAG_AUTO) && (UART4_RX_DMA != DMA_NONE)
    UART4_RX_DMA,
#endif
#if defined(UART4_TX_DMA) && (UART4_TX_DMA != DMA_TAG_AUTO) && (UART4_TX_DMA != DMA_NONE)
    UART4_TX_DMA,
#endif
#if defined(UART5_RX_DMA) && (UART5_RX_DMA != DMA_TAG_AUTO) && (UART5_RX_DMA != DMA_NONE)
    UART5_RX_DMA,
#endif
#if defined(UART5_TX_DMA) && (UART5_TX_DMA != DMA_TAG_AUTO) && (UART5_TX_DMA != DMA_NONE)
    UART5_TX_DMA,
#endif
#if defined(UART6_RX_DMA) && (UART6_RX_DMA != DMA_TAG_AUTO) && (UART6_RX_DMA != DMA_NONE)
    UART6_RX_DMA,
#endif
#if defined(UART6_TX_DMA) && (UART6_TX_DMA != DMA_TAG_AUTO) && (UART6_TX_DMA != DMA_NONE)
    UART6_TX_DMA,
#endif
#if defined(UART7_RX_DMA) && (UART7_RX_DMA != DMA_TAG_AUTO) && (UART7_RX_DMA != DMA_NONE)
    UART7_RX_DMA,
#endif
#if defined(UART7_TX_DMA) && (UART7_TX_DMA != DMA_TAG_AUTO) && (UART7_TX_DMA != DMA_NONE)
    UART7_TX_DMA,
#endif
#if defined(UART8_RX_DMA) && (UART8_RX_DMA != DMA_TAG_AUTO) && (UART8_RX_DMA != DMA_NONE)
    UART8_RX_DMA,
#endif
#if defined(UART8_TX_DMA) && (UART8_TX_DMA != DMA_TAG_AUTO) && (UART8_TX_DMA != DMA_NONE)
    UART8_TX_DMA,
#endif
    DMA_NONE,
};

static inline bool uartDmaStreamNamed(DMA_t dma)
{
    for (unsigned i = 0; i < ARRAYLEN(uartNamedDmaTags); i++) {
        if (uartNamedDmaTags[i] != DMA_NONE && dma == dmaGetByTag(uartNamedDmaTags[i])) {
            return true;
        }
    }
    return false;
}

// A receiver that takes its bytes in bursts, on a port that is not half duplex
#define uartRxTakesBursts(port)     (((port)->options & (SERIAL_RX_BURSTS | SERIAL_BIDIR)) == SERIAL_RX_BURSTS)

// The named stream, else the first free candidate; byte-wise receivers and half-duplex links were never measured on DMA
static inline dmaTag_t uartDmaPick(dmaTag_t named, const dmaTag_t (*candidates)[UART_DMA_CANDIDATES], UARTDevice_e device,
                                   const serialPort_t *port, DMA_t otherDirection)
{
    if (named != DMA_TAG_AUTO) {
        const DMA_t dma = dmaGetByTag(named);
        return (named != DMA_NONE && dma && uartDmaStreamAvailable(dma)) ? named : DMA_NONE;
    }
    if ((port->rxCallback && !uartRxTakesBursts(port)) || (port->options & SERIAL_BIDIR)) {
        return DMA_NONE;
    }
    for (int i = 0; i < UART_DMA_CANDIDATES; i++) {
        const dmaTag_t tag = uartDmaCandidate(candidates, device, i);
        const DMA_t dma = dmaGetByTag(tag);
        if (tag != DMA_NONE && dma && dma != otherDirection && !uartDmaStreamUsedElsewhere(dma) && !uartDmaStreamNamed(dma) &&
            uartDmaStreamAvailable(dma)) {
            return tag;
        }
    }
    UNUSED(candidates);
    UNUSED(device);
    return DMA_NONE;
}
#endif

#ifdef USE_UART_RX_DMA
// False leaves the port on the byte interrupt: no stream named or free, no RX, or an
// rxCallback that wants each byte as it lands
bool uartRxDmaStart(uartPort_t *s);
// Stops the stream and its interrupts and gives it back: for a port about to be reset or closed
void uartRxDmaStop(uartPort_t *s);

#define uartRxDmaRefused(s)     ((s)->port.rxCallback && !uartRxTakesBursts(&(s)->port))

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

// Start bit, 8 data bits, parity and stop bits
static inline uint32_t uartCharNs(const uartPort_t *s)
{
    const uint32_t bits = 10 + ((s->port.options & SERIAL_PARITY_EVEN) ? 1 : 0) + ((s->port.options & SERIAL_STOPBITS_2) ? 1 : 0);
    return bits * (1000000000U / s->port.baudRate);
}

// Hands rxCallback what the stream wrote since the last call. The newest byte ended at lastEndUs and
// the others are timed back from it a character apart: a gap of a character would have ended the burst
static inline void uartRxDmaDeliver(uartPort_t *s, timeUs_t lastEndUs)
{
    // A passthrough drops the callback and reads the ring itself, as on the byte interrupt
    if (!s->port.rxCallback) {
        return;
    }

    const uint32_t size = s->port.rxBufferSize;
    const uint32_t charNs = uartCharNs(s);
    uint32_t tail = s->port.rxBufferTail;

    for (uint32_t left = (uartRxBufferHead(s) + size - tail) % size; left > 0; left--) {
        s->port.rxByteTimeUs = lastEndUs - ((left - 1) * charNs) / 1000;
        s->port.rxCallback(s->port.rxBuffer[tail], s->port.rxCallbackData);
        tail = (tail + 1) % size;
    }
    s->port.rxBufferTail = tail;
}

// The UART flags idle one character after the last stop bit
static inline void uartRxDmaIdle(uartPort_t *s)
{
    uartRxDmaDeliver(s, microsISR() - uartCharNs(s) / 1000);
}
#else
static inline bool uartRxDmaStart(uartPort_t *s) { (void)s; return false; }
static inline void uartRxDmaStop(uartPort_t *s) { (void)s; }
static inline bool uartRxDmaRunning(const uartPort_t *s) { (void)s; return false; }
static inline uint32_t uartRxBufferHead(const uartPort_t *s) { return s->port.rxBufferHead; }
#endif

#ifdef USE_UART_TX_DMA
// False leaves the port on the byte interrupt: no stream named or free, or no TX
bool uartTxDmaStart(uartPort_t *s);
// Stops the stream, dropping any transfer on its way, and gives it back: for a port about to be reset or closed
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

// What a closed port had still to send or had received is dropped, as reopening it would
static inline void uartRelease(serialPort_t *instance)
{
    uartTxDmaStop((uartPort_t *)instance);
    uartRxDmaStop((uartPort_t *)instance);
}

// A stream keeps writing where it was started, so it is started again on the new ring
static inline void uartSetRxBuffer(serialPort_t *instance, volatile uint8_t *buffer, uint32_t size)
{
    uartPort_t *s = (uartPort_t *)instance;

    // Restarted in the same block: an idle or half ring interrupt in between would read the new ring with the old count
    ATOMIC_BLOCK(NVIC_PRIO_MAX) {
        s->port.rxBuffer = buffer;
        s->port.rxBufferSize = size;
        s->port.rxBufferHead = 0;
        s->port.rxBufferTail = 0;
        if (uartRxDmaRunning(s)) {
            uartRxDmaStart(s);
        }
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

