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
 * @file    GCC/crt0.s
 * @brief   Generic PowerPC startup file for GCC.
 *
 * @addtogroup PPC_GCC_CORE
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

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

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

        .section    .crt0, "ax"
        .align		2
        .globl      _boot_address
        .type       _boot_address, @function
_boot_address:
        /* Stack setup.*/
        lis         %r1, __process_stack_end__@h
        ori         %r1, %r1, __process_stack_end__@l
        li          %r0, 0
        stwu        %r0, -8(%r1)

        /* Small sections registers initialization.*/
        lis         %r2, __sdata2_start__@h
        ori         %r2, %r2, __sdata2_start__@l
        lis         %r13, __sdata_start__@h
        ori         %r13, %r13, __sdata_start__@l

        /* Early initialization.*/
        bl          __early_init

#if CRT0_INIT_STACKS == TRUE
        /* Stacks fill pattern.*/
        lis         %r7, CRT0_STACKS_FILL_PATTERN@h
        ori         %r7, %r7, CRT0_STACKS_FILL_PATTERN@l

       /* IRQ Stack initialization. Note, the architecture does not use this
          stack, the size is usually zero. An OS can have special SW handling
          and require this. A 4 bytes alignment is assmend and required.*/
        lis         %r4, __irq_stack_base__@h
        ori         %r4, %r4, __irq_stack_base__@l
        lis         %r5, __irq_stack_end__@h
        ori         %r5, %r5, __irq_stack_end__@l
.irqsloop:
        cmpl        cr0, %r4, %r5
        bge         cr0, .irqsend
        stw         %r7, 0(%r4)
        addi        %r4, %r4, 4
        b           .irqsloop
.irqsend:

        /* Process Stack initialization. Note, does not overwrite the already
           written EABI frame. A 4 bytes alignment is assmend and required.*/
        lis         %r4, __process_stack_base__@h
        ori         %r4, %r4, __process_stack_base__@l
        lis         %r5, (__process_stack_end__ - 8)@h
        ori         %r5, %r5, (__process_stack_end__ - 8)@l
.prcsloop:
        cmpl        cr0, %r4, %r5
        bge         cr0, .prcsend
        stw         %r7, 0(%r4)
        addi        %r4, %r4, 4
        b           .prcsloop
.prcsend:
#endif

#if CRT0_INIT_BSS == TRUE
        /* BSS clearing.*/
        lis         %r4, __bss_start__@h
        ori         %r4, %r4, __bss_start__@l
        lis         %r5, __bss_end__@h
        ori         %r5, %r5, __bss_end__@l
        li          %r7, 0
.bssloop:
        cmpl        cr0, %r4, %r5
        bge         cr0, .bssend
        stw         %r7, 0(%r4)
        addi        %r4, %r4, 4
        b           .bssloop
.bssend:
#endif

#if CRT0_INIT_DATA == TRUE
        /* DATA initialization.*/
        lis         %r4, __romdata_start__@h
        ori         %r4, %r4, __romdata_start__@l
        lis         %r5, __data_start__@h
        ori         %r5, %r5, __data_start__@l
        lis         %r6, __data_end__@h
        ori         %r6, %r6, __data_end__@l
.dataloop:
        cmpl        cr0, %r5, %r6
        bge         cr0, .dataend
        lwz         %r7, 0(%r4)
        addi        %r4, %r4, 4
        stw         %r7, 0(%r5)
        addi        %r5, %r5, 4
        b           .dataloop
.dataend:
#endif

        /* Late initialization.*/
        bl          __late_init

#if CRT0_CALL_CONSTRUCTORS == TRUE
        /* Constructors invocation.*/
        lis         %r4, __init_array_start@h
        ori         %r4, %r4, __init_array_start@l
        lis         %r5, __init_array_end@h
        ori         %r5, %r5, __init_array_end@l
.iniloop:
        cmplw       %cr0, %r4, %r5
        bge         %cr0, .iniend
        lwz         %r6, 0(%r4)
        mtctr       %r6
        addi        %r4, %r4, 4
        bctrl
        b           .iniloop
.iniend:
#endif

        /* Main program invocation.*/
        bl          main

#if CRT0_CALL_DESTRUCTORS == TRUE
        /* Destructors invocation.*/
        lis         %r4, __fini_array_start@h
        ori         %r4, %r4, __fini_array_start@l
        lis         %r5, __fini_array_end@h
        ori         %r5, %r5, __fini_array_end@l
.finiloop:
        cmplw       %cr0, %r4, %r5
        bge         %cr0, .finiend
        lwz         %r6, 0(%r4)
        mtctr       %r6
        addi        %r4, %r4, 4
        bctrl
        b           .finiloop
.finiend:
#endif

        /* Branching to the defined exit handler.*/
        b           __default_exit

        /* Default main exit code, infinite loop.*/
        .weak       __default_exit
        .type       __default_exit, @function
__default_exit:
        b           __default_exit

        /* Default early initialization code, none.*/
        .weak       __early_init
        .type       __early_init, @function
__early_init:
        blr

        /* Default late initialization code, none.*/
        .weak       __late_init
        .type       __late_init, @function
__late_init:
        blr

#endif /* !defined(__DOXYGEN__) */

/** @} */
