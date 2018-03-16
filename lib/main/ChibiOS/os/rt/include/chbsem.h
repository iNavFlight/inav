/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio.

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
 * @file    chbsem.h
 * @brief   Binary semaphores structures and macros.
 *
 * @addtogroup binary_semaphores
 * @details Binary semaphores related APIs and services.
 *          <h2>Operation mode</h2>
 *          Binary semaphores are implemented as a set of inline functions
 *          that use the existing counting semaphores primitives. The
 *          difference between counting and binary semaphores is that the
 *          counter of binary semaphores is not allowed to grow above the
 *          value 1. Repeated signal operation are ignored. A binary
 *          semaphore can thus have only two defined states:
 *          - <b>Taken</b>, when its counter has a value of zero or lower
 *            than zero. A negative number represent the number of threads
 *            queued on the binary semaphore.
 *          - <b>Not taken</b>, when its counter has a value of one.
 *          .
 *          Binary semaphores are different from mutexes because there is no
 *          concept of ownership, a binary semaphore can be taken by a
 *          thread and signaled by another thread or an interrupt handler,
 *          mutexes can only be taken and released by the same thread. Another
 *          difference is that binary semaphores, unlike mutexes, do not
 *          implement the priority inheritance protocol.<br>
 *          In order to use the binary semaphores APIs the
 *          @p CH_CFG_USE_SEMAPHORES option must be enabled in @p chconf.h.
 * @{
 */

#ifndef _CHBSEM_H_
#define _CHBSEM_H_

#if (CH_CFG_USE_SEMAPHORES == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/**
 * @extends semaphore_t
 *
 * @brief   Binary semaphore type.
 */
typedef struct {
  semaphore_t           bs_sem;
} binary_semaphore_t;

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Data part of a static semaphore initializer.
 * @details This macro should be used when statically initializing a semaphore
 *          that is part of a bigger structure.
 *
 * @param[in] name      the name of the semaphore variable
 * @param[in] taken     the semaphore initial state
 */
#define _BSEMAPHORE_DATA(name, taken)                                       \
  {_SEMAPHORE_DATA(name.bs_sem, ((taken) ? 0 : 1))}

/**
 * @brief   Static semaphore initializer.
 * @details Statically initialized semaphores require no explicit
 *          initialization using @p chBSemInit().
 *
 * @param[in] name      the name of the semaphore variable
 * @param[in] taken     the semaphore initial state
 */
#define BSEMAPHORE_DECL(name, taken)                                        \
    binary_semaphore_t name = _BSEMAPHORE_DATA(name, taken)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

/**
 * @brief   Initializes a binary semaphore.
 *
 * @param[out] bsp      pointer to a @p binary_semaphore_t structure
 * @param[in] taken     initial state of the binary semaphore:
 *                      - @a false, the initial state is not taken.
 *                      - @a true, the initial state is taken.
 *                      .
 *
 * @init
 */
static inline void chBSemObjectInit(binary_semaphore_t *bsp, bool taken) {

  chSemObjectInit(&bsp->bs_sem, taken ? (cnt_t)0 : (cnt_t)1);
}

/**
 * @brief   Wait operation on the binary semaphore.
 *
 * @param[in] bsp       pointer to a @p binary_semaphore_t structure
 * @return              A message specifying how the invoking thread has been
 *                      released from the semaphore.
 * @retval MSG_OK       if the binary semaphore has been successfully taken.
 * @retval MSG_RESET    if the binary semaphore has been reset using
 *                      @p bsemReset().
 *
 * @api
 */
static inline msg_t chBSemWait(binary_semaphore_t *bsp) {

  return chSemWait(&bsp->bs_sem);
}

/**
 * @brief   Wait operation on the binary semaphore.
 *
 * @param[in] bsp       pointer to a @p binary_semaphore_t structure
 * @return              A message specifying how the invoking thread has been
 *                      released from the semaphore.
 * @retval MSG_OK       if the binary semaphore has been successfully taken.
 * @retval MSG_RESET    if the binary semaphore has been reset using
 *                      @p bsemReset().
 *
 * @sclass
 */
static inline msg_t chBSemWaitS(binary_semaphore_t *bsp) {

  chDbgCheckClassS();

  return chSemWaitS(&bsp->bs_sem);
}

/**
 * @brief   Wait operation on the binary semaphore.
 *
 * @param[in] bsp       pointer to a @p binary_semaphore_t structure
 * @param[in] time      the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_IMMEDIATE immediate timeout.
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              A message specifying how the invoking thread has been
 *                      released from the semaphore.
 * @retval MSG_OK       if the binary semaphore has been successfully taken.
 * @retval MSG_RESET    if the binary semaphore has been reset using
 *                      @p bsemReset().
 * @retval MSG_TIMEOUT  if the binary semaphore has not been signaled or reset
 *                      within the specified timeout.
 *
 * @sclass
 */
static inline msg_t chBSemWaitTimeoutS(binary_semaphore_t *bsp,
                                       systime_t time) {

  chDbgCheckClassS();

  return chSemWaitTimeoutS(&bsp->bs_sem, time);
}

/**
 * @brief   Wait operation on the binary semaphore.
 *
 * @param[in] bsp       pointer to a @p binary_semaphore_t structure
 * @param[in] time      the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_IMMEDIATE immediate timeout.
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              A message specifying how the invoking thread has been
 *                      released from the semaphore.
 * @retval MSG_OK       if the binary semaphore has been successfully taken.
 * @retval MSG_RESET    if the binary semaphore has been reset using
 *                      @p bsemReset().
 * @retval MSG_TIMEOUT  if the binary semaphore has not been signaled or reset
 *                      within the specified timeout.
 *
 * @api
 */
static inline msg_t chBSemWaitTimeout(binary_semaphore_t *bsp,
                                      systime_t time) {

  return chSemWaitTimeout(&bsp->bs_sem, time);
}

/**
 * @brief   Reset operation on the binary semaphore.
 * @note    The released threads can recognize they were waked up by a reset
 *          rather than a signal because the @p bsemWait() will return
 *          @p MSG_RESET instead of @p MSG_OK.
 * @note    This function does not reschedule.
 *
 * @param[in] bsp       pointer to a @p binary_semaphore_t structure
 * @param[in] taken     new state of the binary semaphore
 *                      - @a false, the new state is not taken.
 *                      - @a true, the new state is taken.
 *                      .
 *
 * @iclass
 */
static inline void chBSemResetI(binary_semaphore_t *bsp, bool taken) {

  chDbgCheckClassI();

  chSemResetI(&bsp->bs_sem, taken ? (cnt_t)0 : (cnt_t)1);
}

/**
 * @brief   Reset operation on the binary semaphore.
 * @note    The released threads can recognize they were waked up by a reset
 *          rather than a signal because the @p bsemWait() will return
 *          @p MSG_RESET instead of @p MSG_OK.
 *
 * @param[in] bsp       pointer to a @p binary_semaphore_t structure
 * @param[in] taken     new state of the binary semaphore
 *                      - @a false, the new state is not taken.
 *                      - @a true, the new state is taken.
 *                      .
 *
 * @api
 */
static inline void chBSemReset(binary_semaphore_t *bsp, bool taken) {

  chSemReset(&bsp->bs_sem, taken ? (cnt_t)0 : (cnt_t)1);
}

/**
 * @brief   Performs a signal operation on a binary semaphore.
 * @note    This function does not reschedule.
 *
 * @param[in] bsp       pointer to a @p binary_semaphore_t structure
 *
 * @iclass
 */
static inline void chBSemSignalI(binary_semaphore_t *bsp) {

  chDbgCheckClassI();

  if (bsp->bs_sem.s_cnt < (cnt_t)1) {
    chSemSignalI(&bsp->bs_sem);
  }
}

/**
 * @brief   Performs a signal operation on a binary semaphore.
 *
 * @param[in] bsp       pointer to a @p binary_semaphore_t structure
 *
 * @api
 */
static inline void chBSemSignal(binary_semaphore_t *bsp) {

  chSysLock();
  chBSemSignalI(bsp);
  chSchRescheduleS();
  chSysUnlock();
}

/**
 * @brief   Returns the binary semaphore current state.
 *
 * @param[in] bsp       pointer to a @p binary_semaphore_t structure
 * @return              The binary semaphore current state.
 * @retval false        if the binary semaphore is not taken.
 * @retval true         if the binary semaphore is taken.
 *
 * @iclass
 */
static inline bool chBSemGetStateI(binary_semaphore_t *bsp) {

  chDbgCheckClassI();

  return (bsp->bs_sem.s_cnt > (cnt_t)0) ? false : true;
}

#endif /* CH_CFG_USE_SEMAPHORES == TRUE */

#endif /* _CHBSEM_H_ */

/** @} */
