/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

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
 * @file    ARMCAx-TZ/compilers/GCC/monitor.S
 * @brief   ARMCAx-TZ monitor code
 *
 * @addtogroup ARM_CORE
 * @{
 */
#define TRUE    1
#define FALSE   0

#define _FROM_ASM_
#include "chlicense.h"
#include "chconf.h"
#include "armparams.h"

/*
 * We are facing an architecure with security extension exploited.
 * The following execution paths are taken by the execution units
 * running in secure state when an irq is fired (sm_irq), and in non-secure
 * state when a fiq interrupt is fired (sm_fiq).
 * They originate by the monitor irq/fiq vector and run in monitor mode,
 * ie in secure state.
 * It assumes the following, intially set at boot time, or wherever it needs:
 *   SCR.FW  == 0 and SCR.FIQ == 1 and SCR.IRQ == 0 in non-secure state,
 *      ie FIQs are taken to monitor mode, IRQs are taken locally
 *   SCR.FW  == 0 and SCR.FIQ == 0 and SCR.IRQ == 1 in secure state,
 *      ie FIQs are taken locally, IRQs are taken to monitor mode
 *   MVBAR holds the address of the monitor vectors base.
 *   The code and the stacks memory reside both in secure memory.
 */

#if !defined(__DOXYGEN__)

                .set    MODE_USR,       0x10
                .set    MODE_FIQ,       0x11
                .set    MODE_IRQ,       0x12
                .set    MODE_SVC,       0x13
                .set    MODE_MON,       0x16
                .set    MODE_ABT,       0x17
                .set    MODE_UND,       0x1B
                .set    MODE_SYS,       0x1F

                .set    SCR_NS,         0x01
                .set    SCR_IRQ,        0x02
                .set    SCR_FIQ,        0x04
                .set    SCR_EA,         0x08
                .set    SCR_FW,         0x10
                .set    SCR_AW,         0x20

                .set    MON_S_SCR,      (SCR_IRQ)           // (SCR_EA|SCR_IRQ)
                .set    MON_NS_SCR,     (SCR_FIQ|SCR_NS)

                .set    SMC_SVC_INTR,   -4

                .comm   sm_secctx, 20*4, 4
                .comm   sm_nsecctx, 20*4, 4

                .global _ns_thread
                .section .text
                .code   32
                .balign 4
/*
 * Helper macros
 */
/*
 * Store banked registers in a world area pointed by rm
 */
                .macro  sm_store_ooctx_regs rm
                // cps  #MODE_SYS              // Assume mode SYS
                stm     \rm!, {sp, lr}
                cps     #MODE_FIQ
                mrs     r12, spsr
                stm     \rm!, {r12, sp, lr}
                cps     #MODE_IRQ
                mrs     r12, spsr
                stm     \rm!, {r12, sp, lr}
                cps     #MODE_ABT
                mrs     r12, spsr
                stm     \rm!, {r12, sp, lr}
                cps     #MODE_SVC
                mrs     r12, spsr
                stm     \rm!, {r12, sp, lr}
                cps     #MODE_UND
                mrs     r12, spsr
                stm     \rm!, {r12, sp, lr}
                .endm
/*
 * Retrieve banked registers from a world area pointed by rm
 */
                .macro  sm_load_ooctx_regs rm
                // cps  #MODE_SYS              // Assume mode SYS
                ldm     \rm!, {sp, lr}
                cps     #MODE_FIQ
                ldm     \rm!, {r12, sp, lr}
                msr     spsr_fsxc, r12
                cps     #MODE_IRQ
                ldm     \rm!, {r12, sp, lr}
                msr     spsr_fsxc, r12
                cps     #MODE_ABT
                ldm     \rm!, {r12, sp, lr}
                msr     spsr_fsxc, r12
                cps     #MODE_SVC
                ldm     \rm!, {r12, sp, lr}
                msr     spsr_fsxc, r12
                cps     #MODE_UND
                ldm     \rm!, {r12, sp, lr}
                msr     spsr_fsxc, r12
                .endm
/*
 * Monitor vectors
 */
				.global _monitor_vectors
				.balign 32
_monitor_vectors:
                b   .           // Reset vector, not used
                b   .           // Undefined instruction, not used
                b   sm_call     // Secure monitor call
                b   .           // Prefetch abort, not taken to Monitor mode
                b   .           // Data abort, not taken to Monitor mode
                b   .           // Reserved
                b   sm_irq      // IRQ
                b   sm_fiq      // FIQ
/*
 * SMC entry
 */
sm_call:
                stmfd   sp!, {r4, r12}
                ldr     r12, =MON_S_SCR         // enter in the secure world
                mcr     p15, 0, r12, c1, c1, 0
                ands    r0, r0                  // OS special service,
                                                // 0 == jump trampoline to non secure world
                                                // r1 contains the address where it jumps
                beq     1f

                mrs     r4, SPSR
                mov     r12, lr
                stmfd   sp!, {r4, r12}          // push r4=spsr_mon, r12=lr_mon.

                cps     #MODE_SYS               // switch to sys mode, ints disabled
                ldr     r4, =sm_nsecctx
                sm_store_ooctx_regs r4

                cps     #MODE_SYS
                ldr     r4, =sm_secctx
                sm_load_ooctx_regs r4

                cps     #MODE_SYS
                bl      smcEntry                // call the C smc handler

                ldr     r4, =sm_secctx
                sm_store_ooctx_regs r4

                cps     #MODE_SYS
                ldr     r4, =sm_nsecctx
                sm_load_ooctx_regs r4

                cps     #MODE_MON           // switch to monitor mode

                ldmfd   sp!, {r4, r12}          // pop r4=spsr_mon, r12=lr_mon.
                msr     SPSR_fsxc, r4
                mov     lr, r12

                ldr     r12, =MON_NS_SCR        // enter in the non-secure world
                mcr     p15, 0, r12, c1, c1, 0
                ldmfd   sp!, {r4, r12}
                subs    pc, lr, #0              // return from smc
1:
                mov     lr, r1                  // use the address in r1 as return address
                                                // in the non secure world
                ldr     r12, =MON_NS_SCR        // enter in the non-secure world
                mcr     p15, 0, r12, c1, c1, 0
                ldmfd   sp!, {r4, r12}
                subs    pc, lr, #0              // return from smc

/*
 * FIQ entry
 *
 * Here the fiq is taken from non-secure state, via the FIQ vector
 * that is in the monitor vector table.
 * Current mode is monitor (so current state is secure).
 * We switch immediately to system mode, enabling FIQ.
 * The FIQ is then served while the ns_thread is running.
 * Because the ns_thread has the highest priority, the handler returns here
 * without scheduling.
 */
sm_fiq:
                                                // check point: SCR.NS == 1
                stmfd   sp!, {r0, r12}
                ldr     r0, =MON_S_SCR          // enter in the secure world
                mcr     p15, 0, r0, c1, c1, 0

                cps     #MODE_SYS
                ldr     r0, =sm_nsecctx
                sm_store_ooctx_regs r0

                cps     #MODE_SYS
                ldr     r0, =sm_secctx
                sm_load_ooctx_regs r0

                cpsie   f, #MODE_SYS            // FIQ enabled, served via base table

                cpsid   f, #MODE_SYS            // the handler returns here, FIQ disabled.
                ldr     r0, =sm_secctx
                sm_store_ooctx_regs r0

                cps     #MODE_SYS
                ldr     r0, =sm_nsecctx
                sm_load_ooctx_regs r0

                cps     #MODE_MON               // switch to monitor mode
                ldr     r0, =MON_NS_SCR         // set non-secure SCR before return
                mcr     p15, 0, r0, c1, c1, 0
                ldmfd   sp!, {r0, r12}
                subs    pc, lr, #4              // return into non-secure world
/*
 * IRQ entry
 *
 * Here the IRQ is taken from secure state.
 * Current mode is monitor (so current state is secure),
 * the previous mode and status is in spsr_mon and
 * the return address+4 is in lr_mon.
 * Because we are running in secure state, we are sure that
 * the main thread is suspended in the smc handler.
 * The main thread is then resumed with SMC_SVC_INTR
 * The non secure world has then the responsibility to return into
 * secure state via a smc.
 *
 */
sm_irq:
                                                // check point: SCR.NS == 0
                cps     #MODE_SYS
                stmfd   sp!, {r0-r3, r12, lr}   // save scratch registers and lr
                cps     #MODE_MON               // switch to monitor mode
                mrs     r0, spsr
                mov     r1, lr
                cps     #MODE_SYS
                stmfd   sp!, {r0, r1}           // push r0=spsr_mon, r1=lr_mon.
                                                // check point: ns_tread != 0
#if (CH_DBG_SYSTEM_STATE_CHECK == TRUE)
                bl      _dbg_check_lock
#endif
                ldr     r0, =_ns_thread
                mov     r1, #SMC_SVC_INTR
                bl      chThdResumeS            // resume the ns_thread and serve the IRQ
                                                // into non-secure world
#if (CH_DBG_SYSTEM_STATE_CHECK == TRUE)
                bl      _dbg_check_unlock
#endif
                // The ns_thread reentered smc, that set SRC.NS to 0
                // re-establish the original conditions
                ldmfd   sp!, {r0, r1}           // pop r0=spsr_mon, r1=lr_mon.
                cps     #MODE_MON               // switch to monitor mode
                msr     SPSR_fsxc, r0
                mov     lr, r1
                cps     #MODE_SYS
                ldmfd   sp!, {r0-r3, r12, lr}
                cps     #MODE_MON               // switch to monitor mode
                subs    pc, lr, #4              // return into secure world

                .global _ns_trampoline
_ns_trampoline:
                mov     r1, r0

                ldr     r0, =sm_secctx
                sm_store_ooctx_regs r0
                cps     #MODE_SYS

                ldr     r0, =#0
/*              mov     r2, r0  */              // r2 could contain the address of the fdt
                mov     r3, r0
                mov     r4, r0
                mov     r5, r0
                mov     r6, r0
                mov     r7, r0
                mov     r8, r0
                mov     r9, r0
                mov     r10, r0
                mov     r11, r0
                mov     r12, r0
                mov     sp, r0
                mov     lr, r0
                smc     #0

#endif /* !defined(__DOXYGEN__) */

/** @} */
