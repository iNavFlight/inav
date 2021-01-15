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
 * @file    chschd.c
 * @brief   Scheduler code.
 *
 * @addtogroup scheduler
 * @details This module provides the default portable scheduler code.
 * @{
 */

#include "ch.h"
#include "platform.h"

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

  queue_init(&ch.rlist.queue);
  ch.rlist.prio = NOPRIO;
#if CH_CFG_USE_REGISTRY == TRUE
  ch.rlist.newer = (thread_t *)&ch.rlist;
  ch.rlist.older = (thread_t *)&ch.rlist;
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
    cp = cp->queue.next;
  } while ((cp != (thread_t *)tqp) && (cp->prio >= tp->prio));
  tp->queue.next             = cp;
  tp->queue.prev             = cp->queue.prev;
  tp->queue.prev->queue.next = tp;
  cp->queue.prev             = tp;
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

  tp->queue.next             = (thread_t *)tqp;
  tp->queue.prev             = tqp->prev;
  tp->queue.prev->queue.next = tp;
  tqp->prev                  = tp;
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
  thread_t *tp = tqp->next;

  tqp->next             = tp->queue.next;
  tqp->next->queue.prev = (thread_t *)tqp;

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
  thread_t *tp = tqp->prev;

  tqp->prev             = tp->queue.prev;
  tqp->prev->queue.next = (thread_t *)tqp;

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

  tp->queue.prev->queue.next = tp->queue.next;
  tp->queue.next->queue.prev = tp->queue.prev;

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

  tp->queue.next = tlp->next;
  tlp->next      = tp;
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

  thread_t *tp = tlp->next;
  tlp->next = tp->queue.next;

  return tp;
}
#endif /* CH_CFG_OPTIMIZE_SPEED */

/**
 * @brief   Inserts a thread in the Ready List placing it behind its peers.
 * @details The thread is positioned behind all threads with higher or equal
 *          priority.
 * @pre     The thread must not be already inserted in any list through its
 *          @p next and @p prev or list corruption would occur.
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
FAST_CODE thread_t *chSchReadyI(thread_t *tp) {
  thread_t *cp;

  chDbgCheckClassI();
  chDbgCheck(tp != NULL);
  chDbgAssert((tp->state != CH_STATE_READY) &&
              (tp->state != CH_STATE_FINAL),
              "invalid state");

  tp->state = CH_STATE_READY;
  cp = (thread_t *)&ch.rlist.queue;
  do {
    cp = cp->queue.next;
  } while (cp->prio >= tp->prio);
  /* Insertion on prev.*/
  tp->queue.next             = cp;
  tp->queue.prev             = cp->queue.prev;
  tp->queue.prev->queue.next = tp;
  cp->queue.prev             = tp;

  return tp;
}

/**
 * @brief   Inserts a thread in the Ready List placing it ahead its peers.
 * @details The thread is positioned ahead all threads with higher or equal
 *          priority.
 * @pre     The thread must not be already inserted in any list through its
 *          @p next and @p prev or list corruption would occur.
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
FAST_CODE thread_t *chSchReadyAheadI(thread_t *tp) {
  thread_t *cp;

  chDbgCheckClassI();
  chDbgCheck(tp != NULL);
  chDbgAssert((tp->state != CH_STATE_READY) &&
              (tp->state != CH_STATE_FINAL),
              "invalid state");

  tp->state = CH_STATE_READY;
  cp = (thread_t *)&ch.rlist.queue;
  do {
    cp = cp->queue.next;
  } while (cp->prio > tp->prio);
  /* Insertion on prev.*/
  tp->queue.next             = cp;
  tp->queue.prev             = cp->queue.prev;
  tp->queue.prev->queue.next = tp;
  cp->queue.prev             = tp;

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
FAST_CODE void chSchGoSleepS(tstate_t newstate) {
  thread_t *otp = currp;

  chDbgCheckClassS();

  /* New state.*/
  otp->state = newstate;

#if CH_CFG_TIME_QUANTUM > 0
  /* The thread is renouncing its remaining time slices so it will have a new
     time quantum when it will wakeup.*/
  otp->ticks = (tslices_t)CH_CFG_TIME_QUANTUM;
#endif

  /* Next thread in ready list becomes current.*/
  currp = queue_fifo_remove(&ch.rlist.queue);
  currp->state = CH_STATE_CURRENT;

  /* Handling idle-enter hook.*/
  if (currp->prio == IDLEPRIO) {
    CH_CFG_IDLE_ENTER_HOOK();
  }

  /* Swap operation as tail call.*/
  chSysSwitch(currp, otp);
}

/*
 * Timeout wakeup callback.
 */
FAST_CODE static void wakeup(void *p) {
  thread_t *tp = (thread_t *)p;

  chSysLockFromISR();
  switch (tp->state) {
  case CH_STATE_READY:
    /* Handling the special case where the thread has been made ready by
       another thread with higher priority.*/
    chSysUnlockFromISR();
    return;
  case CH_STATE_SUSPENDED:
    *tp->u.wttrp = NULL;
    break;
#if CH_CFG_USE_SEMAPHORES == TRUE
  case CH_STATE_WTSEM:
    chSemFastSignalI(tp->u.wtsemp);
#endif
    /* Falls through.*/
    __attribute__ ((fallthrough));
  case CH_STATE_QUEUED:
    /* Falls through.*/
    __attribute__ ((fallthrough));
#if (CH_CFG_USE_CONDVARS == TRUE) && (CH_CFG_USE_CONDVARS_TIMEOUT == TRUE)
  case CH_STATE_WTCOND:
#endif
    /* States requiring dequeuing.*/
    (void) queue_dequeue(tp);
    break;
  default:
    /* Any other state, nothing to do.*/
    break;
  }
  tp->u.rdymsg = MSG_TIMEOUT;
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
 * @param[in] timeout   the number of ticks before the operation timeouts, the
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
FAST_CODE msg_t chSchGoSleepTimeoutS(tstate_t newstate, sysinterval_t timeout) {

  chDbgCheckClassS();

  if (TIME_INFINITE != timeout) {
    virtual_timer_t vt;

    chVTDoSetI(&vt, timeout, wakeup, currp);
    chSchGoSleepS(newstate);
    if (chVTIsArmedI(&vt)) {
      chVTDoResetI(&vt);
    }
  }
  else {
    chSchGoSleepS(newstate);
  }

  return currp->u.rdymsg;
}

/**
 * @brief   Wakes up a thread.
 * @details The thread is inserted into the ready list or immediately made
 *          running depending on its relative priority compared to the current
 *          thread.
 * @pre     The thread must not be already inserted in any list through its
 *          @p next and @p prev or list corruption would occur.
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
FAST_CODE void chSchWakeupS(thread_t *ntp, msg_t msg) {
  thread_t *otp = currp;

  chDbgCheckClassS();

  chDbgAssert((ch.rlist.queue.next == (thread_t *)&ch.rlist.queue) ||
              (ch.rlist.current->prio >= ch.rlist.queue.next->prio),
              "priority order violation");

  /* Storing the message to be retrieved by the target thread when it will
     restart execution.*/
  ntp->u.rdymsg = msg;

  /* If the waken thread has a not-greater priority than the current
     one then it is just inserted in the ready list else it made
     running immediately and the invoking thread goes in the ready
     list instead.*/
  if (ntp->prio <= otp->prio) {
    (void) chSchReadyI(ntp);
  }
  else {
    otp = chSchReadyAheadI(otp);

    /* Handling idle-leave hook.*/
    if (otp->prio == IDLEPRIO) {
      CH_CFG_IDLE_LEAVE_HOOK();
    }

    /* The extracted thread is marked as current.*/
    currp = ntp;
    ntp->state = CH_STATE_CURRENT;

    /* Swap operation as tail call.*/
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
FAST_CODE void chSchRescheduleS(void) {

  chDbgCheckClassS();

  if (chSchIsRescRequiredI()) {
    chSchDoRescheduleAhead();
  }
}

#if !defined(CH_SCH_IS_PREEMPTION_REQUIRED_HOOKED)
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
FAST_CODE bool chSchIsPreemptionRequired(void) {
  tprio_t p1 = firstprio(&ch.rlist.queue);
  tprio_t p2 = currp->prio;

#if CH_CFG_TIME_QUANTUM > 0
  /* If the running thread has not reached its time quantum, reschedule only
     if the first thread on the ready queue has a higher priority.
     Otherwise, if the running thread has used up its time quantum, reschedule
     if the first thread on the ready queue has equal or higher priority.*/
  return (currp->ticks > (tslices_t)0) ? (p1 > p2) : (p1 >= p2);
#else
  /* If the round robin preemption feature is not enabled then performs a
     simpler comparison.*/
  return p1 > p2;
#endif
}
#endif /* !defined(CH_SCH_IS_PREEMPTION_REQUIRED_HOOKED) */

/**
 * @brief   Switches to the first thread on the runnable queue.
 * @details The current thread is positioned in the ready list behind all
 *          threads having the same priority. The thread regains its time
 *          quantum.
 * @note    Not a user function, it is meant to be invoked by the scheduler
 *          itself.
 *
 * @special
 */
FAST_CODE void chSchDoRescheduleBehind(void) {
  thread_t *otp = currp;

  /* Picks the first thread from the ready queue and makes it current.*/
  currp = queue_fifo_remove(&ch.rlist.queue);
  currp->state = CH_STATE_CURRENT;

  /* Handling idle-leave hook.*/
  if (otp->prio == IDLEPRIO) {
    CH_CFG_IDLE_LEAVE_HOOK();
  }

#if CH_CFG_TIME_QUANTUM > 0
  /* It went behind peers so it gets a new time quantum.*/
  otp->ticks = (tslices_t)CH_CFG_TIME_QUANTUM;
#endif

  /* Placing in ready list behind peers.*/
  otp = chSchReadyI(otp);

  /* Swap operation as tail call.*/
  chSysSwitch(currp, otp);
}

/**
 * @brief   Switches to the first thread on the runnable queue.
 * @details The current thread is positioned in the ready list ahead of all
 *          threads having the same priority.
 * @note    Not a user function, it is meant to be invoked by the scheduler
 *          itself.
 *
 * @special
 */
FAST_CODE void chSchDoRescheduleAhead(void) {
  thread_t *otp = currp;

  /* Picks the first thread from the ready queue and makes it current.*/
  currp = queue_fifo_remove(&ch.rlist.queue);
  currp->state = CH_STATE_CURRENT;

  /* Handling idle-leave hook.*/
  if (otp->prio == IDLEPRIO) {
    CH_CFG_IDLE_LEAVE_HOOK();
  }

  /* Placing in ready list ahead of peers.*/
  otp = chSchReadyAheadI(otp);

  /* Swap operation as tail call.*/
  chSysSwitch(currp, otp);
}

#if !defined(CH_SCH_DO_RESCHEDULE_HOOKED)
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
FAST_CODE void chSchDoReschedule(void) {
  thread_t *otp = currp;

  /* Picks the first thread from the ready queue and makes it current.*/
  currp = queue_fifo_remove(&ch.rlist.queue);
  currp->state = CH_STATE_CURRENT;

  /* Handling idle-leave hook.*/
  if (otp->prio == IDLEPRIO) {
    CH_CFG_IDLE_LEAVE_HOOK();
  }

#if CH_CFG_TIME_QUANTUM > 0
  /* If CH_CFG_TIME_QUANTUM is enabled then there are two different scenarios
     to handle on preemption: time quantum elapsed or not.*/
  if (currp->ticks == (tslices_t)0) {

    /* The thread consumed its time quantum so it is enqueued behind threads
       with same priority level, however, it acquires a new time quantum.*/
    otp = chSchReadyI(otp);

    /* The thread being swapped out receives a new time quantum.*/
    otp->ticks = (tslices_t)CH_CFG_TIME_QUANTUM;
  }
  else {
    /* The thread didn't consume all its time quantum so it is put ahead of
       threads with equal priority and does not acquire a new time quantum.*/
    otp = chSchReadyAheadI(otp);
  }
#else /* !(CH_CFG_TIME_QUANTUM > 0) */
  /* If the round-robin mechanism is disabled then the thread goes always
     ahead of its peers.*/
  otp = chSchReadyAheadI(otp);
#endif /* !(CH_CFG_TIME_QUANTUM > 0) */

  /* Swap operation as tail call.*/
  chSysSwitch(currp, otp);
}
#endif /* !defined(CH_SCH_DO_RESCHEDULE_HOOKED) */

/** @} */
