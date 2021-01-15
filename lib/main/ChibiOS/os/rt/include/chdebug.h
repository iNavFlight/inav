/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio.

    This file is part of ChibiOS.

    ChibiOS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file    chdebug.h
 * @brief   Debug support macros and structures.
 *
 * @addtogroup checks_assertions
 * @{
 */

#ifndef CHDEBUG_H
#define CHDEBUG_H

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Debug related settings
 * @{
 */
/**
 * @brief   Fill value for thread stack area in debug mode.
 */
#if !defined(CH_DBG_STACK_FILL_VALUE) || defined(__DOXYGEN__)
#define CH_DBG_STACK_FILL_VALUE             0x55
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

#if CH_DBG_SYSTEM_STATE_CHECK == TRUE
#define _dbg_enter_lock() (ch.dbg.lock_cnt = (cnt_t)1)
#define _dbg_leave_lock() (ch.dbg.lock_cnt = (cnt_t)0)
#endif

/* When the state checker feature is disabled then the following functions
   are replaced by an empty macro.*/
#if CH_DBG_SYSTEM_STATE_CHECK == FALSE
#define _dbg_enter_lock()
#define _dbg_leave_lock()
#define _dbg_check_disable()
#define _dbg_check_suspend()
#define _dbg_check_enable()
#define _dbg_check_lock()
#define _dbg_check_unlock()
#define _dbg_check_lock_from_isr()
#define _dbg_check_unlock_from_isr()
#define _dbg_check_enter_isr()
#define _dbg_check_leave_isr()
#define chDbgCheckClassI()
#define chDbgCheckClassS()
#endif

/**
 * @name    Macro Functions
 * @{
 */
/**
 * @brief   Function parameters check.
 * @details If the condition check fails then the kernel panics and halts.
 * @note    The condition is tested only if the @p CH_DBG_ENABLE_CHECKS switch
 *          is specified in @p chconf.h else the macro does nothing.
 *
 * @param[in] c         the condition to be verified to be true
 *
 * @api
 */
#if !defined(chDbgCheck)
#define chDbgCheck(c) do {                                                  \
  /*lint -save -e506 -e774 [2.1, 14.3] Can be a constant by design.*/       \
  if (CH_DBG_ENABLE_CHECKS != FALSE) {                                      \
    if (!(c)) {                                                             \
  /*lint -restore*/                                                         \
      chSysHalt(__func__);                                                  \
    }                                                                       \
  }                                                                         \
} while (false)
#endif /* !defined(chDbgCheck) */

/**
 * @brief   Condition assertion.
 * @details If the condition check fails then the kernel panics with a
 *          message and halts.
 * @note    The condition is tested only if the @p CH_DBG_ENABLE_ASSERTS switch
 *          is specified in @p chconf.h else the macro does nothing.
 * @note    The remark string is not currently used except for putting a
 *          comment in the code about the assertion.
 *
 * @param[in] c         the condition to be verified to be true
 * @param[in] r         a remark string
 *
 * @api
 */
#if !defined(chDbgAssert)
#define chDbgAssert(c, r) do {                                              \
  /*lint -save -e506 -e774 [2.1, 14.3] Can be a constant by design.*/       \
  if (CH_DBG_ENABLE_ASSERTS != FALSE) {                                     \
    if (!(c)) {                                                             \
  /*lint -restore*/                                                         \
      chSysHalt(__func__);                                                  \
    }                                                                       \
  }                                                                         \
} while (false)
#endif /* !defined(chDbgAssert) */
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
#if CH_DBG_SYSTEM_STATE_CHECK == TRUE
  void _dbg_check_disable(void);
  void _dbg_check_suspend(void);
  void _dbg_check_enable(void);
  void _dbg_check_lock(void);
  void _dbg_check_unlock(void);
  void _dbg_check_lock_from_isr(void);
  void _dbg_check_unlock_from_isr(void);
  void _dbg_check_enter_isr(void);
  void _dbg_check_leave_isr(void);
  void chDbgCheckClassI(void);
  void chDbgCheckClassS(void);
#endif
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

#endif /* CHDEBUG_H */

/** @} */
