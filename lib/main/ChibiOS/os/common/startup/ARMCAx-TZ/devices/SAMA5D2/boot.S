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
 * @file    SAMA5D2/boot.S
 * @brief   SAMA5D2 boot-related code managing the trusted zone.
 *
 * @addtogroup SAMA5D2_BOOT
 * @{
 */
#if !defined(__DOXYGEN__)

                .set    SCR_NS,         0x01
                .set    SCR_IRQ,        0x02
                .set    SCR_FIQ,        0x04
                .set    SCR_EA,         0x08
                .set    SCR_FW,         0x10
                .set    SCR_AW,         0x20
                .set    AIC_REDIR_KEY,  0x5B6C0E26 << 1
                .set    SFR_SN1, 	    0xF8030050
                .set	SFR_AICREDIR,   0xF8030054
                .set    SFR_L2CC_HRAMC, 0xF8030058
                .set    L2CC_CR,        0x00A00100
                .set    SCR_RESET_VAL,  (SCR_EA|SCR_IRQ)

                .section .boot
                .code   32
                .balign 4
/*
 * Boot initialization code
 */
				.global Boot_Handler
Boot_Handler:
                /*
                 * Set VBAR to system vectors table
                 * Set MVBAR to monitor vectors table
                 */
                ldr     r0, =_start
                mcr     p15, 0, r0, c12, c0, 0
                ldr     r0, =_monitor_vectors
                mcr     p15, 0, r0, c12, c0, 1
                /*
                 * Do not redirect secure interrupts to AIC
                 */
                ldr     r0, =AIC_REDIR_KEY
                ldr     r1, =SFR_SN1
                ldr     r1, [r1]
                eor     r0, r0, r1
                bic     r0, r0, #0x1
                ldr     r1, =SFR_AICREDIR
                str     r0, [r1]

#if defined(ARM_RESET_SYS_CTRL)
                /*
                 * Reset SCTLR Settings
                 */
                mrc     p15, 0, r0, c1, c0, 0       // Read CP15 System Control register
                bic     r0, r0, #(0x1 << 12)        // Clear I bit 12 to disable I Cache
                bic     r0, r0, #(0x1 <<  2)        // Clear C bit  2 to disable D Cache
                bic     r0, r0, #0x1                // Clear M bit  0 to disable MMU
                bic     r0, r0, #(0x1 << 11)        // Clear Z bit 11 to disable branch prediction
                bic     r0, r0, #(0x1 << 13)        // Clear V bit 13 to disable hivecs
                mcr     p15, 0, r0, c1, c0, 0       // Write value back to CP15 System Control register
                isb
                /*
                 * Turn off L2Cache
                 */
                bic     r0, r0, #0x1
                ldr     r1, =L2CC_CR
                str     r0, [r1]
                /*
                 * Configure the L2 cache to be used as an internal SRAM
                 */
                bic     r0, r0, #0x1
                ldr     r1, =SFR_L2CC_HRAMC
                str     r0, [r1]
#endif
                /*
                 * Enabling Cycle counter
                 */
                mrc     p15, 0, r0, c9, c12, 0      // read PMCR register
                orr     r0, r0, #(0x1)              // set E bit 0 to enable counter
                mcr     p15, 0, r0, c9, c12, 0      // write r0
                mrc     p15, 0, r0, c9, c12, 1      // read PMCNTENSET register
                orr     r0, r0, #(0x1 << 31)        // set bit 31 to enable counter
                mcr     p15, 0, r0, c9, c12, 1      // write r0
                /*
                 * Configure the intial catching of the interrupts
                 */
                ldr     r0, =SCR_RESET_VAL          // IRQ and external ABT to monitor in secure mode
                mcr     p15, 0, r0, c1, c1, 0
                b       Reset_Handler
#endif /* !defined(__DOXYGEN__) */

/** @} */
