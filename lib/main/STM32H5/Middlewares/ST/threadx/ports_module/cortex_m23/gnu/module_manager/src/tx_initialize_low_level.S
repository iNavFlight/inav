/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** ThreadX Component                                                     */
/**                                                                       */
/**   Initialize                                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


SYSTEM_CLOCK        =     6000000
SYSTICK_CYCLES      =     ((SYSTEM_CLOCK / 100) -1)

/* Setup the stack and heap areas.  */

STACK_SIZE          =     0x00000400
HEAP_SIZE           =     0x00000000


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_initialize_low_level                          Cortex-M23/GNU    */
/*                                                            6.1         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is responsible for any low-level processor            */
/*    initialization, including setting up interrupt vectors, setting     */
/*    up a periodic timer interrupt source, saving the system stack       */
/*    pointer for use in ISR processing later, and finding the first      */
/*    available RAM memory address for tx_application_define.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_initialize_kernel_enter           ThreadX entry function        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
// VOID   _tx_initialize_low_level(VOID)
// {
    .section .text
    .balign 4
    .syntax unified
    .eabi_attribute Tag_ABI_align_preserved, 1
    .global  _tx_initialize_low_level
    .thumb_func
.type _tx_initialize_low_level, function
_tx_initialize_low_level:

    /* Disable interrupts during ThreadX initialization.  */
    CPSID   i

    /* Set base of available memory to end of non-initialised RAM area.  */
    LDR     r0, =_tx_initialize_unused_memory       // Build address of unused memory pointer
    LDR     r1, =Image$$ARM_LIB_STACK$$ZI$$Limit    // Build first free address
    ADDS    r1, r1, #4                              // 
    STR     r1, [r0]                                // Setup first unused memory pointer

    /* Setup Vector Table Offset Register.  */
    LDR     r0, =0xE000ED08                         // Build address of NVIC registers
    LDR     r1, =__Vectors                          // Pickup address of vector table
    STR     r1, [r0]                                // Set vector table address   

//    /* Enable the cycle count register.  */
//
//    LDR     r0, =0xE0001000                         // Build address of DWT register
//    LDR     r1, [r0]                                // Pickup the current value
//    ORR     r1, r1, #1                              // Set the CYCCNTENA bit
//    STR     r1, [r0]                                // Enable the cycle count register 

    /* Set system stack pointer from vector value.  */
    LDR     r0, =_tx_thread_system_stack_ptr        // Build address of system stack pointer
    LDR     r1, =__Vectors                          // Pickup address of vector table
    LDR     r1, [r1]                                // Pickup reset stack pointer
    STR     r1, [r0]                                // Save system stack pointer

    /* Configure SysTick.  */
    LDR     r0, =0xE000E000                         // Build address of NVIC registers
    LDR     r1, =SYSTICK_CYCLES
    STR     r1, [r0, #0x14]                         // Setup SysTick Reload Value
    MOVW    r1, #0x7                                // Build SysTick Control Enable Value
    STR     r1, [r0, #0x10]                         // Setup SysTick Control

    /* Configure handler priorities.  */
    LDR     r1, =0x00000000                         // Rsrv, UsgF, BusF, MemM
    LDR     r0, =0xE000E000                         // Build address of NVIC registers
    LDR     r2, =0xD18                              // 
    ADD     r0, r0, r2                              // 
    STR     r1, [r0]                                // Setup System Handlers 4-7 Priority Registers

    LDR     r1, =0xFF000000                         // SVCl, Rsrv, Rsrv, Rsrv
    LDR     r0, =0xE000E000                         // Build address of NVIC registers
    LDR     r2, =0xD1C                              // 
    ADD     r0, r0, r2                              // 
    STR     r1, [r0]                                // Setup System Handlers 8-11 Priority Registers
                                                    // Note: SVC must be lowest priority, which is 0xFF

    LDR     r1, =0x40FF0000                         // SysT, PnSV, Rsrv, DbgM
    LDR     r0, =0xE000E000                         // Build address of NVIC registers
    LDR     r2, =0xD20                              // 
    ADD     r0, r0, r2                              // 
    STR     r1, [r0]                                // Setup System Handlers 12-15 Priority Registers
                                                    // Note: PnSV must be lowest priority, which is 0xFF

    /* Return to caller.  */
    BX      lr
// }


/* Define shells for each of the unused vectors.  */
    .section .text
    .balign 4
    .syntax unified
    .eabi_attribute Tag_ABI_align_preserved, 1
    .global  __tx_BadHandler
    .thumb_func
.type __tx_BadHandler, function
__tx_BadHandler:
    B       __tx_BadHandler


    .section .text
    .balign 4
    .syntax unified
    .eabi_attribute Tag_ABI_align_preserved, 1
    .global  __tx_IntHandler
    .thumb_func
.type __tx_IntHandler, function
__tx_IntHandler:
// VOID InterruptHandler (VOID)
// {
    PUSH    {r0,lr}     // Save LR (and dummy r0 to maintain stack alignment)
    
    /* Do interrupt handler work here */
    /* .... */

    POP     {r0, r1}
    MOV     lr, r1
    BX      lr
// }


    .section .text
    .balign 4
    .syntax unified
    .eabi_attribute Tag_ABI_align_preserved, 1
    .global  SysTick_Handler
    .thumb_func
.type SysTick_Handler, function
SysTick_Handler:
// VOID TimerInterruptHandler (VOID)
// {
    PUSH    {r0,lr}     // Save LR (and dummy r0 to maintain stack alignment)
    BL      _tx_timer_interrupt
    POP     {r0, r1}
    MOV     lr, r1
    BX      lr
// }


    .section .text
    .balign 4
    .syntax unified
    .eabi_attribute Tag_ABI_align_preserved, 1
    .global  HardFault_Handler
    .thumb_func
.type HardFault_Handler, function
HardFault_Handler:
    // A stack overflow will trigger a hardfault.
    // There is no CFSR in M23, so we will not try to
    // determine if the fault is caused by a stack overflow
    // or some other condition. 
    B       HardFault_Handler

    .end
