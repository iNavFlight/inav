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
 * @file    chschd.c
 * @brief   Scheduler code.
 *
 * @addtogroup scheduler
 * @details This module provides the default portable scheduler code.
 * @{
 */

#include "ch.h"

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   System data structures.
 */
ch_system_t ch;

/*===========================================================================*/
/* Module local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Scheduler initialization.
 *
 * @notapi
 */
void _scheduler_init(void) {

  queue_init(&ch.rlist.r_queue);
  ch.rlist.r_prio = NOPRIO;
#if CH_CFG_USE_REGISTRY == TRUE
  ch.rlist.r_newer = (thread_t *)&ch.rlist;
  ch.rlist.r_older = (thread_t *)&ch.rlist;
#endif
}

#if (CH_CFG_OPTIMIZE_SPEED == FALSE) || defined(__DOXYGEN__)
/**
 * @brief   Inserts a thread into a priority ordered queue.
 * @note    The insertion is done by scanning the list from the highest
 *          priority toward the lowest.
 *
 * @param[in] tp        the pointer to the thread to be inserted in the list
 * @param[in] tqp       the pointer to the threads list header
 *
 * @notapi
 */
void queue_prio_insert(thread_t *tp, threads_queue_t *tqp) {

  thread_t *cp = (thread_t *)tqp;
  do {
    cp = cp->p_next;
  } while ((cp != (thread_t *)tqp) && (cp->p_prio >= tp->p_prio));
  tp->p_next = cp;
  tp->p_prev = cp->p_prev;
  tp->p_prev->p_next = tp;
  cp->p_prev = tp;
}

/**
 * @brief   Inserts a thread into a queue.
 *
 * @param[in] tp        the pointer to the thread to be inserted in the list
 * @param[in] tqp       the pointer to the threads list header
 *
 * @notapi
 */
void queue_insert(thread_t *tp, threads_queue_t *tqp) {

  tp->p_next = (thread_t *)tqp;
  tp->p_prev = tqp->p_prev;
  tp->p_prev->p_next = tp;
  tqp->p_prev = tp;
}

/**
 * @brief   Removes the first-out thread from a queue and returns it.
 * @note    If the queue is priority ordered then this function returns the
 *          thread with the highest priority.
 *
 * @param[in] tqp       the pointer to the threads list header
 * @return              The removed thread pointer.
 *
 * @notapi
 */
thread_t *queue_fifo_remove(threads_queue_t *tqp) {
  thread_t *tp = tqp->p_next;

  tqp->p_next = tp->p_next;
  tqp->p_next->p_prev = (thread_t *)tqp;

  return tp;
}

/**
 * @brief   Removes the last-out thread from a queue and returns it.
 * @note    If the queue is priority ordered then this function returns the
 *          thread with the lowest priority.
 *
 * @param[in] tqp   the pointer to the threads list header
 * @return          The removed thread pointer.
 *
 * @notapi
 */
thread_t *queue_lifo_remove(threads_queue_t *tqp) {
  thread_t *tp = tqp->p_prev;

  tqp->p_prev = tp->p_prev;
  tqp->p_prev->p_next = (thread_t *)tqp;

  return tp;
}

/**
 * @brief   Removes a thread from a queue and returns it.
 * @details The thread is removed from the queue regardless of its relative
 *          position and regardless the used insertion method.
 *
 * @param[in] tp        the pointer to the thread to be removed from the queue
 * @return              The removed thread pointer.
 *
 * @notapi
 */
thread_t *queue_dequeue(thread_t *tp) {

  tp->p_prev->p_next = tp->p_next;
  tp->p_next->p_prev = tp->p_prev;

  return tp;
}

/**
 * @brief   Pushes a thread_t on top of a stack list.
 *
 * @param[in] tp    the pointer to the thread to be inserted in the list
 * @param[in] tlp   the pointer to the threads list header
 *
 * @notapi
 */
void list_insert(thread_t *tp, threads_list_t *tlp) {

  tp->p_next = tlp->p_next;
  tlp->p_next = tp;
}

/**
 * @brief   Pops a thread from the top of a stack list and returns it.
 * @pre     The list must be non-empty before calling this function.
 *
 * @param[in] tlp       the pointer to the threads list header
 * @return              The removed thread pointer.
 *
 * @notapi
 */
thread_t *list_remove(threads_list_t *tlp) {

  thread_t *tp = tlp->p_next;
  tlp->p_next = tp->p_next;

  return tp;
}
#endif /* CH_CFG_OPTIMIZE_SPEED */

/**
 * @brief   Inserts a thread in the Ready List.
 * @details The thread is positioned behind all threads with higher or equal
 *          priority.
 * @pre     The thread must not be already inserted in any list through its
 *          @p p_next and @p p_prev or list corruption would occur.
 * @post    This function does not reschedule so a call to a rescheduling
 *          function must be performed before unlocking the kernel. Note that
 *          interrupt handlers always reschedule on exit so an explicit
 *          reschedule must not be performed in ISRs.
 *
 * @param[in] tp        the thread to be made ready
 * @return              The thread pointer.
 *
 * @iclass
 */
thread_t *chSchReadyI(thread_t *tp) {
  thread_t *cp;

  chDbgCheckClassI();
  chDbgCheck(tp != NULL);
  chDbgAssert((tp->p_state != CH_STATE_READY) &&
              (tp->p_state != CH_STATE_FINAL),
              "invalid state");

  tp->p_state = CH_STATE_READY;
  cp = (thread_t *)&ch.rlist.r_queue;
  do {
    cp = cp->p_next;
  } while (cp->p_prio >= tp->p_prio);
  /* Insertion on p_prev.*/
  tp->p_next = cp;
  tp->p_prev = cp->p_prev;
  tp->p_prev->p_next = tp;
  cp->p_prev = tp;

  return tp;
}

/**
 * @brief   Puts the current thread to sleep into the specified state.
 * @details The thread goes into a sleeping state. The possible
 *          @ref thread_states are defined into @p threads.h.
 *
 * @param[in] newstate  the new thread state
 *
 * @sclass
 */
void chSchGoSleepS(tstate_t newstate) {
  thread_t *otp;

  chDbgCheckClassS();

  otp = currp;
  otp->p_state = newstate;
#if CH_CFG_TIME_QUANTUM > 0
  /* The thread is renouncing its remaining time slices so it will have a new
     time quantum when it will wakeup.*/
  otp->p_preempt = (tslices_t)CH_CFG_TIME_QUANTUM;
#endif
  setcurrp(queue_fifo_remove(&ch.rlist.r_queue));
#if defined(CH_CFG_IDLE_ENTER_HOOK)
  if (currp->p_prio == IDLEPRIO) {
    CH_CFG_IDLE_ENTER_HOOK();
  }
#endif
  currp->p_state = CH_STATE_CURRENT;
  chSysSwitch(currp, otp);
}

/*
 * Timeout wakeup callback.
 */
static void wakeup(void *p) {
  thread_t *tp = (thread_t *)p;

  chSysLockFromISR();
  switch (tp->p_state) {
  case CH_STATE_READY:
    /* Handling the special case where the thread has been made ready by
       another thread with higher priority.*/
    chSysUnlockFromISR();
    return;
  case CH_STATE_SUSPENDED:
    *tp->p_u.wttrp = NULL;
    break;
#if CH_CFG_USE_SEMAPHORES == TRUE
  case CH_STATE_WTSEM:
    chSemFastSignalI(tp->p_u.wtsemp);
    /* Falls into, intentional. */
#endif
#if (CH_CFG_USE_CONDVARS == TRUE) && (CH_CFG_USE_CONDVARS_TIMEOUT == TRUE)
  case CH_STATE_WTCOND:
#endif
  case CH_STATE_QUEUED:
    /* States requiring dequeuing.*/
    (void) queue_dequeue(tp);
    break;
  default:
    /* Any other state, nothing to do.*/
    break;
  }
  tp->p_u.rdymsg = MSG_TIMEOUT;
  (void) chSchReadyI(tp);
  chSysUnlockFromISR();
}

/**
 * @brief   Puts the current thread to sleep into the specified state with
 *          timeout specification.
 * @details The thread goes into a sleeping state, if it is not awakened
 *          explicitly within the specified timeout then it is forcibly
 *          awakened with a @p MSG_TIMEOUT low level message. The possible
 *          @ref thread_states are defined into @p threads.h.
 *
 * @param[in] newstate  the new thread state
 * @param[in] time      the number of ticks before the operation timeouts, the
 *                      special values are handled as follow:
 *                      - @a TIME_INFINITE the thread enters an infinite sleep
 *                        state, this is equivalent to invoking
 *                        @p chSchGoSleepS() but, of course, less efficient.
 *                      - @a TIME_IMMEDIATE this value is not allowed.
 *                      .
 * @return              The wakeup message.
 * @retval MSG_TIMEOUT  if a timeout occurs.
 *
 * @sclass
 */
msg_t chSchGoSleepTimeoutS(tstate_t newstate, systime_t time) {

  chDbgCheckClassS();

  if (TIME_INFINITE != time) {
    virtual_timer_t vt;

    chVTDoSetI(&vt, time, wakeup, currp);
    chSchGoSleepS(newstate);
    if (chVTIsArmedI(&vt)) {
      chVTDoResetI(&vt);
    }
  }
  else {
    chSchGoSleepS(newstate);
  }

  return currp->p_u.rdymsg;
}

/**
 * @brief   Wakes up a thread.
 * @details The thread is inserted into the ready list or immediately made
 *          running depending on its relative priority compared to the current
 *          thread.
 * @pre     The thread must not be already inserted in any list through its
 *          @p p_next and @p p_prev or list corruption would occur.
 * @note    It is equivalent to a @p chSchReadyI() followed by a
 *          @p chSchRescheduleS() but much more efficient.
 * @note    The function assumes that the current thread has the highest
 *          priority.
 *
 * @param[in] ntp       the thread to be made ready
 * @param[in] msg       the wakeup message
 *
 * @sclass
 */
void chSchWakeupS(thread_t *ntp, msg_t msg) {

  chDbgCheckClassS();

  chDbgAssert((ch.rlist.r_queue.p_next == (thread_t *)&ch.rlist.r_queue) ||
              (ch.rlist.r_current->p_prio >= ch.rlist.r_queue.p_next->p_prio),
              "priority order violation");

  /* Storing the message to be retrieved by the target thread when it will
     restart execution.*/
  ntp->p_u.rdymsg = msg;

  /* If the waken thread has a not-greater priority than the current
     one then it is just inserted in the ready list else it made
     running immediately and the invoking thread goes in the ready
     list instead.*/
  if (ntp->p_prio <= currp->p_prio) {
    (void) chSchReadyI(ntp);
  }
  else {
    thread_t *otp = chSchReadyI(currp);
    setcurrp(ntp);
#if defined(CH_CFG_IDLE_LEAVE_HOOK)
    if (otp->p_prio == IDLEPRIO) {
      CH_CFG_IDLE_LEAVE_HOOK();
    }
#endif
    ntp->p_state = CH_STATE_CURRENT;
    chSysSwitch(ntp, otp);
  }
}

/**
 * @brief   Performs a reschedule if a higher priority thread is runnable.
 * @details If a thread with a higher priority than the current thread is in
 *          the ready list then make the higher priority thread running.
 *
 * @sclass
 */
void chSchRescheduleS(void) {

  chDbgCheckClassS();

  if (chSchIsRescRequiredI()) {
    chSchDoRescheduleAhead();
  }
}

/**
 * @brief   Evaluates if preemption is required.
 * @details The decision is taken by comparing the relative priorities and
 *          depending on the state of the round robin timeout counter.
 * @note    Not a user function, it is meant to be invoked by the scheduler
 *          itself or from within the port layer.
 *
 * @retval true         if there is a thread that must go in running state
 *                      immediately.
 * @retval false        if preemption is not required.
 *
 * @special
 */
bool chSchIsPreemptionRequired(void) {
  tprio_t p1 = firstprio(&ch.rlist.r_queue);
  tprio_t p2 = currp->p_prio;

#if CH_CFG_TIME_QUANTUM > 0
  /* If the running thread has not reached its time quantum, reschedule only
     if the first thread on the ready queue has a higher priority.
     Otherwise, if the running thread has used up its time quantum, reschedule
     if the first thread on the ready queue has equal or higher priority.*/
  return (currp->p_preempt > (tslices_t)0) ? (p1 > p2) : (p1 >= p2);
#else
  /* If the round robin preemption feature is not enabled then performs a
     simpler comparison.*/
  return p1 > p2;
#endif
}

/**
 * @brief   Switches to the first thread on the runnable queue.
 * @details The current thread is positioned in the ready list behind all
 *          threads having the same priority. The thread regains its time
 *          quantum.
 * @note    Not a user function, it is meant to be invoked by the scheduler
 *          itself or from within the port layer.
 *
 * @special
 */
void chSchDoRescheduleBehind(void) {
  thread_t *otp;

  otp = currp;
  /* Picks the first thread from the ready queue and makes it current.*/
  setcurrp(queue_fifo_remove(&ch.rlist.r_queue));
#if defined(CH_CFG_IDLE_LEAVE_HOOK)
  if (otp->p_prio == IDLEPRIO) {
    CH_CFG_IDLE_LEAVE_HOOK();
  }
#endif
  currp->p_state = CH_STATE_CURRENT;
#if CH_CFG_TIME_QUANTUM > 0
  otp->p_preempt = (tslices_t)CH_CFG_TIME_QUANTUM;
#endif
  (void) chSchReadyI(otp);
  chSysSwitch(currp, otp);
}

/**
 * @brief   Switches to the first thread on the runnable queue.
 * @details The current thread is positioned in the ready list ahead of all
 *          threads having the same priority.
 * @note    Not a user function, it is meant to be invoked by the scheduler
 *          itself or from within the port layer.
 *
 * @special
 */
void chSchDoRescheduleAhead(void) {
  thread_t *otp, *cp;

  otp = currp;
  /* Picks the first thread from the ready queue and makes it current.*/
  setcurrp(queue_fifo_remove(&ch.rlist.r_queue));
#if defined(CH_CFG_IDLE_LEAVE_HOOK)
  if (otp->p_prio == IDLEPRIO) {
    CH_CFG_IDLE_LEAVE_HOOK();
  }
#endif
  currp->p_state = CH_STATE_CURRENT;

  otp->p_state = CH_STATE_READY;
  cp = (thread_t *)&ch.rlist.r_queue;
  do {
    cp = cp->p_next;
  } while (cp->p_prio > otp->p_prio);
  /* Insertion on p_prev.*/
  otp->p_next = cp;
  otp->p_prev = cp->p_prev;
  otp->p_prev->p_next = otp;
  cp->p_prev = otp;

  chSysSwitch(currp, otp);
}

/**
 * @brief   Switches to the first thread on the runnable queue.
 * @details The current thread is positioned in the ready list behind or
 *          ahead of all threads having the same priority depending on
 *          if it used its whole time slice.
 * @note    Not a user function, it is meant to be invoked by the scheduler
 *          itself or from within the port layer.
 *
 * @special
 */
void chSchDoReschedule(void) {

#if CH_CFG_TIME_QUANTUM > 0
  /* If CH_CFG_TIME_QUANTUM is enabled then there are two different scenarios
     to handle on preemption: time quantum elapsed or not.*/
  if (currp->p_preempt == (tslices_t)0) {
    /* The thread consumed its time quantum so it is enqueued behind threads
       with same priority level, however, it acquires a new time quantum.*/
    chSchDoRescheduleBehind();
  }
  else {
    /* The thread didn't consume all its time quantum so it is put ahead of
       threads with equal priority and does not acquire a new time quantum.*/
    chSchDoRescheduleAhead();
  }
#else /* !(CH_CFG_TIME_QUANTUM > 0) */
  /* If the round-robin mechanism is disabled then the thread goes always
     ahead of its peers.*/
  chSchDoRescheduleAhead();
#endif /* !(CH_CFG_TIME_QUANTUM > 0) */
}

/** @} */
