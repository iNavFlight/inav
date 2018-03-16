/*
    Copyright (C) 2014..2016 Marco Veeneman

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
 * @file    TIVA/LLD/spi_lld.h
 * @brief   TM4C123x/TM4C129x SPI subsystem low level driver.
 *
 * @addtogroup SPI
 * @{
 */

#ifndef HAL_SPI_LLD_H
#define HAL_SPI_LLD_H

#if HAL_USE_SPI || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    Control 0
 * @{
 */
#define TIVA_CR0_DSS_MASK       0x0F
#define TIVA_CR0_DSS(n)         ((n-1) << 0)

#define TIVA_CR0_FRF_MASK       (3 << 4)
#define TIVA_CR0_FRF(n)         ((n) << 4)

#define TIVA_CR0_SPO            (1 << 6)
#define TIVA_CR0_SPH            (1 << 7)

#define TIVA_CR0_SRC_MASK       (0xFF << 8)
#define TIVA_CR0_SRC(n)         ((n) << 8)
/** @} */

/**
 * @name    Control 1
 * @{
 */
#define TIVA_CR1_LBM            (1 << 0)
#define TIVA_CR1_SSE            (1 << 1)
#define TIVA_CR1_MS             (1 << 2)
#define TIVA_CR1_SOD            (1 << 3)
#define TIVA_CR1_EOT            (1 << 4)
/** @} */

/**
 * @name    Status
 * @{
 */
#define TIVA_SR_TFE             (1 << 0)
#define TIVA_SR_TNF             (1 << 1)
#define TIVA_SR_RNE             (1 << 2)
#define TIVA_SR_RFF             (1 << 3)
#define TIVA_SR_BSY             (1 << 4)
/** @} */

/**
 * @name    Interrupt Mask
 * @{
 */
#define TIVA_IM_RORIM           (1 << 0)
#define TIVA_IM_RTIM            (1 << 1)
#define TIVA_IM_RXIM            (1 << 2)
#define TIVA_IM_TXIM            (1 << 3)
/** @} */

/**
 * @name    Interrupt Status
 * @{
 */
#define TIVA_IS_RORIS           (1 << 0)
#define TIVA_IS_RTIS            (1 << 1)
#define TIVA_IS_RXIS            (1 << 2)
#define TIVA_IS_TXIS            (1 << 3)
/** @} */

/**
 * @name    Masked Interrupt Status
 * @{
 */
#define TIVA_MIS_RORMIS         (1 << 0)
#define TIVA_MIS_RTMIS          (1 << 1)
#define TIVA_MIS_RXMIS          (1 << 2)
#define TIVA_MIS_TXMIS          (1 << 3)
/** @} */

/**
 * @name    Interrupt Clear
 * @{
 */
#define TIVA_ICR_RORIC          (1 << 0)
#define TIVA_ICR_RTIC           (1 << 1)
/** @} */

/**
 * @name    DMA Control
 * @{
 */
#define TIVA_DMACTL_RXDMAE      (1 << 0)
#define TIVA_DMACTL_TXDMAE      (1 << 1)
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
 
/**
 * @brief   SSI0 driver enable switch.
 * @details If set to @p TRUE the support for SSI0 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_SPI_USE_SSI0) || defined(__DOXYGEN__)
#define TIVA_SPI_USE_SSI0                   FALSE
#endif

/**
 * @brief   SSI1 driver enable switch.
 * @details If set to @p TRUE the support for SSI1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_SPI_USE_SSI1) || defined(__DOXYGEN__)
#define TIVA_SPI_USE_SSI1                   FALSE
#endif

/**
 * @brief   SSI2 driver enable switch.
 * @details If set to @p TRUE the support for SSI2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_SPI_USE_SSI2) || defined(__DOXYGEN__)
#define TIVA_SPI_USE_SSI2                   FALSE
#endif

/**
 * @brief   SSI3 driver enable switch.
 * @details If set to @p TRUE the support for SSI3 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_SPI_USE_SSI3) || defined(__DOXYGEN__)
#define TIVA_SPI_USE_SSI3                   FALSE
#endif

/**
 * @brief   SPID1 interrupt priority level setting.
 */
#if !defined(TIVA_SPI_SSI0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_SPI_SSI0_IRQ_PRIORITY          5
#endif

/**
 * @brief   SPID2 interrupt priority level setting.
 */
#if !defined(TIVA_SPI_SSI1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_SPI_SSI1_IRQ_PRIORITY          5
#endif

/**
 * @brief   SPID3 interrupt priority level setting.
 */
#if !defined(TIVA_SPI_SSI2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_SPI_SSI2_IRQ_PRIORITY          5
#endif

/**
 * @brief   SPID4 interrupt priority level setting.
 */
#if !defined(TIVA_SPI_SSI3_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_SPI_SSI3_IRQ_PRIORITY          5
#endif

/**
 * @brief   SPI error hook.
 */
#if !defined(TIVA_SPI_SSI_ERROR_HOOK) || defined(__DOXYGEN__)
#define TIVA_SPI_SSI_ERROR_HOOK(spip)       osalSysHalt("SSI failure")
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if TIVA_SPI_USE_SSI0 && !TIVA_HAS_SSI0
#error "SSI0 not present in the selected device"
#endif

#if TIVA_SPI_USE_SSI1 && !TIVA_HAS_SSI1
#error "SSI1 not present in the selected device"
#endif

#if TIVA_SPI_USE_SSI2 && !TIVA_HAS_SSI2
#error "SSI2 not present in the selected device"
#endif

#if TIVA_SPI_USE_SSI3 && !TIVA_HAS_SSI03
#error "SSI3 not present in the selected device"
#endif

#if !TIVA_SPI_USE_SSI0 && !TIVA_SPI_USE_SSI1 && !TIVA_SPI_USE_SSI2 && \
  !TIVA_SPI_USE_SSI3
#error "SPI driver activated but no SSI peripheral assigned"
#endif

#if TIVA_SPI_USE_SSI0 && \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_SPI_SSI0_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to SSI0"
#endif

#if TIVA_SPI_USE_SSI1 && \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_SPI_SSI1_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to SSI1"
#endif

#if TIVA_SPI_USE_SSI2 && \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_SPI_SSI2_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to SSI2"
#endif

#if TM4C123x_SPI_USE_SSI3 && \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_SPI_SSI3_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to SSI3"
#endif

#if !defined(TIVA_UDMA_REQUIRED)
#define TIVA_UDMA_REQUIRED
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
 * @brief   Driver configuration structure.
 */
typedef struct {
  /**
   * @brief Operation complete callback or @p NULL.
   */
  spicallback_t         end_cb;
  /* End of the mandatory fields.*/
  /**
   * @brief The chip select line port.
   */
  ioportid_t            ssport;
  /**
   * @brief The chip select line pad number.
   */
  uint16_t              sspad;
  /**
   * @brief SSI CR0 initialization data.
   */
  uint16_t              cr0;
  /**
   * @brief SSI CPSR initialization data.
   */
  uint32_t              cpsr;
} SPIConfig;

/**
 * @brief   Structure representing a SPI driver.
 */
struct SPIDriver {
  /**
   * @brief Driver state.
   */
  spistate_t            state;
  /**
   * @brief Current configuration data.
   */
  const SPIConfig       *config;
#if SPI_USE_WAIT || defined(__DOXYGEN__)
  /**
   * @brief Waiting thread.
   */
  thread_reference_t     thread;
#endif /* SPI_USE_WAIT */
#if SPI_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
  /**
   * @brief Mutex protecting the bus.
   */
  mutex_t                 mutex;
#endif /* SPI_USE_MUTUAL_EXCLUSION */
#if defined(SPI_DRIVER_EXT_FIELDS)
  SPI_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief Pointer to the SSI registers block.
   */
  SSI_TypeDef           *ssi;
  /**
   * @brief Receive DMA channel number.
   */
  uint8_t               dmarxnr;
  /**
   * @brief Transmit DMA channel number.
   */
  uint8_t               dmatxnr;
  /**
   * @brief Receive DMA channel map.
   */
  uint8_t               rxchnmap;
  /**
   * @brief Transmit DMA channel map.
   */
  uint8_t               txchnmap;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if TIVA_SPI_USE_SSI0 && !defined(__DOXYGEN__)
extern SPIDriver SPID1;
#endif

#if TIVA_SPI_USE_SSI1 && !defined(__DOXYGEN__)
extern SPIDriver SPID2;
#endif

#if TIVA_SPI_USE_SSI2 && !defined(__DOXYGEN__)
extern SPIDriver SPID3;
#endif

#if TIVA_SPI_USE_SSI3 && !defined(__DOXYGEN__)
extern SPIDriver SPID4;
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
