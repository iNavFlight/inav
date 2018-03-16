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
 * @file    ivor.s
 * @brief   Kernel ISRs.
 *
 * @addtogroup PPC_GCC_CORE
 * @{
 */

#if !defined(FALSE) || defined(__DOXYGEN__)
#define FALSE   0
#endif

#if !defined(TRUE) || defined(__DOXYGEN__)
#define TRUE    1
#endif

/*
 * Imports the PPC configuration headers.
 */
#define _FROM_ASM_
#include "nilconf.h"
#include "nilcore.h"

#if !defined(__DOXYGEN__)

        .section    .handlers, "ax"

#if PPC_SUPPORTS_DECREMENTER
        /*
         * _IVOR10 handler (Book-E decrementer).
         */
        .align      4
        .globl      _IVOR10
        .type       _IVOR10, @function
_IVOR10:
        /* Saving the external context (port_extctx structure).*/
        stwu        %sp, -80(%sp)
#if PPC_USE_VLE && PPC_SUPPORTS_VLE_MULTI
        e_stmvsrrw  8(%sp)                  /* Saves PC, MSR.               */
        e_stmvsprw  16(%sp)                 /* Saves CR, LR, CTR, XER.      */
        e_stmvgprw  32(%sp)                 /* Saves GPR0, GPR3...GPR12.    */
#else /* !(PPC_USE_VLE && PPC_SUPPORTS_VLE_MULTI) */
        stw         %r0, 32(%sp)            /* Saves GPR0.                  */
        mfSRR0      %r0
        stw         %r0, 8(%sp)             /* Saves PC.                    */
        mfSRR1      %r0
        stw         %r0, 12(%sp)            /* Saves MSR.                   */
        mfCR        %r0
        stw         %r0, 16(%sp)            /* Saves CR.                    */
        mfLR        %r0
        stw         %r0, 20(%sp)            /* Saves LR.                    */
        mfCTR       %r0
        stw         %r0, 24(%sp)            /* Saves CTR.                   */
        mfXER       %r0
        stw         %r0, 28(%sp)            /* Saves XER.                   */
        stw         %r3, 36(%sp)            /* Saves GPR3...GPR12.          */
        stw         %r4, 40(%sp)
        stw         %r5, 44(%sp)
        stw         %r6, 48(%sp)
        stw         %r7, 52(%sp)
        stw         %r8, 56(%sp)
        stw         %r9, 60(%sp)
        stw         %r10, 64(%sp)
        stw         %r11, 68(%sp)
        stw         %r12, 72(%sp)
#endif /* !(PPC_USE_VLE && PPC_SUPPORTS_VLE_MULTI) */

        /* Increasing the SPGR0 register.*/
        mfspr       %r0, 272
        eaddi       %r0, %r0, 1
        mtspr       272, %r0

        /* Reset DIE bit in TSR register.*/
        lis         %r3, 0x0800             /* DIS bit mask.                */
        mtspr       336, %r3                /* TSR register.                */

        /* Restoring pre-IRQ MSR register value.*/
        mfSRR1      %r0
#if !PPC_USE_IRQ_PREEMPTION
        /* No preemption, keeping EE disabled.*/
        se_bclri    %r0, 16                 /* EE = bit 16.                 */
#endif
        mtMSR       %r0

        /* System tick handler invocation.*/
        bl          chSysTimerHandlerI

#if PPC_USE_IRQ_PREEMPTION
        /* Prevents preemption again.*/
        wrteei      0
#endif

        /* Jumps to the common IVOR epilogue code.*/
        b           _ivor_exit
#endif /* PPC_SUPPORTS_DECREMENTER */

        /*
         * _IVOR4 handler (Book-E external interrupt).
         */
        .align      4
        .globl      _IVOR4
        .type       _IVOR4, @function
_IVOR4:
        /* Saving the external context (port_extctx structure).*/
        stwu        %sp, -80(%sp)
#if PPC_USE_VLE && PPC_SUPPORTS_VLE_MULTI
        e_stmvsrrw  8(%sp)                  /* Saves PC, MSR.               */
        e_stmvsprw  16(%sp)                 /* Saves CR, LR, CTR, XER.      */
        e_stmvgprw  32(%sp)                 /* Saves GPR0, GPR3...GPR12.    */
#else /* !(PPC_USE_VLE && PPC_SUPPORTS_VLE_MULTI) */
        stw         %r0, 32(%sp)            /* Saves GPR0.                  */
        mfSRR0      %r0
        stw         %r0, 8(%sp)             /* Saves PC.                    */
        mfSRR1      %r0
        stw         %r0, 12(%sp)            /* Saves MSR.                   */
        mfCR        %r0
        stw         %r0, 16(%sp)            /* Saves CR.                    */
        mfLR        %r0
        stw         %r0, 20(%sp)            /* Saves LR.                    */
        mfCTR       %r0
        stw         %r0, 24(%sp)            /* Saves CTR.                   */
        mfXER       %r0
        stw         %r0, 28(%sp)            /* Saves XER.                   */
        stw         %r3, 36(%sp)            /* Saves GPR3...GPR12.          */
        stw         %r4, 40(%sp)
        stw         %r5, 44(%sp)
        stw         %r6, 48(%sp)
        stw         %r7, 52(%sp)
        stw         %r8, 56(%sp)
        stw         %r9, 60(%sp)
        stw         %r10, 64(%sp)
        stw         %r11, 68(%sp)
        stw         %r12, 72(%sp)
#endif /* !(PPC_USE_VLE && PPC_SUPPORTS_VLE_MULTI) */

        /* Increasing the SPGR0 register.*/
        mfspr       %r0, 272
        eaddi       %r0, %r0, 1
        mtspr       272, %r0

        /* Software vector address from the INTC register.*/
        lis         %r3, INTC_IACKR_ADDR@h
        ori         %r3, %r3, INTC_IACKR_ADDR@l
        lwz         %r3, 0(%r3)             /* IACKR register value.        */
        lwz         %r3, 0(%r3)
        mtCTR       %r3                     /* Software handler address.    */

        /* Restoring pre-IRQ MSR register value.*/
        mfSRR1      %r0
#if !PPC_USE_IRQ_PREEMPTION
        /* No preemption, keeping EE disabled.*/
        se_bclri    %r0, 16                 /* EE = bit 16.                 */
#endif
        mtMSR       %r0

        /* Exectes the software handler.*/
        bctrl

#if PPC_USE_IRQ_PREEMPTION
        /* Prevents preemption again.*/
        wrteei      0
#endif

        /* Informs the INTC that the interrupt has been served.*/
        mbar        0
        lis         %r3, INTC_EOIR_ADDR@h
        ori         %r3, %r3, INTC_EOIR_ADDR@l
        stw         %r3, 0(%r3)             /* Writing any value should do. */

        /* Common IVOR epilogue code, context restore.*/
        .globl      _ivor_exit
_ivor_exit:
        /* Decreasing the SPGR0 register.*/
        mfspr       %r0, 272
        eaddi       %r0, %r0, -1
        mtspr       272, %r0

        bl          chSchRescheduleS

        /* Restoring the external context.*/
#if PPC_USE_VLE && PPC_SUPPORTS_VLE_MULTI
        e_lmvgprw   32(%sp)                 /* Restores GPR0, GPR3...GPR12. */
        e_lmvsprw   16(%sp)                 /* Restores CR, LR, CTR, XER.   */
        e_lmvsrrw   8(%sp)                  /* Restores PC, MSR.            */
#else /*!(PPC_USE_VLE && PPC_SUPPORTS_VLE_MULTI) */
        lwz         %r3, 36(%sp)            /* Restores GPR3...GPR12.       */
        lwz         %r4, 40(%sp)
        lwz         %r5, 44(%sp)
        lwz         %r6, 48(%sp)
        lwz         %r7, 52(%sp)
        lwz         %r8, 56(%sp)
        lwz         %r9, 60(%sp)
        lwz         %r10, 64(%sp)
        lwz         %r11, 68(%sp)
        lwz         %r12, 72(%sp)
        lwz         %r0, 8(%sp)
        mtSRR0      %r0                     /* Restores PC.                 */
        lwz         %r0, 12(%sp)
        mtSRR1      %r0                     /* Restores MSR.                */
        lwz         %r0, 16(%sp)
        mtCR        %r0                     /* Restores CR.                 */
        lwz         %r0, 20(%sp)
        mtLR        %r0                     /* Restores LR.                 */
        lwz         %r0, 24(%sp)
        mtCTR       %r0                     /* Restores CTR.                */
        lwz         %r0, 28(%sp)
        mtXER       %r0                     /* Restores XER.                */
        lwz         %r0, 32(%sp)            /* Restores GPR0.               */
#endif /* !(PPC_USE_VLE && PPC_SUPPORTS_VLE_MULTI) */
        addi        %sp, %sp, 80            /* Back to the previous frame.  */
        rfi

#endif /* !defined(__DOXYGEN__) */

/** @} */
