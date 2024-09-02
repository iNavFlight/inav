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
@/**   Thread                                                              */
@/**                                                                       */
@/**************************************************************************/
@/**************************************************************************/
@
@
@#define TX_SOURCE_CODE
@
@
@/* Include necessary system files.  */
@
@#include "tx_api.h"
@#include "tx_thread.h"
@
@
    .global     _tx_thread_system_state
    .global     _tx_thread_current_ptr
    .global     __tx_irq_processing_return
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    .global     _tx_execution_isr_enter
#endif
@
@
@/* No 16-bit Thumb mode veneer code is needed for _tx_thread_context_save
@   since it will never be called 16-bit mode.  */
@ 
    .arm
    .text
    .align 2
@/**************************************************************************/ 
@/*                                                                        */ 
@/*  FUNCTION                                               RELEASE        */ 
@/*                                                                        */ 
@/*    _tx_thread_context_save                         SMP/Cortex-A7/GNU   */ 
@/*                                                            6.1         */
@/*  AUTHOR                                                                */
@/*                                                                        */
@/*    William E. Lamie, Microsoft Corporation                             */
@/*                                                                        */
@/*  DESCRIPTION                                                           */
@/*                                                                        */ 
@/*    This function saves the context of an executing thread in the       */ 
@/*    beginning of interrupt processing.  The function also ensures that  */ 
@/*    the system stack is used upon return to the calling ISR.            */ 
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
@/*    ISRs                                                                */ 
@/*                                                                        */ 
@/*  RELEASE HISTORY                                                       */ 
@/*                                                                        */ 
@/*    DATE              NAME                      DESCRIPTION             */
@/*                                                                        */
@/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
@/*                                                                        */
@/**************************************************************************/
@VOID   _tx_thread_context_save(VOID)
@{
    .global _tx_thread_context_save
    .type   _tx_thread_context_save,function
_tx_thread_context_save:
@
@    /* Upon entry to this routine, it is assumed that IRQ interrupts are locked
@       out, we are in IRQ mode, and all registers are intact.  */
@
@    /* Check for a nested interrupt condition.  */
@    if (_tx_thread_system_state[core]++)
@    {
@
    STMDB   sp!, {r0-r3}                        @ Save some working registers 
@
@   /* Save the rest of the scratch registers on the stack and return to the
@      calling ISR.  */
@
    MRS     r0, SPSR                            @ Pickup saved SPSR
    SUB     lr, lr, #4                          @ Adjust point of interrupt 
    STMDB   sp!, {r0, r10, r12, lr}             @ Store other registers
@
#ifdef TX_ENABLE_FIQ_SUPPORT
    CPSID   if                                  @ Disable FIQ interrupts
#endif
@
@    /* Pickup the CPU ID.   */
@
    MRC     p15, 0, r10, c0, c0, 5              @ Read CPU ID register
    AND     r10, r10, #0x03                     @ Mask off, leaving the CPU ID field
    LSL     r12, r10, #2                        @ Build offset to array indexes
@
    LDR     r3, =_tx_thread_system_state        @ Pickup address of system state var
    ADD     r3, r3, r12                         @ Build index into the system state array
    LDR     r2, [r3, #0]                        @ Pickup system state
    CMP     r2, #0                              @ Is this the first interrupt?
    BEQ     __tx_thread_not_nested_save         @ Yes, not a nested context save
@
@    /* Nested interrupt condition.  */
@
    ADD     r2, r2, #1                          @ Increment the interrupt counter
    STR     r2, [r3, #0]                        @ Store it back in the variable
@
@    /* Return to the ISR.  */
@
    MOV     r10, #0                             @ Clear stack limit

#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
@
@    /* Call the ISR enter function to indicate an ISR is executing.  */
@
    PUSH    {r12, lr}                           @ Save ISR lr & r12
    BL      _tx_execution_isr_enter             @ Call the ISR enter function
    POP     {r12, lr}                           @ Recover ISR lr & r12
#endif

    B       __tx_irq_processing_return          @ Continue IRQ processing 
@
__tx_thread_not_nested_save:
@    }
@
@    /* Otherwise, not nested, check to see if a thread was running.  */
@    else if (_tx_thread_current_ptr[core])
@    {
@
    ADD     r2, r2, #1                          @ Increment the interrupt counter
    STR     r2, [r3, #0]                        @ Store it back in the variable
    LDR     r1, =_tx_thread_current_ptr         @ Pickup address of current thread ptr
    ADD     r1, r1, r12                         @ Build index into current thread ptr
    LDR     r0, [r1, #0]                        @ Pickup current thread pointer
    CMP     r0, #0                              @ Is it NULL?
    BEQ     __tx_thread_idle_system_save        @ If so, interrupt occurred in 
                                                @   scheduling loop - nothing needs saving!
@
@    /* Save the current stack pointer in the thread's control block.  */
@    _tx_thread_current_ptr[core] -> tx_thread_stack_ptr =  sp;
@
@    /* Switch to the system stack.  */
@    sp =  _tx_thread_system_stack_ptr;
@
    MOV     r10, #0                             @ Clear stack limit

#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
@
@    /* Call the ISR enter function to indicate an ISR is executing.  */
@
    PUSH    {r12, lr}                           @ Save ISR lr & r12
    BL      _tx_execution_isr_enter             @ Call the ISR enter function
    POP     {r12, lr}                           @ Recover ISR lr & r12
#endif

    B       __tx_irq_processing_return          @ Continue IRQ processing 
@
@    }
@    else
@    {
@
__tx_thread_idle_system_save:
@
@    /* Interrupt occurred in the scheduling loop.  */
@
@    /* Not much to do here, just adjust the stack pointer, and return to IRQ 
@       processing.  */
@
    MOV     r10, #0                             @ Clear stack limit

#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
@
@    /* Call the ISR enter function to indicate an ISR is executing.  */
@
    PUSH    {r12, lr}                           @ Save ISR lr & r12
    BL      _tx_execution_isr_enter             @ Call the ISR enter function
    POP     {r12, lr}                           @ Recover ISR lr & r12
#endif

    ADD     sp, sp, #32                         @ Recover saved registers
    B       __tx_irq_processing_return          @ Continue IRQ processing  
@
@    }
@}
@


