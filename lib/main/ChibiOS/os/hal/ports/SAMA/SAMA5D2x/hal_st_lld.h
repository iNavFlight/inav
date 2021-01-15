/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

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
 * @file    SAMA5D2x/hal_st_lld.h
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
#if !defined(SAMA_ST_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_ST_IRQ_PRIORITY                0
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/
/* Only one source for st */
#if SAMA_ST_USE_TC0
#if defined(ST_ASSIGNED)
#error "ST already assigned"
#else
#define ST_ASSIGNED
#endif
#endif

/* Only one source for st */
#if SAMA_ST_USE_TC1
#if defined(ST_ASSIGNED)
#error "ST already assigned"
#else
#define ST_ASSIGNED
#endif
#endif

/* Only one source for st */
#if SAMA_ST_USE_PIT
#if defined(ST_ASSIGNED)
#error "ST already assigned"
#else
#define ST_ASSIGNED
#endif
#endif

/* Checks on allocation of TCx units.*/
#if SAMA_ST_USE_TC0
#if defined(SAMA_TC0_IS_USED)
#error "ST requires TC0 but the peripheral is already used"
#else
#define SAMA_TC0_IS_USED
#endif
#endif

#if SAMA_ST_USE_TC1
#if defined(SAMA_TC1_IS_USED)
#error "ST requires TC1 but the peripheral is already used"
#else
#define SAMA_TC1_IS_USED
#endif
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

#if (SAMA_ST_USE_TC0 || SAMA_ST_USE_TC1)

#if SAMA_ST_USE_TC0

  Tc *tcp = TC0;
#endif
#if SAMA_ST_USE_TC1

  Tc *tcp = TC1;
#endif

  return (systime_t)tcp->TC_CHANNEL[0].TC_CV;
#else

  return (systime_t)0;
#endif
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
static inline void st_lld_start_alarm(systime_t time) {

#if (SAMA_ST_USE_TC0 || SAMA_ST_USE_TC1)

#if SAMA_ST_USE_TC0

  Tc *tcp = TC0;
#endif
#if SAMA_ST_USE_TC1

  Tc *tcp = TC1;
#endif

  tcp->TC_WPMR = TC_WPMR_WPKEY_PASSWD;
  tcp->TC_CHANNEL[0].TC_RC = TC_RC_RC((uint32_t)time);
  tcp->TC_CHANNEL[0].TC_SR;
  tcp->TC_CHANNEL[0].TC_IER = TC_IER_CPCS;
  tcp->TC_WPMR = TC_WPMR_WPKEY_PASSWD | TC_WPMR_WPEN;
#else

  (void)time;
#endif
}

/**
 * @brief   Stops the alarm interrupt.
 *
 * @notapi
 */
static inline void st_lld_stop_alarm(void) {

#if (SAMA_ST_USE_TC0 || SAMA_ST_USE_TC1)

#if SAMA_ST_USE_TC0

  Tc *tcp = TC0;
#endif
#if SAMA_ST_USE_TC1

  Tc *tcp = TC1;
#endif

  tcp->TC_WPMR = TC_WPMR_WPKEY_PASSWD;
  tcp->TC_CHANNEL[0].TC_IDR = TC_IDR_CPCS;
  tcp->TC_WPMR = TC_WPMR_WPKEY_PASSWD | TC_WPMR_WPEN;
#endif
}

/**
 * @brief   Sets the alarm time.
 *
 * @param[in] time      the time to be set for the next alarm
 *
 * @notapi
 */
static inline void st_lld_set_alarm(systime_t time) {

#if (SAMA_ST_USE_TC0 || SAMA_ST_USE_TC1)

#if SAMA_ST_USE_TC0

  Tc *tcp = TC0;
#endif
#if SAMA_ST_USE_TC1

  Tc *tcp = TC1;
#endif

  tcp->TC_WPMR = TC_WPMR_WPKEY_PASSWD;
  tcp->TC_CHANNEL[0].TC_RC = TC_RC_RC((uint32_t)time);
  tcp->TC_WPMR = TC_WPMR_WPKEY_PASSWD | TC_WPMR_WPEN;
#else

  (void)time;
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

#if (SAMA_ST_USE_TC0 || SAMA_ST_USE_TC1)

#if SAMA_ST_USE_TC0

  Tc *tcp = TC0;
#endif
#if SAMA_ST_USE_TC1

  Tc *tcp = TC1;
#endif

  return (systime_t)tcp->TC_CHANNEL[0].TC_RC;
#else

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

#if (SAMA_ST_USE_TC0 || SAMA_ST_USE_TC1)

#if SAMA_ST_USE_TC0

  Tc *tcp = TC0;
#endif
#if SAMA_ST_USE_TC1

  Tc *tcp = TC1;
#endif

  return (bool)((tcp->TC_CHANNEL[0].TC_IMR & TC_IMR_CPCS) != 0);
#else

  return false;
#endif
}

#endif /* HAL_ST_LLD_H */

/** @} */

