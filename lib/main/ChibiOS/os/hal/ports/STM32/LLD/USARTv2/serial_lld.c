/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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
 * @file    STM32/USARTv2/serial_lld.c
 * @brief   STM32 low level serial driver code.
 *
 * @addtogroup SERIAL
 * @{
 */

#include "hal.h"

#if HAL_USE_SERIAL || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/* For compatibility for those devices without LIN support in the USARTs.*/
#if !defined(USART_ISR_LBDF)
#define USART_ISR_LBDF                      0
#endif

#if !defined(USART_CR2_LBDIE)
#define USART_CR2_LBDIE                     0
#endif

/* STM32L0xx/STM32F7xx ST headers difference.*/
#if !defined(USART_ISR_LBDF)
#define USART_ISR_LBDF                      USART_ISR_LBD
#endif

/* Handling the case where UART4 and UART5 are actually USARTs, this happens
   in the STM32F0xx.*/
#if defined(USART4)
#define UART4                               USART4
#endif

#if defined(USART5)
#define UART5                               USART5
#endif

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief USART1 serial driver identifier.*/
#if STM32_SERIAL_USE_USART1 || defined(__DOXYGEN__)
SerialDriver SD1;
#endif

/** @brief USART2 serial driver identifier.*/
#if STM32_SERIAL_USE_USART2 || defined(__DOXYGEN__)
SerialDriver SD2;
#endif

/** @brief USART3 serial driver identifier.*/
#if STM32_SERIAL_USE_USART3 || defined(__DOXYGEN__)
SerialDriver SD3;
#endif

/** @brief UART4 serial driver identifier.*/
#if STM32_SERIAL_USE_UART4 || defined(__DOXYGEN__)
SerialDriver SD4;
#endif

/** @brief UART5 serial driver identifier.*/
#if STM32_SERIAL_USE_UART5 || defined(__DOXYGEN__)
SerialDriver SD5;
#endif

/** @brief USART6 serial driver identifier.*/
#if STM32_SERIAL_USE_USART6 || defined(__DOXYGEN__)
SerialDriver SD6;
#endif

/** @brief UART7 serial driver identifier.*/
#if STM32_SERIAL_USE_UART7 || defined(__DOXYGEN__)
SerialDriver SD7;
#endif

/** @brief UART8 serial driver identifier.*/
#if STM32_SERIAL_USE_UART8 || defined(__DOXYGEN__)
SerialDriver SD8;
#endif

/** @brief LPUART1 serial driver identifier.*/
#if STM32_SERIAL_USE_LPUART1 || defined(__DOXYGEN__)
SerialDriver LPSD1;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/** @brief Driver default configuration.*/
static const SerialConfig default_config =
{
  SERIAL_DEFAULT_BITRATE,
  0,
  USART_CR2_STOP1_BITS | USART_CR2_LINEN,
  0
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   USART initialization.
 * @details This function must be invoked with interrupts disabled.
 *
 * @param[in] sdp       pointer to a @p SerialDriver object
 * @param[in] config    the architecture-dependent serial driver configuration
 */
static void usart_init(SerialDriver *sdp, const SerialConfig *config) {
  USART_TypeDef *u = sdp->usart;

  /* Baud rate setting.*/
#if STM32_SERIAL_USE_LPUART1
  if ( sdp == &LPSD1 )
  {
      u->BRR = (uint32_t)( ( (uint64_t)sdp->clock * 256 ) / config->speed);
  }
  else
#endif
  u->BRR = (uint32_t)(sdp->clock / config->speed);

  /* Note that some bits are enforced.*/
  u->CR2 = config->cr2 | USART_CR2_LBDIE;
  u->CR3 = config->cr3 | USART_CR3_EIE;
  u->CR1 = config->cr1 | USART_CR1_UE | USART_CR1_PEIE |
                         USART_CR1_RXNEIE | USART_CR1_TE |
                         USART_CR1_RE;
  u->ICR = 0xFFFFFFFFU;
}

/**
 * @brief   USART de-initialization.
 * @details This function must be invoked with interrupts disabled.
 *
 * @param[in] u         pointer to an USART I/O block
 */
static void usart_deinit(USART_TypeDef *u) {

  u->CR1 = 0;
  u->CR2 = 0;
  u->CR3 = 0;
}

/**
 * @brief   Error handling routine.
 *
 * @param[in] sdp       pointer to a @p SerialDriver object
 * @param[in] isr       USART ISR register value
 */
static void set_error(SerialDriver *sdp, uint32_t isr) {
  eventflags_t sts = 0;

  if (isr & USART_ISR_ORE)
    sts |= SD_OVERRUN_ERROR;
  if (isr & USART_ISR_PE)
    sts |= SD_PARITY_ERROR;
  if (isr & USART_ISR_FE)
    sts |= SD_FRAMING_ERROR;
  if (isr & USART_ISR_NE)
    sts |= SD_NOISE_ERROR;
  osalSysLockFromISR();
  chnAddFlagsI(sdp, sts);
  osalSysUnlockFromISR();
}

/**
 * @brief   Common IRQ handler.
 *
 * @param[in] sdp       communication channel associated to the USART
 */
static void serve_interrupt(SerialDriver *sdp) {
  USART_TypeDef *u = sdp->usart;
  uint32_t cr1 = u->CR1;
  uint32_t isr;

  /* Reading and clearing status.*/
  isr = u->ISR;
  u->ICR = isr;

  /* Error condition detection.*/
  if (isr & (USART_ISR_ORE | USART_ISR_NE | USART_ISR_FE  | USART_ISR_PE))
    set_error(sdp, isr);

  /* Special case, LIN break detection.*/
  if (isr & USART_ISR_LBDF) {
    osalSysLockFromISR();
    chnAddFlagsI(sdp, SD_BREAK_DETECTED);
    osalSysUnlockFromISR();
  }

  /* Data available.*/
  if (isr & USART_ISR_RXNE) {
    osalSysLockFromISR();
    sdIncomingDataI(sdp, (uint8_t)u->RDR);
    osalSysUnlockFromISR();
  }

  /* Transmission buffer empty.*/
  if ((cr1 & USART_CR1_TXEIE) && (isr & USART_ISR_TXE)) {
    msg_t b;
    osalSysLockFromISR();
    b = oqGetI(&sdp->oqueue);
    if (b < Q_OK) {
      chnAddFlagsI(sdp, CHN_OUTPUT_EMPTY);
      u->CR1 = (cr1 & ~USART_CR1_TXEIE) | USART_CR1_TCIE;
    }
    else
      u->TDR = b;
    osalSysUnlockFromISR();
  }

  /* Physical transmission end.*/
  if (isr & USART_ISR_TC) {
    osalSysLockFromISR();
    if (oqIsEmptyI(&sdp->oqueue))
      chnAddFlagsI(sdp, CHN_TRANSMISSION_END);
    u->CR1 = cr1 & ~USART_CR1_TCIE;
    osalSysUnlockFromISR();
  }
}

#if STM32_SERIAL_USE_USART1 || defined(__DOXYGEN__)
static void notify1(io_queue_t *qp) {

  (void)qp;
  USART1->CR1 |= USART_CR1_TXEIE;
}
#endif

#if STM32_SERIAL_USE_USART2 || defined(__DOXYGEN__)
static void notify2(io_queue_t *qp) {

  (void)qp;
  USART2->CR1 |= USART_CR1_TXEIE;
}
#endif

#if STM32_SERIAL_USE_USART3 || defined(__DOXYGEN__)
static void notify3(io_queue_t *qp) {

  (void)qp;
  USART3->CR1 |= USART_CR1_TXEIE;
}
#endif

#if STM32_SERIAL_USE_UART4 || defined(__DOXYGEN__)
static void notify4(io_queue_t *qp) {

  (void)qp;
  UART4->CR1 |= USART_CR1_TXEIE;
}
#endif

#if STM32_SERIAL_USE_UART5 || defined(__DOXYGEN__)
static void notify5(io_queue_t *qp) {

  (void)qp;
  UART5->CR1 |= USART_CR1_TXEIE;
}
#endif

#if STM32_SERIAL_USE_USART6 || defined(__DOXYGEN__)
static void notify6(io_queue_t *qp) {

  (void)qp;
  USART6->CR1 |= USART_CR1_TXEIE;
}
#endif

#if STM32_SERIAL_USE_UART7 || defined(__DOXYGEN__)
static void notify7(io_queue_t *qp) {

  (void)qp;
  UART7->CR1 |= USART_CR1_TXEIE;
}
#endif

#if STM32_SERIAL_USE_UART8 || defined(__DOXYGEN__)
static void notify8(io_queue_t *qp) {

  (void)qp;
  UART8->CR1 |= USART_CR1_TXEIE;
}
#endif

#if STM32_SERIAL_USE_LPUART1 || defined(__DOXYGEN__)
static void notifylp1(io_queue_t *qp) {

  (void)qp;
  LPUART1->CR1 |= USART_CR1_TXEIE;
}
#endif

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if STM32_SERIAL_USE_USART1 || defined(__DOXYGEN__)
#if !defined(STM32_USART1_HANDLER)
#error "STM32_USART1_HANDLER not defined"
#endif
/**
 * @brief   USART1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_USART1_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  serve_interrupt(&SD1);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if STM32_SERIAL_USE_USART2 || defined(__DOXYGEN__)
#if !defined(STM32_USART2_HANDLER)
#error "STM32_USART2_HANDLER not defined"
#endif
/**
 * @brief   USART2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_USART2_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  serve_interrupt(&SD2);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if defined(STM32_USART3_8_HANDLER)
#if STM32_SERIAL_USE_USART3 || STM32_SERIAL_USE_UART4  ||                   \
    STM32_SERIAL_USE_UART5  || STM32_SERIAL_USE_USART6 ||                   \
    STM32_SERIAL_USE_UART7  || STM32_SERIAL_USE_UART8  || defined(__DOXYGEN__)
/**
 * @brief   USART2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_USART3_8_HANDLER) {

  OSAL_IRQ_PROLOGUE();

#if STM32_SERIAL_USE_USART3
  serve_interrupt(&SD3);
#endif
#if STM32_SERIAL_USE_UART4
  serve_interrupt(&SD4);
#endif
#if STM32_SERIAL_USE_UART5
  serve_interrupt(&SD5);
#endif
#if STM32_SERIAL_USE_USART6
  serve_interrupt(&SD6);
#endif

  OSAL_IRQ_EPILOGUE();
}
#endif

#else /* !defined(STM32_USART3_8_HANDLER) */

#if STM32_SERIAL_USE_USART3 || defined(__DOXYGEN__)
#if !defined(STM32_USART3_HANDLER)
#error "STM32_USART3_HANDLER not defined"
#endif
/**
 * @brief   USART3 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_USART3_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  serve_interrupt(&SD3);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if STM32_SERIAL_USE_UART4 || defined(__DOXYGEN__)
#if !defined(STM32_UART4_HANDLER)
#error "STM32_UART4_HANDLER not defined"
#endif
/**
 * @brief   UART4 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_UART4_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  serve_interrupt(&SD4);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if STM32_SERIAL_USE_UART5 || defined(__DOXYGEN__)
#if !defined(STM32_UART5_HANDLER)
#error "STM32_UART5_HANDLER not defined"
#endif
/**
 * @brief   UART5 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_UART5_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  serve_interrupt(&SD5);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if STM32_SERIAL_USE_USART6 || defined(__DOXYGEN__)
#if !defined(STM32_USART6_HANDLER)
#error "STM32_USART6_HANDLER not defined"
#endif
/**
 * @brief   USART6 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_USART6_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  serve_interrupt(&SD6);

  OSAL_IRQ_EPILOGUE();
}
#endif

#endif /* !defined(STM32_USART3_8_HANDLER) */

#if STM32_SERIAL_USE_UART7 || defined(__DOXYGEN__)
#if !defined(STM32_UART7_HANDLER)
#error "STM32_UART7_HANDLER not defined"
#endif
/**
 * @brief   UART7 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_UART7_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  serve_interrupt(&SD7);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if STM32_SERIAL_USE_UART8 || defined(__DOXYGEN__)
#if !defined(STM32_UART8_HANDLER)
#error "STM32_UART8_HANDLER not defined"
#endif
/**
 * @brief   UART8 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_UART8_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  serve_interrupt(&SD8);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if STM32_SERIAL_USE_LPUART1 || defined(__DOXYGEN__)
#if !defined(STM32_LPUART1_HANDLER)
#error "STM32_LPUART1_HANDLER not defined"
#endif
/**
 * @brief   LPUART1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_LPUART1_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  serve_interrupt(&LPSD1);

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

#if STM32_SERIAL_USE_USART1
  sdObjectInit(&SD1, NULL, notify1);
  SD1.usart = USART1;
  SD1.clock = STM32_USART1CLK;
#if defined(STM32_USART1_NUMBER)
  nvicEnableVector(STM32_USART1_NUMBER, STM32_SERIAL_USART1_PRIORITY);
#endif
#endif

#if STM32_SERIAL_USE_USART2
  sdObjectInit(&SD2, NULL, notify2);
  SD2.usart = USART2;
  SD2.clock = STM32_USART2CLK;
#if defined(STM32_USART2_NUMBER)
  nvicEnableVector(STM32_USART2_NUMBER, STM32_SERIAL_USART2_PRIORITY);
#endif
#endif

#if STM32_SERIAL_USE_USART3
  sdObjectInit(&SD3, NULL, notify3);
  SD3.usart = USART3;
  SD3.clock = STM32_USART3CLK;
#if defined(STM32_USART3_NUMBER)
  nvicEnableVector(STM32_USART3_NUMBER, STM32_SERIAL_USART3_PRIORITY);
#endif
#endif

#if STM32_SERIAL_USE_UART4
  sdObjectInit(&SD4, NULL, notify4);
  SD4.usart = UART4;
  SD4.clock = STM32_UART4CLK;
#if defined(STM32_UART4_NUMBER)
  nvicEnableVector(STM32_UART4_NUMBER, STM32_SERIAL_UART4_PRIORITY);
#endif
#endif

#if STM32_SERIAL_USE_UART5
  sdObjectInit(&SD5, NULL, notify5);
  SD5.usart = UART5;
  SD5.clock = STM32_UART5CLK;
#if defined(STM32_UART5_NUMBER)
  nvicEnableVector(STM32_UART5_NUMBER, STM32_SERIAL_UART5_PRIORITY);
#endif
#endif

#if STM32_SERIAL_USE_USART6
  sdObjectInit(&SD6, NULL, notify6);
  SD6.usart = USART6;
  SD6.clock = STM32_USART6CLK;
#if defined(STM32_USART6_NUMBER)
  nvicEnableVector(STM32_USART6_NUMBER, STM32_SERIAL_USART6_PRIORITY);
#endif
#endif

#if STM32_SERIAL_USE_UART7
  sdObjectInit(&SD7, NULL, notify7);
  SD7.usart = UART7;
  SD7.clock = STM32_UART7CLK;
#if defined(STM32_UART7_NUMBER)
  nvicEnableVector(STM32_UART7_NUMBER, STM32_SERIAL_UART7_PRIORITY);
#endif
#endif

#if STM32_SERIAL_USE_UART8
  sdObjectInit(&SD8, NULL, notify8);
  SD8.usart = UART8;
  SD8.clock = STM32_UART8CLK;
#if defined(STM32_UART8_NUMBER)
  nvicEnableVector(STM32_UART8_NUMBER, STM32_SERIAL_UART8_PRIORITY);
#endif
#endif

#if STM32_SERIAL_USE_LPUART1
  sdObjectInit(&LPSD1, NULL, notifylp1);
  LPSD1.usart = LPUART1;
  LPSD1.clock = STM32_LPUART1CLK;
#if defined(STM32_LPUART1_NUMBER)
  nvicEnableVector(STM32_LPUART1_NUMBER, STM32_SERIAL_LPUART1_PRIORITY);
#endif
#endif

#if STM32_SERIAL_USE_USART3 || STM32_SERIAL_USE_UART4  ||                   \
    STM32_SERIAL_USE_UART5  || STM32_SERIAL_USE_USART6 ||                   \
    STM32_SERIAL_USE_UART7  ||  STM32_SERIAL_USE_UART8 || defined(__DOXYGEN__)
#if defined(STM32_USART3_8_HANDLER)
  nvicEnableVector(STM32_USART3_8_NUMBER, STM32_SERIAL_USART3_8_PRIORITY);
#endif
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
#if STM32_SERIAL_USE_USART1
    if (&SD1 == sdp) {
      rccEnableUSART1(FALSE);
    }
#endif
#if STM32_SERIAL_USE_USART2
    if (&SD2 == sdp) {
      rccEnableUSART2(FALSE);
    }
#endif
#if STM32_SERIAL_USE_USART3
    if (&SD3 == sdp) {
      rccEnableUSART3(FALSE);
    }
#endif
#if STM32_SERIAL_USE_UART4
    if (&SD4 == sdp) {
      rccEnableUART4(FALSE);
    }
#endif
#if STM32_SERIAL_USE_UART5
    if (&SD5 == sdp) {
      rccEnableUART5(FALSE);
    }
#endif
#if STM32_SERIAL_USE_USART6
    if (&SD6 == sdp) {
      rccEnableUSART6(FALSE);
    }
#endif
#if STM32_SERIAL_USE_UART7
    if (&SD7 == sdp) {
      rccEnableUART7(FALSE);
    }
#endif
#if STM32_SERIAL_USE_UART8
    if (&SD8 == sdp) {
      rccEnableUART8(FALSE);
    }
#endif
#if STM32_SERIAL_USE_LPUART1
    if (&LPSD1 == sdp) {
      rccEnableLPUART1(FALSE);
    }
#endif
  }
  usart_init(sdp, config);
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
    /* UART is de-initialized then clocks are disabled.*/
    usart_deinit(sdp->usart);

#if STM32_SERIAL_USE_USART1
    if (&SD1 == sdp) {
      rccDisableUSART1(FALSE);
      return;
    }
#endif
#if STM32_SERIAL_USE_USART2
    if (&SD2 == sdp) {
      rccDisableUSART2(FALSE);
      return;
    }
#endif
#if STM32_SERIAL_USE_USART3
    if (&SD3 == sdp) {
      rccDisableUSART3(FALSE);
      return;
    }
#endif
#if STM32_SERIAL_USE_UART4
    if (&SD4 == sdp) {
      rccDisableUART4(FALSE);
      return;
    }
#endif
#if STM32_SERIAL_USE_UART5
    if (&SD5 == sdp) {
      rccDisableUART5(FALSE);
      return;
    }
#endif
#if STM32_SERIAL_USE_USART6
    if (&SD6 == sdp) {
      rccDisableUSART6(FALSE);
      return;
    }
#endif
#if STM32_SERIAL_USE_UART7
    if (&SD7 == sdp) {
      rccDisableUART7(FALSE);
      return;
    }
#endif
#if STM32_SERIAL_USE_UART8
    if (&SD8 == sdp) {
      rccDisableUART8(FALSE);
      return;
    }
#endif
#if STM32_SERIAL_USE_LPUART1
    if (&LPSD1 == sdp) {
      rccDisableLPUART1(FALSE);
      return;
    }
#endif
  }
}

#endif /* HAL_USE_SERIAL */

/** @} */
