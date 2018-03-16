/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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
 * @file    osal.c
 * @brief   OSAL module code.
 *
 * @addtogroup OSAL
 * @{
 */

#include "osal.h"

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   Pointer to a halt error message.
 * @note    The message is meant to be retrieved by the debugger after the
 *          system halt caused by an unexpected error.
 */
const char *osal_halt_msg;

/**
 * @brief   Virtual timers delta list header.
 */
virtual_timers_list_t vtlist;

/*===========================================================================*/
/* Module local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Timers initialization.
 *
 * @notapi
 */
static void vtInit(void) {

  /* Virtual Timers initialization.*/
  vtlist.vt_next = vtlist.vt_prev = (void *)&vtlist;
  vtlist.vt_time = (systime_t)-1;
  vtlist.vt_systime = 0;
}

/**
 * @brief   Returns @p TRUE if the specified timer is armed.
 *
 * @param[out] vtp      the @p virtual_timer_t structure pointer
 *
 * @notapi
 */
static bool vtIsArmedI(virtual_timer_t *vtp) {

  return vtp->vt_func != NULL;
}

/**
 * @brief   Virtual timers ticker.
 * @note    The system lock is released before entering the callback and
 *          re-acquired immediately after. It is callback's responsibility
 *          to acquire the lock if needed. This is done in order to reduce
 *          interrupts jitter when many timers are in use.
 *
 * @notapi
 */
static void vtDoTickI(void) {

  vtlist.vt_systime++;
  if (&vtlist != (virtual_timers_list_t *)vtlist.vt_next) {
    virtual_timer_t *vtp;

    --vtlist.vt_next->vt_time;
    while (!(vtp = vtlist.vt_next)->vt_time) {
      vtfunc_t fn = vtp->vt_func;
      vtp->vt_func = (vtfunc_t)NULL;
      vtp->vt_next->vt_prev = (void *)&vtlist;
      (&vtlist)->vt_next = vtp->vt_next;
      osalSysUnlockFromISR();
      fn(vtp->vt_par);
      osalSysLockFromISR();
    }
  }
}

/**
 * @brief   Enables a virtual timer.
 * @note    The associated function is invoked from interrupt context.
 *
 * @param[out] vtp      the @p virtual_timer_t structure pointer
 * @param[in] time      the number of ticks before the operation timeouts, the
 *                      special values are handled as follow:
 *                      - @a TIME_INFINITE is allowed but interpreted as a
 *                        normal time specification.
 *                      - @a TIME_IMMEDIATE this value is not allowed.
 *                      .
 * @param[in] vtfunc    the timer callback function. After invoking the
 *                      callback the timer is disabled and the structure can
 *                      be disposed or reused.
 * @param[in] par       a parameter that will be passed to the callback
 *                      function
 *
 * @notapi
 */
static void vtSetI(virtual_timer_t *vtp, systime_t time,
                   vtfunc_t vtfunc, void *par) {
  virtual_timer_t *p;

  vtp->vt_par = par;
  vtp->vt_func = vtfunc;
  p = vtlist.vt_next;
  while (p->vt_time < time) {
    time -= p->vt_time;
    p = p->vt_next;
  }

  vtp->vt_prev = (vtp->vt_next = p)->vt_prev;
  vtp->vt_prev->vt_next = p->vt_prev = vtp;
  vtp->vt_time = time;
  if (p != (void *)&vtlist)
    p->vt_time -= time;
}

/**
 * @brief   Disables a Virtual Timer.
 * @note    The timer MUST be active when this function is invoked.
 *
 * @param[in] vtp       the @p virtual_timer_t structure pointer
 *
 * @notapi
 */
static void vtResetI(virtual_timer_t *vtp) {

  if (vtp->vt_next != (void *)&vtlist)
    vtp->vt_next->vt_time += vtp->vt_time;
  vtp->vt_prev->vt_next = vtp->vt_next;
  vtp->vt_next->vt_prev = vtp->vt_prev;
  vtp->vt_func = (vtfunc_t)NULL;
}


static void callback_timeout(void *p) {
  osalSysLockFromISR();
  osalThreadResumeI((thread_reference_t *)p, MSG_TIMEOUT);
  osalSysUnlockFromISR();
}

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   OSAL module initialization.
 *
 * @api
 */
void osalInit(void) {

  vtInit();

  OSAL_INIT_HOOK();
}

/**
 * @brief   System halt with error message.
 *
 * @param[in] reason    the halt message pointer
 *
 * @api
 */
#if !defined(__DOXYGEN__)
__attribute__((weak, noreturn))
#endif
void osalSysHalt(const char *reason) {

  osalSysDisable();
  osal_halt_msg = reason;
  while (true) {
  }
}

/**
 * @brief   Polled delay.
 * @note    The real delay is always few cycles in excess of the specified
 *          value.
 *
 * @param[in] cycles    number of cycles
 *
 * @xclass
 */
void osalSysPolledDelayX(rtcnt_t cycles) {

  (void)cycles;
}

/**
 * @brief   System timer handler.
 * @details The handler is used for scheduling and Virtual Timers management.
 *
 * @iclass
 */
void osalOsTimerHandlerI(void) {

  osalDbgCheckClassI();

  vtDoTickI();
}

/**
 * @brief   Checks if a reschedule is required and performs it.
 * @note    I-Class functions invoked from thread context must not reschedule
 *          by themselves, an explicit reschedule using this function is
 *          required in this scenario.
 * @note    Not implemented in this simplified OSAL.
 *
 * @sclass
 */
void osalOsRescheduleS(void) {

}

/**
 * @brief   Current system time.
 * @details Returns the number of system ticks since the @p osalInit()
 *          invocation.
 * @note    The counter can reach its maximum and then restart from zero.
 * @note    This function can be called from any context but its atomicity
 *          is not guaranteed on architectures whose word size is less than
 *          @p systime_t size.
 *
 * @return              The system time in ticks.
 *
 * @xclass
 */
systime_t osalOsGetSystemTimeX(void) {

  return vtlist.vt_systime;
}

/**
 * @brief   Suspends the invoking thread for the specified time.
 *
 * @param[in] time      the delay in system ticks, the special values are
 *                      handled as follow:
 *                      - @a TIME_INFINITE is allowed but interpreted as a
 *                        normal time specification.
 *                      - @a TIME_IMMEDIATE this value is not allowed.
 *                      .
 *
 * @sclass
 */
void osalThreadSleepS(systime_t time) {
  virtual_timer_t vt;
  thread_reference_t tr;

  tr = NULL;
  vtSetI(&vt, time, callback_timeout, (void *)&tr);
  osalThreadSuspendS(&tr);
}

/**
 * @brief   Suspends the invoking thread for the specified time.
 *
 * @param[in] time      the delay in system ticks, the special values are
 *                      handled as follow:
 *                      - @a TIME_INFINITE is allowed but interpreted as a
 *                        normal time specification.
 *                      - @a TIME_IMMEDIATE this value is not allowed.
 *                      .
 *
 * @api
 */
void osalThreadSleep(systime_t time) {

  osalSysLock();
  osalThreadSleepS(time);
  osalSysUnlock();
}

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
msg_t osalThreadSuspendS(thread_reference_t *trp) {
  thread_t self = {MSG_WAIT};

  osalDbgCheck(trp != NULL);

  *trp = &self;
  while (self.message == MSG_WAIT) {
    osalSysUnlock();
    /* A state-changing interrupt could occur here and cause the loop to
       terminate, an hook macro is executed while waiting.*/
    OSAL_IDLE_HOOK();
    osalSysLock();
  }

  return self.message;
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
msg_t osalThreadSuspendTimeoutS(thread_reference_t *trp, systime_t timeout) {
  msg_t msg;
  virtual_timer_t vt;

  osalDbgCheck(trp != NULL);

  if (TIME_INFINITE == timeout)
   return osalThreadSuspendS(trp);

  vtSetI(&vt, timeout, callback_timeout, (void *)trp);
  msg = osalThreadSuspendS(trp);
  if (vtIsArmedI(&vt))
    vtResetI(&vt);

  return msg;
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
void osalThreadResumeI(thread_reference_t *trp, msg_t msg) {

  osalDbgCheck(trp != NULL);

  if (*trp != NULL) {
    (*trp)->message = msg;
    *trp = NULL;
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
void osalThreadResumeS(thread_reference_t *trp, msg_t msg) {

  osalDbgCheck(trp != NULL);

  if (*trp != NULL) {
    (*trp)->message = msg;
    *trp = NULL;
  }
}

/**
 * @brief   Enqueues the caller thread.
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
msg_t osalThreadEnqueueTimeoutS(threads_queue_t *tqp, systime_t timeout) {
  msg_t msg;
  virtual_timer_t vt;

  osalDbgCheck(tqp != NULL);

  if (TIME_IMMEDIATE == timeout)
    return MSG_TIMEOUT;

  tqp->tr = NULL;

  if (TIME_INFINITE == timeout)
   return osalThreadSuspendS(&tqp->tr);

  vtSetI(&vt, timeout, callback_timeout, (void *)&tqp->tr);
  msg = osalThreadSuspendS(&tqp->tr);
  if (vtIsArmedI(&vt))
    vtResetI(&vt);

  return msg;
}

/**
 * @brief   Dequeues and wakes up one thread from the queue, if any.
 *
 * @param[in] tqp       pointer to the threads queue object
 * @param[in] msg       the message code
 *
 * @iclass
 */
void osalThreadDequeueNextI(threads_queue_t *tqp, msg_t msg) {

  osalDbgCheck(tqp != NULL);

  osalThreadResumeI(&tqp->tr, msg);
}

/**
 * @brief   Dequeues and wakes up all threads from the queue.
 *
 * @param[in] tqp       pointer to the threads queue object
 * @param[in] msg       the message code
 *
 * @iclass
 */
void osalThreadDequeueAllI(threads_queue_t *tqp, msg_t msg) {

  osalDbgCheck(tqp != NULL);

  osalThreadResumeI(&tqp->tr, msg);
}

/**
 * @brief   Add flags to an event source object.
 *
 * @param[in] esp       pointer to the event flags object
 * @param[in] flags     flags to be ORed to the flags mask
 *
 * @iclass
 */
void osalEventBroadcastFlagsI(event_source_t *esp, eventflags_t flags) {

  osalDbgCheck(esp != NULL);

  esp->flags |= flags;
  if (esp->cb != NULL) {
    esp->cb(esp);
  }
}

/**
 * @brief   Add flags to an event source object.
 *
 * @param[in] esp       pointer to the event flags object
 * @param[in] flags     flags to be ORed to the flags mask
 *
 * @iclass
 */
void osalEventBroadcastFlags(event_source_t *esp, eventflags_t flags) {

  osalDbgCheck(esp != NULL);

  osalSysLock();
  osalEventBroadcastFlagsI(esp, flags);
  osalSysUnlock();
}

/**
 * @brief   Event callback setup.
 * @note    The callback is invoked from ISR context and can
 *          only invoke I-Class functions. The callback is meant
 *          to wakeup the task that will handle the event by
 *          calling @p osalEventGetAndClearFlagsI().
 * @note    This function is not part of the OSAL API and is provided
 *          exclusively as an example and for convenience.
 *
 * @param[in] esp       pointer to the event flags object
 * @param[in] cb        pointer to the callback function
 * @param[in] param     parameter to be passed to the callback function
 *
 * @api
 */
void osalEventSetCallback(event_source_t *esp,
                          eventcallback_t cb,
                          void *param) {

  osalDbgCheck(esp != NULL);

  esp->cb    = cb;
  esp->param = param;
}

/**
 * @brief   Locks the specified mutex.
 * @post    The mutex is locked and inserted in the per-thread stack of owned
 *          mutexes.
 *
 * @param[in,out] mp    pointer to the @p mutex_t object
 *
 * @api
 */
void osalMutexLock(mutex_t *mp) {

  osalDbgCheck(mp != NULL);

  *mp = 1;
}

/**
 * @brief   Unlocks the specified mutex.
 * @note    The HAL guarantees to release mutex in reverse lock order. The
 *          mutex being unlocked is guaranteed to be the last locked mutex
 *          by the invoking thread.
 *          The implementation can rely on this behavior and eventually
 *          ignore the @p mp parameter which is supplied in order to support
 *          those OSes not supporting a stack of the owned mutexes.
 *
 * @param[in,out] mp    pointer to the @p mutex_t object
 *
 * @api
 */
void osalMutexUnlock(mutex_t *mp) {

  osalDbgCheck(mp != NULL);

  *mp = 0;
}

/** @} */
