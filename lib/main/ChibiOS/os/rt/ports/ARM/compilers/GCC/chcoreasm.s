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
 * @file    ARM/compilers/GCC/chcoreasm.s
 * @brief   ARM architecture port low level code.
 *
 * @addtogroup ARM_CORE
 * @{
 */

#define _FROM_ASM_
#include "chconf.h"
#include "armparams.h"

#define FALSE 0
#define TRUE 1

#if !defined(__DOXYGEN__)

                .set    MODE_USR, 0x10
                .set    MODE_FIQ, 0x11
                .set    MODE_IRQ, 0x12
                .set    MODE_SVC, 0x13
                .set    MODE_ABT, 0x17
                .set    MODE_UND, 0x1B
                .set    MODE_SYS, 0x1F

                .equ    I_BIT, 0x80
                .equ    F_BIT, 0x40

                .text

/*
 * The following functions are only present if there is THUMB code in
 * the system.
 */
#if defined(THUMB_PRESENT)
                .balign 16
                .code   16
                .thumb_func
                .global _port_get_cpsr
_port_get_cpsr:
                mov     r0, pc
                bx      r0
.code 32
                mrs     r0, CPSR
                bx      lr

                .balign 16
                .code   16
                .thumb_func
                .global _port_disable_thumb
_port_disable_thumb:
                mov     r3, pc
                bx      r3
.code 32
                mrs     r3, CPSR
                orr     r3, #I_BIT
                msr     CPSR_c, r3
                orr     r3, #F_BIT
                msr     CPSR_c, r3
                bx      lr

                .balign 16
                .code   16
                .thumb_func
                .global _port_suspend_thumb
_port_suspend_thumb:
                // Goes into _port_unlock_thumb

                .code   16
                .global _port_lock_thumb
_port_lock_thumb:
                mov     r3, pc
                bx      r3
                .code   32
                msr     CPSR_c, #MODE_SYS | I_BIT
                bx      lr

                .balign 16
                .code   16
                .thumb_func
                .global _port_enable_thumb
_port_enable_thumb:
                // Goes into _port_unlock_thumb

                .code   16
                .global _port_unlock_thumb
_port_unlock_thumb:
                mov     r3, pc
                bx      r3
                .code   32
                msr     CPSR_c, #MODE_SYS
                bx      lr
#endif /* defined(THUMB_PRESENT) */

                .balign 16
#if defined(THUMB_PRESENT)
                .code   16
                .thumb_func
                .global _port_switch_thumb
_port_switch_thumb:
                mov     r2, pc
                bx      r2
                // Goes into _port_switch_arm in ARM mode
#endif /* defined(THUMB_PRESENT) */

                .code   32
                .global _port_switch_arm
_port_switch_arm:
                stmfd   sp!, {r4, r5, r6, r7, r8, r9, r10, r11, lr}
                str     sp, [r1, #12]
                ldr     sp, [r0, #12]
#if defined(THUMB_PRESENT)
                ldmfd   sp!, {r4, r5, r6, r7, r8, r9, r10, r11, lr}
                bx      lr
#else /* !defined(THUMB_PRESENT)T */
                ldmfd   sp!, {r4, r5, r6, r7, r8, r9, r10, r11, pc}
#endif /* !defined(THUMB_PRESENT) */

/*
 * Common IRQ code. It expects a macro ARM_IRQ_VECTOR_REG with the address
 * of a register holding the address of the ISR to be invoked, the ISR
 * then returns in the common epilogue code where the context switch will
 * be performed, if required.
 * System stack frame structure after a context switch in the
 * interrupt handler:
 *
 * High +------------+
 *      |   LR_USR   | -+
 *      |     r12    |  |
 *      |     r3     |  |
 *      |     r2     |  | External context: IRQ handler frame
 *      |     r1     |  |
 *      |     r0     |  |
 *      |   LR_IRQ   |  |   (user code return address)
 *      |   PSR_USR  | -+   (user code status)
 *      |    ....    | <- chSchDoReschedule() stack frame, optimize it for space
 *      |     LR     | -+   (system code return address)
 *      |     r11    |  |
 *      |     r10    |  |
 *      |     r9     |  |
 *      |     r8     |  | Internal context: chSysSwitch() frame
 *      |     r7     |  |
 *      |     r6     |  |
 *      |     r5     |  |
 * SP-> |     r4     | -+
 * Low  +------------+
 */
                .balign 16
                .code   32
                .global Irq_Handler
Irq_Handler:
                stmfd   sp!, {r0-r3, r12, lr}
                ldr     r0, =ARM_IRQ_VECTOR_REG
                ldr     r0, [r0]
#if !defined(THUMB_NO_INTERWORKING)
                ldr     lr, =_irq_ret_arm       // ISR return point.
                bx      r0                      // Calling the ISR.
_irq_ret_arm:
#else /* defined(THUMB_NO_INTERWORKING) */
                add     r1, pc, #1
                bx      r1
                .code   16
                bl      _bxr0                   // Calling the ISR.
                mov     lr, pc
                bx      lr
                .code   32
#endif /* defined(THUMB_NO_INTERWORKING) */
                cmp     r0, #0
                ldmfd   sp!, {r0-r3, r12, lr}
                subeqs  pc, lr, #4              // No reschedule, returns.

                // Now the frame is created in the system stack, the IRQ
                // stack is empty.
                msr     CPSR_c, #MODE_SYS | I_BIT
                stmfd   sp!, {r0-r3, r12, lr}
                msr     CPSR_c, #MODE_IRQ | I_BIT
                mrs     r0, SPSR
                mov     r1, lr
                msr     CPSR_c, #MODE_SYS | I_BIT
                stmfd   sp!, {r0, r1}           // Push R0=SPSR, R1=LR_IRQ.

                // Context switch.
#if defined(THUMB_NO_INTERWORKING)
                add     r0, pc, #1
                bx      r0
                .code   16
#if CH_DBG_SYSTEM_STATE_CHECK
                bl      _dbg_check_lock
#endif
                bl      chSchDoReschedule
#if CH_DBG_SYSTEM_STATE_CHECK
                bl      _dbg_check_unlock
#endif
                mov     lr, pc
                bx      lr
                .code   32
#else /* !defined(THUMB_NO_INTERWORKING) */
#if CH_DBG_SYSTEM_STATE_CHECK
                bl      _dbg_check_lock
#endif
                bl      chSchDoReschedule
#if CH_DBG_SYSTEM_STATE_CHECK
                bl      _dbg_check_unlock
#endif
#endif /* !defined(THUMB_NO_INTERWORKING) */

                // Re-establish the IRQ conditions again.
                ldmfd   sp!, {r0, r1}           // Pop R0=SPSR, R1=LR_IRQ.
                msr     CPSR_c, #MODE_IRQ | I_BIT
                msr     SPSR_fsxc, r0
                mov     lr, r1
                msr     CPSR_c, #MODE_SYS | I_BIT
                ldmfd   sp!, {r0-r3, r12, lr}
                msr     CPSR_c, #MODE_IRQ | I_BIT
                subs    pc, lr, #4
#if defined(THUMB_NO_INTERWORKING)
                .code   16
_bxr0:          bx      r0
#endif

/*
 * Threads trampoline code.
 * NOTE: The threads always start in ARM mode and then switches to the
 * thread-function mode.
 */
                .balign 16
                .code   32
                .globl  _port_thread_start
_port_thread_start:
#if defined(THUMB_NO_INTERWORKING)
                add     r0, pc, #1
                bx      r0
                .code   16
#if CH_DBG_SYSTEM_STATE_CHECK
                bl      _dbg_check_unlock
#endif
                bl      _port_unlock_thumb
                mov     r0, r5
                bl      _bxr4
                bl      chThdExit
_zombies:       b       _zombies
_bxr4:          bx      r4

#else /* !defined(THUMB_NO_INTERWORKING) */
#if CH_DBG_SYSTEM_STATE_CHECK
                bl      _dbg_check_unlock
#endif
                msr     CPSR_c, #MODE_SYS
                mov     r0, r5
                mov     lr, pc
                bx      r4
                mov     r0, #0              /* MSG_OK */
                bl      chThdExit
_zombies:       b       _zombies
#endif /* !defined(THUMB_NO_INTERWORKING) */

#endif /* !defined(__DOXYGEN__) */

/** @} */
