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
 * @file    nil.c
 * @brief   Nil RTOS main source file.
 *
 * @addtogroup NIL_KERNEL
 * @{
 */

#include "nil.h"

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   System data structures.
 */
nil_system_t nil;

/*===========================================================================*/
/* Module local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Module interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Initializes the kernel.
 * @details Initializes the kernel structures, the current instructions flow
 *          becomes the idle thread upon return. The idle thread must not
 *          invoke any kernel primitive able to change state to not runnable.
 * @note    This function assumes that the @p nil global variable has been
 *          zeroed by the runtime environment. If this is not the case then
 *          make sure to clear it before calling this function.
 *
 * @special
 */
void chSysInit(void) {
  thread_t *tp;
  const thread_config_t *tcp;

  /* Port layer initialization.*/
  port_init();

  /* System initialization hook.*/
  NIL_CFG_SYSTEM_INIT_HOOK();

  /* Iterates through the list of defined threads.*/
  tp = &nil.threads[0];
  tcp = nil_thd_configs;
  while (tp < &nil.threads[NIL_CFG_NUM_THREADS]) {
#if NIL_CFG_ENABLE_STACK_CHECK
    tp->stklim  = (stkalign_t *)tcp->wbase;
#endif

    /* Port dependent thread initialization.*/
    PORT_SETUP_CONTEXT(tp, tcp->wend, tcp->funcp, tcp->arg);

    /* Initialization hook.*/
    NIL_CFG_THREAD_EXT_INIT_HOOK(tp);

    tp++;
    tcp++;
  }

#if NIL_CFG_ENABLE_STACK_CHECK
  /* The idle thread is a special case because its stack is set up by the
     runtime environment.*/
  tp->stklim  = THD_IDLE_BASE;
#endif

  /* Runs the highest priority thread, the current one becomes the idle
     thread.*/
  nil.current = nil.next = nil.threads;
  port_switch(nil.current, tp);

  /* Interrupts enabled for the idle thread.*/
  chSysEnable();
}

/**
 * @brief   Halts the system.
 * @details This function is invoked by the operating system when an
 *          unrecoverable error is detected, for example because a programming
 *          error in the application code that triggers an assertion while
 *          in debug mode.
 * @note    Can be invoked from any system state.
 *
 * @param[in] reason        pointer to an error string
 *
 * @special
 */
void chSysHalt(const char *reason) {

  port_disable();

#if NIL_DBG_ENABLED
  nil.dbg_panic_msg = reason;
#else
  (void)reason;
#endif

  NIL_CFG_SYSTEM_HALT_HOOK(reason);

  /* Harmless infinite loop.*/
  while (true) {
  }
}

/**
 * @brief   Time management handler.
 * @note    This handler has to be invoked by a periodic ISR in order to
 *          reschedule the waiting threads.
 *
 * @iclass
 */
void chSysTimerHandlerI(void) {

#if NIL_CFG_ST_TIMEDELTA == 0
  thread_t *tp = &nil.threads[0];
  nil.systime++;
  do {
    /* Is the thread in a wait state with timeout?.*/
    if (tp->timeout > (systime_t)0) {

      chDbgAssert(!NIL_THD_IS_READY(tp), "is ready");

     /* Did the timer reach zero?*/
      if (--tp->timeout == (systime_t)0) {
        /* Timeout on semaphores requires a special handling because the
           semaphore counter must be incremented.*/
        /*lint -save -e9013 [15.7] There is no else because it is not needed.*/
        if (NIL_THD_IS_WTSEM(tp)) {
          tp->u1.semp->cnt++;
        }
        else if (NIL_THD_IS_SUSP(tp)) {
          *tp->u1.trp = NULL;
        }
        /*lint -restore*/
        (void) chSchReadyI(tp, MSG_TIMEOUT);
      }
    }
    /* Lock released in order to give a preemption chance on those
       architectures supporting IRQ preemption.*/
    chSysUnlockFromISR();
    tp++;
    chSysLockFromISR();
  } while (tp < &nil.threads[NIL_CFG_NUM_THREADS]);
#else
  thread_t *tp = &nil.threads[0];
  systime_t next = (systime_t)0;

  chDbgAssert(nil.nexttime == port_timer_get_alarm(), "time mismatch");

  do {
    /* Is the thread in a wait state with timeout?.*/
    if (tp->timeout > (systime_t)0) {

      chDbgAssert(!NIL_THD_IS_READY(tp), "is ready");
      chDbgAssert(tp->timeout >= (nil.nexttime - nil.lasttime), "skipped one");

      tp->timeout -= nil.nexttime - nil.lasttime;
      if (tp->timeout == (systime_t)0) {
        /* Timeout on semaphores requires a special handling because the
           semaphore counter must be incremented.*/
        /*lint -save -e9013 [15.7] There is no else because it is not needed.*/
       if (NIL_THD_IS_WTSEM(tp)) {
          tp->u1.semp->cnt++;
        }
        else if (NIL_THD_IS_SUSP(tp)) {
          *tp->u1.trp = NULL;
        }
        /*lint -restore*/
        (void) chSchReadyI(tp, MSG_TIMEOUT);
      }
      else {
        if (tp->timeout <= (systime_t)(next - (systime_t)1)) {
          next = tp->timeout;
        }
      }
    }
    /* Lock released in order to give a preemption chance on those
       architectures supporting IRQ preemption.*/
    chSysUnlockFromISR();
    tp++;
    chSysLockFromISR();
  } while (tp < &nil.threads[NIL_CFG_NUM_THREADS]);
  nil.lasttime = nil.nexttime;
  if (next > (systime_t)0) {
    nil.nexttime += next;
    port_timer_set_alarm(nil.nexttime);
  }
  else {
    /* No tick event needed.*/
    port_timer_stop_alarm();
  }
#endif
}

/**
 * @brief   Unconditionally enters the kernel lock state.
 * @note    Can be called without previous knowledge of the current lock state.
 *          The final state is "s-locked".
 *
 * @special
 */
void chSysUnconditionalLock(void) {

  if (port_irq_enabled(port_get_irq_status())) {
    chSysLock();
  }
}

/**
 * @brief   Unconditionally leaves the kernel lock state.
 * @note    Can be called without previous knowledge of the current lock state.
 *          The final state is "normal".
 *
 * @special
 */
void chSysUnconditionalUnlock(void) {

  if (!port_irq_enabled(port_get_irq_status())) {
    chSysUnlock();
  }
}

/**
 * @brief   Returns the execution status and enters a critical zone.
 * @details This functions enters into a critical zone and can be called
 *          from any context. Because its flexibility it is less efficient
 *          than @p chSysLock() which is preferable when the calling context
 *          is known.
 * @post    The system is in a critical zone.
 *
 * @return              The previous system status, the encoding of this
 *                      status word is architecture-dependent and opaque.
 *
 * @xclass
 */
syssts_t chSysGetStatusAndLockX(void) {

  syssts_t sts = port_get_irq_status();
  if (port_irq_enabled(sts)) {
    if (port_is_isr_context()) {
      chSysLockFromISR();
    }
    else {
      chSysLock();
    }
  }
  return sts;
}

/**
 * @brief   Restores the specified execution status and leaves a critical zone.
 * @note    A call to @p chSchRescheduleS() is automatically performed
 *          if exiting the critical zone and if not in ISR context.
 *
 * @param[in] sts       the system status to be restored.
 *
 * @xclass
 */
void chSysRestoreStatusX(syssts_t sts) {

  if (port_irq_enabled(sts)) {
    if (port_is_isr_context()) {
      chSysUnlockFromISR();
    }
    else {
      chSchRescheduleS();
      chSysUnlock();
    }
  }
}

#if (PORT_SUPPORTS_RT == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   Realtime window test.
 * @details This function verifies if the current realtime counter value
 *          lies within the specified range or not. The test takes care
 *          of the realtime counter wrapping to zero on overflow.
 * @note    When start==end then the function returns always true because the
 *          whole time range is specified.
 * @note    This function is only available if the port layer supports the
 *          option @p PORT_SUPPORTS_RT.
 *
 * @param[in] cnt       the counter value to be tested
 * @param[in] start     the start of the time window (inclusive)
 * @param[in] end       the end of the time window (non inclusive)
 * @retval true         current time within the specified time window.
 * @retval false        current time not within the specified time window.
 *
 * @xclass
 */
bool chSysIsCounterWithinX(rtcnt_t cnt, rtcnt_t start, rtcnt_t end) {

  return (bool)((cnt - start) < (end - start));
}

/**
 * @brief   Polled delay.
 * @note    The real delay is always few cycles in excess of the specified
 *          value.
 * @note    This function is only available if the port layer supports the
 *          option @p PORT_SUPPORTS_RT.
 *
 * @param[in] cycles    number of cycles
 *
 * @xclass
 */
void chSysPolledDelayX(rtcnt_t cycles) {
  rtcnt_t start = chSysGetRealtimeCounterX();
  rtcnt_t end  = start + cycles;

  while (chSysIsCounterWithinX(chSysGetRealtimeCounterX(), start, end)) {
  }
}
#endif /* PORT_SUPPORTS_RT == TRUE */

/**
 * @brief   Makes the specified thread ready for execution.
 *
 * @param[in] tp        pointer to the @p thread_t object
 * @param[in] msg       the wakeup message
 *
 * @return              The same reference passed as parameter.
 */
thread_t *chSchReadyI(thread_t *tp, msg_t msg) {

  chDbgAssert((tp >= nil.threads) &&
              (tp < &nil.threads[NIL_CFG_NUM_THREADS]),
              "pointer out of range");
  chDbgAssert(!NIL_THD_IS_READY(tp), "already ready");
  chDbgAssert(nil.next <= nil.current, "priority ordering");

  tp->u1.msg = msg;
  tp->state = NIL_STATE_READY;
  tp->timeout = (systime_t)0;
  if (tp < nil.next) {
    nil.next = tp;
  }
  return tp;
}

/**
 * @brief   Reschedules if needed.
 *
 * @sclass
 */
void chSchRescheduleS(void) {

  if (chSchIsRescRequiredI()) {
    thread_t *otp = nil.current;

    nil.current = nil.next;
    if (otp == &nil.threads[NIL_CFG_NUM_THREADS]) {
      NIL_CFG_IDLE_LEAVE_HOOK();
    }
    port_switch(nil.next, otp);
  }
}

/**
 * @brief   Puts the current thread to sleep into the specified state with
 *          timeout specification.
 * @details The thread goes into a sleeping state, if it is not awakened
 *          explicitly within the specified system time then it is forcibly
 *          awakened with a @p NIL_MSG_TMO low level message.
 *
 * @param[in] newstate  the new thread state or a semaphore pointer
 * @param[in] timeout   the number of ticks before the operation timeouts.
 *                      the following special values are allowed:
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              The wakeup message.
 * @retval NIL_MSG_TMO  if a timeout occurred.
 *
 * @sclass
 */
msg_t chSchGoSleepTimeoutS(tstate_t newstate, systime_t timeout) {
  thread_t *ntp, *otp = nil.current;

  chDbgAssert(otp != &nil.threads[NIL_CFG_NUM_THREADS],
               "idle cannot sleep");

  /* Storing the wait object for the current thread.*/
  otp->state = newstate;

#if NIL_CFG_ST_TIMEDELTA > 0
  if (timeout != TIME_INFINITE) {
    systime_t abstime;

    /* TIMEDELTA makes sure to have enough time to reprogram the timer
       before the free-running timer counter reaches the selected timeout.*/
    if (timeout < (systime_t)NIL_CFG_ST_TIMEDELTA) {
      timeout = (systime_t)NIL_CFG_ST_TIMEDELTA;
    }

    /* Absolute time of the timeout event.*/
    abstime = chVTGetSystemTimeX() + timeout;

    if (nil.lasttime == nil.nexttime) {
      /* Special case, first thread asking for a timeout.*/
      port_timer_start_alarm(abstime);
      nil.nexttime = abstime;
    }
    else {
      /* Special case, there are already other threads with a timeout
         activated, evaluating the order.*/
      if (chVTIsTimeWithinX(abstime, nil.lasttime, nil.nexttime)) {
        port_timer_set_alarm(abstime);
        nil.nexttime = abstime;
      }
    }

    /* Timeout settings.*/
    otp->timeout = abstime - nil.lasttime;
  }
#else

  /* Timeout settings.*/
  otp->timeout = timeout;
#endif

  /* Scanning the whole threads array.*/
  ntp = nil.threads;
  while (true) {
    /* Is this thread ready to execute?*/
    if (NIL_THD_IS_READY(ntp)) {
      nil.current = nil.next = ntp;
      if (ntp == &nil.threads[NIL_CFG_NUM_THREADS]) {
        NIL_CFG_IDLE_ENTER_HOOK();
      }
      port_switch(ntp, otp);
      return nil.current->u1.msg;
    }

    /* Points to the next thread in lowering priority order.*/
    ntp++;
    chDbgAssert(ntp <= &nil.threads[NIL_CFG_NUM_THREADS],
                "pointer out of range");
  }
}

/**
 * @brief   Sends the current thread sleeping and sets a reference variable.
 * @note    This function must reschedule, it can only be called from thread
 *          context.
 *
 * @param[in] trp       a pointer to a thread reference object
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              The wake up message.
 *
 * @sclass
 */
msg_t chThdSuspendTimeoutS(thread_reference_t *trp, systime_t timeout) {

  chDbgAssert(*trp == NULL, "not NULL");

  *trp = nil.current;
  nil.current->u1.trp = trp;
  return chSchGoSleepTimeoutS(NIL_STATE_SUSP, timeout);
}

/**
 * @brief   Wakes up a thread waiting on a thread reference object.
 * @note    This function must not reschedule because it can be called from
 *          ISR context.
 *
 * @param[in] trp       a pointer to a thread reference object
 * @param[in] msg       the message code
 *
 * @iclass
 */
void chThdResumeI(thread_reference_t *trp, msg_t msg) {

  if (*trp != NULL) {
    thread_reference_t tr = *trp;

    chDbgAssert(NIL_THD_IS_SUSP(tr), "not suspended");

    *trp = NULL;
    (void) chSchReadyI(tr, msg);
  }
}

/**
 * @brief   Suspends the invoking thread for the specified time.
 *
 * @param[in] timeout   the delay in system ticks
 *
 * @api
 */
void chThdSleep(systime_t timeout) {

  chSysLock();
  chThdSleepS(timeout);
  chSysUnlock();
}

/**
 * @brief   Suspends the invoking thread until the system time arrives to the
 *          specified value.
 *
 * @param[in] abstime   absolute system time
 *
 * @api
 */
void chThdSleepUntil(systime_t abstime) {

  chSysLock();
  chThdSleepUntilS(abstime);
  chSysUnlock();
}

/**
 * @brief   Performs a wait operation on a semaphore with timeout specification.
 *
 * @param[in] sp        pointer to a @p semaphore_t structure
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_IMMEDIATE immediate timeout.
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              A message specifying how the invoking thread has been
 *                      released from the semaphore.
 * @retval NIL_MSG_OK   if the thread has not stopped on the semaphore or the
 *                      semaphore has been signaled.
 * @retval NIL_MSG_RST  if the semaphore has been reset using @p chSemReset().
 * @retval NIL_MSG_TMO  if the semaphore has not been signaled or reset within
 *                      the specified timeout.
 *
 * @api
 */
msg_t chSemWaitTimeout(semaphore_t *sp, systime_t timeout) {
  msg_t msg;

  chSysLock();
  msg = chSemWaitTimeoutS(sp, timeout);
  chSysUnlock();

  return msg;
}

/**
 * @brief   Performs a wait operation on a semaphore with timeout specification.
 *
 * @param[in] sp        pointer to a @p semaphore_t structure
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_IMMEDIATE immediate timeout.
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              A message specifying how the invoking thread has been
 *                      released from the semaphore.
 * @retval NIL_MSG_OK   if the thread has not stopped on the semaphore or the
 *                      semaphore has been signaled.
 * @retval NIL_MSG_RST  if the semaphore has been reset using @p chSemReset().
 * @retval NIL_MSG_TMO  if the semaphore has not been signaled or reset within
 *                      the specified timeout.
 *
 * @sclass
 */
msg_t chSemWaitTimeoutS(semaphore_t *sp, systime_t timeout) {

  /* Note, the semaphore counter is a volatile variable so accesses are
     manually optimized.*/
  cnt_t cnt = sp->cnt;
  if (cnt <= (cnt_t)0) {
    if (TIME_IMMEDIATE == timeout) {
      return MSG_TIMEOUT;
    }
    sp->cnt = cnt - (cnt_t)1;
    nil.current->u1.semp = sp;
    return chSchGoSleepTimeoutS(NIL_STATE_WTSEM, timeout);
  }
  sp->cnt = cnt - (cnt_t)1;
  return MSG_OK;
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
 * @api
 */
void chSemSignal(semaphore_t *sp) {

  chSysLock();
  chSemSignalI(sp);
  chSchRescheduleS();
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

  if (++sp->cnt <= (cnt_t)0) {
    thread_reference_t tr = nil.threads;
    while (true) {
      /* Is this thread waiting on this semaphore?*/
      if (tr->u1.semp == sp) {

        chDbgAssert(NIL_THD_IS_WTSEM(tr), "not waiting");

        (void) chSchReadyI(tr, MSG_OK);
        return;
      }
      tr++;

      chDbgAssert(tr < &nil.threads[NIL_CFG_NUM_THREADS],
                  "pointer out of range");
    }
  }
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
 *
 * @param[in] sp        pointer to a @p semaphore_t structure
 * @param[in] n         the new value of the semaphore counter. The value must
 *                      be non-negative.
 *
 * @iclass
 */
void chSemResetI(semaphore_t *sp, cnt_t n) {
  thread_t *tp;
  cnt_t cnt;

  cnt = sp->cnt;
  sp->cnt = n;
  tp = nil.threads;
  while (cnt < (cnt_t)0) {

    chDbgAssert(tp < &nil.threads[NIL_CFG_NUM_THREADS],
                "pointer out of range");

    /* Is this thread waiting on this semaphore?*/
    if (tp->u1.semp == sp) {

      chDbgAssert(NIL_THD_IS_WTSEM(tp), "not waiting");

      cnt++;
      (void) chSchReadyI(tp, MSG_RESET);
    }
    tp++;
  }
}

#if (NIL_CFG_USE_EVENTS == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   Adds a set of event flags directly to the specified @p thread_t.
 *
 * @param[in] tp        the thread to be signaled
 * @param[in] mask      the event flags set to be ORed
 *
 * @api
 */
void chEvtSignal(thread_t *tp, eventmask_t mask) {

  chSysLock();
  chEvtSignalI(tp, mask);
  chSchRescheduleS();
  chSysUnlock();
}

/**
 * @brief   Adds a set of event flags directly to the specified @p thread_t.
 * @post    This function does not reschedule so a call to a rescheduling
 *          function must be performed before unlocking the kernel. Note that
 *          interrupt handlers always reschedule on exit so an explicit
 *          reschedule must not be performed in ISRs.
 *
 * @param[in] tp        the thread to be signaled
 * @param[in] mask      the event flags set to be ORed
 *
 * @iclass
 */
void chEvtSignalI(thread_t *tp, eventmask_t mask) {

  tp->epmask |= mask;
  if (NIL_THD_IS_WTOREVT(tp) &&
      ((tp->epmask & tp->u1.ewmask) != (eventmask_t)0)) {
    (void) chSchReadyI(tp, MSG_OK);
  }
}

/**
 * @brief   Waits for any of the specified events.
 * @details The function waits for any event among those specified in
 *          @p mask to become pending then the events are cleared and
 *          returned.
 *
 * @param[in] mask      mask of the event flags that the function should wait
 *                      for, @p ALL_EVENTS enables all the events
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_IMMEDIATE immediate timeout.
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              The mask of the served and cleared events.
 * @retval 0            if the operation has timed out.
 *
 * @api
 */
eventmask_t chEvtWaitAnyTimeout(eventmask_t mask, systime_t timeout) {
  eventmask_t m;

  chSysLock();
  m = chEvtWaitAnyTimeoutS(mask, timeout);
  chSysUnlock();

  return m;
}

/**
 * @brief   Waits for any of the specified events.
 * @details The function waits for any event among those specified in
 *          @p mask to become pending then the events are cleared and
 *          returned.
 *
 * @param[in] mask      mask of the event flags that the function should wait
 *                      for, @p ALL_EVENTS enables all the events
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_IMMEDIATE immediate timeout.
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              The mask of the served and cleared events.
 * @retval 0            if the operation has timed out.
 *
 * @sclass
 */
eventmask_t chEvtWaitAnyTimeoutS(eventmask_t mask, systime_t timeout) {
  thread_t *ctp = nil.current;
  eventmask_t m;

  if ((m = (ctp->epmask & mask)) == (eventmask_t)0) {
    if (TIME_IMMEDIATE == timeout) {
      chSysUnlock();

      return (eventmask_t)0;
    }
    ctp->u1.ewmask = mask;
    if (chSchGoSleepTimeoutS(NIL_STATE_WTOREVT, timeout) < MSG_OK) {
      chSysUnlock();

      return (eventmask_t)0;
    }
    m = ctp->epmask & mask;
  }
  ctp->epmask &= ~m;

  return m;
}
#endif /* NIL_CFG_USE_EVENTS == TRUE */

/** @} */
