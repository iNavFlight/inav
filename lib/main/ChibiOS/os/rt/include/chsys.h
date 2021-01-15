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
 * @file    chsys.h
 * @brief   System related macros and structures.
 *
 * @addtogroup system
 * @{
 */

#ifndef CHSYS_H
#define CHSYS_H

/*lint -sem(chSysHalt, r_no)*/

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/**
 * @name    Masks of executable integrity checks.
 * @{
 */
#define CH_INTEGRITY_RLIST                  1U
#define CH_INTEGRITY_VTLIST                 2U
#define CH_INTEGRITY_REGISTRY               4U
#define CH_INTEGRITY_PORT                   8U
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

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/**
 * @name    ISRs abstraction macros
 */
/**
 * @brief   Priority level validation macro.
 * @details This macro determines if the passed value is a valid priority
 *          level for the underlying architecture.
 *
 * @param[in] prio      the priority level
 * @return              Priority range result.
 * @retval false        if the priority is invalid or if the architecture
 *                      does not support priorities.
 * @retval true         if the priority is valid.
 */
#if defined(PORT_IRQ_IS_VALID_PRIORITY) || defined(__DOXYGEN__)
#define CH_IRQ_IS_VALID_PRIORITY(prio)                                      \
  PORT_IRQ_IS_VALID_PRIORITY(prio)
#else
#define CH_IRQ_IS_VALID_PRIORITY(prio) false
#endif

/**
 * @brief   Priority level validation macro.
 * @details This macro determines if the passed value is a valid priority
 *          level that cannot preempt the kernel critical zone.
 *
 * @param[in] prio      the priority level
 * @return              Priority range result.
 * @retval false        if the priority is invalid or if the architecture
 *                      does not support priorities.
 * @retval true         if the priority is valid.
 */
#if defined(PORT_IRQ_IS_VALID_KERNEL_PRIORITY) || defined(__DOXYGEN__)
#define CH_IRQ_IS_VALID_KERNEL_PRIORITY(prio)                               \
  PORT_IRQ_IS_VALID_KERNEL_PRIORITY(prio)
#else
#define CH_IRQ_IS_VALID_KERNEL_PRIORITY(prio) false
#endif

/**
 * @brief   IRQ handler enter code.
 * @note    Usually IRQ handlers functions are also declared naked.
 * @note    On some architectures this macro can be empty.
 *
 * @special
 */
#define CH_IRQ_PROLOGUE()                                                   \
  PORT_IRQ_PROLOGUE();                                                      \
  CH_CFG_IRQ_PROLOGUE_HOOK();                                               \
  _stats_increase_irq();                                                    \
  _trace_isr_enter(__func__);                                               \
  _dbg_check_enter_isr()

/**
 * @brief   IRQ handler exit code.
 * @note    Usually IRQ handlers function are also declared naked.
 * @note    This macro usually performs the final reschedule by using
 *          @p chSchIsPreemptionRequired() and @p chSchDoReschedule().
 *
 * @special
 */
#define CH_IRQ_EPILOGUE()                                                   \
  _dbg_check_leave_isr();                                                   \
  _trace_isr_leave(__func__);                                               \
  CH_CFG_IRQ_EPILOGUE_HOOK();                                               \
  PORT_IRQ_EPILOGUE()

/**
 * @brief   Standard normal IRQ handler declaration.
 * @note    @p id can be a function name or a vector number depending on the
 *          port implementation.
 *
 * @special
 */
#define CH_IRQ_HANDLER(id) PORT_IRQ_HANDLER(id)
/** @} */

/**
 * @name    Fast ISRs abstraction macros
 */
/**
 * @brief   Standard fast IRQ handler declaration.
 * @note    @p id can be a function name or a vector number depending on the
 *          port implementation.
 * @note    Not all architectures support fast interrupts.
 *
 * @special
 */
#define CH_FAST_IRQ_HANDLER(id) PORT_FAST_IRQ_HANDLER(id)
/** @} */

/**
 * @name    Time conversion utilities for the realtime counter
 * @{
 */
/**
 * @brief   Seconds to realtime counter.
 * @details Converts from seconds to realtime counter cycles.
 * @note    The macro assumes that @p freq >= @p 1.
 *
 * @param[in] freq      clock frequency, in Hz, of the realtime counter
 * @param[in] sec       number of seconds
 * @return              The number of cycles.
 *
 * @api
 */
#define S2RTC(freq, sec) ((freq) * (sec))

/**
 * @brief   Milliseconds to realtime counter.
 * @details Converts from milliseconds to realtime counter cycles.
 * @note    The result is rounded upward to the next millisecond boundary.
 * @note    The macro assumes that @p freq >= @p 1000.
 *
 * @param[in] freq      clock frequency, in Hz, of the realtime counter
 * @param[in] msec      number of milliseconds
 * @return              The number of cycles.
 *
 * @api
 */
#define MS2RTC(freq, msec) (rtcnt_t)((((freq) + 999UL) / 1000UL) * (msec))

/**
 * @brief   Microseconds to realtime counter.
 * @details Converts from microseconds to realtime counter cycles.
 * @note    The result is rounded upward to the next microsecond boundary.
 * @note    The macro assumes that @p freq >= @p 1000000.
 *
 * @param[in] freq      clock frequency, in Hz, of the realtime counter
 * @param[in] usec      number of microseconds
 * @return              The number of cycles.
 *
 * @api
 */
#define US2RTC(freq, usec) (rtcnt_t)((((freq) + 999999UL) / 1000000UL) * (usec))

/**
 * @brief   Realtime counter cycles to seconds.
 * @details Converts from realtime counter cycles number to seconds.
 * @note    The result is rounded up to the next second boundary.
 * @note    The macro assumes that @p freq >= @p 1.
 *
 * @param[in] freq      clock frequency, in Hz, of the realtime counter
 * @param[in] n         number of cycles
 * @return              The number of seconds.
 *
 * @api
 */
#define RTC2S(freq, n) ((((n) - 1UL) / (freq)) + 1UL)

/**
 * @brief   Realtime counter cycles to milliseconds.
 * @details Converts from realtime counter cycles number to milliseconds.
 * @note    The result is rounded up to the next millisecond boundary.
 * @note    The macro assumes that @p freq >= @p 1000.
 *
 * @param[in] freq      clock frequency, in Hz, of the realtime counter
 * @param[in] n         number of cycles
 * @return              The number of milliseconds.
 *
 * @api
 */
#define RTC2MS(freq, n) ((((n) - 1UL) / ((freq) / 1000UL)) + 1UL)

/**
 * @brief   Realtime counter cycles to microseconds.
 * @details Converts from realtime counter cycles number to microseconds.
 * @note    The result is rounded up to the next microsecond boundary.
 * @note    The macro assumes that @p freq >= @p 1000000.
 *
 * @param[in] freq      clock frequency, in Hz, of the realtime counter
 * @param[in] n         number of cycles
 * @return              The number of microseconds.
 *
 * @api
 */
#define RTC2US(freq, n) ((((n) - 1UL) / ((freq) / 1000000UL)) + 1UL)
/** @} */

/**
 * @brief   Returns the current value of the system real time counter.
 * @note    This function is only available if the port layer supports the
 *          option @p PORT_SUPPORTS_RT.
 *
 * @return              The value of the system realtime counter of
 *                      type rtcnt_t.
 *
 * @xclass
 */
#if (PORT_SUPPORTS_RT == TRUE) || defined(__DOXYGEN__)
#define chSysGetRealtimeCounterX() (rtcnt_t)port_rt_get_counter_value()
#endif

/**
 * @brief   Performs a context switch.
 * @note    Not a user function, it is meant to be invoked by the scheduler
 *          itself or from within the port layer.
 *
 * @param[in] ntp       the thread to be switched in
 * @param[in] otp       the thread to be switched out
 *
 * @special
 */
#define chSysSwitch(ntp, otp) {                                             \
                                                                            \
  _trace_switch(ntp, otp);                                                  \
  _stats_ctxswc(ntp, otp);                                                  \
  CH_CFG_CONTEXT_SWITCH_HOOK(ntp, otp);                                     \
  port_switch(ntp, otp);                                                    \
}

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if !defined(__DOXYGEN__)
extern stkalign_t ch_idle_thread_wa[];
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void chSysInit(void);
  bool chSysIntegrityCheckI(unsigned testmask);
  void chSysTimerHandlerI(void);
  syssts_t chSysGetStatusAndLockX(void);
  void chSysRestoreStatusX(syssts_t sts);
#if PORT_SUPPORTS_RT == TRUE
  bool chSysIsCounterWithinX(rtcnt_t cnt, rtcnt_t start, rtcnt_t end);
  void chSysPolledDelayX(rtcnt_t cycles);
#endif
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

/**
 * @brief   Raises the system interrupt priority mask to the maximum level.
 * @details All the maskable interrupt sources are disabled regardless their
 *          hardware priority.
 * @note    Do not invoke this API from within a kernel lock.
 *
 * @special
 */
static inline void chSysDisable(void) {

  port_disable();
  _dbg_check_disable();
}

/**
 * @brief   Raises the system interrupt priority mask to system level.
 * @details The interrupt sources that should not be able to preempt the kernel
 *          are disabled, interrupt sources with higher priority are still
 *          enabled.
 * @note    Do not invoke this API from within a kernel lock.
 * @note    This API is no replacement for @p chSysLock(), the @p chSysLock()
 *          could do more than just disable the interrupts.
 *
 * @special
 */
static inline void chSysSuspend(void) {

  port_suspend();
  _dbg_check_suspend();
}

/**
 * @brief   Lowers the system interrupt priority mask to user level.
 * @details All the interrupt sources are enabled.
 * @note    Do not invoke this API from within a kernel lock.
 * @note    This API is no replacement for @p chSysUnlock(), the
 *          @p chSysUnlock() could do more than just enable the interrupts.
 *
 * @special
 */
static inline void chSysEnable(void) {

  _dbg_check_enable();
  port_enable();
}

/**
 * @brief   Enters the kernel lock state.
 *
 * @special
 */
static inline void chSysLock(void) {

  port_lock();
  _stats_start_measure_crit_thd();
  _dbg_check_lock();
}

/**
 * @brief   Leaves the kernel lock state.
 *
 * @special
 */
static inline void chSysUnlock(void) {

  _dbg_check_unlock();
  _stats_stop_measure_crit_thd();

  /* The following condition can be triggered by the use of i-class functions
     in a critical section not followed by a chSchResceduleS(), this means
     that the current thread has a lower priority than the next thread in
     the ready list.*/
  chDbgAssert((ch.rlist.queue.next == (thread_t *)&ch.rlist.queue) ||
              (ch.rlist.current->prio >= ch.rlist.queue.next->prio),
              "priority order violation");

  port_unlock();
}

/**
 * @brief   Enters the kernel lock state from within an interrupt handler.
 * @note    This API may do nothing on some architectures, it is required
 *          because on ports that support preemptable interrupt handlers
 *          it is required to raise the interrupt mask to the same level of
 *          the system mutual exclusion zone.<br>
 *          It is good practice to invoke this API before invoking any I-class
 *          syscall from an interrupt handler.
 * @note    This API must be invoked exclusively from interrupt handlers.
 *
 * @special
 */
static inline void chSysLockFromISR(void) {

  port_lock_from_isr();
  _stats_start_measure_crit_isr();
  _dbg_check_lock_from_isr();
}

/**
 * @brief   Leaves the kernel lock state from within an interrupt handler.
 *
 * @note    This API may do nothing on some architectures, it is required
 *          because on ports that support preemptable interrupt handlers
 *          it is required to raise the interrupt mask to the same level of
 *          the system mutual exclusion zone.<br>
 *          It is good practice to invoke this API after invoking any I-class
 *          syscall from an interrupt handler.
 * @note    This API must be invoked exclusively from interrupt handlers.
 *
 * @special
 */
static inline void chSysUnlockFromISR(void) {

  _dbg_check_unlock_from_isr();
  _stats_stop_measure_crit_isr();
  port_unlock_from_isr();
}

/**
 * @brief   Unconditionally enters the kernel lock state.
 * @note    Can be called without previous knowledge of the current lock state.
 *          The final state is "s-locked".
 *
 * @special
 */
static inline void chSysUnconditionalLock(void) {

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
static inline void chSysUnconditionalUnlock(void) {

  if (!port_irq_enabled(port_get_irq_status())) {
    chSysUnlock();
  }
}

#if (CH_CFG_NO_IDLE_THREAD == FALSE) || defined(__DOXYGEN__)
/**
 * @brief   Returns a pointer to the idle thread.
 * @pre     In order to use this function the option @p CH_CFG_NO_IDLE_THREAD
 *          must be disabled.
 * @note    The reference counter of the idle thread is not incremented but
 *          it is not strictly required being the idle thread a static
 *          object.
 *
 * @return              Pointer to the idle thread.
 *
 * @xclass
 */
static inline thread_t *chSysGetIdleThreadX(void) {

  return ch.rlist.queue.prev;
}
#endif /* CH_CFG_NO_IDLE_THREAD == FALSE */

#endif /* CHSYS_H */

/** @} */
