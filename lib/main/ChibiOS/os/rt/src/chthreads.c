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
 * @file    chthreads.c
 * @brief   Threads code.
 *
 * @addtogroup threads
 * @details Threads related APIs and services.
 *          <h2>Operation mode</h2>
 *          A thread is an abstraction of an independent instructions flow.
 *          In ChibiOS/RT a thread is represented by a "C" function owning
 *          a processor context, state informations and a dedicated stack
 *          area. In this scenario static variables are shared among all
 *          threads while automatic variables are local to the thread.<br>
 *          Operations defined for threads:
 *          - <b>Create</b>, a thread is started on the specified thread
 *            function. This operation is available in multiple variants,
 *            both static and dynamic.
 *          - <b>Exit</b>, a thread terminates by returning from its top
 *            level function or invoking a specific API, the thread can
 *            return a value that can be retrieved by other threads.
 *          - <b>Wait</b>, a thread waits for the termination of another
 *            thread and retrieves its return value.
 *          - <b>Resume</b>, a thread created in suspended state is started.
 *          - <b>Sleep</b>, the execution of a thread is suspended for the
 *            specified amount of time or the specified future absolute time
 *            is reached.
 *          - <b>SetPriority</b>, a thread changes its own priority level.
 *          - <b>Yield</b>, a thread voluntarily renounces to its time slot.
 *          .
 *          The threads subsystem is implicitly included in kernel however
 *          some of its part may be excluded by disabling them in @p chconf.h,
 *          see the @p CH_CFG_USE_WAITEXIT and @p CH_CFG_USE_DYNAMIC configuration
 *          options.
 * @{
 */

#include "ch.h"

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

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

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Initializes a thread structure.
 * @note    This is an internal functions, do not use it in application code.
 *
 * @param[in] tp        pointer to the thread
 * @param[in] prio      the priority level for the new thread
 * @return              The same thread pointer passed as parameter.
 *
 * @notapi
 */
thread_t *_thread_init(thread_t *tp, tprio_t prio) {

  tp->p_prio = prio;
  tp->p_state = CH_STATE_WTSTART;
  tp->p_flags = CH_FLAG_MODE_STATIC;
#if CH_CFG_TIME_QUANTUM > 0
  tp->p_preempt = (tslices_t)CH_CFG_TIME_QUANTUM;
#endif
#if CH_CFG_USE_MUTEXES == TRUE
  tp->p_realprio = prio;
  tp->p_mtxlist = NULL;
#endif
#if CH_CFG_USE_EVENTS == TRUE
  tp->p_epending = (eventmask_t)0;
#endif
#if CH_DBG_THREADS_PROFILING == TRUE
  tp->p_time = (systime_t)0;
#endif
#if CH_CFG_USE_DYNAMIC == TRUE
  tp->p_refs = (trefs_t)1;
#endif
#if CH_CFG_USE_REGISTRY == TRUE
  tp->p_name = NULL;
  REG_INSERT(tp);
#endif
#if CH_CFG_USE_WAITEXIT == TRUE
  list_init(&tp->p_waiting);
#endif
#if CH_CFG_USE_MESSAGES == TRUE
  queue_init(&tp->p_msgqueue);
#endif
#if CH_DBG_ENABLE_STACK_CHECK == TRUE
  tp->p_stklimit = (stkalign_t *)(tp + 1);
#endif
#if CH_DBG_STATISTICS == TRUE
  chTMObjectInit(&tp->p_stats);
#endif
#if defined(CH_CFG_THREAD_INIT_HOOK)
  CH_CFG_THREAD_INIT_HOOK(tp);
#endif
  return tp;
}

#if (CH_DBG_FILL_THREADS == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   Memory fill utility.
 *
 * @param[in] startp    first address to fill
 * @param[in] endp      last address to fill +1
 * @param[in] v         filler value
 *
 * @notapi
 */
void _thread_memfill(uint8_t *startp, uint8_t *endp, uint8_t v) {

  while (startp < endp) {
    *startp++ = v;
  }
}
#endif /* CH_DBG_FILL_THREADS */

/**
 * @brief   Creates a new thread into a static memory area.
 * @details The new thread is initialized but not inserted in the ready list,
 *          the initial state is @p CH_STATE_WTSTART.
 * @post    The initialized thread can be subsequently started by invoking
 *          @p chThdStart(), @p chThdStartI() or @p chSchWakeupS()
 *          depending on the execution context.
 * @note    A thread can terminate by calling @p chThdExit() or by simply
 *          returning from its main function.
 * @note    Threads created using this function do not obey to the
 *          @p CH_DBG_FILL_THREADS debug option because it would keep
 *          the kernel locked for too much time.
 *
 * @param[out] wsp      pointer to a working area dedicated to the thread stack
 * @param[in] size      size of the working area
 * @param[in] prio      the priority level for the new thread
 * @param[in] pf        the thread function
 * @param[in] arg       an argument passed to the thread function. It can be
 *                      @p NULL.
 * @return              The pointer to the @p thread_t structure allocated for
 *                      the thread into the working space area.
 *
 * @iclass
 */
thread_t *chThdCreateI(void *wsp, size_t size,
                       tprio_t prio, tfunc_t pf, void *arg) {
  /* The thread structure is laid out in the lower part of the thread
     workspace.*/
  thread_t *tp = wsp;

  chDbgCheckClassI();
  chDbgCheck((wsp != NULL) && (size >= THD_WORKING_AREA_SIZE(0)) &&
             (prio <= HIGHPRIO) && (pf != NULL));

  PORT_SETUP_CONTEXT(tp, wsp, size, pf, arg);

  return _thread_init(tp, prio);
}

/**
 * @brief   Creates a new thread into a static memory area.
 * @note    A thread can terminate by calling @p chThdExit() or by simply
 *          returning from its main function.
 *
 * @param[out] wsp      pointer to a working area dedicated to the thread stack
 * @param[in] size      size of the working area
 * @param[in] prio      the priority level for the new thread
 * @param[in] pf        the thread function
 * @param[in] arg       an argument passed to the thread function. It can be
 *                      @p NULL.
 * @return              The pointer to the @p thread_t structure allocated for
 *                      the thread into the working space area.
 *
 * @api
 */
thread_t *chThdCreateStatic(void *wsp, size_t size,
                            tprio_t prio, tfunc_t pf, void *arg) {
  thread_t *tp;
  
#if CH_DBG_FILL_THREADS == TRUE
  _thread_memfill((uint8_t *)wsp,
                  (uint8_t *)wsp + sizeof(thread_t),
                  CH_DBG_THREAD_FILL_VALUE);
  _thread_memfill((uint8_t *)wsp + sizeof(thread_t),
                  (uint8_t *)wsp + size,
                  CH_DBG_STACK_FILL_VALUE);
#endif

  chSysLock();
  tp = chThdCreateI(wsp, size, prio, pf, arg);
  chSchWakeupS(tp, MSG_OK);
  chSysUnlock();

  return tp;
}

/**
 * @brief   Resumes a thread created with @p chThdCreateI().
 *
 * @param[in] tp        pointer to the thread
 * @return              The pointer to the @p thread_t structure allocated for
 *                      the thread into the working space area.
 *
 * @api
 */
thread_t *chThdStart(thread_t *tp) {

  chSysLock();
  tp = chThdStartI(tp);
  chSysUnlock();

  return tp;
}

/**
 * @brief   Changes the running thread priority level then reschedules if
 *          necessary.
 * @note    The function returns the real thread priority regardless of the
 *          current priority that could be higher than the real priority
 *          because the priority inheritance mechanism.
 *
 * @param[in] newprio   the new priority level of the running thread
 * @return              The old priority level.
 *
 * @api
 */
tprio_t chThdSetPriority(tprio_t newprio) {
  tprio_t oldprio;

  chDbgCheck(newprio <= HIGHPRIO);

  chSysLock();
#if CH_CFG_USE_MUTEXES == TRUE
  oldprio = currp->p_realprio;
  if ((currp->p_prio == currp->p_realprio) || (newprio > currp->p_prio)) {
    currp->p_prio = newprio;
  }
  currp->p_realprio = newprio;
#else
  oldprio = currp->p_prio;
  currp->p_prio = newprio;
#endif
  chSchRescheduleS();
  chSysUnlock();

  return oldprio;
}

/**
 * @brief   Requests a thread termination.
 * @pre     The target thread must be written to invoke periodically
 *          @p chThdShouldTerminate() and terminate cleanly if it returns
 *          @p true.
 * @post    The specified thread will terminate after detecting the termination
 *          condition.
 *
 * @param[in] tp        pointer to the thread
 *
 * @api
 */
void chThdTerminate(thread_t *tp) {

  chSysLock();
  tp->p_flags |= CH_FLAG_TERMINATE;
  chSysUnlock();
}

/**
 * @brief   Suspends the invoking thread for the specified time.
 *
 * @param[in] time      the delay in system ticks, the special values are
 *                      handled as follow:
 *                      - @a TIME_INFINITE the thread enters an infinite sleep
 *                        state.
 *                      - @a TIME_IMMEDIATE this value is not allowed.
 *                      .
 *
 * @api
 */
void chThdSleep(systime_t time) {

  chSysLock();
  chThdSleepS(time);
  chSysUnlock();
}

/**
 * @brief   Suspends the invoking thread until the system time arrives to the
 *          specified value.
 * @note    The function has no concept of "past", all specifiable times
 *          are in the future, this means that if you call this function
 *          exceeding your calculated intervals then the function will
 *          return in a far future time, not immediately.
 * @see     chThdSleepUntilWindowed()
 *
 * @param[in] time      absolute system time
 *
 * @api
 */
void chThdSleepUntil(systime_t time) {

  chSysLock();
  time -= chVTGetSystemTimeX();
  if (time > (systime_t)0) {
    chThdSleepS(time);
  }
  chSysUnlock();
}

/**
 * @brief   Suspends the invoking thread until the system time arrives to the
 *          specified value.
 * @note    The system time is assumed to be between @p prev and @p time
 *          else the call is assumed to have been called outside the
 *          allowed time interval, in this case no sleep is performed.
 * @see     chThdSleepUntil()
 *
 * @param[in] prev      absolute system time of the previous deadline
 * @param[in] next      absolute system time of the next deadline
 * @return				the @p next parameter
 *
 * @api
 */
systime_t chThdSleepUntilWindowed(systime_t prev, systime_t next) {
  systime_t time;

  chSysLock();
  time = chVTGetSystemTimeX();
  if (chVTIsTimeWithinX(time, prev, next)) {
	chThdSleepS(next - time);
  }
  chSysUnlock();

  return next;
}

/**
 * @brief   Yields the time slot.
 * @details Yields the CPU control to the next thread in the ready list with
 *          equal priority, if any.
 *
 * @api
 */
void chThdYield(void) {

  chSysLock();
  chSchDoYieldS();
  chSysUnlock();
}

/**
 * @brief   Terminates the current thread.
 * @details The thread goes in the @p CH_STATE_FINAL state holding the
 *          specified exit status code, other threads can retrieve the
 *          exit status code by invoking the function @p chThdWait().
 * @post    Eventual code after this function will never be executed,
 *          this function never returns. The compiler has no way to
 *          know this so do not assume that the compiler would remove
 *          the dead code.
 *
 * @param[in] msg       thread exit code
 *
 * @api
 */
void chThdExit(msg_t msg) {

  chSysLock();
  chThdExitS(msg);
  /* The thread never returns here.*/
}

/**
 * @brief   Terminates the current thread.
 * @details The thread goes in the @p CH_STATE_FINAL state holding the
 *          specified exit status code, other threads can retrieve the
 *          exit status code by invoking the function @p chThdWait().
 * @post    Eventual code after this function will never be executed,
 *          this function never returns. The compiler has no way to
 *          know this so do not assume that the compiler would remove
 *          the dead code.
 *
 * @param[in] msg       thread exit code
 *
 * @sclass
 */
void chThdExitS(msg_t msg) {
  thread_t *tp = currp;

  tp->p_u.exitcode = msg;
#if defined(CH_CFG_THREAD_EXIT_HOOK)
  CH_CFG_THREAD_EXIT_HOOK(tp);
#endif
#if CH_CFG_USE_WAITEXIT == TRUE
  while (list_notempty(&tp->p_waiting)) {
    (void) chSchReadyI(list_remove(&tp->p_waiting));
  }
#endif
#if CH_CFG_USE_REGISTRY == TRUE
  /* Static threads are immediately removed from the registry because
     there is no memory to recover.*/
  if ((tp->p_flags & CH_FLAG_MODE_MASK) == CH_FLAG_MODE_STATIC) {
    REG_REMOVE(tp);
  }
#endif
  chSchGoSleepS(CH_STATE_FINAL);

  /* The thread never returns here.*/
  chDbgAssert(false, "zombies apocalypse");
}

#if (CH_CFG_USE_WAITEXIT == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   Blocks the execution of the invoking thread until the specified
 *          thread terminates then the exit code is returned.
 * @details This function waits for the specified thread to terminate then
 *          decrements its reference counter, if the counter reaches zero then
 *          the thread working area is returned to the proper allocator.<br>
 *          The memory used by the exited thread is handled in different ways
 *          depending on the API that spawned the thread:
 *          - If the thread was spawned by @p chThdCreateStatic() or by
 *            @p chThdCreateI() then nothing happens and the thread working
 *            area is not released or modified in any way. This is the
 *            default, totally static, behavior.
 *          - If the thread was spawned by @p chThdCreateFromHeap() then
 *            the working area is returned to the system heap.
 *          - If the thread was spawned by @p chThdCreateFromMemoryPool()
 *            then the working area is returned to the owning memory pool.
 *          .
 * @pre     The configuration option @p CH_CFG_USE_WAITEXIT must be enabled in
 *          order to use this function.
 * @post    Enabling @p chThdWait() requires 2-4 (depending on the
 *          architecture) extra bytes in the @p thread_t structure.
 * @post    After invoking @p chThdWait() the thread pointer becomes invalid
 *          and must not be used as parameter for further system calls.
 * @note    If @p CH_CFG_USE_DYNAMIC is not specified this function just waits for
 *          the thread termination, no memory allocators are involved.
 *
 * @param[in] tp        pointer to the thread
 * @return              The exit code from the terminated thread.
 *
 * @api
 */
msg_t chThdWait(thread_t *tp) {
  msg_t msg;

  chDbgCheck(tp != NULL);

  chSysLock();
  chDbgAssert(tp != currp, "waiting self");
#if CH_CFG_USE_DYNAMIC == TRUE
  chDbgAssert(tp->p_refs > (trefs_t)0, "not referenced");
#endif
  if (tp->p_state != CH_STATE_FINAL) {
    list_insert(currp, &tp->p_waiting);
    chSchGoSleepS(CH_STATE_WTEXIT);
  }
  msg = tp->p_u.exitcode;
  chSysUnlock();

#if CH_CFG_USE_DYNAMIC == TRUE
  /* Releasing a lock if it is a dynamic thread.*/
  chThdRelease(tp);
#endif

  return msg;
}
#endif /* CH_CFG_USE_WAITEXIT */

/**
 * @brief   Sends the current thread sleeping and sets a reference variable.
 * @note    This function must reschedule, it can only be called from thread
 *          context.
 *
 * @param[in] trp       a pointer to a thread reference object
 * @return              The wake up message.
 *
 * @sclass
 */
msg_t chThdSuspendS(thread_reference_t *trp) {
  thread_t *tp = chThdGetSelfX();

  chDbgAssert(*trp == NULL, "not NULL");

  *trp = tp;
  tp->p_u.wttrp = trp;
  chSchGoSleepS(CH_STATE_SUSPENDED);

  return chThdGetSelfX()->p_u.rdymsg;
}

/**
 * @brief   Sends the current thread sleeping and sets a reference variable.
 * @note    This function must reschedule, it can only be called from thread
 *          context.
 *
 * @param[in] trp       a pointer to a thread reference object
 * @param[in] timeout   the timeout in system ticks, the special values are
 *                      handled as follow:
 *                      - @a TIME_INFINITE the thread enters an infinite sleep
 *                        state.
 *                      - @a TIME_IMMEDIATE the thread is not enqueued and
 *                        the function returns @p MSG_TIMEOUT as if a timeout
 *                        occurred.
 *                      .
 * @return              The wake up message.
 * @retval MSG_TIMEOUT  if the operation timed out.
 *
 * @sclass
 */
msg_t chThdSuspendTimeoutS(thread_reference_t *trp, systime_t timeout) {
  thread_t *tp = chThdGetSelfX();

  chDbgAssert(*trp == NULL, "not NULL");

  if (TIME_IMMEDIATE == timeout) {
    return MSG_TIMEOUT;
  }

  *trp = tp;
  tp->p_u.wttrp = trp;

  return chSchGoSleepTimeoutS(CH_STATE_SUSPENDED, timeout);
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
    thread_t *tp = *trp;

    chDbgAssert(tp->p_state == CH_STATE_SUSPENDED,
                "not THD_STATE_SUSPENDED");

    *trp = NULL;
    tp->p_u.rdymsg = msg;
    (void) chSchReadyI(tp);
  }
}

/**
 * @brief   Wakes up a thread waiting on a thread reference object.
 * @note    This function must reschedule, it can only be called from thread
 *          context.
 *
 * @param[in] trp       a pointer to a thread reference object
 * @param[in] msg       the message code
 *
 * @iclass
 */
void chThdResumeS(thread_reference_t *trp, msg_t msg) {

  if (*trp != NULL) {
    thread_t *tp = *trp;

    chDbgAssert(tp->p_state == CH_STATE_SUSPENDED,
                "not THD_STATE_SUSPENDED");

    *trp = NULL;
    chSchWakeupS(tp, msg);
  }
}

/**
 * @brief   Wakes up a thread waiting on a thread reference object.
 * @note    This function must reschedule, it can only be called from thread
 *          context.
 *
 * @param[in] trp       a pointer to a thread reference object
 * @param[in] msg       the message code
 *
 * @api
 */
void chThdResume(thread_reference_t *trp, msg_t msg) {

  chSysLock();
  chThdResumeS(trp, msg);
  chSysUnlock();
}

/**
 * @brief   Enqueues the caller thread on a threads queue object.
 * @details The caller thread is enqueued and put to sleep until it is
 *          dequeued or the specified timeouts expires.
 *
 * @param[in] tqp       pointer to the threads queue object
 * @param[in] timeout   the timeout in system ticks, the special values are
 *                      handled as follow:
 *                      - @a TIME_INFINITE the thread enters an infinite sleep
 *                        state.
 *                      - @a TIME_IMMEDIATE the thread is not enqueued and
 *                        the function returns @p MSG_TIMEOUT as if a timeout
 *                        occurred.
 *                      .
 * @return              The message from @p osalQueueWakeupOneI() or
 *                      @p osalQueueWakeupAllI() functions.
 * @retval MSG_TIMEOUT  if the thread has not been dequeued within the
 *                      specified timeout or if the function has been
 *                      invoked with @p TIME_IMMEDIATE as timeout
 *                      specification.
 *
 * @sclass
 */
msg_t chThdEnqueueTimeoutS(threads_queue_t *tqp, systime_t timeout) {

  if (TIME_IMMEDIATE == timeout) {
    return MSG_TIMEOUT;
  }

  queue_insert(currp, tqp);

  return chSchGoSleepTimeoutS(CH_STATE_QUEUED, timeout);
}

/**
 * @brief   Dequeues and wakes up one thread from the threads queue object,
 *          if any.
 *
 * @param[in] tqp       pointer to the threads queue object
 * @param[in] msg       the message code
 *
 * @iclass
 */
void chThdDequeueNextI(threads_queue_t *tqp, msg_t msg) {

  if (queue_notempty(tqp)) {
    chThdDoDequeueNextI(tqp, msg);
  }
}

/**
 * @brief   Dequeues and wakes up all threads from the threads queue object.
 *
 * @param[in] tqp       pointer to the threads queue object
 * @param[in] msg       the message code
 *
 * @iclass
 */
void chThdDequeueAllI(threads_queue_t *tqp, msg_t msg) {

  while (queue_notempty(tqp)) {
    chThdDoDequeueNextI(tqp, msg);
  }
}

/** @} */
