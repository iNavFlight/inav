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
 * @file    SSI/hal_spi_lld.c
 * @brief   TM4C123x/TM4C129x SPI subsystem low level driver.
 *
 * @addtogroup SPI
 * @{
 */

#include "ch.h"
#include "hal.h"

#if HAL_USE_SPI || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   SPI1 driver identifier.
 */
#if TIVA_SPI_USE_SSI0 || defined(__DOXYGEN__)
SPIDriver SPID1;
#endif

/**
 * @brief   SPI2 driver identifier.
 */
#if TIVA_SPI_USE_SSI1 || defined(__DOXYGEN__)
SPIDriver SPID2;
#endif

/**
 * @brief   SPI3 driver identifier.
 */
#if TIVA_SPI_USE_SSI2 || defined(__DOXYGEN__)
SPIDriver SPID3;
#endif

/**
 * @brief   SPI4 driver identifier.
 */
#if TIVA_SPI_USE_SSI3 || defined(__DOXYGEN__)
SPIDriver SPID4;
#endif

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

static uint16_t dummytx;
static uint16_t dummyrx;

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Common IRQ handler.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 */
static void spi_serve_interrupt(SPIDriver *spip)
{
  uint32_t ssi = spip->ssi;
  uint32_t mis = HWREG(ssi + SSI_O_MIS);
  uint32_t dmachis = HWREG(UDMA_CHIS);

  /* SPI error handling.*/
  if ((mis & (SSI_MIS_RORMIS | SSI_MIS_RTMIS)) != 0) {
    TIVA_SPI_SSI_ERROR_HOOK(spip);
  }

  if ((dmachis & ((1 << spip->dmarxnr) | (1 << spip->dmatxnr))) ==
      (uint32_t)((1 << spip->dmarxnr) | (1 << spip->dmatxnr))) {
    /* Clear DMA Channel interrupts.*/
    HWREG(UDMA_CHIS) = (1 << spip->dmarxnr) | (1 << spip->dmatxnr);

    /* Portable SPI ISR code defined in the high level driver, note, it is a
       macro.*/
    _spi_isr_code(spip);
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if TIVA_SPI_USE_SSI0 || defined(__DOXYGEN__)
/**
 * @brief   SSI0 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_SSI0_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  spi_serve_interrupt(&SPID1);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_SPI_USE_SSI1 || defined(__DOXYGEN__)
/**
 * @brief   SSI1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_SSI1_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  spi_serve_interrupt(&SPID2);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_SPI_USE_SSI2 || defined(__DOXYGEN__)
/**
 * @brief   SSI2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_SSI2_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  spi_serve_interrupt(&SPID3);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_SPI_USE_SSI3 || defined(__DOXYGEN__)
/**
 * @brief   SSI3 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_SSI3_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  spi_serve_interrupt(&SPID4);

  OSAL_IRQ_EPILOGUE();
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
void spi_lld_init(void) 
{
  dummytx = 0xFFFF;

#if TIVA_SPI_USE_SSI0
  spiObjectInit(&SPID1);
  SPID1.ssi = SSI0_BASE;
  SPID1.dmarxnr = TIVA_SPI_SSI0_RX_UDMA_CHANNEL;
  SPID1.dmatxnr = TIVA_SPI_SSI0_TX_UDMA_CHANNEL;
  SPID1.rxchnmap = TIVA_SPI_SSI0_RX_UDMA_MAPPING;
  SPID1.txchnmap = TIVA_SPI_SSI0_TX_UDMA_MAPPING;
#endif

#if TIVA_SPI_USE_SSI1
  spiObjectInit(&SPID2);
  SPID2.ssi = SSI1_BASE;
  SPID2.dmarxnr = TIVA_SPI_SSI1_RX_UDMA_CHANNEL;
  SPID2.dmatxnr = TIVA_SPI_SSI1_TX_UDMA_CHANNEL;
  SPID2.rxchnmap = TIVA_SPI_SSI1_RX_UDMA_MAPPING;
  SPID2.txchnmap = TIVA_SPI_SSI1_TX_UDMA_MAPPING;
#endif

#if TIVA_SPI_USE_SSI2
  spiObjectInit(&SPID3);
  SPID3.ssi = SSI2_BASE;
  SPID3.dmarxnr = TIVA_SPI_SSI2_RX_UDMA_CHANNEL;
  SPID3.dmatxnr = TIVA_SPI_SSI2_TX_UDMA_CHANNEL;
  SPID3.rxchnmap = TIVA_SPI_SSI2_RX_UDMA_MAPPING;
  SPID3.txchnmap = TIVA_SPI_SSI2_TX_UDMA_MAPPING;
#endif

#if TIVA_SPI_USE_SSI3
  spiObjectInit(&SPID4);
  SPID4.ssi = SSI3_BASE;
  SPID4.dmarxnr = TIVA_SPI_SSI3_RX_UDMA_CHANNEL;
  SPID4.dmatxnr = TIVA_SPI_SSI3_TX_UDMA_CHANNEL;
  SPID4.rxchnmap = TIVA_SPI_SSI3_RX_UDMA_MAPPING;
  SPID4.txchnmap = TIVA_SPI_SSI3_TX_UDMA_MAPPING;
#endif
}

/**
 * @brief   Configures and activates the SPI peripheral.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @notapi
 */
void spi_lld_start(SPIDriver *spip)
{
  if (spip->state == SPI_STOP) {
    /* Clock activation.*/
#if TIVA_SPI_USE_SSI0
    if (&SPID1 == spip) {
      bool b;
      b = udmaChannelAllocate(spip->dmarxnr);
      osalDbgAssert(!b, "channel already allocated");
      b = udmaChannelAllocate(spip->dmatxnr);
      osalDbgAssert(!b, "channel already allocated");

      /* Enable SSI0 module.*/
      HWREG(SYSCTL_RCGCSSI) |= (1 << 0);
      while (!(HWREG(SYSCTL_PRSSI) & (1 << 0)))
        ;

      nvicEnableVector(TIVA_SSI0_NUMBER, TIVA_SPI_SSI0_IRQ_PRIORITY);
    }
#endif
#if TIVA_SPI_USE_SSI1
    if (&SPID2 == spip) {
      bool b;
      b = udmaChannelAllocate(spip->dmarxnr);
      osalDbgAssert(!b, "channel already allocated");
      b = udmaChannelAllocate(spip->dmatxnr);
      osalDbgAssert(!b, "channel already allocated");

      /* Enable SSI0 module.*/
      HWREG(SYSCTL_RCGCSSI) |= (1 << 1);
      while (!(HWREG(SYSCTL_PRSSI) & (1 << 1)))
        ;

      nvicEnableVector(TIVA_SSI1_NUMBER, TIVA_SPI_SSI1_IRQ_PRIORITY);
    }
#endif
#if TIVA_SPI_USE_SSI2
    if (&SPID2 == spip) {
      bool b;
      b = udmaChannelAllocate(spip->dmarxnr);
      osalDbgAssert(!b, "channel already allocated");
      b = udmaChannelAllocate(spip->dmatxnr);
      osalDbgAssert(!b, "channel already allocated");

      /* Enable SSI0 module.*/
      HWREG(SYSCTL_RCGCSSI) |= (1 << 2);
      while (!(HWREG(SYSCTL_PRSSI) & (1 << 2)))
        ;

      nvicEnableVector(TIVA_SSI2_NUMBER, TIVA_SPI_SSI2_IRQ_PRIORITY);
    }
#endif
#if TIVA_SPI_USE_SSI3
    if (&SPID2 == spip) {
      bool b;
      b = udmaChannelAllocate(spip->dmarxnr);
      osalDbgAssert(!b, "channel already allocated");
      b = udmaChannelAllocate(spip->dmatxnr);
      osalDbgAssert(!b, "channel already allocated");

      /* Enable SSI0 module.*/
      HWREG(SYSCTL_RCGCSSI) |= (1 << 3);
      while (!(HWREG(SYSCTL_PRSSI) & (1 << 3)))
        ;

      nvicEnableVector(TIVA_SSI3_NUMBER, TIVA_SPI_SSI3_IRQ_PRIORITY);
    }
#endif

    HWREG(UDMA_CHMAP0 + (spip->dmarxnr / 8) * 4) |= (spip->rxchnmap << (spip->dmarxnr % 8));
    HWREG(UDMA_CHMAP0 + (spip->dmatxnr / 8) * 4) |= (spip->txchnmap << (spip->dmatxnr % 8));
  }
  /* Set master operation mode.*/
  HWREG(spip->ssi + SSI_O_CR1) = 0;

  /* Clock configuration - System Clock.*/
  HWREG(spip->ssi + SSI_O_CC) = 0;

  /* Clear pending interrupts.*/
  HWREG(spip->ssi + SSI_O_ICR) = SSI_ICR_RTIC | SSI_ICR_RORIC;

  /* Enable Receive Time-Out and Receive Overrun Interrupts.*/
  HWREG(spip->ssi + SSI_O_IM) = SSI_IM_RTIM | SSI_IM_RORIM;

  /* Configure the clock prescale divisor.*/
  HWREG(spip->ssi + SSI_O_CPSR) = spip->config->cpsr;

  /* Serial clock rate, phase/polarity, data size, fixed SPI frame format.*/
  HWREG(spip->ssi + SSI_O_CR0) = (spip->config->cr0 & ~SSI_CR0_FRF_M) | SSI_CR0_FRF_MOTO;

  /* Enable SSI.*/
  HWREG(spip->ssi + SSI_O_CR1) |= SSI_CR1_SSE;

  /* Enable RX and TX DMA channels.*/
  HWREG(spip->ssi + SSI_O_DMACTL) = (SSI_DMACTL_TXDMAE | SSI_DMACTL_RXDMAE);
}

/**
 * @brief   Deactivates the SPI peripheral.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @notapi
 */
void spi_lld_stop(SPIDriver *spip)
{
  if (spip->state != SPI_STOP) {
    HWREG(spip->ssi + SSI_O_CR1) = 0;
    HWREG(spip->ssi + SSI_O_CR0) = 0;
    HWREG(spip->ssi + SSI_O_CPSR) = 0;

    udmaChannelRelease(spip->dmarxnr);
    udmaChannelRelease(spip->dmatxnr);

#if TIVA_SPI_USE_SSI0
    if (&SPID1 == spip) {
      nvicDisableVector(TIVA_SSI0_NUMBER);
    }
#endif
#if TIVA_SPI_USE_SSI1
    if (&SPID2 == spip) {
      nvicDisableVector(TIVA_SSI1_NUMBER);
    }
#endif
#if TIVA_SPI_USE_SSI2
    if (&SPID3 == spip) {
      nvicDisableVector(TIVA_SSI2_NUMBER);
    }
#endif
#if TIVA_SPI_USE_SSI3
    if (&SPID4 == spip) {
      nvicDisableVector(TIVA_SSI3_NUMBER);
    }
#endif
  }
}

#if (SPI_SELECT_MODE == SPI_SELECT_MODE_LLD) || defined(__DOXYGEN__)
/**
 * @brief   Asserts the slave select signal and prepares for transfers.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @notapi
 */
void spi_lld_select(SPIDriver *spip)
{
  /* No implementation on Tiva.*/
}

/**
 * @brief   Deasserts the slave select signal.
 * @details The previously selected peripheral is unselected.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @notapi
 */
void spi_lld_unselect(SPIDriver *spip)
{
  /* No implementation on Tiva.*/
}
#endif

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
void spi_lld_ignore(SPIDriver *spip, size_t n)
{
  tiva_udma_table_entry_t *primary = udmaControlTable.primary;

  if ((spip->config->cr0 & SSI_CR0_DSS_M) < 8) {
    /* Configure for 8-bit transfers.*/
    primary[spip->dmatxnr].srcendp = (volatile void *)&dummytx;
    primary[spip->dmatxnr].dstendp = (void *)(spip->ssi + SSI_O_DR);
    primary[spip->dmatxnr].chctl = UDMA_CHCTL_DSTSIZE_8 | UDMA_CHCTL_DSTINC_NONE |
                                   UDMA_CHCTL_SRCSIZE_8 | UDMA_CHCTL_SRCINC_NONE |
                                   UDMA_CHCTL_ARBSIZE_4 |
                                   UDMA_CHCTL_XFERSIZE(n) |
                                   UDMA_CHCTL_XFERMODE_BASIC;

    primary[spip->dmarxnr].srcendp = (void *)(spip->ssi + SSI_O_DR);
    primary[spip->dmarxnr].dstendp = &dummyrx;
    primary[spip->dmarxnr].chctl = UDMA_CHCTL_DSTSIZE_8 | UDMA_CHCTL_DSTINC_NONE |
                                   UDMA_CHCTL_SRCSIZE_8 | UDMA_CHCTL_SRCINC_NONE |
                                   UDMA_CHCTL_ARBSIZE_4 |
                                   UDMA_CHCTL_XFERSIZE(n) |
                                   UDMA_CHCTL_XFERMODE_BASIC;
  }
  else {
    /* Configure for 16-bit transfers.*/
    primary[spip->dmatxnr].srcendp = (volatile void *)&dummytx;
    primary[spip->dmatxnr].dstendp = (void *)(spip->ssi + SSI_O_DR);
    primary[spip->dmatxnr].chctl = UDMA_CHCTL_DSTSIZE_16 | UDMA_CHCTL_DSTINC_NONE |
                                   UDMA_CHCTL_SRCSIZE_16 | UDMA_CHCTL_SRCINC_NONE |
                                   UDMA_CHCTL_ARBSIZE_4 |
                                   UDMA_CHCTL_XFERSIZE(n) |
                                   UDMA_CHCTL_XFERMODE_BASIC;

    primary[spip->dmarxnr].srcendp = (void *)(spip->ssi + SSI_O_DR);
    primary[spip->dmarxnr].dstendp = &dummyrx;
    primary[spip->dmarxnr].chctl = UDMA_CHCTL_DSTSIZE_16 | UDMA_CHCTL_DSTINC_NONE |
                                   UDMA_CHCTL_SRCSIZE_16 | UDMA_CHCTL_SRCINC_NONE |
                                   UDMA_CHCTL_ARBSIZE_4 |
                                   UDMA_CHCTL_XFERSIZE(n) |
                                   UDMA_CHCTL_XFERMODE_BASIC;
  }

  dmaChannelSingleBurst(spip->dmatxnr);
  dmaChannelPrimary(spip->dmatxnr);
  dmaChannelPriorityDefault(spip->dmatxnr);
  dmaChannelEnableRequest(spip->dmatxnr);

  dmaChannelSingleBurst(spip->dmarxnr);
  dmaChannelPrimary(spip->dmarxnr);
  dmaChannelPriorityDefault(spip->dmarxnr);
  dmaChannelEnableRequest(spip->dmarxnr);

  /* Enable DMA channels, when the TX channel is enabled the transfer starts.*/
  dmaChannelEnable(spip->dmarxnr);
  dmaChannelEnable(spip->dmatxnr);
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
void spi_lld_exchange(SPIDriver *spip, size_t n, const void *txbuf, void *rxbuf)
{
  tiva_udma_table_entry_t *primary = udmaControlTable.primary;

  if ((spip->config->cr0 & SSI_CR0_DSS_M) < 8) {
    /* Configure for 8-bit transfers.*/
    primary[spip->dmatxnr].srcendp = (volatile void *)txbuf+n-1;
    primary[spip->dmatxnr].dstendp = (void *)(spip->ssi + SSI_O_DR);
    primary[spip->dmatxnr].chctl = UDMA_CHCTL_DSTSIZE_8 | UDMA_CHCTL_DSTINC_NONE |
                                   UDMA_CHCTL_SRCSIZE_8 | UDMA_CHCTL_SRCINC_8 |
                                   UDMA_CHCTL_ARBSIZE_4 |
                                   UDMA_CHCTL_XFERSIZE(n) |
                                   UDMA_CHCTL_XFERMODE_BASIC;

    primary[spip->dmarxnr].srcendp = (void *)(spip->ssi + SSI_O_DR);
    primary[spip->dmarxnr].dstendp = rxbuf+n-1;
    primary[spip->dmarxnr].chctl = UDMA_CHCTL_DSTSIZE_8 | UDMA_CHCTL_DSTINC_8 |
                                   UDMA_CHCTL_SRCSIZE_8 | UDMA_CHCTL_SRCINC_NONE |
                                   UDMA_CHCTL_ARBSIZE_4 |
                                   UDMA_CHCTL_XFERSIZE(n) |
                                   UDMA_CHCTL_XFERMODE_BASIC;
  }
  else {
    /* Configure for 16-bit transfers.*/
    primary[spip->dmatxnr].srcendp = (volatile void *)txbuf+(n*2)-1;
    primary[spip->dmatxnr].dstendp = (void *)(spip->ssi + SSI_O_DR);
    primary[spip->dmatxnr].chctl = UDMA_CHCTL_DSTSIZE_16 | UDMA_CHCTL_DSTINC_NONE |
                                   UDMA_CHCTL_SRCSIZE_16 | UDMA_CHCTL_SRCINC_16 |
                                   UDMA_CHCTL_ARBSIZE_4 |
                                   UDMA_CHCTL_XFERSIZE(n) |
                                   UDMA_CHCTL_XFERMODE_BASIC;

    primary[spip->dmarxnr].srcendp = (void *)(spip->ssi + SSI_O_DR);
    primary[spip->dmarxnr].dstendp = rxbuf+(n*2)-1;
    primary[spip->dmarxnr].chctl = UDMA_CHCTL_DSTSIZE_16 | UDMA_CHCTL_DSTINC_16 |
                                   UDMA_CHCTL_SRCSIZE_16 | UDMA_CHCTL_SRCINC_NONE |
                                   UDMA_CHCTL_ARBSIZE_4 |
                                   UDMA_CHCTL_XFERSIZE(n) |
                                   UDMA_CHCTL_XFERMODE_BASIC;
  }

  dmaChannelSingleBurst(spip->dmatxnr);
  dmaChannelPrimary(spip->dmatxnr);
  dmaChannelPriorityDefault(spip->dmatxnr);
  dmaChannelEnableRequest(spip->dmatxnr);

  dmaChannelSingleBurst(spip->dmarxnr);
  dmaChannelPrimary(spip->dmarxnr);
  dmaChannelPriorityDefault(spip->dmarxnr);
  dmaChannelEnableRequest(spip->dmarxnr);

  /* Enable DMA channels, when the TX channel is enabled the transfer starts.*/
  dmaChannelEnable(spip->dmarxnr);
  dmaChannelEnable(spip->dmatxnr);
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
void spi_lld_send(SPIDriver *spip, size_t n, const void *txbuf)
{
  tiva_udma_table_entry_t *primary = udmaControlTable.primary;

  if ((spip->config->cr0 & SSI_CR0_DSS_M) < 8) {
    /* Configure for 8-bit transfers.*/
    primary[spip->dmatxnr].srcendp = (volatile void *)txbuf+n-1;
    primary[spip->dmatxnr].dstendp = (void *)(spip->ssi + SSI_O_DR);
    primary[spip->dmatxnr].chctl = UDMA_CHCTL_DSTSIZE_8 | UDMA_CHCTL_DSTINC_NONE |
                                   UDMA_CHCTL_SRCSIZE_8 | UDMA_CHCTL_SRCINC_8 |
                                   UDMA_CHCTL_ARBSIZE_4 |
                                   UDMA_CHCTL_XFERSIZE(n) |
                                   UDMA_CHCTL_XFERMODE_BASIC;

    primary[spip->dmarxnr].dstendp = (void *)(spip->ssi + SSI_O_DR);
    primary[spip->dmarxnr].srcendp = &dummyrx;
    primary[spip->dmarxnr].chctl = UDMA_CHCTL_DSTSIZE_8 | UDMA_CHCTL_DSTINC_NONE |
                                   UDMA_CHCTL_SRCSIZE_8 | UDMA_CHCTL_SRCINC_NONE |
                                   UDMA_CHCTL_ARBSIZE_4 |
                                   UDMA_CHCTL_XFERSIZE(n) |
                                   UDMA_CHCTL_XFERMODE_BASIC;
  }
  else {
    /* Configure for 16-bit transfers.*/
    primary[spip->dmatxnr].srcendp = (volatile void *)txbuf+(n*2)-1;
    primary[spip->dmatxnr].dstendp = (void *)(spip->ssi + SSI_O_DR);
    primary[spip->dmatxnr].chctl = UDMA_CHCTL_DSTSIZE_16 | UDMA_CHCTL_DSTINC_NONE |
                                   UDMA_CHCTL_SRCSIZE_16 | UDMA_CHCTL_SRCINC_16 |
                                   UDMA_CHCTL_ARBSIZE_4 |
                                   UDMA_CHCTL_XFERSIZE(n) |
                                   UDMA_CHCTL_XFERMODE_BASIC;

    primary[spip->dmarxnr].dstendp = (void *)(spip->ssi + SSI_O_DR);
    primary[spip->dmarxnr].srcendp = &dummyrx;
    primary[spip->dmarxnr].chctl = UDMA_CHCTL_DSTSIZE_16 | UDMA_CHCTL_DSTINC_NONE |
                                   UDMA_CHCTL_SRCSIZE_16 | UDMA_CHCTL_SRCINC_NONE |
                                   UDMA_CHCTL_ARBSIZE_4 |
                                   UDMA_CHCTL_XFERSIZE(n) |
                                   UDMA_CHCTL_XFERMODE_BASIC;
  }

  dmaChannelSingleBurst(spip->dmatxnr);
  dmaChannelPrimary(spip->dmatxnr);
  dmaChannelPriorityDefault(spip->dmatxnr);
  dmaChannelEnableRequest(spip->dmatxnr);

  dmaChannelSingleBurst(spip->dmarxnr);
  dmaChannelPrimary(spip->dmarxnr);
  dmaChannelPriorityDefault(spip->dmarxnr);
  dmaChannelEnableRequest(spip->dmarxnr);

  /* Enable DMA channels, when the TX channel is enabled the transfer starts.*/
  dmaChannelEnable(spip->dmarxnr);
  dmaChannelEnable(spip->dmatxnr);
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
void spi_lld_receive(SPIDriver *spip, size_t n, void *rxbuf) 
{
  tiva_udma_table_entry_t *primary = udmaControlTable.primary;

  if ((spip->config->cr0 & SSI_CR0_DSS_M) < 8) {
    /* Configure for 8-bit transfers.*/
    primary[spip->dmatxnr].srcendp = (volatile void *)&dummytx;
    primary[spip->dmatxnr].dstendp = (void *)(spip->ssi + SSI_O_DR);
    primary[spip->dmatxnr].chctl = UDMA_CHCTL_DSTSIZE_8 | UDMA_CHCTL_DSTINC_NONE |
                                   UDMA_CHCTL_SRCSIZE_8 | UDMA_CHCTL_SRCINC_NONE |
                                   UDMA_CHCTL_ARBSIZE_4 |
                                   UDMA_CHCTL_XFERSIZE(n) |
                                   UDMA_CHCTL_XFERMODE_BASIC;

    primary[spip->dmarxnr].srcendp = (void *)(spip->ssi + SSI_O_DR);
    primary[spip->dmarxnr].dstendp = rxbuf+n-1;
    primary[spip->dmarxnr].chctl = UDMA_CHCTL_DSTSIZE_8 | UDMA_CHCTL_DSTINC_8 |
                                   UDMA_CHCTL_SRCSIZE_8 | UDMA_CHCTL_SRCINC_NONE |
                                   UDMA_CHCTL_ARBSIZE_4 |
                                   UDMA_CHCTL_XFERSIZE(n) |
                                   UDMA_CHCTL_XFERMODE_BASIC;
  }
  else {
    /* Configure for 16-bit transfers.*/
    primary[spip->dmatxnr].srcendp = (volatile void *)&dummytx;
    primary[spip->dmatxnr].dstendp = (void *)(spip->ssi + SSI_O_DR);
    primary[spip->dmatxnr].chctl = UDMA_CHCTL_DSTSIZE_16 | UDMA_CHCTL_DSTINC_NONE |
                                   UDMA_CHCTL_SRCSIZE_16 | UDMA_CHCTL_SRCINC_NONE |
                                   UDMA_CHCTL_ARBSIZE_4 |
                                   UDMA_CHCTL_XFERSIZE(n) |
                                   UDMA_CHCTL_XFERMODE_BASIC;

    primary[spip->dmarxnr].srcendp = (void *)(spip->ssi + SSI_O_DR);
    primary[spip->dmarxnr].dstendp = rxbuf+(n*2)-1;
    primary[spip->dmarxnr].chctl = UDMA_CHCTL_DSTSIZE_16 | UDMA_CHCTL_DSTINC_16 |
                                   UDMA_CHCTL_SRCSIZE_16 | UDMA_CHCTL_SRCINC_NONE |
                                   UDMA_CHCTL_ARBSIZE_4 |
                                   UDMA_CHCTL_XFERSIZE(n) |
                                   UDMA_CHCTL_XFERMODE_BASIC;
  }

  dmaChannelSingleBurst(spip->dmatxnr);
  dmaChannelPrimary(spip->dmatxnr);
  dmaChannelPriorityDefault(spip->dmatxnr);
  dmaChannelEnableRequest(spip->dmatxnr);

  dmaChannelSingleBurst(spip->dmarxnr);
  dmaChannelPrimary(spip->dmarxnr);
  dmaChannelPriorityDefault(spip->dmarxnr);
  dmaChannelEnableRequest(spip->dmarxnr);

  /* Enable DMA channels, when the TX channel is enabled the transfer starts.*/
  dmaChannelEnable(spip->dmarxnr);
  dmaChannelEnable(spip->dmatxnr);
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
uint16_t spi_lld_polled_exchange(SPIDriver *spip, uint16_t frame)
{
  HWREG(spip->ssi + SSI_O_DR) = (uint32_t)frame;
  while ((HWREG(spip->ssi + SSI_O_SR) & SSI_SR_RNE) == 0)
    ;
  return (uint16_t)HWREG(spip->ssi + SSI_O_DR);
}

#endif /* HAL_USE_SPI */

/** @} */
