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
 * @file    dac_lld.h
 * @brief   PLATFORM DAC subsystem low level driver header.
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
 * @brief   Maximum number of DAC channels per unit.
 */
#define DAC_MAX_CHANNELS                    2

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   DAC1 CH1 driver enable switch.
 * @details If set to @p TRUE the support for DAC1 channel 1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(PLATFORM_DAC_USE_DAC1) || defined(__DOXYGEN__)
#define PLATFORM_DAC_USE_DAC1               FALSE
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a DAC channel index.
 */
typedef uint32_t dacchannel_t;

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
} DACConversionGroup;

/**
 * @brief   Driver configuration structure.
 */
typedef struct {
  /* End of the mandatory fields.*/
  uint32_t                  dummy;
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
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if PLATFORM_DAC_USE_DAC1 && !defined(__DOXYGEN__)
extern DACDriver DACD1;
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
