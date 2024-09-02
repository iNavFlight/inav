@/**************************************************************************/
@/*                                                                        */
@/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
@/*                                                                        */
@/*       This software is licensed under the Microsoft Software License   */
@/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
@/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
@/*       and in the root directory of this software.                      */
@/*                                                                        */
@/**************************************************************************/
@
@
@/**************************************************************************/
@/**************************************************************************/
@/**                                                                       */
@/** ThreadX Component                                                     */
@/**                                                                       */
@/**   Initialize                                                          */
@/**                                                                       */
@/**************************************************************************/
@/**************************************************************************/
@
@#define TX_SOURCE_CODE
@
@
@/* Include necessary system files.  */
@
@#include "tx_api.h"
@#include "tx_initialize.h"
@#include "tx_thread.h"
@#include "tx_timer.h"
@
@
    .global  _tx_thread_system_stack_ptr
    .global  _tx_initialize_unused_memory
    .global  __RAM_segment_used_end__
    .global  _tx_thread_context_save
    .global  _tx_thread_context_restore
    .global  _tx_timer_interrupt
    .global  _vectors
@
@
SYSTEM_CLOCK      =   8000000
SYSTICK_CYCLES    =   ((SYSTEM_CLOCK / 100) -1)

    .text
    .align 4
    .syntax unified
@/**************************************************************************/
@/*                                                                        */
@/*  FUNCTION                                               RELEASE        */
@/*                                                                        */
@/*    _tx_initialize_low_level                          Cortex-M0/GNU     */
@/*                                                           6.1          */
@/*  AUTHOR                                                                */
@/*                                                                        */
@/*    William E. Lamie, Microsoft Corporation                             */
@/*                                                                        */
@/*  DESCRIPTION                                                           */
@/*                                                                        */
@/*    This function is responsible for any low-level processor            */
@/*    initialization, including setting up interrupt vectors, setting     */
@/*    up a periodic timer interrupt source, saving the system stack       */
@/*    pointer for use in ISR processing later, and finding the first      */
@/*    available RAM memory address for tx_application_define.             */
@/*                                                                        */
@/*  INPUT                                                                 */
@/*                                                                        */
@/*    None                                                                */
@/*                                                                        */
@/*  OUTPUT                                                                */
@/*                                                                        */
@/*    None                                                                */
@/*                                                                        */
@/*  CALLS                                                                 */
@/*                                                                        */
@/*    None                                                                */
@/*                                                                        */
@/*  CALLED BY                                                             */
@/*                                                                        */
@/*    _tx_initialize_kernel_enter           ThreadX entry function        */
@/*                                                                        */
@/*  RELEASE HISTORY                                                       */
@/*                                                                        */
@/*    DATE              NAME                      DESCRIPTION             */
@/*                                                                        */
@/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
@/*  09-30-2020     Scott Larson             Modified comment(s), and      */
@/*                                            commented out code for      */
@/*                                            enabling DWT,               */
@/*                                            resulting in version 6.1    */
@/*                                                                        */
@/**************************************************************************/
@VOID   _tx_initialize_low_level(VOID)
@{
    .global  _tx_initialize_low_level
    .thumb_func
_tx_initialize_low_level:
@
@    /* Disable interrupts during ThreadX initialization.  */
@
    CPSID   i
@
@    /* Set base of available memory to end of non-initialised RAM area.  */
@     
    LDR     r0, =_tx_initialize_unused_memory       @ Build address of unused memory pointer
    LDR     r1, =__RAM_segment_used_end__           @ Build first free address   
    ADDS    r1, r1, #4                              @ 
    STR     r1, [r0]                                @ Setup first unused memory pointer
@
@    /* Enable the cycle count register.  */
@    /* Not all M0 have DWT, uncomment if you have a DWT and want to use it. */
@    LDR     r0, =0xE0001000                         @ Build address of DWT register
@    LDR     r1, [r0]                                @ Pickup the current value
@    MOVS    r2, #1
@    ORRS    r1, r1, r2                              @ Set the CYCCNTENA bit
@    STR     r1, [r0]                                @ Enable the cycle count register 
@
@    /* Setup Vector Table Offset Register.  */
@    
    LDR     r0, =0xE000E000                         @ Build address of NVIC registers
    LDR     r2, =0xD08                              @ Offset to vector base register
    ADD     r0, r0, r2                              @ Build vector base register
    LDR     r1, =_vectors                           @ Pickup address of vector table
    STR     r1, [r0]                                @ Set vector table address    
@
@    /* Set system stack pointer from vector value.  */
@
    LDR     r0, =_tx_thread_system_stack_ptr        @ Build address of system stack pointer
    LDR     r1, =_vectors                           @ Pickup address of vector table
    LDR     r1, [r1]                                @ Pickup reset stack pointer
    STR     r1, [r0]                                @ Save system stack pointer
@
@    /* Configure SysTick for 100Hz clock, or 16384 cycles if no reference.  */
@
    LDR     r0, =0xE000E000                         @ Build address of NVIC registers
    LDR     r1, =SYSTICK_CYCLES
    STR     r1, [r0, #0x14]                         // Setup SysTick Reload Value
    LDR     r1, =0x7                                // Build SysTick Control Enable Value
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
    .global  __tx_BadHandler
    .thumb_func
__tx_BadHandler:
    B       __tx_BadHandler

/* added to catch the hardfault */
    .global  __tx_HardfaultHandler
    .thumb_func
__tx_HardfaultHandler:
    B       __tx_HardfaultHandler

/* Generic interrupt handler template */
    .global  __tx_IntHandler
    .thumb_func
__tx_IntHandler:
// VOID InterruptHandler (VOID)
// {
    PUSH    {r0, lr}
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    BL      _tx_execution_isr_enter             // Call the ISR enter function
#endif
    /* Do interrupt handler work here */
    /* BL <your C Function>.... */
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    BL      _tx_execution_isr_exit              // Call the ISR exit function
#endif
    POP     {r0, r1}
    MOV     lr, r1
    BX      lr
// }

/* System Tick timer interrupt handler */
    .global  __tx_SysTickHandler
    .global  SysTick_Handler
    .thumb_func
__tx_SysTickHandler:
    .thumb_func
SysTick_Handler:
// VOID SysTick_Handler (VOID)
// {
    PUSH    {r0, lr}
#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
    BL      _tx_execution_isr_enter             // Call the ISR enter function
#endif
    BL      _tx_timer_interrupt
#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
    BL      _tx_execution_isr_exit              // Call the ISR exit function
#endif
    POP     {r0, r1}
    MOV     lr, r1
    BX      lr
// }

/* NMI, DBG handlers */
    .global  __tx_NMIHandler
    .thumb_func
__tx_NMIHandler:
    B       __tx_NMIHandler

    .global  __tx_DBGHandler
    .thumb_func
__tx_DBGHandler:
    B       __tx_DBGHandler
