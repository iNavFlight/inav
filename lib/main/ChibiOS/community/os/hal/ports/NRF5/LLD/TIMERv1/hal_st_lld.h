/*
    ChibiOS - Copyright (C) 2015 Fabio Utzig

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
 * @file    TIMERv1/st_lld.h
 * @brief   NRF5 ST subsystem low level driver header.
 * @details This header is designed to be include-able without having to
 *          include other files from the HAL.
 *
 * @addtogroup ST
 * @{
 */

#ifndef HAL_ST_LLD_H
#define HAL_ST_LLD_H

#include "halconf.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   Use RTC0 to generates system ticks
 *
 * @note    Avoid using RTC0, as PPI has pre-programmed channels on it
 *          that can be used to control RADIO or TIMER0
 */
#if !defined(NRF5_ST_USE_RTC0) || defined(__DOXYGEN__)
#define NRF5_ST_USE_RTC0        FALSE
#endif

/**
 * @brief   Use RTC1 to generates system ticks
 */
#if !defined(NRF5_ST_USE_RTC1) || defined(__DOXYGEN__)
#define NRF5_ST_USE_RTC1        TRUE
#endif

/**
 * @brief   Use TIMER0 to generates system ticks
 *
 * @note    Avoid using TIMER0 as it will draw more current
 */
#if !defined(NRF5_ST_USE_TIMER0) || defined(__DOXYGEN__)
#define NRF5_ST_USE_TIMER0      FALSE
#endif

/**
 * @brief   ST interrupt priority level setting.
 */
#if !defined(NRF5_ST_PRIORITY) || defined(__DOXYGEN__)
#if !defined(SOFTDEVICE_PRESENT)
#define NRF5_ST_PRIORITY        CORTEX_MAX_KERNEL_PRIORITY
#else
#define NRF5_ST_PRIORITY        1
#endif
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if OSAL_ST_MODE != OSAL_ST_MODE_NONE
#if (NRF5_ST_USE_TIMER0 == TRUE) && (NRF5_GPT_USE_TIMER0 == TRUE)
#error "TIMER0 already used by GPT driver"
#endif

#if (NRF5_ST_USE_RTC0   == FALSE) && \
    (NRF5_ST_USE_RTC1   == FALSE) && \
    (NRF5_ST_USE_TIMER0 == FALSE)
#error "One clock source is needed, enable one (RTC0, RTC1, or TIMER0)"
#endif

#if ((NRF5_ST_USE_RTC0   == TRUE ? 1 : 0) + \
     (NRF5_ST_USE_RTC1   == TRUE ? 1 : 0) + \
     (NRF5_ST_USE_TIMER0 == TRUE ? 1 : 0)) > 1
#error "Only one clock source can be used (RTC0, RTC1, or TIMER0)"
#endif

#if defined(SOFTDEVICE_PRESENT)
#if NRF5_ST_USE_RTC0 == TRUE
#error "RTC0 cannot be used for system ticks when SOFTDEVICE present"
#endif

#if NRF5_ST_USE_TIMER0 == TRUE
#error "TIMER0 cannot be used for system ticks when SOFTDEVICE present"
#endif

#if NRF5_ST_PRIORITY != 1
#error "ST priority must be 1 when SOFTDEVICE present"
#endif

#endif /* defined(SOFTDEVICE_PRESENT) */
#endif /* OSAL_ST_MODE != OSAL_ST_MODE_NONE */

#if OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING
#if defined(CH_CFG_ST_TIMEDELTA) && (CH_CFG_ST_TIMEDELTA < 5)
#error "CH_CFG_ST_TIMEDELTA is too low"
#endif
#if NRF5_ST_USE_TIMER0 == TRUE
#error "Freeruning (tick-less) mode not supported with TIMER, use RTC"
#endif
#endif /* OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING */

#if !OSAL_IRQ_IS_VALID_PRIORITY(NRF5_ST_PRIORITY)
#error "Invalid IRQ priority assigned to ST driver"
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
static inline systime_t st_lld_get_counter(void) {
#if NRF5_ST_USE_RTC0 == TRUE
  return (systime_t)NRF_RTC0->COUNTER;
#endif
#if NRF5_ST_USE_RTC1 == TRUE
  return (systime_t)NRF_RTC1->COUNTER;
#endif
#if NRF5_ST_USE_TIMER0 == TRUE
  return (systime_t)0;
#endif
}

/**
 * @brief   Starts the alarm.
 * @note    Makes sure that no spurious alarms are triggered after
 *          this call.
 *
 * @param[in] abstime   the time to be set for the first alarm
 *
 * @notapi
 */
static inline void st_lld_start_alarm(systime_t abstime) {
#if NRF5_ST_USE_RTC0 == TRUE
  NRF_RTC0->CC[0]               = abstime;
  NRF_RTC0->EVENTS_COMPARE[0]   = 0;
#if CORTEX_MODEL >= 4
  (void)NRF_RTC0->EVENTS_COMPARE[0];
#endif
  NRF_RTC0->EVTENSET            = RTC_EVTENSET_COMPARE0_Msk;
#endif
#if NRF5_ST_USE_RTC1 == TRUE
  NRF_RTC1->CC[0]               = abstime;
  NRF_RTC1->EVENTS_COMPARE[0]   = 0;
#if CORTEX_MODEL >= 4
  (void)NRF_RTC1->EVENTS_COMPARE[0];
#endif
  NRF_RTC1->EVTENSET            = RTC_EVTENSET_COMPARE0_Msk;
#endif
#if NRF5_ST_USE_TIMER0 == TRUE
  (void)abstime;
#endif
}

/**
 * @brief   Stops the alarm interrupt.
 *
 * @notapi
 */
static inline void st_lld_stop_alarm(void) {
#if NRF5_ST_USE_RTC0 == TRUE
  NRF_RTC0->EVTENCLR            = RTC_EVTENCLR_COMPARE0_Msk;
  NRF_RTC0->EVENTS_COMPARE[0]   = 0;
#if CORTEX_MODEL >= 4
  (void)NRF_RTC0->EVENTS_COMPARE[0];
#endif
#endif
#if NRF5_ST_USE_RTC1 == TRUE
  NRF_RTC1->EVTENCLR            = RTC_EVTENCLR_COMPARE0_Msk;
  NRF_RTC1->EVENTS_COMPARE[0]   = 0;
#if CORTEX_MODEL >= 4
  (void)NRF_RTC1->EVENTS_COMPARE[0];
#endif
#endif
}

/**
 * @brief   Sets the alarm time.
 *
 * @param[in] abstime   the time to be set for the next alarm
 *
 * @notapi
 */
static inline void st_lld_set_alarm(systime_t abstime) {
#if NRF5_ST_USE_RTC0 == TRUE
    NRF_RTC0->CC[0]             = abstime;
#endif
#if NRF5_ST_USE_RTC1 == TRUE
    NRF_RTC1->CC[0]             = abstime;
#endif
#if NRF5_ST_USE_TIMER0 == TRUE
  (void)abstime;
#endif
}

/**
 * @brief   Returns the current alarm time.
 *
 * @return              The currently set alarm time.
 *
 * @notapi
 */
static inline systime_t st_lld_get_alarm(void) {
#if NRF5_ST_USE_RTC0 == TRUE
  return (systime_t)NRF_RTC0->CC[0];
#endif
#if NRF5_ST_USE_RTC1 == TRUE
  return (systime_t)NRF_RTC1->CC[0];
#endif
#if NRF5_ST_USE_TIMER0 == TRUE
  return (systime_t)0;
#endif
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
static inline bool st_lld_is_alarm_active(void) {
#if NRF5_ST_USE_RTC0 == TRUE
  return NRF_RTC0->EVTEN & RTC_EVTEN_COMPARE0_Msk;
#endif
#if NRF5_ST_USE_RTC1 == TRUE
  return NRF_RTC1->EVTEN & RTC_EVTEN_COMPARE0_Msk;
#endif
#if NRF5_ST_USE_TIMER0 == TRUE
  return false;
#endif
}

#endif /* HAL_ST_LLD_H */

/** @} */
