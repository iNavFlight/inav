/*
    ChibiOS/HAL - Copyright (C) 2016 Andrew Wygle aka awygle

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
 * @file    MSP430X/nilcore.h
 * @brief   MSP430X port macros and structures.
 *
 * @addtogroup MSP430X_CORE
 * @{
 */

#ifndef CHCORE_H
#define CHCORE_H

#include <msp430.h>
#include <in430.h>

extern bool __msp430x_in_isr;

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
#define PORT_ARCHITECTURE_MSP430X

/**
 * @brief   Name of the implemented architecture.
 */
#define PORT_ARCHITECTURE_NAME          "MSP430X"

/**
 * @brief   Name of the architecture variant.
 */
#define PORT_CORE_VARIANT_NAME          "MSP430Xv2"

/* The following code is not processed when the file is included from an
 * asm module because those intrinsic macrosa re not necessarily defined
 * by the assembler too.*/
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
typedef uint16_t stkalign_t;

/**
 * @brief   Type of natural register size - depends on memory model.
 */
#if defined(__MSP430X_LARGE__)
typedef unsigned __int20 reg_t;
#else
typedef uint16_t reg_t;
#endif

/**
 * @brief   Natural alignment constant.
 * @note    It is the minimum alignment for pointer-size variables.
 */
#define PORT_NATURAL_ALIGN              2U

/**
 * @brief   Stack alignment constant.
 * @note    It is the alignement required for the stack pointer.
 */
#define PORT_STACK_ALIGN                2U

/**
 * @brief   Working Areas alignment constant.
 * @note    It is the alignment to be enforced for thread working areas.
 */
#define PORT_WORKING_AREA_ALIGN         2U
/** @} */

/**
 * @brief   System saved context.
 * @details This structure represents the inner stack frame during a context
 *          switching.
 */
struct port_intctx {
  reg_t       r4;
  reg_t       r5;
  reg_t       r6;
  reg_t       r7;
  reg_t       r8;
  reg_t       r9;
  reg_t       r10;
  reg_t       r0; /* program counter */
};

/**
 * @brief   Platform dependent part of the @p thread_t structure.
 * @details This structure usually contains just the saved stack pointer
 *          defined as a pointer to a @p port_intctx structure.
 */
struct port_context {
  struct port_intctx *sp;
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
#define PORT_SETUP_CONTEXT(tp, wbase, wtop, pf, arg) {                        \
    (tp)->ctx.sp = (struct port_intctx*)(((uint8_t *)(wtop)) -                \
                                         sizeof(struct port_intctx));         \
    (tp)->ctx.sp->r4  = (reg_t)pf;                                            \
    (tp)->ctx.sp->r5  = (reg_t)arg;                                           \
    (tp)->ctx.sp->r0  = (reg_t)_port_thread_start;                            \
}

/**
 * @brief   Static working area allocation.
 * @details This macro is used to allocate a static thread working area
 *          aligned as both position and size.
 *
 * @param[in] s         the name to be assigned to the stack array
 * @param[in] n         the stack size to be assigned to the thread
 */
#define PORT_WORKING_AREA(s, n)                                               \
  stkalign_t s[THD_WORKING_AREA_SIZE(n) / sizeof (stkalign_t)]

/**
 * @brief   Computes the thread working area global size.
 * @note    There is no need to perform alignments in this macro.
 */
#define PORT_WA_SIZE(n) ((sizeof(struct port_intctx) - 1) +                   \
                         (n) + (PORT_INT_REQUIRED_STACK))

/**
 * @brief   IRQ prologue code.
 * @details This macro must be inserted at the start of all IRQ handlers
 *          enabled to invoke system APIs.
 */
#define PORT_IRQ_PROLOGUE() __msp430x_in_isr = true;

/**
 * @brief   IRQ epilogue code.
 * @details This macro must be inserted at the end of all IRQ handlers
 *          enabled to invoke system APIs.
 */
#define PORT_IRQ_EPILOGUE() {                                               \
  __msp430x_in_isr = false;                                                 \
  _dbg_check_lock();                                                        \
  if (chSchIsPreemptionRequired())                                          \
    chSchDoReschedule();                                                    \
  _dbg_check_unlock();                                                      \
}

/**
 * @brief   IRQ handler function declaration.
 * @note    @p id can be a function name or a vector number depending on the
 *          port implementation.
 */
#define PORT_IRQ_HANDLER(id) __attribute__ ((interrupt(id)))                \
  void ISR_ ## id (void)

/**
 * @brief   Fast IRQ handler function declaration.
 * @note    @p id can be a function name or a vector number depending on the
 *          port implementation.
 */
#define PORT_FAST_IRQ_HANDLER(id) PORT_IRQ_HANDLER(id)

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
  __msp430x_in_isr = false;
}

/**
 * @brief   Returns a word encoding the current interrupts status.
 *
 * @return              The interrupts status.
 */
static inline syssts_t port_get_irq_status(void) {

  return __get_SR_register();
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

  return sts & GIE;
}

/**
 * @brief   Determines the current execution context.
 *
 * @return              The execution context.
 * @retval false        not running in ISR mode.
 * @retval true         running in ISR mode.
 */
static inline bool port_is_isr_context(void) {
  return __msp430x_in_isr;
}

/**
 * @brief   Kernel-lock action.
 */
static inline void port_lock(void) {

  _disable_interrupts();
  asm volatile("nop");
}

/**
 * @brief   Kernel-unlock action.
 */
static inline void port_unlock(void) {
  asm volatile("nop");
  _enable_interrupts();
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

  _disable_interrupts();
  asm volatile("nop");
}

/**
 * @brief   Disables the interrupt sources below kernel-level priority.
 */
static inline void port_suspend(void) {

  _disable_interrupts();
  asm volatile("nop");
}

/**
 * @brief   Enables all the interrupt sources.
 */
static inline void port_enable(void) {

  asm volatile("nop");
  _enable_interrupts();
}

/**
 * @brief   Enters an architecture-dependent IRQ-waiting mode.
 * @details The function is meant to return when an interrupt becomes pending.
 *          The simplest implementation is an empty function or macro but this
 *          would not take advantage of architecture-specific power saving
 *          modes.
 */
static inline void port_wait_for_interrupt(void) {
  
}

/**
 * @brief   Returns the current value of the realtime counter.
 *
 * @return              The realtime counter value.
 */
static inline rtcnt_t port_rt_get_counter_value(void) {
  /* TODO implement realtime counter */
  return 0;
}

#endif /* !defined(_FROM_ASM_) */

/*===========================================================================*/
/* Module late inclusions.                                                   */
/*===========================================================================*/

#if !defined(_FROM_ASM_)

#if CH_CFG_ST_TIMEDELTA > 0
#if !PORT_USE_ALT_TIMER
#include "chcore_timer.h"
#else /* PORT_USE_ALT_TIMER */
#include "chcore_timer_alt.h"
#endif /* PORT_USE_ALT_TIMER */
#endif /* CH_CFG_ST_TIMEDELTA > 0 */

#endif /* !defined(_FROM_ASM_) */

#endif /* CHCORE_H */

/** @} */
