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
 * @file    chschd.h
 * @brief   Scheduler macros and structures.
 *
 * @addtogroup scheduler
 * @{
 */

#ifndef CHSCHD_H
#define CHSCHD_H

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/**
 * @name    Wakeup status codes
 * @{
 */
#define MSG_OK              (msg_t)0    /**< @brief Normal wakeup message.  */
#define MSG_TIMEOUT         (msg_t)-1   /**< @brief Wakeup caused by a timeout
                                             condition.                     */
#define MSG_RESET           (msg_t)-2   /**< @brief Wakeup caused by a reset
                                             condition.                     */
/** @} */

/**
 * @name    Priority constants
 * @{
 */
#define NOPRIO              (tprio_t)0      /**< @brief Ready list header
                                                 priority.                  */
#define IDLEPRIO            (tprio_t)1      /**< @brief Idle priority.      */
#define LOWPRIO             (tprio_t)2      /**< @brief Lowest priority.    */
#define NORMALPRIO          (tprio_t)128    /**< @brief Normal priority.    */
#define HIGHPRIO            (tprio_t)255    /**< @brief Highest priority.   */
/** @} */

/**
 * @name    Thread states
 * @{
 */
#define CH_STATE_READY      (tstate_t)0      /**< @brief Waiting on the
                                                  ready list.               */
#define CH_STATE_CURRENT    (tstate_t)1      /**< @brief Currently running. */
#define CH_STATE_WTSTART    (tstate_t)2      /**< @brief Just created.      */
#define CH_STATE_SUSPENDED  (tstate_t)3      /**< @brief Suspended state.   */
#define CH_STATE_QUEUED     (tstate_t)4      /**< @brief On a queue.        */
#define CH_STATE_WTSEM      (tstate_t)5      /**< @brief On a semaphore.    */
#define CH_STATE_WTMTX      (tstate_t)6      /**< @brief On a mutex.        */
#define CH_STATE_WTCOND     (tstate_t)7      /**< @brief On a cond.variable.*/
#define CH_STATE_SLEEPING   (tstate_t)8      /**< @brief Sleeping.          */
#define CH_STATE_WTEXIT     (tstate_t)9      /**< @brief Waiting a thread.  */
#define CH_STATE_WTOREVT    (tstate_t)10     /**< @brief One event.         */
#define CH_STATE_WTANDEVT   (tstate_t)11     /**< @brief Several events.    */
#define CH_STATE_SNDMSGQ    (tstate_t)12     /**< @brief Sending a message,
                                                  in queue.                 */
#define CH_STATE_SNDMSG     (tstate_t)13     /**< @brief Sent a message,
                                                  waiting answer.           */
#define CH_STATE_WTMSG      (tstate_t)14     /**< @brief Waiting for a
                                                  message.                  */
#define CH_STATE_FINAL      (tstate_t)15     /**< @brief Thread terminated. */

/**
 * @brief   Thread states as array of strings.
 * @details Each element in an array initialized with this macro can be
 *          indexed using the numeric thread state values.
 */
#define CH_STATE_NAMES                                                     \
  "READY", "CURRENT", "WTSTART", "SUSPENDED", "QUEUED", "WTSEM", "WTMTX",  \
  "WTCOND", "SLEEPING", "WTEXIT", "WTOREVT", "WTANDEVT", "SNDMSGQ",        \
  "SNDMSG", "WTMSG", "FINAL"
/** @} */

/**
 * @name    Thread flags and attributes
 * @{
 */
#define CH_FLAG_MODE_MASK   (tmode_t)3U     /**< @brief Thread memory mode
                                                 mask.                      */
#define CH_FLAG_MODE_STATIC (tmode_t)0U     /**< @brief Static thread.      */
#define CH_FLAG_MODE_HEAP   (tmode_t)1U     /**< @brief Thread allocated
                                                 from a Memory Heap.        */
#define CH_FLAG_MODE_MPOOL  (tmode_t)2U     /**< @brief Thread allocated
                                                 from a Memory Pool.        */
#define CH_FLAG_TERMINATE   (tmode_t)4U     /**< @brief Termination requested
                                                 flag.                      */
/** @} */

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
 * @brief   Generic threads single link list, it works like a stack.
 */
struct ch_threads_list {
  thread_t              *next;      /**< @brief Next in the list/queue.     */
};

/**
 * @brief   Generic threads bidirectional linked list header and element.
 */
struct ch_threads_queue {
  thread_t              *next;      /**< @brief Next in the list/queue.     */
  thread_t              *prev;      /**< @brief Previous in the queue.      */
};

/**
 * @brief   Structure representing a thread.
 * @note    Not all the listed fields are always needed, by switching off some
 *          not needed ChibiOS/RT subsystems it is possible to save RAM space
 *          by shrinking this structure.
 */
struct ch_thread {
  threads_queue_t       queue;      /**< @brief Threads queue header.       */
  tprio_t               prio;       /**< @brief Thread priority.            */
  struct port_context   ctx;        /**< @brief Processor context.          */
#if (CH_CFG_USE_REGISTRY == TRUE) || defined(__DOXYGEN__)
  thread_t              *newer;     /**< @brief Newer registry element.     */
  thread_t              *older;     /**< @brief Older registry element.     */
#endif
  /* End of the fields shared with the ReadyList structure. */
#if (CH_CFG_USE_REGISTRY == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief   Thread name or @p NULL.
   */
  const char            *name;
#endif
#if (CH_DBG_ENABLE_STACK_CHECK == TRUE) || (CH_CFG_USE_DYNAMIC == TRUE) || (CH_DBG_FILL_THREADS == TRUE) ||  \
    defined(__DOXYGEN__)
  /**
   * @brief   Working area base address.
   * @note    This pointer is used for stack overflow checks and for
   *          dynamic threading.
   */
  stkalign_t            *wabase;
  stkalign_t            *waend;
#endif
  /**
   * @brief   Current thread state.
   */
  tstate_t              state;
  /**
   * @brief   Various thread flags.
   */
  tmode_t               flags;
#if (CH_CFG_USE_REGISTRY == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief   References to this thread.
   */
  trefs_t               refs;
#endif
  /**
   * @brief   Number of ticks remaining to this thread.
   */
#if (CH_CFG_TIME_QUANTUM > 0) || defined(__DOXYGEN__)
  tslices_t             ticks;
#endif
#if (CH_DBG_THREADS_PROFILING == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief   Thread consumed time in ticks.
   * @note    This field can overflow.
   */
  volatile systime_t    time;
#endif
  /**
   * @brief   State-specific fields.
   * @note    All the fields declared in this union are only valid in the
   *          specified state or condition and are thus volatile.
   */
  union {
    /**
     * @brief   Thread wakeup code.
     * @note    This field contains the low level message sent to the thread
     *          by the waking thread or interrupt handler. The value is valid
     *          after exiting the @p chSchWakeupS() function.
     */
    msg_t               rdymsg;
    /**
     * @brief   Thread exit code.
     * @note    The thread termination code is stored in this field in order
     *          to be retrieved by the thread performing a @p chThdWait() on
     *          this thread.
     */
    msg_t               exitcode;
    /**
     * @brief   Pointer to a generic "wait" object.
     * @note    This field is used to get a generic pointer to a synchronization
     *          object and is valid when the thread is in one of the wait
     *          states.
     */
    void                *wtobjp;
    /**
     * @brief   Pointer to a generic thread reference object.
     * @note    This field is used to get a pointer to a synchronization
     *          object and is valid when the thread is in @p CH_STATE_SUSPENDED
     *          state.
     */
    thread_reference_t  *wttrp;
#if (CH_CFG_USE_MESSAGES == TRUE) || defined(__DOXYGEN__)
    /**
     * @brief   Thread sent message.
     */
    msg_t               sentmsg;
#endif
#if (CH_CFG_USE_SEMAPHORES == TRUE) || defined(__DOXYGEN__)
    /**
     * @brief   Pointer to a generic semaphore object.
     * @note    This field is used to get a pointer to a synchronization
     *          object and is valid when the thread is in @p CH_STATE_WTSEM
     *          state.
     */
    struct ch_semaphore *wtsemp;
#endif
#if (CH_CFG_USE_MUTEXES == TRUE) || defined(__DOXYGEN__)
    /**
     * @brief   Pointer to a generic mutex object.
     * @note    This field is used to get a pointer to a synchronization
     *          object and is valid when the thread is in @p CH_STATE_WTMTX
     *          state.
     */
    struct ch_mutex     *wtmtxp;
#endif
#if (CH_CFG_USE_EVENTS == TRUE) || defined(__DOXYGEN__)
    /**
     * @brief   Enabled events mask.
     * @note    This field is only valid while the thread is in the
     *          @p CH_STATE_WTOREVT or @p CH_STATE_WTANDEVT states.
     */
    eventmask_t         ewmask;
#endif
  }                     u;
#if (CH_CFG_USE_WAITEXIT == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief   Termination waiting list.
   */
  threads_list_t        waiting;
#endif
#if (CH_CFG_USE_MESSAGES == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief   Messages queue.
   */
  threads_queue_t       msgqueue;
#endif
#if (CH_CFG_USE_EVENTS == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief   Pending events mask.
   */
  eventmask_t           epending;
#endif
#if (CH_CFG_USE_MUTEXES == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief   List of the mutexes owned by this thread.
   * @note    The list is terminated by a @p NULL in this field.
   */
  struct ch_mutex       *mtxlist;
  /**
   * @brief   Thread's own, non-inherited, priority.
   */
  tprio_t               realprio;
#endif
#if ((CH_CFG_USE_DYNAMIC == TRUE) && (CH_CFG_USE_MEMPOOLS == TRUE)) ||      \
    defined(__DOXYGEN__)
  /**
   * @brief   Memory Pool where the thread workspace is returned.
   */
  void                  *mpool;
#endif
#if (CH_DBG_STATISTICS == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief   Thread statistics.
   */
  time_measurement_t    stats;
#endif
#if defined(CH_CFG_THREAD_EXTRA_FIELDS)
  /* Extra fields defined in chconf.h.*/
  CH_CFG_THREAD_EXTRA_FIELDS
#endif
};

/**
 * @extends virtual_timers_list_t
 *
 * @brief   Virtual Timer descriptor structure.
 */
struct ch_virtual_timer {
  virtual_timer_t       *next;      /**< @brief Next timer in the list.     */
  virtual_timer_t       *prev;      /**< @brief Previous timer in the list. */
  sysinterval_t         delta;      /**< @brief Time delta before timeout.  */
  vtfunc_t              func;       /**< @brief Timer callback function
                                                pointer.                    */
  void                  *par;       /**< @brief Timer callback function
                                                parameter.                  */
};

/**
 * @brief   Virtual timers list header.
 * @note    The timers list is implemented as a double link bidirectional list
 *          in order to make the unlink time constant, the reset of a virtual
 *          timer is often used in the code.
 */
struct ch_virtual_timers_list {
  virtual_timer_t       *next;      /**< @brief Next timer in the delta
                                                list.                       */
  virtual_timer_t       *prev;      /**< @brief Last timer in the delta
                                                list.                       */
  sysinterval_t         delta;      /**< @brief Must be initialized to -1.  */
#if (CH_CFG_ST_TIMEDELTA == 0) || defined(__DOXYGEN__)
  volatile systime_t    systime;    /**< @brief System Time counter.        */
#endif
#if (CH_CFG_ST_TIMEDELTA > 0) || defined(__DOXYGEN__)
  /**
   * @brief   System time of the last tick event.
   */
  systime_t             lasttime;   /**< @brief System time of the last
                                                tick event.                 */
#endif
};

/**
 * @extends threads_queue_t
 */
struct ch_ready_list {
  threads_queue_t       queue;      /**< @brief Threads queue.              */
  tprio_t               prio;       /**< @brief This field must be
                                                initialized to zero.        */
  struct port_context   ctx;        /**< @brief Not used, present because
                                                offsets.                    */
#if (CH_CFG_USE_REGISTRY == TRUE) || defined(__DOXYGEN__)
  thread_t              *newer;     /**< @brief Newer registry element.     */
  thread_t              *older;     /**< @brief Older registry element.     */
#endif
  /* End of the fields shared with the thread_t structure.*/
  thread_t              *current;   /**< @brief The currently running
                                                thread.                     */
};

/**
 * @brief   System debug data structure.
 */
struct ch_system_debug {
  /**
   * @brief   Pointer to the panic message.
   * @details This pointer is meant to be accessed through the debugger, it is
   *          written once and then the system is halted.
   * @note    Accesses to this pointer must never be optimized out so the
   *          field itself is declared volatile.
   */
  const char            * volatile panic_msg;
#if (CH_DBG_SYSTEM_STATE_CHECK == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief   ISR nesting level.
   */
  cnt_t                 isr_cnt;
  /**
   * @brief   Lock nesting level.
   */
  cnt_t                 lock_cnt;
#endif
#if (CH_DBG_TRACE_MASK != CH_DBG_TRACE_MASK_DISABLED) || defined(__DOXYGEN__)
  /**
   * @brief   Public trace buffer.
   */
  ch_trace_buffer_t     trace_buffer;
#endif
};

/**
 * @brief   System data structure.
 * @note    This structure contain all the data areas used by the OS except
 *          stacks.
 */
struct ch_system {
  /**
   * @brief   Ready list header.
   */
  ready_list_t          rlist;
  /**
   * @brief   Virtual timers delta list header.
   */
  virtual_timers_list_t vtlist;
  /**
   * @brief   System debug.
   */
  system_debug_t        dbg;
  /**
   * @brief   Main thread descriptor.
   */
  thread_t              mainthread;
#if (CH_CFG_USE_TM == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief   Time measurement calibration data.
   */
  tm_calibration_t      tm;
#endif
#if (CH_DBG_STATISTICS == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief   Global kernel statistics.
   */
  kernel_stats_t        kernel_stats;
#endif
  CH_CFG_SYSTEM_EXTRA_FIELDS
};

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Returns the priority of the first thread on the given ready list.
 *
 * @notapi
 */
#define firstprio(rlp)  ((rlp)->next->prio)

/**
 * @brief   Current thread pointer access macro.
 * @note    This macro is not meant to be used in the application code but
 *          only from within the kernel, use @p chThdGetSelfX() instead.
 */
#define currp ch.rlist.current

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if !defined(__DOXYGEN__)
extern ch_system_t ch;
#endif

/*
 * Scheduler APIs.
 */
#ifdef __cplusplus
extern "C" {
#endif
  void _scheduler_init(void);
  thread_t *chSchReadyI(thread_t *tp);
  thread_t *chSchReadyAheadI(thread_t *tp);
  void chSchGoSleepS(tstate_t newstate);
  msg_t chSchGoSleepTimeoutS(tstate_t newstate, sysinterval_t timeout);
  void chSchWakeupS(thread_t *ntp, msg_t msg);
  void chSchRescheduleS(void);
  bool chSchIsPreemptionRequired(void);
  void chSchDoRescheduleBehind(void);
  void chSchDoRescheduleAhead(void);
  void chSchDoReschedule(void);
#if CH_CFG_OPTIMIZE_SPEED == FALSE
  void queue_prio_insert(thread_t *tp, threads_queue_t *tqp);
  void queue_insert(thread_t *tp, threads_queue_t *tqp);
  thread_t *queue_fifo_remove(threads_queue_t *tqp);
  thread_t *queue_lifo_remove(threads_queue_t *tqp);
  thread_t *queue_dequeue(thread_t *tp);
  void list_insert(thread_t *tp, threads_list_t *tlp);
  thread_t *list_remove(threads_list_t *tlp);
#endif /* CH_CFG_OPTIMIZE_SPEED == FALSE */
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

/**
 * @brief   Threads list initialization.
 *
 * @param[in] tlp       pointer to the threads list object
 *
 * @notapi
 */
static inline void list_init(threads_list_t *tlp) {

  tlp->next = (thread_t *)tlp;
}

/**
 * @brief   Evaluates to @p true if the specified threads list is empty.
 *
 * @param[in] tlp       pointer to the threads list object
 * @return              The status of the list.
 *
 * @notapi
 */
static inline bool list_isempty(threads_list_t *tlp) {

  return (bool)(tlp->next == (thread_t *)tlp);
}

/**
 * @brief   Evaluates to @p true if the specified threads list is not empty.
 *
 * @param[in] tlp       pointer to the threads list object
 * @return              The status of the list.
 *
 * @notapi
 */
static inline bool list_notempty(threads_list_t *tlp) {

  return (bool)(tlp->next != (thread_t *)tlp);
}

/**
 * @brief   Threads queue initialization.
 *
 * @param[in] tqp       pointer to the threads queue object
 *
 * @notapi
 */
static inline void queue_init(threads_queue_t *tqp) {

  tqp->next = (thread_t *)tqp;
  tqp->prev = (thread_t *)tqp;
}

/**
 * @brief   Evaluates to @p true if the specified threads queue is empty.
 *
 * @param[in] tqp       pointer to the threads queue object
 * @return              The status of the queue.
 *
 * @notapi
 */
static inline bool queue_isempty(const threads_queue_t *tqp) {

  return (bool)(tqp->next == (const thread_t *)tqp);
}

/**
 * @brief   Evaluates to @p true if the specified threads queue is not empty.
 *
 * @param[in] tqp       pointer to the threads queue object
 * @return              The status of the queue.
 *
 * @notapi
 */
static inline bool queue_notempty(const threads_queue_t *tqp) {

  return (bool)(tqp->next != (const thread_t *)tqp);
}

/* If the performance code path has been chosen then all the following
   functions are inlined into the various kernel modules.*/
#if CH_CFG_OPTIMIZE_SPEED == TRUE
static inline void list_insert(thread_t *tp, threads_list_t *tlp) {

  tp->queue.next = tlp->next;
  tlp->next = tp;
}

static inline thread_t *list_remove(threads_list_t *tlp) {

  thread_t *tp = tlp->next;
  tlp->next = tp->queue.next;

  return tp;
}

static inline void queue_prio_insert(thread_t *tp, threads_queue_t *tqp) {

  thread_t *cp = (thread_t *)tqp;
  do {
    cp = cp->queue.next;
  } while ((cp != (thread_t *)tqp) && (cp->prio >= tp->prio));
  tp->queue.next             = cp;
  tp->queue.prev             = cp->queue.prev;
  tp->queue.prev->queue.next = tp;
  cp->queue.prev             = tp;
}

static inline void queue_insert(thread_t *tp, threads_queue_t *tqp) {

  tp->queue.next             = (thread_t *)tqp;
  tp->queue.prev             = tqp->prev;
  tp->queue.prev->queue.next = tp;
  tqp->prev                  = tp;
}

static inline thread_t *queue_fifo_remove(threads_queue_t *tqp) {
  thread_t *tp = tqp->next;

  tqp->next             = tp->queue.next;
  tqp->next->queue.prev = (thread_t *)tqp;

  return tp;
}

static inline thread_t *queue_lifo_remove(threads_queue_t *tqp) {
  thread_t *tp = tqp->prev;

  tqp->prev             = tp->queue.prev;
  tqp->prev->queue.next = (thread_t *)tqp;

  return tp;
}

static inline thread_t *queue_dequeue(thread_t *tp) {

  tp->queue.prev->queue.next = tp->queue.next;
  tp->queue.next->queue.prev = tp->queue.prev;

  return tp;
}
#endif /* CH_CFG_OPTIMIZE_SPEED == TRUE */

/**
 * @brief   Determines if the current thread must reschedule.
 * @details This function returns @p true if there is a ready thread with
 *          higher priority.
 *
 * @return              The priorities situation.
 * @retval false        if rescheduling is not necessary.
 * @retval true         if there is a ready thread at higher priority.
 *
 * @iclass
 */
static inline bool chSchIsRescRequiredI(void) {

  chDbgCheckClassI();

  return firstprio(&ch.rlist.queue) > currp->prio;
}

/**
 * @brief   Determines if yielding is possible.
 * @details This function returns @p true if there is a ready thread with
 *          equal or higher priority.
 *
 * @return              The priorities situation.
 * @retval false        if yielding is not possible.
 * @retval true         if there is a ready thread at equal or higher priority.
 *
 * @sclass
 */
static inline bool chSchCanYieldS(void) {

  chDbgCheckClassS();

  return firstprio(&ch.rlist.queue) >= currp->prio;
}

/**
 * @brief   Yields the time slot.
 * @details Yields the CPU control to the next thread in the ready list with
 *          equal or higher priority, if any.
 *
 * @sclass
 */
static inline void chSchDoYieldS(void) {

  chDbgCheckClassS();

  if (chSchCanYieldS()) {
    chSchDoRescheduleBehind();
  }
}

/**
 * @brief   Inline-able preemption code.
 * @details This is the common preemption code, this function must be invoked
 *          exclusively from the port layer.
 *
 * @special
 */
static inline void chSchPreemption(void) {
  tprio_t p1 = firstprio(&ch.rlist.queue);
  tprio_t p2 = currp->prio;

#if CH_CFG_TIME_QUANTUM > 0
  if (currp->ticks > (tslices_t)0) {
    if (p1 > p2) {
      chSchDoRescheduleAhead();
    }
  }
  else {
    if (p1 >= p2) {
      chSchDoRescheduleBehind();
    }
  }
#else /* CH_CFG_TIME_QUANTUM == 0 */
  if (p1 > p2) {
    chSchDoRescheduleAhead();
  }
#endif /* CH_CFG_TIME_QUANTUM == 0 */
}

#endif /* CHSCHD_H */

/** @} */
