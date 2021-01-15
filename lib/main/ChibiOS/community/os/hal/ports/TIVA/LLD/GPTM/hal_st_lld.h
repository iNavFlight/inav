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
 * @file    GPTM/hal_st_lld.h
 * @brief   ST Driver subsystem low level driver header.
 * @details This header is designed to be include-able without having to
 *          include other files from the HAL.
 *
 * @addtogroup ST
 * @{
 */

#ifndef HAL_ST_LLD_H
#define HAL_ST_LLD_H

#include "mcuconf.h"
#include "tiva_registry.h"

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
 * @brief   SysTick timer IRQ priority.
 */
#if !defined(TIVA_ST_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_ST_IRQ_PRIORITY                2
#endif

/**
 * @brief   GPTx unit (by number) to be used for free running operations.
 * @note    You must select a 32 bits timer if a 32 bits @p systick_t type
 *          is required.
 */
#if !defined(TIVA_ST_TIMER_NUMBER) || defined(__DOXYGEN__)
#define TIVA_ST_TIMER_NUMBER                0
#endif

/**
 * @brief   When set to @p TRUE a wide timer is used. When set to @p FALSE a
 *          normal timer is used.
 */
#if !defined(TIVA_ST_USE_WIDE_TIMER) || defined(__DOXYGEN__)
#define TIVA_ST_USE_WIDE_TIMER              TRUE
#endif

/**
 * @}
 */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if (TIVA_ST_USE_WIDE_TIMER == TRUE)

#if TIVA_ST_TIMER_NUMBER == 0
#if !TIVA_HAS_WGPT0
#error "WGPT0 not present"
#endif
#define TIVA_ST_TIM                         WTIMER0_BASE

#elif TIVA_ST_TIMER_NUMBER == 1
#if !TIVA_HAS_WGPT1
#error "WGPT1 not present"
#endif
#define TIVA_ST_TIM                         WTIMER1_BASE

#elif TIVA_ST_TIMER_NUMBER == 2
#if !TIVA_HAS_WGPT2
#error "WGPT2 not present"
#endif
#define TIVA_ST_TIM                         WTIMER2_BASE

#elif TIVA_ST_TIMER_NUMBER == 3
#if !TIVA_HAS_WGPT3
#error "WGPT3 not present"
#endif
#define TIVA_ST_TIM                         WTIMER3_BASE

#elif TIVA_ST_TIMER_NUMBER == 4
#if !TIVA_HAS_WGPT4
#error "WGPT4 not present"
#endif
#define TIVA_ST_TIM                         WTIMER4_BASE

#elif TIVA_ST_TIMER_NUMBER == 5
#if !TIVA_HAS_WGPT5
#error "WGPT5 not present"
#endif
#define TIVA_ST_TIM                         WTIMER5_BASE

#else
#error "TIVA_ST_USE_TIMER specifies an unsupported timer"
#endif

#elif (TIVA_ST_USE_WIDE_TIMER == FALSE)

#if TIVA_ST_TIMER_NUMBER == 0
#if !TIVA_HAS_GPT0
#error "GPT0 not present"
#endif
#define TIVA_ST_TIM                         TIMER0_BASE

#elif TIVA_ST_TIMER_NUMBER == 1
#if !TIVA_HAS_GPT1
#error "GPT1 not present"
#endif
#define TIVA_ST_TIM                         TIMER1_BASE

#elif TIVA_ST_TIMER_NUMBER == 2
#if !TIVA_HAS_GPT2
#error "GPT2 not present"
#endif
#define TIVA_ST_TIM                         TIMER2_BASE

#elif TIVA_ST_TIMER_NUMBER == 3
#if !TIVA_HAS_GPT3
#error "GPT3 not present"
#endif
#define TIVA_ST_TIM                         TIMER3_BASE

#elif TIVA_ST_TIMER_NUMBER == 4
#if !TIVA_HAS_GPT4
#error "GPT4 not present"
#endif
#define TIVA_ST_TIM                         TIMER4_BASE

#elif TIVA_ST_TIMER_NUMBER == 5
#if !TIVA_HAS_GPT5
#error "GPT5 not present"
#endif
#define TIVA_ST_TIM                         TIMER5_BASE

#else
#error "TIVA_ST_TIMER_NUMBER specifies an unsupported timer"
#endif

#else
#error "wrong value defined for TIVA_ST_USE_WIDE_TIMER"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void st_lld_init(void);
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Driver inline functions.                                                  */
/*===========================================================================*/

/**
 * @brief   Returns the time counter value.
 *
 * @return              The counter value.
 *
 * @notapi
 */
static inline systime_t st_lld_get_counter(void)
{
  return (systime_t) (((systime_t) 0xffffffff) - HWREG(TIVA_ST_TIM + TIMER_O_TAV));
}

/**
 * @brief   Starts the alarm.
 * @note    Makes sure that no spurious alarms are triggered after
 *          this call.
 *
 * @param[in] time      the time to be set for the first alarm
 *
 * @notapi
 */
static inline void st_lld_start_alarm(systime_t time)
{
  HWREG(TIVA_ST_TIM + TIMER_O_TAMATCHR) = (systime_t) (((systime_t) 0xffffffff) - time);
  HWREG(TIVA_ST_TIM + TIMER_O_ICR) = HWREG(TIVA_ST_TIM + TIMER_O_MIS);
  HWREG(TIVA_ST_TIM + TIMER_O_IMR) = TIMER_IMR_TAMIM;
}

/**
 * @brief   Stops the alarm interrupt.
 *
 * @notapi
 */
static inline void st_lld_stop_alarm(void)
{
  HWREG(TIVA_ST_TIM + TIMER_O_IMR) = 0;
}

/**
 * @brief   Sets the alarm time.
 *
 * @param[in] time      the time to be set for the next alarm
 *
 * @notapi
 */
static inline void st_lld_set_alarm(systime_t time)
{
  HWREG(TIVA_ST_TIM + TIMER_O_TAMATCHR) = (systime_t) (((systime_t) 0xffffffff) - time);
}

/**
 * @brief   Returns the current alarm time.
 *
 * @return              The currently set alarm time.
 *
 * @notapi
 */
static inline systime_t st_lld_get_alarm(void)
{
  return (systime_t) (((systime_t)0xffffffff) - HWREG(TIVA_ST_TIM + TIMER_O_TAMATCHR));
}

/**
 * @brief   Determines if the alarm is active.
 *
 * @return              The alarm status.
 * @retval false        if the alarm is not active.
 * @retval true         is the alarm is active
 *
 * @notapi
 */
static inline bool st_lld_is_alarm_active(void)
{
  return (bool) ((HWREG(TIVA_ST_TIM + TIMER_O_IMR) & TIMER_IMR_TAMIM) !=0);
}

#endif /* HAL_ST_LLD_H */

/**
 * @}
 */
