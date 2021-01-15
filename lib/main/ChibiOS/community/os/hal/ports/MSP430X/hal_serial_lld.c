/*
    ChibiOS - Copyright (C) 2016 Andrew Wygle aka awygle

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
 * @file    MSP430X/hal_serial_lld.c
 * @brief   MSP430X serial subsystem low level driver source.
 *
 * @addtogroup SERIAL
 * @{
 */

#include "hal.h"

#if (HAL_USE_SERIAL == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief USART0 serial driver identifier.*/
#if (MSP430X_SERIAL_USE_USART0 == TRUE) || defined(__DOXYGEN__)
#ifndef __MSP430_HAS_EUSCI_A0__
#error "Cannot find USCI module to use for SD0"
#endif
#ifdef MSP430X_USCI_A0_USED
#error "USCI module A0 already in use - USART0 not available"
#endif
SerialDriver SD0;
#define MSP430X_USCI_A0_USED
#endif

/** @brief USART1 serial driver identifier.*/
#if (MSP430X_SERIAL_USE_USART1 == TRUE) || defined(__DOXYGEN__)
#ifndef __MSP430_HAS_EUSCI_A1__
#error "Cannot find USCI module to use for SD1"
#endif
#ifdef MSP430X_USCI_A1_USED
#error "USCI module A1 already in use - USART1 not available"
#endif
SerialDriver SD1;
#define MSP430X_USCI_A1_USED
#endif

/** @brief USART2 serial driver identifier.*/
#if (MSP430X_SERIAL_USE_USART2 == TRUE) || defined(__DOXYGEN__)
#ifndef __MSP430_HAS_EUSCI_A2__
#error "Cannot find USCI module to use for SD2"
#endif
#ifdef MSP430X_USCI_A2_USED
#error "USCI module A2 already in use - USART2 not available"
#endif
SerialDriver SD2;
#define MSP430X_USCI_A2_USED
#endif

/** @brief USART3 serial driver identifier.*/
#if (MSP430X_SERIAL_USE_USART3 == TRUE) || defined(__DOXYGEN__)
#ifndef __MSP430_HAS_EUSCI_A3__
#error "Cannot find USCI module to use for SD3"
#endif
#ifdef MSP430X_USCI_A3_USED
#error "USCI module A3 already in use - USART3 not available"
#endif
SerialDriver SD3;
#define MSP430X_USCI_A3_USED
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   Driver default configuration.
 */
static const SerialConfig default_config = { SERIAL_DEFAULT_BITRATE };

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief     UCBRS calculation.
 * @details   This function calculates the UCBRS value for oversampled baud
 *            rates.
 *
 * @param[in] frac    Fractional part of baud rate division, times 10000.
 */
static uint8_t UCBRS(uint16_t frac) {
  /* TODO there must be a better way */
  if (frac < 529)
    return 0x00;
  else if (frac < 715)
    return 0x01;
  else if (frac < 835)
    return 0x02;
  else if (frac < 1001)
    return 0x04;
  else if (frac < 1252)
    return 0x08;
  else if (frac < 1430)
    return 0x10;
  else if (frac < 1670)
    return 0x20;
  else if (frac < 2147)
    return 0x11;
  else if (frac < 2224)
    return 0x21;
  else if (frac < 2503)
    return 0x22;
  else if (frac < 3000)
    return 0x44;
  else if (frac < 3335)
    return 0x25;
  else if (frac < 3575)
    return 0x49;
  else if (frac < 3753)
    return 0x4A;
  else if (frac < 4003)
    return 0x52;
  else if (frac < 4286)
    return 0x92;
  else if (frac < 4378)
    return 0x53;
  else if (frac < 5002)
    return 0x55;
  else if (frac < 5715)
    return 0xAA;
  else if (frac < 6003)
    return 0x6B;
  else if (frac < 6254)
    return 0xAD;
  else if (frac < 6432)
    return 0xB5;
  else if (frac < 6667)
    return 0xB6;
  else if (frac < 7001)
    return 0xD6;
  else if (frac < 7147)
    return 0xB7;
  else if (frac < 7503)
    return 0xBB;
  else if (frac < 7861)
    return 0xDD;
  else if (frac < 8004)
    return 0xED;
  else if (frac < 8333)
    return 0xEE;
  else if (frac < 8464)
    return 0xBF;
  else if (frac < 8572)
    return 0xDF;
  else if (frac < 8751)
    return 0xEF;
  else if (frac < 9004)
    return 0xF7;
  else if (frac < 9170)
    return 0xFB;
  else if (frac < 9288)
    return 0xFD;
  else
    return 0xFE;
}

/**
 * @brief     Modulation control word calculator.
 * @details   This function calculates the modulation control word from the
 *            input clock frequency and the requested baud rate.
 *
 * @param[in] baud    Requested baud rate
 * @param[in] freq    Frequency of the clock driving the USCI module
 */
static uint16_t UCAxMCTLW(uint32_t baud, uint32_t freq) {

  uint16_t n = freq / baud;
  /*uint16_t frac = (freq * 10000 / baud) - ((freq / baud) * 10000);*/
  uint16_t frac = (freq - (n * baud)) * 10000 / baud;
  if (n > 16) {
    while (n > 16) {
      n -= 16;
    }
    return (UCBRS(frac) << 8) | (n << 4) | UCOS16;
  }
  return UCBRS(frac) << 8;
}

/**
 * @brief     UCBRW calculation.
 * @details   This function calculates the UCBRW value for all baud
 *            rates.
 *
 * @param[in] baud    Requested baud rate
 * @param[in] freq    Frequency of the clock driving the USCI module
 */
static uint16_t UCAxBRW(uint32_t baud, uint32_t freq) {
  uint16_t n = freq / baud;
  if (n > 16) {
    return n >> 4;
  }
  return n;
}

#if (MSP430X_SERIAL_USE_USART0 == TRUE) || defined(__DOXYGEN__)
static void usart0_init(const SerialConfig * config) {
  UCA0BRW   = UCAxBRW(config->sc_bitrate, MSP430X_USART0_CLK_FREQ);
  UCA0MCTLW = UCAxMCTLW(config->sc_bitrate, MSP430X_USART0_CLK_FREQ);
  UCA0STATW = 0;
  UCA0ABCTL = 0;
  UCA0IRCTL = 0;
  UCA0CTLW0 = (MSP430X_USART0_PARITY << 14) | (MSP430X_USART0_ORDER << 13) |
              (MSP430X_USART0_SIZE << 12) | (MSP430X_USART0_STOP << 11) |
              (MSP430X_USART0_UCSSEL);
  UCA0IE = UCRXIE;
}
#endif

#if (MSP430X_SERIAL_USE_USART1 == TRUE) || defined(__DOXYGEN__)
static void usart1_init(const SerialConfig * config) {
  UCA1BRW   = UCAxBRW(config->sc_bitrate, MSP430X_USART1_CLK_FREQ);
  UCA1MCTLW = UCAxMCTLW(config->sc_bitrate, MSP430X_USART1_CLK_FREQ);
  UCA1STATW = 0;
  UCA1ABCTL = 0;
  UCA1IRCTL = 0;
  UCA1CTLW0 = (MSP430X_USART1_PARITY << 14) | (MSP430X_USART1_ORDER << 13) |
              (MSP430X_USART1_SIZE << 12) | (MSP430X_USART1_STOP << 11) |
              (MSP430X_USART1_UCSSEL);
  UCA1IE = UCRXIE;
}
#endif

#if (MSP430X_SERIAL_USE_USART2 == TRUE) || defined(__DOXYGEN__)
static void usart2_init(const SerialConfig * config) {
  UCA2BRW   = UCAxBRW(config->sc_bitrate, MSP430X_USART2_CLK_FREQ);
  UCA2MCTLW = UCAxMCTLW(config->sc_bitrate);
  UCA2STATW = 0;
  UCA2ABCTL = 0;
  UCA2IRCTL = 0;
  UCA2CTLW0 = (MSP430X_USART2_PARITY << 14) | (MSP430X_USART2_ORDER << 13) |
              (MSP430X_USART2_SIZE << 12) | (MSP430X_USART2_STOP << 11) |
              (MSP430X_USART2_UCSSEL);
  UCA2IE = UCRXIE;
}
#endif

#if (MSP430X_SERIAL_USE_USART3 == TRUE) || defined(__DOXYGEN__)
static void usart3_init(const SerialConfig * config) {
  UCA3BRW   = UCAxBRW(config->sc_bitrate, MSP430X_USART3_CLK_FREQ);
  UCA3MCTLW = UCAxMCTLW(config->sc_bitrate, MSP430X_USART3_CLK_FREQ);
  UCA3STATW = 0;
  UCA3ABCTL = 0;
  UCA3IRCTL = 0;
  UCA3CTLW0 = (MSP430X_USART3_PARITY << 14) | (MSP430X_USART3_ORDER << 13) |
              (MSP430X_USART3_SIZE << 12) | (MSP430X_USART3_STOP << 11) |
              (MSP430X_USART3_UCSSEL);
  UCA3IE = UCRXIE;
}
#endif

#if (MSP430X_SERIAL_USE_USART0 == TRUE) || defined(__DOXYGEN__)
static void notify0(io_queue_t * qp) {

  (void)qp;
  UCA0IE |= UCTXIE;
}
#endif

#if (MSP430X_SERIAL_USE_USART1 == TRUE) || defined(__DOXYGEN__)
static void notify1(io_queue_t * qp) {

  (void)qp;
  UCA1IE |= UCTXIE;
}
#endif

#if (MSP430X_SERIAL_USE_USART2 == TRUE) || defined(__DOXYGEN__)
static void notify2(io_queue_t * qp) {

  (void)qp;
  UCA2IE |= UCTXIE;
}
#endif

#if (MSP430X_SERIAL_USE_USART3 == TRUE) || defined(__DOXYGEN__)
static void notify3(io_queue_t * qp) {

  (void)qp;
  UCA3IE |= UCTXIE;
}
#endif

/**
 * @brief     Error handling routine.
 *
 * @param[in] sra     USCI status register containing errors
 * @param[in] sdp     pointer to a @p SerialDriver object
 */
static void set_error(uint16_t sra, SerialDriver * sdp) {
  eventflags_t sts = 0;

  if (sra & UCOE)
    sts |= SD_OVERRUN_ERROR;
  if (sra & UCPE)
    sts |= SD_PARITY_ERROR;
  if (sra & UCFE)
    sts |= SD_FRAMING_ERROR;
  osalSysLockFromISR();
  chnAddFlagsI(sdp, sts);
  osalSysUnlockFromISR();
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if MSP430X_SERIAL_USE_USART0 || defined(__DOXYGEN__)
/**
 * @brief   USART0 interrupt handler.
 *
 * @isr
 */
PORT_IRQ_HANDLER(USCI_A0_VECTOR) {
  msg_t b;

  OSAL_IRQ_PROLOGUE();

  switch (__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG)) {
  case USCI_UART_UCRXIFG: /* RX interrupt */

    /* Detect errors */
    if (UCA0STATW & UCRXERR)
      set_error(UCA0STATW, &SD0);

    /* Data available */
    osalSysLockFromISR();
    sdIncomingDataI(&SD0, UCA0RXBUF);
    osalSysUnlockFromISR();
    break;

  case USCI_UART_UCTXIFG: /* TX interrupt */

    /* Transmission buffer empty */
    osalSysLockFromISR();
    b = sdRequestDataI(&SD0);
    if (b < Q_OK) {
      chnAddFlagsI(&SD0, CHN_OUTPUT_EMPTY);
      UCA0IE = (UCA0IE & ~UCTXIE) | UCTXCPTIE;
      UCA0IFG |= UCTXIFG; /* If we don't write to TXBUF, IFG won't get set */
    }
    else
      UCA0TXBUF = b;
    osalSysUnlockFromISR();
    break;

  case USCI_UART_UCTXCPTIFG: /* TX complete interrupt */

    /* Physical transmission end */
    osalSysLockFromISR();
    if (oqIsEmptyI(&SD0.oqueue))
      chnAddFlagsI(&SD0, CHN_TRANSMISSION_END);
    UCA0IE &= ~UCTXCPTIE;
    osalSysUnlockFromISR();
    break;

  default: /* other interrupts */
    osalDbgAssert(false, "unhandled serial interrupt");
    break;
  }

  OSAL_IRQ_EPILOGUE();
}
#endif

#if MSP430X_SERIAL_USE_USART1 || defined(__DOXYGEN__)
/**
 * @brief   USART1 interrupt handler.
 *
 * @isr
 */
PORT_IRQ_HANDLER(USCI_A1_VECTOR) {
  msg_t b;

  OSAL_IRQ_PROLOGUE();

  switch (__even_in_range(UCA1IV, USCI_UART_UCTXCPTIFG)) {
  case USCI_UART_UCRXIFG: /* RX interrupt */

    /* Detect errors */
    if (UCA1STATW & UCRXERR)
      set_error(UCA1STATW, &SD1);

    /* Data available */
    osalSysLockFromISR();
    sdIncomingDataI(&SD1, UCA1RXBUF);
    osalSysUnlockFromISR();
    break;

  case USCI_UART_UCTXIFG: /* TX interrupt */

    /* Transmission buffer empty */
    osalSysLockFromISR();
    b = sdRequestDataI(&SD1);
    if (b < Q_OK) {
      chnAddFlagsI(&SD1, CHN_OUTPUT_EMPTY);
      UCA1IE = (UCA1IE & ~UCTXIE) | UCTXCPTIE;
      UCA1IFG |= UCTXIFG; /* If we don't write to TXBUF, IFG won't get set */
    }
    else
      UCA1TXBUF = b;
    osalSysUnlockFromISR();
    break;

  case USCI_UART_UCTXCPTIFG: /* TX complete interrupt */

    /* Physical transmission end */
    osalSysLockFromISR();
    if (oqIsEmptyI(&SD1.oqueue))
      chnAddFlagsI(&SD1, CHN_TRANSMISSION_END);
    UCA1IE &= ~UCTXCPTIE;
    osalSysUnlockFromISR();
    break;

  default: /* other interrupts */
    osalDbgAssert(false, "unhandled serial interrupt");
    break;
  }

  OSAL_IRQ_EPILOGUE();
}
#endif

#if MSP430X_SERIAL_USE_USART2 || defined(__DOXYGEN__)
/**
 * @brief   USART2 interrupt handler.
 *
 * @isr
 */
PORT_IRQ_HANDLER(USCI_A2_VECTOR) {
  msg_t b;

  OSAL_IRQ_PROLOGUE();

  switch (__even_in_range(UCA2IV, USCI_UART_UCTXCPTIFG)) {
  case USCI_UART_UCRXIFG: /* RX interrupt */

    /* Detect errors */
    if (UCA2STATW & UCRXERR)
      set_error(UCA2STATW, &SD2);

    /* Data available */
    osalSysLockFromISR();
    sdIncomingDataI(&SD2, UCA2RXBUF);
    osalSysUnlockFromISR();
    break;

  case USCI_UART_UCTXIFG: /* TX interrupt */

    /* Transmission buffer empty */
    osalSysLockFromISR();
    b = sdRequestDataI(&SD2);
    if (b < Q_OK) {
      chnAddFlagsI(&SD2, CHN_OUTPUT_EMPTY);
      UCA2IE = (UCA2IE & ~UCTXIE) | UCTXCPTIE;
      UCA2IFG |= UCTXIFG; /* If we don't write to TXBUF, IFG won't get set */
    }
    else
      UCA2TXBUF = b;
    osalSysUnlockFromISR();
    break;

  case USCI_UART_UCTXCPTIFG: /* TX complete interrupt */

    /* Physical transmission end */
    osalSysLockFromISR();
    if (oqIsEmptyI(&SD2.oqueue))
      chnAddFlagsI(&SD2, CHN_TRANSMISSION_END);
    UCA2IE &= ~UCTXCPTIE;
    osalSysUnlockFromISR();
    break;

  default: /* other interrupts */
    osalDbgAssert(false, "unhandled serial interrupt");
    break;
  }

  OSAL_IRQ_EPILOGUE();
}
#endif

#if MSP430X_SERIAL_USE_USART3 || defined(__DOXYGEN__)
/**
 * @brief   USART3 interrupt handler.
 *
 * @isr
 */
PORT_IRQ_HANDLER(USCI_A3_VECTOR) {
  msg_t b;

  OSAL_IRQ_PROLOGUE();

  switch (__even_in_range(UCA3IV, USCI_UART_UCTXCPTIFG)) {
  case USCI_UART_UCRXIFG: /* RX interrupt */

    /* Detect errors */
    if (UCA3STATW & UCRXERR)
      set_error(UCA3STATW, &SD3);

    /* Data available */
    osalSysLockFromISR();
    sdIncomingDataI(&SD3, UCA3RXBUF);
    osalSysUnlockFromISR();
    break;

  case USCI_UART_UCTXIFG: /* TX interrupt */

    /* Transmission buffer empty */
    osalSysLockFromISR();
    b = sdRequestDataI(&SD3);
    if (b < Q_OK) {
      chnAddFlagsI(&SD3, CHN_OUTPUT_EMPTY);
      UCA3IE = (UCA3IE & ~UCTXIE) | UCTXCPTIE;
      UCA3IFG |= UCTXIFG; /* If we don't write to TXBUF, IFG won't get set */
    }
    else
      UCA3TXBUF = b;
    osalSysUnlockFromISR();
    break;

  case USCI_UART_UCTXCPTIFG: /* TX complete interrupt */

    /* Physical transmission end */
    osalSysLockFromISR();
    if (oqIsEmptyI(&SD3.oqueue))
      chnAddFlagsI(&SD3, CHN_TRANSMISSION_END);
    UCA3IE &= ~UCTXCPTIE;
    osalSysUnlockFromISR();
    break;

  default: /* other interrupts */
    osalDbgAssert(false, "unhandled serial interrupt");
    break;
  }

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

#if MSP430X_SERIAL_USE_USART0 == TRUE
  sdObjectInit(&SD0, NULL, notify0);
#endif

#if MSP430X_SERIAL_USE_USART1 == TRUE
  sdObjectInit(&SD1, NULL, notify1);
#endif

#if MSP430X_SERIAL_USE_USART2 == TRUE
  sdObjectInit(&SD2, NULL, notify2);
#endif

#if MSP430X_SERIAL_USE_USART3 == TRUE
  sdObjectInit(&SD3, NULL, notify3);
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
void sd_lld_start(SerialDriver * sdp, const SerialConfig * config) {

  if (config == NULL) {
    config = &default_config;
  }

  if (sdp->state == SD_STOP) {
#if MSP430X_SERIAL_USE_USART0 == TRUE
    if (&SD0 == sdp) {
      usart0_init(config);
    }
#endif
#if MSP430X_SERIAL_USE_USART1 == TRUE
    if (&SD1 == sdp) {
      usart1_init(config);
    }
#endif
#if MSP430X_SERIAL_USE_USART2 == TRUE
    if (&SD2 == sdp) {
      usart2_init(config);
    }
#endif
#if MSP430X_SERIAL_USE_USART3 == TRUE
    if (&SD3 == sdp) {
      usart3_init(config);
    }
#endif
  }
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
void sd_lld_stop(SerialDriver * sdp) {

  if (sdp->state == SD_READY) {
#if MSP430X_SERIAL_USE_USART0 == TRUE
    if (&SD0 == sdp) {
      UCA0CTLW0 = UCSWRST;
    }
#endif
#if MSP430X_SERIAL_USE_USART1 == TRUE
    if (&SD1 == sdp) {
      UCA1CTLW0 = UCSWRST;
    }
#endif
#if MSP430X_SERIAL_USE_USART2 == TRUE
    if (&SD2 == sdp) {
      UCA2CTLW0 = UCSWRST;
    }
#endif
#if MSP430X_SERIAL_USE_USART3 == TRUE
    if (&SD3 == sdp) {
      UCA3CTLW0 = UCSWRST;
    }
#endif
  }
}

#endif /* HAL_USE_SERIAL == TRUE */

/** @} */
