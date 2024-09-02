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
;

IRQ_MODE        EQU     0x12                    ; IRQ mode
SVC_MODE        EQU     0x13                    ; SVC mode
SYS_MODE        EQU     0x1F                    ; SYS mode
THUMB_MASK      EQU     0x20                    ; Thumb bit mask

    IF  :DEF:TX_ENABLE_FIQ_SUPPORT
DISABLE_INTS    EQU     0xC0                    ; Disable IRQ & FIQ interrupts
    ELSE
DISABLE_INTS    EQU     0x80                    ; Disable IRQ interrupts
    ENDIF
;
;
    IMPORT      _tx_thread_system_state
    IMPORT      _tx_thread_current_ptr
    IMPORT      _tx_thread_execute_ptr
    IMPORT      _tx_timer_time_slice
    IMPORT      _tx_thread_schedule
    IMPORT      _tx_thread_preempt_disable
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
;/*    _tx_thread_context_restore                      Cortex-A7/MMU/AC5   */ 
;/*                                                           6.1          */
;/*  AUTHOR                                                                */
;/*                                                                        */
;/*    Scott Larson, Microsoft Corporation                                 */
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
;/*  09-30-2020      Scott Larson            Initial Version 6.1           */
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
;
;    /* Determine if interrupts are nested.  */
;    if (--_tx_thread_system_state)
;    {
;
    LDR     r3, =_tx_thread_system_state        ; Pickup address of system state var
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
;    else if (((_tx_thread_current_ptr) && (_tx_thread_current_ptr == _tx_thread_execute_ptr) 
;               || (_tx_thread_preempt_disable))
;    {
;
    LDR     r1, =_tx_thread_current_ptr         ; Pickup address of current thread ptr
    LDR     r0, [r1, #0]                        ; Pickup actual current thread pointer
    CMP     r0, #0                              ; Is it NULL?
    BEQ     __tx_thread_idle_system_restore     ; Yes, idle system was interrupted
;
    LDR     r3, =_tx_thread_preempt_disable     ; Pickup preempt disable address
    LDR     r2, [r3, #0]                        ; Pickup actual preempt disable flag
    CMP     r2, #0                              ; Is it set?
    BNE     __tx_thread_no_preempt_restore      ; Yes, don't preempt this thread
    LDR     r3, =_tx_thread_execute_ptr         ; Pickup address of execute thread ptr
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
;    tmp_ptr =  _tx_thread_current_ptr -> tx_thread_stack_ptr;
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
    LDMIA   sp!, {r3, r10, r12, lr}             ; Recover temporarily saved registers
    MOV     r1, lr                              ; Save lr (point of interrupt)
    CPS     #SYS_MODE                           ; Enter SYS mode
    STR     r1, [sp, #-4]!                      ; Save point of interrupt on thread's stack
    STMDB   sp!, {r4-r12, lr}                   ; Save upper half of registers on thread's stack
    MOV     r4, r3                              ; Save SPSR in r4
    CPS     #IRQ_MODE                           ; Enter IRQ mode
    LDMIA   sp!, {r0-r3}                        ; Recover r0-r3
    CPS     #SYS_MODE                           ; Enter SYS mode
    STMDB   sp!, {r0-r3}                        ; Save r0-r3 on thread's stack
    
    LDR     r1, =_tx_thread_current_ptr         ; Pickup address of current thread ptr
    LDR     r0, [r1, #0]                        ; Pickup current thread pointer
    
    IF  {TARGET_FPU_VFP} = {TRUE}
    LDR     r2, [r0, #144]                      ; Pickup the VFP enabled flag
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
;    if (_tx_timer_time_slice)
;    {
;
    LDR     r3, =_tx_timer_time_slice           ; Pickup time-slice variable address
    LDR     r2, [r3, #0]                        ; Pickup time-slice
    CMP     r2, #0                              ; Is it active?
    BEQ     __tx_thread_dont_save_ts            ; No, don't save it
;
;        _tx_thread_current_ptr -> tx_thread_time_slice =  _tx_timer_time_slice;
;        _tx_timer_time_slice =  0;
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
;    _tx_thread_current_ptr =  TX_NULL;
;
    MOV     r0, #0                              ; NULL value
    STR     r0, [r1, #0]                        ; Clear current thread pointer
;
;    /* Return to the scheduler.  */
;    _tx_thread_schedule();
;
    CPS     #IRQ_MODE                           ; Enter IRQ mode
    MRS     r1, SPSR                            ; Get SPSR
    ORR     r1, r1, #SYS_MODE                   ; Change to SYS Mode
    BIC     r1, r1, #THUMB_MASK                 ; Clear thumb bit
    MSR     SPSR_cxsf, r1                       ; Put SYS Mode in SPSR
    LDR     lr, =_tx_thread_schedule            ; Load scheduler address
    MOVS    pc, lr                              ; Return to scheduler
;    }
;
__tx_thread_idle_system_restore
;
;    /* Just return back to the scheduler!  */
;
    LDR     lr, =_tx_thread_schedule            ; Load scheduler address
    MOVS    pc, lr                              ; Return to scheduler
;}
;
    END

