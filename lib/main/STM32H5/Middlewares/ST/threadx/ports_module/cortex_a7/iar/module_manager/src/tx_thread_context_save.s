;/**************************************************************************/
;/*                                                                        */
;/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
;/*                                                                        */
;/*       This software is licensed under the Microsoft Software License   */
;/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
;/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
;/*       and in the root directory of this software.                      */
;/*                                                                        */
;/**************************************************************************/
;
;
;/**************************************************************************/
;/**************************************************************************/
;/**                                                                       */ 
;/** ThreadX Component                                                     */ 
;/**                                                                       */
;/**   Thread                                                              */
;/**                                                                       */
;/**************************************************************************/
;/**************************************************************************/
;
;

    EXTERN      _tx_thread_system_state
    EXTERN      _tx_thread_current_ptr
    EXTERN      __tx_irq_processing_return
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    EXTERN      _tx_execution_isr_enter
#endif
;
;

;/**************************************************************************/ 
;/*                                                                        */ 
;/*  FUNCTION                                               RELEASE        */ 
;/*                                                                        */ 
;/*    _tx_thread_context_save                            Cortex-A7/IAR    */ 
;/*                                                           6.1          */
;/*  AUTHOR                                                                */
;/*                                                                        */
;/*    William E. Lamie, Microsoft Corporation                             */
;/*                                                                        */
;/*  DESCRIPTION                                                           */
;/*                                                                        */ 
;/*    This function saves the context of an executing thread in the       */ 
;/*    beginning of interrupt processing.  The function also ensures that  */ 
;/*    the system stack is used upon return to the calling ISR.            */ 
;/*                                                                        */ 
;/*  INPUT                                                                 */ 
;/*                                                                        */ 
;/*    None                                                                */ 
;/*                                                                        */ 
;/*  OUTPUT                                                                */ 
;/*                                                                        */ 
;/*    None                                                                */ 
;/*                                                                        */ 
;/*  CALLS                                                                 */ 
;/*                                                                        */ 
;/*    None                                                                */ 
;/*                                                                        */ 
;/*  CALLED BY                                                             */ 
;/*                                                                        */ 
;/*    ISRs                                                                */ 
;/*                                                                        */ 
;/*  RELEASE HISTORY                                                       */ 
;/*                                                                        */ 
;/*    DATE              NAME                      DESCRIPTION             */
;/*                                                                        */
;/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
;/*                                                                        */
;/**************************************************************************/
;VOID   _tx_thread_context_save(VOID)
;{
    RSEG    .text:CODE:NOROOT(2)
    PUBLIC  _tx_thread_context_save
    ARM
_tx_thread_context_save
;
;    /* Upon entry to this routine, it is assumed that IRQ interrupts are locked
;       out, we are in IRQ mode, and all registers are intact.  */
;
;    /* Check for a nested interrupt condition.  */
;    if (_tx_thread_system_state++)
;    {
;
    STMDB   sp!, {r0-r3}                        ; Save some working registers 
#ifdef TX_ENABLE_FIQ_SUPPORT
    CPSID   if                                  ; Disable FIQ interrupts
#endif
    LDR     r3, =_tx_thread_system_state        ; Pickup address of system state var
    LDR     r2, [r3, #0]                        ; Pickup system state
    CMP     r2, #0                              ; Is this the first interrupt?
    BEQ     __tx_thread_not_nested_save         ; Yes, not a nested context save
;
;    /* Nested interrupt condition.  */
;
    ADD     r2, r2, #1                          ; Increment the interrupt counter
    STR     r2, [r3, #0]                        ; Store it back in the variable
;
;   /* Save the rest of the scratch registers on the stack and return to the
;      calling ISR.  */
;
    MRS     r0, SPSR                            ; Pickup saved SPSR
    SUB     lr, lr, #4                          ; Adjust point of interrupt 
    STMDB   sp!, {r0, r10, r12, lr}             ; Store other registers
;
;    /* Return to the ISR.  */
;
    MOV     r10, #0                             ; Clear stack limit

#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
;
;    /* Call the ISR enter function to indicate an ISR is executing.  */
;
    PUSH    {lr}                                ; Save ISR lr
    BL      _tx_execution_isr_enter             ; Call the ISR enter function
    POP     {lr}                                ; Recover ISR lr
#endif

    B       __tx_irq_processing_return          ; Continue IRQ processing 
;
__tx_thread_not_nested_save
;    }
;
;    /* Otherwise, not nested, check to see if a thread was running.  */
;    else if (_tx_thread_current_ptr)
;    {
;
    ADD     r2, r2, #1                          ; Increment the interrupt counter
    STR     r2, [r3, #0]                        ; Store it back in the variable
    LDR     r1, =_tx_thread_current_ptr         ; Pickup address of current thread ptr
    LDR     r0, [r1, #0]                        ; Pickup current thread pointer
    CMP     r0, #0                              ; Is it NULL?
    BEQ     __tx_thread_idle_system_save        ; If so, interrupt occurred in 
                                                ;   scheduling loop - nothing needs saving!
;
;    /* Save minimal context of interrupted thread.  */
;
    MRS     r2, SPSR                            ; Pickup saved SPSR
    SUB     lr, lr, #4                          ; Adjust point of interrupt 
    STMDB   sp!, {r2, r10, r12, lr}             ; Store other registers
;
;    /* Save the current stack pointer in the thread's control block.  */
;    _tx_thread_current_ptr -> tx_thread_stack_ptr =  sp;
;
;    /* Switch to the system stack.  */
;    sp =  _tx_thread_system_stack_ptr;
;
    MOV     r10, #0                             ; Clear stack limit

#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
;
;    /* Call the ISR enter function to indicate an ISR is executing.  */
;
    PUSH    {lr}                                ; Save ISR lr
    BL      _tx_execution_isr_enter             ; Call the ISR enter function
    POP     {lr}                                ; Recover ISR lr
#endif

    B       __tx_irq_processing_return          ; Continue IRQ processing 
;
;    }
;    else
;    {
;
__tx_thread_idle_system_save
;
;    /* Interrupt occurred in the scheduling loop.  */
;
;    /* Not much to do here, just adjust the stack pointer, and return to IRQ 
;       processing.  */
;
    MOV     r10, #0                             ; Clear stack limit

#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
;
;    /* Call the ISR enter function to indicate an ISR is executing.  */
;
    PUSH    {lr}                                ; Save ISR lr
    BL      _tx_execution_isr_enter             ; Call the ISR enter function
    POP     {lr}                                ; Recover ISR lr
#endif

    ADD     sp, sp, #16                         ; Recover saved registers
    B       __tx_irq_processing_return          ; Continue IRQ processing  
;
;    }
;}
;
    END

