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
 * @file    chcore_v6m.h
 * @brief   ARMv6-M architecture port macros and structures.
 *
 * @addtogroup ARMCMx_V6M_CORE
 * @{
 */

#ifndef _CHCORE_V6M_H_
#define _CHCORE_V6M_H_

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/**
 * @brief   This port does not support a realtime counter.
 */
#define PORT_SUPPORTS_RT                FALSE

/**
 * @brief   PendSV priority level.
 * @note    This priority is enforced to be equal to @p 0,
 *          this handler always has the highest priority that cannot preempt
 *          the kernel.
 */
#define CORTEX_PRIORITY_PENDSV          0

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   Stack size for the system idle thread.
 * @details This size depends on the idle thread implementation, usually
 *          the idle thread should take no more space than those reserved
 *          by @p PORT_INT_REQUIRED_STACK.
 * @note    In this port it is set to 16 because the idle thread does have
 *          a stack frame when compiling without optimizations. You may
 *          reduce this value to zero when compiling with optimizations.
 */
#if !defined(PORT_IDLE_THREAD_STACK_SIZE)
#define PORT_IDLE_THREAD_STACK_SIZE     16
#endif

/**
 * @brief   Per-thread stack overhead for interrupts servicing.
 * @details This constant is used in the calculation of the correct working
 *          area size.
 * @note    In this port this value is conservatively set to 64 because the
 *          function @p chSchDoReschedule() can have a stack frame, especially
 *          with compiler optimizations disabled. The value can be reduced
 *          when compiler optimizations are enabled.
 */
#if !defined(PORT_INT_REQUIRED_STACK)
#define PORT_INT_REQUIRED_STACK         64
#endif

/**
 * @brief   Enables the use of the WFI instruction in the idle thread loop.
 */
#if !defined(CORTEX_ENABLE_WFI_IDLE)
#define CORTEX_ENABLE_WFI_IDLE          FALSE
#endif

/**
 * @brief   Alternate preemption method.
 * @details Activating this option will make the Kernel use the PendSV
 *          handler for preemption instead of the NMI handler.
 */
#ifndef CORTEX_ALTERNATE_SWITCH
#define CORTEX_ALTERNATE_SWITCH         FALSE
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/**
 * @name    Architecture and Compiler
 * @{
 */
#if ((CORTEX_MODEL == 0) && !defined(__CORE_CM0PLUS_H_DEPENDANT)) ||        \
    defined(__DOXYGEN__)
/**
 * @brief   Macro defining the specific ARM architecture.
 */
#define PORT_ARCHITECTURE_ARM_v6M

/**
 * @brief   Name of the implemented architecture.
 */
#define PORT_ARCHITECTURE_NAME          "ARMv6-M"

/**
 * @brief   Name of the architecture variant.
 */
#define PORT_CORE_VARIANT_NAME          "Cortex-M0"

#elif (CORTEX_MODEL == 0) && defined(__CORE_CM0PLUS_H_DEPENDANT)
#define PORT_ARCHITECTURE_ARM_v6M
#define PORT_ARCHITECTURE_NAME          "ARMv6-M"
#define PORT_CORE_VARIANT_NAME          "Cortex-M0+"
#endif

/**
 * @brief   Port-specific information string.
 */
#if (CORTEX_ALTERNATE_SWITCH == FALSE) || defined(__DOXYGEN__)
#define PORT_INFO                       "Preemption through NMI"
#else
#define PORT_INFO                       "Preemption through PendSV"
#endif
/** @} */

/**
 * @brief   Maximum usable priority for normal ISRs.
 */
#if (CORTEX_ALTERNATE_SWITCH == TRUE) || defined(__DOXYGEN__)
#define CORTEX_MAX_KERNEL_PRIORITY      1
#else
#define CORTEX_MAX_KERNEL_PRIORITY      0
#endif

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

#if !defined(_FROM_ASM_)

 /* The documentation of the following declarations is in chconf.h in order
    to not have duplicated structure names into the documentation.*/
#if !defined(__DOXYGEN__)
struct port_extctx {
  regarm_t      r0;
  regarm_t      r1;
  regarm_t      r2;
  regarm_t      r3;
  regarm_t      r12;
  regarm_t      lr_thd;
  regarm_t      pc;
  regarm_t      xpsr;
};

struct port_intctx {
  regarm_t      r8;
  regarm_t      r9;
  regarm_t      r10;
  regarm_t      r11;
  regarm_t      r4;
  regarm_t      r5;
  regarm_t      r6;
  regarm_t      r7;
  regarm_t      lr;
};
#endif /* !defined(__DOXYGEN__) */

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
  (tp)->p_ctx.r13->lr = (regarm_t)_port_thread_start;                       \
}

/**
 * @brief   Computes the thread working area global size.
 * @note    There is no need to perform alignments in this macro.
 */
#define PORT_WA_SIZE(n) (sizeof(struct port_intctx) +                       \
                         sizeof(struct port_extctx) +                       \
                         ((size_t)(n)) + ((size_t)(PORT_INT_REQUIRED_STACK)))

/**
 * @brief   IRQ prologue code.
 * @details This macro must be inserted at the start of all IRQ handlers
 *          enabled to invoke system APIs.
 */
#if defined(__GNUC__) || defined(__DOXYGEN__)
#define PORT_IRQ_PROLOGUE()                                                 \
  regarm_t _saved_lr = (regarm_t)__builtin_return_address(0)
#elif defined(__ICCARM__)
#define PORT_IRQ_PROLOGUE()                                                 \
  regarm_t _saved_lr = (regarm_t)__get_LR()
#elif defined(__CC_ARM)
#define PORT_IRQ_PROLOGUE()                                                 \
  regarm_t _saved_lr = (regarm_t)__return_address()
#endif

/**
 * @brief   IRQ epilogue code.
 * @details This macro must be inserted at the end of all IRQ handlers
 *          enabled to invoke system APIs.
 */
#define PORT_IRQ_EPILOGUE() _port_irq_epilogue(_saved_lr)

/**
 * @brief   IRQ handler function declaration.
 * @note    @p id can be a function name or a vector number depending on the
 *          port implementation.
 */
#define PORT_IRQ_HANDLER(id) void id(void)

/**
 * @brief   Fast IRQ handler function declaration.
 * @note    @p id can be a function name or a vector number depending on the
 *          port implementation.
 */
#define PORT_FAST_IRQ_HANDLER(id) void id(void)

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
#if (CH_DBG_ENABLE_STACK_CHECK == FALSE) || defined(__DOXYGEN__)
#define port_switch(ntp, otp) _port_switch(ntp, otp)
#else
#define port_switch(ntp, otp) {                                             \
  struct port_intctx *r13 = (struct port_intctx *)__get_PSP();              \
  if ((stkalign_t *)(r13 - 1) < (otp)->p_stklimit) {                        \
    chSysHalt("stack overflow");                                            \
  }                                                                         \
  _port_switch(ntp, otp);                                                   \
}
#endif

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void _port_irq_epilogue(regarm_t lr);
  void _port_switch(thread_t *ntp, thread_t *otp);
  void _port_thread_start(void);
  void _port_switch_from_isr(void);
  void _port_exit_from_isr(void);
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

  NVIC_SetPriority(PendSV_IRQn, CORTEX_PRIORITY_PENDSV);
}

/**
 * @brief   Returns a word encoding the current interrupts status.
 *
 * @return              The interrupts status.
 */
static inline syssts_t port_get_irq_status(void) {

  return (syssts_t)__get_PRIMASK();
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

  return (sts & (syssts_t)1) == (syssts_t)0;
}

/**
 * @brief   Determines the current execution context.
 *
 * @return              The execution context.
 * @retval false        not running in ISR mode.
 * @retval true         running in ISR mode.
 */
static inline bool port_is_isr_context(void) {

  return (bool)((__get_IPSR() & 0x1FFU) != 0U);
}

/**
 * @brief   Kernel-lock action.
 * @details In this port this function disables interrupts globally.
 */
static inline void port_lock(void) {

  __disable_irq();
}

/**
 * @brief   Kernel-unlock action.
 * @details In this port this function enables interrupts globally.
 */
static inline void port_unlock(void) {

  __enable_irq();
}

/**
 * @brief   Kernel-lock action from an interrupt handler.
 * @details In this port this function disables interrupts globally.
 * @note    Same as @p port_lock() in this port.
 */
static inline void port_lock_from_isr(void) {

  port_lock();
}

/**
 * @brief   Kernel-unlock action from an interrupt handler.
 * @details In this port this function enables interrupts globally.
 * @note    Same as @p port_lock() in this port.
 */
static inline void port_unlock_from_isr(void) {

  port_unlock();
}

/**
 * @brief   Disables all the interrupt sources.
 */
static inline void port_disable(void) {

  __disable_irq();
}

/**
 * @brief   Disables the interrupt sources below kernel-level priority.
 */
static inline void port_suspend(void) {

  __disable_irq();
}

/**
 * @brief   Enables all the interrupt sources.
 */
static inline void port_enable(void) {

  __enable_irq();
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

#if CORTEX_ENABLE_WFI_IDLE == TRUE
  __WFI();
#endif
}

#endif /* _FROM_ASM_ */

#endif /* _CHCORE_V6M_H_ */

/** @} */
