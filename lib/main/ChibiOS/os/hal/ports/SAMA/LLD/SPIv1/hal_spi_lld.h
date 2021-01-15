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
 * @file    hal_spi_lld.h
 * @brief   SAMA SPI subsystem low level driver header.
 *
 * @addtogroup SPI
 * @{
 */

#ifndef HAL_SPI_LLD_H
#define HAL_SPI_LLD_H

#if (HAL_USE_SPI == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   Circular mode support flag.
 */
#define SPI_SUPPORTS_CIRCULAR           FALSE

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   SPI0 driver enable switch.
 * @details If set to @p TRUE the support for SPI0 is included.
 */
#if !defined(SAMA_SPI_USE_SPI0) || defined(__DOXYGEN__)
#define SAMA_SPI_USE_SPI0                   FALSE
#endif

/**
 * @brief   SPI0 Generic clock enable.
 * @details If set to @p TRUE the support for GCLK SPI0 is included.
 */
#if !defined(SAMA_SPI0_USE_GCLK) || defined(__DOXYGEN__)
#define SAMA_SPI0_USE_GCLK                  FALSE
#endif

/**
 * @brief   SPI0 Generic clock source.
 */
#if !defined(SAMA_SPI0_GCLK_SOURCE) || defined(__DOXYGEN__)
#define SAMA_SPI0_GCLK_SOURCE               SAMA_GCLK_MCK_CLK
#endif

/**
 * @brief   SPI0 Generic clock div.
 */
#if !defined(SAMA_SPI0_GCLK_DIV) || defined(__DOXYGEN__)
#define SAMA_SPI0_GCLK_DIV                  21
#endif

/**
 * @brief   SPI1 driver enable switch.
 * @details If set to @p TRUE the support for SPI1 is included.
 */
#if !defined(SAMA_SPI_USE_SPI1) || defined(__DOXYGEN__)
#define SAMA_SPI_USE_SPI1                   FALSE
#endif

/**
 * @brief   SPI1 Generic clock enable.
 * @details If set to @p TRUE the support for GCLK SPI1 is included.
 */
#if !defined(SAMA_SPI1_USE_GCLK) || defined(__DOXYGEN__)
#define SAMA_SPI1_USE_GCLK                  FALSE
#endif

/**
 * @brief   SPI1 Generic clock source.
 */
#if !defined(SAMA_SPI1_GCLK_SOURCE) || defined(__DOXYGEN__)
#define SAMA_SPI1_GCLK_SOURCE               SAMA_GCLK_MCK_CLK
#endif

/**
 * @brief   SPI1 Generic clock div.
 */
#if !defined(SAMA_SPI1_GCLK_DIV) || defined(__DOXYGEN__)
#define SAMA_SPI1_GCLK_DIV                  21
#endif

/**
 * @brief   SPI FLEXCOM0 driver enable switch.
 * @details If set to @p TRUE the support for FLEXCOM0 is included.
 */
#if !defined(SAMA_SPI_USE_FLEXCOM0) || defined(__DOXYGEN__)
#define SAMA_SPI_USE_FLEXCOM0               FALSE
#endif

/**
 * @brief   FSPI0 Generic clock enable.
 * @details If set to @p TRUE the support for GCLK FSPI0 is included.
 */
#if !defined(SAMA_FSPI0_USE_GCLK) || defined(__DOXYGEN__)
#define SAMA_FSPI0_USE_GCLK                 FALSE
#endif

/**
 * @brief   FSPI0 Generic clock source.
 */
#if !defined(SAMA_FSPI0_GCLK_SOURCE) || defined(__DOXYGEN__)
#define SAMA_FSPI0_GCLK_SOURCE              SAMA_GCLK_MCK_CLK
#endif

/**
 * @brief   FSPI0 Generic clock div.
 */
#if !defined(SAMA_FSPI0_GCLK_DIV) || defined(__DOXYGEN__)
#define SAMA_FSPI0_GCLK_DIV                 21
#endif

/**
 * @brief   SPI FLEXCOM1 driver enable switch.
 * @details If set to @p TRUE the support for FLEXCOM1 is included.
 */
#if !defined(SAMA_SPI_USE_FLEXCOM1) || defined(__DOXYGEN__)
#define SAMA_SPI_USE_FLEXCOM1               FALSE
#endif

/**
 * @brief   FSPI1 Generic clock enable.
 * @details If set to @p TRUE the support for GCLK FSPI1 is included.
 */
#if !defined(SAMA_FSPI1_USE_GCLK) || defined(__DOXYGEN__)
#define SAMA_FSPI1_USE_GCLK                 FALSE
#endif

/**
 * @brief   FSPI1 Generic clock source.
 */
#if !defined(SAMA_FSPI1_GCLK_SOURCE) || defined(__DOXYGEN__)
#define SAMA_FSPI1_GCLK_SOURCE              SAMA_GCLK_MCK_CLK
#endif

/**
 * @brief   FSPI1 Generic clock div.
 */
#if !defined(SAMA_FSPI1_GCLK_DIV) || defined(__DOXYGEN__)
#define SAMA_FSPI1_GCLK_DIV                 21
#endif

/**
 * @brief   SPI FLEXCOM2 driver enable switch.
 * @details If set to @p TRUE the support for FLEXCOM2 is included.
 */
#if !defined(SAMA_SPI_USE_FLEXCOM2) || defined(__DOXYGEN__)
#define SAMA_SPI_USE_FLEXCOM2               FALSE
#endif

/**
 * @brief   FSPI2 Generic clock enable.
 * @details If set to @p TRUE the support for GCLK FSPI2 is included.
 */
#if !defined(SAMA_FSPI2_USE_GCLK) || defined(__DOXYGEN__)
#define SAMA_FSPI2_USE_GCLK                 FALSE
#endif

/**
 * @brief   FSPI2 Generic clock source.
 */
#if !defined(SAMA_FSPI2_GCLK_SOURCE) || defined(__DOXYGEN__)
#define SAMA_FSPI2_GCLK_SOURCE              SAMA_GCLK_MCK_CLK
#endif

/**
 * @brief   FSPI2 Generic clock div.
 */
#if !defined(SAMA_FSPI2_GCLK_DIV) || defined(__DOXYGEN__)
#define SAMA_FSPI2_GCLK_DIV                 21
#endif

/**
 * @brief   SPI FLEXCOM3 driver enable switch.
 * @details If set to @p TRUE the support for FLEXCOM3 is included.
 */
#if !defined(SAMA_SPI_USE_FLEXCOM3) || defined(__DOXYGEN__)
#define SAMA_SPI_USE_FLEXCOM3               FALSE
#endif

/**
 * @brief   FSPI3 Generic clock enable.
 * @details If set to @p TRUE the support for GCLK FSPI3 is included.
 */
#if !defined(SAMA_FSPI3_USE_GCLK) || defined(__DOXYGEN__)
#define SAMA_FSPI3_USE_GCLK                 FALSE
#endif

/**
 * @brief   FSPI3 Generic clock source.
 */
#if !defined(SAMA_FSPI3_GCLK_SOURCE) || defined(__DOXYGEN__)
#define SAMA_FSPI3_GCLK_SOURCE              SAMA_GCLK_MCK_CLK
#endif

/**
 * @brief   FSPI3 Generic clock div.
 */
#if !defined(SAMA_FSPI3_GCLK_DIV) || defined(__DOXYGEN__)
#define SAMA_FSPI3_GCLK_DIV                 21
#endif

/**
 * @brief   SPI FLEXCOM4 driver enable switch.
 * @details If set to @p TRUE the support for FLEXCOM4 is included.
 */
#if !defined(SAMA_SPI_USE_FLEXCOM4) || defined(__DOXYGEN__)
#define SAMA_SPI_USE_FLEXCOM4               FALSE
#endif

/**
 * @brief   FSPI4 Generic clock enable.
 * @details If set to @p TRUE the support for GCLK FSPI4 is included.
 */
#if !defined(SAMA_FSPI4_USE_GCLK) || defined(__DOXYGEN__)
#define SAMA_FSPI4_USE_GCLK                 FALSE
#endif

/**
 * @brief   FSPI4 Generic clock source.
 */
#if !defined(SAMA_FSPI4_GCLK_SOURCE) || defined(__DOXYGEN__)
#define SAMA_FSPI4_GCLK_SOURCE              SAMA_GCLK_MCK_CLK
#endif

/**
 * @brief   FSPI4 Generic clock div.
 */
#if !defined(SAMA_FSPI4_GCLK_DIV) || defined(__DOXYGEN__)
#define SAMA_FSPI4_GCLK_DIV                 21
#endif

/**
 * @brief   SPI0 DMA interrupt priority level setting.
 */
#if !defined(SAMA_SPI_SPI0_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_SPI_SPI0_DMA_IRQ_PRIORITY      4
#endif

/**
 * @brief   SPI1 DMA interrupt priority level setting.
 */
#if !defined(SAMA_SPI_SPI1_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_SPI_SPI1_DMA_IRQ_PRIORITY      4
#endif

/**
 * @brief   FLEXCOM0 DMA interrupt priority level setting.
 */
#if !defined(SAMA_SPI_FLEXCOM0_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_SPI_FLEXCOM0_DMA_IRQ_PRIORITY  4
#endif

/**
 * @brief   FLEXCOM1 DMA interrupt priority level setting.
 */
#if !defined(SAMA_SPI_FLEXCOM1_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_SPI_FLEXCOM1_DMA_IRQ_PRIORITY  4
#endif

/**
 * @brief   FLEXCOM2 DMA interrupt priority level setting.
 */
#if !defined(SAMA_SPI_FLEXCOM2_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_SPI_FLEXCOM2_DMA_IRQ_PRIORITY  4
#endif

/**
 * @brief   FLEXCOM3 DMA interrupt priority level setting.
 */
#if !defined(SAMA_SPI_FLEXCOM3_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_SPI_FLEXCOM3_DMA_IRQ_PRIORITY  4
#endif

/**
 * @brief   FLEXCOM4 DMA interrupt priority level setting.
 */
#if !defined(SAMA_SPI_FLEXCOM4_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_SPI_FLEXCOM4_DMA_IRQ_PRIORITY  4
#endif
/** @} */

/**
 * @brief   SPI DMA error hook.
 * @note    The default action for DMA errors is a system halt because DMA
 *          error can only happen because programming errors.
 */
#if !defined(SAMA_SPI_DMA_ERROR_HOOK) || defined(__DOXYGEN__)
#define SAMA_SPI_DMA_ERROR_HOOK(spip)      osalSysHalt("DMA failure")
#endif

/**
 * @brief   SPI cache managing.
 */
#if !defined(SAMA_SPI_CACHE_USER_MANAGED) || defined(__DOXYGEN__)
#define SAMA_SPI_CACHE_USER_MANAGED        FALSE
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/
/**
 * @brief   At least an SPI unit is in use.
 */
#define SAMA_SPI_USE_SPI (SAMA_SPI_USE_SPI0 ||                               \
                          SAMA_SPI_USE_SPI1)

/**
 * @brief   At least an FLEXCOM unit is in use.
 */
#define SAMA_SPI_USE_FLEXCOM (SAMA_SPI_USE_FLEXCOM0 ||                       \
                              SAMA_SPI_USE_FLEXCOM1 ||                       \
                              SAMA_SPI_USE_FLEXCOM2 ||                       \
                              SAMA_SPI_USE_FLEXCOM3 ||                       \
                              SAMA_SPI_USE_FLEXCOM4)

#if !SAMA_SPI_USE_SPI0 && !SAMA_SPI_USE_SPI1 &&                              \
    !SAMA_SPI_USE_FLEXCOM0 && !SAMA_SPI_USE_FLEXCOM1 &&                      \
    !SAMA_SPI_USE_FLEXCOM2 && !SAMA_SPI_USE_FLEXCOM3 &&                      \
    !SAMA_SPI_USE_FLEXCOM4
#error "SPI driver activated but no SPI peripheral assigned"
#endif

/* Checks on allocation of UARTx units.*/
#if SAMA_SPI_USE_FLEXCOM0
#if defined(SAMA_FLEXCOM0_IS_USED)
#error "FSPID0 requires FLEXCOM0 but the peripheral is already used"
#else
#define SAMA_FLEXCOM0_IS_USED
#endif
#endif

#if SAMA_SPI_USE_FLEXCOM1
#if defined(SAMA_FLEXCOM1_IS_USED)
#error "FSPID1 requires FLEXCOM1 but the peripheral is already used"
#else
#define SAMA_FLEXCOM1_IS_USED
#endif
#endif

#if SAMA_SPI_USE_FLEXCOM2
#if defined(SAMA_FLEXCOM2_IS_USED)
#error "FSPID2 requires FLEXCOM2 but the peripheral is already used"
#else
#define SAMA_FLEXCOM2_IS_USED
#endif
#endif

#if SAMA_SPI_USE_FLEXCOM3
#if defined(SAMA_FLEXCOM3_IS_USED)
#error "FSPID3 requires FLEXCOM3 but the peripheral is already used"
#else
#define SAMA_FLEXCOM3_IS_USED
#endif
#endif

#if SAMA_SPI_USE_FLEXCOM4
#if defined(SAMA_FLEXCOM4_IS_USED)
#error "FSPID4 requires FLEXCOM4 but the peripheral is already used"
#else
#define SAMA_FLEXCOM4_IS_USED
#endif
#endif

#if SPI_SELECT_MODE == (SPI_SELECT_MODE_LLD || SPI_SELECT_MODE_PAD ||        \
                        SPI_SELECT_MODE_PORT || SPI_SELECT_MODE_LINE)
#error "SPI_SELECT_MODE_NONE is supported by this driver"
#endif

#if !defined(SAMA_DMA_REQUIRED)
#define SAMA_DMA_REQUIRED
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

#define spi_lld_driver_fields                                               \
  /* Pointer to the SPIx registers block.*/                                 \
  Spi                      *spi;                                            \
  /* Pointer to the FLEXCOMx registers block.*/                             \
  Flexcom                  *flexcom;                                        \
  /* Receive DMA stream.*/                                                  \
  sama_dma_channel_t       *dmarx;                                          \
  /* Transmit DMA stream.*/                                                 \
  sama_dma_channel_t       *dmatx;                                          \
  /* RX DMA mode bit mask.*/                                                \
  uint32_t                 rxdmamode;                                       \
  /* TX DMA mode bit mask.*/                                                \
  uint32_t                 txdmamode;                                       \
  /* Pointer to the TX buffer location.*/                                   \
  const uint8_t            *txbuf;                                          \
  /* Pointer to the RX buffer location.*/                                   \
  uint8_t                  *rxbuf;                                          \
  /* Number of bytes in RX phase.*/                                         \
  size_t                   rxbytes;

/**
 * @brief   Low level fields of the SPI configuration structure.
 */
#define spi_lld_config_fields                                               \
  /* The chip select line number.*/                                         \
  uint8_t                  npcs;                                            \
  /* SPI MR register initialization data.*/                                 \
  uint32_t                  mr;                                             \
  /* SPI CSR register initialization data.*/                                \
  uint32_t                  csr;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/
#define SPI_PCS(npcs)       SPI_MR_PCS((~(1 << npcs) & 0xF))

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if SAMA_SPI_USE_SPI0 && !defined(__DOXYGEN__)
extern SPIDriver SPID0;
#endif

#if SAMA_SPI_USE_SPI1 && !defined(__DOXYGEN__)
extern SPIDriver SPID1;
#endif

#if SAMA_SPI_USE_FLEXCOM0 && !defined(__DOXYGEN__)
extern SPIDriver FSPID0;
#endif

#if SAMA_SPI_USE_FLEXCOM1 && !defined(__DOXYGEN__)
extern SPIDriver FSPID1;
#endif

#if SAMA_SPI_USE_FLEXCOM2 && !defined(__DOXYGEN__)
extern SPIDriver FSPID2;
#endif

#if SAMA_SPI_USE_FLEXCOM3 && !defined(__DOXYGEN__)
extern SPIDriver FSPID3;
#endif

#if SAMA_SPI_USE_FLEXCOM4 && !defined(__DOXYGEN__)
extern SPIDriver FSPID4;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void spi_lld_init(void);
  void spi_lld_start(SPIDriver *spip);
  void spi_lld_stop(SPIDriver *spip);
  void spi_lld_select(SPIDriver *spip);
  void spi_lld_unselect(SPIDriver *spip);
  void spi_lld_ignore(SPIDriver *spip, size_t n);
  void spi_lld_exchange(SPIDriver *spip, size_t n,
                        const void *txbuf, void *rxbuf);
  void spi_lld_send(SPIDriver *spip, size_t n, const void *txbuf);
  void spi_lld_receive(SPIDriver *spip, size_t n, void *rxbuf);
  uint16_t spi_lld_polled_exchange(SPIDriver *spip, uint16_t frame);

#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_SPI */

#endif /* HAL_SPI_LLD_H */

/** @} */
