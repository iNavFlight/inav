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
 * @file    hal_timcap.c
 * @brief   TIMCAP Driver code.
 *
 * @addtogroup TIMCAP
 * @{
 */

#include "hal_timcap.h"

#if HAL_USE_TIMCAP || defined(__DOXYGEN__)


/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   TIMCAP Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void timcapInit(void) {

  timcap_lld_init();
}

/**
 * @brief   Initializes the standard part of a @p TIMCAPDriver structure.
 *
 * @param[out] timcapp     pointer to the @p TIMCAPDriver object
 *
 * @init
 */
void timcapObjectInit(TIMCAPDriver *timcapp) {

  timcapp->state  = TIMCAP_STOP;
  timcapp->config = NULL;
}

/**
 * @brief   Configures and activates the TIMCAP peripheral.
 *
 * @param[in] timcapp      pointer to the @p TIMCAPDriver object
 * @param[in] config    pointer to the @p TIMCAPConfig object
 *
 * @api
 */
void timcapStart(TIMCAPDriver *timcapp, const TIMCAPConfig *config) {

  osalDbgCheck((timcapp != NULL) && (config != NULL));

  osalSysLock();
  osalDbgAssert((timcapp->state == TIMCAP_STOP) || (timcapp->state == TIMCAP_READY),
              "invalid state");
  timcapp->config = config;
  timcap_lld_start(timcapp);
  timcapp->state = TIMCAP_READY;
  osalSysUnlock();
}

/**
 * @brief   Deactivates the TIMCAP peripheral.
 *
 * @param[in] timcapp      pointer to the @p TIMCAPDriver object
 *
 * @api
 */
void timcapStop(TIMCAPDriver *timcapp) {

  osalDbgCheck(timcapp != NULL);

  osalSysLock();
  osalDbgAssert((timcapp->state == TIMCAP_STOP) || (timcapp->state == TIMCAP_READY),
              "invalid state");
  timcap_lld_stop(timcapp);
  timcapp->state = TIMCAP_STOP;
  osalSysUnlock();
}

/**
 * @brief   Enables the input capture.
 *
 * @param[in] timcapp      pointer to the @p TIMCAPDriver object
 *
 * @api
 */
void timcapEnable(TIMCAPDriver *timcapp) {

  osalDbgCheck(timcapp != NULL);

  osalSysLock();
  osalDbgAssert(timcapp->state == TIMCAP_READY, "invalid state");
  timcap_lld_enable(timcapp);
  timcapp->state = TIMCAP_WAITING;
  osalSysUnlock();
}

/**
 * @brief   Disables the input capture.
 *
 * @param[in] timcapp      pointer to the @p TIMCAPDriver object
 *
 * @api
 */
void timcapDisable(TIMCAPDriver *timcapp) {

  osalDbgCheck(timcapp != NULL);

  osalSysLock();
  osalDbgAssert((timcapp->state == TIMCAP_READY) || (timcapp->state == TIMCAP_WAITING) ||
              (timcapp->state == TIMCAP_ACTIVE) || (timcapp->state == TIMCAP_IDLE),
             "invalid state");
  timcap_lld_disable(timcapp);
  timcapp->state = TIMCAP_READY;
  osalSysUnlock();
}

#endif /* HAL_USE_TIMCAP */

/** @} */
