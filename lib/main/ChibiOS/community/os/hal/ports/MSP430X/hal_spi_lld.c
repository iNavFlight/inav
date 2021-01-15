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
 * @file    hal_spi_lld.c
 * @brief   MSP430X SPI subsystem low level driver source.
 *
 * @addtogroup SPI
 * @{
 */

#include "hal.h"

#if (HAL_USE_SPI == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   SPIA0 driver identifier.
 */
#if (MSP430X_SPI_USE_SPIA0 == TRUE) || defined(__DOXYGEN__)
SPIDriver SPIDA0;
#endif

/**
 * @brief   SPIA1 driver identifier.
 */
#if (MSP430X_SPI_USE_SPIA1 == TRUE) || defined(__DOXYGEN__)
SPIDriver SPIDA1;
#endif

/**
 * @brief   SPIA2 driver identifier.
 */
#if (MSP430X_SPI_USE_SPIA2 == TRUE) || defined(__DOXYGEN__)
SPIDriver SPIDA2;
#endif

/**
 * @brief   SPIA3 driver identifier.
 */
#if (MSP430X_SPI_USE_SPIA3 == TRUE) || defined(__DOXYGEN__)
SPIDriver SPIDA3;
#endif

/**
 * @brief   SPIB0 driver identifier.
 */
#if (MSP430X_SPI_USE_SPIB0 == TRUE) || defined(__DOXYGEN__)
SPIDriver SPIDB0;
#endif

/**
 * @brief   SPIB1 driver identifier.
 */
#if (MSP430X_SPI_USE_SPIB1 == TRUE) || defined(__DOXYGEN__)
SPIDriver SPIDB1;
#endif

/**
 * @brief   SPIB2 driver identifier.
 */
#if (MSP430X_SPI_USE_SPIB2 == TRUE) || defined(__DOXYGEN__)
SPIDriver SPIDB2;
#endif

/**
 * @brief   SPIB3 driver identifier.
 */
#if (MSP430X_SPI_USE_SPIB3 == TRUE) || defined(__DOXYGEN__)
SPIDriver SPIDB3;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

static const uint16_t dummytx = 0xFFFFU;
static uint16_t dummyrx;

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

static void init_transfer(SPIDriver * spip) {

#if MSP430X_SPI_EXCLUSIVE_DMA == TRUE || defined(__DOXYGEN__)
  if (spip->config->dmarx_index >= MSP430X_DMA_CHANNELS) {
    dmaRequestS(&(spip->rx_req), TIME_INFINITE);
  }
  else {
    dmaTransfer(&(spip->dmarx), &(spip->rx_req));
  }
  if (spip->config->dmatx_index >= MSP430X_DMA_CHANNELS) {
    dmaRequestS(&(spip->tx_req), TIME_INFINITE);
  }
  else {
    dmaTransfer(&(spip->dmatx), &(spip->tx_req));
  }
#else
  dmaRequestS(&(spip->rx_req), TIME_INFINITE);
  dmaRequestS(&(spip->tx_req), TIME_INFINITE);
#endif

  *(spip->ifg) |= UCTXIFG;
}

/**
 * @brief   Shared end-of-transfer callback.
 *
 * @param[in] spip    pointer to the @p SPIDriver object
 * @note    This function is called in ISR context by the DMA code.
 */
static void spi_lld_end_of_transfer(void * spip) {

  /* So that future transfers will actually work */
  *(((SPIDriver *)spip)->ifg) &= ~(UCTXIFG);
  /* NOTE to future me - this macro sets the driver state and calls the
   * configured callback end_cb, if applicable. That callback doesn't seem to
   * be modifiable without reconfiguring the whole driver. */
  _spi_isr_code((SPIDriver *)spip);
}
/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level SPI driver initialization.
 *
 * @notapi
 */
void spi_lld_init(void) {

#if MSP430X_SPI_USE_SPIA0 == TRUE
  /* Driver initialization.*/
  spiObjectInit(&SPIDA0);
  SPIDA0.regs                     = (msp430x_spi_reg_t *)(&UCA0CTLW0);
  SPIDA0.ifg                      = (volatile uint16_t *)&UCA0IFG;
  SPIDA0.tx_req.trigger           = DMA_TRIGGER_MNEM(UCA0TXIFG);
  SPIDA0.rx_req.trigger           = DMA_TRIGGER_MNEM(UCA0RXIFG);
  SPIDA0.tx_req.dest_addr         = &(SPIDA0.regs->txbuf);
  SPIDA0.rx_req.source_addr       = &(SPIDA0.regs->rxbuf);
  SPIDA0.tx_req.data_mode         = MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE;
  SPIDA0.rx_req.data_mode         = MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE;
  SPIDA0.tx_req.transfer_mode     = MSP430X_DMA_SINGLE;
  SPIDA0.rx_req.transfer_mode     = MSP430X_DMA_SINGLE;
  SPIDA0.tx_req.callback.callback = NULL;
  SPIDA0.tx_req.callback.args     = NULL;
  SPIDA0.rx_req.callback.callback = spi_lld_end_of_transfer;
  SPIDA0.rx_req.callback.args     = &SPIDA0;
/* NOTE to my future self - this must be SINGLE because BLOCK and BURST
 * don't wait for triggers and would overflow both buffers. Don't worry, it
 * still works - the transfer isn't complete until SZ bytes are transferred */
#endif

#if MSP430X_SPI_USE_SPIA1 == TRUE
  /* Driver initialization.*/
  spiObjectInit(&SPIDA1);
  SPIDA1.regs                     = (msp430x_spi_reg_t *)(&UCA1CTLW0);
  SPIDA1.ifg                      = (volatile uint16_t *)&UCA1IFG;
  SPIDA1.tx_req.trigger           = DMA_TRIGGER_MNEM(UCA1TXIFG);
  SPIDA1.rx_req.trigger           = DMA_TRIGGER_MNEM(UCA1RXIFG);
  SPIDA1.tx_req.dest_addr         = &(SPIDA1.regs->txbuf);
  SPIDA1.rx_req.source_addr       = &(SPIDA1.regs->rxbuf);
  SPIDA1.tx_req.data_mode         = MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE;
  SPIDA1.rx_req.data_mode         = MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE;
  SPIDA1.tx_req.transfer_mode     = MSP430X_DMA_SINGLE;
  SPIDA1.rx_req.transfer_mode     = MSP430X_DMA_SINGLE;
  SPIDA1.tx_req.callback.callback = NULL;
  SPIDA1.tx_req.callback.args     = NULL;
  SPIDA1.rx_req.callback.callback = spi_lld_end_of_transfer;
  SPIDA1.rx_req.callback.args     = &SPIDA1;
#endif

#if MSP430X_SPI_USE_SPIA2 == TRUE
  /* Driver initialization.*/
  spiObjectInit(&SPIDA2);
  SPIDA2.regs                     = (msp430x_spi_reg_t *)(&UCA2CTLW0);
  SPIDA2.ifg                      = (volatile uint16_t *)&UCA2IFG;
  SPIDA2.tx_req.trigger           = DMA_TRIGGER_MNEM(UCA2TXIFG);
  SPIDA2.rx_req.trigger           = DMA_TRIGGER_MNEM(UCA2RXIFG);
  SPIDA2.tx_req.dest_addr         = &(SPIDA2.regs->txbuf);
  SPIDA2.rx_req.source_addr       = &(SPIDA2.regs->rxbuf);
  SPIDA2.tx_req.data_mode         = MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE;
  SPIDA2.rx_req.data_mode         = MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE;
  SPIDA2.tx_req.transfer_mode     = MSP430X_DMA_SINGLE;
  SPIDA2.rx_req.transfer_mode     = MSP430X_DMA_SINGLE;
  SPIDA2.tx_req.callback.callback = NULL;
  SPIDA2.tx_req.callback.args     = NULL;
  SPIDA2.rx_req.callback.callback = spi_lld_end_of_transfer;
  SPIDA2.rx_req.callback.args     = &SPIDA2;
#endif

#if MSP430X_SPI_USE_SPIA3 == TRUE
  /* Driver initialization.*/
  spiObjectInit(&SPIDA3);
  SPIDA3.regs                     = (msp430x_spi_reg_t *)(&UCA3CTLW0);
  SPIDA3.ifg                      = (volatile uint16_t *)&UCA3IFG;
  SPIDA3.tx_req.trigger           = DMA_TRIGGER_MNEM(UCA3TXIFG);
  SPIDA3.rx_req.trigger           = DMA_TRIGGER_MNEM(UCA3RXIFG);
  SPIDA3.tx_req.dest_addr         = &(SPIDA3.regs->txbuf);
  SPIDA3.rx_req.source_addr       = &(SPIDA3.regs->rxbuf);
  SPIDA3.tx_req.data_mode         = MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE;
  SPIDA3.rx_req.data_mode         = MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE;
  SPIDA3.tx_req.transfer_mode     = MSP430X_DMA_SINGLE;
  SPIDA3.rx_req.transfer_mode     = MSP430X_DMA_SINGLE;
  SPIDA3.tx_req.callback.callback = NULL;
  SPIDA3.tx_req.callback.args     = NULL;
  SPIDA3.rx_req.callback.callback = spi_lld_end_of_transfer;
  SPIDA3.rx_req.callback.args     = &SPIDA3;
#endif

#if MSP430X_SPI_USE_SPIB0 == TRUE
  /* Driver initialization.*/
  spiObjectInit(&SPIDB0);
  SPIDB0.regs                     = (msp430x_spi_reg_t *)(&UCB0CTLW0);
  SPIDB0.ifg                      = (volatile uint16_t *)&UCB0IFG;
  SPIDB0.tx_req.trigger           = DMA_TRIGGER_MNEM(UCB0TXIFG0);
  SPIDB0.rx_req.trigger           = DMA_TRIGGER_MNEM(UCB0RXIFG0);
  SPIDB0.tx_req.dest_addr         = &(SPIDB0.regs->txbuf);
  SPIDB0.rx_req.source_addr       = &(SPIDB0.regs->rxbuf);
  SPIDB0.tx_req.data_mode         = MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE;
  SPIDB0.rx_req.data_mode         = MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE;
  SPIDB0.tx_req.transfer_mode     = MSP430X_DMA_SINGLE;
  SPIDB0.rx_req.transfer_mode     = MSP430X_DMA_SINGLE;
  SPIDB0.tx_req.callback.callback = NULL;
  SPIDB0.tx_req.callback.args     = NULL;
  SPIDB0.rx_req.callback.callback = spi_lld_end_of_transfer;
  SPIDB0.rx_req.callback.args     = &SPIDB0;
#endif

#if MSP430X_SPI_USE_SPIB1 == TRUE
  /* Driver initialization.*/
  spiObjectInit(&SPIDB1);
  SPIDB1.regs                     = (msp430x_spi_reg_t *)(&UCB1CTLW0);
  SPIDB1.ifg                      = (volatile uint16_t *)&UCB1IFG;
  SPIDB1.tx_req.trigger           = DMA_TRIGGER_MNEM(UCB1TXIFG0);
  SPIDB1.rx_req.trigger           = DMA_TRIGGER_MNEM(UCB1RXIFG0);
  SPIDB1.tx_req.dest_addr         = &(SPIDB1.regs->txbuf);
  SPIDB1.rx_req.source_addr       = &(SPIDB1.regs->rxbuf);
  SPIDB1.tx_req.data_mode         = MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE;
  SPIDB1.rx_req.data_mode         = MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE;
  SPIDB1.tx_req.transfer_mode     = MSP430X_DMA_SINGLE;
  SPIDB1.rx_req.transfer_mode     = MSP430X_DMA_SINGLE;
  SPIDB1.tx_req.callback.callback = NULL;
  SPIDB1.tx_req.callback.args     = NULL;
  SPIDB1.rx_req.callback.callback = spi_lld_end_of_transfer;
  SPIDB1.rx_req.callback.args     = &SPIDB1;
#endif

#if MSP430X_SPI_USE_SPIB2 == TRUE
  /* Driver initialization.*/
  spiObjectInit(&SPIDB2);
  SPIDB2.regs                     = (msp430x_spi_reg_t *)(&UCB2CTLW0);
  SPIDB2.ifg                      = (volatile uint16_t *)&UCB2IFG;
  SPIDB2.tx_req.trigger           = DMA_TRIGGER_MNEM(UCB2TXIFG0);
  SPIDB2.rx_req.trigger           = DMA_TRIGGER_MNEM(UCB2RXIFG0);
  SPIDB2.tx_req.dest_addr         = &(SPIDB2.regs->txbuf);
  SPIDB2.rx_req.source_addr       = &(SPIDB2.regs->rxbuf);
  SPIDB2.tx_req.data_mode         = MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE;
  SPIDB2.rx_req.data_mode         = MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE;
  SPIDB2.tx_req.transfer_mode     = MSP430X_DMA_SINGLE;
  SPIDB2.rx_req.transfer_mode     = MSP430X_DMA_SINGLE;
  SPIDB2.tx_req.callback.callback = NULL;
  SPIDB2.tx_req.callback.args     = NULL;
  SPIDB2.rx_req.callback.callback = spi_lld_end_of_transfer;
  SPIDB2.rx_req.callback.args     = &SPIDB2;
#endif

#if MSP430X_SPI_USE_SPIB3 == TRUE
  /* Driver initialization.*/
  spiObjectInit(&SPIDB3);
  SPIDB3.regs                     = (msp430x_spi_reg_t *)(&UCB3CTLW0);
  SPIDB3.ifg                      = (volatile uint16_t *)&UCB3IFG;
  SPIDB3.tx_req.trigger           = DMA_TRIGGER_MNEM(UCB3TXIFG0);
  SPIDB3.rx_req.trigger           = DMA_TRIGGER_MNEM(UCB3RXIFG0);
  SPIDB3.tx_req.dest_addr         = &(SPIDB3.regs->txbuf);
  SPIDB3.rx_req.source_addr       = &(SPIDB3.regs->rxbuf);
  SPIDB3.tx_req.data_mode         = MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE;
  SPIDB3.rx_req.data_mode         = MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE;
  SPIDB3.tx_req.transfer_mode     = MSP430X_DMA_SINGLE;
  SPIDB3.rx_req.transfer_mode     = MSP430X_DMA_SINGLE;
  SPIDB3.tx_req.callback.callback = NULL;
  SPIDB3.tx_req.callback.args     = NULL;
  SPIDB3.rx_req.callback.callback = spi_lld_end_of_transfer;
  SPIDB3.rx_req.callback.args     = &SPIDB3;
#endif
}

/**
 * @brief   Configures and activates the SPI peripheral.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @notapi
 */
void spi_lld_start(SPIDriver * spip) {

  if (spip->state == SPI_STOP) {
/* Enables the peripheral.*/
#if MSP430X_SPI_EXCLUSIVE_DMA == TRUE
    /* Claim DMA streams here */
    bool b;
    if (spip->config->dmatx_index < MSP430X_DMA_CHANNELS) {
      b = dmaAcquireI(&(spip->dmatx), spip->config->dmatx_index);
      osalDbgAssert(!b, "stream already allocated");
    }
    if (spip->config->dmarx_index < MSP430X_DMA_CHANNELS) {
      b = dmaAcquireI(&(spip->dmarx), spip->config->dmarx_index);
      osalDbgAssert(!b, "stream already allocated");
    }
#endif /* MSP430X_SPI_EXCLUSIVE_DMA */
  }
  uint16_t brw = 0;
  uint8_t ssel = 0;
#if MSP430X_SPI_USE_SPIA0
  if (spip == &SPIDA0) {
    brw  = MSP430X_SPIA0_CLK_FREQ / spip->config->bit_rate;
    ssel = MSP430X_SPIA0_UCSSEL;
  }
#endif
#if MSP430X_SPI_USE_SPIA1
  if (spip == &SPIDA1) {
    brw  = MSP430X_SPIA1_CLK_FREQ / spip->config->bit_rate;
    ssel = MSP430X_SPIA1_UCSSEL;
  }
#endif
#if MSP430X_SPI_USE_SPIA2
  if (spip == &SPIDA2) {
    brw  = MSP430X_SPIA2_CLK_FREQ / spip->config->bit_rate;
    ssel = MSP430X_SPIA2_UCSSEL;
  }
#endif
#if MSP430X_SPI_USE_SPIA3
  if (spip == &SPIDA3) {
    brw  = MSP430X_SPIA3_CLK_FREQ / spip->config->bit_rate;
    ssel = MSP430X_SPIA3_UCSSEL;
  }
#endif
#if MSP430X_SPI_USE_SPIB0
  if (spip == &SPIDB0) {
    brw  = MSP430X_SPIB0_CLK_FREQ / spip->config->bit_rate;
    ssel = MSP430X_SPIB0_UCSSEL;
  }
#endif
#if MSP430X_SPI_USE_SPIB1
  if (spip == &SPIDB1) {
    brw  = MSP430X_SPIB1_CLK_FREQ / spip->config->bit_rate;
    ssel = MSP430X_SPIB1_UCSSEL;
  }
#endif
#if MSP430X_SPI_USE_SPIB2
  if (spip == &SPIDB2) {
    brw  = MSP430X_SPIB2_CLK_FREQ / spip->config->bit_rate;
    ssel = MSP430X_SPIB2_UCSSEL;
  }
#endif
#if MSP430X_SPI_USE_SPIB3
  if (spip == &SPIDB3) {
    brw  = MSP430X_SPIB3_CLK_FREQ / spip->config->bit_rate;
    ssel = MSP430X_SPIB3_UCSSEL;
  }
#endif
  /* Configures the peripheral.*/
  spip->regs->ctlw0 = UCSWRST;
  spip->regs->brw   = brw;
  spip->regs->ctlw0 =
      ((spip->config->spi_mode ^ 0x02) << 14) | (spip->config->bit_order << 13) |
      (spip->config->data_size << 12) | (UCMST) |
      ((spip->config->ss_line ? 0 : 2) << 9) | (UCSYNC) | (ssel) | (UCSTEM);
  *(spip->ifg) = 0;
  spi_lld_unselect(spip);
}

/**
 * @brief   Deactivates the SPI peripheral.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @notapi
 */
void spi_lld_stop(SPIDriver * spip) {

  if (spip->state == SPI_READY) {
/* Disables the peripheral.*/
#if MSP430X_SPI_EXCLUSIVE_DMA == TRUE
    if (spip->config->dmatx_index < MSP430X_DMA_CHANNELS) {
      dmaRelease(&(spip->dmatx));
    }
    if (spip->config->dmarx_index < MSP430X_DMA_CHANNELS) {
      dmaRelease(&(spip->dmarx));
    }
#endif
    spip->regs->ctlw0 = UCSWRST;
  }
}

/**
 * @brief   Asserts the slave select signal and prepares for transfers.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @notapi
 */
void spi_lld_select(SPIDriver * spip) {

  if (spip->config->ss_line) {
    palClearLine(spip->config->ss_line);
  }
}

/**
 * @brief   Deasserts the slave select signal.
 * @details The previously selected peripheral is unselected.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @notapi
 */
void spi_lld_unselect(SPIDriver * spip) {

  if (spip->config->ss_line) {
    palSetLine(spip->config->ss_line);
  }
}

/**
 * @brief   Ignores data on the SPI bus.
 * @details This asynchronous function starts the transmission of a series of
 *          idle bytes on the SPI bus and ignores the received data.
 * @post    At the end of the operation the configured callback is invoked.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 * @param[in] n         number of bytes to be ignored
 *
 * @notapi
 */
void spi_lld_ignore(SPIDriver * spip, size_t n) {

  spip->tx_req.source_addr = &dummytx;
  spip->tx_req.size        = n;
  spip->tx_req.addr_mode   = 0;

  spip->rx_req.dest_addr = &dummyrx;
  spip->rx_req.size      = n;
  spip->rx_req.addr_mode = 0;

  init_transfer(spip);
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
void spi_lld_exchange(SPIDriver * spip,
                      size_t n,
                      const void * txbuf,
                      void * rxbuf) {

  spip->tx_req.source_addr = txbuf;
  spip->tx_req.size        = n;
  spip->tx_req.addr_mode   = MSP430X_DMA_SRCINCR;

  spip->rx_req.dest_addr = rxbuf;
  spip->rx_req.size      = n;
  spip->rx_req.addr_mode = MSP430X_DMA_DSTINCR;

  init_transfer(spip);
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
void spi_lld_send(SPIDriver * spip, size_t n, const void * txbuf) {

  spip->tx_req.source_addr = txbuf;
  spip->tx_req.size        = n;
  spip->tx_req.addr_mode   = MSP430X_DMA_SRCINCR;

  spip->rx_req.dest_addr = &dummyrx;
  spip->rx_req.size      = n;
  spip->rx_req.addr_mode = 0;

  init_transfer(spip);
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
void spi_lld_receive(SPIDriver * spip, size_t n, void * rxbuf) {

  spip->tx_req.source_addr = &dummytx;
  spip->tx_req.size        = n;
  spip->tx_req.addr_mode   = 0;

  spip->rx_req.dest_addr = rxbuf;
  spip->rx_req.size      = n;
  spip->rx_req.addr_mode = MSP430X_DMA_DSTINCR;

  init_transfer(spip);
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
uint8_t spi_lld_polled_exchange(SPIDriver * spip, uint8_t frame) {

  spip->regs->txbuf = frame;
  while (!(*(spip->ifg) & UCRXIFG))
    ;
  *(spip->ifg) &= ~(UCRXIFG | UCTXIFG);
  return spip->regs->rxbuf;
}

#endif /* HAL_USE_SPI == TRUE */

/** @} */
