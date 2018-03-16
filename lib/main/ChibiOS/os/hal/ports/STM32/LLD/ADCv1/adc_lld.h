/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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
 * @file    STM32/LLD/ADCv1/adc_lld.h
 * @brief   STM32 ADC subsystem low level driver header.
 *
 * @addtogroup ADC
 * @{
 */

#ifndef _ADC_LLD_H_
#define _ADC_LLD_H_

#if HAL_USE_ADC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    Sampling rates
 * @{
 */
#define ADC_SMPR_SMP_1P5        0U  /**< @brief 14 cycles conversion time   */
#define ADC_SMPR_SMP_7P5        1U  /**< @brief 21 cycles conversion time.  */
#define ADC_SMPR_SMP_13P5       2U  /**< @brief 28 cycles conversion time.  */
#define ADC_SMPR_SMP_28P5       3U  /**< @brief 41 cycles conversion time.  */
#define ADC_SMPR_SMP_41P5       4U  /**< @brief 54 cycles conversion time.  */
#define ADC_SMPR_SMP_55P5       5U  /**< @brief 68 cycles conversion time.  */
#define ADC_SMPR_SMP_71P5       6U  /**< @brief 84 cycles conversion time.  */
#define ADC_SMPR_SMP_239P5      7U  /**< @brief 252 cycles conversion time. */
/** @} */

/**
 * @name    CFGR1 register configuration helpers
 * @{
 */
#define ADC_CFGR1_RES_12BIT             (0U << 3U)
#define ADC_CFGR1_RES_10BIT             (1U << 3U)
#define ADC_CFGR1_RES_8BIT              (2U << 3U)
#define ADC_CFGR1_RES_6BIT              (3U << 3U)

#define ADC_CFGR1_EXTSEL_MASK           (15U << 6U)
#define ADC_CFGR1_EXTSEL_SRC(n)         ((n) << 6U)

#define ADC_CFGR1_EXTEN_MASK            (3U << 10U)
#define ADC_CFGR1_EXTEN_DISABLED        (0U << 10U)
#define ADC_CFGR1_EXTEN_RISING          (1U << 10U)
#define ADC_CFGR1_EXTEN_FALLING         (2U << 10U)
#define ADC_CFGR1_EXTEN_BOTH            (3U << 10U)
/** @} */

/**
 * @name    CFGR2 register configuration helpers
 * @{
 */
#define STM32_ADC_CKMODE_MASK           (3U << 30U)
#define STM32_ADC_CKMODE_ADCCLK         (0U << 30U)
#define STM32_ADC_CKMODE_PCLK_DIV2      (1U << 30U)
#define STM32_ADC_CKMODE_PCLK_DIV4      (2U << 30U)
#define STM32_ADC_CKMODE_PCLK           (3U << 30U)

#if (STM32_ADC_SUPPORTS_OVERSAMPLING == TRUE) || defined(__DOXYGEN__)
#define ADC_CFGR2_OVSR_MASK             (7U << 2U)
#define ADC_CFGR2_OVSR_2X               (0U << 2U)
#define ADC_CFGR2_OVSR_4X               (1U << 2U)
#define ADC_CFGR2_OVSR_8X               (2U << 2U)
#define ADC_CFGR2_OVSR_16X              (3U << 2U)
#define ADC_CFGR2_OVSR_32X              (4U << 2U)
#define ADC_CFGR2_OVSR_64X              (5U << 2U)
#define ADC_CFGR2_OVSR_128X             (6U << 2U)
#define ADC_CFGR2_OVSR_256X             (7U << 2U)

#define ADC_CFGR2_OVSS_MASK             (15 << 5U)
#define ADC_CFGR2_OVSS_SHIFT(n)         ((n) << 5U)
#endif
/** @} */

/**
 * @name    Threashold register initializer
 * @{
 */
#define ADC_TR(low, high)               (((uint32_t)(high) << 16U) |        \
                                         (uint32_t)(low))
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   ADC1 driver enable switch.
 * @details If set to @p TRUE the support for ADC1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(STM32_ADC_USE_ADC1) || defined(__DOXYGEN__)
#define STM32_ADC_USE_ADC1                  FALSE
#endif

/**
 * @brief   ADC1 clock source selection.
 */
#if !defined(STM32_ADC_ADC1_CKMODE) || defined(__DOXYGEN__)
#define STM32_ADC_ADC1_CKMODE               STM32_ADC_CKMODE_ADCCLK
#endif

/**
 * @brief   ADC1 DMA priority (0..3|lowest..highest).
 */
#if !defined(STM32_ADC_ADC1_DMA_PRIORITY) || defined(__DOXYGEN__)
#define STM32_ADC_ADC1_DMA_PRIORITY         2
#endif

/**
 * @brief   ADC interrupt priority level setting.
 */
#if !defined(STM32_ADC_ADC1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_ADC_ADC1_IRQ_PRIORITY         2
#endif

/**
 * @brief   ADC1 DMA interrupt priority level setting.
 */
#if !defined(STM32_ADC_ADC1_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_ADC_ADC1_DMA_IRQ_PRIORITY     2
#endif

#if (STM32_ADC_SUPPORTS_PRESCALER == TRUE) || defined(__DOXYGEN__)
/*
 * @brief   ADC prescaler setting.
 * @note    This setting has effect only in asynchronous clock mode (the
 *          default, @p STM32_ADC_CKMODE_ADCCLK).
 */
#if !defined(STM32_ADC_PRESCALER_VALUE) || defined(__DOXYGEN__)
#define STM32_ADC_PRESCALER_VALUE           1
#endif
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if STM32_ADC_USE_ADC1 && !STM32_HAS_ADC1
#error "ADC1 not present in the selected device"
#endif

#if !STM32_ADC_USE_ADC1
#error "ADC driver activated but no ADC peripheral assigned"
#endif

#if STM32_ADC1_IRQ_SHARED_WITH_EXTI == FALSE
#if STM32_ADC_USE_ADC1 &&                                                   \
    !OSAL_IRQ_IS_VALID_PRIORITY(STM32_ADC_ADC1_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to ADC1"
#endif
#endif

#if STM32_ADC_USE_ADC1 &&                                                   \
    !OSAL_IRQ_IS_VALID_PRIORITY(STM32_ADC_ADC1_DMA_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to ADC1 DMA"
#endif

#if STM32_ADC_USE_ADC1 &&                                                   \
    !STM32_DMA_IS_VALID_PRIORITY(STM32_ADC_ADC1_DMA_PRIORITY)
#error "Invalid DMA priority assigned to ADC1"
#endif

#if STM32_ADC_SUPPORTS_PRESCALER == TRUE
#if STM32_ADC_PRESCALER_VALUE == 1
#define STM32_ADC_PRESC                     0U
#elif STM32_ADC_PRESCALER_VALUE == 2
#define STM32_ADC_PRESC                     1U
#elif STM32_ADC_PRESCALER_VALUE == 4
#define STM32_ADC_PRESC                     2U
#elif STM32_ADC_PRESCALER_VALUE == 6
#define STM32_ADC_PRESC                     3U
#elif STM32_ADC_PRESCALER_VALUE == 8
#define STM32_ADC_PRESC                     4U
#elif STM32_ADC_PRESCALER_VALUE == 10
#define STM32_ADC_PRESC                     5U
#elif STM32_ADC_PRESCALER_VALUE == 12
#define STM32_ADC_PRESC                     6U
#elif STM32_ADC_PRESCALER_VALUE == 16
#define STM32_ADC_PRESC                     7U
#elif STM32_ADC_PRESCALER_VALUE == 32
#define STM32_ADC_PRESC                     8U
#elif STM32_ADC_PRESCALER_VALUE == 64
#define STM32_ADC_PRESC                     9U
#elif STM32_ADC_PRESCALER_VALUE == 128
#define STM32_ADC_PRESC                     10U
#elif STM32_ADC_PRESCALER_VALUE == 256
#define STM32_ADC_PRESC                     11U
#else
#error "Invalid value assigned to STM32_ADC_PRESCALER_VALUE"
#endif
#endif

/* Check on the presence of the DMA streams settings in mcuconf.h.*/
#if STM32_ADC_USE_ADC1 && !defined(STM32_ADC_ADC1_DMA_STREAM)
#error "ADC DMA stream not defined"
#endif

/* Check on the validity of the assigned DMA channels.*/
#if STM32_ADC_USE_ADC1 &&                                                   \
    !STM32_DMA_IS_VALID_ID(STM32_ADC_ADC1_DMA_STREAM, STM32_ADC1_DMA_MSK)
#error "invalid DMA stream associated to ADC1"
#endif

#if !defined(STM32_DMA_REQUIRED)
#define STM32_DMA_REQUIRED
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   ADC sample data type.
 */
typedef uint16_t adcsample_t;

/**
 * @brief   Channels number in a conversion group.
 */
typedef uint16_t adc_channels_num_t;

/**
 * @brief   Possible ADC failure causes.
 * @note    Error codes are architecture dependent and should not relied
 *          upon.
 */
typedef enum {
  ADC_ERR_DMAFAILURE = 0,                   /**< DMA operations failure.    */
  ADC_ERR_OVERFLOW = 1,                     /**< ADC overflow condition.    */
  ADC_ERR_AWD = 2                           /**< Analog watchdog triggered. */
} adcerror_t;

/**
 * @brief   Type of a structure representing an ADC driver.
 */
typedef struct ADCDriver ADCDriver;

/**
 * @brief   ADC notification callback type.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object triggering the
 *                      callback
 * @param[in] buffer    pointer to the most recent samples data
 * @param[in] n         number of buffer rows available starting from @p buffer
 */
typedef void (*adccallback_t)(ADCDriver *adcp, adcsample_t *buffer, size_t n);

/**
 * @brief   ADC error callback type.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object triggering the
 *                      callback
 * @param[in] err       ADC error code
 */
typedef void (*adcerrorcallback_t)(ADCDriver *adcp, adcerror_t err);

/**
 * @brief   Conversion group configuration structure.
 * @details This implementation-dependent structure describes a conversion
 *          operation.
 * @note    The use of this configuration structure requires knowledge of
 *          STM32 ADC cell registers interface, please refer to the STM32
 *          reference manual for details.
 */
typedef struct {
  /**
   * @brief   Enables the circular buffer mode for the group.
   */
  bool                      circular;
  /**
   * @brief   Number of the analog channels belonging to the conversion group.
   */
  adc_channels_num_t        num_channels;
  /**
   * @brief   Callback function associated to the group or @p NULL.
   */
  adccallback_t             end_cb;
  /**
   * @brief   Error callback or @p NULL.
   */
  adcerrorcallback_t        error_cb;
  /* End of the mandatory fields.*/
  /**
   * @brief   ADC CFGR1 register initialization data.
   * @note    The bits DMAEN and DMACFG are enforced internally
   *          to the driver, keep them to zero.
   * @note    The bits @p ADC_CFGR1_CONT or @p ADC_CFGR1_DISCEN must be
   *          specified in continuous more or if the buffer depth is
   *          greater than one.
   */
  uint32_t                  cfgr1;
#if (STM32_ADC_SUPPORTS_OVERSAMPLING == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief   ADC CFGR2 register initialization data.
   * @note    CKMODE bits must not be specified in this field and left to
   *          zero.
   */
  uint32_t                  cfgr2;
#endif
  /**
   * @brief   ADC TR register initialization data.
   */
  uint32_t                  tr;
  /**
   * @brief   ADC SMPR register initialization data.
   */
  uint32_t                  smpr;
  /**
   * @brief   ADC CHSELR register initialization data.
   * @details The number of bits at logic level one in this register must
   *          be equal to the number in the @p num_channels field.
   */
  uint32_t                  chselr;
} ADCConversionGroup;

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct {
  uint32_t                  dummy;
} ADCConfig;

/**
 * @brief   Structure representing an ADC driver.
 */
struct ADCDriver {
  /**
   * @brief Driver state.
   */
  adcstate_t                state;
  /**
   * @brief Current configuration data.
   */
  const ADCConfig           *config;
  /**
   * @brief Current samples buffer pointer or @p NULL.
   */
  adcsample_t               *samples;
  /**
   * @brief Current samples buffer depth or @p 0.
   */
  size_t                    depth;
  /**
   * @brief Current conversion group pointer or @p NULL.
   */
  const ADCConversionGroup  *grpp;
#if ADC_USE_WAIT || defined(__DOXYGEN__)
  /**
   * @brief Waiting thread.
   */
  thread_reference_t        thread;
#endif
#if ADC_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
  /**
   * @brief Mutex protecting the peripheral.
   */
  mutex_t                   mutex;
#endif /* ADC_USE_MUTUAL_EXCLUSION */
#if defined(ADC_DRIVER_EXT_FIELDS)
  ADC_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief Pointer to the ADCx registers block.
   */
  ADC_TypeDef               *adc;
  /**
   * @brief Pointer to associated DMA channel.
   */
  const stm32_dma_stream_t  *dmastp;
  /**
   * @brief DMA mode bit mask.
   */
  uint32_t                  dmamode;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Changes the value of the ADC CCR register.
 * @details Use this function in order to enable or disable the internal
 *          analog sources. See the documentation in the STM32 Reference
 *          Manual.
 * @note    PRESC bits must not be specified and left to zero.
 */
#define adcSTM32SetCCR(ccr) (ADC->CCR = (ccr))

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if STM32_ADC_USE_ADC1 && !defined(__DOXYGEN__)
extern ADCDriver ADCD1;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void adc_lld_init(void);
  void adc_lld_start(ADCDriver *adcp);
  void adc_lld_stop(ADCDriver *adcp);
  void adc_lld_start_conversion(ADCDriver *adcp);
  void adc_lld_stop_conversion(ADCDriver *adcp);
  void adc_lld_serve_interrupt(ADCDriver *adcp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_ADC */

#endif /* _ADC_LLD_H_ */

/** @} */
