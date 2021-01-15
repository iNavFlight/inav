/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012,2013 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file    hal_timcap.h
 * @brief   TIMCAP Driver macros and structures.
 *
 * @addtogroup TIMCAP
 * @{
 */

#ifndef HAL_TIMCAP_H_
#define HAL_TIMCAP_H_

#include "hal.h"

#if (HAL_USE_TIMCAP == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

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
  TIMCAP_UNINIT = 0,                   /**< Not initialized.                   */
  TIMCAP_STOP = 1,                     /**< Stopped.                           */
  TIMCAP_READY = 2,                    /**< Ready.                             */
  TIMCAP_WAITING = 3,                  /**< Waiting first edge.                */
  TIMCAP_ACTIVE = 4,                   /**< Active cycle phase.                */
  TIMCAP_IDLE = 5,                     /**< Idle cycle phase.                  */
} timcapstate_t;

/**
 * @brief   Type of a structure representing an TIMCAP driver.
 */
typedef struct TIMCAPDriver TIMCAPDriver;


/**
 * @brief   TIMCAP notification callback type.
 *
 * @param[in] timcapp      pointer to a @p TIMCAPDriver object
 */
typedef void (*timcapcallback_t)(TIMCAPDriver *timcapp);

#include "hal_timcap_lld.h"

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @name    Macro Functions
 * @{
 */
/**
 * @brief   Enables the input capture.
 *
 * @param[in] timcapp      pointer to the @p TIMCAPDriver object
 *
 * @iclass
 */
#define timcapEnableI(timcapp) timcap_lld_enable(timcapp)

/**
 * @brief   Disables the input capture.
 *
 * @param[in] timcapp      pointer to the @p TIMCAPDriver object
 *
 * @iclass
 */
#define timcapDisableI(timcapp) timcap_lld_disable(timcapp)




/** @} */

/**
 * @name    Low Level driver helper macros
 * @{
 */


/**
 * @brief   Common ISR code, TIMCAP channel 1 event.
 *
 * @param[in] timcapp      pointer to the @p TIMCAPDriver object
 *
 * @notapi
 */
#define _timcap_isr_invoke_channel1_cb(timcapp) {                                   \
  timcapstate_t previous_state = (timcapp)->state;                                \
  (timcapp)->state = TIMCAP_ACTIVE;                                               \
  if (previous_state != TIMCAP_WAITING)                                        \
    (timcapp)->config->capture_cb_array[0](timcapp);                                        \
}

/**
 * @brief   Common ISR code, TIMCAP channel 2 event.
 *
 * @param[in] timcapp      pointer to the @p TIMCAPDriver object
 *
 * @notapi
 */
#define _timcap_isr_invoke_channel2_cb(timcapp) {                                   \
  timcapstate_t previous_state = (timcapp)->state;                                \
  (timcapp)->state = TIMCAP_ACTIVE;                                               \
  if (previous_state != TIMCAP_WAITING)                                        \
    (timcapp)->config->capture_cb_array[1](timcapp);                                        \
}

/**
 * @brief   Common ISR code, TIMCAP channel 3 event.
 *
 * @param[in] timcapp      pointer to the @p TIMCAPDriver object
 *
 * @notapi
 */
#define _timcap_isr_invoke_channel3_cb(timcapp) {                                   \
  timcapstate_t previous_state = (timcapp)->state;                                \
  (timcapp)->state = TIMCAP_ACTIVE;                                               \
  if (previous_state != TIMCAP_WAITING)                                        \
    (timcapp)->config->capture_cb_array[2](timcapp);                                        \
}

/**
 * @brief   Common ISR code, TIMCAP channel 4 event.
 *
 * @param[in] timcapp      pointer to the @p TIMCAPDriver object
 *
 * @notapi
 */
#define _timcap_isr_invoke_channel4_cb(timcapp) {                                   \
  timcapstate_t previous_state = (timcapp)->state;                                \
  (timcapp)->state = TIMCAP_ACTIVE;                                               \
  if (previous_state != TIMCAP_WAITING)                                        \
    (timcapp)->config->capture_cb_array[3](timcapp);                                        \
}

/**
 * @brief   Common ISR code, TIMCAP timer overflow event.
 *
 * @param[in] timcapp      pointer to the @p TIMCAPDriver object
 *
 * @notapi
 */
#define _timcap_isr_invoke_overflow_cb(timcapp) {                                 \
  (timcapp)->config->overflow_cb(timcapp);                                        \
}
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void timcapInit(void);
  void timcapObjectInit(TIMCAPDriver *timcapp);
  void timcapStart(TIMCAPDriver *timcapp, const TIMCAPConfig *config);
  void timcapStop(TIMCAPDriver *timcapp);
  void timcapEnable(TIMCAPDriver *timcapp);
  void timcapDisable(TIMCAPDriver *timcapp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_TIMCAP */

#endif /* HAL_TIMCAP_H_ */

/** @} */
