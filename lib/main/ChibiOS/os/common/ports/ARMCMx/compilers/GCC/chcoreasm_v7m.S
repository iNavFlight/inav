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
 * @file    compilers/GCC/chcoreasm_v7m.S
 * @brief   ARMv7-M architecture port low level code.
 *
 * @addtogroup ARMCMx_GCC_CORE
 * @{
 */

#if !defined(FALSE) || defined(__DOXYGEN__)
#define FALSE   0
#endif

#if !defined(TRUE) || defined(__DOXYGEN__)
#define TRUE    1
#endif

#define _FROM_ASM_
#include "chlicense.h"
#include "chconf.h"
#include "chcore.h"

#if !defined(__DOXYGEN__)

/*
 * RTOS-specific context offset.
 */
#if defined(_CHIBIOS_RT_CONF_)
#define CONTEXT_OFFSET  12
#elif defined(_CHIBIOS_NIL_CONF_)
#define CONTEXT_OFFSET  0
#else
#error "invalid chconf.h"
#endif

                .set    SCB_ICSR, 0xE000ED04
                .set    ICSR_PENDSVSET, 0x10000000

                .syntax unified
                .cpu    cortex-m4
#if CORTEX_USE_FPU
                .fpu    fpv4-sp-d16
#else
                .fpu    softvfp
#endif

                .thumb
                .text

/*--------------------------------------------------------------------------*
 * Performs a context switch between two threads.
 *--------------------------------------------------------------------------*/
                .thumb_func
                .globl  _port_switch
_port_switch:
                push    {r4, r5, r6, r7, r8, r9, r10, r11, lr}
#if CORTEX_USE_FPU
                vpush   {s16-s31}
#endif

                str     sp, [r1, #CONTEXT_OFFSET]
#if (CORTEX_SIMPLIFIED_PRIORITY == FALSE) &&                                \
    ((CORTEX_MODEL == 3) || (CORTEX_MODEL == 4))
                /* Workaround for ARM errata 752419, only applied if
                   condition exists for it to be triggered.*/
                ldr     r3, [r0, #CONTEXT_OFFSET]
                mov     sp, r3
#else
                ldr     sp, [r0, #CONTEXT_OFFSET]
#endif

#if CORTEX_USE_FPU
                vpop    {s16-s31}
#endif
                pop     {r4, r5, r6, r7, r8, r9, r10, r11, pc}

/*--------------------------------------------------------------------------*
 * Start a thread by invoking its work function.
 *
 * Threads execution starts here, the code leaves the system critical zone
 * and then jumps into the thread function passed in register R4. The
 * register R5 contains the thread parameter. The function chThdExit() is
 * called on thread function return.
 *--------------------------------------------------------------------------*/
                .thumb_func
                .globl  _port_thread_start
_port_thread_start:
#if CH_DBG_ENABLE_STACK_CHECK && PORT_ENABLE_GUARD_PAGES
                bl      _port_set_region
#endif
#if CH_DBG_SYSTEM_STATE_CHECK
                bl      _dbg_check_unlock
#endif
#if CH_DBG_STATISTICS
                bl      _stats_stop_measure_crit_thd
#endif
#if CORTEX_SIMPLIFIED_PRIORITY
                cpsie   i
#else
                movs    r3, #0              /* CORTEX_BASEPRI_DISABLED */
                msr     BASEPRI, r3
#endif
                mov     r0, r5
                blx     r4
#if defined(_CHIBIOS_RT_CONF_)
                movs    r0, #0              /* MSG_OK */
                bl      chThdExit
#endif
#if defined(_CHIBIOS_NIL_CONF_)
                mov     r3, #0
                bl      chSysHalt
#endif

/*--------------------------------------------------------------------------*
 * Post-IRQ switch code.
 *
 * Exception handlers return here for context switching.
 *--------------------------------------------------------------------------*/
                .thumb_func
                .globl  _port_switch_from_isr
_port_switch_from_isr:
#if CH_DBG_STATISTICS
                bl      _stats_start_measure_crit_thd
#endif
#if CH_DBG_SYSTEM_STATE_CHECK
                bl      _dbg_check_lock
#endif
                bl      chSchDoReschedule
#if CH_DBG_SYSTEM_STATE_CHECK
                bl      _dbg_check_unlock
#endif
#if CH_DBG_STATISTICS
                bl      _stats_stop_measure_crit_thd
#endif
                .globl  _port_exit_from_isr
_port_exit_from_isr:
#if CORTEX_SIMPLIFIED_PRIORITY
                movw    r3, #:lower16:SCB_ICSR
                movt    r3, #:upper16:SCB_ICSR
                mov     r2, ICSR_PENDSVSET
                str     r2, [r3, #0]
                cpsie   i
#else /* !CORTEX_SIMPLIFIED_PRIORITY */
                svc     #0
#endif /* !CORTEX_SIMPLIFIED_PRIORITY */
.L1:            b       .L1

#endif /* !defined(__DOXYGEN__) */

/** @} */
