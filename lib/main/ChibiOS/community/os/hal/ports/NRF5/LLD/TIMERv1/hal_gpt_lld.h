/*
    Copyright (C) 2015 Stephen Caudle

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
 * @file    TIMERv1/gpt_lld.h
 * @brief   NRF5 GPT subsystem low level driver header.
 *
 * @addtogroup GPT
 * @{
 */

#ifndef HAL_GPT_LLD_H
#define HAL_GPT_LLD_H

#if HAL_USE_GPT || defined(__DOXYGEN__)

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
 * @brief   GPTD1 driver enable switch.
 * @details If set to @p TRUE the support for GPTD1 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(NRF5_GPT_USE_TIMER0) || defined(__DOXYGEN__)
#define NRF5_GPT_USE_TIMER0                  FALSE
#endif

/**
 * @brief   GPTD2 driver enable switch.
 * @details If set to @p TRUE the support for GPTD2 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(NRF5_GPT_USE_TIMER1) || defined(__DOXYGEN__)
#define NRF5_GPT_USE_TIMER1                  FALSE
#endif

/**
 * @brief   GPTD3 driver enable switch.
 * @details If set to @p TRUE the support for GPTD3 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(NRF5_GPT_USE_TIMER2) || defined(__DOXYGEN__)
#define NRF5_GPT_USE_TIMER2                  FALSE
#endif

/**
 * @brief   GPTD1 interrupt priority level setting.
 */
#if !defined(NRF5_GPT_TIMER0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define NRF5_GPT_TIMER0_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPTD2 interrupt priority level setting.
 */
#if !defined(NRF5_GPT_TIMER1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define NRF5_GPT_TIMER1_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPTD3 interrupt priority level setting.
 */
#if !defined(NRF5_GPT_TIMER2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define NRF5_GPT_TIMER2_IRQ_PRIORITY         3
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !NRF5_GPT_USE_TIMER0 && !NRF5_GPT_USE_TIMER1 &&                         \
    !NRF5_GPT_USE_TIMER2
#error "GPT driver activated but no TIMER peripheral assigned"
#endif

#if NRF5_GPT_USE_TIMER0 &&                                                   \
    !OSAL_IRQ_IS_VALID_PRIORITY(NRF5_GPT_TIMER0_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIMER0"
#endif

#if NRF5_GPT_USE_TIMER1 &&                                                   \
    !OSAL_IRQ_IS_VALID_PRIORITY(NRF5_GPT_TIMER1_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIMER1"
#endif

#if NRF5_GPT_USE_TIMER2 &&                                                   \
    !OSAL_IRQ_IS_VALID_PRIORITY(NRF5_GPT_TIMER2_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIMER2"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   GPT frequency type.
 */
typedef enum {
  NRF5_GPT_FREQ_31250HZ = 31250,
  NRF5_GPT_FREQ_62500HZ = 62500,
  NRF5_GPT_FREQ_125KHZ = 125000,
  NRF5_GPT_FREQ_250KHZ = 250000,
  NRF5_GPT_FREQ_500KHZ = 500000,
  NRF5_GPT_FREQ_1MHZ = 1000000,
  NRF5_GPT_FREQ_2MHZ = 2000000,
  NRF5_GPT_FREQ_4MHZ = 4000000,
  NRF5_GPT_FREQ_8MHZ = 8000000,
  NRF5_GPT_FREQ_16MHZ = 16000000,
} gptfreq_t;

/**
 * @brief   GPT counter type.
 */
typedef uint32_t gptcnt_t;

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct {
  /**
   * @brief   Timer clock in Hz.
   * @note    The low level can use assertions in order to catch invalid
   *          frequency specifications.
   */
  gptfreq_t                 frequency;
  /**
   * @brief   Timer callback pointer.
   * @note    This callback is invoked on GPT counter events.
   * @note    This callback can be set to @p NULL but in that case the
   *          one-shot mode cannot be used.
   */
  gptcallback_t             callback;
  /* End of the mandatory fields.*/
  /**
   * @brief The timer resolution in bits (8/16/24/32)
   * @note  The default value of this field is 16 bits
   * @note  The 24 and 32 bit modes are only valid for TIMER0
   */
  uint8_t                  resolution;
} GPTConfig;

/**
 * @brief   Structure representing a GPT driver.
 */
struct GPTDriver {
  /**
   * @brief Driver state.
   */
  gptstate_t                state;
  /**
   * @brief Current configuration data.
   */
  const GPTConfig           *config;
#if defined(GPT_DRIVER_EXT_FIELDS)
  GPT_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief Pointer to the TIMERx registers block.
   */
  NRF_TIMER_Type            *tim;
  /**
   * @brief Index of the TIMERx capture/compare register used for setting the
   *        interval between compare events.
   */
  uint8_t                   cc_int;
  /**
   * @brief Index of the TIMERx capture/compare register used for getting the
   *        current timer counter value.
   */
  uint8_t                   cc_get;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Changes the interval of GPT peripheral.
 * @details This function changes the interval of a running GPT unit.
 * @pre     The GPT unit must be running in continuous mode.
 * @post    The GPT unit interval is changed to the new value.
 * @note    The function has effect at the next cycle start.
 *
 * @param[in] gptp      pointer to a @p GPTDriver object
 * @param[in] interval  new cycle time in timer ticks
 *
 * @notapi
 */
#define gpt_lld_change_interval(gptp, interval)                               \
  ((gptp)->tim->CC[(gptp)->cc_int] = (uint32_t)((interval) - 1))

/**
 * @brief   Returns the interval of GPT peripheral.
 * @pre     The GPT unit must be running in continuous mode.
 *
 * @param[in] gptp      pointer to a @p GPTDriver object
 * @return              The current interval.
 *
 * @notapi
 */
#define gpt_lld_get_interval(gptp)                                            \
  ((gptcnt_t)((gptp)->tim->CC[(gptp)->cc_int]) + 1)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if NRF5_GPT_USE_TIMER0 && !defined(__DOXYGEN__)
extern GPTDriver GPTD1;
#endif

#if NRF5_GPT_USE_TIMER1 && !defined(__DOXYGEN__)
extern GPTDriver GPTD2;
#endif

#if NRF5_GPT_USE_TIMER2 && !defined(__DOXYGEN__)
extern GPTDriver GPTD3;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void gpt_lld_init(void);
  void gpt_lld_start(GPTDriver *gptp);
  void gpt_lld_stop(GPTDriver *gptp);
  void gpt_lld_start_timer(GPTDriver *gptp, gptcnt_t period);
  void gpt_lld_stop_timer(GPTDriver *gptp);
  void gpt_lld_polled_delay(GPTDriver *gptp, gptcnt_t interval);
  gptcnt_t gpt_lld_get_counter(GPTDriver *gptp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_GPT */

#endif /* HAL_GPT_LLD_H */

/** @} */
