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
#include "rcc.h"
#include "drivers/nvic.h"

#include "serial.h"
#include "serial_uart.h"
#include "serial_uart_impl.h"

#define UART_RX_BUFFER_SIZE UART1_RX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE UART1_RX_BUFFER_SIZE

typedef struct uartDevice_s {
    USART_TypeDef* dev;
    uartPort_t port;
    ioTag_t rx;
    ioTag_t tx;
    volatile uint8_t rxBuffer[UART_RX_BUFFER_SIZE];
    volatile uint8_t txBuffer[UART_TX_BUFFER_SIZE];
    uint32_t rcc_ahb1;
    rccPeriphTag_t rcc_apb2;
    rccPeriphTag_t rcc_apb1;
    uint8_t af;
    uint8_t irq;
    uint32_t irqPriority;
} uartDevice_t;

//static uartPort_t uartPort[MAX_UARTS];
#ifdef USE_UART1
static uartDevice_t uart1 =
{
    .dev = USART1,
    .rx = IO_TAG(UART1_RX_PIN),
    .tx = IO_TAG(UART1_TX_PIN),
    .af = GPIO_AF_USART1,
#ifdef UART1_AHB1_PERIPHERALS
    .rcc_ahb1 = UART1_AHB1_PERIPHERALS,
#endif
    .rcc_apb2 = RCC_APB2(USART1),
    .irq = USART1_IRQn,
    .irqPriority = NVIC_PRIO_SERIALUART
};
#endif

#ifdef USE_UART2
static uartDevice_t uart2 =
{
    .dev = USART2,
    .rx = IO_TAG(UART2_RX_PIN),
    .tx = IO_TAG(UART2_TX_PIN),
    .af = GPIO_AF_USART2,
#ifdef UART2_AHB1_PERIPHERALS
    .rcc_ahb1 = UART2_AHB1_PERIPHERALS,
#endif
    .rcc_apb1 = RCC_APB1(USART2),
    .irq = USART2_IRQn,
    .irqPriority = NVIC_PRIO_SERIALUART
};
#endif

#ifdef USE_UART3
static uartDevice_t uart3 =
{
    .dev = USART3,
    .rx = IO_TAG(UART3_RX_PIN),
    .tx = IO_TAG(UART3_TX_PIN),
    .af = GPIO_AF_USART3,
#ifdef UART3_AHB1_PERIPHERALS
    .rcc_ahb1 = UART3_AHB1_PERIPHERALS,
#endif
    .rcc_apb1 = RCC_APB1(USART3),
    .irq = USART3_IRQn,
    .irqPriority = NVIC_PRIO_SERIALUART
};
#endif

#ifdef USE_UART4
static uartDevice_t uart4 =
{
    .dev = UART4,
    .rx = IO_TAG(UART4_RX_PIN),
    .tx = IO_TAG(UART4_TX_PIN),
    .af = GPIO_AF_UART4,
#ifdef UART4_AHB1_PERIPHERALS
    .rcc_ahb1 = UART4_AHB1_PERIPHERALS,
#endif
    .rcc_apb1 = RCC_APB1(UART4),
    .irq = UART4_IRQn,
    .irqPriority = NVIC_PRIO_SERIALUART
};
#endif

#ifdef USE_UART5
static uartDevice_t uart5 =
{
    .dev = UART5,
    .rx = IO_TAG(UART5_RX_PIN),
    .tx = IO_TAG(UART5_TX_PIN),
    .af = GPIO_AF_UART5,
#ifdef UART5_AHB1_PERIPHERALS
    .rcc_ahb1 = UART5_AHB1_PERIPHERALS,
#endif
    .rcc_apb1 = RCC_APB1(UART5),
    .irq = UART5_IRQn,
    .irqPriority = NVIC_PRIO_SERIALUART
};
#endif

#ifdef USE_UART6
static uartDevice_t uart6 =
{
    .dev = USART6,
    .rx = IO_TAG(UART6_RX_PIN),
    .tx = IO_TAG(UART6_TX_PIN),
    .af = GPIO_AF_USART6,
#ifdef UART6_AHB1_PERIPHERALS
    .rcc_ahb1 = UART6_AHB1_PERIPHERALS,
#endif
    .rcc_apb2 = RCC_APB2(USART6),
    .irq = USART6_IRQn,
    .irqPriority = NVIC_PRIO_SERIALUART
};
#endif

#ifdef USE_UART7
static uartDevice_t uart7 =
{
    .dev = UART7,
    .rx = IO_TAG(UART7_RX_PIN),
    .tx = IO_TAG(UART7_TX_PIN),
    .af = GPIO_AF_UART7,
    .rcc_apb1 = RCC_APB1(UART7),
    .irq = UART7_IRQn,
    .irqPriority = NVIC_PRIO_SERIALUART
};
#endif

#ifdef USE_UART8
static uartDevice_t uart8 =
{
    .dev = UART8,
    .rx = IO_TAG(UART8_RX_PIN),
    .tx = IO_TAG(UART8_TX_PIN),
    .af = GPIO_AF_UART8,
    .rcc_apb1 = RCC_APB1(UART8),
    .irq = UART8_IRQn,
    .irqPriority = NVIC_PRIO_SERIALUART
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
static const dmaTag_t uartRxDmaTag[UARTDEV_MAX] = {
#ifdef UART1_RX_DMA
    [UARTDEV_1] = UART1_RX_DMA,
#endif
#ifdef UART2_RX_DMA
    [UARTDEV_2] = UART2_RX_DMA,
#endif
#ifdef UART3_RX_DMA
    [UARTDEV_3] = UART3_RX_DMA,
#endif
#ifdef UART4_RX_DMA
    [UARTDEV_4] = UART4_RX_DMA,
#endif
#ifdef UART5_RX_DMA
    [UARTDEV_5] = UART5_RX_DMA,
#endif
#ifdef UART6_RX_DMA
    [UARTDEV_6] = UART6_RX_DMA,
#endif
#ifdef UART7_RX_DMA
    [UARTDEV_7] = UART7_RX_DMA,
#endif
#ifdef UART8_RX_DMA
    [UARTDEV_8] = UART8_RX_DMA,
#endif
};

void uartRxDmaStop(uartPort_t *s)
{
    if (s->rxDma) {
        USART_DMACmd(s->USARTx, USART_DMAReq_Rx, DISABLE);
        DMA_Cmd(s->rxDma->ref, DISABLE);
        while (DMA_GetCmdStatus(s->rxDma->ref) != DISABLE);
        uartDmaRelease(s->rxDma);
        s->rxDma = NULL;
    }
}

bool uartRxDmaStart(uartPort_t *s)
{
    uartRxDmaStop(s);

    const UARTDevice_e device = uartDeviceOf(s);
    if (device == UARTDEV_MAX || !(s->port.mode & MODE_RX) || s->port.rxCallback) {
        return false;
    }

    const dmaTag_t tag = uartDmaPick(uartRxDmaTag[device], uartRxDmaCandidates, device, &s->port, uartTxDmaOf(s));
    const DMA_t dma = dmaGetByTag(tag);
    if (!dma) {
        return false;
    }

    dmaInit(dma, OWNER_SERIAL, RESOURCE_INDEX(device));
    DMA_DeInit(dma->ref);

    DMA_InitTypeDef init;
    DMA_StructInit(&init);
    init.DMA_Channel = DMATAG_GET_CHANNEL(tag) << 25;    // DMA_Channel_n is n in CHSEL
    init.DMA_PeripheralBaseAddr = (uint32_t)&s->USARTx->DR;
    init.DMA_Memory0BaseAddr = (uint32_t)s->port.rxBuffer;
    init.DMA_DIR = DMA_DIR_PeripheralToMemory;
    init.DMA_BufferSize = s->port.rxBufferSize;
    init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    init.DMA_MemoryInc = DMA_MemoryInc_Enable;
    init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    init.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    init.DMA_Mode = DMA_Mode_Circular;
    init.DMA_Priority = DMA_Priority_Medium;
    // No FIFO: a byte is in the ring as soon as the count says so
    init.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_Init(dma->ref, &init);
    DMA_Cmd(dma->ref, ENABLE);

    s->port.rxBufferHead = s->port.rxBufferTail = 0;
    s->rxDma = dma;

    USART_ITConfig(s->USARTx, USART_IT_RXNE, DISABLE);
    USART_DMACmd(s->USARTx, USART_DMAReq_Rx, ENABLE);
    return true;
}
#endif

#ifdef USE_UART_TX_DMA
static const dmaTag_t uartTxDmaTag[UARTDEV_MAX] = {
#ifdef UART1_TX_DMA
    [UARTDEV_1] = UART1_TX_DMA,
#endif
#ifdef UART2_TX_DMA
    [UARTDEV_2] = UART2_TX_DMA,
#endif
#ifdef UART3_TX_DMA
    [UARTDEV_3] = UART3_TX_DMA,
#endif
#ifdef UART4_TX_DMA
    [UARTDEV_4] = UART4_TX_DMA,
#endif
#ifdef UART5_TX_DMA
    [UARTDEV_5] = UART5_TX_DMA,
#endif
#ifdef UART6_TX_DMA
    [UARTDEV_6] = UART6_TX_DMA,
#endif
#ifdef UART7_TX_DMA
    [UARTDEV_7] = UART7_TX_DMA,
#endif
#ifdef UART8_TX_DMA
    [UARTDEV_8] = UART8_TX_DMA,
#endif
};

#define UART_TX_DMA_FLAGS   (DMA_IT_TCIF | DMA_IT_HTIF | DMA_IT_TEIF | DMA_IT_DMEIF | DMA_IT_FEIF)
#define UART_TX_DMA_ENDED   (DMA_IT_TCIF | DMA_IT_TEIF)

void uartTxDmaStop(uartPort_t *s)
{
    if (s->txDma) {
        USART_DMACmd(s->USARTx, USART_DMAReq_Tx, DISABLE);
        DMA_ITConfig(s->txDma->ref, DMA_IT_TC | DMA_IT_TE, DISABLE);
        DMA_Cmd(s->txDma->ref, DISABLE);
        while (DMA_GetCmdStatus(s->txDma->ref) != DISABLE);
        DMA_CLEAR_FLAG(s->txDma, UART_TX_DMA_FLAGS);
        uartDmaRelease(s->txDma);
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

            s->txDmaCount = count;
            s->txDma->ref->M0AR = (uint32_t)&s->port.txBuffer[tail];
            DMA_SetCurrDataCounter(s->txDma->ref, count);
            DMA_Cmd(s->txDma->ref, ENABLE);
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
        const uint32_t sent = s->txDmaCount - DMA_GetCurrDataCounter(dma->ref);
        s->port.txBufferTail = (s->port.txBufferTail + (sent ? sent : s->txDmaCount)) % s->port.txBufferSize;
        s->txDmaCount = 0;
        uartStartTxDMA(s);
    }
}

bool uartTxDmaStart(uartPort_t *s)
{
    uartTxDmaStop(s);

    const UARTDevice_e device = uartDeviceOf(s);
    if (device == UARTDEV_MAX || !(s->port.mode & MODE_TX)) {
        return false;
    }

    const dmaTag_t tag = uartDmaPick(uartTxDmaTag[device], uartTxDmaCandidates, device, &s->port, uartRxDmaOf(s));
    const DMA_t dma = dmaGetByTag(tag);
    if (!dma) {
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
    DMA_DeInit(dma->ref);

    DMA_InitTypeDef init;
    DMA_StructInit(&init);
    init.DMA_Channel = DMATAG_GET_CHANNEL(tag) << 25;    // DMA_Channel_n is n in CHSEL
    init.DMA_PeripheralBaseAddr = (uint32_t)&s->USARTx->DR;
    init.DMA_Memory0BaseAddr = (uint32_t)s->port.txBuffer;
    init.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    init.DMA_MemoryInc = DMA_MemoryInc_Enable;
    init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    init.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    init.DMA_Mode = DMA_Mode_Normal;
    init.DMA_Priority = DMA_Priority_Medium;
    init.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_Init(dma->ref, &init);
    DMA_CLEAR_FLAG(dma, UART_TX_DMA_FLAGS);
    DMA_ITConfig(dma->ref, DMA_IT_TC | DMA_IT_TE, ENABLE);

    s->txDmaCount = 0;
    s->txDma = dma;

    USART_ITConfig(s->USARTx, USART_IT_TXE, DISABLE);
    USART_DMACmd(s->USARTx, USART_DMAReq_Tx, ENABLE);
    return true;
}
#endif

void uartIrqHandler(uartPort_t *s)
{
    if (USART_GetITStatus(s->USARTx, USART_IT_RXNE) == SET) {
        if (s->port.rxCallback) {
            s->port.rxCallback(s->USARTx->DR, s->port.rxCallbackData);
        } else {
            s->port.rxBuffer[s->port.rxBufferHead] = s->USARTx->DR;
            s->port.rxBufferHead = (s->port.rxBufferHead + 1) % s->port.rxBufferSize;
        }
    }

    if (USART_GetITStatus(s->USARTx, USART_IT_TXE) == SET) {
        if (s->port.txBufferTail != s->port.txBufferHead) {
            USART_SendData(s->USARTx, s->port.txBuffer[s->port.txBufferTail]);
            s->port.txBufferTail = (s->port.txBufferTail + 1) % s->port.txBufferSize;
        } else {
            USART_ITConfig(s->USARTx, USART_IT_TXE, DISABLE);
        }
    }

    if (USART_GetITStatus(s->USARTx, USART_FLAG_ORE) == SET)
    {
        USART_ClearITPendingBit (s->USARTx, USART_IT_ORE);
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
    (void) s->USARTx->SR;
    (void) s->USARTx->DR;
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
        RCC_AHB1PeriphClockCmd(uart->rcc_ahb1, ENABLE);

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

    NVIC_SetPriority(uart->irq, uart->irqPriority);
    NVIC_EnableIRQ(uart->irq);

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
// USART2 - GPS or Spektrum or ?? (RX + TX by IRQ)
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
