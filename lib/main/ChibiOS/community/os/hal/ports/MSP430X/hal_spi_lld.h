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
 * @file    hal_spi_lld.h
 * @brief   MSP430X SPI subsystem low level driver header.
 *
 * @addtogroup SPI
 * @{
 */

#ifndef HAL_SPI_LLD_H
#define HAL_SPI_LLD_H

#if (HAL_USE_SPI == TRUE) || defined(__DOXYGEN__)

#include "hal_dma_lld.h"
/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    MSP430X configuration options
 * @{
 */
/**
 * @brief   SPIA0 driver enable switch.
 * @details If set to @p TRUE the support for SPIA0 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_SPI_USE_SPIA0) || defined(__DOXYGEN__)
#define MSP430X_SPI_USE_SPIA0                  FALSE
#endif

/**
 * @brief   SPIA1 driver enable switch.
 * @details If set to @p TRUE the support for SPIA1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_SPI_USE_SPIA1) || defined(__DOXYGEN__)
#define MSP430X_SPI_USE_SPIA1                  FALSE
#endif

/**
 * @brief   SPIA2 driver enable switch.
 * @details If set to @p TRUE the support for SPIA2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_SPI_USE_SPIA2) || defined(__DOXYGEN__)
#define MSP430X_SPI_USE_SPIA2                  FALSE
#endif

/**
 * @brief   SPIA3 driver enable switch.
 * @details If set to @p TRUE the support for SPIA3 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_SPI_USE_SPIA3) || defined(__DOXYGEN__)
#define MSP430X_SPI_USE_SPIA3                  FALSE
#endif

/**
 * @brief   SPIB0 driver enable switch.
 * @details If set to @p TRUE the support for SPIB0 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_SPI_USE_SPIB0) || defined(__DOXYGEN__)
#define MSP430X_SPI_USE_SPIB0                  FALSE
#endif

/**
 * @brief   SPIB1 driver enable switch.
 * @details If set to @p TRUE the support for SPIB1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_SPI_USE_SPIB1) || defined(__DOXYGEN__)
#define MSP430X_SPI_USE_SPIB1                  FALSE
#endif

/**
 * @brief   SPIB2 driver enable switch.
 * @details If set to @p TRUE the support for SPIB2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_SPI_USE_SPIB2) || defined(__DOXYGEN__)
#define MSP430X_SPI_USE_SPIB2                  FALSE
#endif

/**
 * @brief   SPIB3 driver enable switch.
 * @details If set to @p TRUE the support for SPIB3 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_SPI_USE_SPIB3) || defined(__DOXYGEN__)
#define MSP430X_SPI_USE_SPIB3                  FALSE
#endif

/**
 * @brief   Exclusive DMA enable switch.
 * @details If set to @p TRUE the support for exclusive DMA is included.
 * @note    This increases the size of the compiled executable somewhat.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_SPI_EXCLUSIVE_DMA) || defined(__DOXYGEN__)
#define MSP430X_SPI_EXCLUSIVE_DMA              FALSE
#endif

/**
 * @brief   SPIA0 clock source switch.
 * @details Sets the clock source for SPIA0.
 * @note    Legal values are @p MSP430X_SMCLK_SRC or @p MSP430X_ACLK_SRC.
 * @note    The default is @p MSP430X_SMCLK_SRC.
 */
#if !defined(MSP430X_SPIA0_CLK_SRC)
  #define MSP430X_SPIA0_CLK_SRC MSP430X_SMCLK_SRC
#endif

/**
 * @brief   SPIA1 clock source switch.
 * @details Sets the clock source for SPIA1.
 * @note    Legal values are @p MSP430X_SMCLK_SRC or @p MSP430X_ACLK_SRC.
 * @note    The default is @p MSP430X_SMCLK_SRC.
 */
#if !defined(MSP430X_SPIA1_CLK_SRC)
  #define MSP430X_SPIA1_CLK_SRC MSP430X_SMCLK_SRC
#endif

/**
 * @brief   SPIA2 clock source switch.
 * @details Sets the clock source for SPIA2.
 * @note    Legal values are @p MSP430X_SMCLK_SRC or @p MSP430X_ACLK_SRC.
 * @note    The default is @p MSP430X_SMCLK_SRC.
 */
#if !defined(MSP430X_SPIA2_CLK_SRC)
  #define MSP430X_SPIA2_CLK_SRC MSP430X_SMCLK_SRC
#endif

/**
 * @brief   SPIA3 clock source switch.
 * @details Sets the clock source for SPIA3.
 * @note    Legal values are @p MSP430X_SMCLK_SRC or @p MSP430X_ACLK_SRC.
 * @note    The default is @p MSP430X_SMCLK_SRC.
 */
#if !defined(MSP430X_SPIA3_CLK_SRC)
  #define MSP430X_SPIA3_CLK_SRC MSP430X_SMCLK_SRC
#endif

/**
 * @brief   SPIB0 clock source switch.
 * @details Sets the clock source for SPIB0.
 * @note    Legal values are @p MSP430X_SMCLK_SRC or @p MSP430X_ACLK_SRC.
 * @note    The default is @p MSP430X_SMCLK_SRC.
 */
#if !defined(MSP430X_SPIB0_CLK_SRC)
  #define MSP430X_SPIB0_CLK_SRC MSP430X_SMCLK_SRC
#endif

/**
 * @brief   SPIB1 clock source switch.
 * @details Sets the clock source for SPIB1.
 * @note    Legal values are @p MSP430X_SMCLK_SRC or @p MSP430X_ACLK_SRC.
 * @note    The default is @p MSP430X_SMCLK_SRC.
 */
#if !defined(MSP430X_SPIB1_CLK_SRC)
  #define MSP430X_SPIB1_CLK_SRC MSP430X_SMCLK_SRC
#endif

/**
 * @brief   SPIB2 clock source switch.
 * @details Sets the clock source for SPIB2.
 * @note    Legal values are @p MSP430X_SMCLK_SRC or @p MSP430X_ACLK_SRC.
 * @note    The default is @p MSP430X_SMCLK_SRC.
 */
#if !defined(MSP430X_SPIB2_CLK_SRC)
  #define MSP430X_SPIB2_CLK_SRC MSP430X_SMCLK_SRC
#endif

/**
 * @brief   SPIB3 clock source switch.
 * @details Sets the clock source for SPIB3.
 * @note    Legal values are @p MSP430X_SMCLK_SRC or @p MSP430X_ACLK_SRC.
 * @note    The default is @p MSP430X_SMCLK_SRC.
 */
#if !defined(MSP430X_SPIB3_CLK_SRC)
  #define MSP430X_SPIB3_CLK_SRC MSP430X_SMCLK_SRC
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if MSP430X_SPI_USE_SPIA0 && !defined(__MSP430_HAS_EUSCI_A0__)
  #error "Cannot find MSP430X_USCI module to use for SPIA0"
#endif

#if MSP430X_SPI_USE_SPIA1 && !defined(__MSP430_HAS_EUSCI_A1__)
  #error "Cannot find MSP430X_USCI module to use for SPIA1"
#endif

#if MSP430X_SPI_USE_SPIA2 && !defined(__MSP430_HAS_EUSCI_A2__)
  #error "Cannot find MSP430X_USCI module to use for SPIA2"
#endif

#if MSP430X_SPI_USE_SPIA3 && !defined(__MSP430_HAS_EUSCI_A3__)
  #error "Cannot find MSP430X_USCI module to use for SPIA3"
#endif

#if MSP430X_SPI_USE_SPIB0 && !defined(__MSP430_HAS_EUSCI_B0__)
  #error "Cannot find MSP430X_USCI module to use for SPIB0"
#endif

#if MSP430X_SPI_USE_SPIB1 && !defined(__MSP430_HAS_EUSCI_B1__)
  #error "Cannot find MSP430X_USCI module to use for SPIB1"
#endif

#if MSP430X_SPI_USE_SPIB2 && !defined(__MSP430_HAS_EUSCI_B2__)
  #error "Cannot find MSP430X_USCI module to use for SPIB2"
#endif

#if MSP430X_SPI_USE_SPIB3 && !defined(__MSP430_HAS_EUSCI_B3__)
  #error "Cannot find MSP430X_USCI module to use for SPIB3"
#endif

#if MSP430X_SPI_USE_SPIA0
  #ifdef MSP430X_USCI_A0_USED
    #error "USCI module A0 already in use - SPIA0 not available"
  #else
    #define MSP430X_USCI_A0_USED
  #endif
#endif

#if MSP430X_SPI_USE_SPIA1
  #ifdef MSP430X_USCI_A1_USED
    #error "USCI module A1 already in use - SPIA1 not available"
  #else
    #define MSP430X_USCI_A1_USED
  #endif
#endif

#if MSP430X_SPI_USE_SPIA2
  #ifdef MSP430X_USCI_A2_USED
    #error "USCI module A2 already in use - SPIA2 not available"
  #else
    #define MSP430X_USCI_A2_USED
  #endif
#endif

#if MSP430X_SPI_USE_SPIA3
  #ifdef MSP430X_USCI_A3_USED
    #error "USCI module A3 already in use - SPIA3 not available"
  #else
    #define MSP430X_USCI_A3_USED
  #endif
#endif

#if MSP430X_SPI_USE_SPIB0
  #ifdef MSP430X_USCI_B0_USED
    #error "USCI module B0 already in use - SPIB0 not available"
  #else
    #define MSP430X_USCI_B0_USED
  #endif
#endif

#if MSP430X_SPI_USE_SPIB1
  #ifdef MSP430X_USCI_B1_USED
    #error "USCI module B1 already in use - SPIB1 not available"
  #else
    #define MSP430X_USCI_B1_USED
  #endif
#endif

#if MSP430X_SPI_USE_SPIB2
  #ifdef MSP430X_USCI_B2_USED
    #error "USCI module B2 already in use - SPIB2 not available"
  #else
    #define MSP430X_USCI_B2_USED
  #endif
#endif

#if MSP430X_SPI_USE_SPIB3
  #ifdef MSP430X_USCI_B3_USED
    #error "USCI module B3 already in use - SPIB3 not available"
  #else
    #define MSP430X_USCI_B3_USED
  #endif
#endif

#if defined(MSP430X_SPIA0_TX_DMA) && (MSP430X_SPI_DMA >= MSP430X_DMA_CHANNELS)
  #error "Requested DMA for SPIA0 TX, but requested index is invalid"
#endif
#if defined(MSP430X_SPIA0_RX_DMA) && (MSP430X_SPI_DMA >= MSP430X_DMA_CHANNELS)
  #error "Requested DMA for SPIA0 RX, but requested index is invalid"
#endif

#if defined(MSP430X_SPIA1_TX_DMA) && (MSP430X_SPI_DMA >= MSP430X_DMA_CHANNELS)
  #error "Requested DMA for SPIA1 TX, but requested index is invalid"
#endif
#if defined(MSP430X_SPIA1_RX_DMA) && (MSP430X_SPI_DMA >= MSP430X_DMA_CHANNELS)
  #error "Requested DMA for SPIA1 RX, but requested index is invalid"
#endif

#if defined(MSP430X_SPIA2_TX_DMA) && (MSP430X_SPI_DMA >= MSP430X_DMA_CHANNELS)
  #error "Requested DMA for SPIA2 TX, but requested index is invalid"
#endif
#if defined(MSP430X_SPIA2_RX_DMA) && (MSP430X_SPI_DMA >= MSP430X_DMA_CHANNELS)
  #error "Requested DMA for SPIA2 RX, but requested index is invalid"
#endif

#if defined(MSP430X_SPIA3_TX_DMA) && (MSP430X_SPI_DMA >= MSP430X_DMA_CHANNELS)
  #error "Requested DMA for SPIA3 TX, but requested index is invalid"
#endif
#if defined(MSP430X_SPIA3_RX_DMA) && (MSP430X_SPI_DMA >= MSP430X_DMA_CHANNELS)
  #error "Requested DMA for SPIA3 RX, but requested index is invalid"
#endif

#if defined(MSP430X_SPIB0_TX_DMA) && (MSP430X_SPI_DMA >= MSP430X_DMA_CHANNELS)
  #error "Requested DMA for SPIB0 TX, but requested index is invalid"
#endif
#if defined(MSP430X_SPIB0_RX_DMA) && (MSP430X_SPI_DMA >= MSP430X_DMA_CHANNELS)
  #error "Requested DMA for SPIB0 RX, but requested index is invalid"
#endif

#if defined(MSP430X_SPIB1_TX_DMA) && (MSP430X_SPI_DMA >= MSP430X_DMA_CHANNELS)
  #error "Requested DMA for SPIB1 TX, but requested index is invalid"
#endif
#if defined(MSP430X_SPIB1_RX_DMA) && (MSP430X_SPI_DMA >= MSP430X_DMA_CHANNELS)
  #error "Requested DMA for SPIB1 RX, but requested index is invalid"
#endif

#if defined(MSP430X_SPIB2_TX_DMA) && (MSP430X_SPI_DMA >= MSP430X_DMA_CHANNELS)
  #error "Requested DMA for SPIB2 TX, but requested index is invalid"
#endif
#if defined(MSP430X_SPIB2_RX_DMA) && (MSP430X_SPI_DMA >= MSP430X_DMA_CHANNELS)
  #error "Requested DMA for SPIB2 RX, but requested index is invalid"
#endif

#if defined(MSP430X_SPIB3_TX_DMA) && (MSP430X_SPI_DMA >= MSP430X_DMA_CHANNELS)
  #error "Requested DMA for SPIB3 TX, but requested index is invalid"
#endif
#if defined(MSP430X_SPIB3_RX_DMA) && (MSP430X_SPI_DMA >= MSP430X_DMA_CHANNELS)
  #error "Requested DMA for SPIB3 RX, but requested index is invalid"
#endif

/* TODO figure out a way to check for conflicting DMA channels */

#if MSP430X_SPIA0_CLK_SRC == MSP430X_ACLK_SRC
  #define MSP430X_SPIA0_CLK_FREQ MSP430X_ACLK_FREQ
  #define MSP430X_SPIA0_UCSSEL UCSSEL__ACLK
#elif MSP430X_SPIA0_CLK_SRC == MSP430X_SMCLK_SRC
  #define MSP430X_SPIA0_CLK_FREQ MSP430X_SMCLK_FREQ
  #define MSP430X_SPIA0_UCSSEL UCSSEL__SMCLK
#endif

#if MSP430X_SPIA1_CLK_SRC == MSP430X_ACLK_SRC
  #define MSP430X_SPIA1_CLK_FREQ MSP430X_ACLK_FREQ
  #define MSP430X_SPIA1_UCSSEL UCSSEL__ACLK
#elif MSP430X_SPIA1_CLK_SRC == MSP430X_SMCLK_SRC
  #define MSP430X_SPIA1_CLK_FREQ MSP430X_SMCLK_FREQ
  #define MSP430X_SPIA1_UCSSEL UCSSEL__SMCLK
#endif

#if MSP430X_SPIA2_CLK_SRC == MSP430X_ACLK_SRC
  #define MSP430X_SPIA2_CLK_FREQ MSP430X_ACLK_FREQ
  #define MSP430X_SPIA2_UCSSEL UCSSEL__ACLK
#elif MSP430X_SPIA2_CLK_SRC == MSP430X_SMCLK_SRC
  #define MSP430X_SPIA2_CLK_FREQ MSP430X_SMCLK_FREQ
  #define MSP430X_SPIA2_UCSSEL UCSSEL__SMCLK
#endif

#if MSP430X_SPIA3_CLK_SRC == MSP430X_ACLK_SRC
  #define MSP430X_SPIA3_CLK_FREQ MSP430X_ACLK_FREQ
  #define MSP430X_SPIA3_UCSSEL UCSSEL__ACLK
#elif MSP430X_SPIA3_CLK_SRC == MSP430X_SMCLK_SRC
  #define MSP430X_SPIA3_CLK_FREQ MSP430X_SMCLK_FREQ
  #define MSP430X_SPIA3_UCSSEL UCSSEL__SMCLK
#endif

#if MSP430X_SPIB0_CLK_SRC == MSP430X_ACLK_SRC
  #define MSP430X_SPIB0_CLK_FREQ MSP430X_ACLK_FREQ
  #define MSP430X_SPIB0_UCSSEL UCSSEL__ACLK
#elif MSP430X_SPIB0_CLK_SRC == MSP430X_SMCLK_SRC
  #define MSP430X_SPIB0_CLK_FREQ MSP430X_SMCLK_FREQ
  #define MSP430X_SPIB0_UCSSEL UCSSEL__SMCLK
#endif

#if MSP430X_SPIB1_CLK_SRC == MSP430X_ACLK_SRC
  #define MSP430X_SPIB1_CLK_FREQ MSP430X_ACLK_FREQ
  #define MSP430X_SPIB1_UCSSEL UCSSEL__ACLK
#elif MSP430X_SPIB1_CLK_SRC == MSP430X_SMCLK_SRC
  #define MSP430X_SPIB1_CLK_FREQ MSP430X_SMCLK_FREQ
  #define MSP430X_SPIB1_UCSSEL UCSSEL__SMCLK
#endif

#if MSP430X_SPIB2_CLK_SRC == MSP430X_ACLK_SRC
  #define MSP430X_SPIB2_CLK_FREQ MSP430X_ACLK_FREQ
  #define MSP430X_SPIB2_UCSSEL UCSSEL__ACLK
#elif MSP430X_SPIB2_CLK_SRC == MSP430X_SMCLK_SRC
  #define MSP430X_SPIB2_CLK_FREQ MSP430X_SMCLK_FREQ
  #define MSP430X_SPIB2_UCSSEL UCSSEL__SMCLK
#endif

#if MSP430X_SPIB3_CLK_SRC == MSP430X_ACLK_SRC
  #define MSP430X_SPIB3_CLK_FREQ MSP430X_ACLK_FREQ
  #define MSP430X_SPIB3_UCSSEL UCSSEL__ACLK
#elif MSP430X_SPIB3_CLK_SRC == MSP430X_SMCLK_SRC
  #define MSP430X_SPIB3_CLK_FREQ MSP430X_SMCLK_FREQ
  #define MSP430X_SPIB3_UCSSEL UCSSEL__SMCLK
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a structure representing an SPI driver.
 */
typedef struct SPIDriver SPIDriver;

/**
 * @brief   SPI notification callback type.
 *
 * @param[in] spip      pointer to the @p SPIDriver object triggering the
 *                      callback
 */
typedef void (*spicallback_t)(SPIDriver *spip);

/**
 * @brief   Enumerated type for SPI bit order.
 */
typedef enum {
  MSP430X_SPI_BO_LSB = 0,
  MSP430X_SPI_BO_MSB = 1
} msp430x_spi_bit_order_t;

/**
 * @brief   Enumerated type for SPI data size.
 */
typedef enum {
  MSP430X_SPI_DS_EIGHT = 0,
  MSP430X_SPI_DS_SEVEN = 1
} msp430x_spi_data_size_t;

/**
 * @brief   Driver configuration structure.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
typedef struct {
  /**
   * @brief Operation complete callback or @p NULL.
   */
  spicallback_t             end_cb;
  /* End of the mandatory fields.*/
  /**
   * @brief The chip select line.
   * @note  This may be PAL_NOLINE to indicate that hardware chip select is used.
   */
  ioline_t                  ss_line;
  /**
   * @brief The bit rate of the SPI interface.
   * @note  Nearest available rate is used.
   */
  uint32_t                  bit_rate;
  /**
   * @brief The bit order of the peripheral - LSB or MSB first.
   */
  msp430x_spi_bit_order_t bit_order;
  /**
   * @brief The data size of the peripheral - 7 or 8 bits.
   */
  msp430x_spi_data_size_t data_size;
  /**
   * @brief The SPI mode to use - 0 through 3.
   */
  uint8_t spi_mode;
#if MSP430X_SPI_EXCLUSIVE_DMA == TRUE || defined(__DOXYGEN__)
  /**
   * @brief The index of the TX DMA channel.
   * @note  This may be >MSP430X_DMA_CHANNELS to indicate that exclusive DMA is not used.
   */
  uint8_t dmatx_index;
  /**
   * @brief The index of the RX DMA channel.
   * @note  This may be >MSP430X_DMA_CHANNELS to indicate that exclusive DMA is not used.
   */
  uint8_t dmarx_index;
#endif
} SPIConfig;

/**
 * @brief   MSP430X SPI register structure.
 */
typedef struct {
  uint16_t ctlw0;
  uint16_t _padding0;
  uint16_t _padding1;
  uint16_t brw;
  uint16_t statw_b;
  uint16_t statw_a;
  uint16_t rxbuf;
  uint16_t txbuf;
} msp430x_spi_reg_t;

/**
 * @brief   Structure representing an SPI driver.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
struct SPIDriver {
  /**
   * @brief Driver state.
   */
  spistate_t                state;
  /**
   * @brief Current configuration data.
   */
  const SPIConfig           *config;
#if (SPI_USE_WAIT == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief   Waiting thread.
   */
  thread_reference_t        thread;
#endif /* SPI_USE_WAIT */
#if (SPI_USE_MUTUAL_EXCLUSION == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief   Mutex protecting the peripheral.
   */
  mutex_t                   mutex;
#endif
#if defined(SPI_DRIVER_EXT_FIELDS)
  SPI_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief   Configuration registers.
   */
  msp430x_spi_reg_t * regs;
  /**
   * @brief   Interrupt flag register.
   */
  volatile uint16_t * ifg;
  /**
   * @brief   TX DMA request.
   */
  msp430x_dma_req_t tx_req;
  /**
   * @brief   RX DMA request.
   */
  msp430x_dma_req_t rx_req;
#if MSP430X_SPI_EXCLUSIVE_DMA == TRUE || defined(__DOXYGEN__)
  /**
   * @brief   TX DMA stream.
   */
  msp430x_dma_ch_t dmatx;
  /**
   * @brief   RX DMA stream.
   */
  msp430x_dma_ch_t dmarx;
#endif
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if (MSP430X_SPI_USE_SPIA0 == TRUE) && !defined(__DOXYGEN__)
extern SPIDriver SPIDA0;
#endif

#if (MSP430X_SPI_USE_SPIA1 == TRUE) && !defined(__DOXYGEN__)
extern SPIDriver SPIDA1;
#endif

#if (MSP430X_SPI_USE_SPIA2 == TRUE) && !defined(__DOXYGEN__)
extern SPIDriver SPIDA2;
#endif

#if (MSP430X_SPI_USE_SPIA3 == TRUE) && !defined(__DOXYGEN__)
extern SPIDriver SPIDA3;
#endif

#if (MSP430X_SPI_USE_SPIB0 == TRUE) && !defined(__DOXYGEN__)
extern SPIDriver SPIDB0;
#endif

#if (MSP430X_SPI_USE_SPIB1 == TRUE) && !defined(__DOXYGEN__)
extern SPIDriver SPIDB1;
#endif

#if (MSP430X_SPI_USE_SPIB2 == TRUE) && !defined(__DOXYGEN__)
extern SPIDriver SPIDB2;
#endif

#if (MSP430X_SPI_USE_SPIB3 == TRUE) && !defined(__DOXYGEN__)
extern SPIDriver SPIDB3;
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
  uint8_t spi_lld_polled_exchange(SPIDriver *spip, uint8_t frame);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_SPI == TRUE */

#endif /* HAL_SPI_LLD_H */

/** @} */
