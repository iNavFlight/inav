/*
    Copyright (C) 2014..2017 Marco Veeneman

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    UART/hal_uart_lld.c
 * @brief   Tiva low level UART driver code.
 *
 * @addtogroup UART
 * @{
 */

#include "hal.h"

#if HAL_USE_UART || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief UART0 UART driver identifier.*/
#if TIVA_UART_USE_UART0 || defined(__DOXYGEN__)
UARTDriver UARTD1;
#endif

/** @brief UART1 UART driver identifier.*/
#if TIVA_UART_USE_UART1 || defined(__DOXYGEN__)
UARTDriver UARTD2;
#endif

/** @brief UART2 UART driver identifier.*/
#if TIVA_UART_USE_UART2 || defined(__DOXYGEN__)
UARTDriver UARTD3;
#endif

/** @brief UART3 UART driver identifier.*/
#if TIVA_UART_USE_UART3 || defined(__DOXYGEN__)
UARTDriver UARTD4;
#endif

/** @brief UART4 UART driver identifier.*/
#if TIVA_UART_USE_UART4 || defined(__DOXYGEN__)
UARTDriver UARTD5;
#endif

/** @brief UART5 UART driver identifier.*/
#if TIVA_UART_USE_UART5 || defined(__DOXYGEN__)
UARTDriver UARTD6;
#endif

/** @brief UART6 UART driver identifier.*/
#if TIVA_UART_USE_UART6 || defined(__DOXYGEN__)
UARTDriver UARTD7;
#endif

/** @brief UART7 UART driver identifier.*/
#if TIVA_UART_USE_UART7 || defined(__DOXYGEN__)
UARTDriver UARTD8;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Status bits translation.
 *
 * @param[in] err       UART LSR register value
 *
 * @return  The error flags.
 */
static uartflags_t translate_errors(uint32_t err)
{
  uartflags_t sts = 0;

  if (err & UART_MIS_FEMIS)
    sts |= UART_FRAMING_ERROR;
  if (err & UART_MIS_PEMIS)
    sts |= UART_PARITY_ERROR;
  if (err & UART_MIS_BEMIS)
    sts |= UART_BREAK_DETECTED;
  if (err & UART_MIS_OEMIS)
    sts |= UART_OVERRUN_ERROR;

  return sts;
}

/**
 * @brief   Puts the receiver in the UART_RX_IDLE state.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 */
static void uart_enter_rx_idle_loop(UARTDriver *uartp)
{
  tiva_udma_table_entry_t *primary = udmaControlTable.primary;

  dmaChannelDisable(uartp->dmarxnr);

  /* Configure for 8-bit transfers.*/
  primary[uartp->dmarxnr].srcendp = (void *)(uartp->uart + UART_O_DR);
  primary[uartp->dmarxnr].dstendp = (volatile void *)&uartp->rxbuf;
  primary[uartp->dmarxnr].chctl = UDMA_CHCTL_DSTSIZE_8 | UDMA_CHCTL_DSTINC_8 |
                                  UDMA_CHCTL_SRCSIZE_8 | UDMA_CHCTL_SRCINC_NONE |
                                  UDMA_CHCTL_ARBSIZE_4 |
                                  UDMA_CHCTL_XFERSIZE(1) |
                                  UDMA_CHCTL_XFERMODE_BASIC;

  dmaChannelSingleBurst(uartp->dmarxnr);
  dmaChannelPrimary(uartp->dmarxnr);
  dmaChannelPriorityDefault(uartp->dmarxnr);
  dmaChannelEnableRequest(uartp->dmarxnr);

  /* Enable DMA channel, transfer starts immediately.*/
  dmaChannelEnable(uartp->dmarxnr);
}

/**
 * @brief   UART de-initialization.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 */
static void uart_stop(UARTDriver *uartp)
{
  /* Stops RX and TX DMA channels.*/
  dmaChannelDisable(uartp->dmarxnr);
  dmaChannelDisable(uartp->dmatxnr);

  /* Stops USART operations.*/
  HWREG(uartp->uart + UART_O_CTL) &= ~UART_CTL_UARTEN;
}

/**
 * @brief   UART initialization.
 *
 * @param[in] uartp     pointer to a @p UARTDriver object
 */
static void uart_init(UARTDriver *uartp)
{
  uint32_t u = uartp->uart;
  const UARTConfig *config = uartp->config;
  uint32_t brd;
  uint32_t speed = config->speed;
  uint32_t clock_source;

  if (uartp->config->ctl & UART_CTL_HSE) {
    /* High speed mode is enabled, half the baud rate to compensate
     * for high speed mode.*/
    speed = (speed + 1) / 2;
  }

  if ((config->cc & UART_CC_CS_M) == UART_CC_CS_SYSCLK) {
    /* UART is clocked using the SYSCLK.*/
    clock_source = TIVA_SYSCLK * 8;
  }
  else {
    /* UART is clocked using the PIOSC.*/
    clock_source = 16000000 * 8;
  }

  /* Calculate the baud rate divisor */
  brd = ((clock_source / speed) + 1) / 2;

  /* Disable UART.*/
  HWREG(u + UART_O_CTL) &= ~UART_CTL_UARTEN;

  /* Set baud rate.*/
  HWREG(u + UART_O_IBRD) = brd / 64;
  HWREG(u + UART_O_FBRD) = brd % 64;

  /* Line control/*/
  HWREG(u + UART_O_LCRH) = config->lcrh;

  /* Select clock source.*/
  HWREG(u + UART_O_CC) = config->cc & UART_CC_CS_M;

  /* FIFO configuration.*/
  HWREG(u + UART_O_IFLS) = config->ifls & (UART_IFLS_RX_M | UART_IFLS_TX_M);

  /* Enable interrupts.*/
  HWREG(u + UART_O_IM) = UART_IM_TXIM | UART_IM_OEIM | UART_IM_BEIM | UART_IM_PEIM | UART_IM_FEIM;

  /* Enable DMA for the UART */
  HWREG(u + UART_O_DMACTL) = UART_DMACTL_TXDMAE | UART_DMACTL_RXDMAE | UART_DMACTL_DMAERR;

  /* Note that some bits are enforced.*/
  HWREG(u + UART_O_CTL)  = config->ctl | UART_CTL_RXE | UART_CTL_TXE | UART_CTL_UARTEN | UART_CTL_EOT;

  /* Starting the receiver idle loop.*/
  uart_enter_rx_idle_loop(uartp);
}

/**
 * @brief   UART common service routine.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 */
static void uart_serve_interrupt(UARTDriver *uartp)
{
  uint32_t dmachis = HWREG(UDMA_CHIS);
  uint32_t mis = HWREG(uartp->uart + UART_O_MIS);

  if (mis & UART_MIS_TXMIS) {
    /* End of transfer */
    _uart_tx2_isr_code(uartp);
  }

  if (mis & (UART_MIS_OEMIS | UART_MIS_BEMIS | UART_MIS_PEMIS | UART_MIS_FEMIS)) {
    /* Error occurred */
    _uart_rx_error_isr_code(uartp, translate_errors(mis));
  }

  if (dmachis & (1 << uartp->dmarxnr)) {
    if (uartp->rxstate == UART_RX_IDLE) {
      /* Receiver in idle state, a callback is generated, if enabled, for each
         received character and then the driver stays in the same state.*/
      _uart_rx_idle_code(uartp);
      uart_enter_rx_idle_loop(uartp);
    }
    else {
      /* Receiver in active state, a callback is generated, if enabled, after
         a completed transfer.*/
      _uart_rx_complete_isr_code(uartp);
    }
  }

  if (dmachis & (1 << uartp->dmatxnr)) {
    /* A callback is generated, if enabled, after a completed transfer.*/
    _uart_tx1_isr_code(uartp);
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if TIVA_UART_USE_UART0 || defined(__DOXYGEN__)
#if !defined(TIVA_UART0_HANDLER)
#error "TIVA_UART0_HANDLER not defined"
#endif
/**
 * @brief   UART0 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_UART0_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  uart_serve_interrupt(&UARTD1);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_UART_USE_UART1 || defined(__DOXYGEN__)
#if !defined(TIVA_UART1_HANDLER)
#error "TIVA_UART1_HANDLER not defined"
#endif
/**
 * @brief   UART1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_UART1_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  uart_serve_interrupt(&UARTD2);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_UART_USE_UART2 || defined(__DOXYGEN__)
#if !defined(TIVA_UART2_HANDLER)
#error "TIVA_UART2_HANDLER not defined"
#endif
/**
 * @brief   UART2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_UART2_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  uart_serve_interrupt(&UARTD3);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_UART_USE_UART3 || defined(__DOXYGEN__)
#if !defined(TIVA_UART3_HANDLER)
#error "TIVA_UART3_HANDLER not defined"
#endif
/**
 * @brief   UART3 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_UART3_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  uart_serve_interrupt(&UARTD4);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_UART_USE_UART4 || defined(__DOXYGEN__)
#if !defined(TIVA_UART4_HANDLER)
#error "TIVA_UART4_HANDLER not defined"
#endif
/**
 * @brief   UART4 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_UART4_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  uart_serve_interrupt(&UARTD5);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_UART_USE_UART5 || defined(__DOXYGEN__)
#if !defined(TIVA_UART5_HANDLER)
#error "TIVA_UART5_HANDLER not defined"
#endif
/**
 * @brief   UART5 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_UART5_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  uart_serve_interrupt(&UARTD6);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_UART_USE_UART6 || defined(__DOXYGEN__)
#if !defined(TIVA_UART6_HANDLER)
#error "TIVA_UART6_HANDLER not defined"
#endif
/**
 * @brief   UART6 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_UART6_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  uart_serve_interrupt(&UARTD7);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_UART_USE_UART7 || defined(__DOXYGEN__)
#if !defined(TIVA_UART7_HANDLER)
#error "TIVA_UART7_HANDLER not defined"
#endif
/**
 * @brief   UART7 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_UART7_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  uart_serve_interrupt(&UARTD8);

  OSAL_IRQ_EPILOGUE();
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level UART driver initialization.
 *
 * @notapi
 */
void uart_lld_init(void)
{
#if TIVA_UART_USE_UART0
  uartObjectInit(&UARTD1);
  UARTD1.uart    = UART0_BASE;
  UARTD1.dmarxnr = TIVA_UART_UART0_RX_UDMA_CHANNEL;
  UARTD1.dmatxnr = TIVA_UART_UART0_TX_UDMA_CHANNEL;
  UARTD1.rxchnmap = TIVA_UART_UART0_RX_UDMA_MAPPING;
  UARTD1.txchnmap = TIVA_UART_UART0_TX_UDMA_MAPPING;
#endif
#if TIVA_UART_USE_UART1
  uartObjectInit(&UARTD2);
  UARTD2.uart    = UART1_BASE;
  UARTD2.dmarxnr = TIVA_UART_UART1_RX_UDMA_CHANNEL;
  UARTD2.dmatxnr = TIVA_UART_UART1_TX_UDMA_CHANNEL;
  UARTD2.rxchnmap = TIVA_UART_UART1_RX_UDMA_MAPPING;
  UARTD2.txchnmap = TIVA_UART_UART1_TX_UDMA_MAPPING;
#endif
#if TIVA_UART_USE_UART2
  uartObjectInit(&UARTD3);
  UARTD2.uart    = UART2_BASE;
  UARTD2.dmarxnr = TIVA_UART_UART2_RX_UDMA_CHANNEL;
  UARTD2.dmatxnr = TIVA_UART_UART2_TX_UDMA_CHANNEL;
  UARTD2.rxchnmap = TIVA_UART_UART2_RX_UDMA_MAPPING;
  UARTD2.txchnmap = TIVA_UART_UART2_TX_UDMA_MAPPING;
#endif
#if TIVA_UART_USE_UART3
  uartObjectInit(&UARTD4);
  UARTD4.uart    = UART3_BASE;
  UARTD4.dmarxnr = TIVA_UART_UART3_RX_UDMA_CHANNEL;
  UARTD4.dmatxnr = TIVA_UART_UART3_TX_UDMA_CHANNEL;
  UARTD4.rxchnmap = TIVA_UART_UART3_RX_UDMA_MAPPING;
  UARTD4.txchnmap = TIVA_UART_UART3_TX_UDMA_MAPPING;
#endif
#if TIVA_UART_USE_UART4
  uartObjectInit(&UARTD5);
  UARTD5.uart    = UART4_BASE;
  UARTD5.dmarxnr = TIVA_UART_UART4_RX_UDMA_CHANNEL;
  UARTD5.dmatxnr = TIVA_UART_UART4_TX_UDMA_CHANNEL;
  UARTD5.rxchnmap = TIVA_UART_UART4_RX_UDMA_MAPPING;
  UARTD5.txchnmap = TIVA_UART_UART4_TX_UDMA_MAPPING;
#endif
#if TIVA_UART_USE_UART5
  uartObjectInit(&UARTD6);
  UARTD6.uart    = UART5_BASE;
  UARTD6.dmarxnr = TIVA_UART_UART5_RX_UDMA_CHANNEL;
  UARTD6.dmatxnr = TIVA_UART_UART5_TX_UDMA_CHANNEL;
  UARTD6.rxchnmap = TIVA_UART_UART5_RX_UDMA_MAPPING;
  UARTD6.txchnmap = TIVA_UART_UART5_TX_UDMA_MAPPING;
#endif
#if TIVA_UART_USE_UART6
  uartObjectInit(&UARTD7);
  UARTD7.uart    = UART6_BASE;
  UARTD7.dmarxnr = TIVA_UART_UART6_RX_UDMA_CHANNEL;
  UARTD7.dmatxnr = TIVA_UART_UART6_TX_UDMA_CHANNEL;
  UARTD7.rxchnmap = TIVA_UART_UART6_RX_UDMA_MAPPING;
  UARTD7.txchnmap = TIVA_UART_UART6_TX_UDMA_MAPPING;
#endif
#if TIVA_UART_USE_UART7
  uartObjectInit(&UARTD8);
  UARTD8.uart    = UART7_BASE;
  UARTD8.dmarxnr = TIVA_UART_UART7_RX_UDMA_CHANNEL;
  UARTD8.dmatxnr = TIVA_UART_UART7_TX_UDMA_CHANNEL;
  UARTD8.rxchnmap = TIVA_UART_UART7_RX_UDMA_MAPPING;
  UARTD8.txchnmap = TIVA_UART_UART7_TX_UDMA_MAPPING;
#endif
}

/**
 * @brief   Configures and activates the UART peripheral.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 *
 * @notapi
 */
void uart_lld_start(UARTDriver *uartp) {

  if (uartp->state == UART_STOP) {
#if TIVA_UART_USE_UART0
    if (&UARTD1 == uartp) {
      bool b;
      b = udmaChannelAllocate(uartp->dmarxnr);
      osalDbgAssert(!b, "channel already allocated");
      b = udmaChannelAllocate(uartp->dmatxnr);
      osalDbgAssert(!b, "channel already allocated");

      HWREG(SYSCTL_RCGCUART) |= (1 << 0);

      while (!(HWREG(SYSCTL_PRUART) & (1 << 0)))
        ;

      nvicEnableVector(TIVA_UART0_NUMBER, TIVA_UART_UART0_PRIORITY);
    }
#endif
#if TIVA_UART_USE_UART1
    if (&UARTD2 == uartp) {
      bool b;
      b = udmaChannelAllocate(uartp->dmarxnr);
      osalDbgAssert(!b, "channel already allocated");
      b = udmaChannelAllocate(uartp->dmatxnr);
      osalDbgAssert(!b, "channel already allocated");

      HWREG(SYSCTL_RCGCUART) |= (1 << 1);

      while (!(HWREG(SYSCTL_PRUART) & (1 << 1)))
        ;

      nvicEnableVector(TIVA_UART1_NUMBER, TIVA_UART_UART1_PRIORITY);
    }
#endif
#if TIVA_UART_USE_UART2
    if (&UARTD3 == uartp) {
      bool b;
      b = udmaChannelAllocate(uartp->dmarxnr);
      osalDbgAssert(!b, "channel already allocated");
      b = udmaChannelAllocate(uartp->dmatxnr);
      osalDbgAssert(!b, "channel already allocated");

      HWREG(SYSCTL_RCGCUART) |= (1 << 2);

      while (!(HWREG(SYSCTL_PRUART) & (1 << 2)))
        ;

      nvicEnableVector(TIVA_UART2_NUMBER, TIVA_UART_UART2_PRIORITY);
    }
#endif
#if TIVA_UART_USE_UART3
    if (&UARTD4 == uartp) {
      bool b;
      b = udmaChannelAllocate(uartp->dmarxnr);
      osalDbgAssert(!b, "channel already allocated");
      b = udmaChannelAllocate(uartp->dmatxnr);
      osalDbgAssert(!b, "channel already allocated");

      HWREG(SYSCTL_RCGCUART) |= (1 << 3);

      while (!(HWREG(SYSCTL_PRUART) & (1 << 3)))
        ;

      nvicEnableVector(TIVA_UART3_NUMBER, TIVA_UART_UART3_PRIORITY);
    }
#endif
#if TIVA_UART_USE_UART4
    if (&UARTD5 == uartp) {
      bool b;
      b = udmaChannelAllocate(uartp->dmarxnr);
      osalDbgAssert(!b, "channel already allocated");
      b = udmaChannelAllocate(uartp->dmatxnr);
      osalDbgAssert(!b, "channel already allocated");

      HWREG(SYSCTL_RCGCUART) |= (1 << 4);

      while (!(HWREG(SYSCTL_PRUART) & (1 << 4)))
        ;

      nvicEnableVector(TIVA_UART4_NUMBER, TIVA_UART_UART4_PRIORITY);
    }
#endif
#if TIVA_UART_USE_UART5
    if (&UARTD6 == uartp) {
      bool b;
      b = udmaChannelAllocate(uartp->dmarxnr);
      osalDbgAssert(!b, "channel already allocated");
      b = udmaChannelAllocate(uartp->dmatxnr);
      osalDbgAssert(!b, "channel already allocated");

      HWREG(SYSCTL_RCGCUART) |= (1 << 5);

      while (!(HWREG(SYSCTL_PRUART) & (1 << 5)))
        ;

      nvicEnableVector(TIVA_UART5_NUMBER, TIVA_UART_UART5_PRIORITY);
    }
#endif
#if TIVA_UART_USE_UART6
    if (&UARTD7 == uartp) {
      bool b;
      b = udmaChannelAllocate(uartp->dmarxnr);
      osalDbgAssert(!b, "channel already allocated");
      b = udmaChannelAllocate(uartp->dmatxnr);
      osalDbgAssert(!b, "channel already allocated");

      HWREG(SYSCTL_RCGCUART) |= (1 << 6);

      while (!(HWREG(SYSCTL_PRUART) & (1 << 6)))
        ;

      nvicEnableVector(TIVA_UART6_NUMBER, TIVA_UART_UART6_PRIORITY);
    }
#endif
#if TIVA_UART_USE_UART7
    if (&UARTD8 == uartp) {
      bool b;
      b = udmaChannelAllocate(uartp->dmarxnr);
      osalDbgAssert(!b, "channel already allocated");
      b = udmaChannelAllocate(uartp->dmatxnr);
      osalDbgAssert(!b, "channel already allocated");

      HWREG(SYSCTL_RCGCUART) |= (1 << 7);

      while (!(HWREG(SYSCTL_PRUART) & (1 << 7)))
        ;

      nvicEnableVector(TIVA_UART7_NUMBER, TIVA_UART_UART7_PRIORITY);
    }
#endif

    uartp->rxbuf = 0;

    HWREG(UDMA_CHMAP0 + (uartp->dmarxnr / 8) * 4) |= (uartp->rxchnmap << (uartp->dmarxnr % 8));
    HWREG(UDMA_CHMAP0 + (uartp->dmatxnr / 8) * 4) |= (uartp->txchnmap << (uartp->dmatxnr % 8));
  }

  uartp->rxstate = UART_RX_IDLE;
  uartp->txstate = UART_TX_IDLE;
  uart_init(uartp);
}

/**
 * @brief   Deactivates the UART peripheral.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 *
 * @notapi
 */
void uart_lld_stop(UARTDriver *uartp) {

  if (uartp->state == UART_READY) {
    uart_stop(uartp);
    udmaChannelRelease(uartp->dmarxnr);
    udmaChannelRelease(uartp->dmatxnr);

#if TIVA_UART_USE_UART0
    if (&UARTD1 == uartp) {
      HWREG(SYSCTL_RCGCUART) &= ~(1 << 0);
      return;
    }
#endif
#if TIVA_UART_USE_UART1
    if (&UARTD2 == uartp) {
      HWREG(SYSCTL_RCGCUART) &= ~(1 << 1);
      return;
    }
#endif
#if TIVA_UART_USE_UART2
    if (&UARTD3 == uartp) {
      HWREG(SYSCTL_RCGCUART) &= ~(1 << 2);
      return;
    }
#endif
#if TIVA_UART_USE_UART3
    if (&UARTD4 == uartp) {
      HWREG(SYSCTL_RCGCUART) &= ~(1 << 3);
      return;
    }
#endif
#if TIVA_UART_USE_UART4
    if (&UARTD5 == uartp) {
      HWREG(SYSCTL_RCGCUART) &= ~(1 << 4);
      return;
    }
#endif
#if TIVA_UART_USE_UART5
    if (&UARTD6 == uartp) {
      HWREG(SYSCTL_RCGCUART) &= ~(1 << 5);
      return;
    }
#endif
#if TIVA_UART_USE_UART6
    if (&UARTD7 == uartp) {
      HWREG(SYSCTL_RCGCUART) &= ~(1 << 6);
      return;
    }
#endif
#if TIVA_UART_USE_UART7
    if (&UARTD8 == uartp) {
      HWREG(SYSCTL_RCGCUART) &= ~(1 << 7);
      return;
    }
#endif
  }
}

/**
 * @brief   Starts a transmission on the UART peripheral.
 * @note    The buffers are organized as uint8_t arrays for data sizes below
 *          or equal to 8 bits else it is organized as uint16_t arrays.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 * @param[in] n         number of data frames to send
 * @param[in] txbuf     the pointer to the transmit buffer
 *
 * @notapi
 */
void uart_lld_start_send(UARTDriver *uartp, size_t n, const void *txbuf)
{
  tiva_udma_table_entry_t *primary = udmaControlTable.primary;

  /* TODO: This assert should be moved to the dma helper driver */
  osalDbgAssert((uint32_t)txbuf >= SRAM_BASE, "txbuf should be in SRAM region.");

  dmaChannelDisable(uartp->dmatxnr);

  /* Configure for 8-bit transfers.*/
  primary[uartp->dmatxnr].srcendp = (volatile void *)txbuf+n-1;
  primary[uartp->dmatxnr].dstendp = (void *)(uartp->uart + UART_O_DR);
  primary[uartp->dmatxnr].chctl = UDMA_CHCTL_DSTSIZE_8 | UDMA_CHCTL_DSTINC_NONE |
                                  UDMA_CHCTL_SRCSIZE_8 | UDMA_CHCTL_SRCINC_8 |
                                  UDMA_CHCTL_ARBSIZE_4 |
                                  UDMA_CHCTL_XFERSIZE(n) |
                                  UDMA_CHCTL_XFERMODE_BASIC;

  dmaChannelSingleBurst(uartp->dmatxnr);
  dmaChannelPrimary(uartp->dmatxnr);
  dmaChannelPriorityDefault(uartp->dmatxnr);
  dmaChannelEnableRequest(uartp->dmatxnr);

  /* Enable DMA channel, transfer starts immediately.*/
  dmaChannelEnable(uartp->dmatxnr);
}

/**
 * @brief   Stops any ongoing transmission.
 * @note    Stopping a transmission also suppresses the transmission callbacks.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 *
 * @return              The number of data frames not transmitted by the
 *                      stopped transmit operation.
 *
 * @notapi
 */
size_t uart_lld_stop_send(UARTDriver *uartp)
{
  tiva_udma_table_entry_t *primary = udmaControlTable.primary;
  uint16_t left;

  dmaChannelDisable(uartp->dmatxnr);

  left = ((primary[uartp->dmatxnr].chctl & UDMA_CHCTL_XFERSIZE_M) + 1) >> 4;

  return left;
}

/**
 * @brief   Starts a receive operation on the UART peripheral.
 * @note    The buffers are organized as uint8_t arrays for data sizes below
 *          or equal to 8 bits else it is organized as uint16_t arrays.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 * @param[in] n         number of data frames to send
 * @param[out] rxbuf    the pointer to the receive buffer
 *
 * @notapi
 */
void uart_lld_start_receive(UARTDriver *uartp, size_t n, void *rxbuf)
{
  tiva_udma_table_entry_t *primary = udmaControlTable.primary;

  /* TODO: This assert should be moved to the dma helper driver */
  osalDbgAssert((uint32_t)rxbuf >= SRAM_BASE, "rxbuf should be in SRAM region.");

  dmaChannelDisable(uartp->dmarxnr);

  /* Configure for 8-bit transfers.*/
  primary[uartp->dmarxnr].srcendp = (void *)(uartp->uart + UART_O_DR);
  primary[uartp->dmarxnr].dstendp = (volatile void *)rxbuf+n-1;
  primary[uartp->dmarxnr].chctl = UDMA_CHCTL_DSTSIZE_8 | UDMA_CHCTL_DSTINC_8 |
                                  UDMA_CHCTL_SRCSIZE_8 | UDMA_CHCTL_SRCINC_NONE |
                                  UDMA_CHCTL_ARBSIZE_4 |
                                  UDMA_CHCTL_XFERSIZE(n) |
                                  UDMA_CHCTL_XFERMODE_BASIC;

  dmaChannelSingleBurst(uartp->dmarxnr);
  dmaChannelPrimary(uartp->dmarxnr);
  dmaChannelPriorityDefault(uartp->dmarxnr);
  dmaChannelEnableRequest(uartp->dmarxnr);

  /* Enable DMA channel, transfer starts immediately.*/
  dmaChannelEnable(uartp->dmarxnr);
}

/**
 * @brief   Stops any ongoing receive operation.
 * @note    Stopping a receive operation also suppresses the receive callbacks.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 *
 * @return              The number of data frames not received by the
 *                      stopped receive operation.
 *
 * @notapi
 */
size_t uart_lld_stop_receive(UARTDriver *uartp)
{
  tiva_udma_table_entry_t *primary = udmaControlTable.primary;
  uint16_t left;

  dmaChannelDisable(uartp->dmatxnr);

  left = ((primary[uartp->dmatxnr].chctl & UDMA_CHCTL_XFERSIZE_M) + 1) >> 4;

  uart_enter_rx_idle_loop(uartp);

  return left;
}

#endif /* HAL_USE_UART */

/** @} */
