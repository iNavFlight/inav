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
 * @file    chcore_v7m.h
 * @brief   ARMv7-M architecture port macros and structures.
 *
 * @addtogroup ARMCMx_V7M_CORE
 * @{
 */

#ifndef _CHCORE_V7M_H_
#define _CHCORE_V7M_H_

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/**
 * @brief   This port supports a realtime counter.
 */
#define PORT_SUPPORTS_RT                TRUE

/**
 * @brief   Disabled value for BASEPRI register.
 */
#define CORTEX_BASEPRI_DISABLED         0U

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
#if !defined(PORT_IDLE_THREAD_STACK_SIZE) || defined(__DOXYGEN__)
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
#if !defined(PORT_INT_REQUIRED_STACK) || defined(__DOXYGEN__)
#define PORT_INT_REQUIRED_STACK         64
#endif

/**
 * @brief   Enables the use of the WFI instruction in the idle thread loop.
 */
#if !defined(CORTEX_ENABLE_WFI_IDLE)
#define CORTEX_ENABLE_WFI_IDLE          FALSE
#endif

/**
 * @brief   FPU support in context switch.
 * @details Activating this option activates the FPU support in the kernel.
 */
#if !defined(CORTEX_USE_FPU)
#define CORTEX_USE_FPU                  CORTEX_HAS_FPU
#elif (CORTEX_USE_FPU == TRUE) && (CORTEX_HAS_FPU == FALSE)
/* This setting requires an FPU presence check in case it is externally
   redefined.*/
#error "the selected core does not have an FPU"
#endif

/**
 * @brief   Simplified priority handling flag.
 * @details Activating this option makes the Kernel work in compact mode.
 *          In compact mode interrupts are disabled globally instead of
 *          raising the priority mask to some intermediate level.
 */
#if !defined(CORTEX_SIMPLIFIED_PRIORITY)
#define CORTEX_SIMPLIFIED_PRIORITY      FALSE
#endif

/**
 * @brief   SVCALL handler priority.
 * @note    The default SVCALL handler priority is defaulted to
 *          @p CORTEX_MAXIMUM_PRIORITY+1, this reserves the
 *          @p CORTEX_MAXIMUM_PRIORITY priority level as fast interrupts
 *          priority level.
 */
#if !defined(CORTEX_PRIORITY_SVCALL)
#define CORTEX_PRIORITY_SVCALL          (CORTEX_MAXIMUM_PRIORITY + 1U)
#elif !PORT_IRQ_IS_VALID_PRIORITY(CORTEX_PRIORITY_SVCALL)
/* If it is externally redefined then better perform a validity check on it.*/
#error "invalid priority level specified for CORTEX_PRIORITY_SVCALL"
#endif

/**
 * @brief   NVIC VTOR initialization expression.
 */
#if !defined(CORTEX_VTOR_INIT) || defined(__DOXYGEN__)
#define CORTEX_VTOR_INIT                0x00000000U
#endif

/**
 * @brief   NVIC PRIGROUP initialization expression.
 * @details The default assigns all available priority bits as preemption
 *          priority with no sub-priority.
 */
#if !defined(CORTEX_PRIGROUP_INIT) || defined(__DOXYGEN__)
#define CORTEX_PRIGROUP_INIT            (7 - CORTEX_PRIORITY_BITS)
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/**
 * @name    Architecture and Compiler
 * @{
 */
#if (CORTEX_MODEL == 3) || defined(__DOXYGEN__)
/**
 * @brief   Macro defining the specific ARM architecture.
 */
#define PORT_ARCHITECTURE_ARM_v7M

/**
 * @brief   Name of the implemented architecture.
 */
#define PORT_ARCHITECTURE_NAME          "ARMv7-M"

/**
 * @brief   Name of the architecture variant.
 */
#define PORT_CORE_VARIANT_NAME          "Cortex-M3"

#elif (CORTEX_MODEL == 4)
#define PORT_ARCHITECTURE_ARM_v7ME
#define PORT_ARCHITECTURE_NAME          "ARMv7E-M"
#if CORTEX_USE_FPU
#define PORT_CORE_VARIANT_NAME          "Cortex-M4F"
#else
#define PORT_CORE_VARIANT_NAME          "Cortex-M4"
#endif

#elif (CORTEX_MODEL == 7)
#define PORT_ARCHITECTURE_ARM_v7ME
#define PORT_ARCHITECTURE_NAME          "ARMv7E-M"
#if CORTEX_USE_FPU
#define PORT_CORE_VARIANT_NAME          "Cortex-M7F"
#else
#define PORT_CORE_VARIANT_NAME          "Cortex-M7"
#endif
#endif

/**
 * @brief   Port-specific information string.
 */
#if (CORTEX_SIMPLIFIED_PRIORITY == FALSE) || defined(__DOXYGEN__)
#define PORT_INFO                       "Advanced kernel mode"
#else
#define PORT_INFO                       "Compact kernel mode"
#endif
/** @} */

#if (CORTEX_SIMPLIFIED_PRIORITY == FALSE) || defined(__DOXYGEN__)
/**
 * @brief   Maximum usable priority for normal ISRs.
 */
#define CORTEX_MAX_KERNEL_PRIORITY      (CORTEX_PRIORITY_SVCALL + 1U)

/**
 * @brief   BASEPRI level within kernel lock.
 */
#define CORTEX_BASEPRI_KERNEL                                               \
  CORTEX_PRIO_MASK(CORTEX_MAX_KERNEL_PRIORITY)
#else

#define CORTEX_MAX_KERNEL_PRIORITY      0U
#endif

/**
 * @brief   PendSV priority level.
 * @note    This priority is enforced to be equal to
 *          @p CORTEX_MAX_KERNEL_PRIORITY, this handler always have the
 *          highest priority that cannot preempt the kernel.
 */
#define CORTEX_PRIORITY_PENDSV          CORTEX_MAX_KERNEL_PRIORITY

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/* The following code is not processed when the file is included from an
   asm module.*/
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
#if CORTEX_USE_FPU
  regarm_t      s0;
  regarm_t      s1;
  regarm_t      s2;
  regarm_t      s3;
  regarm_t      s4;
  regarm_t      s5;
  regarm_t      s6;
  regarm_t      s7;
  regarm_t      s8;
  regarm_t      s9;
  regarm_t      s10;
  regarm_t      s11;
  regarm_t      s12;
  regarm_t      s13;
  regarm_t      s14;
  regarm_t      s15;
  regarm_t      fpscr;
  regarm_t      reserved;
#endif /* CORTEX_USE_FPU */
};

struct port_intctx {
#if CORTEX_USE_FPU
  regarm_t      s16;
  regarm_t      s17;
  regarm_t      s18;
  regarm_t      s19;
  regarm_t      s20;
  regarm_t      s21;
  regarm_t      s22;
  regarm_t      s23;
  regarm_t      s24;
  regarm_t      s25;
  regarm_t      s26;
  regarm_t      s27;
  regarm_t      s28;
  regarm_t      s29;
  regarm_t      s30;
  regarm_t      s31;
#endif /* CORTEX_USE_FPU */
  regarm_t      r4;
  regarm_t      r5;
  regarm_t      r6;
  regarm_t      r7;
  regarm_t      r8;
  regarm_t      r9;
  regarm_t      r10;
  regarm_t      r11;
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
                                           (size_t)(wsize) -                \
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
#define PORT_IRQ_PROLOGUE()

/**
 * @brief   IRQ epilogue code.
 * @details This macro must be inserted at the end of all IRQ handlers
 *          enabled to invoke system APIs.
 */
#define PORT_IRQ_EPILOGUE() _port_irq_epilogue()

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
  void _port_irq_epilogue(void);
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

  /* Initialization of the vector table and priority related settings.*/
  SCB->VTOR = CORTEX_VTOR_INIT;

  /* Initializing priority grouping.*/
  NVIC_SetPriorityGrouping(CORTEX_PRIGROUP_INIT);

  /* DWT cycle counter enable, note, the M7 requires DWT unlocking.*/
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
#if CORTEX_MODEL == 7
  DWT->LAR = 0xC5ACCE55U;
#endif
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  /* Initialization of the system vectors used by the port.*/
#if CORTEX_SIMPLIFIED_PRIORITY == FALSE
  NVIC_SetPriority(SVCall_IRQn, CORTEX_PRIORITY_SVCALL);
#endif
  NVIC_SetPriority(PendSV_IRQn, CORTEX_PRIORITY_PENDSV);
}

/**
 * @brief   Returns a word encoding the current interrupts status.
 *
 * @return              The interrupts status.
 */
static inline syssts_t port_get_irq_status(void) {
  syssts_t sts;

#if CORTEX_SIMPLIFIED_PRIORITY == FALSE
  sts = (syssts_t)__get_BASEPRI();
#else /* CORTEX_SIMPLIFIED_PRIORITY */
  sts = (syssts_t)__get_PRIMASK();
#endif /* CORTEX_SIMPLIFIED_PRIORITY */
  return sts;
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

#if CORTEX_SIMPLIFIED_PRIORITY == FALSE
  return sts == (syssts_t)CORTEX_BASEPRI_DISABLED;
#else /* CORTEX_SIMPLIFIED_PRIORITY */
  return (sts & (syssts_t)1) == (syssts_t)0;
#endif /* CORTEX_SIMPLIFIED_PRIORITY */
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
 * @details In this port this function raises the base priority to kernel
 *          level.
 */
static inline void port_lock(void) {

#if CORTEX_SIMPLIFIED_PRIORITY == FALSE
#if defined(__CM7_REV)
#if __CM7_REV <= 1
  __disable_irq();
#endif
#endif
  __set_BASEPRI(CORTEX_BASEPRI_KERNEL);
#if defined(__CM7_REV)
#if __CM7_REV <= 1
  __enable_irq();
#endif
#endif
#else /* CORTEX_SIMPLIFIED_PRIORITY */
  __disable_irq();
#endif /* CORTEX_SIMPLIFIED_PRIORITY */
}

/**
 * @brief   Kernel-unlock action.
 * @details In this port this function lowers the base priority to user
 *          level.
 */
static inline void port_unlock(void) {

#if CORTEX_SIMPLIFIED_PRIORITY == FALSE
  __set_BASEPRI(CORTEX_BASEPRI_DISABLED);
#else /* CORTEX_SIMPLIFIED_PRIORITY */
  __enable_irq();
#endif /* CORTEX_SIMPLIFIED_PRIORITY */
}

/**
 * @brief   Kernel-lock action from an interrupt handler.
 * @details In this port this function raises the base priority to kernel
 *          level.
 * @note    Same as @p port_lock() in this port.
 */
static inline void port_lock_from_isr(void) {

  port_lock();
}

/**
 * @brief   Kernel-unlock action from an interrupt handler.
 * @details In this port this function lowers the base priority to user
 *          level.
 * @note    Same as @p port_unlock() in this port.
 */
static inline void port_unlock_from_isr(void) {

  port_unlock();
}

/**
 * @brief   Disables all the interrupt sources.
 * @note    In this port it disables all the interrupt sources by raising
 *          the priority mask to level 0.
 */
static inline void port_disable(void) {

  __disable_irq();
}

/**
 * @brief   Disables the interrupt sources below kernel-level priority.
 * @note    Interrupt sources above kernel level remains enabled.
 * @note    In this port it raises/lowers the base priority to kernel level.
 */
static inline void port_suspend(void) {

#if (CORTEX_SIMPLIFIED_PRIORITY == FALSE) || defined(__DOXYGEN__)
  __set_BASEPRI(CORTEX_BASEPRI_KERNEL);
  __enable_irq();
#else
  __disable_irq();
#endif
}

/**
 * @brief   Enables all the interrupt sources.
 * @note    In this port it lowers the base priority to user level.
 */
static inline void port_enable(void) {

#if (CORTEX_SIMPLIFIED_PRIORITY == FALSE) || defined(__DOXYGEN__)
  __set_BASEPRI(CORTEX_BASEPRI_DISABLED);
#endif
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

/**
 * @brief   Returns the current value of the realtime counter.
 *
 * @return              The realtime counter value.
 */
static inline rtcnt_t port_rt_get_counter_value(void) {

  return DWT->CYCCNT;
}

#endif /* !defined(_FROM_ASM_) */

#endif /* _CHCORE_V7M_H_ */

/** @} */
