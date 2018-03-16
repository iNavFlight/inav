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
 * @file    chsem.c
 * @brief   Semaphores code.
 *
 * @addtogroup semaphores
 * @details Semaphores related APIs and services.
 *          <h2>Operation mode</h2>
 *          Semaphores are a flexible synchronization primitive, ChibiOS/RT
 *          implements semaphores in their "counting semaphores" variant as
 *          defined by Edsger Dijkstra plus several enhancements like:
 *          - Wait operation with timeout.
 *          - Reset operation.
 *          - Atomic wait+signal operation.
 *          - Return message from the wait operation (OK, RESET, TIMEOUT).
 *          .
 *          The binary semaphores variant can be easily implemented using
 *          counting semaphores.<br>
 *          Operations defined for semaphores:
 *          - <b>Signal</b>: The semaphore counter is increased and if the
 *            result is non-positive then a waiting thread is removed from
 *            the semaphore queue and made ready for execution.
 *          - <b>Wait</b>: The semaphore counter is decreased and if the result
 *            becomes negative the thread is queued in the semaphore and
 *            suspended.
 *          - <b>Reset</b>: The semaphore counter is reset to a non-negative
 *            value and all the threads in the queue are released.
 *          .
 *          Semaphores can be used as guards for mutual exclusion zones
 *          (note that mutexes are recommended for this kind of use) but
 *          also have other uses, queues guards and counters for example.<br>
 *          Semaphores usually use a FIFO queuing strategy but it is possible
 *          to make them order threads by priority by enabling
 *          @p CH_CFG_USE_SEMAPHORES_PRIORITY in @p chconf.h.
 * @pre     In order to use the semaphore APIs the @p CH_CFG_USE_SEMAPHORES
 *          option must be enabled in @p chconf.h.
 * @{
 */

#include "ch.h"

#if (CH_CFG_USE_SEMAPHORES == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Module local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

#if CH_CFG_USE_SEMAPHORES_PRIORITY == TRUE
#define sem_insert(tp, qp) queue_prio_insert(tp, qp)
#else
#define sem_insert(tp, qp) queue_insert(tp, qp)
#endif

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Initializes a semaphore with the specified counter value.
 *
 * @param[out] sp       pointer to a @p semaphore_t structure
 * @param[in] n         initial value of the semaphore counter. Must be
 *                      non-negative.
 *
 * @init
 */
void chSemObjectInit(semaphore_t *sp, cnt_t n) {

  chDbgCheck((sp != NULL) && (n >= (cnt_t)0));

  queue_init(&sp->s_queue);
  sp->s_cnt = n;
}

/**
 * @brief   Performs a reset operation on the semaphore.
 * @post    After invoking this function all the threads waiting on the
 *          semaphore, if any, are released and the semaphore counter is set
 *          to the specified, non negative, value.
 * @note    The released threads can recognize they were waked up by a reset
 *          rather than a signal because the @p chSemWait() will return
 *          @p MSG_RESET instead of @p MSG_OK.
 *
 * @param[in] sp        pointer to a @p semaphore_t structure
 * @param[in] n         the new value of the semaphore counter. The value must
 *                      be non-negative.
 *
 * @api
 */
void chSemReset(semaphore_t *sp, cnt_t n) {

  chSysLock();
  chSemResetI(sp, n);
  chSchRescheduleS();
  chSysUnlock();
}

/**
 * @brief   Performs a reset operation on the semaphore.
 * @post    After invoking this function all the threads waiting on the
 *          semaphore, if any, are released and the semaphore counter is set
 *          to the specified, non negative, value.
 * @post    This function does not reschedule so a call to a rescheduling
 *          function must be performed before unlocking the kernel. Note that
 *          interrupt handlers always reschedule on exit so an explicit
 *          reschedule must not be performed in ISRs.
 * @note    The released threads can recognize they were waked up by a reset
 *          rather than a signal because the @p chSemWait() will return
 *          @p MSG_RESET instead of @p MSG_OK.
 *
 * @param[in] sp        pointer to a @p semaphore_t structure
 * @param[in] n         the new value of the semaphore counter. The value must
 *                      be non-negative.
 *
 * @iclass
 */
void chSemResetI(semaphore_t *sp, cnt_t n) {
  cnt_t cnt;

  chDbgCheckClassI();
  chDbgCheck((sp != NULL) && (n >= (cnt_t)0));
  chDbgAssert(((sp->s_cnt >= (cnt_t)0) && queue_isempty(&sp->s_queue)) ||
              ((sp->s_cnt < (cnt_t)0) && queue_notempty(&sp->s_queue)),
              "inconsistent semaphore");

  cnt = sp->s_cnt;
  sp->s_cnt = n;
  while (++cnt <= (cnt_t)0) {
    chSchReadyI(queue_lifo_remove(&sp->s_queue))->p_u.rdymsg = MSG_RESET;
  }
}

/**
 * @brief   Performs a wait operation on a semaphore.
 *
 * @param[in] sp        pointer to a @p semaphore_t structure
 * @return              A message specifying how the invoking thread has been
 *                      released from the semaphore.
 * @retval MSG_OK       if the thread has not stopped on the semaphore or the
 *                      semaphore has been signaled.
 * @retval MSG_RESET    if the semaphore has been reset using @p chSemReset().
 *
 * @api
 */
msg_t chSemWait(semaphore_t *sp) {
  msg_t msg;

  chSysLock();
  msg = chSemWaitS(sp);
  chSysUnlock();

  return msg;
}

/**
 * @brief   Performs a wait operation on a semaphore.
 *
 * @param[in] sp        pointer to a @p semaphore_t structure
 * @return              A message specifying how the invoking thread has been
 *                      released from the semaphore.
 * @retval MSG_OK       if the thread has not stopped on the semaphore or the
 *                      semaphore has been signaled.
 * @retval MSG_RESET    if the semaphore has been reset using @p chSemReset().
 *
 * @sclass
 */
msg_t chSemWaitS(semaphore_t *sp) {

  chDbgCheckClassS();
  chDbgCheck(sp != NULL);
  chDbgAssert(((sp->s_cnt >= (cnt_t)0) && queue_isempty(&sp->s_queue)) ||
              ((sp->s_cnt < (cnt_t)0) && queue_notempty(&sp->s_queue)),
              "inconsistent semaphore");

  if (--sp->s_cnt < (cnt_t)0) {
    currp->p_u.wtsemp = sp;
    sem_insert(currp, &sp->s_queue);
    chSchGoSleepS(CH_STATE_WTSEM);

    return currp->p_u.rdymsg;
  }

  return MSG_OK;
}

/**
 * @brief   Performs a wait operation on a semaphore with timeout specification.
 *
 * @param[in] sp        pointer to a @p semaphore_t structure
 * @param[in] time      the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_IMMEDIATE immediate timeout.
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              A message specifying how the invoking thread has been
 *                      released from the semaphore.
 * @retval MSG_OK       if the thread has not stopped on the semaphore or the
 *                      semaphore has been signaled.
 * @retval MSG_RESET    if the semaphore has been reset using @p chSemReset().
 * @retval MSG_TIMEOUT  if the semaphore has not been signaled or reset within
 *                      the specified timeout.
 *
 * @api
 */
msg_t chSemWaitTimeout(semaphore_t *sp, systime_t time) {
  msg_t msg;

  chSysLock();
  msg = chSemWaitTimeoutS(sp, time);
  chSysUnlock();

  return msg;
}

/**
 * @brief   Performs a wait operation on a semaphore with timeout specification.
 *
 * @param[in] sp        pointer to a @p semaphore_t structure
 * @param[in] time      the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_IMMEDIATE immediate timeout.
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              A message specifying how the invoking thread has been
 *                      released from the semaphore.
 * @retval MSG_OK       if the thread has not stopped on the semaphore or the
 *                      semaphore has been signaled.
 * @retval MSG_RESET    if the semaphore has been reset using @p chSemReset().
 * @retval MSG_TIMEOUT  if the semaphore has not been signaled or reset within
 *                      the specified timeout.
 *
 * @sclass
 */
msg_t chSemWaitTimeoutS(semaphore_t *sp, systime_t time) {

  chDbgCheckClassS();
  chDbgCheck(sp != NULL);
  chDbgAssert(((sp->s_cnt >= (cnt_t)0) && queue_isempty(&sp->s_queue)) ||
              ((sp->s_cnt < (cnt_t)0) && queue_notempty(&sp->s_queue)),
              "inconsistent semaphore");

  if (--sp->s_cnt < (cnt_t)0) {
    if (TIME_IMMEDIATE == time) {
      sp->s_cnt++;

      return MSG_TIMEOUT;
    }
    currp->p_u.wtsemp = sp;
    sem_insert(currp, &sp->s_queue);

    return chSchGoSleepTimeoutS(CH_STATE_WTSEM, time);
  }

  return MSG_OK;
}

/**
 * @brief   Performs a signal operation on a semaphore.
 *
 * @param[in] sp        pointer to a @p semaphore_t structure
 *
 * @api
 */
void chSemSignal(semaphore_t *sp) {

  chDbgCheck(sp != NULL);
  chDbgAssert(((sp->s_cnt >= (cnt_t)0) && queue_isempty(&sp->s_queue)) ||
              ((sp->s_cnt < (cnt_t)0) && queue_notempty(&sp->s_queue)),
              "inconsistent semaphore");

  chSysLock();
  if (++sp->s_cnt <= (cnt_t)0) {
    chSchWakeupS(queue_fifo_remove(&sp->s_queue), MSG_OK);
  }
  chSysUnlock();
}

/**
 * @brief   Performs a signal operation on a semaphore.
 * @post    This function does not reschedule so a call to a rescheduling
 *          function must be performed before unlocking the kernel. Note that
 *          interrupt handlers always reschedule on exit so an explicit
 *          reschedule must not be performed in ISRs.
 *
 * @param[in] sp    pointer to a @p semaphore_t structure
 *
 * @iclass
 */
void chSemSignalI(semaphore_t *sp) {

  chDbgCheckClassI();
  chDbgCheck(sp != NULL);
  chDbgAssert(((sp->s_cnt >= (cnt_t)0) && queue_isempty(&sp->s_queue)) ||
              ((sp->s_cnt < (cnt_t)0) && queue_notempty(&sp->s_queue)),
              "inconsistent semaphore");

  if (++sp->s_cnt <= (cnt_t)0) {
    /* Note, it is done this way in order to allow a tail call on
             chSchReadyI().*/
    thread_t *tp = queue_fifo_remove(&sp->s_queue);
    tp->p_u.rdymsg = MSG_OK;
    (void) chSchReadyI(tp);
  }
}

/**
 * @brief   Adds the specified value to the semaphore counter.
 * @post    This function does not reschedule so a call to a rescheduling
 *          function must be performed before unlocking the kernel. Note that
 *          interrupt handlers always reschedule on exit so an explicit
 *          reschedule must not be performed in ISRs.
 *
 * @param[in] sp        pointer to a @p semaphore_t structure
 * @param[in] n         value to be added to the semaphore counter. The value
 *                      must be positive.
 *
 * @iclass
 */
void chSemAddCounterI(semaphore_t *sp, cnt_t n) {

  chDbgCheckClassI();
  chDbgCheck((sp != NULL) && (n > (cnt_t)0));
  chDbgAssert(((sp->s_cnt >= (cnt_t)0) && queue_isempty(&sp->s_queue)) ||
              ((sp->s_cnt < (cnt_t)0) && queue_notempty(&sp->s_queue)),
              "inconsistent semaphore");

  while (n > (cnt_t)0) {
    if (++sp->s_cnt <= (cnt_t)0) {
      chSchReadyI(queue_fifo_remove(&sp->s_queue))->p_u.rdymsg = MSG_OK;
    }
    n--;
  }
}

/**
 * @brief   Performs atomic signal and wait operations on two semaphores.
 *
 * @param[in] sps       pointer to a @p semaphore_t structure to be signaled
 * @param[in] spw       pointer to a @p semaphore_t structure to wait on
 * @return              A message specifying how the invoking thread has been
 *                      released from the semaphore.
 * @retval MSG_OK       if the thread has not stopped on the semaphore or the
 *                      semaphore has been signaled.
 * @retval MSG_RESET    if the semaphore has been reset using @p chSemReset().
 *
 * @api
 */
msg_t chSemSignalWait(semaphore_t *sps, semaphore_t *spw) {
  msg_t msg;

  chDbgCheck((sps != NULL) && (spw != NULL));
  chDbgAssert(((sps->s_cnt >= (cnt_t)0) && queue_isempty(&sps->s_queue)) ||
              ((sps->s_cnt < (cnt_t)0) && queue_notempty(&sps->s_queue)),
              "inconsistent semaphore");
  chDbgAssert(((spw->s_cnt >= (cnt_t)0) && queue_isempty(&spw->s_queue)) ||
              ((spw->s_cnt < (cnt_t)0) && queue_notempty(&spw->s_queue)),
              "inconsistent semaphore");

  chSysLock();
  if (++sps->s_cnt <= (cnt_t)0) {
    chSchReadyI(queue_fifo_remove(&sps->s_queue))->p_u.rdymsg = MSG_OK;
  }
  if (--spw->s_cnt < (cnt_t)0) {
    thread_t *ctp = currp;
    sem_insert(ctp, &spw->s_queue);
    ctp->p_u.wtsemp = spw;
    chSchGoSleepS(CH_STATE_WTSEM);
    msg = ctp->p_u.rdymsg;
  }
  else {
    chSchRescheduleS();
    msg = MSG_OK;
  }
  chSysUnlock();

  return msg;
}

#endif /* CH_CFG_USE_SEMAPHORES == TRUE */

/** @} */
