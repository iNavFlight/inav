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
 * @file    STM32/DACv1/dac_lld.h
 * @brief   STM32 DAC subsystem low level driver header.
 *
 * @addtogroup DAC
 * @{
 */

#ifndef _DAC_LLD_H_
#define _DAC_LLD_H_

#if HAL_USE_DAC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    DAC trigger modes
 * @{
 */
#define DAC_TRG_MASK                    7U
#define DAC_TRG(n)                      (n)
#define DAC_TRG_EXT                     6U
#define DAC_TRG_SW                      7U
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   Enables the DAC dual mode.
 * @note    In dual mode DAC second channels cannot be accessed individually.
 */
#if !defined(STM32_DAC_DUAL_MODE) || defined(__DOXYGEN__)
#define STM32_DAC_DUAL_MODE                 FALSE
#endif

/**
 * @brief   DAC1 CH1 driver enable switch.
 * @details If set to @p TRUE the support for DAC1 channel 1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(STM32_DAC_USE_DAC1_CH1) || defined(__DOXYGEN__)
#define STM32_DAC_USE_DAC1_CH1              FALSE
#endif

/**
 * @brief   DAC1 CH2 driver enable switch.
 * @details If set to @p TRUE the support for DAC1 channel 2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(STM32_DAC_USE_DAC1_CH2) || defined(__DOXYGEN__)
#define STM32_DAC_USE_DAC1_CH2              FALSE
#endif

/**
 * @brief   DAC2 CH1 driver enable switch.
 * @details If set to @p TRUE the support for DAC2 channel 1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(STM32_DAC_USE_DAC2_CH1) || defined(__DOXYGEN__)
#define STM32_DAC_USE_DAC2_CH1              FALSE
#endif

/**
 * @brief   DAC2 CH2 driver enable switch.
 * @details If set to @p TRUE the support for DAC2 channel 2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(STM32_DAC_USE_DAC2_CH2) || defined(__DOXYGEN__)
#define STM32_DAC_USE_DAC2_CH2              FALSE
#endif

/**
 * @brief   DAC1 CH1 interrupt priority level setting.
 */
#if !defined(STM32_DAC_DAC1_CH1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_DAC_DAC1_CH1_IRQ_PRIORITY     10
#endif

/**
 * @brief   DAC1 CH2 interrupt priority level setting.
 */
#if !defined(STM32_DAC_DAC1_CH2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_DAC_DAC1_CH2_IRQ_PRIORITY     10
#endif

/**
 * @brief   DAC2 CH1 interrupt priority level setting.
 */
#if !defined(STM32_DAC_DAC2_CH1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_DAC_DAC2_CH1_IRQ_PRIORITY     10
#endif

/**
 * @brief   DAC2 CH2 interrupt priority level setting.
 */
#if !defined(STM32_DAC_DAC2_CH2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_DAC_DAC2_CH2_IRQ_PRIORITY     10
#endif

/**
 * @brief   DAC1 CH1 DMA priority (0..3|lowest..highest).
 */
#if !defined(STM32_DAC_DAC1_CH1_DMA_PRIORITY) || defined(__DOXYGEN__)
#define STM32_DAC_DAC1_CH1_DMA_PRIORITY     2
#endif

/**
 * @brief   DAC1 CH2 DMA priority (0..3|lowest..highest).
 */
#if !defined(STM32_DAC_DAC1_CH2_DMA_PRIORITY) || defined(__DOXYGEN__)
#define STM32_DAC_DAC1_CH2_DMA_PRIORITY     2
#endif

/**
 * @brief   DAC2 CH1 DMA priority (0..3|lowest..highest).
 */
#if !defined(STM32_DAC_DAC2_CH1_DMA_PRIORITY) || defined(__DOXYGEN__)
#define STM32_DAC_DAC2_CH1_DMA_PRIORITY     2
#endif

/**
 * @brief   DAC2 CH2 DMA priority (0..3|lowest..highest).
 */
#if !defined(STM32_DAC_DAC2_CH2_DMA_PRIORITY) || defined(__DOXYGEN__)
#define STM32_DAC_DAC2_CH2_DMA_PRIORITY     2
#endif
/** @} */
 
/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if STM32_DAC_USE_DAC1_CH1 && !STM32_HAS_DAC1_CH1
#error "DAC1 CH1 not present in the selected device"
#endif

#if STM32_DAC_USE_DAC1_CH2 && !STM32_HAS_DAC1_CH2
#error "DAC1 CH2 not present in the selected device"
#endif

#if STM32_DAC_USE_DAC2_CH1 && !STM32_HAS_DAC2_CH1
#error "DAC2 CH1 not present in the selected device"
#endif

#if STM32_DAC_USE_DAC2_CH2 && !STM32_HAS_DAC2_CH2
#error "DAC2 CH2 not present in the selected device"
#endif

#if (STM32_DAC_USE_DAC1_CH2 || STM32_DAC_USE_DAC2_CH2) && STM32_DAC_DUAL_MODE
#error "DACx CH2 cannot be used independently in dual mode"
#endif

#if !STM32_DAC_USE_DAC1_CH1 && !STM32_DAC_USE_DAC1_CH2 &&                   \
    !STM32_DAC_USE_DAC2_CH1 && !STM32_DAC_USE_DAC2_CH2
#error "DAC driver activated but no DAC peripheral assigned"
#endif

/* The following checks are only required when there is a DMA able to
   reassign streams to different channels.*/
#if STM32_ADVANCED_DMA
/* Check on the presence of the DMA streams settings in mcuconf.h.*/
#if STM32_DAC_USE_DAC1_CH1 && !defined(STM32_DAC_DAC1_CH1_DMA_STREAM)
#error "DAC1 CH1 DMA stream not defined"
#endif

#if STM32_DAC_USE_DAC1_CH2 && !defined(STM32_DAC_DAC1_CH2_DMA_STREAM)
#error "DAC1 CH2 DMA stream not defined"
#endif

#if STM32_DAC_USE_DAC2_CH1 && !defined(STM32_DAC_DAC2_CH1_DMA_STREAM)
#error "DAC2 CH1 DMA stream not defined"
#endif

#if STM32_DAC_USE_DAC2_CH2 && !defined(STM32_DAC_DAC2_CH2_DMA_STREAM)
#error "DAC2 CH2 DMA stream not defined"
#endif

/* Check on the validity of the assigned DMA channels.*/
#if STM32_DAC_USE_DAC1_CH1 &&                                               \
    !STM32_DMA_IS_VALID_ID(STM32_DAC_DAC1_CH1_DMA_STREAM, STM32_DAC1_CH1_DMA_MSK)
#error "invalid DMA stream associated to DAC1 CH1"
#endif

#if STM32_DAC_USE_DAC1_CH2 &&                                               \
    !STM32_DMA_IS_VALID_ID(STM32_DAC_DAC1_CH2_DMA_STREAM, STM32_DAC1_CH2_DMA_MSK)
#error "invalid DMA stream associated to DAC1 CH2"
#endif

#if STM32_DAC_USE_DAC2_CH1 &&                                               \
    !STM32_DMA_IS_VALID_ID(STM32_DAC_DAC2_CH1_DMA_STREAM, STM32_DAC2_CH1_DMA_MSK)
#error "invalid DMA stream associated to DAC2 CH1"
#endif

#if STM32_DAC_USE_DAC2_CH2 &&                                               \
    !STM32_DMA_IS_VALID_ID(STM32_DAC_DAC2_CH2_DMA_STREAM, STM32_DAC2_CH2_DMA_MSK)
#error "invalid DMA stream associated to DAC2 CH2"
#endif
#endif /* STM32_ADVANCED_DMA */

#if !defined(STM32_DMA_REQUIRED)
#define STM32_DMA_REQUIRED
#endif

/**
 * @brief   Max DAC channels.
 */
#if STM32_DAC_DUAL_MODE == FALSE
#define DAC_MAX_CHANNELS                    1
#else
#define DAC_MAX_CHANNELS                    2
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a DAC channel index.
 */
typedef uint32_t dacchannel_t;

/**
 * @brief   DAC channel parameters type.
 */
typedef struct {
  /**
   * @brief   Pointer to the DAC registers block.
   */
  DAC_TypeDef               *dac;
  /**
   * @brief   DAC data registers offset.
   */
  uint32_t                  dataoffset;
  /**
   * @brief   DAC CR register bit offset.
   */
  uint32_t                  regshift;
  /**
   * @brief   DAC CR register mask.
   */
  uint32_t                  regmask;
  /**
   * @brief   Associated DMA.
   */
  const stm32_dma_stream_t  *dma;
  /**
   * @brief   Mode bits for the DMA.
   */
  uint32_t                  dmamode;
  /**
   * @brief   DMA channel IRQ priority.
   */
  uint32_t                  dmairqprio;
} dacparams_t;

/**
 * @brief   Type of a structure representing an DAC driver.
 */
typedef struct DACDriver DACDriver;

/**
 * @brief   Type representing a DAC sample.
 */
typedef uint16_t dacsample_t;

/**
 * @brief   Possible DAC failure causes.
 * @note    Error codes are architecture dependent and should not relied
 *          upon.
 */
typedef enum {
  DAC_ERR_DMAFAILURE = 0,                   /**< DMA operations failure.    */
  DAC_ERR_UNDERFLOW = 1                     /**< DAC overflow condition.    */
} dacerror_t;

/**
 * @brief   DAC notification callback type.
 *
 * @param[in] dacp      pointer to the @p DACDriver object triggering the
 * @param[in] buffer    pointer to the next semi-buffer to be filled
 * @param[in] n         number of buffer rows available starting from @p buffer
 *                      callback
 */
typedef void (*daccallback_t)(DACDriver *dacp,
                              const dacsample_t *buffer,
                              size_t n);

/**
 * @brief   ADC error callback type.
 *
 * @param[in] dacp      pointer to the @p DACDriver object triggering the
 *                      callback
 * @param[in] err       ADC error code
 */
typedef void (*dacerrorcallback_t)(DACDriver *dacp, dacerror_t err);

/**
 * @brief   Samples alignment and size mode.
 */
typedef enum { 
  DAC_DHRM_12BIT_RIGHT = 0,
  DAC_DHRM_12BIT_LEFT = 1,
  DAC_DHRM_8BIT_RIGHT = 2,
#if STM32_DAC_DUAL_MODE && !defined(__DOXYGEN__)
  DAC_DHRM_12BIT_RIGHT_DUAL = 3,
  DAC_DHRM_12BIT_LEFT_DUAL = 4,
  DAC_DHRM_8BIT_RIGHT_DUAL = 5
#endif
} dacdhrmode_t;

/**
 * @brief   DAC Conversion group structure.
 */
typedef struct {
  /**
   * @brief   Number of DAC channels.
   */
  uint32_t                  num_channels;
  /**
   * @brief   Operation complete callback or @p NULL.
   */
  daccallback_t             end_cb;
  /**
   * @brief   Error handling callback or @p NULL.
   */
  dacerrorcallback_t        error_cb;
  /* End of the mandatory fields.*/
  /**
   * @brief   DAC initialization data.
   * @note    This field contains the (not shifted) value to be put into the
   *          TSEL field of the DAC CR register during initialization. All
   *          other fields are handled internally.
   */
  uint32_t                  trigger;
} DACConversionGroup;

/**
 * @brief   Driver configuration structure.
 */
typedef struct {
  /* End of the mandatory fields.*/
  /**
   * @brief   Initial output on DAC channels.
   */
  dacsample_t               init;
  /**
   * @brief   DAC data holding register mode.
   */
  dacdhrmode_t              datamode;
} DACConfig;

/**
 * @brief   Structure representing a DAC driver.
 */
struct DACDriver {
  /**
   * @brief   Driver state.
   */
  dacstate_t                state;
  /**
   * @brief   Conversion group.
   */
  const DACConversionGroup  *grpp;
  /**
   * @brief   Samples buffer pointer.
   */
  const dacsample_t         *samples;
  /**
   * @brief   Samples buffer size.
   */
  uint16_t                  depth;
  /**
   * @brief   Current configuration data.
   */
  const DACConfig           *config;
#if DAC_USE_WAIT || defined(__DOXYGEN__)
  /**
   * @brief   Waiting thread.
   */
  thread_reference_t        thread;
#endif /* DAC_USE_WAIT */
#if DAC_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
  /**
   * @brief   Mutex protecting the bus.
   */
  mutex_t                   mutex;
#endif /* DAC_USE_MUTUAL_EXCLUSION */
#if defined(DAC_DRIVER_EXT_FIELDS)
  DAC_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief   DAC channel parameters.
   */
  const dacparams_t         *params;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if STM32_DAC_USE_DAC1_CH1 && !defined(__DOXYGEN__)
extern DACDriver DACD1;
#endif

#if STM32_DAC_USE_DAC1_CH2 && !STM32_DAC_DUAL_MODE && !defined(__DOXYGEN__)
extern DACDriver DACD2;
#endif

#if STM32_DAC_USE_DAC2_CH1 && !defined(__DOXYGEN__)
extern DACDriver DACD3;
#endif

#if STM32_DAC_USE_DAC2_CH2 && !STM32_DAC_DUAL_MODE && !defined(__DOXYGEN__)
extern DACDriver DACD4;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void dac_lld_init(void);
  void dac_lld_start(DACDriver *dacp);
  void dac_lld_stop(DACDriver *dacp);
  void dac_lld_put_channel(DACDriver *dacp,
                           dacchannel_t channel,
                           dacsample_t sample);
  void dac_lld_start_conversion(DACDriver *dacp);
  void dac_lld_stop_conversion(DACDriver *dacp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_DAC */

#endif /* _DAC_LLD_H_ */

/** @} */
