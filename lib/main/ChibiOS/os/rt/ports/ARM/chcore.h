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
 * @file    ARM/chcore.h
 * @brief   ARM7/9 architecture port macros and structures.
 *
 * @addtogroup ARM_CORE
 * @{
 */

#ifndef _CHCORE_H_
#define _CHCORE_H_

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/**
 * @name    Architecture and Compiler
 * @{
 */
/**
 * @brief   Macro defining a generic ARM architecture.
 */
#define PORT_ARCHITECTURE_ARM

/* The following code is not processed when the file is included from an
   asm module because those intrinsic macros are not necessarily defined
   by the assembler too.*/
#if !defined(_FROM_ASM_)

/**
 * @brief   Compiler name and version.
 */
#if defined(__GNUC__) || defined(__DOXYGEN__)
#define PORT_COMPILER_NAME              "GCC " __VERSION__

#else
#error "unsupported compiler"
#endif

#endif /* !defined(_FROM_ASM_) */
/** @} */

/**
 * @name    ARM variants
 * @{
 */
#define ARM_CORE_ARM7TDMI               7
#define ARM_CORE_ARM9                   9
#define ARM_CORE_CORTEX_A8              108
#define ARM_CORE_CORTEX_A9              109
/** @} */

/* Inclusion of the ARM implementation specific parameters.*/
#include "armparams.h"

/**
 * @brief   This port supports a realtime counter.
 */
#define PORT_SUPPORTS_RT                FALSE

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   Enables an alternative timer implementation.
 * @details Usually the port uses a timer interface defined in the file
 *          @p chcore_timer.h, if this option is enabled then the file
 *          @p chcore_timer_alt.h is included instead.
 */
#if !defined(PORT_USE_ALT_TIMER)
#define PORT_USE_ALT_TIMER              FALSE
#endif

/**
 * @brief   Stack size for the system idle thread.
 * @details This size depends on the idle thread implementation, usually
 *          the idle thread should take no more space than those reserved
 *          by @p PORT_INT_REQUIRED_STACK.
 * @note    In this port it is set to 32 because the idle thread does have
 *          a stack frame when compiling without optimizations. You may
 *          reduce this value to zero when compiling with optimizations.
 */
#if !defined(PORT_IDLE_THREAD_STACK_SIZE) || defined(__DOXYGEN__)
#define PORT_IDLE_THREAD_STACK_SIZE     32
#endif

/**
 * @brief   Per-thread stack overhead for interrupts servicing.
 * @details This constant is used in the calculation of the correct working
 *          area size.
 */
#if !defined(PORT_INT_REQUIRED_STACK) || defined(__DOXYGEN__)
#define PORT_INT_REQUIRED_STACK         32
#endif

/**
 * @brief   If enabled allows the idle thread to enter a low power mode.
 */
#ifndef ARM_ENABLE_WFI_IDLE
#define ARM_ENABLE_WFI_IDLE             FALSE
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/* The following code is not processed when the file is included from an
   asm module.*/
#if !defined(_FROM_ASM_)

/* ARM core check.*/
#if (ARM_CORE == ARM_CORE_ARM7TDMI) || defined(__DOXYGEN__)
#define PORT_ARCHITECTURE_ARM_ARM7
#define PORT_ARCHITECTURE_NAME          "ARMv4T"
#define PORT_CORE_VARIANT_NAME          "ARM7"

#elif ARM_CORE == ARM_CORE_ARM9
#define PORT_ARCHITECTURE_ARM_ARM9
#define PORT_ARCHITECTURE_NAME          "ARMv5T"
#define PORT_CORE_VARIANT_NAME          "ARM9"

#elif ARM_CORE == ARM_CORE_CORTEX_A8
#define PORT_ARCHITECTURE_ARM_CORTEXA8
#define PORT_ARCHITECTURE_NAME          "ARMv7"
#define PORT_CORE_VARIANT_NAME          "ARM Cortex-A8"

#elif ARM_CORE == ARM_CORE_CORTEX_A9
#define PORT_ARCHITECTURE_ARM_CORTEXA9
#define PORT_ARCHITECTURE_NAME          "ARMv7"
#define PORT_CORE_VARIANT_NAME          "ARM Cortex-A9"

#else
#error "unknown or unsupported ARM core"
#endif

#if defined(THUMB_PRESENT)
#if defined(THUMB_NO_INTERWORKING)
#define PORT_INFO                       "Pure THUMB mode"
#else
#define PORT_INFO                       "Interworking mode"
#endif
#else
#define PORT_INFO                       "Pure ARM mode"
#endif

#endif /* !defined(_FROM_ASM_) */

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/* The following code is not processed when the file is included from an
   asm module.*/
#if !defined(_FROM_ASM_)

/**
 * @brief   Type of stack and memory alignment enforcement.
 * @note    In this architecture the stack alignment is enforced to 64 bits.
 */
typedef uint64_t stkalign_t;

/**
 * @brief   Generic ARM register.
 */
typedef void *regarm_t;

/**
 * @brief   Interrupt saved context.
 * @details This structure represents the stack frame saved during an
 *          interrupt handler.
 */
struct port_extctx {
  regarm_t              spsr_irq;
  regarm_t              lr_irq;
  regarm_t              r0;
  regarm_t              r1;
  regarm_t              r2;
  regarm_t              r3;
  regarm_t              r12;
  regarm_t              lr_usr;
};

/**
 * @brief   System saved context.
 * @details This structure represents the inner stack frame during a context
 *          switch.
 */
struct port_intctx {
  regarm_t              r4;
  regarm_t              r5;
  regarm_t              r6;
  regarm_t              r7;
  regarm_t              r8;
  regarm_t              r9;
  regarm_t              r10;
  regarm_t              r11;
  regarm_t              lr;
};

/**
 * @brief   Platform dependent part of the @p thread_t structure.
 * @details In this port the structure just holds a pointer to the
 *          @p port_intctx structure representing the stack pointer
 *          at context switch time.
 */
struct context {
  struct port_intctx    *r13;
};

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Platform dependent part of the @p chThdCreateI() API.
 * @details This code usually setup the context switching frame represented
 *          by an @p port_intctx structure.
 */
#define PORT_SETUP_CONTEXT(tp, workspace, wsize, pf, arg) {                 \
  (tp)->p_ctx.r13 = (struct port_intctx *)((uint8_t *)(workspace) +         \
                                           (wsize) -                        \
                                           sizeof(struct port_intctx));     \
  (tp)->p_ctx.r13->r4 = (regarm_t)(pf);                                     \
  (tp)->p_ctx.r13->r5 = (regarm_t)(arg);                                    \
  (tp)->p_ctx.r13->lr = (regarm_t)(_port_thread_start);                     \
}

/**
 * @brief   Computes the thread working area global size.
 * @note    There is no need to perform alignments in this macro.
 */
#define PORT_WA_SIZE(n) (sizeof(struct port_intctx) +                       \
                         sizeof(struct port_extctx) +                       \
                         ((size_t)(n)) + ((size_t)(PORT_INT_REQUIRED_STACK)))

/**
 * @brief   Priority level verification macro.
 * @todo    Add the required parameters to armparams.h.
 */
#define PORT_IRQ_IS_VALID_PRIORITY(n) false

/**
 * @brief   IRQ prologue code.
 * @details This macro must be inserted at the start of all IRQ handlers
 *          enabled to invoke system APIs.
 */
#define PORT_IRQ_PROLOGUE()

/**
 * @brief   IRQ epilogue code.
 * @details This macro must be inserted at the end of all IRQ handlers
 *          enabled to invoke system APIs.
 */
#define PORT_IRQ_EPILOGUE() return chSchIsPreemptionRequired()

/**
 * @brief   IRQ handler function declaration.
 * @note    @p id can be a function name or a vector number depending on the
 *          port implementation.
 */
#define PORT_IRQ_HANDLER(id) bool id(void)

/**
 * @brief   Fast IRQ handler function declaration.
 * @note    @p id can be a function name or a vector number depending on the
 *          port implementation.
 */
#define PORT_FAST_IRQ_HANDLER(id)                                           \
  __attribute__((interrupt("FIQ"))) void id(void)

/**
 * @brief   Performs a context switch between two threads.
 * @details This is the most critical code in any port, this function
 *          is responsible for the context switch between 2 threads.
 * @note    The implementation of this code affects <b>directly</b> the context
 *          switch performance so optimize here as much as you can.
 * @note    Implemented as inlined code for performance reasons.
 *
 * @param[in] ntp       the thread to be switched in
 * @param[in] otp       the thread to be switched out
 */
#if defined(THUMB)

#if CH_DBG_ENABLE_STACK_CHECK == TRUE
#define port_switch(ntp, otp) {                                             \
  register struct port_intctx *r13 asm ("r13");                             \
  if ((stkalign_t *)(r13 - 1) < otp->p_stklimit)                            \
    chSysHalt("stack overflow");                                            \
  _port_switch_thumb(ntp, otp);                                             \
}
#else
#define port_switch(ntp, otp) _port_switch_thumb(ntp, otp)
#endif

#else /* !defined(THUMB) */

#if CH_DBG_ENABLE_STACK_CHECK == TRUE
#define port_switch(ntp, otp) {                                             \
  register struct port_intctx *r13 asm ("r13");                             \
  if ((stkalign_t *)(r13 - 1) < otp->p_stklimit)                            \
  chSysHalt("stack overflow");                                              \
  _port_switch_arm(ntp, otp);                                               \
}
#else
#define port_switch(ntp, otp) _port_switch_arm(ntp, otp)
#endif

#endif /* !defined(THUMB) */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
#if defined(THUMB_PRESENT)
  syssts_t _port_get_cpsr(void);
#endif
#if defined(THUMB)
  void _port_switch_thumb(thread_t *ntp, thread_t *otp);
#else
  void _port_switch_arm(thread_t *ntp, thread_t *otp);
#endif
  void _port_thread_start(void);
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

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
  syssts_t sts;

#if defined(THUMB)
  sts = _port_get_cpsr();
#else
  __asm volatile ("mrs     %[p0], CPSR" : [p0] "=r" (sts) :);
#endif
  /*lint -save -e530 [9.1] Asm instruction not seen by lint.*/
  return sts;
  /*lint -restore*/
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

  return (sts & (syssts_t)0x80) == (syssts_t)0;
}

/**
 * @brief   Determines the current execution context.
 *
 * @return              The execution context.
 * @retval false        not running in ISR mode.
 * @retval true         running in ISR mode.
 */
static inline bool port_is_isr_context(void) {
  syssts_t sts;

#if defined(THUMB)
  sts = _port_get_cpsr();
#else
  __asm volatile ("mrs     %[p0], CPSR" : [p0] "=r" (sts) :);
#endif

  /*lint -save -e530 [9.1] Asm instruction not seen by lint.*/
  return (sts & (syssts_t)0x1F) == (syssts_t)0x12;
  /*lint -restore*/
}

/**
 * @brief   Kernel-lock action.
 * @details In this port it disables the IRQ sources and keeps FIQ sources
 *          enabled.
 */
static inline void port_lock(void) {

#if defined(THUMB)
  __asm volatile ("bl     _port_lock_thumb" : : : "r3", "lr", "memory");
#else
  __asm volatile ("msr     CPSR_c, #0x9F" : : : "memory");
#endif
}

/**
 * @brief   Kernel-unlock action.
 * @details In this port it enables both the IRQ and FIQ sources.
 */
static inline void port_unlock(void) {

#if defined(THUMB)
  __asm volatile ("bl     _port_unlock_thumb" : : : "r3", "lr", "memory");
#else
  __asm volatile ("msr     CPSR_c, #0x1F" : : : "memory");
#endif
}

/**
 * @brief   Kernel-lock action from an interrupt handler.
 * @note    Empty in this port.
 */
static inline void port_lock_from_isr(void) {

}

/**
 * @brief   Kernel-unlock action from an interrupt handler.
 * @note    Empty in this port.
 */
static inline void port_unlock_from_isr(void) {

}

/**
 * @brief   Disables all the interrupt sources.
 * @details In this port it disables both the IRQ and FIQ sources.
 * @note    Implements a workaround for spurious interrupts taken from the NXP
 *          LPC214x datasheet.
 */
static inline void port_disable(void) {

#if defined(THUMB)
  __asm volatile ("bl     _port_disable_thumb" : : : "r3", "lr", "memory");
#else
  __asm volatile ("mrs     r3, CPSR                       \n\t"
                  "orr     r3, #0x80                      \n\t"
                  "msr     CPSR_c, r3                     \n\t"
                  "orr     r3, #0x40                      \n\t"
                  "msr     CPSR_c, r3" : : : "r3", "memory");
#endif
}

/**
 * @brief   Disables the interrupt sources below kernel-level priority.
 * @note    Interrupt sources above kernel level remains enabled.
 * @note    In this port it disables the IRQ sources and enables the
 *          FIQ sources.
 */
static inline void port_suspend(void) {

#if defined(THUMB)
  __asm volatile ("bl     _port_suspend_thumb" : : : "r3", "lr", "memory");
#else
  __asm volatile ("msr     CPSR_c, #0x9F" : : : "memory");
#endif
}

/**
 * @brief   Enables all the interrupt sources.
 * @note    In this port it enables both the IRQ and FIQ sources.
 */
static inline void port_enable(void) {

#if defined(THUMB)
  __asm volatile ("bl     _port_enable_thumb" : : : "r3", "lr", "memory");
#else
  __asm volatile ("msr     CPSR_c, #0x1F" : : : "memory");
#endif
}

/**
 * @brief   Enters an architecture-dependent IRQ-waiting mode.
 * @details The function is meant to return when an interrupt becomes pending.
 *          The simplest implementation is an empty function or macro but this
 *          would not take advantage of architecture-specific power saving
 *          modes.
 * @note    Implemented as an inlined @p WFI instruction.
 */
static inline void port_wait_for_interrupt(void) {

#if ARM_ENABLE_WFI_IDLE == TRUE
  ARM_WFI_IMPL;
#endif
}

#if CH_CFG_ST_TIMEDELTA > 0
#if PORT_USE_ALT_TIMER == FALSE
#include "chcore_timer.h"
#else /* PORT_USE_ALT_TIMER */
#include "chcore_timer_alt.h"
#endif /* PORT_USE_ALT_TIMER */
#endif /* CH_CFG_ST_TIMEDELTA > 0 */

#endif /* !defined(_FROM_ASM_) */

#endif /* _CHCORE_H_ */

/** @} */
