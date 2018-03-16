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
 * @brief   Error handling routine.
 *
 * @param[in] sdp       pointer to a @p SerialDriver object
 * @param[in] isr       UART s1 register value
 */
static void set_error(SerialDriver *sdp, uint8_t s1) {
  eventflags_t sts = 0;

  if (s1 & UARTx_S1_OR)
    sts |= SD_OVERRUN_ERROR;
  if (s1 & UARTx_S1_PF)
    sts |= SD_PARITY_ERROR;
  if (s1 & UARTx_S1_FE)
    sts |= SD_FRAMING_ERROR;
  if (s1 & UARTx_S1_NF)
    sts |= SD_NOISE_ERROR;
  osalSysLockFromISR();
  chnAddFlagsI(sdp, sts);
  osalSysUnlockFromISR();
}

/**
 * @brief   Common error IRQ handler.
 *
 * @param[in] sdp       communication channel associated to the UART
 */
static void serve_error_interrupt(SerialDriver *sdp) {
  UART_w_TypeDef *u = &(sdp->uart);
  uint8_t s1 = *(u->s1_p);

  /* S1 bits are write-1-to-clear for UART0 on KL2x. */
  /* Clearing on K20x and KL2x/UART>0 is done by reading S1 and
   * then reading D.*/

#if defined(KL2x) && KINETIS_SERIAL_USE_UART0
  if(sdp == &SD1) {
    if(s1 & UARTx_S1_IDLE) {
      *(u->s1_p) |= UARTx_S1_IDLE;
    }

    if(s1 & (UARTx_S1_OR | UARTx_S1_NF | UARTx_S1_FE | UARTx_S1_PF)) {
      set_error(sdp, s1);
      *(u->s1_p) |= UARTx_S1_OR | UARTx_S1_NF | UARTx_S1_FE | UARTx_S1_PF;
    }
    return;
  }
#endif /* KL2x && KINETIS_SERIAL_USE_UART0 */

  if(s1 & UARTx_S1_IDLE) {
    (void)*(u->d_p);
  }

  if(s1 & (UARTx_S1_OR | UARTx_S1_NF | UARTx_S1_FE | UARTx_S1_PF)) {
    set_error(sdp, s1);
    (void)*(u->d_p);
  }
}

/**
 * @brief   Common IRQ handler.
 * @note    Tries hard to clear all the pending interrupt sources, we don't
 *          want to go through the whole ISR and have another interrupt soon
 *          after.
 *
 * @param[in] sdp       communication channel associated to the UART
 */
static void serve_interrupt(SerialDriver *sdp) {
  UART_w_TypeDef *u = &(sdp->uart);
  uint8_t s1 = *(u->s1_p);

  if (s1 & UARTx_S1_RDRF) {
    osalSysLockFromISR();
    if (iqIsEmptyI(&sdp->iqueue))
      chnAddFlagsI(sdp, CHN_INPUT_AVAILABLE);
    if (iqPutI(&sdp->iqueue, *(u->d_p)) < Q_OK)
      chnAddFlagsI(sdp, SD_OVERRUN_ERROR);
    osalSysUnlockFromISR();
  }

  if (s1 & UARTx_S1_TDRE) {
    msg_t b;

    osalSysLockFromISR();
    b = oqGetI(&sdp->oqueue);
    osalSysUnlockFromISR();

    if (b < Q_OK) {
      osalSysLockFromISR();
      chnAddFlagsI(sdp, CHN_OUTPUT_EMPTY);
      osalSysUnlockFromISR();
      *(u->c2_p) &= ~UARTx_C2_TIE;
    } else {
       *(u->d_p) = b;
    }
  }

  serve_error_interrupt(sdp);
}

/**
 * @brief   Attempts a TX preload
 */
static void preload(SerialDriver *sdp) {
  UART_w_TypeDef *u = &(sdp->uart);

  if (*(u->s1_p) & UARTx_S1_TDRE) {
    msg_t b = oqGetI(&sdp->oqueue);
    if (b < Q_OK) {
      chnAddFlagsI(sdp, CHN_OUTPUT_EMPTY);
      return;
    }
    *(u->d_p) = b;
    *(u->c2_p) |= UARTx_C2_TIE;
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
static void configure_uart(SerialDriver *sdp, const SerialConfig *config) {

  UART_w_TypeDef *uart = &(sdp->uart);
  uint32_t divisor;

  /* Discard any incoming data. */
  while (*(uart->s1_p) & UARTx_S1_RDRF) {
    (void)*(uart->d_p);
  }

  /* Disable UART while configuring */
  *(uart->c2_p) &= ~(UARTx_C2_RE | UARTx_C2_TE);

  /* The clock sources for various UARTs can be different. */
  divisor=KINETIS_BUSCLK_FREQUENCY;

#if defined(KL2x)

#if KINETIS_SERIAL_USE_UART0
  if (sdp == &SD1) {
    /* UART0 can be clocked from several sources on KL2x. */
    divisor = KINETIS_UART0_CLOCK_FREQ;
    /* FIXME: change fixed OSR = 16 to dynamic value based on baud */
    /* Note: OSR only works on KL2x/UART0; further UARTs have fixed 16. */
    *(uart->c4_p) = UARTx_C4_OSR(16 - 1);
  }
#endif /* KINETIS_SERIAL_USE_UART0 */

#elif defined(K20x) /* KL2x */

  /* UARTs 0 and 1 are clocked from SYSCLK, others from BUSCLK on K20x. */
#if KINETIS_SERIAL_USE_UART0
  if(sdp == &SD1)
    divisor = KINETIS_SYSCLK_FREQUENCY;
#endif /* KINETIS_SERIAL_USE_UART0 */
#if KINETIS_SERIAL_USE_UART1
  if(sdp == &SD2)
    divisor = KINETIS_SYSCLK_FREQUENCY;
#endif /* KINETIS_SERIAL_USE_UART1 */

#else /* K20x */
#error Baud rate selection not implemented for this MCU type
#endif /* K20x */

  divisor = (divisor * 2 + 1) / config->sc_speed;

  *(uart->bdh_p) = UARTx_BDH_SBR(divisor >> 13) | (*(uart->bdh_p) & ~UARTx_BDH_SBR_MASK);
  *(uart->bdl_p) = UARTx_BDL_SBR(divisor >> 5);
#if defined(K20x)
  *(uart->c4_p) = UARTx_C4_BRFA(divisor) | (*(uart->c4_p) & ~UARTx_C4_BRFA_MASK);
#endif /* K20x */

  /* Line settings. */
  *(uart->c1_p) = 0;
  /* Enable error event interrupts (overrun, noise, framing, parity) */
  *(uart->c3_p) = UARTx_C3_ORIE | UARTx_C3_NEIE | UARTx_C3_FEIE | UARTx_C3_PEIE;
  /* Enable the peripheral; including receive interrupts. */
  *(uart->c2_p) |= UARTx_C2_RE | UARTx_C2_RIE | UARTx_C2_TE;
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if KINETIS_SERIAL_USE_UART0 || defined(__DOXYGEN__)
OSAL_IRQ_HANDLER(KINETIS_SERIAL0_IRQ_VECTOR) {
  OSAL_IRQ_PROLOGUE();
  serve_interrupt(&SD1);
  OSAL_IRQ_EPILOGUE();
}
#endif

#if KINETIS_SERIAL_USE_UART1 || defined(__DOXYGEN__)
OSAL_IRQ_HANDLER(KINETIS_SERIAL1_IRQ_VECTOR) {
  OSAL_IRQ_PROLOGUE();
  serve_interrupt(&SD2);
  OSAL_IRQ_EPILOGUE();
}
#endif

#if KINETIS_SERIAL_USE_UART2 || defined(__DOXYGEN__)
OSAL_IRQ_HANDLER(KINETIS_SERIAL2_IRQ_VECTOR) {
  OSAL_IRQ_PROLOGUE();
  serve_interrupt(&SD3);
  OSAL_IRQ_EPILOGUE();
}
#endif

#if KINETIS_HAS_SERIAL_ERROR_IRQ

#if KINETIS_SERIAL_USE_UART0 || defined(__DOXYGEN__)
OSAL_IRQ_HANDLER(KINETIS_SERIAL0_ERROR_IRQ_VECTOR) {
  OSAL_IRQ_PROLOGUE();
  serve_error_interrupt(&SD1);
  OSAL_IRQ_EPILOGUE();
}
#endif

#if KINETIS_SERIAL_USE_UART1 || defined(__DOXYGEN__)
OSAL_IRQ_HANDLER(KINETIS_SERIAL1_ERROR_IRQ_VECTOR) {
  OSAL_IRQ_PROLOGUE();
  serve_error_interrupt(&SD2);
  OSAL_IRQ_EPILOGUE();
}
#endif

#if KINETIS_SERIAL_USE_UART2 || defined(__DOXYGEN__)
OSAL_IRQ_HANDLER(KINETIS_SERIAL2_ERROR_IRQ_VECTOR) {
  OSAL_IRQ_PROLOGUE();
  serve_error_interrupt(&SD3);
  OSAL_IRQ_EPILOGUE();
}
#endif

#endif /* KINETIS_HAS_SERIAL_ERROR_IRQ */

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
#if ! KINETIS_SERIAL0_IS_LPUART
  SD1.uart.bdh_p = &(UART0->BDH);
  SD1.uart.bdl_p = &(UART0->BDL);
  SD1.uart.c1_p =  &(UART0->C1);
  SD1.uart.c2_p =  &(UART0->C2);
  SD1.uart.c3_p =  &(UART0->C3);
  SD1.uart.c4_p =  &(UART0->C4);
  SD1.uart.s1_p =  (volatile uint8_t *)&(UART0->S1);
  SD1.uart.s2_p =  &(UART0->S2);
  SD1.uart.d_p =   &(UART0->D);
#else /* ! KINETIS_SERIAL0_IS_LPUART */
  /* little endian! */
  SD1.uart.bdh_p = ((uint8_t *)&(LPUART0->BAUD)) + 1; /* BDH: BAUD, byte 3 */
  SD1.uart.bdl_p = ((uint8_t *)&(LPUART0->BAUD)) + 0; /* BDL: BAUD, byte 4 */
  SD1.uart.c1_p =  ((uint8_t *)&(LPUART0->CTRL)) + 0; /* C1: CTRL, byte 4 */
  SD1.uart.c2_p =  ((uint8_t *)&(LPUART0->CTRL)) + 2; /* C2: CTRL, byte 2 */
  SD1.uart.c3_p =  ((uint8_t *)&(LPUART0->CTRL)) + 3; /* C3: CTRL, byte 1 */
  SD1.uart.c4_p =  ((uint8_t *)&(LPUART0->BAUD)) + 3; /* C4: BAUD, byte 1 */
  SD1.uart.s1_p =  ((uint8_t *)&(LPUART0->STAT)) + 2; /* S1: STAT, byte 2 */
  SD1.uart.s2_p =  ((uint8_t *)&(LPUART0->STAT)) + 3; /* S2: STAT, byte 1 */
  SD1.uart.d_p =   ((uint8_t *)&(LPUART0->DATA)) + 0; /* D: DATA, byte 4 */
#endif /* ! KINETIS_SERIAL0_IS_LPUART */
#if KINETIS_SERIAL0_IS_UARTLP
  SD1.uart.uartlp_p = UART0;
  SD1.uart.uart_p = NULL;
#elif KINETIS_SERIAL0_IS_LPUART
  SD1.uart.lpuart_p = LPUART0;
  SD1.uart.uart_p = NULL;
#else /* KINETIS_SERIAL0_IS_LPUART */
  SD1.uart.uart_p = UART0;
#endif /* KINETIS_SERIAL0_IS_LPUART */
#endif /* KINETIS_SERIAL_USE_UART0 */

#if KINETIS_SERIAL_USE_UART1
  /* Driver initialization.*/
  sdObjectInit(&SD2, NULL, notify2);
#if ! KINETIS_SERIAL1_IS_LPUART
  SD2.uart.bdh_p = &(UART1->BDH);
  SD2.uart.bdl_p = &(UART1->BDL);
  SD2.uart.c1_p =  &(UART1->C1);
  SD2.uart.c2_p =  &(UART1->C2);
  SD2.uart.c3_p =  &(UART1->C3);
  SD2.uart.c4_p =  &(UART1->C4);
  SD2.uart.s1_p =  (volatile uint8_t *)&(UART1->S1);
  SD2.uart.s2_p =  &(UART1->S2);
  SD2.uart.d_p =   &(UART1->D);
  SD2.uart.uart_p = UART1;
#else /* ! KINETIS_SERIAL1_IS_LPUART */
  /* little endian! */
  SD2.uart.bdh_p = ((uint8_t *)&(LPUART1->BAUD)) + 1; /* BDH: BAUD, byte 3 */
  SD2.uart.bdl_p = ((uint8_t *)&(LPUART1->BAUD)) + 0; /* BDL: BAUD, byte 4 */
  SD2.uart.c1_p =  ((uint8_t *)&(LPUART1->CTRL)) + 0; /* C1: CTRL, byte 4 */
  SD2.uart.c2_p =  ((uint8_t *)&(LPUART1->CTRL)) + 2; /* C2: CTRL, byte 2 */
  SD2.uart.c3_p =  ((uint8_t *)&(LPUART1->CTRL)) + 3; /* C3: CTRL, byte 1 */
  SD2.uart.c4_p =  ((uint8_t *)&(LPUART1->BAUD)) + 3; /* C4: BAUD, byte 1 */
  SD2.uart.s1_p =  ((uint8_t *)&(LPUART1->STAT)) + 2; /* S1: STAT, byte 2 */
  SD2.uart.s2_p =  ((uint8_t *)&(LPUART1->STAT)) + 3; /* S2: STAT, byte 1 */
  SD2.uart.d_p =   ((uint8_t *)&(LPUART1->DATA)) + 0; /* D: DATA, byte 4 */
  SD2.uart.lpuart_p = LPUART1;
  SD2.uart.uart_p = NULL;
#endif /* ! KINETIS_SERIAL1_IS_LPUART */
#endif /* KINETIS_SERIAL_USE_UART1 */

#if KINETIS_SERIAL_USE_UART2
  /* Driver initialization.*/
  sdObjectInit(&SD3, NULL, notify3);
  SD3.uart.bdh_p = &(UART2->BDH);
  SD3.uart.bdl_p = &(UART2->BDL);
  SD3.uart.c1_p =  &(UART2->C1);
  SD3.uart.c2_p =  &(UART2->C2);
  SD3.uart.c3_p =  &(UART2->C3);
  SD3.uart.c4_p =  &(UART2->C4);
  SD3.uart.s1_p =  (volatile uint8_t *)&(UART2->S1);
  SD3.uart.s2_p =  &(UART2->S2);
  SD3.uart.d_p =   &(UART2->D);
  SD3.uart.uart_p = UART2;
#endif /* KINETIS_SERIAL_USE_UART2 */
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
#if KINETIS_SERIAL0_IS_LPUART
      SIM->SCGC5 |= SIM_SCGC5_LPUART0;
      SIM->SOPT2 =
              (SIM->SOPT2 & ~SIM_SOPT2_LPUART0SRC_MASK) |
              SIM_SOPT2_LPUART0SRC(KINETIS_UART0_CLOCK_SRC);
#else /* KINETIS_SERIAL0_IS_LPUART */
      SIM->SCGC4 |= SIM_SCGC4_UART0;
#endif /* KINETIS_SERIAL0_IS_LPUART */
#if KINETIS_SERIAL0_IS_UARTLP
      SIM->SOPT2 =
              (SIM->SOPT2 & ~SIM_SOPT2_UART0SRC_MASK) |
              SIM_SOPT2_UART0SRC(KINETIS_UART0_CLOCK_SRC);
#endif /* KINETIS_SERIAL0_IS_UARTLP */
      configure_uart(sdp, config);
#if KINETIS_HAS_SERIAL_ERROR_IRQ
      nvicEnableVector(UART0Status_IRQn, KINETIS_SERIAL_UART0_PRIORITY);
      nvicEnableVector(UART0Error_IRQn, KINETIS_SERIAL_UART0_PRIORITY);
#else /* KINETIS_HAS_SERIAL_ERROR_IRQ */
#if KINETIS_SERIAL0_IS_LPUART
      nvicEnableVector(LPUART0_IRQn, KINETIS_SERIAL_UART0_PRIORITY);
#else /* KINETIS_SERIAL0_IS_LPUART */
      nvicEnableVector(UART0_IRQn, KINETIS_SERIAL_UART0_PRIORITY);
#endif /* KINETIS_SERIAL0_IS_LPUART */
#endif /* KINETIS_HAS_SERIAL_ERROR_IRQ */
    }
#endif /* KINETIS_SERIAL_USE_UART0 */

#if KINETIS_SERIAL_USE_UART1
    if (sdp == &SD2) {
#if KINETIS_SERIAL1_IS_LPUART
      SIM->SCGC5 |= SIM_SCGC5_LPUART1;
      SIM->SOPT2 =
              (SIM->SOPT2 & ~SIM_SOPT2_LPUART1SRC_MASK) |
              SIM_SOPT2_LPUART1SRC(KINETIS_UART1_CLOCK_SRC);
#else /* KINETIS_SERIAL1_IS_LPUART */
      SIM->SCGC4 |= SIM_SCGC4_UART1;
#endif /* KINETIS_SERIAL1_IS_LPUART */
      configure_uart(sdp, config);
#if KINETIS_HAS_SERIAL_ERROR_IRQ
      nvicEnableVector(UART1Status_IRQn, KINETIS_SERIAL_UART1_PRIORITY);
      nvicEnableVector(UART1Error_IRQn, KINETIS_SERIAL_UART0_PRIORITY);
#else /* KINETIS_HAS_SERIAL_ERROR_IRQ */
#if KINETIS_SERIAL1_IS_LPUART
      nvicEnableVector(LPUART1_IRQn, KINETIS_SERIAL_UART1_PRIORITY);
#else /* KINETIS_SERIAL1_IS_LPUART */
      nvicEnableVector(UART1_IRQn, KINETIS_SERIAL_UART1_PRIORITY);
#endif /* KINETIS_SERIAL1_IS_LPUART */
#endif /* KINETIS_HAS_SERIAL_ERROR_IRQ */
    }
#endif /* KINETIS_SERIAL_USE_UART1 */

#if KINETIS_SERIAL_USE_UART2
    if (sdp == &SD3) {
      SIM->SCGC4 |= SIM_SCGC4_UART2;
      configure_uart(sdp, config);
#if KINETIS_HAS_SERIAL_ERROR_IRQ
      nvicEnableVector(UART2Status_IRQn, KINETIS_SERIAL_UART2_PRIORITY);
      nvicEnableVector(UART2Error_IRQn, KINETIS_SERIAL_UART0_PRIORITY);
#else /* KINETIS_HAS_SERIAL_ERROR_IRQ */
      nvicEnableVector(UART2_IRQn, KINETIS_SERIAL_UART2_PRIORITY);
#endif /* KINETIS_HAS_SERIAL_ERROR_IRQ */
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
#if KINETIS_HAS_SERIAL_ERROR_IRQ
      nvicDisableVector(UART0Status_IRQn);
      nvicDisableVector(UART0Error_IRQn);
#else /* KINETIS_HAS_SERIAL_ERROR_IRQ */
#if KINETIS_SERIAL0_IS_LPUART
      nvicDisableVector(LPUART0_IRQn);
#else /* KINETIS_SERIAL0_IS_LPUART */
      nvicDisableVector(UART0_IRQn);
#endif /* KINETIS_SERIAL0_IS_LPUART */
#endif /* KINETIS_HAS_SERIAL_ERROR_IRQ */
#if KINETIS_SERIAL0_IS_LPUART
      SIM->SCGC5 &= ~SIM_SCGC5_LPUART0;
#else /* KINETIS_SERIAL0_IS_LPUART */
      SIM->SCGC4 &= ~SIM_SCGC4_UART0;
#endif /* KINETIS_SERIAL0_IS_LPUART */
    }
#endif

#if KINETIS_SERIAL_USE_UART1
    if (sdp == &SD2) {
#if KINETIS_HAS_SERIAL_ERROR_IRQ
      nvicDisableVector(UART1Status_IRQn);
      nvicDisableVector(UART1Error_IRQn);
#else /* KINETIS_HAS_SERIAL_ERROR_IRQ */
#if KINETIS_SERIAL1_IS_LPUART
      nvicDisableVector(LPUART1_IRQn);
#else /* KINETIS_SERIAL1_IS_LPUART */
      nvicDisableVector(UART1_IRQn);
#endif /* KINETIS_SERIAL1_IS_LPUART */
#endif /* KINETIS_HAS_SERIAL_ERROR_IRQ */
#if KINETIS_SERIAL1_IS_LPUART
      SIM->SCGC5 &= ~SIM_SCGC5_LPUART1;
#else /* KINETIS_SERIAL1_IS_LPUART */
      SIM->SCGC4 &= ~SIM_SCGC4_UART1;
#endif /* KINETIS_SERIAL1_IS_LPUART */
    }
#endif

#if KINETIS_SERIAL_USE_UART2
    if (sdp == &SD3) {
#if KINETIS_HAS_SERIAL_ERROR_IRQ
      nvicDisableVector(UART2Status_IRQn);
      nvicDisableVector(UART2Error_IRQn);
#else /* KINETIS_HAS_SERIAL_ERROR_IRQ */
      nvicDisableVector(UART2_IRQn);
#endif /* KINETIS_HAS_SERIAL_ERROR_IRQ */
      SIM->SCGC4 &= ~SIM_SCGC4_UART2;
    }
#endif
  }
}

#endif /* HAL_USE_SERIAL */

/** @} */
