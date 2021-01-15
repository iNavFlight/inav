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
 * @file    chtrace.c
 * @brief   Tracer code.
 *
 * @addtogroup trace
 * @details System events tracing service.
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

#if (CH_DBG_TRACE_MASK != CH_DBG_TRACE_MASK_DISABLED) || defined(__DOXYGEN__)
/**
 * @brief   Writes a time stamp and increases the trace buffer pointer.
 *
 * @notapi
 */
static NOINLINE void trace_next(void) {

  ch.dbg.trace_buffer.ptr->time    = chVTGetSystemTimeX();
#if PORT_SUPPORTS_RT == TRUE
  ch.dbg.trace_buffer.ptr->rtstamp = chSysGetRealtimeCounterX();
#else
  ch.dbg.trace_buffer.ptr->rtstamp = (rtcnt_t)0;
#endif

  /* Trace hook, useful in order to interface debug tools.*/
  CH_CFG_TRACE_HOOK(ch.dbg.trace_buffer.ptr);

  if (++ch.dbg.trace_buffer.ptr >=
      &ch.dbg.trace_buffer.buffer[CH_DBG_TRACE_BUFFER_SIZE]) {
    ch.dbg.trace_buffer.ptr = &ch.dbg.trace_buffer.buffer[0];
  }
}
#endif

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

#if (CH_DBG_TRACE_MASK != CH_DBG_TRACE_MASK_DISABLED) || defined(__DOXYGEN__)
/**
 * @brief   Trace circular buffer subsystem initialization.
 * @note    Internal use only.
 */
void _trace_init(void) {
  unsigned i;

  ch.dbg.trace_buffer.suspended = (uint16_t)~CH_DBG_TRACE_MASK;
  ch.dbg.trace_buffer.size      = CH_DBG_TRACE_BUFFER_SIZE;
  ch.dbg.trace_buffer.ptr       = &ch.dbg.trace_buffer.buffer[0];
  for (i = 0U; i < (unsigned)CH_DBG_TRACE_BUFFER_SIZE; i++) {
    ch.dbg.trace_buffer.buffer[i].type = CH_TRACE_TYPE_UNUSED;
  }
}

/**
 * @brief   Inserts in the circular debug trace buffer a context switch record.
 *
 * @param[in] ntp       the thread being switched in
 * @param[in] otp       the thread being switched out
 *
 * @notapi
 */
void _trace_switch(thread_t *ntp, thread_t *otp) {

  (void)ntp;

  if ((ch.dbg.trace_buffer.suspended & CH_DBG_TRACE_MASK_SWITCH) == 0U) {
    ch.dbg.trace_buffer.ptr->type        = CH_TRACE_TYPE_SWITCH;
    ch.dbg.trace_buffer.ptr->state       = (uint8_t)otp->state;
    ch.dbg.trace_buffer.ptr->u.sw.ntp    = currp;
    ch.dbg.trace_buffer.ptr->u.sw.wtobjp = otp->u.wtobjp;
    trace_next();
  }
}

/**
 * @brief   Inserts in the circular debug trace buffer an ISR-enter record.
 *
 * @param[in] isr       name of the isr
 *
 * @notapi
 */
void _trace_isr_enter(const char *isr) {

  if ((ch.dbg.trace_buffer.suspended & CH_DBG_TRACE_MASK_ISR) == 0U) {
    port_lock_from_isr();
    ch.dbg.trace_buffer.ptr->type        = CH_TRACE_TYPE_ISR_ENTER;
    ch.dbg.trace_buffer.ptr->state       = 0U;
    ch.dbg.trace_buffer.ptr->u.isr.name  = isr;
    trace_next();
    port_unlock_from_isr();
  }
}

/**
 * @brief   Inserts in the circular debug trace buffer an ISR-leave record.
 *
 * @param[in] isr       name of the isr
 *
 * @notapi
 */
void _trace_isr_leave(const char *isr) {

  if ((ch.dbg.trace_buffer.suspended & CH_DBG_TRACE_MASK_ISR) == 0U) {
    port_lock_from_isr();
    ch.dbg.trace_buffer.ptr->type        = CH_TRACE_TYPE_ISR_LEAVE;
    ch.dbg.trace_buffer.ptr->state       = 0U;
    ch.dbg.trace_buffer.ptr->u.isr.name  = isr;
    trace_next();
    port_unlock_from_isr();
  }
}

/**
 * @brief   Inserts in the circular debug trace buffer an halt record.
 *
 * @param[in] reason    the halt error string
 *
 * @notapi
 */
void _trace_halt(const char *reason) {

  if ((ch.dbg.trace_buffer.suspended & CH_DBG_TRACE_MASK_HALT) == 0U) {
    ch.dbg.trace_buffer.ptr->type          = CH_TRACE_TYPE_HALT;
    ch.dbg.trace_buffer.ptr->state         = 0;
    ch.dbg.trace_buffer.ptr->u.halt.reason = reason;
    trace_next();
  }
}

/**
 * @brief   Adds an user trace record to the trace buffer.
 *
 * @param[in] up1       user parameter 1
 * @param[in] up2       user parameter 2
 *
 * @iclass
 */
void chDbgWriteTraceI(void *up1, void *up2) {

  chDbgCheckClassI();

  if ((ch.dbg.trace_buffer.suspended & CH_DBG_TRACE_MASK_USER) == 0U) {
    ch.dbg.trace_buffer.ptr->type       = CH_TRACE_TYPE_USER;
    ch.dbg.trace_buffer.ptr->state      = 0;
    ch.dbg.trace_buffer.ptr->u.user.up1 = up1;
    ch.dbg.trace_buffer.ptr->u.user.up2 = up2;
    trace_next();
  }
}

/**
 * @brief   Adds an user trace record to the trace buffer.
 *
 * @param[in] up1       user parameter 1
 * @param[in] up2       user parameter 2
 *
 * @api
 */
void chDbgWriteTrace(void *up1, void *up2) {

  chSysLock();
  chDbgWriteTraceI(up1, up2);
  chSysUnlock();
}

/**
 * @brief   Suspends one or more trace events.
 *
 * @param[in] mask      mask of the trace events to be suspended
 *
 * @iclass
 */
void chDbgSuspendTraceI(uint16_t mask) {

  chDbgCheckClassI();

  ch.dbg.trace_buffer.suspended |= mask;
}

/**
 * @brief   Suspends one or more trace events.
 *
 * @param[in] mask      mask of the trace events to be suspended
 *
 * @api
 */
void chDbgSuspendTrace(uint16_t mask) {

  chSysLock();
  chDbgSuspendTraceI(mask);
  chSysUnlock();
}

/**
 * @brief   Resumes one or more trace events.
 *
 * @param[in] mask      mask of the trace events to be resumed
 *
 * @iclass
 */
void chDbgResumeTraceI(uint16_t mask) {

  chDbgCheckClassI();

  ch.dbg.trace_buffer.suspended &= ~mask;
}

/**
 * @brief   Resumes one or more trace events.
 *
 * @param[in] mask      mask of the trace events to be resumed
 *
 * @api
 */
void chDbgResumeTrace(uint16_t mask) {

  chSysLock();
  chDbgResumeTraceI(mask);
  chSysUnlock();
}
#endif /* CH_DBG_TRACE_MASK != CH_DBG_TRACE_MASK_DISABLED */

/** @} */
