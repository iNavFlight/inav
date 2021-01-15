/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

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
 * @file    USARTv1/hal_serial_lld.c
 * @brief   SAMA low level serial driver code.
 *
 * @addtogroup SERIAL
 * @{
 */

#include "hal.h"

#if HAL_USE_SERIAL || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local macros.                                                      */
/*===========================================================================*/

#if SAMA_SERIAL_USE_UART
/**
 * @brief   Enable write protection on SD registers block.
 *
 * @param[in] sdp    pointer to a SD register block
 *
 * @notapi
 */
#define sdEnableWP(sdp) {                                                    \
  sdp->UART_WPMR = UART_WPMR_WPKEY_PASSWD | UART_WPMR_WPEN;                  \
}

/**
 * @brief   Disable write protection on SD registers block.
 *
 * @param[in] sdp    pointer to a SD register block
 *
 * @notapi
 */
#define sdDisableWP(sdp) {                                                   \
  sdp->UART_WPMR = UART_WPMR_WPKEY_PASSWD;                                   \
}
#endif

#if SAMA_SERIAL_USE_FLEXCOM
/**
 * @brief   Enable write protection on SD registers block.
 *
 * @param[in] sdp    pointer to a SD register block
 *
 * @notapi
 */
#define sdFlexEnableWP(sdp) {                                                    \
  sdp->US_WPMR = US_WPMR_WPKEY_PASSWD | US_WPMR_WPEN;                            \
}

/**
 * @brief   Disable write protection on SD registers block.
 *
 * @param[in] sdp    pointer to a SD register block
 *
 * @notapi
 */
#define sdFlexDisableWP(sdp) {                                                   \
  sdp->US_WPMR = US_WPMR_WPKEY_PASSWD;                                           \
}
#endif

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief UART0 serial driver identifier.*/
#if SAMA_SERIAL_USE_UART0 || defined(__DOXYGEN__)
SerialDriver SD0;
#endif

/** @brief UART1 serial driver identifier.*/
#if SAMA_SERIAL_USE_UART1 || defined(__DOXYGEN__)
SerialDriver SD1;
#endif

/** @brief UART2 serial driver identifier.*/
#if SAMA_SERIAL_USE_UART2 || defined(__DOXYGEN__)
SerialDriver SD2;
#endif

/** @brief UART3 serial driver identifier.*/
#if SAMA_SERIAL_USE_UART3 || defined(__DOXYGEN__)
SerialDriver SD3;
#endif

/** @brief UART4 serial driver identifier.*/
#if SAMA_SERIAL_USE_UART4 || defined(__DOXYGEN__)
SerialDriver SD4;
#endif

/** @brief FLEXCOM0 serial driver identifier.*/
#if SAMA_SERIAL_USE_FLEXCOM0 || defined(__DOXYGEN__)
SerialDriver FSD0;
#endif

/** @brief FLEXCOM1 serial driver identifier.*/
#if SAMA_SERIAL_USE_FLEXCOM1 || defined(__DOXYGEN__)
SerialDriver FSD1;
#endif

/** @brief FLEXCOM2 serial driver identifier.*/
#if SAMA_SERIAL_USE_FLEXCOM2 || defined(__DOXYGEN__)
SerialDriver FSD2;
#endif

/** @brief FLEXCOM3 serial driver identifier.*/
#if SAMA_SERIAL_USE_FLEXCOM3 || defined(__DOXYGEN__)
SerialDriver FSD3;
#endif

/** @brief FLEXCOM0 serial driver identifier.*/
#if SAMA_SERIAL_USE_FLEXCOM4 || defined(__DOXYGEN__)
SerialDriver FSD4;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/** @brief Driver default configuration.*/
static const SerialConfig default_config =
{
  SERIAL_DEFAULT_BITRATE,
  0,
#if SAMA_SERIAL_USE_FLEXCOM
  US_MR_CHRL_8_BIT | US_MR_PAR_NO
#else
  UART_MR_PAR_NO
#endif
};

#if SAMA_SERIAL_USE_UART0 || defined(__DOXYGEN__)
/** @brief Input buffer for SD0.*/
static uint8_t sd_in_buf0[SAMA_SERIAL_UART0_IN_BUF_SIZE];

/** @brief Output buffer for SD0.*/
static uint8_t sd_out_buf0[SAMA_SERIAL_UART0_OUT_BUF_SIZE];
#endif

#if SAMA_SERIAL_USE_UART1 || defined(__DOXYGEN__)
/** @brief Input buffer for SD1.*/
static uint8_t sd_in_buf1[SAMA_SERIAL_UART1_IN_BUF_SIZE];

/** @brief Output buffer for SD1.*/
static uint8_t sd_out_buf1[SAMA_SERIAL_UART1_OUT_BUF_SIZE];
#endif

#if SAMA_SERIAL_USE_UART2 || defined(__DOXYGEN__)
/** @brief Input buffer for SD2.*/
static uint8_t sd_in_buf2[SAMA_SERIAL_UART2_IN_BUF_SIZE];

/** @brief Output buffer for SD2.*/
static uint8_t sd_out_buf2[SAMA_SERIAL_UART2_OUT_BUF_SIZE];
#endif

#if SAMA_SERIAL_USE_UART3 || defined(__DOXYGEN__)
/** @brief Input buffer for SD3.*/
static uint8_t sd_in_buf3[SAMA_SERIAL_UART3_IN_BUF_SIZE];

/** @brief Output buffer for SD3.*/
static uint8_t sd_out_buf3[SAMA_SERIAL_UART3_IN_BUF_SIZE];
#endif

#if SAMA_SERIAL_USE_UART4 || defined(__DOXYGEN__)
/** @brief Input buffer for SD4.*/
static uint8_t sd_in_buf4[SAMA_SERIAL_UART4_IN_BUF_SIZE];

/** @brief Output buffer for SD4.*/
static uint8_t sd_out_buf4[SAMA_SERIAL_UART4_IN_BUF_SIZE];
#endif

#if SAMA_SERIAL_USE_FLEXCOM0 || defined(__DOXYGEN__)
/** @brief Input buffer for FSD0.*/
static uint8_t sdFlex_in_buf0[SAMA_SERIAL_FLEXCOM0_IN_BUF_SIZE];

/** @brief Output buffer for FSD0.*/
static uint8_t sdFlex_out_buf0[SAMA_SERIAL_FLEXCOM0_OUT_BUF_SIZE];
#endif

#if SAMA_SERIAL_USE_FLEXCOM1 || defined(__DOXYGEN__)
/** @brief Input buffer for FSD1.*/
static uint8_t sdFlex_in_buf1[SAMA_SERIAL_FLEXCOM1_IN_BUF_SIZE];

/** @brief Output buffer for FSD1.*/
static uint8_t sdFlex_out_buf1[SAMA_SERIAL_FLEXCOM1_OUT_BUF_SIZE];
#endif

#if SAMA_SERIAL_USE_FLEXCOM2 || defined(__DOXYGEN__)
/** @brief Input buffer for FSD2.*/
static uint8_t sdFlex_in_buf2[SAMA_SERIAL_FLEXCOM2_IN_BUF_SIZE];

/** @brief Output buffer for FSD2.*/
static uint8_t sdFlex_out_buf2[SAMA_SERIAL_FLEXCOM2_OUT_BUF_SIZE];
#endif

#if SAMA_SERIAL_USE_FLEXCOM3 || defined(__DOXYGEN__)
/** @brief Input buffer for FSD3.*/
static uint8_t sdFlex_in_buf3[SAMA_SERIAL_FLEXCOM3_IN_BUF_SIZE];

/** @brief Output buffer for FSD3.*/
static uint8_t sdFlex_out_buf3[SAMA_SERIAL_FLEXCOM3_OUT_BUF_SIZE];
#endif

#if SAMA_SERIAL_USE_FLEXCOM4 || defined(__DOXYGEN__)
/** @brief Input buffer for FSD4.*/
static uint8_t sdFlex_in_buf4[SAMA_SERIAL_FLEXCOM4_IN_BUF_SIZE];

/** @brief Output buffer for FSD4.*/
static uint8_t sdFlex_out_buf4[SAMA_SERIAL_FLEXCOM4_OUT_BUF_SIZE];
#endif

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   UART initialization.
 * @details This function must be invoked with interrupts disabled.
 *
 * @param[in] sdp       pointer to a @p SerialDriver object
 * @param[in] config    the architecture-dependent serial driver configuration
 */
static void uart_init(SerialDriver *sdp, const SerialConfig *config) {
#if SAMA_SERIAL_USE_UART && SAMA_SERIAL_USE_FLEXCOM
  if (sdp->uart != NULL)
#endif /* SAMA_SERIAL_USE_UART && SAMA_SERIAL_USE_FLEXCOM */
#if SAMA_SERIAL_USE_UART
  {
    Uart *u = sdp->uart;

    /* Disabling write protection */
    sdDisableWP(u);
    /* Baud rate setting.*/
    u->UART_BRGR = UART_BRGR_CD(sdp->clock / (16 * config->speed));

    u->UART_CR = config->cr;
    u->UART_MR = config->mr;
    u->UART_IER = UART_IER_RXRDY;

    /* Clearing error status bit */
    u->UART_CR |= UART_CR_RSTSTA;
    /* Enabling Tx and Rx */
    u->UART_CR |= UART_CR_RXEN | UART_CR_TXEN;
    /* Enabling write protection */
    sdEnableWP(u);
  }
#endif /* SAMA_SERIAL_USE_UART */
#if SAMA_SERIAL_USE_UART && SAMA_SERIAL_USE_FLEXCOM
  else if (sdp->usart != NULL)
#endif /* SAMA_SERIAL_USE_UART && SAMA_SERIAL_USE_FLEXCOM */
#if SAMA_SERIAL_USE_FLEXCOM
  {
    Flexcom *fl = sdp->flexcom;
    Usart *us = sdp->usart;

    /* Disabling write protection */
    sdFlexDisableWP(us)
    /* Enabling USART on FLEXCOM */
    fl->FLEX_MR = FLEX_MR_OPMODE_USART;
    /* Baud rate setting (OVER = 0 and SYNC = 0)*/
    us->US_BRGR = US_BRGR_CD(sdp->clock / (16 * config->speed));

    us->US_CR = config->cr;
    us->US_MR = config->mr;
    us->US_IER = US_IER_RXRDY;

    /* Clearing status bit */
    us->US_CR |= US_CR_RSTSTA;
    /* Enabling Tx and Rx */
    us->US_CR |= US_CR_RXEN | US_CR_TXEN;
    /* Enabling write protection */
    sdFlexEnableWP(us)
  }
#endif /* SAMA_SERIAL_USE_FLEXCOM */
#if SAMA_SERIAL_USE_UART && SAMA_SERIAL_USE_FLEXCOM
  else {
    osalDbgAssert(FALSE, "invalid state");
  }
#endif /* SAMA_SERIAL_USE_UART && SAMA_SERIAL_USE_FLEXCOM */
}

/**
 * @brief   USART de-initialization.
 * @details This function must be invoked with interrupts disabled.
 *
 * @param[in] u         pointer to an UART I/O block
 */
static void uart_deinit(SerialDriver *sdp) {
#if SAMA_SERIAL_USE_UART && SAMA_SERIAL_USE_FLEXCOM
  if (sdp->uart != NULL)
#endif /* SAMA_SERIAL_USE_UART && SAMA_SERIAL_USE_FLEXCOM */
#if SAMA_SERIAL_USE_UART
  {
    Uart *u = sdp->uart;
    /* Disabling write protection */
    sdDisableWP(u);
    u->UART_CR = 0;
    u->UART_MR = 0;
    /* Enabling write protection */
    sdEnableWP(u);
  }
#endif /* SAMA_SERIAL_USE_UART */
#if SAMA_SERIAL_USE_UART && SAMA_SERIAL_USE_FLEXCOM
  else if (sdp->usart != NULL)
#endif /* SAMA_SERIAL_USE_UART && SAMA_SERIAL_USE_FLEXCOM */
#if SAMA_SERIAL_USE_FLEXCOM
  {
    Usart *us = sdp->usart;

    /* Disabling write protection */
    sdFlexDisableWP(us)
    us->US_CR = 0;
    us->US_MR = 0;

    /* Enabling write protection */
    sdFlexEnableWP(us)
  }
#endif /* SAMA_SERIAL_USE_FLEXCOM */
#if SAMA_SERIAL_USE_UART && SAMA_SERIAL_USE_FLEXCOM
  else {
    osalDbgAssert(FALSE, "invalid state");
  }
#endif /* SAMA_SERIAL_USE_UART && SAMA_SERIAL_USE_FLEXCOM */
}

/**
 * @brief   Error handling routine.
 *
 * @param[in] sdp       pointer to a @p SerialDriver object
 * @param[in] isr       USART ISR register value
 */
static void set_error(SerialDriver *sdp, uint32_t isr) {
  eventflags_t sts = 0;

  if (isr & (UART_SR_OVRE | US_CSR_OVRE))
    sts |= SD_OVERRUN_ERROR;
  if (isr & (UART_SR_PARE | US_CSR_PARE))
    sts |= SD_PARITY_ERROR;
  if (isr & (UART_SR_FRAME | US_CSR_FRAME))
    sts |= UART_SR_FRAME;
  osalSysLockFromISR();
  chnAddFlagsI(sdp, sts);
  osalSysUnlockFromISR();
}

#if SAMA_SERIAL_USE_UART
/**
 * @brief   Common IRQ handler.
 *
 * @param[in] sdp       communication channel associated to the UART
 */
static void serve_interrupt(SerialDriver *sdp) {
  Uart *u = sdp->uart;
  uint32_t imr = u->UART_IMR;
  uint32_t sr;

  /* Reading and clearing status.*/
  sr = u->UART_SR;
  u->UART_CR |= UART_CR_RSTSTA;

  /* Error condition detection.*/
  if (sr & (UART_SR_OVRE | UART_SR_FRAME  | UART_SR_PARE)){
    set_error(sdp, sr);
  }

  /* Data available.*/
  if (sr & UART_SR_RXRDY) {
    osalSysLockFromISR();
    sdIncomingDataI(sdp, (uint8_t)u->UART_RHR);
    osalSysUnlockFromISR();
  }

  /* Transmission buffer empty.*/
  if ((imr & UART_IMR_TXRDY) && (sr & UART_SR_TXRDY)) {
    msg_t b;
    osalSysLockFromISR();
    b = oqGetI(&sdp->oqueue);
    if (b < MSG_OK) {
      chnAddFlagsI(sdp, CHN_OUTPUT_EMPTY);
      u->UART_IDR |= UART_IDR_TXRDY;
      u->UART_IER = UART_IER_TXEMPTY;
    }
    else
      u->UART_THR = b;
    osalSysUnlockFromISR();
  }

  /* Physical transmission end.*/
  if ((imr & UART_IMR_TXEMPTY) && (sr & (UART_SR_TXRDY | UART_SR_TXEMPTY))) {
    osalSysLockFromISR();
    if (oqIsEmptyI(&sdp->oqueue))
      chnAddFlagsI(sdp, CHN_TRANSMISSION_END);
    u->UART_IDR |= UART_IDR_TXRDY | UART_IDR_TXEMPTY;
    osalSysUnlockFromISR();
  }
}
#endif

#if SAMA_SERIAL_USE_FLEXCOM
/**
 * @brief   Common IRQ handler.
 *
 * @param[in] sdp       communication channel associated to the UART
 */
static void serve_uartFlex_interrupt(SerialDriver *sdp) {
  Usart *us = sdp->usart;
  uint32_t imr = us->US_IMR;
  uint32_t sr;

  /* Reading and clearing status.*/
  sr = us->US_CSR;
  us->US_CR |= US_CR_RSTSTA;

  /* Error condition detection.*/
  if (sr & (US_CSR_OVRE | US_CSR_FRAME  | US_CSR_PARE)){
    set_error(sdp, sr);
  }

  /* Data available.*/
  if (sr & US_CSR_RXRDY) {
    osalSysLockFromISR();
    sdIncomingDataI(sdp, (uint8_t)us->US_RHR);
    osalSysUnlockFromISR();
  }

  /* Transmission buffer empty.*/
  if ((imr & US_IMR_TXRDY) && (sr & US_CSR_TXRDY)) {
    msg_t b;
    osalSysLockFromISR();
    b = oqGetI(&sdp->oqueue);
    if (b < MSG_OK) {
      chnAddFlagsI(sdp, CHN_OUTPUT_EMPTY);
      us->US_IDR |= US_IDR_TXRDY;
      us->US_IER = US_IER_TXEMPTY;
    }
    else
      us->US_THR = b;
    osalSysUnlockFromISR();
  }

  /* Physical transmission end.*/
  if ((imr & US_IMR_TXEMPTY) && (sr & (US_CSR_TXRDY | US_CSR_TXEMPTY))) {
    osalSysLockFromISR();
    if (oqIsEmptyI(&sdp->oqueue))
      chnAddFlagsI(sdp, CHN_TRANSMISSION_END);
    us->US_IDR |= US_IDR_TXRDY | US_IDR_TXEMPTY;
    osalSysUnlockFromISR();
  }
}
#endif

#if SAMA_SERIAL_USE_UART0 || defined(__DOXYGEN__)
static void notify0(io_queue_t *qp) {

  (void)qp;
  UART0->UART_IER |= UART_IER_TXRDY;
}
#endif

#if SAMA_SERIAL_USE_UART1 || defined(__DOXYGEN__)
static void notify1(io_queue_t *qp) {

  (void)qp;
  UART1->UART_IER |= UART_IER_TXRDY;
}
#endif

#if SAMA_SERIAL_USE_UART2 || defined(__DOXYGEN__)
static void notify2(io_queue_t *qp) {

  (void)qp;
  UART2->UART_IER |= UART_IER_TXRDY;
}
#endif

#if SAMA_SERIAL_USE_UART3 || defined(__DOXYGEN__)
static void notify3(io_queue_t *qp) {

  (void)qp;
  UART3->UART_IER |= UART_IER_TXRDY;
}
#endif

#if SAMA_SERIAL_USE_UART4 || defined(__DOXYGEN__)
static void notify4(io_queue_t *qp) {

  (void)qp;
  UART4->UART_IER |= UART_IER_TXRDY;
}
#endif

#if SAMA_SERIAL_USE_FLEXCOM0 || defined(__DOXYGEN__)
static void notifyFlex0(io_queue_t *qp) {

  (void)qp;
  USART0->US_IER |= US_IER_TXRDY;
}
#endif

#if SAMA_SERIAL_USE_FLEXCOM1 || defined(__DOXYGEN__)
static void notifyFlex1(io_queue_t *qp) {

  (void)qp;
  USART1->US_IER |= US_IER_TXRDY;
}
#endif

#if SAMA_SERIAL_USE_FLEXCOM2 || defined(__DOXYGEN__)
static void notifyFlex2(io_queue_t *qp) {

  (void)qp;
  USART2->US_IER |= US_IER_TXRDY;
}
#endif

#if SAMA_SERIAL_USE_FLEXCOM3 || defined(__DOXYGEN__)
static void notifyFlex3(io_queue_t *qp) {

  (void)qp;
  USART3->US_IER |= US_IER_TXRDY;
}
#endif

#if SAMA_SERIAL_USE_FLEXCOM4 || defined(__DOXYGEN__)
static void notifyFlex4(io_queue_t *qp) {

  (void)qp;
  USART4->US_IER |= US_IER_TXRDY;
}
#endif

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if SAMA_SERIAL_USE_UART0 || defined(__DOXYGEN__)
/**
 * @brief   UART0 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SAMA_UART0_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  serve_interrupt(&SD0);
  aicAckInt();
  OSAL_IRQ_EPILOGUE();
}
#endif

#if SAMA_SERIAL_USE_UART1 || defined(__DOXYGEN__)
/**
 * @brief   UART1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SAMA_UART1_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  serve_interrupt(&SD1);
  aicAckInt();
  OSAL_IRQ_EPILOGUE();
}
#endif

#if SAMA_SERIAL_USE_UART2 || defined(__DOXYGEN__)
/**
 * @brief   UART2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SAMA_UART2_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  serve_interrupt(&SD2);
  aicAckInt();
  OSAL_IRQ_EPILOGUE();
}
#endif

#if SAMA_SERIAL_USE_UART3 || defined(__DOXYGEN__)
/**
 * @brief   UART3 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SAMA_UART3_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  serve_interrupt(&SD3);
  aicAckInt();
  OSAL_IRQ_EPILOGUE();
}
#endif

#if SAMA_SERIAL_USE_UART4 || defined(__DOXYGEN__)
/**
 * @brief   UART4 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SAMA_UART4_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  serve_interrupt(&SD4);
  aicAckInt();
  OSAL_IRQ_EPILOGUE();
}
#endif

#if SAMA_SERIAL_USE_FLEXCOM0 || defined(__DOXYGEN__)
/**
 * @brief   FLEXCOM0 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SAMA_SERIAL_FLEXCOM0_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  serve_uartFlex_interrupt(&FSD0);
  aicAckInt();
  OSAL_IRQ_EPILOGUE();
}
#endif

#if SAMA_SERIAL_USE_FLEXCOM1 || defined(__DOXYGEN__)
/**
 * @brief   FLEXCOM1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SAMA_SERIAL_FLEXCOM1_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  serve_uartFlex_interrupt(&FSD1);
  aicAckInt();
  OSAL_IRQ_EPILOGUE();
}
#endif

#if SAMA_SERIAL_USE_FLEXCOM2 || defined(__DOXYGEN__)
/**
 * @brief   FLEXCOM2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SAMA_SERIAL_FLEXCOM2_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  serve_uartFlex_interrupt(&FSD2);
  aicAckInt();
  OSAL_IRQ_EPILOGUE();
}
#endif

#if SAMA_SERIAL_USE_FLEXCOM3 || defined(__DOXYGEN__)
/**
 * @brief   FLEXCOM3 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SAMA_SERIAL_FLEXCOM3_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  serve_uartFlex_interrupt(&FSD3);
  aicAckInt();
  OSAL_IRQ_EPILOGUE();
}
#endif

#if SAMA_SERIAL_USE_FLEXCOM4 || defined(__DOXYGEN__)
/**
 * @brief   FLEXCOM4 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SAMA_SERIAL_FLEXCOM4_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  serve_uartFlex_interrupt(&FSD4);
  aicAckInt();
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

#if SAMA_SERIAL_USE_UART0
#if SAMA_HAL_IS_SECURE
  mtxConfigPeriphSecurity(MATRIX1, ID_UART0, SECURE_PER);
#endif /* SAMA_HAL_IS_SECURE */
  sdObjectInit(&SD0);
  iqObjectInit(&SD0.iqueue, sd_in_buf0, sizeof sd_in_buf0, NULL, &SD0);
  oqObjectInit(&SD0.oqueue, sd_out_buf0, sizeof sd_out_buf0, notify0, &SD0);
  SD0.uart = UART0;
  SD0.clock = SAMA_UART0CLK;

  aicSetSourcePriority(ID_UART0, SAMA_SERIAL_UART0_IRQ_PRIORITY);
  aicSetSourceHandler(ID_UART0, SAMA_UART0_HANDLER);
  aicEnableInt(ID_UART0);
#endif

#if SAMA_SERIAL_USE_UART1
#if SAMA_HAL_IS_SECURE
  mtxConfigPeriphSecurity(MATRIX1, ID_UART1, SECURE_PER);
#endif
  sdObjectInit(&SD1);
  iqObjectInit(&SD1.iqueue, sd_in_buf1, sizeof sd_in_buf1, NULL, &SD1);
  oqObjectInit(&SD1.oqueue, sd_out_buf1, sizeof sd_out_buf1, notify1, &SD1);
  SD1.uart = UART1;
  SD1.clock = SAMA_UART1CLK;

  aicSetSourcePriority(ID_UART1, SAMA_SERIAL_UART1_IRQ_PRIORITY);
  aicSetSourceHandler(ID_UART1, SAMA_UART1_HANDLER);
  aicEnableInt(ID_UART1);
#endif

#if SAMA_SERIAL_USE_UART2
#if SAMA_HAL_IS_SECURE
  mtxConfigPeriphSecurity(MATRIX1, ID_UART2, SECURE_PER);
#endif /* SAMA_HAL_IS_SECURE */
  sdObjectInit(&SD2);
  iqObjectInit(&SD2.iqueue, sd_in_buf2, sizeof sd_in_buf2, NULL, &SD2);
  oqObjectInit(&SD2.oqueue, sd_out_buf2, sizeof sd_out_buf2, notify2, &SD2);
  SD2.uart = UART2;
  SD2.clock = SAMA_UART2CLK;

  aicSetSourcePriority(ID_UART2, SAMA_SERIAL_UART2_IRQ_PRIORITY);
  aicSetSourceHandler(ID_UART2, SAMA_UART2_HANDLER);
  aicEnableInt(ID_UART2);
#endif

#if SAMA_SERIAL_USE_UART3
#if SAMA_HAL_IS_SECURE
  mtxConfigPeriphSecurity(MATRIX1, ID_UART3, SECURE_PER);
#endif /* SAMA_HAL_IS_SECURE */
  sdObjectInit(&SD3);
  iqObjectInit(&SD3.iqueue, sd_in_buf3, sizeof sd_in_buf3, NULL, &SD3);
  oqObjectInit(&SD3.oqueue, sd_out_buf3, sizeof sd_out_buf3, notify3, &SD3);
  SD3.uart = UART3;
  SD3.clock = SAMA_UART3CLK;

  aicSetSourcePriority(ID_UART3, SAMA_SERIAL_UART3_IRQ_PRIORITY);
  aicSetSourceHandler(ID_UART3, SAMA_UART3_HANDLER);
  aicEnableInt(ID_UART3);
#endif

#if SAMA_SERIAL_USE_UART4
#if SAMA_HAL_IS_SECURE
  mtxConfigPeriphSecurity(MATRIX1, ID_UART4, SECURE_PER);
#endif /* SAMA_HAL_IS_SECURE */
  sdObjectInit(&SD4);
  iqObjectInit(&SD4.iqueue, sd_in_buf4, sizeof sd_in_buf4, NULL, &SD4);
  oqObjectInit(&SD4.oqueue, sd_out_buf4, sizeof sd_out_buf4, notify4, &SD4);
  SD4.uart = UART4;
  SD4.clock = SAMA_UART4CLK;

  aicSetSourcePriority(ID_UART4, SAMA_SERIAL_UART4_IRQ_PRIORITY);
  aicSetSourceHandler(ID_UART4, SAMA_UART4_HANDLER);
  aicEnableInt(ID_UART4);
#endif

#if SAMA_SERIAL_USE_FLEXCOM0
#if SAMA_HAL_IS_SECURE
  mtxConfigPeriphSecurity(MATRIX1, ID_FLEXCOM0, SECURE_PER);
#endif /* SAMA_HAL_IS_SECURE */
  sdObjectInit(&FSD0);
  iqObjectInit(&FSD0.iqueue, sdFlex_in_buf0, sizeof sdFlex_in_buf0, NULL, &FSD0);
  oqObjectInit(&FSD0.oqueue, sdFlex_out_buf0, sizeof sdFlex_out_buf0, notifyFlex0, &FSD0);
  FSD0.flexcom = FLEXCOM0;
  FSD0.usart   = USART0;
  FSD0.clock   = SAMA_FLEXCOM0CLK;

  aicSetSourcePriority(ID_USART0, SAMA_SERIAL_FLEXCOM0_IRQ_PRIORITY);
  aicSetSourceHandler(ID_USART0, SAMA_SERIAL_FLEXCOM0_HANDLER);
  aicEnableInt(ID_USART0);
#endif

#if SAMA_SERIAL_USE_FLEXCOM1
#if SAMA_HAL_IS_SECURE
  mtxConfigPeriphSecurity(MATRIX1, ID_FLEXCOM1, SECURE_PER);
#endif /* SAMA_HAL_IS_SECURE */
  sdObjectInit(&FSD1);
  iqObjectInit(&FSD1.iqueue, sdFlex_in_buf1, sizeof sdFlex_in_buf1, NULL, &FSD1);
  oqObjectInit(&FSD1.oqueue, sdFlex_out_buf1, sizeof sdFlex_out_buf1, notifyFlex1, &FSD1);
  FSD1.flexcom = FLEXCOM1;
  FSD1.usart   = USART1;
  FSD1.clock   = SAMA_FLEXCOM1CLK;

  aicSetSourcePriority(ID_USART1, SAMA_SERIAL_FLEXCOM1_IRQ_PRIORITY);
  aicSetSourceHandler(ID_USART1, SAMA_SERIAL_FLEXCOM1_HANDLER);
  aicEnableInt(ID_USART1);
#endif

#if SAMA_SERIAL_USE_FLEXCOM2
#if SAMA_HAL_IS_SECURE
  mtxConfigPeriphSecurity(MATRIX1, ID_FLEXCOM2, SECURE_PER);
#endif /* SAMA_HAL_IS_SECURE */
  sdObjectInit(&FSD2);
  iqObjectInit(&FSD2.iqueue, sdFlex_in_buf2, sizeof sdFlex_in_buf2, NULL, &FSD2);
  oqObjectInit(&FSD2.oqueue, sdFlex_out_buf2, sizeof sdFlex_out_buf2, notifyFlex2, &FSD2);
  FSD2.flexcom = FLEXCOM2;
  FSD2.usart   = USART2;
  FSD2.clock   = SAMA_FLEXCOM2CLK;

  aicSetSourcePriority(ID_USART2, SAMA_SERIAL_FLEXCOM2_IRQ_PRIORITY);
  aicSetSourceHandler(ID_USART2, SAMA_SERIAL_FLEXCOM2_HANDLER);
  aicEnableInt(ID_USART2);
#endif

#if SAMA_SERIAL_USE_FLEXCOM3
#if SAMA_HAL_IS_SECURE
  mtxConfigPeriphSecurity(MATRIX1, ID_FLEXCOM3, SECURE_PER);
#endif /* SAMA_HAL_IS_SECURE */
  sdObjectInit(&FSD3);
  iqObjectInit(&FSD3.iqueue, sdFlex_in_buf3, sizeof sdFlex_in_buf3, NULL, &FSD3);
  oqObjectInit(&FSD3.oqueue, sdFlex_out_buf3, sizeof sdFlex_out_buf3, notifyFlex3, &FSD3);
  FSD3.flexcom = FLEXCOM3;
  FSD3.usart   = USART3;
  FSD3.clock   = SAMA_FLEXCOM3CLK;

  aicSetSourcePriority(ID_USART3, SAMA_SERIAL_FLEXCOM3_IRQ_PRIORITY);
  aicSetSourceHandler(ID_USART3, SAMA_SERIAL_FLEXCOM3_HANDLER);
  aicEnableInt(ID_USART3);
#endif

#if SAMA_SERIAL_USE_FLEXCOM4
#if SAMA_HAL_IS_SECURE
  mtxConfigPeriphSecurity(MATRIX1, ID_FLEXCOM4, SECURE_PER);
#endif /* SAMA_HAL_IS_SECURE */
  sdObjectInit(&FSD4);
  iqObjectInit(&FSD4.iqueue, sdFlex_in_buf4, sizeof sdFlex_in_buf4, NULL, &FSD4);
  oqObjectInit(&FSD4.oqueue, sdFlex_out_buf4, sizeof sdFlex_out_buf4, notifyFlex4, &FSD4);
  FSD4.flexcom = FLEXCOM4;
  FSD4.usart   = USART4;
  FSD4.clock   = SAMA_FLEXCOM4CLK;

  aicSetSourcePriority(ID_USART4, SAMA_SERIAL_FLEXCOM4_IRQ_PRIORITY);
  aicSetSourceHandler(ID_USART4, SAMA_SERIAL_FLEXCOM4_HANDLER);
  aicEnableInt(ID_USART4);
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

  if (config == NULL) {

    config = &default_config;
  }
  if (sdp->state == SD_STOP) {
#if SAMA_SERIAL_USE_UART0
    if (&SD0 == sdp) {
      pmcEnableUART0();
    }
#endif
#if SAMA_SERIAL_USE_UART1
    if (&SD1 == sdp) {
      pmcEnableUART1();
    }
#endif
#if SAMA_SERIAL_USE_UART2
    if (&SD2 == sdp) {
      pmcEnableUART2();
    }
#endif
#if SAMA_SERIAL_USE_UART3
    if (&SD3 == sdp) {
      pmcEnableUART3();
    }
#endif
#if SAMA_SERIAL_USE_UART4
    if (&SD4 == sdp) {
      pmcEnableUART4();
    }
#endif
#if SAMA_SERIAL_USE_FLEXCOM0
    if (&FSD0 == sdp) {
      pmcEnableFLEXCOM0();
    }
#endif
#if SAMA_SERIAL_USE_FLEXCOM1
    if (&FSD1 == sdp) {
      pmcEnableFLEXCOM1();
    }
#endif
#if SAMA_SERIAL_USE_FLEXCOM2
    if (&FSD2 == sdp) {
      pmcEnableFLEXCOM2();
    }
#endif
#if SAMA_SERIAL_USE_FLEXCOM3
    if (&FSD3 == sdp) {
      pmcEnableFLEXCOM3();
    }
#endif
#if SAMA_SERIAL_USE_FLEXCOM4
    if (&FSD4 == sdp) {
      pmcEnableFLEXCOM4();
    }
#endif
  }
  uart_init(sdp, config);
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
    uart_deinit(sdp);

#if SAMA_SERIAL_USE_UART0
    if (&SD0 == sdp) {
      pmcDisableUART0();
      return;
    }
#endif
#if SAMA_SERIAL_USE_UART1
    if (&SD1 == sdp) {
      pmcDisableUART1();
      return;
    }
#endif
#if SAMA_SERIAL_USE_UART2
    if (&SD2 == sdp) {
      pmcDisableUART2();
      return;
    }
#endif
#if SAMA_SERIAL_USE_UART3
    if (&SD3 == sdp) {
      pmcDisableUART3();
      return;
    }
#endif
#if SAMA_SERIAL_USE_UART4
    if (&SD4 == sdp) {
      pmcDisableUART4();
      return;
    }
#endif
#if SAMA_SERIAL_USE_FLEXCOM0
    if (&FSD0 == sdp) {
      pmcDisableFLEXCOM0();
      return;
    }
#endif
#if SAMA_SERIAL_USE_FLEXCOM1
    if (&FSD1 == sdp) {
      pmcDisableFLEXCOM1();
      return;
    }
#endif
#if SAMA_SERIAL_USE_FLEXCOM2
    if (&FSD2 == sdp) {
      pmcDisableFLEXCOM2();
      return;
    }
#endif
#if SAMA_SERIAL_USE_FLEXCOM3
    if (&FSD3 == sdp) {
      pmcDisableFLEXCOM3();
      return;
    }
#endif
#if SAMA_SERIAL_USE_FLEXCOM4
    if (&FSD4 == sdp) {
      pmcDisableFLEXCOM4();
      return;
    }
#endif
  }
}

#endif /* HAL_USE_SERIAL */

/** @} */
