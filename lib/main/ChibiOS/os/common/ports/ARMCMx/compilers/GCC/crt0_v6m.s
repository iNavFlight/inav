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
 * @file    crt0_v6m.s
 * @brief   Generic ARMv6-M (Cortex-M0/M1) startup file for ChibiOS.
 *
 * @addtogroup ARMCMx_GCC_STARTUP_V6M
 * @{
 */

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

#if !defined(FALSE) || defined(__DOXYGEN__)
#define FALSE                               0
#endif

#if !defined(TRUE) || defined(__DOXYGEN__)
#define TRUE                                1
#endif

#define CONTROL_MODE_PRIVILEGED             0
#define CONTROL_MODE_UNPRIVILEGED           1
#define CONTROL_USE_MSP                     0
#define CONTROL_USE_PSP                     2

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   Control special register initialization value.
 * @details The system is setup to run in privileged mode using the PSP
 *          stack (dual stack mode).
 */
#if !defined(CRT0_CONTROL_INIT) || defined(__DOXYGEN__)
#define CRT0_CONTROL_INIT                   (CONTROL_USE_PSP |              \
                                             CONTROL_MODE_PRIVILEGED)
#endif

/**
 * @brief   Core initialization switch.
 */
#if !defined(CRT0_INIT_CORE) || defined(__DOXYGEN__)
#define CRT0_INIT_CORE                      TRUE
#endif

/**
 * @brief   Stack segments initialization switch.
 */
#if !defined(CRT0_STACKS_FILL_PATTERN) || defined(__DOXYGEN__)
#define CRT0_STACKS_FILL_PATTERN            0x55555555
#endif

/**
 * @brief   Stack segments initialization switch.
 */
#if !defined(CRT0_INIT_STACKS) || defined(__DOXYGEN__)
#define CRT0_INIT_STACKS                    TRUE
#endif

/**
 * @brief   DATA segment initialization switch.
 */
#if !defined(CRT0_INIT_DATA) || defined(__DOXYGEN__)
#define CRT0_INIT_DATA                      TRUE
#endif

/**
 * @brief   BSS segment initialization switch.
 */
#if !defined(CRT0_INIT_BSS) || defined(__DOXYGEN__)
#define CRT0_INIT_BSS                       TRUE
#endif

/**
 * @brief   RAM areas initialization switch.
 */
#if !defined(CRT0_INIT_RAM_AREAS) || defined(__DOXYGEN__)
#define CRT0_INIT_RAM_AREAS                 TRUE
#endif

/**
 * @brief   Constructors invocation switch.
 */
#if !defined(CRT0_CALL_CONSTRUCTORS) || defined(__DOXYGEN__)
#define CRT0_CALL_CONSTRUCTORS              TRUE
#endif

/**
 * @brief   Destructors invocation switch.
 */
#if !defined(CRT0_CALL_DESTRUCTORS) || defined(__DOXYGEN__)
#define CRT0_CALL_DESTRUCTORS               TRUE
#endif

/*===========================================================================*/
/* Code section.                                                             */
/*===========================================================================*/

#if !defined(__DOXYGEN__)

                .cpu    cortex-m0
                .fpu    softvfp

                .thumb
                .text

/*
 * Reset handler.
 */
                .align  2
                .thumb_func
                .global Reset_Handler
Reset_Handler:
                /* Interrupts are globally masked initially.*/
                cpsid   i

                /* PSP stack pointers initialization.*/
                ldr     r0, =__process_stack_end__
                msr     PSP, r0

                /* CPU mode initialization as configured.*/
                movs    r0, #CRT0_CONTROL_INIT
                msr     CONTROL, r0
                isb

#if CRT0_INIT_CORE == TRUE
                /* Core initialization.*/
                bl      __core_init
#endif

                /* Early initialization..*/
                bl      __early_init

#if CRT0_INIT_STACKS == TRUE
                ldr     r0, =CRT0_STACKS_FILL_PATTERN
                /* Main Stack initialization. Note, it assumes that the
                   stack size is a multiple of 4 so the linker file must
                   ensure this.*/
                ldr     r1, =__main_stack_base__
                ldr     r2, =__main_stack_end__
msloop:
                cmp     r1, r2
                bge     endmsloop
                str     r0, [r1]
                add     r1, r1, #4
                b       msloop
endmsloop:
                /* Process Stack initialization. Note, it assumes that the
                   stack size is a multiple of 4 so the linker file must
                   ensure this.*/
                ldr     r1, =__process_stack_base__
                ldr     r2, =__process_stack_end__
psloop:
                cmp     r1, r2
                bge     endpsloop
                str     r0, [r1]
                add     r1, r1, #4
                b       psloop
endpsloop:
#endif

#if CRT0_INIT_DATA == TRUE
                /* Data initialization. Note, it assumes that the DATA size
                  is a multiple of 4 so the linker file must ensure this.*/
                ldr     r1, =_textdata
                ldr     r2, =_data
                ldr     r3, =_edata
dloop:
                cmp     r2, r3
                bge     enddloop
                ldr     r0, [r1]
                str     r0, [r2]
                add     r1, r1, #4
                add     r2, r2, #4
                b       dloop
enddloop:
#endif

#if CRT0_INIT_BSS == TRUE
                /* BSS initialization. Note, it assumes that the DATA size
                  is a multiple of 4 so the linker file must ensure this.*/
                movs    r0, #0
                ldr     r1, =_bss_start
                ldr     r2, =_bss_end
bloop:
                cmp     r1, r2
                bge     endbloop
                str     r0, [r1]
                add     r1, r1, #4
                b       bloop
endbloop:
#endif

#if CRT0_INIT_RAM_AREAS == TRUE
                /* RAM areas initialization.*/
                bl      __init_ram_areas
#endif

                /* Late initialization..*/
                bl      __late_init

#if CRT0_CALL_CONSTRUCTORS == TRUE
                /* Constructors invocation.*/
                ldr     r4, =__init_array_start
                ldr     r5, =__init_array_end
initloop:
                cmp     r4, r5
                bge     endinitloop
                ldr     r1, [r4]
                blx     r1
                add     r4, r4, #4
                b       initloop
endinitloop:
#endif

                /* Main program invocation, r0 contains the returned value.*/
                bl      main

#if CRT0_CALL_DESTRUCTORS == TRUE
                /* Destructors invocation.*/
                ldr     r4, =__fini_array_start
                ldr     r5, =__fini_array_end
finiloop:
                cmp     r4, r5
                bge     endfiniloop
                ldr     r1, [r4]
                blx     r1
                add     r4, r4, #4
                b       finiloop
endfiniloop:
#endif

                /* Branching to the defined exit handler.*/
                ldr     r1, =__default_exit
                bx      r1

#endif

/** @} */
