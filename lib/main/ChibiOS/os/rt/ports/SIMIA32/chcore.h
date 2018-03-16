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
 * @file    SIMIA32/chcore.h
 * @brief   Simulator on IA32 port macros and structures.
 *
 * @addtogroup SIMIA32_GCC_CORE
 * @{
 */

#ifndef _CHCORE_H_
#define _CHCORE_H_

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/**
 * Macro defining the a simulated architecture into x86.
 */
#define PORT_ARCHITECTURE_SIMIA32

/**
 * Name of the implemented architecture.
 */
#define PORT_ARCHITECTURE_NAME          "Simulator"

/**
 * @brief   Name of the architecture variant (optional).
 */
#define PORT_CORE_VARIANT_NAME          "x86 (integer only)"

/**
 * @brief   Name of the compiler supported by this port.
 */
#define PORT_COMPILER_NAME              "GCC " __VERSION__

/**
 * @brief   Port-specific information string.
 */
#define PORT_INFO                       "No preemption"

/**
 * @brief   This port supports a realtime counter.
 */
#define PORT_SUPPORTS_RT                TRUE

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   Stack size for the system idle thread.
 * @details This size depends on the idle thread implementation, usually
 *          the idle thread should take no more space than those reserved
 *          by @p PORT_INT_REQUIRED_STACK.
 */
#ifndef PORT_IDLE_THREAD_STACK_SIZE
#define PORT_IDLE_THREAD_STACK_SIZE     256
#endif

/**
 * @brief   Per-thread stack overhead for interrupts servicing.
 * @details This constant is used in the calculation of the correct working
 *          area size.
 */
#ifndef PORT_INT_REQUIRED_STACK
#define PORT_INT_REQUIRED_STACK         16384
#endif

/**
 * @brief   Enables an alternative timer implementation.
 * @details Usually the port uses a timer interface defined in the file
 *          @p chcore_timer.h, if this option is enabled then the file
 *          @p chcore_timer_alt.h is included instead.
 */
#if !defined(PORT_USE_ALT_TIMER)
#define PORT_USE_ALT_TIMER              FALSE
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if CH_DBG_ENABLE_STACK_CHECK
#error "option CH_DBG_ENABLE_STACK_CHECK not supported by this port"
#endif

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   16 bytes stack and memory alignment enforcement.
 */
typedef struct {
  uint8_t a[16];
} stkalign_t __attribute__((aligned(16)));

/**
 * @brief   Type of a generic x86 register.
 */
typedef void *regx86;

/**
 * @brief   Interrupt saved context.
 * @details This structure represents the stack frame saved during a
 *          preemption-capable interrupt handler.
 */
struct port_extctx {
};

/**
 * @brief   System saved context.
 * @details This structure represents the inner stack frame during a context
 *          switch.
 */
struct port_intctx {
  regx86  ebx;
  regx86  edi;
  regx86  esi;
  regx86  ebp;
  regx86  eip;
};

/**
 * @brief   Platform dependent part of the @p thread_t structure.
 * @details In this port the structure just holds a pointer to the
 *          @p port_intctx structure representing the stack pointer
 *          at context switch time.
 */
struct context {
  struct port_intctx *esp;
};

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

#define APUSH(p, a) do {                                                    \
  (p) -= sizeof(void *);                                                    \
  *(void **)(p) = (void*)(a);                                               \
} while (false)

/* Darwin requires the stack to be aligned to a 16-byte boundary at
 * the time of a call instruction (in case the called function needs
 * to save MMX registers). This aligns to 'mod' module 16, so that we'll end
 * up with the right alignment after pushing the args. */
#define AALIGN(p, mask, mod)                                                \
  p = (void *)((((uint32_t)(p) - (uint32_t)(mod)) & ~(uint32_t)(mask)) + (uint32_t)(mod)) \

/**
 * @brief   Platform dependent part of the @p chThdCreateI() API.
 * @details This code usually setup the context switching frame represented
 *          by an @p port_intctx structure.
 */
#define PORT_SETUP_CONTEXT(tp, workspace, wsize, pf, arg) {                 \
  /*lint -save -e611 -e9033 -e9074 -e9087 [10.8, 11.1, 11.3] Valid casts.*/ \
  uint8_t *esp = (uint8_t *)workspace + wsize;                              \
  APUSH(esp, 0);                                                            \
  uint8_t *savebp = esp;                                                    \
  AALIGN(esp, 15, 8);                                                       \
  APUSH(esp, arg);                                                          \
  APUSH(esp, pf);                                                           \
  APUSH(esp, 0);                                                            \
  esp -= sizeof(struct port_intctx);                                        \
  ((struct port_intctx *)esp)->eip = (void *)_port_thread_start;            \
  ((struct port_intctx *)esp)->ebx = NULL;                                  \
  ((struct port_intctx *)esp)->edi = NULL;                                  \
  ((struct port_intctx *)esp)->esi = NULL;                                  \
  ((struct port_intctx *)esp)->ebp = (void *)savebp;                        \
  (tp)->p_ctx.esp = (struct port_intctx *)esp;                              \
  /*lint -restore*/                                                         \
}

 /**
 * @brief   Computes the thread working area global size.
 * @note    There is no need to perform alignments in this macro.
  */
#define PORT_WA_SIZE(n) ((sizeof(void *) * 4U) +                            \
                         sizeof(struct port_intctx) +                       \
                         ((size_t)(n)) +                                    \
                         ((size_t)(PORT_INT_REQUIRED_STACK)))

/**
 * @brief   IRQ prologue code.
 * @details This macro must be inserted at the start of all IRQ handlers
 *          enabled to invoke system APIs.
 */
#define PORT_IRQ_PROLOGUE() {                                               \
  port_isr_context_flag = true;                                             \
}

/**
 * @brief   IRQ epilogue code.
 * @details This macro must be inserted at the end of all IRQ handlers
 *          enabled to invoke system APIs.
 */
#define PORT_IRQ_EPILOGUE() {                                               \
  port_isr_context_flag = false;                                            \
}


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

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern bool port_isr_context_flag;
extern syssts_t port_irq_sts;

#ifdef __cplusplus
extern "C" {
#endif
  /*lint -save -e950 [Dir-2.1] Non-ANSI keywords are fine in the port layer.*/
  __attribute__((fastcall)) void port_switch(thread_t *ntp, thread_t *otp);
  __attribute__((cdecl, noreturn)) void _port_thread_start(msg_t (*pf)(void *p),
                                                           void *p);
  /*lint -restore*/
  rtcnt_t port_rt_get_counter_value(void);
  void _sim_check_for_interrupts(void);
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

  port_irq_sts = (syssts_t)0;
  port_isr_context_flag = false;
}

/**
 * @brief   Returns a word encoding the current interrupts status.
 *
 * @return              The interrupts status.
 */
static inline syssts_t port_get_irq_status(void) {

  return port_irq_sts;
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

  return sts == (syssts_t)0;
}

/**
 * @brief   Determines the current execution context.
 *
 * @return              The execution context.
 * @retval false        not running in ISR mode.
 * @retval true         running in ISR mode.
 */
static inline bool port_is_isr_context(void) {

  return port_isr_context_flag;
}

/**
 * @brief   Kernel-lock action.
 * @details In this port this function disables interrupts globally.
 */
static inline void port_lock(void) {

  port_irq_sts = (syssts_t)1;
}

/**
 * @brief   Kernel-unlock action.
 * @details In this port this function enables interrupts globally.
 */
static inline void port_unlock(void) {

  port_irq_sts = (syssts_t)0;
}

/**
 * @brief   Kernel-lock action from an interrupt handler.
 * @details In this port this function disables interrupts globally.
 * @note    Same as @p port_lock() in this port.
 */
static inline void port_lock_from_isr(void) {

  port_irq_sts = (syssts_t)1;
}

/**
 * @brief   Kernel-unlock action from an interrupt handler.
 * @details In this port this function enables interrupts globally.
 * @note    Same as @p port_lock() in this port.
 */
static inline void port_unlock_from_isr(void) {

  port_irq_sts = (syssts_t)0;
}

/**
 * @brief   Disables all the interrupt sources.
 */
static inline void port_disable(void) {

  port_irq_sts = (syssts_t)1;
}

/**
 * @brief   Disables the interrupt sources below kernel-level priority.
 */
static inline void port_suspend(void) {

  port_irq_sts = (syssts_t)1;
}

/**
 * @brief   Enables all the interrupt sources.
 */
static inline void port_enable(void) {

  port_irq_sts = (syssts_t)0;
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

  _sim_check_for_interrupts();
}

#endif /* _CHCORE_H_ */

/** @} */
