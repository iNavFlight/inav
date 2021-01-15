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
 * @file    QUADSPIv1/hal_wspi_lld.h
 * @brief   STM32 WSPI subsystem low level driver header.
 *
 * @addtogroup WSPI
 * @{
 */

#ifndef HAL_WSPI_LLD_H
#define HAL_WSPI_LLD_H

#if (HAL_USE_WSPI == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    WSPI implementation capabilities
 * @{
 */
#define WSPI_SUPPORTS_MEMMAP                TRUE
#define WSPI_DEFAULT_CFG_MASKS              FALSE
/** @} */

/**
 * @name    Transfer options
 * @note    The low level driver has the option to override the following
 *          definitions and use its own ones. In must take care to use
 *          the same name for the same function or compatibility is not
 *          ensured.
 * @note    There are the following limitations in this implementation:
 *          - Eight lines are not supported.
 *          - DDR mode is only supported for the whole command, separate
 *            masks are defined but all define the same bit.
 *          - Only 8 bits instructions are supported.
 *          .
 * @{
 */
#define WSPI_CFG_CMD_MODE_MASK              (3LU << 8LU)
#define WSPI_CFG_CMD_MODE_NONE              (0LU << 8LU)
#define WSPI_CFG_CMD_MODE_ONE_LINE          (1LU << 8LU)
#define WSPI_CFG_CMD_MODE_TWO_LINES         (2LU << 8LU)
#define WSPI_CFG_CMD_MODE_FOUR_LINES        (3LU << 8LU)

#define WSPI_CFG_CMD_DDR                    (1LU << 31LU)

#define WSPI_CFG_CMD_SIZE_MASK              0LU
#define WSPI_CFG_CMD_SIZE_8                 0LU

#define WSPI_CFG_ADDR_MODE_MASK             (3LU << 10LU)
#define WSPI_CFG_ADDR_MODE_NONE             (0LU << 10LU)
#define WSPI_CFG_ADDR_MODE_ONE_LINE         (1LU << 10LU)
#define WSPI_CFG_ADDR_MODE_TWO_LINES        (2LU << 10LU)
#define WSPI_CFG_ADDR_MODE_FOUR_LINES       (3LU << 10LU)

#define WSPI_CFG_ADDR_DDR                   (1LU << 31LU)

#define WSPI_CFG_ADDR_SIZE_MASK             (3LU << 12LU)
#define WSPI_CFG_ADDR_SIZE_8                (0LU << 12LU)
#define WSPI_CFG_ADDR_SIZE_16               (1LU << 12LU)
#define WSPI_CFG_ADDR_SIZE_24               (2LU << 12LU)
#define WSPI_CFG_ADDR_SIZE_32               (3LU << 12LU)

#define WSPI_CFG_ALT_MODE_MASK              (3LU << 14LU)
#define WSPI_CFG_ALT_MODE_NONE              (0LU << 14LU)
#define WSPI_CFG_ALT_MODE_ONE_LINE          (1LU << 14LU)
#define WSPI_CFG_ALT_MODE_TWO_LINES         (2LU << 14LU)
#define WSPI_CFG_ALT_MODE_FOUR_LINES        (3LU << 14LU)

#define WSPI_CFG_ALT_DDR                    (1LU << 31LU)

#define WSPI_CFG_ALT_SIZE_MASK              (3LU << 16LU)
#define WSPI_CFG_ALT_SIZE_8                 (0LU << 16LU)
#define WSPI_CFG_ALT_SIZE_16                (1LU << 16LU)
#define WSPI_CFG_ALT_SIZE_24                (2LU << 16LU)
#define WSPI_CFG_ALT_SIZE_32                (3LU << 16LU)

#define WSPI_CFG_DATA_MODE_MASK             (3LU << 24LU)
#define WSPI_CFG_DATA_MODE_NONE             (0LU << 24LU)
#define WSPI_CFG_DATA_MODE_ONE_LINE         (1LU << 24LU)
#define WSPI_CFG_DATA_MODE_TWO_LINES        (2LU << 24LU)
#define WSPI_CFG_DATA_MODE_FOUR_LINES       (3LU << 24LU)

#define WSPI_CFG_DATA_DDR                   (1LU << 31LU)

#define WSPI_CFG_SIOO                       (1LU << 28LU)
/** @} */

/**
 * @name    Helpers for CCR register.
 * @{
 */
#define QUADSPI_CCR_DUMMY_CYCLES_MASK       (0x1FLU << 18LU)
#define QUADSPI_CCR_DUMMY_CYCLES(n)         ((n) << 18LU)
/** @} */

/**
 * @name    DCR register options
 * @{
 */
#define STM32_DCR_CK_MODE                   (1U << 0U)
#define STM32_DCR_CSHT_MASK                 (7U << 8U)
#define STM32_DCR_CSHT(n)                   ((n) << 8U)
#define STM32_DCR_FSIZE_MASK                (31U << 16U)
#define STM32_DCR_FSIZE(n)                  ((n) << 16U)
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   WSPID1 driver enable switch.
 * @details If set to @p TRUE the support for QUADSPI1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(STM32_WSPI_USE_QUADSPI1) || defined(__DOXYGEN__)
#define STM32_WSPI_USE_QUADSPI1             FALSE
#endif

/**
 * @brief   QUADSPI1 prescaler setting.
 * @note    This is the prescaler divider value 1..256. The maximum frequency
 *          varies depending on the STM32 model and operating conditions,
 *          find the details in the data sheet.
 */
#if !defined(STM32_WSPI_QUADSPI1_PRESCALER_VALUE) || defined(__DOXYGEN__)
#define STM32_WSPI_QUADSPI1_PRESCALER_VALUE 1
#endif

/**
 * @brief   QUADSPI1 interrupt priority level setting.
 */
#if !defined(STM32_WSPI_QUADSPI1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_WSPI_QUADSPI1_IRQ_PRIORITY    10
#endif

/**
 * @brief   QUADSPI1 DMA priority (0..3|lowest..highest).
 */
#if !defined(STM32_WSPI_QUADSPI1_DMA_PRIORITY) || defined(__DOXYGEN__)
#define STM32_WSPI_QUADSPI1_DMA_PRIORITY    1
#endif

/**
 * @brief   QUADSPI1 DMA interrupt priority level setting.
 */
#if !defined(STM32_WSPI_QUADSPI1_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_WSPI_QUADSPI1_DMA_IRQ_PRIORITY 10
#endif

/**
 * @brief   QUADSPI DMA error hook.
 */
#if !defined(STM32_WSPI_DMA_ERROR_HOOK) || defined(__DOXYGEN__)
#define STM32_WSPI_DMA_ERROR_HOOK(qspip)    osalSysHalt("DMA failure")
#endif

/**
 * @brief   Enables a workaround for a STM32L476 QUADSPI errata.
 * @details The document DM00111498 states: "QUADSPI_BK1_IO1 is always an
 *          input when the command is sent in dual or quad SPI mode".
 *          This workaround makes commands without address or data phases
 *          to be sent as alternate bytes.
 */
#if !defined(STM32_USE_STM32_D1_WORKAROUND) || defined(__DOXYGEN__)
#define STM32_USE_STM32_D1_WORKAROUND       TRUE
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !defined(STM32_HAS_QUADSPI1)
#define STM32_HAS_QUADSPI1                  FALSE
#endif

#if STM32_WSPI_USE_QUADSPI1 && !STM32_HAS_QUADSPI1
#error "QUADSPI1 not present in the selected device"
#endif

#if !STM32_WSPI_USE_QUADSPI1
#error "WSPI driver activated but no QUADSPI peripheral assigned"
#endif

#if STM32_WSPI_USE_QUADSPI1 &&                                              \
    !OSAL_IRQ_IS_VALID_PRIORITY(STM32_WSPI_QUADSPI1_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to QUADSPI1"
#endif

#if STM32_WSPI_USE_QUADSPI1 &&                                              \
    !OSAL_IRQ_IS_VALID_PRIORITY(STM32_WSPI_QUADSPI1_DMA_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to QUADSPI1 DMA"
#endif

#if STM32_WSPI_USE_QUADSPI1 &&                                              \
    !STM32_DMA_IS_VALID_PRIORITY(STM32_WSPI_QUADSPI1_DMA_PRIORITY)
#error "Invalid DMA priority assigned to QUADSPI1"
#endif

#if (STM32_WSPI_QUADSPI1_PRESCALER_VALUE < 1) ||                            \
    (STM32_WSPI_QUADSPI1_PRESCALER_VALUE > 256)
#error "STM32_WSPI_QUADSPI1_PRESCALER_VALUE not within 1..256"
#endif

/* Check on the presence of the DMA streams settings in mcuconf.h.*/
#if STM32_WSPI_USE_QUADSPI1 && !defined(STM32_WSPI_QUADSPI1_DMA_STREAM)
#error "QUADSPI1 DMA stream not defined"
#endif

/* Check on the validity of the assigned DMA channels.*/
#if STM32_WSPI_USE_QUADSPI1 &&                                              \
    !STM32_DMA_IS_VALID_STREAM(STM32_WSPI_QUADSPI1_DMA_STREAM)
#error "invalid DMA stream associated to QUADSPI1"
#endif

/* Check on the validity of the assigned DMA channels.*/
#if STM32_WSPI_USE_QUADSPI1 &&                                                   \
    !STM32_DMA_IS_VALID_ID(STM32_WSPI_QUADSPI1_DMA_STREAM, STM32_QUADSPI1_DMA_MSK)
#error "invalid DMA stream associated to QUADSPI1"
#endif

#if !defined(STM32_DMA_REQUIRED)
#define STM32_DMA_REQUIRED
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Low level fields of the WSPI configuration structure.
 */
#define wspi_lld_config_fields                                              \
  /* DCR register initialization data.*/                                    \
  uint32_t                  dcr

/**
 * @brief   Low level fields of the WSPI driver structure.
 */
#define wspi_lld_driver_fields                                              \
  /* Pointer to the QUADSPIx registers block.*/                             \
  QUADSPI_TypeDef           *qspi;                                          \
  /* QUADSPI DMA stream.*/                                                  \
  const stm32_dma_stream_t  *dma;                                           \
  /* QUADSPI DMA mode bit mask.*/                                           \
  uint32_t                  dmamode

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if (STM32_WSPI_USE_QUADSPI1 == TRUE) && !defined(__DOXYGEN__)
extern WSPIDriver WSPID1;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void wspi_lld_init(void);
  void wspi_lld_start(WSPIDriver *wspip);
  void wspi_lld_stop(WSPIDriver *wspip);
  void wspi_lld_command(WSPIDriver *wspip, const wspi_command_t *cmdp);
  void wspi_lld_send(WSPIDriver *wspip, const wspi_command_t *cmdp,
                     size_t n, const uint8_t *txbuf);
  void wspi_lld_receive(WSPIDriver *wspip, const wspi_command_t *cmdp,
                        size_t n, uint8_t *rxbuf);
#if WSPI_SUPPORTS_MEMMAP == TRUE
  void wspi_lld_map_flash(WSPIDriver *wspip,
                          const wspi_command_t *cmdp,
                          uint8_t **addrp);
  void wspi_lld_unmap_flash(WSPIDriver *wspip);
#endif
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_WSPI */

#endif /* HAL_WSPI_LLD_H */

/** @} */
