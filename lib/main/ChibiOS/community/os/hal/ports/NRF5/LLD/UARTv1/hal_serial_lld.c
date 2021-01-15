/*
    Copyright (C) 2015 Fabio Utzig

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
 * @file    UARTv1/hal_serial_lld.c
 * @brief   NRF5 serial subsystem low level driver source.
 *
 * @addtogroup SERIAL
 * @{
 */

#include "hal.h"

#if (HAL_USE_SERIAL == TRUE) || defined(__DOXYGEN__)

#if   NRF_SERIES == 51
#include "nrf51.h"
#elif NRF_SERIES == 52
#include "nrf52.h"
#define UART0_IRQn UARTE0_UART0_IRQn
#endif

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/


/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief USART1 serial driver identifier.*/
#if (NRF5_SERIAL_USE_UART0 == TRUE) || defined(__DOXYGEN__)
SerialDriver SD1;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   Driver default configuration.
 */
static const SerialConfig default_config = {
  .speed   = 38400,
  .tx_pad  = NRF5_SERIAL_PAD_DISCONNECTED,
  .rx_pad  = NRF5_SERIAL_PAD_DISCONNECTED,
#if (NRF5_SERIAL_USE_HWFLOWCTRL == TRUE)
  .rts_pad = NRF5_SERIAL_PAD_DISCONNECTED,
  .cts_pad = NRF5_SERIAL_PAD_DISCONNECTED,
#endif
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*
 * @brief Maps a baudrate speed to a BAUDRATE register value.
 */

/**
 * @brief   Common UART configuration.
 *
 */
static void configure_uart(const SerialConfig *config)
{
  uint32_t speed = UART_BAUDRATE_BAUDRATE_Baud250000;

  switch (config->speed) {
    case 1200: speed = UART_BAUDRATE_BAUDRATE_Baud1200; break;
    case 2400: speed = UART_BAUDRATE_BAUDRATE_Baud2400; break;
    case 4800: speed = UART_BAUDRATE_BAUDRATE_Baud4800; break;
    case 9600: speed = UART_BAUDRATE_BAUDRATE_Baud9600; break;
    case 14400: speed = UART_BAUDRATE_BAUDRATE_Baud14400; break;
    case 19200: speed = UART_BAUDRATE_BAUDRATE_Baud19200; break;
    case 28800: speed = UART_BAUDRATE_BAUDRATE_Baud28800; break;
    case 38400: speed = UART_BAUDRATE_BAUDRATE_Baud38400; break;
    case 57600: speed = UART_BAUDRATE_BAUDRATE_Baud57600; break;
    case 76800: speed = UART_BAUDRATE_BAUDRATE_Baud76800; break;
    case 115200: speed = UART_BAUDRATE_BAUDRATE_Baud115200; break;
    case 230400: speed = UART_BAUDRATE_BAUDRATE_Baud230400; break;
    case 250000: speed = UART_BAUDRATE_BAUDRATE_Baud250000; break;
    case 460800: speed = UART_BAUDRATE_BAUDRATE_Baud460800; break;
    case 921600: speed = UART_BAUDRATE_BAUDRATE_Baud921600; break;
    case 1000000: speed = UART_BAUDRATE_BAUDRATE_Baud1M; break;
    default: osalDbgAssert(0, "invalid baudrate"); break;
  };

  /* Configure PINs mode */
  if (config->tx_pad != NRF5_SERIAL_PAD_DISCONNECTED) {
    palSetPadMode(IOPORT1, config->tx_pad, PAL_MODE_OUTPUT_PUSHPULL);
  }
  if (config->rx_pad != NRF5_SERIAL_PAD_DISCONNECTED) {
    palSetPadMode(IOPORT1, config->rx_pad, PAL_MODE_INPUT);
  }
#if (NRF5_SERIAL_USE_HWFLOWCTRL == TRUE)
  if (config->rts_pad != NRF5_SERIAL_PAD_DISCONNECTED) {
    palSetPadMode(IOPORT1, config->rts_pad, PAL_MODE_OUTPUT_PUSHPULL);
  }
  if (config->cts_pad != NRF5_SERIAL_PAD_DISCONNECTED) {
    palSetPadMode(IOPORT1, config->cts_pad, PAL_MODE_INPUT);
  }
#endif
  
  /* Select PINs used by UART */
  NRF_UART0->PSELTXD = config->tx_pad;
  NRF_UART0->PSELRXD = config->rx_pad;
#if (NRF5_SERIAL_USE_HWFLOWCTRL == TRUE)
  NRF_UART0->PSELRTS = config->rts_pad;
  NRF_UART0->PSELCTS = config->cts_pad;
#else
  NRF_UART0->PSELRTS = NRF5_SERIAL_PAD_DISCONNECTED;
  NRF_UART0->PSELCTS = NRF5_SERIAL_PAD_DISCONNECTED;
#endif

  /* Set baud rate */
  NRF_UART0->BAUDRATE = speed;

  /* Set config */
  NRF_UART0->CONFIG = (UART_CONFIG_PARITY_Excluded << UART_CONFIG_PARITY_Pos);

  /* Adjust flow control */
#if (NRF5_SERIAL_USE_HWFLOWCTRL == TRUE)
  if ((config->rts_pad < TOTAL_GPIO_PADS) ||
      (config->cts_pad < TOTAL_GPIO_PADS)) {
    NRF_UART0->CONFIG |=   UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos;
  } else {
    NRF_UART0->CONFIG &= ~(UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos);
  }
#else
  NRF_UART0->CONFIG   &= ~(UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos);
#endif
  
  /* Enable UART and clear events */
  NRF_UART0->ENABLE = UART_ENABLE_ENABLE_Enabled;
  NRF_UART0->EVENTS_RXDRDY = 0;
  NRF_UART0->EVENTS_TXDRDY = 0;
#if CORTEX_MODEL >= 4
  (void)NRF_UART0->EVENTS_RXDRDY;
  (void)NRF_UART0->EVENTS_TXDRDY;
#endif
  
  if (config->rx_pad != NRF5_SERIAL_PAD_DISCONNECTED) {
    while (NRF_UART0->EVENTS_RXDRDY != 0) {
      (void)NRF_UART0->RXD;
    }
  }
}


/**
 * @brief   Driver output notification.
 */
#if NRF5_SERIAL_USE_UART0 || defined(__DOXYGEN__)
static void notify1(io_queue_t *qp)
{
  SerialDriver *sdp = &SD1;

  (void)qp;

  if (NRF_UART0->PSELTXD == NRF5_SERIAL_PAD_DISCONNECTED)
    return;

  if (!sdp->tx_busy) {
    msg_t b = oqGetI(&sdp->oqueue);

    if (b < Q_OK) {
      chnAddFlagsI(sdp, CHN_OUTPUT_EMPTY);
      NRF_UART0->TASKS_STOPTX = 1;
      return;
    }
    sdp->tx_busy = 1;
    NRF_UART0->TASKS_STARTTX = 1;
    NRF_UART0->TXD = b;
  }
}
#endif


/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if NRF5_SERIAL_USE_UART0 || defined(__DOXYGEN__)
OSAL_IRQ_HANDLER(Vector48) {

  OSAL_IRQ_PROLOGUE();

  SerialDriver *sdp = &SD1;
  uint32_t isr = NRF_UART0->INTENSET;

  if ((NRF_UART0->EVENTS_RXDRDY != 0) && (isr & UART_INTENSET_RXDRDY_Msk)) {
    // Clear UART RX event flag
    NRF_UART0->EVENTS_RXDRDY = 0;
#if CORTEX_MODEL >= 4
    (void)NRF_UART0->EVENTS_RXDRDY;
#endif
    
    osalSysLockFromISR();
    if (iqIsEmptyI(&sdp->iqueue))
      chnAddFlagsI(sdp, CHN_INPUT_AVAILABLE);
    if (iqPutI(&sdp->iqueue, NRF_UART0->RXD) < Q_OK)
      chnAddFlagsI(sdp, SD_OVERRUN_ERROR);
    osalSysUnlockFromISR();
  }

  if ((NRF_UART0->EVENTS_TXDRDY != 0) && (isr & UART_INTENSET_TXDRDY_Msk)) {
    msg_t b;

    // Clear UART TX event flag.
    NRF_UART0->EVENTS_TXDRDY = 0;
#if CORTEX_MODEL >= 4
    (void)NRF_UART0->EVENTS_TXDRDY;
#endif
    
    osalSysLockFromISR();
    b = oqGetI(&sdp->oqueue);
    osalSysUnlockFromISR();

    if (b < Q_OK) {
      osalSysLockFromISR();
      chnAddFlagsI(sdp, CHN_OUTPUT_EMPTY);
      osalSysUnlockFromISR();
      NRF_UART0->TASKS_STOPTX = 1;
      sdp->tx_busy = 0;
    } else {
      sdp->tx_busy = 1;
      NRF_UART0->TXD = b;
    }
  }

  /* TODO: Error handling for EVENTS_ERROR */
  if ((NRF_UART0->EVENTS_ERROR != 0) && (isr & UART_INTENSET_ERROR_Msk)) {
    // Clear UART ERROR event flag.
    NRF_UART0->EVENTS_ERROR = 0;
#if CORTEX_MODEL >= 4
    (void)NRF_UART0->EVENTS_ERROR;
#endif
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

#if NRF5_SERIAL_USE_UART0 == TRUE
  sdObjectInit(&SD1, NULL, notify1);
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

  osalDbgAssert(
      (config->rx_pad < TOTAL_GPIO_PADS) || (config->tx_pad < TOTAL_GPIO_PADS),
      "must configure at least an RX or TX pad");

  if (sdp->state == SD_STOP) {

#if NRF5_SERIAL_USE_UART0 == TRUE
    if (sdp == &SD1) {
      configure_uart(config);

      // Enable UART interrupt
      NRF_UART0->INTENCLR = (uint32_t)-1;
      NRF_UART0->INTENSET = UART_INTENSET_ERROR_Msk;
      if (config->rx_pad != NRF5_SERIAL_PAD_DISCONNECTED)
          NRF_UART0->INTENSET |= UART_INTENSET_RXDRDY_Msk;
      if (config->tx_pad != NRF5_SERIAL_PAD_DISCONNECTED)
          NRF_UART0->INTENSET |= UART_INTENSET_TXDRDY_Msk;

      nvicEnableVector(UART0_IRQn, NRF5_SERIAL_UART0_PRIORITY);

      if (config->rx_pad != NRF5_SERIAL_PAD_DISCONNECTED)
        NRF_UART0->TASKS_STARTRX = 1;
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
void sd_lld_stop(SerialDriver *sdp) {

  if (sdp->state == SD_READY) {

#if NRF5_SERIAL_USE_UART0 == TRUE
    if (&SD1 == sdp) {
      nvicDisableVector(UART0_IRQn);
      NRF_UART0->ENABLE = UART_ENABLE_ENABLE_Disabled;
    }
#endif
  }
}

#endif /* HAL_USE_SERIAL == TRUE */

/** @} */
