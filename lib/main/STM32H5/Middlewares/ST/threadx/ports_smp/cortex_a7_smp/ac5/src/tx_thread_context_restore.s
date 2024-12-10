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
;#define TX_SOURCE_CODE
;
;
;/* Include necessary system files.  */
;
;#include "tx_api.h"
;#include "tx_thread.h"
;#include "tx_timer.h"
;
;/* Include macros for modifying the wait list.  */
#include "tx_thread_smp_protection_wait_list_macros.h"

    IF  :DEF:TX_ENABLE_FIQ_SUPPORT
DISABLE_INTS    EQU     0xC0                    ; Disable IRQ & FIQ interrupts
IRQ_MODE        EQU     0xD2                    ; IRQ mode
SVC_MODE        EQU     0xD3                    ; SVC mode
    ELSE
DISABLE_INTS    EQU     0x80                    ; Disable IRQ interrupts
IRQ_MODE        EQU     0x92                    ; IRQ mode
SVC_MODE        EQU     0x93                    ; SVC mode
    ENDIF
;
;
    IMPORT      _tx_thread_system_state
    IMPORT      _tx_thread_current_ptr
    IMPORT      _tx_thread_execute_ptr
    IMPORT      _tx_timer_time_slice
    IMPORT      _tx_thread_schedule
    IMPORT      _tx_thread_preempt_disable
    IMPORT      _tx_timer_interrupt_active
    IMPORT      _tx_thread_smp_protection
    IMPORT      _tx_thread_smp_protect_wait_counts
    IMPORT      _tx_thread_smp_protect_wait_list
    IMPORT      _tx_thread_smp_protect_wait_list_lock_protect_in_force
    IMPORT      _tx_thread_smp_protect_wait_list_tail
    IMPORT      _tx_thread_smp_protect_wait_list_size
    IF :DEF:TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    IMPORT      _tx_execution_isr_exit
    ENDIF
;
;
        AREA ||.text||, CODE, READONLY
        PRESERVE8
;/**************************************************************************/ 
;/*                                                                        */ 
;/*  FUNCTION                                               RELEASE        */ 
;/*                                                                        */ 
;/*    _tx_thread_context_restore                      SMP/Cortex-A7/AC5   */
;/*                                                            6.1         */
;/*  AUTHOR                                                                */
;/*                                                                        */
;/*    William E. Lamie, Microsoft Corporation                             */
;/*                                                                        */
;/*  DESCRIPTION                                                           */
;/*                                                                        */ 
;/*    This function restores the interrupt context if it is processing a  */ 
;/*    nested interrupt.  If not, it returns to the interrupt thread if no */ 
;/*    preemption is necessary.  Otherwise, if preemption is necessary or  */ 
;/*    if no thread was running, the function returns to the scheduler.    */ 
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
;/*    _tx_thread_schedule                   Thread scheduling routine     */ 
;/*                                                                        */ 
;/*  CALLED BY                                                             */ 
;/*                                                                        */ 
;/*    ISRs                                  Interrupt Service Routines    */ 
;/*                                                                        */ 
;/*  RELEASE HISTORY                                                       */ 
;/*                                                                        */ 
;/*    DATE              NAME                      DESCRIPTION             */
;/*                                                                        */
;/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
;/*                                                                        */
;/**************************************************************************/
;VOID   _tx_thread_context_restore(VOID)
;{
    EXPORT  _tx_thread_context_restore
_tx_thread_context_restore
;
;    /* Lockout interrupts.  */
;
    IF  :DEF:TX_ENABLE_FIQ_SUPPORT
    CPSID   if                                  ; Disable IRQ and FIQ interrupts
    ELSE
    CPSID   i                                   ; Disable IRQ interrupts
    ENDIF

    IF :DEF:TX_ENABLE_EXECUTION_CHANGE_NOTIFY
;
;    /* Call the ISR exit function to indicate an ISR is complete.  */
;
    BL      _tx_execution_isr_exit              ; Call the ISR exit function
    ENDIF

;    /* Pickup the CPU ID.   */
;
    MRC     p15, 0, r10, c0, c0, 5              ; Read CPU ID register
    AND     r10, r10, #0x03                     ; Mask off, leaving the CPU ID field
    LSL     r12, r10, #2                        ; Build offset to array indexes
;
;    /* Determine if interrupts are nested.  */
;    if (--_tx_thread_system_state[core])
;    {
;
    LDR     r3, =_tx_thread_system_state        ; Pickup address of system state var
    ADD     r3, r3, r12                         ; Build array offset
    LDR     r2, [r3, #0]                        ; Pickup system state
    SUB     r2, r2, #1                          ; Decrement the counter
    STR     r2, [r3, #0]                        ; Store the counter 
    CMP     r2, #0                              ; Was this the first interrupt?
    BEQ     __tx_thread_not_nested_restore      ; If so, not a nested restore
;
;    /* Interrupts are nested.  */
;
;    /* Just recover the saved registers and return to the point of 
;       interrupt.  */
;
    LDMIA   sp!, {r0, r10, r12, lr}             ; Recover SPSR, POI, and scratch regs
    MSR     SPSR_cxsf, r0                       ; Put SPSR back
    LDMIA   sp!, {r0-r3}                        ; Recover r0-r3
    MOVS    pc, lr                              ; Return to point of interrupt
;
;    }
__tx_thread_not_nested_restore
;
;    /* Determine if a thread was interrupted and no preemption is required.  */
;    else if (((_tx_thread_current_ptr[core]) && (_tx_thread_current_ptr[core] == _tx_thread_execute_ptr[core]) 
;               || (_tx_thread_preempt_disable))
;    {
;
    LDR     r1, =_tx_thread_current_ptr         ; Pickup address of current thread ptr
    ADD     r1, r1, r12                         ; Build index to this core's current thread ptr
    LDR     r0, [r1, #0]                        ; Pickup actual current thread pointer
    CMP     r0, #0                              ; Is it NULL?
    BEQ     __tx_thread_idle_system_restore     ; Yes, idle system was interrupted
;
    LDR     r3, =_tx_thread_smp_protection      ; Get address of protection structure
    LDR     r2, [r3, #8]                        ; Pickup owning core
    CMP     r2, r10                             ; Is the owning core the same as the protected core?
    BNE     __tx_thread_skip_preempt_check      ; No, skip the preempt disable check since this is only valid for the owning core

    LDR     r3, =_tx_thread_preempt_disable     ; Pickup preempt disable address
    LDR     r2, [r3, #0]                        ; Pickup actual preempt disable flag
    CMP     r2, #0                              ; Is it set?
    BNE     __tx_thread_no_preempt_restore      ; Yes, don't preempt this thread
__tx_thread_skip_preempt_check

    LDR     r3, =_tx_thread_execute_ptr         ; Pickup address of execute thread ptr
    ADD     r3, r3, r12                         ; Build index to this core's execute thread ptr
    LDR     r2, [r3, #0]                        ; Pickup actual execute thread pointer
    CMP     r0, r2                              ; Is the same thread highest priority?
    BNE     __tx_thread_preempt_restore         ; No, preemption needs to happen
;
;
__tx_thread_no_preempt_restore
;
;    /* Restore interrupted thread or ISR.  */
;
;    /* Pickup the saved stack pointer.  */
;    tmp_ptr =  _tx_thread_current_ptr[core] -> tx_thread_stack_ptr;
;
;   /* Recover the saved context and return to the point of interrupt.  */
;
    LDMIA   sp!, {r0, r10, r12, lr}             ; Recover SPSR, POI, and scratch regs
    MSR     SPSR_cxsf, r0                       ; Put SPSR back
    LDMIA   sp!, {r0-r3}                        ; Recover r0-r3
    MOVS    pc, lr                              ; Return to point of interrupt
;
;    }
;    else
;    {
__tx_thread_preempt_restore
;
;    /* Was the thread being preempted waiting for the lock?  */
;    if (_tx_thread_smp_protect_wait_counts[this_core] != 0)
;    {
;
    LDR     r1, =_tx_thread_smp_protect_wait_counts ; Load waiting count list
    LDR     r2, [r1, r10, LSL #2]               ; Load waiting value for this core
    CMP     r2, #0
    BEQ     _nobody_waiting_for_lock            ; Is the core waiting for the lock?
;
;    /* Do we not have the lock? This means the ISR never got the inter-core lock.  */
;    if (_tx_thread_smp_protection.tx_thread_smp_protect_owned != this_core)
;    {
;
    LDR     r1, =_tx_thread_smp_protection      ; Load address of protection structure
    LDR     r2, [r1, #8]                        ; Pickup the owning core
    CMP     r10, r2                             ; Compare our core to the owning core
    BEQ     _this_core_has_lock                 ; Do we have the lock?
;
;    /* We don't have the lock. This core should be in the list. Remove it.  */
;    _tx_thread_smp_protect_wait_list_remove(this_core);
;
macro_call0 _tx_thread_smp_protect_wait_list_remove ; Call macro to remove core from the list
    B       _nobody_waiting_for_lock            ; Leave
;
;    }
;    else
;    {
;    /* We have the lock. This means the ISR got the inter-core lock, but
;       never released it because it saw that there was someone waiting.
;       Note this core is not in the list.  */
;
_this_core_has_lock
;
;    /* We're no longer waiting. Note that this should be zero since this happens during thread preemption.  */
;    _tx_thread_smp_protect_wait_counts[core]--;
;
    LDR     r1, =_tx_thread_smp_protect_wait_counts ; Load waiting count list
    LDR     r2, [r1, r10, LSL #2]               ; Load waiting value for this core
    SUB     r2, r2, #1                          ; Decrement waiting value. Should be zero now
    STR     r2, [r1, r10, LSL #2]               ; Store new waiting value
;
;    /* Now release the inter-core lock.  */
;
;    /* Set protected core as invalid.  */
;    _tx_thread_smp_protection.tx_thread_smp_protect_core = 0xFFFFFFFF;
;
    LDR     r1, =_tx_thread_smp_protection      ; Load address of protection structure
    MOV     r2, #0xFFFFFFFF                     ; Build invalid value
    STR     r2, [r1, #8]                        ; Mark the protected core as invalid
    DMB                                         ; Ensure that accesses to shared resource have completed
;
;    /* Release protection.  */
;    _tx_thread_smp_protection.tx_thread_smp_protect_in_force = 0;
;
    MOV     r2, #0                              ; Build release protection value
    STR     r2, [r1, #0]                        ; Release the protection
    DSB     ISH                                 ; To ensure update of the protection occurs before other CPUs awake
;
;    /* Wake up waiting processors. Note interrupts are already enabled.  */
;
    IF  :DEF:TX_ENABLE_WFE
    SEV                                         ; Send event to other CPUs
    ENDIF
;
;    }
;    }
;

_nobody_waiting_for_lock

    LDMIA   sp!, {r3, r10, r12, lr}             ; Recover temporarily saved registers
    MOV     r1, lr                              ; Save lr (point of interrupt)
    MOV     r2, #SVC_MODE                       ; Build SVC mode CPSR
    MSR     CPSR_c, r2                          ; Enter SVC mode
    STR     r1, [sp, #-4]!                      ; Save point of interrupt
    STMDB   sp!, {r4-r12, lr}                   ; Save upper half of registers
    MOV     r4, r3                              ; Save SPSR in r4
    MOV     r2, #IRQ_MODE                       ; Build IRQ mode CPSR
    MSR     CPSR_c, r2                          ; Enter IRQ mode
    LDMIA   sp!, {r0-r3}                        ; Recover r0-r3
    MOV     r5, #SVC_MODE                       ; Build SVC mode CPSR
    MSR     CPSR_c, r5                          ; Enter SVC mode
    STMDB   sp!, {r0-r3}                        ; Save r0-r3 on thread's stack

    MRC     p15, 0, r10, c0, c0, 5              ; Read CPU ID register
    AND     r10, r10, #0x03                     ; Mask off, leaving the CPU ID field
    LSL     r12, r10, #2                        ; Build offset to array indexes
    LDR     r1, =_tx_thread_current_ptr         ; Pickup address of current thread ptr
    ADD     r1, r1, r12                         ; Build index to current thread ptr
    LDR     r0, [r1, #0]                        ; Pickup current thread pointer

    IF  {TARGET_FPU_VFP} = {TRUE}
    LDR     r2, [r0, #160]                      ; Pickup the VFP enabled flag
    CMP     r2, #0                              ; Is the VFP enabled?
    BEQ     _tx_skip_irq_vfp_save               ; No, skip VFP IRQ save
    VMRS    r2, FPSCR                           ; Pickup the FPSCR
    STR     r2, [sp, #-4]!                      ; Save FPSCR
    VSTMDB  sp!, {D16-D31}                      ; Save D16-D31
    VSTMDB  sp!, {D0-D15}                       ; Save D0-D15
_tx_skip_irq_vfp_save
    ENDIF

    MOV     r3, #1                              ; Build interrupt stack type
    STMDB   sp!, {r3, r4}                       ; Save interrupt stack type and SPSR
    STR     sp, [r0, #8]                        ; Save stack pointer in thread control
                                                ;   block
;
;    /* Save the remaining time-slice and disable it.  */
;    if (_tx_timer_time_slice[core])
;    {
;
    LDR     r3, =_tx_timer_interrupt_active     ; Pickup timer interrupt active flag's address
_tx_wait_for_timer_to_finish
    LDR     r2, [r3, #0]                        ; Pickup timer interrupt active flag
    CMP     r2, #0                              ; Is the timer interrupt active?
    BNE     _tx_wait_for_timer_to_finish        ; If timer interrupt is active, wait until it completes

    LDR     r3, =_tx_timer_time_slice           ; Pickup time-slice variable address
    ADD     r3, r3, r12                         ; Build index to core's time slice
    LDR     r2, [r3, #0]                        ; Pickup time-slice
    CMP     r2, #0                              ; Is it active?
    BEQ     __tx_thread_dont_save_ts            ; No, don't save it
;
;        _tx_thread_current_ptr[core] -> tx_thread_time_slice =  _tx_timer_time_slice[core];
;        _tx_timer_time_slice[core] =  0;
;
    STR     r2, [r0, #24]                       ; Save thread's time-slice
    MOV     r2, #0                              ; Clear value
    STR     r2, [r3, #0]                        ; Disable global time-slice flag
;
;    }
__tx_thread_dont_save_ts
;
;
;    /* Clear the current task pointer.  */
;    _tx_thread_current_ptr[core] =  TX_NULL;
;
    MOV     r2, #0                              ; NULL value
    STR     r2, [r1, #0]                        ; Clear current thread pointer
;
;    /* Set bit indicating this thread is ready for execution.  */
;
    LDR     r2, [r0, #152]                      ; Pickup the ready bit 
    ORR     r2, r2, #0x8000                     ; Set ready bit (bit 15)
    STR     r2, [r0, #152]                      ; Make this thread ready for executing again
    DMB                                         ; Ensure that accesses to shared resource have completed
;
;    /* Return to the scheduler.  */
;    _tx_thread_schedule();
;
    B       _tx_thread_schedule                 ; Return to scheduler
;    }
;
__tx_thread_idle_system_restore
;
;    /* Just return back to the scheduler!  */
;
    MOV     r3, #SVC_MODE                       ; Build SVC mode with interrupts disabled
    MSR     CPSR_c, r3                          ; Change to SVC mode
    B       _tx_thread_schedule                 ; Return to scheduler
;}
;
    END

