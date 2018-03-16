/*
    ChibiOS - Copyright (C) 2016 Andrew Wygle aka awygle

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
 * @file    MSP430X/hal_st_lld.h
 * @brief   MSP430X ST subsystem low level driver header.
 * @details This header is designed to be include-able without having to
 *          include other files from the HAL.
 *
 * @addtogroup MSP430X
 * @{
 */

#ifndef _ST_LLD_H_
#define _ST_LLD_H_

#include <msp430.h>

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief Timer maximum value
 */
#define MSP_TIMER_COUNTER_MAX 65535

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name      Configuration options
 * @{
 */
/**
 * @brief     System timer clock source.
 * 
 * @note      Legal values are undefined, MSP430X_ACLK_SRC, and 
 *            MSP430X_SMCLK_SRC. 
 * @note      If undefined, must define MSP430X_ST_CLK_FREQ as frequency of
 *            external clock and configure PAL appropriately.
 */
#if !defined (MSP430X_ST_CLK_SRC)
  #ifndef MSP430X_ST_CLK_FREQ
    #warning "Requested external source for ST but no frequency given"
    #warning "- assuming OSAL_ST_FREQUENCY"
    #define MSP430X_ST_CLK_FREQ OSAL_ST_FREQUENCY
  #endif
  #define MSP430X_ST_TASSEL TASSEL__TACLK
#elif MSP430X_ST_CLK_SRC == MSP430X_ACLK_SRC
  #define MSP430X_ST_CLK_FREQ MSP430X_ACLK_FREQ
  #define MSP430X_ST_TASSEL TASSEL__ACLK
#elif MSP430X_ST_CLK_SRC == MSP430X_SMCLK_SRC
  #define MSP430X_ST_CLK_FREQ MSP430X_SMCLK_FREQ
  #define MSP430X_ST_TASSEL TASSEL__SMCLK
#endif

/* Timers */
/**
 * @brief   Timer type (by letter) to be used for ST.
 * @note    Legal values are A and B. D support not yet implemented.
 * @note    Defaults to A
 */
#if !defined(MSP430X_ST_TIMER_TYPE)
  #define MSP430X_ST_TIMER_TYPE A
#endif
/**
 * @brief   Timer instance (by number) to be used for ST.
 * @note    Defaults to 0
 */
#if !defined (MSP430X_ST_TIMER_INDEX)
  #define MSP430X_ST_TIMER_INDEX 0
#endif 

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#define TIMER_HELPER2(x, y) x ## y
#define TIMER_HELPER(x, y) TIMER_HELPER2(x, y)
#define MSP430X_ST_TIMER TIMER_HELPER(MSP430X_ST_TIMER_TYPE, MSP430X_ST_TIMER_INDEX)
#define CCR_HELPER(x) T ## x ## CCR0
#define MSP430X_ST_CCR(x) CCR_HELPER(x)
#define CCTL_HELPER(x) T ## x ## CCTL0
#define MSP430X_ST_CCTL(x) CCTL_HELPER(x)
#define EX_HELPER(x) T ## x ## EX0
#define MSP430X_ST_EX(x) EX_HELPER(x)
#define CTL_HELPER(x) T ## x ## CTL
#define MSP430X_ST_CTL(x) CTL_HELPER(x)
#define R_HELPER(x) T ## x ## R
#define MSP430X_ST_R(x) R_HELPER(x)
#define ISR_HELPER2(x, y) TIMER ## y ## _ ## x ## 0_VECTOR
#define ISR_HELPER(x, y) ISR_HELPER2(x, y)
#define MSP430X_ST_ISR ISR_HELPER(MSP430X_ST_TIMER_TYPE, MSP430X_ST_TIMER_INDEX)

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

  return (systime_t)MSP430X_ST_R(MSP430X_ST_TIMER);
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

  MSP430X_ST_CCR(MSP430X_ST_TIMER) = abstime;
  
  /* Reset pending interrupt */
  MSP430X_ST_CCTL(MSP430X_ST_TIMER) &= (~CCIFG);
  
  /* Enable interrupt */
  MSP430X_ST_CCTL(MSP430X_ST_TIMER) |= CCIE;
}

/**
 * @brief   Stops the alarm interrupt.
 *
 * @notapi
 */
static inline void st_lld_stop_alarm(void) {

  MSP430X_ST_CCTL(MSP430X_ST_TIMER) &= (~CCIE);
}

/**
 * @brief   Sets the alarm time.
 *
 * @param[in] abstime   the time to be set for the next alarm
 *
 * @notapi
 */
static inline void st_lld_set_alarm(systime_t abstime) {

  MSP430X_ST_CCR(MSP430X_ST_TIMER) = abstime;
}

/**
 * @brief   Returns the current alarm time.
 *
 * @return              The currently set alarm time.
 *
 * @notapi
 */
static inline systime_t st_lld_get_alarm(void) {

  return MSP430X_ST_CCR(MSP430X_ST_TIMER);
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

  return (bool)((MSP430X_ST_CCTL(MSP430X_ST_TIMER) & CCIE) != 0);
}

#endif /* _ST_LLD_H_ */

/** @} */
