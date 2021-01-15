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
 * @file    chtrace.h
 * @brief   Tracer macros and structures.
 *
 * @addtogroup trace
 * @{
 */

#ifndef CHTRACE_H
#define CHTRACE_H

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/**
 * @name    Trace record types
 * @{
 */
#define CH_TRACE_TYPE_UNUSED                0U
#define CH_TRACE_TYPE_SWITCH                1U
#define CH_TRACE_TYPE_ISR_ENTER             2U
#define CH_TRACE_TYPE_ISR_LEAVE             3U
#define CH_TRACE_TYPE_HALT                  4U
#define CH_TRACE_TYPE_USER                  5U
/** @} */

/**
 * @name    Events to trace
 * @{
 */
#define CH_DBG_TRACE_MASK_DISABLED          255U
#define CH_DBG_TRACE_MASK_NONE              0U
#define CH_DBG_TRACE_MASK_SWITCH            1U
#define CH_DBG_TRACE_MASK_ISR               2U
#define CH_DBG_TRACE_MASK_HALT              4U
#define CH_DBG_TRACE_MASK_USER              8U
#define CH_DBG_TRACE_MASK_SLOW              (CH_DBG_TRACE_MASK_SWITCH |     \
                                             CH_DBG_TRACE_MASK_HALT |       \
                                             CH_DBG_TRACE_MASK_USER)
#define CH_DBG_TRACE_MASK_ALL               (CH_DBG_TRACE_MASK_SWITCH |     \
                                             CH_DBG_TRACE_MASK_ISR |        \
                                             CH_DBG_TRACE_MASK_HALT |       \
                                             CH_DBG_TRACE_MASK_USER)
/** @} */

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Debug related settings
 * @{
 */
/**
 * @brief   Trace buffer entries.
 */
#if !defined(CH_DBG_TRACE_MASK) || defined(__DOXYGEN__)
#define CH_DBG_TRACE_MASK                   CH_DBG_TRACE_MASK_DISABLED
#endif

/**
 * @brief   Trace buffer entries.
 * @note    The trace buffer is only allocated if @p CH_DBG_TRACE_MASK is
 *          different from @p CH_DBG_TRACE_MASK_DISABLED.
 */
#if !defined(CH_DBG_TRACE_BUFFER_SIZE) || defined(__DOXYGEN__)
#define CH_DBG_TRACE_BUFFER_SIZE            128
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

#if (CH_DBG_TRACE_MASK != CH_DBG_TRACE_MASK_DISABLED) || defined(__DOXYGEN__)
/*lint -save -e46 [6.1] An uint32_t is required.*/
/**
 * @brief   Trace buffer record.
 */
typedef struct {
  /**
   * @brief   Record type.
   */
  uint32_t              type:3;
  /**
   * @brief   Switched out thread state.
   */
  uint32_t              state:5;
  /**
   * @brief   Accurate time stamp.
   * @note    This field only available if the post supports
   *          @p PORT_SUPPORTS_RT else it is set to zero.
   */
  uint32_t              rtstamp:24;
  /**
   * @brief   System time stamp of the switch event.
   */
  systime_t             time;
  union {
    /**
     * @brief   Structure representing a  context switch.
     */
    struct {
      /**
       * @brief   Switched in thread.
       */
      thread_t              *ntp;
      /**
       * @brief   Object where going to sleep.
       */
      void                  *wtobjp;
    } sw;
    /**
     * @brief   Structure representing an ISR enter.
     */
    struct {
      /**
       * @brief   ISR function name taken using @p __func__.
       */
      const char            *name;
    } isr;
    /**
     * @brief   Structure representing an halt.
     */
    struct {
      /**
       * @brief   Halt error string.
       */
      const char            *reason;
    } halt;
    /**
     * @brief   User trace structure.
     */
    struct {
      /**
       * @brief   Trace user parameter 1.
       */
      void                  *up1;
      /**
       * @brief   Trace user parameter 2.
       */
      void                  *up2;
    } user;
  } u;
} ch_trace_event_t;
/*lint -restore*/

/**
 * @brief   Trace buffer header.
 */
typedef struct {
  /**
   * @brief   Suspended trace sources mask.
   */
  uint16_t              suspended;
  /**
   * @brief   Trace buffer size (entries).
   */
  uint16_t              size;
  /**
   * @brief   Pointer to the buffer front.
   */
  ch_trace_event_t      *ptr;
  /**
   * @brief   Ring buffer.
   */
  ch_trace_event_t      buffer[CH_DBG_TRACE_BUFFER_SIZE];
} ch_trace_buffer_t;
#endif /* CH_DBG_TRACE_MASK != CH_DBG_TRACE_MASK_DISABLED */

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/* When a trace feature is disabled the associated functions are replaced by
   an empty macro. Note that the macros can be externally redefined in
   order to interface 3rd parties tracing tools.*/
#if CH_DBG_TRACE_MASK == CH_DBG_TRACE_MASK_DISABLED
#if !defined(_trace_init)
#define _trace_init()
#endif
#if !defined(_trace_switch)
#define _trace_switch(ntp, otp)
#endif
#if !defined(_trace_isr_enter)
#define _trace_isr_enter(isr)
#endif
#if !defined(_trace_isr_leave)
#define _trace_isr_leave(isr)
#endif
#if !defined(_trace_halt)
#define _trace_halt(reason)
#endif
#if !defined(chDbgWriteTraceI)
#define chDbgWriteTraceI(up1, up2)
#endif
#if !defined(chDbgWriteTrace)
#define chDbgWriteTrace(up1, up2)
#endif
#endif /* CH_DBG_TRACE_MASK == CH_DBG_TRACE_MASK_DISABLED */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
#if (CH_DBG_TRACE_MASK != CH_DBG_TRACE_MASK_DISABLED) || defined(__DOXYGEN__)
  void _trace_init(void);
  void _trace_switch(thread_t *ntp, thread_t *otp);
  void _trace_isr_enter(const char *isr);
  void _trace_isr_leave(const char *isr);
  void _trace_halt(const char *reason);
  void chDbgWriteTraceI(void *up1, void *up2);
  void chDbgWriteTrace(void *up1, void *up2);
  void chDbgSuspendTraceI(uint16_t mask);
  void chDbgSuspendTrace(uint16_t mask);
  void chDbgResumeTraceI(uint16_t mask);
  void chDbgResumeTrace(uint16_t mask);
#endif /* CH_DBG_TRACE_MASK != CH_DBG_TRACE_MASK_DISABLED */
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

#endif /* CHTRACE_H */

/** @} */
