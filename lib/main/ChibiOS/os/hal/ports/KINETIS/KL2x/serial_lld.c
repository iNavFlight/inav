/*
    ChibiOS - Copyright (C) 2013-2015 Fabio Utzig

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
 * @file    KL2x/serial_lld.c
 * @brief   Kinetis KL2x Serial Driver subsystem low level driver source.
 *
 * @addtogroup SERIAL
 * @{
 */

#include "osal.h"
#include "hal.h"

#if HAL_USE_SERIAL || defined(__DOXYGEN__)

#include "kl25z.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   SD1 driver identifier.
 */
#if KINETIS_SERIAL_USE_UART0 || defined(__DOXYGEN__)
SerialDriver SD1;
#endif

#if KINETIS_SERIAL_USE_UART1 || defined(__DOXYGEN__)
SerialDriver SD2;
#endif

#if KINETIS_SERIAL_USE_UART2 || defined(__DOXYGEN__)
SerialDriver SD3;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   Driver default configuration.
 */
static const SerialConfig default_config = {
  38400
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Common IRQ handler.
 * @note    Tries hard to clear all the pending interrupt sources, we don't
 *          want to go through the whole ISR and have another interrupt soon
 *          after.
 *
 * @param[in] u         pointer to an UART I/O block
 * @param[in] sdp       communication channel associated to the UART
 */
static void serve_interrupt(SerialDriver *sdp) {
  UARTLP_TypeDef *u = sdp->uart;

  if (u->S1 & UARTx_S1_RDRF) {
    osalSysLockFromISR();
    if (iqIsEmptyI(&sdp->iqueue))
      chnAddFlagsI(sdp, CHN_INPUT_AVAILABLE);
    if (iqPutI(&sdp->iqueue, u->D) < Q_OK)
      chnAddFlagsI(sdp, SD_OVERRUN_ERROR);
    osalSysUnlockFromISR();
  }

  if (u->S1 & UARTx_S1_TDRE) {
    msg_t b;

    osalSysLockFromISR();
    b = oqGetI(&sdp->oqueue);
    osalSysUnlockFromISR();

    if (b < Q_OK) {
      osalSysLockFromISR();
      chnAddFlagsI(sdp, CHN_OUTPUT_EMPTY);
      osalSysUnlockFromISR();
      u->C2 &= ~UARTx_C2_TIE;
    } else {
       u->D = b;
    }
  }

  if (u->S1 & UARTx_S1_IDLE)
    u->S1 = UARTx_S1_IDLE;  // Clear IDLE (S1 bits are write-1-to-clear).

  if (u->S1 & (UARTx_S1_OR | UARTx_S1_NF | UARTx_S1_FE | UARTx_S1_PF)) {
    // FIXME: need to add set_error()
    // Clear flags (S1 bits are write-1-to-clear).
    u->S1 = UARTx_S1_OR | UARTx_S1_NF | UARTx_S1_FE | UARTx_S1_PF;
  }
}

/**
 * @brief   Attempts a TX preload
 */
static void preload(SerialDriver *sdp) {
  UARTLP_TypeDef *u = sdp->uart;

  if (u->S1 & UARTx_S1_TDRE) {
    msg_t b = oqGetI(&sdp->oqueue);
    if (b < Q_OK) {
      chnAddFlagsI(sdp, CHN_OUTPUT_EMPTY);
      return;
    }
    u->D = b;
    u->C2 |= UARTx_C2_TIE;
  }
}

/**
 * @brief   Driver output notification.
 */
#if KINETIS_SERIAL_USE_UART0 || defined(__DOXYGEN__)
static void notify1(io_queue_t *qp)
{
  (void)qp;
  preload(&SD1);
}
#endif

#if KINETIS_SERIAL_USE_UART1 || defined(__DOXYGEN__)
static void notify2(io_queue_t *qp)
{
  (void)qp;
  preload(&SD2);
}
#endif

#if KINETIS_SERIAL_USE_UART2 || defined(__DOXYGEN__)
static void notify3(io_queue_t *qp)
{
  (void)qp;
  preload(&SD3);
}
#endif

/**
 * @brief   Common UART configuration.
 *
 */
static void configure_uart(UARTLP_TypeDef *uart, const SerialConfig *config)
{
  uint32_t uart_clock;

  uart->C1 = 0;
  uart->C3 = UARTx_C3_ORIE | UARTx_C3_NEIE | UARTx_C3_FEIE | UARTx_C3_PEIE;
  uart->S1 = UARTx_S1_IDLE | UARTx_S1_OR | UARTx_S1_NF | UARTx_S1_FE | UARTx_S1_PF;
  while (uart->S1 & UARTx_S1_RDRF) {
    (void)uart->D;
  }

#if KINETIS_SERIAL_USE_UART0
    if (uart == UART0) {
        /* UART0 can be clocked from several sources. */
        uart_clock = KINETIS_UART0_CLOCK_FREQ;
    }
#endif
#if KINETIS_SERIAL_USE_UART1
    if (uart == UART1) {
        uart_clock = KINETIS_BUSCLK_FREQUENCY;
    }
#endif
#if KINETIS_SERIAL_USE_UART2
    if (uart == UART2) {
        uart_clock = KINETIS_BUSCLK_FREQUENCY;
    }
#endif

  /* FIXME: change fixed OSR = 16 to dynamic value based on baud */
  uint16_t divisor = (uart_clock / 16) / config->sc_speed;
  uart->C4 = UARTx_C4_OSR & (16 - 1);
  uart->BDH = (divisor >> 8) & UARTx_BDH_SBR;
  uart->BDL = (divisor & UARTx_BDL_SBR);

  uart->C2 = UARTx_C2_RE | UARTx_C2_RIE | UARTx_C2_TE;
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if KINETIS_SERIAL_USE_UART0 || defined(__DOXYGEN__)
OSAL_IRQ_HANDLER(Vector70) {

  OSAL_IRQ_PROLOGUE();
  serve_interrupt(&SD1);
  OSAL_IRQ_EPILOGUE();
}
#endif

#if KINETIS_SERIAL_USE_UART1 || defined(__DOXYGEN__)
OSAL_IRQ_HANDLER(Vector74) {

  OSAL_IRQ_PROLOGUE();
  serve_interrupt(&SD2);
  OSAL_IRQ_EPILOGUE();
}
#endif

#if KINETIS_SERIAL_USE_UART2 || defined(__DOXYGEN__)
OSAL_IRQ_HANDLER(Vector78) {

  OSAL_IRQ_PROLOGUE();
  serve_interrupt(&SD3);
  OSAL_IRQ_EPILOGUE();
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level serial driver initialization.
 *
 * @notapi
 */
void sd_lld_init(void) {

#if KINETIS_SERIAL_USE_UART0
  /* Driver initialization.*/
  sdObjectInit(&SD1, NULL, notify1);
  SD1.uart = UART0;
#endif

#if KINETIS_SERIAL_USE_UART1
  /* Driver initialization.*/
  sdObjectInit(&SD2, NULL, notify2);
  SD2.uart = UART1;
#endif

#if KINETIS_SERIAL_USE_UART2
  /* Driver initialization.*/
  sdObjectInit(&SD3, NULL, notify3);
  SD3.uart = UART2;
#endif
}

/**
 * @brief   Low level serial driver configuration and (re)start.
 *
 * @param[in] sdp       pointer to a @p SerialDriver object
 * @param[in] config    the architecture-dependent serial driver configuration.
 *                      If this parameter is set to @p NULL then a default
 *                      configuration is used.
 *
 * @notapi
 */
void sd_lld_start(SerialDriver *sdp, const SerialConfig *config) {

  if (config == NULL)
    config = &default_config;

  if (sdp->state == SD_STOP) {
    /* Enables the peripheral.*/

#if KINETIS_SERIAL_USE_UART0
    if (sdp == &SD1) {
      SIM->SCGC4 |= SIM_SCGC4_UART0;
      SIM->SOPT2 =
              (SIM->SOPT2 & ~SIM_SOPT2_UART0SRC_MASK) |
              SIM_SOPT2_UART0SRC(KINETIS_UART0_CLOCK_SRC);
      configure_uart(sdp->uart, config);
      nvicEnableVector(UART0_IRQn, KINETIS_SERIAL_UART0_PRIORITY);
    }
#endif /* KINETIS_SERIAL_USE_UART0 */

#if KINETIS_SERIAL_USE_UART1
    if (sdp == &SD2) {
      SIM->SCGC4 |= SIM_SCGC4_UART1;
      configure_uart(sdp->uart, config);
      nvicEnableVector(UART1_IRQn, KINETIS_SERIAL_UART1_PRIORITY);
    }
#endif /* KINETIS_SERIAL_USE_UART1 */

#if KINETIS_SERIAL_USE_UART2
    if (sdp == &SD3) {
      SIM->SCGC4 |= SIM_SCGC4_UART2;
      configure_uart(sdp->uart, config);
      nvicEnableVector(UART2_IRQn, KINETIS_SERIAL_UART2_PRIORITY);
    }
#endif /* KINETIS_SERIAL_USE_UART2 */

  }
  /* Configures the peripheral.*/

}

/**
 * @brief   Low level serial driver stop.
 * @details De-initializes the USART, stops the associated clock, resets the
 *          interrupt vector.
 *
 * @param[in] sdp       pointer to a @p SerialDriver object
 *
 * @notapi
 */
void sd_lld_stop(SerialDriver *sdp) {

  if (sdp->state == SD_READY) {
    /* TODO: Resets the peripheral.*/

#if KINETIS_SERIAL_USE_UART0
    if (sdp == &SD1) {
      nvicDisableVector(UART0_IRQn);
      SIM->SCGC4 &= ~SIM_SCGC4_UART0;
    }
#endif

#if KINETIS_SERIAL_USE_UART1
    if (sdp == &SD2) {
      nvicDisableVector(UART1_IRQn);
      SIM->SCGC4 &= ~SIM_SCGC4_UART1;
    }
#endif

#if KINETIS_SERIAL_USE_UART2
    if (sdp == &SD3) {
      nvicDisableVector(UART2_IRQn);
      SIM->SCGC4 &= ~SIM_SCGC4_UART2;
    }
#endif
  }
}

#endif /* HAL_USE_SERIAL */

/** @} */
