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
 * @file    hal_st.c
 * @brief   ST Driver code.
 *
 * @addtogroup ST
 * @{
 */

#include "hal.h"

#if (OSAL_ST_MODE != OSAL_ST_MODE_NONE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   ST Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void stInit(void) {

  st_lld_init();
}

#if (OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING) || defined(__DOXYGEN__)
/**
 * @brief   Starts the alarm.
 * @note    Makes sure that no spurious alarms are triggered after
 *          this call.
 * @note    This functionality is only available in free running mode, the
 *          behavior in periodic mode is undefined.
 *
 * @param[in] abstime   the time to be set for the first alarm
 *
 * @api
 */
void stStartAlarm(systime_t abstime) {

  osalDbgAssert(stIsAlarmActive() == false, "already active");

  st_lld_start_alarm(abstime);
}

/**
 * @brief   Stops the alarm interrupt.
 * @note    This functionality is only available in free running mode, the
 *          behavior in periodic mode is undefined.
 *
 * @api
 */
void stStopAlarm(void) {

  st_lld_stop_alarm();
}

/**
 * @brief   Sets the alarm time.
 * @note    This functionality is only available in free running mode, the
 *          behavior in periodic mode is undefined.
 *
 * @param[in] abstime   the time to be set for the next alarm
 *
 * @api
 */
void stSetAlarm(systime_t abstime) {

  osalDbgAssert(stIsAlarmActive() != false, "not active");

  st_lld_set_alarm(abstime);
}

/**
 * @brief   Returns the current alarm time.
 * @note    This functionality is only available in free running mode, the
 *          behavior in periodic mode is undefined.
 *
 * @return              The currently set alarm time.
 *
 * @api
 */
systime_t stGetAlarm(void) {

  osalDbgAssert(stIsAlarmActive() != false, "not active");

  return st_lld_get_alarm();
}
#endif /* OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING */

#endif /* OSAL_ST_MODE != OSAL_ST_MODE_NONE */

/** @} */
