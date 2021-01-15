/*
    ChibiOS - Copyright (C) 2014 Derek Mulcahy

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
 * @file    PITv1/hal_gpt_lld.h
 * @brief   KINETIS GPT subsystem low level driver header.
 *
 * @addtogroup GPT
 * @{
 */

#ifndef HAL_GPT_LLD_H_
#define HAL_GPT_LLD_H_

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
#if !defined(KINETIS_GPT_USE_PIT0) || defined(__DOXYGEN__)
#define KINETIS_GPT_USE_PIT0                  FALSE
#endif

/**
 * @brief   GPTD2 driver enable switch.
 * @details If set to @p TRUE the support for GPTD2 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(KINETIS_GPT_USE_PIT1) || defined(__DOXYGEN__)
#define KINETIS_GPT_USE_PIT1                  FALSE
#endif

/**
 * @brief   GPTD3 driver enable switch.
 * @details If set to @p TRUE the support for GPTD3 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(KINETIS_GPT_USE_PIT2) || defined(__DOXYGEN__)
#define KINETIS_GPT_USE_PIT2                  FALSE
#endif

/**
 * @brief   GPTD4 driver enable switch.
 * @details If set to @p TRUE the support for GPTD4 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(KINETIS_GPT_USE_PIT3) || defined(__DOXYGEN__)
#define KINETIS_GPT_USE_PIT3                  FALSE
#endif

/**
 * @brief   GPTD1 interrupt priority level setting.
 */
#if !defined(KINETIS_GPT_PIT0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define KINETIS_GPT_PIT0_IRQ_PRIORITY         7
#endif

/**
 * @brief   GPTD2 interrupt priority level setting.
 */
#if !defined(KINETIS_GPT_PIT1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define KINETIS_GPT_PIT1_IRQ_PRIORITY         7
#endif

/**
 * @brief   GPTD3 interrupt priority level setting.
 */
#if !defined(KINETIS_GPT_PIT2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define KINETIS_GPT_PIT2_IRQ_PRIORITY         7
#endif

/**
 * @brief   GPTD4 interrupt priority level setting.
 */
#if !defined(KINETIS_GPT_PIT3_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define KINETIS_GPT_PIT3_IRQ_PRIORITY         7
#endif

/**
 * @brief   GPTD* common interrupt priority level setting.
 */
#if (KINETIS_HAS_PIT_COMMON_IRQ && !defined(KINETIS_GPT_PIT_IRQ_PRIORITY))    \
     || defined(__DOXYGEN__)
#define KINETIS_GPT_PIT_IRQ_PRIORITY          2
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if KINETIS_GPT_USE_PIT0 && !KINETIS_HAS_PIT0
#error "PIT0 not present in the selected device"
#endif

#if KINETIS_GPT_USE_PIT1 && !KINETIS_HAS_PIT1
#error "PIT1 not present in the selected device"
#endif

#if KINETIS_GPT_USE_PIT2 && !KINETIS_HAS_PIT2
#error "PIT2 not present in the selected device"
#endif

#if KINETIS_GPT_USE_PIT3 && !KINETIS_HAS_PIT3
#error "PIT3 not present in the selected device"
#endif

#if !KINETIS_GPT_USE_PIT0 && !KINETIS_GPT_USE_PIT1 &&                         \
    !KINETIS_GPT_USE_PIT2 && !KINETIS_GPT_USE_PIT3
#error "GPT driver activated but no PIT peripheral assigned"
#endif

#if KINETIS_GPT_USE_PIT0 && !KINETIS_HAS_PIT_COMMON_IRQ &&                    \
    !OSAL_IRQ_IS_VALID_PRIORITY(KINETIS_GPT_PIT0_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to PIT0"
#endif

#if KINETIS_GPT_USE_PIT1 && !KINETIS_HAS_PIT_COMMON_IRQ &&                    \
    !OSAL_IRQ_IS_VALID_PRIORITY(KINETIS_GPT_PIT1_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to PIT1"
#endif

#if KINETIS_GPT_USE_PIT2 && !KINETIS_HAS_PIT_COMMON_IRQ &&                    \
    !OSAL_IRQ_IS_VALID_PRIORITY(KINETIS_GPT_PIT2_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to PIT2"
#endif

#if KINETIS_GPT_USE_PIT3 && !KINETIS_HAS_PIT_COMMON_IRQ &&                    \
    !OSAL_IRQ_IS_VALID_PRIORITY(KINETIS_GPT_PIT3_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to PIT3"
#endif

#if KINETIS_HAS_PIT_COMMON_IRQ &&                                             \
    !OSAL_IRQ_IS_VALID_PRIORITY(KINETIS_GPT_PIT_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to PIT"
#endif

#if KINETIS_GPT_USE_PIT0 && !defined(KINETIS_PIT0_IRQ_VECTOR) && \
    !KINETIS_HAS_PIT_COMMON_IRQ
#error "KINETIS_PIT0_IRQ_VECTOR not defined"
#endif

#if KINETIS_GPT_USE_PIT1 && !defined(KINETIS_PIT1_IRQ_VECTOR) && \
    !KINETIS_HAS_PIT_COMMON_IRQ
#error "KINETIS_PIT1_IRQ_VECTOR not defined"
#endif

#if KINETIS_GPT_USE_PIT2 && !defined(KINETIS_PIT2_IRQ_VECTOR) && \
    !KINETIS_HAS_PIT_COMMON_IRQ
#error "KINETIS_PIT2_IRQ_VECTOR not defined"
#endif

#if KINETIS_GPT_USE_PIT3 && !defined(KINETIS_PIT3_IRQ_VECTOR) && \
    !KINETIS_HAS_PIT_COMMON_IRQ
#error "KINETIS_PIT3_IRQ_VECTOR not defined"
#endif

#if KINETIS_HAS_PIT_COMMON_IRQ && !defined(KINETIS_PIT_IRQ_VECTOR)
#error "KINETIS_PIT_IRQ_VECTOR not defined"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   GPT frequency type.
 */
typedef uint32_t gptfreq_t;

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
   * @brief Timer base clock.
   */
  uint32_t                  clock;
  /**
   * @brief Channel structure in PIT registers block.
   */
  struct PIT_CHANNEL        *channel;
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
  ((gptp)->channel->LDVAL = (uint32_t)( \
                            ( (gptp)->clock / (gptp)->config->frequency ) * \
                            ( interval ) ))

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
  ((uint32_t)( ( (uint64_t)(gptp)->channel->LDVAL * (gptp)->config->frequency ) / \
               ( (uint32_t)(gptp)->clock ) ))

/**
 * @brief   Returns the counter value of GPT peripheral.
 * @pre     The GPT unit must be running in continuous mode.
 * @note    The nature of the counter is not defined, it may count upward
 *          or downward, it could be continuously running or not.
 *
 * @param[in] gptp      pointer to a @p GPTDriver object
 * @return              The current counter value.
 *
 * @notapi
 */
#define gpt_lld_get_counter(gptp) ((gptcnt_t)(gptp)->pit->CHANNEL[gptp->channel].CVAL)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if KINETIS_GPT_USE_PIT0 && !defined(__DOXYGEN__)
extern GPTDriver GPTD1;
#endif

#if KINETIS_GPT_USE_PIT1 && !defined(__DOXYGEN__)
extern GPTDriver GPTD2;
#endif

#if KINETIS_GPT_USE_PIT2 && !defined(__DOXYGEN__)
extern GPTDriver GPTD3;
#endif

#if KINETIS_GPT_USE_PIT3 && !defined(__DOXYGEN__)
extern GPTDriver GPTD4;
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
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_GPT */

#endif /* HAL_GPT_LLD_H_ */

/** @} */
