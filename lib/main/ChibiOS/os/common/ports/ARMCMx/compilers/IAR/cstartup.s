/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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
 * @file    ARMCMx/IAR/cstartup.s
 * @brief   Generic IAR Cortex-Mx startup file.
 *
 * @addtogroup ARMCMx_IAR_STARTUP
 * @{
 */

#if !defined(__DOXYGEN__)

        MODULE  ?cstartup

CONTROL_MODE_PRIVILEGED SET 0
CONTROL_MODE_UNPRIVILEGED SET 1
CONTROL_USE_MSP SET 0
CONTROL_USE_PSP SET 2

        AAPCS INTERWORK, VFP_COMPATIBLE, ROPI
        PRESERVE8

        SECTION .intvec:CODE:NOROOT(3)

        SECTION CSTACK:DATA:NOROOT(3)
        PUBLIC  __main_thread_stack_base__
__main_thread_stack_base__:
        PUBLIC  __heap_end__
__heap_end__:

        SECTION SYSHEAP:DATA:NOROOT(3)
        PUBLIC  __heap_base__
__heap_base__:

        PUBLIC  __iar_program_start
        EXTERN  __vector_table
        EXTWEAK __iar_init_core
        EXTWEAK __iar_init_vfp
        EXTERN  __cmain

        SECTION .text:CODE:REORDER(2)
        REQUIRE __vector_table
        THUMB
__iar_program_start:
        cpsid   i
        ldr     r0, =SFE(CSTACK)
        msr     PSP, r0
        movs    r0, #CONTROL_MODE_PRIVILEGED | CONTROL_USE_PSP
        msr     CONTROL, r0
        isb
        bl      __early_init
        bl      __iar_init_core
        bl      __iar_init_vfp
        b       __cmain

        SECTION .text:CODE:NOROOT:REORDER(2)
        PUBWEAK __early_init
__early_init:
        bx      lr

        END

#endif /* !defined(__DOXYGEN__) */

/**< @} */
