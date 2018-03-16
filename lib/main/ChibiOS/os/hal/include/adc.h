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
 * @file    adc.h
 * @brief   ADC Driver macros and structures.
 *
 * @addtogroup ADC
 * @{
 */

#ifndef _ADC_H_
#define _ADC_H_

#if (HAL_USE_ADC == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    ADC configuration options
 * @{
 */
/**
 * @brief   Enables synchronous APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(ADC_USE_WAIT) || defined(__DOXYGEN__)
#define ADC_USE_WAIT                TRUE
#endif

/**
 * @brief   Enables the @p adcAcquireBus() and @p adcReleaseBus() APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(ADC_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define ADC_USE_MUTUAL_EXCLUSION    TRUE
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  ADC_UNINIT = 0,                           /**< Not initialized.           */
  ADC_STOP = 1,                             /**< Stopped.                   */
  ADC_READY = 2,                            /**< Ready.                     */
  ADC_ACTIVE = 3,                           /**< Converting.                */
  ADC_COMPLETE = 4,                         /**< Conversion complete.       */
  ADC_ERROR = 5                             /**< Conversion error.          */
} adcstate_t;

#include "adc_lld.h"

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @name    Low level driver helper macros
 * @{
 */
#if (ADC_USE_WAIT == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   Resumes a thread waiting for a conversion completion.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
#define _adc_reset_i(adcp)                                                  \
  osalThreadResumeI(&(adcp)->thread, MSG_RESET)

/**
 * @brief   Resumes a thread waiting for a conversion completion.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
#define _adc_reset_s(adcp)                                                  \
  osalThreadResumeS(&(adcp)->thread, MSG_RESET)

/**
 * @brief   Wakes up the waiting thread.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
#define _adc_wakeup_isr(adcp) {                                             \
  osalSysLockFromISR();                                                     \
  osalThreadResumeI(&(adcp)->thread, MSG_OK);                               \
  osalSysUnlockFromISR();                                                   \
}

/**
 * @brief   Wakes up the waiting thread with a timeout message.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
#define _adc_timeout_isr(adcp) {                                            \
  osalSysLockFromISR();                                                     \
  osalThreadResumeI(&(adcp)->thread, MSG_TIMEOUT);                          \
  osalSysUnlockFromISR();                                                   \
}

#else /* !ADC_USE_WAIT */
#define _adc_reset_i(adcp)
#define _adc_reset_s(adcp)
#define _adc_wakeup_isr(adcp)
#define _adc_timeout_isr(adcp)
#endif /* !ADC_USE_WAIT */

/**
 * @brief   Common ISR code, half buffer event.
 * @details This code handles the portable part of the ISR code:
 *          - Callback invocation.
 *          .
 * @note    This macro is meant to be used in the low level drivers
 *          implementation only.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
#define _adc_isr_half_code(adcp) {                                          \
  if ((adcp)->grpp->end_cb != NULL) {                                       \
    (adcp)->grpp->end_cb(adcp, (adcp)->samples, (adcp)->depth / 2);         \
  }                                                                         \
}

/**
 * @brief   Common ISR code, full buffer event.
 * @details This code handles the portable part of the ISR code:
 *          - Callback invocation.
 *          - Waiting thread wakeup, if any.
 *          - Driver state transitions.
 *          .
 * @note    This macro is meant to be used in the low level drivers
 *          implementation only.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
#define _adc_isr_full_code(adcp) {                                          \
  if ((adcp)->grpp->circular) {                                             \
    /* Callback handling.*/                                                 \
    if ((adcp)->grpp->end_cb != NULL) {                                     \
      if ((adcp)->depth > 1) {                                              \
        /* Invokes the callback passing the 2nd half of the buffer.*/       \
        size_t half = (adcp)->depth / 2;                                    \
        size_t half_index = half * (adcp)->grpp->num_channels;              \
        (adcp)->grpp->end_cb(adcp, (adcp)->samples + half_index, half);     \
      }                                                                     \
      else {                                                                \
        /* Invokes the callback passing the whole buffer.*/                 \
        (adcp)->grpp->end_cb(adcp, (adcp)->samples, (adcp)->depth);         \
      }                                                                     \
    }                                                                       \
  }                                                                         \
  else {                                                                    \
    /* End conversion.*/                                                    \
    adc_lld_stop_conversion(adcp);                                          \
    if ((adcp)->grpp->end_cb != NULL) {                                     \
      (adcp)->state = ADC_COMPLETE;                                         \
      /* Invoke the callback passing the whole buffer.*/                    \
      (adcp)->grpp->end_cb(adcp, (adcp)->samples, (adcp)->depth);           \
      if ((adcp)->state == ADC_COMPLETE) {                                  \
        (adcp)->state = ADC_READY;                                          \
        (adcp)->grpp = NULL;                                                \
      }                                                                     \
    }                                                                       \
    else {                                                                  \
      (adcp)->state = ADC_READY;                                            \
      (adcp)->grpp = NULL;                                                  \
    }                                                                       \
    _adc_wakeup_isr(adcp);                                                  \
  }                                                                         \
}

/**
 * @brief   Common ISR code, error event.
 * @details This code handles the portable part of the ISR code:
 *          - Callback invocation.
 *          - Waiting thread timeout signaling, if any.
 *          - Driver state transitions.
 *          .
 * @note    This macro is meant to be used in the low level drivers
 *          implementation only.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 * @param[in] err       platform dependent error code
 *
 * @notapi
 */
#define _adc_isr_error_code(adcp, err) {                                    \
  adc_lld_stop_conversion(adcp);                                            \
  if ((adcp)->grpp->error_cb != NULL) {                                     \
    (adcp)->state = ADC_ERROR;                                              \
    (adcp)->grpp->error_cb(adcp, err);                                      \
    if ((adcp)->state == ADC_ERROR)                                         \
      (adcp)->state = ADC_READY;                                            \
      (adcp)->grpp = NULL;                                                  \
  }                                                                         \
  else {                                                                    \
    (adcp)->state = ADC_READY;                                              \
    (adcp)->grpp = NULL;                                                    \
  }                                                                         \
  _adc_timeout_isr(adcp);                                                   \
}
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void adcInit(void);
  void adcObjectInit(ADCDriver *adcp);
  void adcStart(ADCDriver *adcp, const ADCConfig *config);
  void adcStop(ADCDriver *adcp);
  void adcStartConversion(ADCDriver *adcp,
                          const ADCConversionGroup *grpp,
                          adcsample_t *samples,
                          size_t depth);
  void adcStartConversionI(ADCDriver *adcp,
                           const ADCConversionGroup *grpp,
                           adcsample_t *samples,
                           size_t depth);
  void adcStopConversion(ADCDriver *adcp);
  void adcStopConversionI(ADCDriver *adcp);
#if ADC_USE_WAIT == TRUE
  msg_t adcConvert(ADCDriver *adcp,
                   const ADCConversionGroup *grpp,
                   adcsample_t *samples,
                   size_t depth);
#endif
#if ADC_USE_MUTUAL_EXCLUSION == TRUE
  void adcAcquireBus(ADCDriver *adcp);
  void adcReleaseBus(ADCDriver *adcp);
#endif
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_ADC == TRUE */

#endif /* _ADC_H_ */

/** @} */
