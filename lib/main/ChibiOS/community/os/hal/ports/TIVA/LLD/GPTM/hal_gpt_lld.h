/*
    Copyright (C) 2014..2017 Marco Veeneman

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
 * @file    GPTM/hal_gpt_lld.h
 * @brief   TM4C123x/TM4C129x GPT subsystem low level driver header.
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
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_GPT_USE_GPT0) || defined(__DOXYGEN__)
#define TIVA_GPT_USE_GPT0                   FALSE
#endif

/**
 * @brief   GPTD2 driver enable switch.
 * @details If set to @p TRUE the support for GPTD2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_GPT_USE_GPT1) || defined(__DOXYGEN__)
#define TIVA_GPT_USE_GPT1                   FALSE
#endif

/**
 * @brief   GPTD3 driver enable switch.
 * @details If set to @p TRUE the support for GPTD3 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_GPT_USE_GPT2) || defined(__DOXYGEN__)
#define TIVA_GPT_USE_GPT2                   FALSE
#endif

/**
 * @brief   GPTD4 driver enable switch.
 * @details If set to @p TRUE the support for GPTD4 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_GPT_USE_GPT3) || defined(__DOXYGEN__)
#define TIVA_GPT_USE_GPT3                   FALSE
#endif

/**
 * @brief   GPTD5 driver enable switch.
 * @details If set to @p TRUE the support for GPTD5 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_GPT_USE_GPT4) || defined(__DOXYGEN__)
#define TIVA_GPT_USE_GPT4                   FALSE
#endif

/**
 * @brief   GPTD6 driver enable switch.
 * @details If set to @p TRUE the support for GPTD6 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_GPT_USE_GPT5) || defined(__DOXYGEN__)
#define TIVA_GPT_USE_GPT5                   FALSE
#endif

/**
 * @brief   GPTD7 driver enable switch.
 * @details If set to @p TRUE the support for GPTD1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_GPT_USE_WGPT0) || defined(__DOXYGEN__)
#define TIVA_GPT_USE_WGPT0                  FALSE
#endif

/**
 * @brief   GPTD8 driver enable switch.
 * @details If set to @p TRUE the support for GPTD2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_GPT_USE_WGPT1) || defined(__DOXYGEN__)
#define TIVA_GPT_USE_WGPT1                  FALSE
#endif

/**
 * @brief   GPTD9 driver enable switch.
 * @details If set to @p TRUE the support for GPTD3 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_GPT_USE_WGPT2) || defined(__DOXYGEN__)
#define TIVA_GPT_USE_WGPT2                  FALSE
#endif

/**
 * @brief   GPTD10 driver enable switch.
 * @details If set to @p TRUE the support for GPTD4 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_GPT_USE_WGPT3) || defined(__DOXYGEN__)
#define TIVA_GPT_USE_WGPT3                  FALSE
#endif

/**
 * @brief   GPTD11 driver enable switch.
 * @details If set to @p TRUE the support for GPTD5 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_GPT_USE_WGPT4) || defined(__DOXYGEN__)
#define TIVA_GPT_USE_WGPT4                  FALSE
#endif

/**
 * @brief   GPTD12 driver enable switch.
 * @details If set to @p TRUE the support for GPTD6 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_GPT_USE_WGPT5) || defined(__DOXYGEN__)
#define TIVA_GPT_USE_WGPT5                  FALSE
#endif

/**
 * @brief   GPTD1 interrupt priority level setting.
 */
#if !defined(TIVA_GPT_GPT0A_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_GPT_GPT0A_IRQ_PRIORITY          7
#endif

/**
 * @brief   GPTD2 interrupt priority level setting.
 */
#if !defined(TIVA_GPT_GPT1A_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_GPT_GPT1A_IRQ_PRIORITY          7
#endif

/**
 * @brief   GPTD3 interrupt priority level setting.
 */
#if !defined(TIVA_GPT_GPT2A_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_GPT_GPT2A_IRQ_PRIORITY          7
#endif

/**
 * @brief   GPTD4 interrupt priority level setting.
 */
#if !defined(TIVA_GPT_GPT3A_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_GPT_GPT3A_IRQ_PRIORITY          7
#endif

/**
 * @brief   GPTD5 interrupt priority level setting.
 */
#if !defined(TIVA_GPT_GPT4A_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_GPT_GPT4A_IRQ_PRIORITY          7
#endif

/**
 * @brief   GPTD6 interrupt priority level setting.
 */
#if !defined(TIVA_GPT_GPT5A_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_GPT_GPT5A_IRQ_PRIORITY          7
#endif

/**
 * @brief   GPTD7 interrupt priority level setting.
 */
#if !defined(TIVA_GPT_WGPT0A_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_GPT_WGPT0A_IRQ_PRIORITY         7
#endif

/**
 * @brief   GPTD8 interrupt priority level setting.
 */
#if !defined(TIVA_GPT_WGPT1A_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_GPT_WGPT1A_IRQ_PRIORITY         7
#endif

/**
 * @brief   GPTD9 interrupt priority level setting.
 */
#if !defined(TIVA_GPT_WGPT2A_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_GPT_WGPT2A_IRQ_PRIORITY         7
#endif

/**
 * @brief   GPTD10 interrupt priority level setting.
 */
#if !defined(TIVA_GPT_WGPT3A_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_GPT_WGPT3A_IRQ_PRIORITY         7
#endif

/**
 * @brief   GPTD11 interrupt priority level setting.
 */
#if !defined(TIVA_GPT_WGPT4A_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_GPT_WGPT4A_IRQ_PRIORITY         7
#endif

/**
 * @brief   GPTD12 interrupt priority level setting.
 */
#if !defined(TIVA_GPT_WGPT5A_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_GPT_WGPT5A_IRQ_PRIORITY         7
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if TIVA_GPT_USE_GPT0 && !TIVA_HAS_GPT0
#error "GPT0 not present in the selected device"
#endif

#if TIVA_GPT_USE_GPT1 && !TIVA_HAS_GPT1
#error "GPT1 not present in the selected device"
#endif

#if TIVA_GPT_USE_GPT2 && !TIVA_HAS_GPT2
#error "GPT2 not present in the selected device"
#endif

#if TIVA_GPT_USE_GPT3 && !TIVA_HAS_GPT3
#error "GPT3 not present in the selected device"
#endif

#if TIVA_GPT_USE_GPT4 && !TIVA_HAS_GPT4
#error "GPT4 not present in the selected device"
#endif

#if TIVA_GPT_USE_GPT5 && !TIVA_HAS_GPT5
#error "GPT5 not present in the selected device"
#endif

#if TIVA_GPT_USE_WGPT0 && !TIVA_HAS_WGPT0
#error "WGPT0 not present in the selected device"
#endif

#if TIVA_GPT_USE_WGPT1 && !TIVA_HAS_WGPT1
#error "WGPT1 not present in the selected device"
#endif

#if TIVA_GPT_USE_WGPT2 && !TIVA_HAS_WGPT2
#error "WGPT2 not present in the selected device"
#endif

#if TIVA_GPT_USE_WGPT3 && !TIVA_HAS_WGPT3
#error "WGPT3 not present in the selected device"
#endif

#if TIVA_GPT_USE_WGPT4 && !TIVA_HAS_WGPT4
#error "WGPT4 not present in the selected device"
#endif

#if TIVA_GPT_USE_WGPT5 && !TIVA_HAS_WGPT5
#error "WGPT5 not present in the selected device"
#endif

#if !TIVA_GPT_USE_GPT0 && !TIVA_GPT_USE_GPT1 && !TIVA_GPT_USE_GPT2 &&        \
    !TIVA_GPT_USE_GPT3 && !TIVA_GPT_USE_GPT4 && !TIVA_GPT_USE_GPT5 &&        \
    !TIVA_GPT_USE_WGPT0 && !TIVA_GPT_USE_WGPT1 && !TIVA_GPT_USE_WGPT2 &&     \
    !TIVA_GPT_USE_WGPT3 && !TIVA_GPT_USE_WGPT4 && !TIVA_GPT_USE_WGPT5
#error "GPT driver activated but no (W)GPT peripheral assigned"
#endif

#if TIVA_GPT_USE_GPT0 &&                                                     \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_GPT_GPT0A_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPT0"
#endif

#if TIVA_GPT_USE_GPT1 &&                                                     \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_GPT_GPT1A_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPT1"
#endif

#if TIVA_GPT_USE_GPT2 &&                                                     \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_GPT_GPT2A_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPT2"
#endif

#if TIVA_GPT_USE_GPT3 &&                                                     \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_GPT_GPT3A_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPT3"
#endif

#if TIVA_GPT_USE_GPT4 &&                                                     \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_GPT_GPT4A_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPT4"
#endif

#if TIVA_GPT_USE_GPT5 &&                                                     \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_GPT_GPT5A_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPT5"
#endif

#if TIVA_GPT_USE_WGPT0 &&                                                    \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_GPT_WGPT0A_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to WGPT0"
#endif

#if TIVA_GPT_USE_WGPT1 &&                                                    \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_GPT_WGPT1A_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to WGPT1"
#endif

#if TIVA_GPT_USE_WGPT2 &&                                                    \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_GPT_WGPT2A_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to WGPT2"
#endif

#if TIVA_GPT_USE_WGPT3 &&                                                    \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_GPT_WGPT3A_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to WGPT3"
#endif

#if TIVA_GPT_USE_WGPT4 &&                                                    \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_GPT_WGPT4A_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to WGPT4"
#endif

#if TIVA_GPT_USE_WGPT5 &&                                                    \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_GPT_WGPT5A_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to WGPT5"
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
typedef uint16_t gptcnt_t;

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
   * @brief Pointer to the GPT registers block.
   */
  uint32_t                   gpt;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Changes the interval of GPT peripheral.
 * @details This function changes the interval of a running GPT unit.
 * @pre     The GPT unit must have been activated using @p gptStart().
 * @pre     The GPT unit must have been running in continuous mode using
 *          @p gptStartContinuous().
 * @post    The GPT unit interval is changed to the new value.
 * @note    The function has effect at the next cycle start.
 *
 * @param[in] gptp      pointer to a @p GPTDriver object
 * @param[in] interval  new cycle time in timer ticks
 * @notapi
 */
#define gpt_lld_change_interval(gptp, interval) {                           \
  HWREG(gptp->gpt + TIMER_O_TAILR) = interval - 1;                                          \
}

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if TIVA_GPT_USE_GPT0 && !defined(__DOXYGEN__)
extern GPTDriver GPTD1;
#endif

#if TIVA_GPT_USE_GPT1 && !defined(__DOXYGEN__)
extern GPTDriver GPTD2;
#endif

#if TIVA_GPT_USE_GPT2 && !defined(__DOXYGEN__)
extern GPTDriver GPTD3;
#endif

#if TIVA_GPT_USE_GPT3 && !defined(__DOXYGEN__)
extern GPTDriver GPTD4;
#endif

#if TIVA_GPT_USE_GPT4 && !defined(__DOXYGEN__)
extern GPTDriver GPTD5;
#endif

#if TIVA_GPT_USE_GPT5 && !defined(__DOXYGEN__)
extern GPTDriver GPTD6;
#endif

#if TIVA_GPT_USE_WGPT0 && !defined(__DOXYGEN__)
extern GPTDriver GPTD7;
#endif

#if TIVA_GPT_USE_WGPT1 && !defined(__DOXYGEN__)
extern GPTDriver GPTD8;
#endif

#if TIVA_GPT_USE_WGPT2 && !defined(__DOXYGEN__)
extern GPTDriver GPTD9;
#endif

#if TIVA_GPT_USE_WGPT3 && !defined(__DOXYGEN__)
extern GPTDriver GPTD10;
#endif

#if TIVA_GPT_USE_WGPT4 && !defined(__DOXYGEN__)
extern GPTDriver GPTD11;
#endif

#if TIVA_GPT_USE_WGPT5 && !defined(__DOXYGEN__)
extern GPTDriver GPTD12;
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

#endif /* HAL_GPT_LLD_H */

/** @} */
