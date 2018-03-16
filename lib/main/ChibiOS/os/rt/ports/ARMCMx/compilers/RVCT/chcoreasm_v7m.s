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
 * @file    compilers/RVCT/chcoreasm_v7m.s
 * @brief   ARMv7-M architecture port low level code.
 *
 * @addtogroup ARMCMx_RVCT_CORE
 * @{
 */

#if !defined(FALSE) || defined(__DOXYGEN__)
#define FALSE   0
#endif

#if !defined(TRUE) || defined(__DOXYGEN__)
#define TRUE    1
#endif

#define _FROM_ASM_
#include "chconf.h"
#include "chcore.h"

#if !defined(__DOXYGEN__)

CONTEXT_OFFSET  EQU     12
SCB_ICSR        EQU     0xE000ED04
ICSR_PENDSVSET  EQU     0x10000000

                PRESERVE8
                THUMB
                AREA    |.text|, CODE, READONLY

                IMPORT  chThdExit
                IMPORT  chSchDoReschedule
#if CH_DBG_STATISTICS
                IMPORT   _stats_start_measure_crit_thd
                IMPORT   _stats_stop_measure_crit_thd
#endif
#if CH_DBG_SYSTEM_STATE_CHECK
                IMPORT  _dbg_check_unlock
                IMPORT  _dbg_check_lock
#endif

/*
 * Performs a context switch between two threads.
 */
                EXPORT _port_switch
_port_switch    PROC
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
                ENDP

/*
 * Start a thread by invoking its work function.
 * If the work function returns @p chThdExit() is automatically invoked.
 */
                EXPORT  _port_thread_start
_port_thread_start PROC
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
                bl      chThdExit
                ENDP

/*
 * Post-IRQ switch code.
 * Exception handlers return here for context switching.
 */
                EXPORT  _port_switch_from_isr
                EXPORT  _port_exit_from_isr
_port_switch_from_isr PROC
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
_port_exit_from_isr
#if CORTEX_SIMPLIFIED_PRIORITY
                mov     r3, #SCB_ICSR :AND: 0xFFFF
                movt    r3, #SCB_ICSR :SHR: 16
                mov     r2, #ICSR_PENDSVSET
                str     r2, [r3, #0]
                cpsie   i
#else
                svc     #0
#endif
waithere        b       waithere
                ENDP

                END

#endif /* !defined(__DOXYGEN__) */

/** @} */
