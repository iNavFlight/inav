/*
    Copyright (C) 2015 Stephen Caudle

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
 * @file    SPIv1/hal_spi_lld.c
 * @brief   NRF5 low level SPI driver code.
 *
 * @addtogroup SPI
 * @{
 */

#include "hal.h"

#if HAL_USE_SPI || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

#if NRF5_SPI_USE_SPI0 || defined(__DOXYGEN__)
/** @brief SPI1 driver identifier.*/
SPIDriver SPID1;
#endif

#if NRF5_SPI_USE_SPI1 || defined(__DOXYGEN__)
/** @brief SPI2 driver identifier.*/
SPIDriver SPID2;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Preloads the transmit FIFO.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 */
static void port_fifo_preload(SPIDriver *spip) {
  NRF_SPI_Type *port = spip->port;

  if (spip->txcnt > 0 && spip->txptr != NULL)
    port->TXD = *(uint8_t *)spip->txptr++;
  else
    port->TXD = 0xFF;
  spip->txcnt--;
}

#if defined(__GNUC__)
__attribute__((noinline))
#endif
/**
 * @brief   Common IRQ handler.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 */
static void serve_interrupt(SPIDriver *spip) {
  NRF_SPI_Type *port = spip->port;

  // Clear SPI READY event flag
  port->EVENTS_READY = 0;
#if CORTEX_MODEL >= 4
  (void)port->EVENTS_READY;
#endif
  
  if (spip->rxptr != NULL) {
    *(uint8_t *)spip->rxptr++ = port->RXD;
  }
  else {
    (void)port->RXD;
    if (--spip->rxcnt == 0) {
      osalDbgAssert(spip->txcnt == 0, "counter out of synch");
      /* Stops the IRQ sources.*/
      spip->port->INTENCLR = (SPI_INTENCLR_READY_Clear << SPI_INTENCLR_READY_Pos);
      /* Portable SPI ISR code defined in the high level driver, note, it is
         a macro.*/
      _spi_isr_code(spip);
      return;
    }
  }
  if (spip->txcnt > 0) {
    port_fifo_preload(spip);
  }
  else {
    spip->port->INTENCLR = (SPI_INTENCLR_READY_Clear << SPI_INTENCLR_READY_Pos);
    /* Portable SPI ISR code defined in the high level driver, note, it is
       a macro.*/
    _spi_isr_code(spip);
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if NRF5_SPI_USE_SPI0 || defined(__DOXYGEN__)
/**
 * @brief   SPI0 interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(Vector4C) {

  CH_IRQ_PROLOGUE();
  serve_interrupt(&SPID1);
  CH_IRQ_EPILOGUE();
}
#endif
#if NRF5_SPI_USE_SPI1 || defined(__DOXYGEN__)
/**
 * @brief   SPI1 interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(Vector50) {

  CH_IRQ_PROLOGUE();
  serve_interrupt(&SPID2);
  CH_IRQ_EPILOGUE();
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level SPI driver initialization.
 *
 * @notapi
 */
void spi_lld_init(void) {

#if NRF5_SPI_USE_SPI0
  spiObjectInit(&SPID1);
  SPID1.port = NRF_SPI0;
#endif
#if NRF5_SPI_USE_SPI1
  spiObjectInit(&SPID2);
  SPID2.port = NRF_SPI1;
#endif
}

/**
 * @brief   Configures and activates the SPI peripheral.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @notapi
 */
void spi_lld_start(SPIDriver *spip) {
  uint32_t config;

  if (spip->state == SPI_STOP) {
#if NRF5_SPI_USE_SPI0
    if (&SPID1 == spip)
      nvicEnableVector(SPI0_TWI0_IRQn, NRF5_SPI_SPI0_IRQ_PRIORITY);
#endif
#if NRF5_SPI_USE_SPI1
    if (&SPID2 == spip)
      nvicEnableVector(SPI1_TWI1_IRQn, NRF5_SPI_SPI1_IRQ_PRIORITY);
#endif
  }

  config = spip->config->lsbfirst ?
    (SPI_CONFIG_ORDER_LsbFirst << SPI_CONFIG_ORDER_Pos) :
    (SPI_CONFIG_ORDER_MsbFirst << SPI_CONFIG_ORDER_Pos);

  switch (spip->config->mode) {
    case 1:
      config |= (SPI_CONFIG_CPOL_ActiveLow << SPI_CONFIG_CPOL_Pos);
      config |= (SPI_CONFIG_CPHA_Trailing << SPI_CONFIG_CPHA_Pos);
      break;
    case 2:
      config |= (SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos);
      config |= (SPI_CONFIG_CPHA_Leading << SPI_CONFIG_CPHA_Pos);
      break;
    case 3:
      config |= (SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos);
      config |= (SPI_CONFIG_CPHA_Trailing << SPI_CONFIG_CPHA_Pos);
      break;
    default:
      config |= (SPI_CONFIG_CPOL_ActiveLow << SPI_CONFIG_CPOL_Pos);
      config |= (SPI_CONFIG_CPHA_Leading << SPI_CONFIG_CPHA_Pos);
      break;
  }

  /* Configuration.*/
  spip->port->CONFIG = config;
#if NRF_SERIES == 51
  spip->port->PSELSCK = spip->config->sckpad;
  spip->port->PSELMOSI = spip->config->mosipad;
  spip->port->PSELMISO = spip->config->misopad;
#else
  spip->port->PSEL.SCK = spip->config->sckpad;
  spip->port->PSEL.MOSI = spip->config->mosipad;
  spip->port->PSEL.MISO = spip->config->misopad;
#endif
  spip->port->FREQUENCY = spip->config->freq;
  spip->port->ENABLE = (SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos);

  /* clear events flag */
  spip->port->EVENTS_READY = 0;
#if CORTEX_MODEL >= 4
  (void)spip->port->EVENTS_READY;
#endif
}

/**
 * @brief   Deactivates the SPI peripheral.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @notapi
 */
void spi_lld_stop(SPIDriver *spip) {

  if (spip->state != SPI_STOP) {
    spip->port->ENABLE  = (SPI_ENABLE_ENABLE_Disabled << SPI_ENABLE_ENABLE_Pos);
    spip->port->INTENCLR = (SPI_INTENCLR_READY_Clear << SPI_INTENCLR_READY_Pos);
#if NRF5_SPI_USE_SPI0
    if (&SPID1 == spip)
      nvicDisableVector(SPI0_TWI0_IRQn);
#endif
#if NRF5_SPI_USE_SPI1
    if (&SPID2 == spip)
      nvicDisableVector(SPI1_TWI1_IRQn);
#endif
  }
}

/**
 * @brief   Asserts the slave select signal and prepares for transfers.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @notapi
 */
void spi_lld_select(SPIDriver *spip) {

  palClearPad(IOPORT1, spip->config->sspad);
}

/**
 * @brief   Deasserts the slave select signal.
 * @details The previously selected peripheral is unselected.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @notapi
 */
void spi_lld_unselect(SPIDriver *spip) {

  palSetPad(IOPORT1, spip->config->sspad);
}

/**
 * @brief   Ignores data on the SPI bus.
 * @details This function transmits a series of idle words on the SPI bus and
 *          ignores the received data. This function can be invoked even
 *          when a slave select signal has not been yet asserted.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 * @param[in] n         number of words to be ignored
 *
 * @notapi
 */
void spi_lld_ignore(SPIDriver *spip, size_t n) {

  spip->rxptr = NULL;
  spip->txptr = NULL;
  spip->rxcnt = spip->txcnt = n;
  port_fifo_preload(spip);
  spip->port->INTENSET = (SPI_INTENCLR_READY_Enabled << SPI_INTENCLR_READY_Pos);
}

/**
 * @brief   Exchanges data on the SPI bus.
 * @details This asynchronous function starts a simultaneous transmit/receive
 *          operation.
 * @post    At the end of the operation the configured callback is invoked.
 * @note    The buffers are organized as uint8_t arrays for data sizes below or
 *          equal to 8 bits else it is organized as uint16_t arrays.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 * @param[in] n         number of words to be exchanged
 * @param[in] txbuf     the pointer to the transmit buffer
 * @param[out] rxbuf    the pointer to the receive buffer
 *
 * @notapi
 */
void spi_lld_exchange(SPIDriver *spip, size_t n,
                      const void *txbuf, void *rxbuf) {

  spip->rxptr = rxbuf;
  spip->txptr = txbuf;
  spip->rxcnt = spip->txcnt = n;
  port_fifo_preload(spip);
  spip->port->INTENSET = (SPI_INTENCLR_READY_Enabled << SPI_INTENCLR_READY_Pos);
}

/**
 * @brief   Sends data over the SPI bus.
 * @details This asynchronous function starts a transmit operation.
 * @post    At the end of the operation the configured callback is invoked.
 * @note    The buffers are organized as uint8_t arrays for data sizes below or
 *          equal to 8 bits else it is organized as uint16_t arrays.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 * @param[in] n         number of words to send
 * @param[in] txbuf     the pointer to the transmit buffer
 *
 * @notapi
 */
void spi_lld_send(SPIDriver *spip, size_t n, const void *txbuf) {

  spip->rxptr = NULL;
  spip->txptr = txbuf;
  spip->rxcnt = spip->txcnt = n;
  port_fifo_preload(spip);
  spip->port->INTENSET = (SPI_INTENCLR_READY_Enabled << SPI_INTENCLR_READY_Pos);
}

/**
 * @brief   Receives data from the SPI bus.
 * @details This asynchronous function starts a receive operation.
 * @post    At the end of the operation the configured callback is invoked.
 * @note    The buffers are organized as uint8_t arrays for data sizes below or
 *          equal to 8 bits else it is organized as uint16_t arrays.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 * @param[in] n         number of words to receive
 * @param[out] rxbuf    the pointer to the receive buffer
 *
 * @notapi
 */
void spi_lld_receive(SPIDriver *spip, size_t n, void *rxbuf) {

  spip->rxptr = rxbuf;
  spip->txptr = NULL;
  spip->rxcnt = spip->txcnt = n;
  port_fifo_preload(spip);
  spip->port->INTENSET = (SPI_INTENCLR_READY_Enabled << SPI_INTENCLR_READY_Pos);
}

/**
 * @brief   Exchanges one frame using a polled wait.
 * @details This synchronous function exchanges one frame using a polled
 *          synchronization method. This function is useful when exchanging
 *          small amount of data on high speed channels, usually in this
 *          situation is much more efficient just wait for completion using
 *          polling than suspending the thread waiting for an interrupt.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 * @param[in] frame     the data frame to send over the SPI bus
 * @return              The received data frame from the SPI bus.
 */
uint16_t spi_lld_polled_exchange(SPIDriver *spip, uint16_t frame) {

  spip->port->TXD = (uint8_t)frame;
  while (spip->port->EVENTS_READY == 0)
    ;
  spip->port->EVENTS_READY = 0;
#if CORTEX_MODEL >= 4
  (void)spip->port->EVENTS_READY;
#endif
  return spip->port->RXD;
}

#endif /* HAL_USE_SPI */

/** @} */
