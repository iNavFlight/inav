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
 * @file    AVR/nilcore.h
 * @brief   AVR port macros and structures.
 *
 * @addtogroup AVR_CORE
 * @{
 */

#ifndef _NILCORE_H_
#define _NILCORE_H_

#include <avr/io.h>
#include <avr/interrupt.h>

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/**
 * @name    Architecture and Compiler
 * @{
 */
/**
 * @brief   Macro defining the port architecture.
 */
#define PORT_ARCHITECTURE_AVR

/**
 * @brief   Name of the implemented architecture.
 */
#define PORT_ARCHITECTURE_NAME          "AVR"

/**
 * @brief   Name of the architecture variant.
 */
#define PORT_CORE_VARIANT_NAME          "MegaAVR"

/**
 * @brief   Compiler name and version.
 */
#if defined(__GNUC__) || defined(__DOXYGEN__)
#define PORT_COMPILER_NAME              "GCC " __VERSION__

#else
#error "unsupported compiler"
#endif

/**
 * @brief   Port-specific information string.
 */
#define PORT_INFO                       "16 bits code addressing"

/**
 * @brief   This port supports a realtime counter.
 */
#define PORT_SUPPORTS_RT                FALSE
/** @} */

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   Stack size for the system idle thread.
 * @details This size depends on the idle thread implementation, usually
 *          the idle thread should take no more space than those reserved
 *          by @p PORT_INT_REQUIRED_STACK.
 * @note    In this port it is set to 8.
 */
#if !defined(PORT_IDLE_THREAD_STACK_SIZE) || defined(__DOXYGEN__)
#define PORT_IDLE_THREAD_STACK_SIZE     8
#endif

/**
 * @brief   Per-thread stack overhead for interrupts servicing.
 * @details This constant is used in the calculation of the correct working
 *          area size.
 * @note    In this port the default is 32 bytes per thread.
 */
#if !defined(PORT_INT_REQUIRED_STACK) || defined(__DOXYGEN__)
#define PORT_INT_REQUIRED_STACK         32
#endif

/**
 * @brief   Enables an alternative timer implementation.
 * @details Usually the port uses a timer interface defined in the file
 *          @p nilcore_timer.h, if this option is enabled then the file
 *          @p nilcore_timer_alt.h is included instead.
 */
#if !defined(PORT_USE_ALT_TIMER)
#define PORT_USE_ALT_TIMER              FALSE
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/* The following code is not processed when the file is included from an
   asm module.*/
#if !defined(_FROM_ASM_)

/**
 * @brief   Type of stack and memory alignment enforcement.
 */
typedef uint8_t stkalign_t;

/**
 * @brief   System saved context.
 * @details This structure represents the inner stack frame during a context
 *          switching.
 */
struct port_intctx {
  uint8_t       _next;
  uint8_t       r29;
  uint8_t       r28;
  uint8_t       r17;
  uint8_t       r16;
  uint8_t       r15;
  uint8_t       r14;
  uint8_t       r13;
  uint8_t       r12;
  uint8_t       r11;
  uint8_t       r10;
  uint8_t       r9;
  uint8_t       r8;
  uint8_t       r7;
  uint8_t       r6;
  uint8_t       r5;
  uint8_t       r4;
  uint8_t       r3;
  uint8_t       r2;
#ifdef __AVR_3_BYTE_PC__
  uint8_t       pcx;
#endif
  uint8_t       pcl;
  uint8_t       pch;
};

#endif /* !defined(_FROM_ASM_) */

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Platform dependent thread stack setup.
 * @details This code usually setup the context switching frame represented
 *          by an @p port_intctx structure.
 */
#ifdef __AVR_3_BYTE_PC__
#define PORT_SETUP_CONTEXT(tp, wend, pf, arg) {                             \
    (tp)->ctxp = (struct port_intctx*)(((uint8_t *)(wend)) -                \
                                         sizeof(struct port_intctx));       \
    (tp)->ctxp->r2  = (int)pf;                                              \
    (tp)->ctxp->r3  = (int)pf >> 8;                                         \
    (tp)->ctxp->r4  = (int)arg;                                             \
    (tp)->ctxp->r5  = (int)arg >> 8;                                        \
    (tp)->ctxp->pcx = (int)0;                                               \
    (tp)->ctxp->pcl = (int)_port_thread_start >> 8;                         \
    (tp)->ctxp->pch = (int)_port_thread_start;                              \
}
#else /* __AVR_3_BYTE_PC__ */
#define PORT_SETUP_CONTEXT(tp, wend, pf, arg) {                             \
    (tp)->ctxp = (struct port_intctx*)(((uint8_t *)(wend)) -                \
                                         sizeof(struct port_intctx));       \
    (tp)->ctxp->r2  = (int)pf;                                              \
    (tp)->ctxp->r3  = (int)pf >> 8;                                         \
    (tp)->ctxp->r4  = (int)arg;                                             \
    (tp)->ctxp->r5  = (int)arg >> 8;                                        \
    (tp)->ctxp->pcl = (int)_port_thread_start >> 8;                         \
    (tp)->ctxp->pch = (int)_port_thread_start;                              \
}
#endif /* __AVR_3_BYTE_PC__ */
/**
 * @brief   Computes the thread working area global size.
 * @note    There is no need to perform alignments in this macro.
 */
#define PORT_WA_SIZE(n) ((sizeof(struct port_intctx) - 1) +                 \
                         (n) + (PORT_INT_REQUIRED_STACK))

/**
 * @brief   IRQ prologue code.
 * @details This macro must be inserted at the start of all IRQ handlers
 *          enabled to invoke system APIs.
 * @note    This code tricks the compiler to save all the specified registers
 *          by "touching" them.
 */
#define PORT_IRQ_PROLOGUE() {                                               \
  asm ("" : : : "r18", "r19", "r20", "r21", "r22", "r23", "r24",            \
                "r25", "r26", "r27", "r30", "r31");                         \
}

/**
 * @brief   IRQ epilogue code.
 * @details This macro must be inserted at the end of all IRQ handlers
 *          enabled to invoke system APIs.
 */
#define PORT_IRQ_EPILOGUE() chSchRescheduleS()

/**
 * @brief   IRQ handler function declaration.
 * @note    @p id can be a function name or a vector number depending on the
 *          port implementation.
 */
#define PORT_IRQ_HANDLER(id) ISR(id)

/**
 * @brief   Fast IRQ handler function declaration.
 * @note    @p id can be a function name or a vector number depending on the
 *          port implementation.
 */
#define PORT_FAST_IRQ_HANDLER(id) ISR(id)

/**
 * @brief   Performs a context switch between two threads.
 * @details This is the most critical code in any port, this function
 *          is responsible for the context switch between 2 threads.
 * @note    The implementation of this code affects <b>directly</b> the context
 *          switch performance so optimize here as much as you can.
 *
 * @param[in] ntp       the thread to be switched in
 * @param[in] otp       the thread to be switched out
 */
#define port_switch(ntp, otp) _port_switch(ntp, otp)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

/* The following code is not processed when the file is included from an
   asm module.*/
#if !defined(_FROM_ASM_)

#ifdef __cplusplus
extern "C" {
#endif
  void _port_irq_epilogue(void);
  void _port_switch(thread_t *ntp, thread_t *otp);
  void _port_thread_start(void);
#ifdef __cplusplus
}
#endif

#endif /* !defined(_FROM_ASM_) */

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

/* The following code is not processed when the file is included from an
   asm module.*/
#if !defined(_FROM_ASM_)

/**
 * @brief   Port-related initialization code.
 */
static inline void port_init(void) {

}

/**
 * @brief   Returns a word encoding the current interrupts status.
 *
 * @return              The interrupts status.
 */
static inline syssts_t port_get_irq_status(void) {

  return 0;
}

/**
 * @brief   Checks the interrupt status.
 *
 * @param[in] sts       the interrupt status word
 *
 * @return              The interrupt status.
 * @retvel false        the word specified a disabled interrupts status.
 * @retvel true         the word specified an enabled interrupts status.
 */
static inline bool port_irq_enabled(syssts_t sts) {

  return false;
}

/**
 * @brief   Determines the current execution context.
 *
 * @return              The execution context.
 * @retval false        not running in ISR mode.
 * @retval true         running in ISR mode.
 */
static inline bool port_is_isr_context(void) {

  return false;
}

/**
 * @brief   Kernel-lock action.
 */
static inline void port_lock(void) {

  asm volatile ("cli" : : : "memory");
}

/**
 * @brief   Kernel-unlock action.
 */
static inline void port_unlock(void) {

  asm volatile ("sei" : : : "memory");
}

/**
 * @brief   Kernel-lock action from an interrupt handler.
 * @note    This function is empty in this port.
 */
static inline void port_lock_from_isr(void) {

}

/**
 * @brief   Kernel-unlock action from an interrupt handler.
 * @note    This function is empty in this port.
 */
static inline void port_unlock_from_isr(void) {

}

/**
 * @brief   Disables all the interrupt sources.
 */
static inline void port_disable(void) {

  asm volatile ("cli" : : : "memory");
}

/**
 * @brief   Disables the interrupt sources below kernel-level priority.
 */
static inline void port_suspend(void) {

  asm volatile ("cli" : : : "memory");
}

/**
 * @brief   Enables all the interrupt sources.
 */
static inline void port_enable(void) {

  asm volatile ("sei" : : : "memory");
}

/**
 * @brief   Enters an architecture-dependent IRQ-waiting mode.
 * @details The function is meant to return when an interrupt becomes pending.
 *          The simplest implementation is an empty function or macro but this
 *          would not take advantage of architecture-specific power saving
 *          modes.
 */
static inline void port_wait_for_interrupt(void) {

  asm volatile ("sleep" : : : "memory");
}

/**
 * @brief   Returns the current value of the realtime counter.
 *
 * @return              The realtime counter value.
 */
static inline rtcnt_t port_rt_get_counter_value(void) {

  return 0;
}

#endif /* !defined(_FROM_ASM_) */

/*===========================================================================*/
/* Module late inclusions.                                                   */
/*===========================================================================*/

#if !defined(_FROM_ASM_)

#if NIL_CFG_ST_TIMEDELTA > 0
#if !PORT_USE_ALT_TIMER
#include "nilcore_timer.h"
#else /* PORT_USE_ALT_TIMER */
#include "nilcore_timer_alt.h"
#endif /* PORT_USE_ALT_TIMER */
#endif /* NIL_CFG_ST_TIMEDELTA > 0 */

#endif /* !defined(_FROM_ASM_) */

#endif /* _NILCORE_H_ */

/** @} */
