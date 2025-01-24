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
    usart_type* dev;
    uartPort_t port;
    ioTag_t rx;
    uint8_t rx_af;
    ioTag_t tx;
    uint8_t tx_af;
    volatile uint8_t rxBuffer[UART_RX_BUFFER_SIZE];
    volatile uint8_t txBuffer[UART_TX_BUFFER_SIZE];
    uint32_t rcc_ahb1;
    rccPeriphTag_t rcc_apb2;
    rccPeriphTag_t rcc_apb1;
    uint8_t irq;
    uint32_t irqPriority;
    bool pinSwap;
} uartDevice_t;

#ifdef USE_UART1
static uartDevice_t uart1 = {
    .dev = USART1,
    .rx = IO_TAG(UART1_RX_PIN),
    .tx = IO_TAG(UART1_TX_PIN),
#if defined(UART1_RX_AF)
    .rx_af = CONCAT(GPIO_MUX_, UART1_RX_AF),
#elif defined(UART1_AF)
    .rx_af = CONCAT(GPIO_MUX_, UART1_AF),
#else
    .rx_af = GPIO_MUX_7,
#endif
#if defined(UART1_TX_AF)
    .tx_af = CONCAT(GPIO_MUX_, UART1_TX_AF),
#elif defined(UART1_AF)
    .tx_af = CONCAT(GPIO_MUX_, UART1_AF),
#else
    .tx_af = GPIO_MUX_7,
#endif
#ifdef UART1_AHB1_PERIPHERALS
    .rcc_ahb1 = UART1_AHB1_PERIPHERALS,
#endif
    .rcc_apb2 = RCC_APB2(USART1),
    .irq = USART1_IRQn,
    .irqPriority = NVIC_PRIO_SERIALUART,
#ifdef USE_UART1_PIN_SWAP
    .pinSwap = true,
#else
    .pinSwap = false,
#endif
};
#endif

#ifdef USE_UART2
static uartDevice_t uart2 =
{
    .dev = USART2,
    .rx = IO_TAG(UART2_RX_PIN),
    .tx = IO_TAG(UART2_TX_PIN),
#if defined(UART2_RX_AF)
    .rx_af = CONCAT(GPIO_MUX_, UART2_RX_AF),
#elif defined(UART2_AF)
    .rx_af = CONCAT(GPIO_MUX_, UART2_AF),
#else
    .rx_af = GPIO_MUX_7,
#endif
#if defined(UART2_TX_AF)
    .tx_af = CONCAT(GPIO_MUX_, UART2_TX_AF),
#elif defined(UART2_AF)
    .tx_af = CONCAT(GPIO_MUX_, UART2_AF),
#else
    .tx_af = GPIO_MUX_7,
#endif
#ifdef UART2_AHB1_PERIPHERALS
    .rcc_ahb1 = UART2_AHB1_PERIPHERALS,
#endif
    .rcc_apb1 = RCC_APB1(USART2),
    .irq = USART2_IRQn,
    .irqPriority = NVIC_PRIO_SERIALUART,
#ifdef USE_UART2_PIN_SWAP
    .pinSwap = true,
#else
    .pinSwap = false,
#endif
};
#endif

#ifdef USE_UART3
static uartDevice_t uart3 =
{
    .dev = USART3,
    .rx = IO_TAG(UART3_RX_PIN),
    .tx = IO_TAG(UART3_TX_PIN),
#if defined(UART3_RX_AF)
    .rx_af = CONCAT(GPIO_MUX_, UART3_RX_AF),
#elif defined(UART3_AF)
    .rx_af = CONCAT(GPIO_MUX_, UART3_AF),
#else
    .rx_af = GPIO_MUX_7,
#endif
#if defined(UART3_TX_AF)
    .tx_af = CONCAT(GPIO_MUX_, UART3_TX_AF),
#elif defined(UART3_AF)
    .tx_af = CONCAT(GPIO_MUX_, UART3_AF),
#else
    .tx_af = GPIO_MUX_7,
#endif
#ifdef UART3_AHB1_PERIPHERALS
    .rcc_ahb1 = UART3_AHB1_PERIPHERALS,
#endif
    .rcc_apb1 = RCC_APB1(USART3),
    .irq = USART3_IRQn,
    .irqPriority = NVIC_PRIO_SERIALUART,
#ifdef USE_UART3_PIN_SWAP
    .pinSwap = true,
#else
    .pinSwap = false,
#endif
};
#endif

#ifdef USE_UART4
static uartDevice_t uart4 =
{
    .dev = UART4,
    .rx = IO_TAG(UART4_RX_PIN),
    .tx = IO_TAG(UART4_TX_PIN),
#if defined(UART4_RX_AF)
    .rx_af = CONCAT(GPIO_MUX_, UART4_RX_AF),
#elif defined(UART4_AF)
    .rx_af = CONCAT(GPIO_MUX_, UART4_AF),
#else
    .rx_af = GPIO_MUX_8,
#endif
#if defined(UART4_TX_AF)
    .tx_af = CONCAT(GPIO_MUX_, UART4_TX_AF),
#elif defined(UART4_AF)
    .tx_af = CONCAT(GPIO_MUX_, UART4_AF),
#else
    .tx_af = GPIO_MUX_8,
#endif
#ifdef UART4_AHB1_PERIPHERALS
    .rcc_ahb1 = UART4_AHB1_PERIPHERALS,
#endif
    .rcc_apb1 = RCC_APB1(UART4),
    .irq = UART4_IRQn,
    .irqPriority = NVIC_PRIO_SERIALUART,
#ifdef USE_UART4_PIN_SWAP
    .pinSwap = true,
#else
    .pinSwap = false,
#endif
};
#endif

#ifdef USE_UART5
static uartDevice_t uart5 =
{
    .dev = UART5,
    .rx = IO_TAG(UART5_RX_PIN),
    .tx = IO_TAG(UART5_TX_PIN),
#if defined(UART5_RX_AF)
    .rx_af = CONCAT(GPIO_MUX_, UART5_RX_AF),
#elif defined(UART5_AF)
    .rx_af = CONCAT(GPIO_MUX_, UART5_AF),
#else
    .rx_af = GPIO_MUX_8,
#endif
#if defined(UART5_TX_AF)
    .tx_af = CONCAT(GPIO_MUX_, UART5_TX_AF),
#elif defined(UART5_AF)
    .tx_af = CONCAT(GPIO_MUX_, UART5_AF),
#else
    .tx_af = GPIO_MUX_8,
#endif
#ifdef UART5_AHB1_PERIPHERALS
    .rcc_ahb1 = UART5_AHB1_PERIPHERALS,
#endif
    .rcc_apb1 = RCC_APB1(UART5),
    .irq = UART5_IRQn,
    .irqPriority = NVIC_PRIO_SERIALUART,
#ifdef USE_UART5_PIN_SWAP
    .pinSwap = true,
#else
    .pinSwap = false,
#endif
};
#endif

#ifdef USE_UART6
static uartDevice_t uart6 =
{
    .dev = USART6,
    .rx = IO_TAG(UART6_RX_PIN),
    .tx = IO_TAG(UART6_TX_PIN),
#if defined(UART6_RX_AF)
    .rx_af = CONCAT(GPIO_MUX_, UART6_RX_AF),
#elif defined(UART6_AF)
    .rx_af = CONCAT(GPIO_MUX_, UART6_AF),
#else
    .rx_af = GPIO_MUX_8,
#endif
#if defined(UART6_TX_AF)
    .tx_af = CONCAT(GPIO_MUX_, UART6_TX_AF),
#elif defined(UART6_AF)
    .tx_af = CONCAT(GPIO_MUX_, UART6_AF),
#else
    .tx_af = GPIO_MUX_8,
#endif
#ifdef UART6_AHB1_PERIPHERALS
    .rcc_ahb1 = UART6_AHB1_PERIPHERALS,
#endif
    .rcc_apb2 = RCC_APB2(USART6),
    .irq = USART6_IRQn,
    .irqPriority = NVIC_PRIO_SERIALUART,
#ifdef USE_UART6_PIN_SWAP
    .pinSwap = true,
#else
    .pinSwap = false,
#endif
};
#endif

#ifdef USE_UART7
static uartDevice_t uart7 =
{
    .dev = UART7,
    .rx = IO_TAG(UART7_RX_PIN),
    .tx = IO_TAG(UART7_TX_PIN),
#if defined(UART7_RX_AF)
    .rx_af = CONCAT(GPIO_MUX_, UART7_RX_AF),
#elif defined(UART7_AF)
    .rx_af = CONCAT(GPIO_MUX_, UART7_AF),
#else
    .rx_af = GPIO_MUX_8,
#endif
#if defined(UART7_TX_AF)
    .tx_af = CONCAT(GPIO_MUX_, UART7_TX_AF),
#elif defined(UART7_AF)
    .tx_af = CONCAT(GPIO_MUX_, UART7_AF),
#else
    .tx_af = GPIO_MUX_8,
#endif
    .rcc_apb1 = RCC_APB1(UART7),
    .irq = UART7_IRQn,
    .irqPriority = NVIC_PRIO_SERIALUART,
#ifdef USE_UART7_PIN_SWAP
    .pinSwap = true,
#else
    .pinSwap = false,
#endif
};
#endif

#ifdef USE_UART8
static uartDevice_t uart8 =
{
    .dev = UART8,
    .rx = IO_TAG(UART8_RX_PIN),
    .tx = IO_TAG(UART8_TX_PIN),
#if defined(UART8_RX_AF)
    .rx_af = CONCAT(GPIO_MUX_, UART8_RX_AF),
#elif defined(UART8_AF)
    .rx_af = CONCAT(GPIO_MUX_, UART8_AF),
#else
    .rx_af = GPIO_MUX_8,
#endif
#if defined(UART8_TX_AF)
    .tx_af = CONCAT(GPIO_MUX_, UART8_TX_AF),
#elif defined(UART8_AF)
    .tx_af = CONCAT(GPIO_MUX_, UART8_AF),
#else
    .tx_af = GPIO_MUX_8,
#endif
    .rcc_apb1 = RCC_APB1(UART8),
    .irq = UART8_IRQn,
    .irqPriority = NVIC_PRIO_SERIALUART,
#ifdef USE_UART8_PIN_SWAP
    .pinSwap = true,
#else
    .pinSwap = false,
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

static uartDevice_t *uartFindDevice(uartPort_t *uartPort)
{
    for (uint32_t i = 0; i < UARTDEV_MAX; i++) {
        uartDevice_t *pDevice = uartHardwareMap[i];

        if (pDevice->dev == uartPort->USARTx) {
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
        usart_transmit_receive_pin_swap(uartPort->USARTx, TRUE);
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
            IOConfigGPIOAF(tx, IOCFG_AF_PP, uart->tx_af);
        } else {
            IOConfigGPIOAF(tx,
                    (options & SERIAL_BIDIR_NOPULL) ? IOCFG_AF_OD : IOCFG_AF_OD_UP,
                    uart->tx_af);
        }
    }
    else {
        if (mode & MODE_TX) {
            IOInit(tx, OWNER_SERIAL, RESOURCE_UART_TX, RESOURCE_INDEX(device));
            IOConfigGPIOAF(tx, IOCFG_AF_PP, uart->rx_af);
        }

        if (mode & MODE_RX) {
            IOInit(rx, OWNER_SERIAL, RESOURCE_UART_RX, RESOURCE_INDEX(device));
            IOConfigGPIOAF(rx, IOCFG_AF_PP, uart->rx_af);
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
