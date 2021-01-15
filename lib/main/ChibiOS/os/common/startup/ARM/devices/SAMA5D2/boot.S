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
 * @brief   SAMA5D2 boot-related code.
 *
 * @addtogroup SAMA5D2_BOOT
 * @{
 */
#define FALSE 0
#define TRUE  1
#define _FROM_ASM_
#include "chlicense.h"
#include "chconf.h"
#include "armparams.h"

#if !defined(__DOXYGEN__)

                .set    AIC_REDIR_KEY,  0x5B6C0E26 << 1
                .set    SFR_SN1, 	    0xF8030050
                .set	SFR_AICREDIR,   0xF8030054
                .set    SFR_L2CC_HRAMC, 0xF8030058
                .set    L2CC_CR,        0x00A00100

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
                 */
                ldr     r0, =_start
                mcr     p15, 0, r0, c12, c0, 0
                /*
                 * Reset SCTLR Settings
                 */
                mrc     p15, 0, r0, c1, c0, 0       // Read CP15 System Control register
#if defined(ARM_RESET_SYS_CTRL)
                bic     r0, r0, #(0x1 << 12)        // Clear I bit 12 to disable I Cache
                bic     r0, r0, #(0x1 <<  2)        // Clear C bit  2 to disable D Cache
                bic     r0, r0, #0x1                // Clear M bit  0 to disable MMU
                bic     r0, r0, #(0x1 << 11)        // Clear Z bit 11 to disable branch prediction
#endif
                bic     r0, r0, #(0x1 << 13)        // Clear V bit 13 to disable hivecs
                mcr     p15, 0, r0, c1, c0, 0       // Write value back to CP15 System Control register
                isb
                b       Reset_Handler
#endif /* !defined(__DOXYGEN__) */

/** @} */
