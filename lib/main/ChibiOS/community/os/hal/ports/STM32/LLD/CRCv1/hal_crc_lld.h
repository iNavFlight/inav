/*
    ChibiOS - Copyright (C) 2015 Michael D. Spradling

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
 * @file    STM32/CRCv1/crc_lld.h
 * @brief   STM32 CRC subsystem low level driver header.
 *
 * @addtogroup CRC
 * @{
 */

#ifndef _CRC_LLD_H_
#define _CRC_LLD_H_

#if (HAL_USE_CRC == TRUE) || defined(__DOXYGEN__)

/*
 * This error check must occur outsite of CRCSW_USE_CRC1 to check if
 * two LLD drivers are enabled at the same time
 */
#if STM32_CRC_USE_CRC1 == TRUE &&                                           \
    CRCSW_USE_CRC1 == TRUE
#error "Software CRC can't be enable with STM32_CRC_USE_CRC1"
#endif

/**
 * Allow CRC Software override for ST drivers.  Some ST CRC implimentations
 * have limited capabilities.
 */
#if CRCSW_USE_CRC1 != TRUE

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   CRC1 driver enable switch.
 * @details If set to @p TRUE the support for CRC1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(STM32_CRC_USE_CRC1) || defined(__DOXYGEN__)
#define STM32_CRC_USE_CRC1                  FALSE
#endif

/**
 * @brief   CRC1 DMA priority (0..3|lowest..highest).
 * @note    The priority level is for CRC DMA stream.
 */
#if !defined(STM32_CRC_CRC1_DMA_PRIORITY) || defined(__DOXYGEN__)
#define STM32_CRC_CRC1_DMA_PRIORITY         2
#endif

/**
 * @brief   CRC1 DMA interrupt priority level setting.
 */
#if !defined(STM32_CRC_CRC1_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_CRC_CRC1_DMA_IRQ_PRIORITY     1
#endif

/**
 * @brief   CRC1 DMA STREAM to use when performing CRC calculation.
 */
#if !defined(STM32_CRC_CRC1_DMA_STREAM) || defined(__DOXYGEN__)
#define STM32_CRC_CRC1_DMA_STREAM           STM32_DMA1_STREAM2
#endif

/**
 * @brief   CRC DMA error hook.
 */
#if !defined(STM32_CRC_DMA_ERROR_HOOK) || defined(__DOXYGEN__)
#define STM32_CRC_DMA_ERROR_HOOK(spip)      osalSysHalt("DMA failure")
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if STM32_CRC_USE_CRC1 && !STM32_HAS_CRC
#error "Hardware CRC not present in the selected device"
#error "Use CRCSW_USE_CRC1 for software implementation"
#endif

#if CRC_USE_DMA
#if STM32_CRC_USE_CRC1 &&                                                   \
    !OSAL_IRQ_IS_VALID_PRIORITY(STM32_CRC_CRC1_DMA_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to CRC1"
#endif

#if STM32_CRC_USE_CRC1 &&                                                   \
    !STM32_DMA_IS_VALID_PRIORITY(STM32_CRC_CRC1_DMA_PRIORITY)
#error "Invalid DMA priority assigned to CRC1"
#endif

#if !defined(STM32_DMA_REQUIRED)
#define STM32_DMA_REQUIRED
#endif
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a structure representing an CRC driver.
 */
typedef struct CRCDriver CRCDriver;

/**
 * @brief   CRC notification callback type
 *
 * @param[in] crcp      pointer to the @ CRCDriver object triggering the
 *                      callback
 */
typedef void (*crccallback_t)(CRCDriver *crcp, uint32_t crc);

/**
 * @brief   Driver configuration structure.
 */
typedef struct {
  /**
   * @brief The size of polynomial to be used for CRC.
   */
  uint32_t                 poly_size;
  /**
   * @brief The coefficients of the polynomial to be used for CRC.
   */
  uint32_t                 poly;
  /**
   * @brief The inital value
   */
  uint32_t                 initial_val;
  /**
   * @brief The final XOR value
   */
  uint32_t                 final_val;
  /**
   * @brief Reflect bit order data going into CRC
   */
  bool                     reflect_data;
  /**
   * @brief Reflect bit order of final remainder
   */
  bool                     reflect_remainder;
  /* End of the mandatory fields.*/
  /**
   * @brief Operation complete callback or @p NULL
   */
  crccallback_t            end_cb;
} CRCConfig;


/**
 * @brief   Structure representing an CRC driver.
 */
struct CRCDriver {
  /**
   * @brief Driver state.
   */
  crcstate_t                state;
  /**
   * @brief Current configuration data.
   */
  const CRCConfig           *config;
#if CRC_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
  /**
   * @brief   Mutex protecting the peripheral.
   */
  mutex_t                   mutex;
#endif /* CRC_USE_MUTUAL_EXCLUSION */
#if defined(CRC_DRIVER_EXT_FIELDS)
  CRC_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief Pointer to the CRCx registers block.
   */
  CRC_TypeDef               *crc;

#if CRC_USE_DMA == TRUE
  /**
   * @brief   Waiting thread.
   */
  thread_reference_t        thread;
  /**
   * @brief CRC DMA stream
   */
  const stm32_dma_stream_t  *dma;
  /**
   * @brief DMA mode bit mask.
   */
  uint32_t                  dmamode;
#endif
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if STM32_CRC_USE_CRC1 && !defined(__DOXYGEN__)
extern CRCDriver CRCD1;
#endif /* STM32_CRC_USE_CRC1 */

#ifdef __cplusplus
extern "C" {
#endif
  void crc_lld_init(void);
  void crc_lld_start(CRCDriver *crcp);
  void crc_lld_stop(CRCDriver *crcp);
  void crc_lld_reset(CRCDriver *crcp);
  uint32_t crc_lld_calc(CRCDriver *crcp, size_t n, const void *buf);
#if CRC_USE_DMA
  void crc_lld_start_calc(CRCDriver *crcp, size_t n, const void *buf);
#endif
#ifdef __cplusplus
}
#endif

#endif /* CRCSW_USE_CRC1 */

#endif /* HAL_USE_CRC */

#endif /* _CRC_LLD_H_ */

/** @} */
