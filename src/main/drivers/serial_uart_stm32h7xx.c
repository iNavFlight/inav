/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#include "drivers/time.h"
#include "drivers/io.h"
#include "rcc.h"
#include "drivers/nvic.h"

#include "serial.h"
#include "serial_uart.h"
#include "serial_uart_impl.h"

#define UART_RX_BUFFER_SIZE UART1_RX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE UART1_TX_BUFFER_SIZE

typedef struct uartDevice_s {
    USART_TypeDef* dev;
    uartPort_t port;
    ioTag_t rx;
    ioTag_t tx;
    volatile uint8_t rxBuffer[UART_RX_BUFFER_SIZE];
    volatile uint8_t txBuffer[UART_TX_BUFFER_SIZE];
    rccPeriphTag_t rcc;
    uint8_t af_rx;
    uint8_t af_tx;
    uint8_t irq;
    bool pinSwap;
} uartDevice_t;

#define UART_PIN_AF_HELPER(uart, pin)  CONCAT4(UART_PIN_AF_UART, uart, _, pin)

#ifdef USE_UART1
#define UART_PIN_AF_UART1_PA9       GPIO_AF7_USART1
#define UART_PIN_AF_UART1_PA10      GPIO_AF7_USART1
#define UART_PIN_AF_UART1_PB6       GPIO_AF7_USART1
#define UART_PIN_AF_UART1_PB7       GPIO_AF7_USART1
#define UART_PIN_AF_UART1_PB14      GPIO_AF4_USART1
#define UART_PIN_AF_UART1_PB15      GPIO_AF4_USART1

static uartDevice_t uart1 =
{
    .dev = USART1,
    .rx = IO_TAG(UART1_RX_PIN),
    .tx = IO_TAG(UART1_TX_PIN),
    .af_rx = UART_PIN_AF_HELPER(1, UART1_RX_PIN),
    .af_tx = UART_PIN_AF_HELPER(1, UART1_TX_PIN),
    .rcc = RCC_APB2(USART1),
    .irq = USART1_IRQn,
#ifdef USE_UART1_PIN_SWAP
    .pinSwap = true,
#endif
};
#endif

#ifdef USE_UART2
static uartDevice_t uart2 =
{
    .dev = USART2,
    .rx = IO_TAG(UART2_RX_PIN),
    .tx = IO_TAG(UART2_TX_PIN),
    .af_rx = GPIO_AF7_USART2,
    .af_tx = GPIO_AF7_USART2,
    .rcc = RCC_APB1L(USART2),
    .irq = USART2_IRQn,
#ifdef USE_UART2_PIN_SWAP
    .pinSwap = true,
#endif
};
#endif

#ifdef USE_UART3
static uartDevice_t uart3 =
{
    .dev = USART3,
    .rx = IO_TAG(UART3_RX_PIN),
    .tx = IO_TAG(UART3_TX_PIN),
    .af_rx = GPIO_AF7_USART3,
    .af_tx = GPIO_AF7_USART3,
    .rcc = RCC_APB1L(USART3),
    .irq = USART3_IRQn,
#ifdef USE_UART3_PIN_SWAP
    .pinSwap = true,
#endif
};
#endif

#ifdef USE_UART4
#define UART_PIN_AF_UART4_PA0       GPIO_AF8_UART4
#define UART_PIN_AF_UART4_PA1       GPIO_AF8_UART4
#define UART_PIN_AF_UART4_PA11      GPIO_AF6_UART4
#define UART_PIN_AF_UART4_PA12      GPIO_AF6_UART4
#define UART_PIN_AF_UART4_PB8       GPIO_AF8_UART4
#define UART_PIN_AF_UART4_PB9       GPIO_AF8_UART4
#define UART_PIN_AF_UART4_PC10      GPIO_AF8_UART4
#define UART_PIN_AF_UART4_PC11      GPIO_AF8_UART4
#define UART_PIN_AF_UART4_PD0       GPIO_AF8_UART4
#define UART_PIN_AF_UART4_PD1       GPIO_AF8_UART4

static uartDevice_t uart4 =
{
    .dev = UART4,
    .rx = IO_TAG(UART4_RX_PIN),
    .tx = IO_TAG(UART4_TX_PIN),
    .af_rx = UART_PIN_AF_HELPER(4, UART4_RX_PIN),
    .af_tx = UART_PIN_AF_HELPER(4, UART4_TX_PIN),
    .rcc = RCC_APB1L(UART4),
    .irq = UART4_IRQn,
#ifdef USE_UART4_PIN_SWAP
    .pinSwap = true,
#endif
};
#endif

#ifdef USE_UART5
#define UART_PIN_AF_UART5_PB5       GPIO_AF14_UART5
#define UART_PIN_AF_UART5_PB6       GPIO_AF14_UART5
#define UART_PIN_AF_UART5_PB12      GPIO_AF14_UART5
#define UART_PIN_AF_UART5_PB13      GPIO_AF14_UART5
#define UART_PIN_AF_UART5_PD2       GPIO_AF8_UART5
#define UART_PIN_AF_UART5_PC12      GPIO_AF8_UART5

static uartDevice_t uart5 =
{
    .dev = UART5,
    .rx = IO_TAG(UART5_RX_PIN),
    .tx = IO_TAG(UART5_TX_PIN),
    .af_rx = UART_PIN_AF_HELPER(5, UART5_RX_PIN),
    .af_tx = UART_PIN_AF_HELPER(5, UART5_TX_PIN),
    .rcc = RCC_APB1L(UART5),
    .irq = UART5_IRQn,
#ifdef USE_UART5_PIN_SWAP
    .pinSwap = true,
#endif
};
#endif

#ifdef USE_UART6
static uartDevice_t uart6 =
{
    .dev = USART6,
    .rx = IO_TAG(UART6_RX_PIN),
    .tx = IO_TAG(UART6_TX_PIN),
    .af_rx = GPIO_AF7_USART6,
    .af_tx = GPIO_AF7_USART6,
    .rcc = RCC_APB2(USART6),
    .irq = USART6_IRQn,
#ifdef USE_UART6_PIN_SWAP
    .pinSwap = true,
#endif
};
#endif

#ifdef USE_UART7
#define UART_PIN_AF_UART7_PA8       GPIO_AF11_UART7
#define UART_PIN_AF_UART7_PA15      GPIO_AF11_UART7
#define UART_PIN_AF_UART7_PB3       GPIO_AF11_UART7
#define UART_PIN_AF_UART7_PB4       GPIO_AF11_UART7
#define UART_PIN_AF_UART7_PE7       GPIO_AF7_UART7
#define UART_PIN_AF_UART7_PE8       GPIO_AF7_UART7
#define UART_PIN_AF_UART7_PF6       GPIO_AF7_UART7
#define UART_PIN_AF_UART7_PF7       GPIO_AF7_UART7

static uartDevice_t uart7 =
{
    .dev = UART7,
    .rx = IO_TAG(UART7_RX_PIN),
#ifdef UART7_TX_PIN
    .tx = IO_TAG(UART7_TX_PIN),
#endif
    .af_rx = UART_PIN_AF_HELPER(7, UART7_RX_PIN),
#ifdef UART7_TX_PIN
    .af_tx = UART_PIN_AF_HELPER(7, UART7_TX_PIN),
#endif
    .rcc = RCC_APB1L(UART7),
    .irq = UART7_IRQn,
#ifdef USE_UART7_PIN_SWAP
    .pinSwap = true,
#endif
};
#endif

#ifdef USE_UART8
static uartDevice_t uart8 =
{
    .dev = UART8,
    .rx = IO_TAG(UART8_RX_PIN),
    .tx = IO_TAG(UART8_TX_PIN),
    .af_rx = GPIO_AF8_UART8,
    .af_tx = GPIO_AF8_UART8,
    .rcc = RCC_APB1L(UART8),
    .irq = UART8_IRQn,
#ifdef USE_UART8_PIN_SWAP
    .pinSwap = true,
#endif
};
#endif



static uartDevice_t* uartHardwareMap[] = {
#ifdef USE_UART1
    &uart1,
#else
    NULL,
#endif
#ifdef USE_UART2
    &uart2,
#else
    NULL,
#endif
#ifdef USE_UART3
    &uart3,
#else
    NULL,
#endif
#ifdef USE_UART4
    &uart4,
#else
    NULL,
#endif
#ifdef USE_UART5
    &uart5,
#else
    NULL,
#endif
#ifdef USE_UART6
    &uart6,
#else
    NULL,
#endif
#ifdef USE_UART7
    &uart7,
#else
    NULL,
#endif
#ifdef USE_UART8
    &uart8,
#else
    NULL,
#endif
};

static uartDevice_t *uartFindDevice(uartPort_t *uartPort)
{
    for (unsigned i = 0; i < UARTDEV_MAX; i++) {
        uartDevice_t *pDevice = uartHardwareMap[i];
        if (pDevice && pDevice->dev == uartPort->USARTx) {
            return pDevice;
        }
    }
    return NULL;
}

void uartConfigurePinSwap(uartPort_t *uartPort)
{
    uartDevice_t *uartDevice = uartFindDevice(uartPort);
    if (!uartDevice) {
        return;
    }

    if (uartDevice->pinSwap) {
        uartPort->Handle.AdvancedInit.AdvFeatureInit |= UART_ADVFEATURE_SWAP_INIT;
        uartPort->Handle.AdvancedInit.Swap = UART_ADVFEATURE_SWAP_ENABLE;
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

#if defined(USE_UART_RX_DMA) || defined(USE_UART_TX_DMA)
static UARTDevice_e uartDeviceOf(const uartPort_t *s)
{
    for (int device = 0; device < UARTDEV_MAX; device++) {
        if (uartHardwareMap[device] && &uartHardwareMap[device]->port == s) {
            return device;
        }
    }
    return UARTDEV_MAX;
}

#endif

#ifdef USE_UART_RX_DMA
// DMA rings go in D2 SRAM, which the MPU marks shareable and so uncached: in AXI SRAM the CPU
// would read the cache rather than what the stream wrote
#ifdef UART1_RX_DMA
static DMA_RAM uint8_t uart1RxDmaBuffer[UART_RX_BUFFER_SIZE];
#endif
#ifdef UART2_RX_DMA
static DMA_RAM uint8_t uart2RxDmaBuffer[UART_RX_BUFFER_SIZE];
#endif
#ifdef UART3_RX_DMA
static DMA_RAM uint8_t uart3RxDmaBuffer[UART_RX_BUFFER_SIZE];
#endif
#ifdef UART4_RX_DMA
static DMA_RAM uint8_t uart4RxDmaBuffer[UART_RX_BUFFER_SIZE];
#endif
#ifdef UART5_RX_DMA
static DMA_RAM uint8_t uart5RxDmaBuffer[UART_RX_BUFFER_SIZE];
#endif
#ifdef UART6_RX_DMA
static DMA_RAM uint8_t uart6RxDmaBuffer[UART_RX_BUFFER_SIZE];
#endif
#ifdef UART7_RX_DMA
static DMA_RAM uint8_t uart7RxDmaBuffer[UART_RX_BUFFER_SIZE];
#endif
#ifdef UART8_RX_DMA
static DMA_RAM uint8_t uart8RxDmaBuffer[UART_RX_BUFFER_SIZE];
#endif

typedef struct {
    dmaTag_t tag;
    uint32_t request;               // this UART's receiver on the DMAMUX
    volatile uint8_t *buffer;
} uartRxDmaConfig_t;

static const uartRxDmaConfig_t uartRxDmaConfig[UARTDEV_MAX] = {
#ifdef UART1_RX_DMA
    [UARTDEV_1] = { UART1_RX_DMA, LL_DMAMUX1_REQ_USART1_RX, uart1RxDmaBuffer },
#endif
#ifdef UART2_RX_DMA
    [UARTDEV_2] = { UART2_RX_DMA, LL_DMAMUX1_REQ_USART2_RX, uart2RxDmaBuffer },
#endif
#ifdef UART3_RX_DMA
    [UARTDEV_3] = { UART3_RX_DMA, LL_DMAMUX1_REQ_USART3_RX, uart3RxDmaBuffer },
#endif
#ifdef UART4_RX_DMA
    [UARTDEV_4] = { UART4_RX_DMA, LL_DMAMUX1_REQ_UART4_RX, uart4RxDmaBuffer },
#endif
#ifdef UART5_RX_DMA
    [UARTDEV_5] = { UART5_RX_DMA, LL_DMAMUX1_REQ_UART5_RX, uart5RxDmaBuffer },
#endif
#ifdef UART6_RX_DMA
    [UARTDEV_6] = { UART6_RX_DMA, LL_DMAMUX1_REQ_USART6_RX, uart6RxDmaBuffer },
#endif
#ifdef UART7_RX_DMA
    [UARTDEV_7] = { UART7_RX_DMA, LL_DMAMUX1_REQ_UART7_RX, uart7RxDmaBuffer },
#endif
#ifdef UART8_RX_DMA
    [UARTDEV_8] = { UART8_RX_DMA, LL_DMAMUX1_REQ_UART8_RX, uart8RxDmaBuffer },
#endif
};

static void uartRxDmaStop(uartPort_t *s)
{
    if (s->rxDma) {
        const uint32_t stream = DMATAG_GET_STREAM(s->rxDma->tag);  // LL_DMA_STREAM_n is n
        CLEAR_BIT(s->USARTx->CR3, USART_CR3_DMAR);
        LL_DMA_DisableStream(s->rxDma->dma, stream);
        while (LL_DMA_IsEnabledStream(s->rxDma->dma, stream));
        s->rxDma = NULL;
    }
}

// USART_CR3_DMAR is set again in uartReconfigure(): the HAL clears CR3 whenever it reprograms the port
bool uartRxDmaStart(uartPort_t *s)
{
    uartRxDmaStop(s);

    const UARTDevice_e device = uartDeviceOf(s);
    if (device == UARTDEV_MAX || uartRxDmaConfig[device].tag == DMA_NONE || !(s->port.mode & MODE_RX) || s->port.rxCallback) {
        return false;
    }

    const DMA_t dma = dmaGetByTag(uartRxDmaConfig[device].tag);
    if (!dma || !uartDmaStreamAvailable(dma, device)) {
        return false;
    }

    dmaInit(dma, OWNER_SERIAL, RESOURCE_INDEX(device));

    const uint32_t stream = DMATAG_GET_STREAM(uartRxDmaConfig[device].tag);
    LL_DMA_DeInit(dma->dma, stream);

    LL_DMA_InitTypeDef init;
    LL_DMA_StructInit(&init);
    init.PeriphRequest = uartRxDmaConfig[device].request;
    init.PeriphOrM2MSrcAddress = (uint32_t)&s->USARTx->RDR;
    init.MemoryOrM2MDstAddress = (uint32_t)s->port.rxBuffer;
    init.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
    init.Mode = LL_DMA_MODE_CIRCULAR;
    init.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
    init.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
    init.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
    init.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
    init.NbData = s->port.rxBufferSize;
    init.Priority = LL_DMA_PRIORITY_MEDIUM;
    // No FIFO: a byte is in the ring as soon as the count says so
    init.FIFOMode = LL_DMA_FIFOMODE_DISABLE;
    LL_DMA_Init(dma->dma, stream, &init);
    LL_DMA_EnableStream(dma->dma, stream);
    // Restarted on a new ring (serialSetRxBuffer) the port gets no reprogramming to set it
    SET_BIT(s->USARTx->CR3, USART_CR3_DMAR);

    s->port.rxBufferHead = s->port.rxBufferTail = 0;
    s->rxDma = dma;
    return true;
}
#endif

#ifdef USE_UART_TX_DMA
// TX rings go in uncached D2 SRAM too, so the stream reads what the CPU wrote
#ifdef UART1_TX_DMA
static DMA_RAM uint8_t uart1TxDmaBuffer[UART_TX_BUFFER_SIZE];
#endif
#ifdef UART2_TX_DMA
static DMA_RAM uint8_t uart2TxDmaBuffer[UART_TX_BUFFER_SIZE];
#endif
#ifdef UART3_TX_DMA
static DMA_RAM uint8_t uart3TxDmaBuffer[UART_TX_BUFFER_SIZE];
#endif
#ifdef UART4_TX_DMA
static DMA_RAM uint8_t uart4TxDmaBuffer[UART_TX_BUFFER_SIZE];
#endif
#ifdef UART5_TX_DMA
static DMA_RAM uint8_t uart5TxDmaBuffer[UART_TX_BUFFER_SIZE];
#endif
#ifdef UART6_TX_DMA
static DMA_RAM uint8_t uart6TxDmaBuffer[UART_TX_BUFFER_SIZE];
#endif
#ifdef UART7_TX_DMA
static DMA_RAM uint8_t uart7TxDmaBuffer[UART_TX_BUFFER_SIZE];
#endif
#ifdef UART8_TX_DMA
static DMA_RAM uint8_t uart8TxDmaBuffer[UART_TX_BUFFER_SIZE];
#endif

typedef struct {
    dmaTag_t tag;
    uint32_t request;               // this UART's transmitter on the DMAMUX
    volatile uint8_t *buffer;
} uartTxDmaConfig_t;

static const uartTxDmaConfig_t uartTxDmaConfig[UARTDEV_MAX] = {
#ifdef UART1_TX_DMA
    [UARTDEV_1] = { UART1_TX_DMA, LL_DMAMUX1_REQ_USART1_TX, uart1TxDmaBuffer },
#endif
#ifdef UART2_TX_DMA
    [UARTDEV_2] = { UART2_TX_DMA, LL_DMAMUX1_REQ_USART2_TX, uart2TxDmaBuffer },
#endif
#ifdef UART3_TX_DMA
    [UARTDEV_3] = { UART3_TX_DMA, LL_DMAMUX1_REQ_USART3_TX, uart3TxDmaBuffer },
#endif
#ifdef UART4_TX_DMA
    [UARTDEV_4] = { UART4_TX_DMA, LL_DMAMUX1_REQ_UART4_TX, uart4TxDmaBuffer },
#endif
#ifdef UART5_TX_DMA
    [UARTDEV_5] = { UART5_TX_DMA, LL_DMAMUX1_REQ_UART5_TX, uart5TxDmaBuffer },
#endif
#ifdef UART6_TX_DMA
    [UARTDEV_6] = { UART6_TX_DMA, LL_DMAMUX1_REQ_USART6_TX, uart6TxDmaBuffer },
#endif
#ifdef UART7_TX_DMA
    [UARTDEV_7] = { UART7_TX_DMA, LL_DMAMUX1_REQ_UART7_TX, uart7TxDmaBuffer },
#endif
#ifdef UART8_TX_DMA
    [UARTDEV_8] = { UART8_TX_DMA, LL_DMAMUX1_REQ_UART8_TX, uart8TxDmaBuffer },
#endif
};

#define UART_TX_DMA_FLAGS   (DMA_IT_TCIF | DMA_IT_HTIF | DMA_IT_TEIF | DMA_IT_DMEIF | DMA_IT_FEIF)
#define UART_TX_DMA_ENDED   (DMA_IT_TCIF | DMA_IT_TEIF)

void uartTxDmaStop(uartPort_t *s)
{
    if (s->txDma) {
        const uint32_t stream = DMATAG_GET_STREAM(s->txDma->tag);  // LL_DMA_STREAM_n is n
        CLEAR_BIT(s->USARTx->CR3, USART_CR3_DMAT);
        LL_DMA_DisableIT_TC(s->txDma->dma, stream);
        LL_DMA_DisableIT_TE(s->txDma->dma, stream);
        LL_DMA_DisableStream(s->txDma->dma, stream);
        while (LL_DMA_IsEnabledStream(s->txDma->dma, stream));
        DMA_CLEAR_FLAG(s->txDma, UART_TX_DMA_FLAGS);
        s->txDma = NULL;
        s->txDmaCount = 0;
    }
}

void uartStartTxDMA(uartPort_t *s)
{
    // Masked: the end-of-transfer interrupt also starts the next one
    ATOMIC_BLOCK(NVIC_PRIO_SERIALUART) {
        const uint32_t head = s->port.txBufferHead;
        const uint32_t tail = s->port.txBufferTail;

        if (s->txDma && !s->txDmaCount && head != tail) {
            // Up to the head, or to the end of the ring if the queue wraps: the rest follows
            const uint32_t count = (head > tail) ? head - tail : s->port.txBufferSize - tail;
            const uint32_t stream = DMATAG_GET_STREAM(s->txDma->tag);

            s->txDmaCount = count;
            LL_DMA_SetMemoryAddress(s->txDma->dma, stream, (uint32_t)&s->port.txBuffer[tail]);
            LL_DMA_SetDataLength(s->txDma->dma, stream, count);
            // The bytes are in the ring before the stream goes to read them
            __DSB();
            LL_DMA_EnableStream(s->txDma->dma, stream);
        }
    }
}

// An error ends the transfer too, or the port would wait for it forever
static void uartTxDmaHandler(DMA_t dma)
{
    uartPort_t *s = (uartPort_t *)dma->userParam;

    // Clear every flag: one left set re-raises the interrupt, starving USB at this priority
    const bool ended = DMA_GET_FLAG_STATUS(dma, UART_TX_DMA_ENDED);
    DMA_CLEAR_FLAG(dma, UART_TX_DMA_FLAGS);

    if (ended) {
        // After an error the rest goes again, unless nothing went: that error would only repeat
        const uint32_t sent = s->txDmaCount - LL_DMA_GetDataLength(dma->dma, DMATAG_GET_STREAM(dma->tag));
        s->port.txBufferTail = (s->port.txBufferTail + (sent ? sent : s->txDmaCount)) % s->port.txBufferSize;
        s->txDmaCount = 0;
        uartStartTxDMA(s);
    }
}

// USART_CR3_DMAT is set in uartReconfigure(), as DMAR is
bool uartTxDmaStart(uartPort_t *s)
{
    uartTxDmaStop(s);

    const UARTDevice_e device = uartDeviceOf(s);
    if (device == UARTDEV_MAX || uartTxDmaConfig[device].tag == DMA_NONE || !(s->port.mode & MODE_TX)) {
        return false;
    }

    const DMA_t dma = dmaGetByTag(uartTxDmaConfig[device].tag);
    if (!dma || !uartDmaStreamAvailable(dma, device)) {
        return false;
    }
#ifdef USE_UART_RX_DMA
    // The stream this port receives on is this UART's too, so it looks free to it
    if (dma == s->rxDma) {
        return false;
    }
#endif

    dmaInit(dma, OWNER_SERIAL, RESOURCE_INDEX(device));
    dmaSetHandler(dma, uartTxDmaHandler, NVIC_PRIO_SERIALUART, (uint32_t)s);

    const uint32_t stream = DMATAG_GET_STREAM(uartTxDmaConfig[device].tag);
    LL_DMA_DeInit(dma->dma, stream);

    LL_DMA_InitTypeDef init;
    LL_DMA_StructInit(&init);
    init.PeriphRequest = uartTxDmaConfig[device].request;
    init.PeriphOrM2MSrcAddress = (uint32_t)&s->USARTx->TDR;
    init.MemoryOrM2MDstAddress = (uint32_t)s->port.txBuffer;
    init.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    init.Mode = LL_DMA_MODE_NORMAL;
    init.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
    init.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
    init.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
    init.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
    init.Priority = LL_DMA_PRIORITY_MEDIUM;
    init.FIFOMode = LL_DMA_FIFOMODE_DISABLE;
    LL_DMA_Init(dma->dma, stream, &init);
    DMA_CLEAR_FLAG(dma, UART_TX_DMA_FLAGS);
    LL_DMA_EnableIT_TC(dma->dma, stream);
    LL_DMA_EnableIT_TE(dma->dma, stream);

    s->txDmaCount = 0;
    s->txDma = dma;
    return true;
}
#endif

void uartIrqHandler(uartPort_t *s)
{
    UART_HandleTypeDef *huart = &s->Handle;
    /* UART in mode Receiver ---------------------------------------------------*/
    // With RX on DMA the interrupt still fires for TX: an RX byte is the stream's to take
    if ((__HAL_UART_GET_IT(huart, UART_IT_RXNE) != RESET) && !uartRxDmaRunning(s)) {
        uint8_t rbyte = (uint8_t)(huart->Instance->RDR & (uint8_t) 0xff);

        if (s->port.rxCallback) {
            s->port.rxCallback(rbyte, s->port.rxCallbackData);
        } else {
            s->port.rxBuffer[s->port.rxBufferHead] = rbyte;
            s->port.rxBufferHead = (s->port.rxBufferHead + 1) % s->port.rxBufferSize;
        }
        CLEAR_BIT(huart->Instance->CR1, (USART_CR1_PEIE));

        /* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
        CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

        __HAL_UART_SEND_REQ(huart, UART_RXDATA_FLUSH_REQUEST);
    }

    /* UART parity error interrupt occurred -------------------------------------*/
    if ((__HAL_UART_GET_IT(huart, UART_IT_PE) != RESET)) {
        __HAL_UART_CLEAR_IT(huart, UART_CLEAR_PEF);
    }

    /* UART frame error interrupt occurred --------------------------------------*/
    if ((__HAL_UART_GET_IT(huart, UART_IT_FE) != RESET)) {
        __HAL_UART_CLEAR_IT(huart, UART_CLEAR_FEF);
    }

    /* UART noise error interrupt occurred --------------------------------------*/
    if ((__HAL_UART_GET_IT(huart, UART_IT_NE) != RESET)) {
        __HAL_UART_CLEAR_IT(huart, UART_CLEAR_NEF);
    }

    /* UART Over-Run interrupt occurred -----------------------------------------*/
    if ((__HAL_UART_GET_IT(huart, UART_IT_ORE) != RESET)) {
        __HAL_UART_CLEAR_IT(huart, UART_CLEAR_OREF);
    }

    /* UART in mode Transmitter ------------------------------------------------*/
    // With TX on DMA the data register is the stream's to feed
    if ((__HAL_UART_GET_IT(huart, UART_IT_TXE) != RESET) && !uartTxDmaRunning(s)) {
        /* Check that a Tx process is ongoing */
        if (huart->gState != HAL_UART_STATE_BUSY_TX) {
            if (s->port.txBufferTail == s->port.txBufferHead) {
                huart->TxXferCount = 0;
                /* Disable the UART Transmit Data Register Empty Interrupt */
                CLEAR_BIT(huart->Instance->CR1, USART_CR1_TXEIE);
            } else {
                if ((huart->Init.WordLength == UART_WORDLENGTH_9B) && (huart->Init.Parity == UART_PARITY_NONE)) {
                    huart->Instance->TDR = (((uint16_t) s->port.txBuffer[s->port.txBufferTail]) & (uint16_t) 0x01FFU);
                } else {
                    huart->Instance->TDR = (uint8_t)(s->port.txBuffer[s->port.txBufferTail]);
                }
                s->port.txBufferTail = (s->port.txBufferTail + 1) % s->port.txBufferSize;
            }
        }
    }

    /* UART in mode Transmitter (transmission end) -----------------------------*/
    // TCIE is never enabled here, so TC is only a stale status flag left over from
    // the last transmitted byte. It used to be forwarded to HAL_UART_IRQHandler(),
    // which treats a pending overrun as a blocking error and permanently disables
    // the RX interrupts (UART_EndRxTransfer) - a receiver on a busy UART then dies
    // after any long IRQ-disabled window such as an internal flash write. Nothing
    // in the HAL handler is needed for INAV's own TXE/RXNE-driven transfers, so
    // just acknowledge the flag.
    if ((__HAL_UART_GET_IT(huart, UART_IT_TC) != RESET)) {
        __HAL_UART_CLEAR_IT(huart, UART_CLEAR_TCF);
    }
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
#ifdef USE_UART_RX_DMA
    if (uartRxDmaConfig[device].buffer) {
        s->port.rxBuffer = uartRxDmaConfig[device].buffer;
    }
#endif
    s->port.txBuffer = uart->txBuffer;
#ifdef USE_UART_TX_DMA
    if (uartTxDmaConfig[device].buffer) {
        s->port.txBuffer = uartTxDmaConfig[device].buffer;
    }
#endif
    s->port.rxBufferSize = sizeof(uart->rxBuffer);
    s->port.txBufferSize = sizeof(uart->txBuffer);

    s->USARTx = uart->dev;

    s->Handle.Instance = uart->dev;

    if (uart->rcc) {
        RCC_ClockCmd(uart->rcc, ENABLE);
    }

    IO_t tx = IOGetByTag(uart->tx);
    IO_t rx = IOGetByTag(uart->rx);

    if (options & SERIAL_BIDIR) {
        IOInit(tx, OWNER_SERIAL, RESOURCE_UART_TXRX, RESOURCE_INDEX(device));
        IOConfigGPIOAF(tx, IOCFG_AF_PP, uart->af_tx);
    }
    else {
        if (mode & MODE_TX) {
            IOInit(tx, OWNER_SERIAL, RESOURCE_UART_TX, RESOURCE_INDEX(device));
            IOConfigGPIOAF(tx, IOCFG_AF_PP, uart->af_tx);
        }

        if (mode & MODE_RX) {
            IOInit(rx, OWNER_SERIAL, RESOURCE_UART_RX, RESOURCE_INDEX(device));
            IOConfigGPIOAF(rx, IOCFG_AF_PP, uart->af_rx);
        }
    }

    HAL_NVIC_SetPriority(uart->irq, NVIC_PRIO_SERIALUART, 0);
    HAL_NVIC_EnableIRQ(uart->irq);

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
uartPort_t *serialUART2(uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    return serialUART(UARTDEV_2, baudRate, mode, options);
}

// USART2 Rx/Tx IRQ Handler
void USART2_IRQHandler(void)
{
    uartPort_t *s = &(uartHardwareMap[UARTDEV_2]->port);
    uartIrqHandler(s);
}
#endif

#ifdef USE_UART3
uartPort_t *serialUART3(uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    return serialUART(UARTDEV_3, baudRate, mode, options);
}

// USART3 Rx/Tx IRQ Handler
void USART3_IRQHandler(void)
{
    uartPort_t *s = &(uartHardwareMap[UARTDEV_3]->port);
    uartIrqHandler(s);
}
#endif

#ifdef USE_UART4
uartPort_t *serialUART4(uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    return serialUART(UARTDEV_4, baudRate, mode, options);
}

// UART4 Rx/Tx IRQ Handler
void UART4_IRQHandler(void)
{
    uartPort_t *s = &(uartHardwareMap[UARTDEV_4]->port);
    uartIrqHandler(s);
}
#endif

#ifdef USE_UART5
uartPort_t *serialUART5(uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    return serialUART(UARTDEV_5, baudRate, mode, options);
}

// UART5 Rx/Tx IRQ Handler
void UART5_IRQHandler(void)
{
    uartPort_t *s = &(uartHardwareMap[UARTDEV_5]->port);
    uartIrqHandler(s);
}
#endif

#ifdef USE_UART6
uartPort_t *serialUART6(uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    return serialUART(UARTDEV_6, baudRate, mode, options);
}

// USART6 Rx/Tx IRQ Handler
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
