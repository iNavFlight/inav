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
SVC_MODE        DEFINE  0xD3                    ; SVC mode
FIQ_MODE        DEFINE  0xD1                    ; FIQ mode
#ifdef TX_ENABLE_FIQ_SUPPORT
DISABLE_INTS    DEFINE  0xC0                    ; Disable IRQ & FIQ interrupts
#else
DISABLE_INTS    DEFINE  0x80                    ; Disable IRQ interrupts
#endif
MODE_MASK       DEFINE  0x1F                    ; Mode mask 
THUMB_MASK      DEFINE  0x20                    ; Thumb bit mask
IRQ_MODE_BITS   DEFINE  0x12                    ; IRQ mode bits
SVC_MODE_BITS   DEFINE  0x13                    ; SVC mode value

;
    EXTERN      _tx_thread_system_state
    EXTERN      _tx_thread_current_ptr
    EXTERN      _tx_thread_execute_ptr
    EXTERN      _tx_timer_time_slice
    EXTERN      _tx_thread_schedule
    EXTERN      _tx_thread_preempt_disable
    EXTERN      _tx_execution_isr_exit
;
;
;/**************************************************************************/ 
;/*                                                                        */ 
;/*  FUNCTION                                               RELEASE        */ 
;/*                                                                        */ 
;/*    _tx_thread_fiq_context_restore                     Cortex-A7/IAR    */ 
;/*                                                           6.1.9        */
;/*  AUTHOR                                                                */
;/*                                                                        */
;/*    William E. Lamie, Microsoft Corporation                             */
;/*                                                                        */
;/*  DESCRIPTION                                                           */
;/*                                                                        */ 
;/*    This function restores the fiq interrupt context when processing a  */ 
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
;/*    FIQ ISR                               Interrupt Service Routines    */ 
;/*                                                                        */ 
;/*  RELEASE HISTORY                                                       */ 
;/*                                                                        */ 
;/*    DATE              NAME                      DESCRIPTION             */
;/*                                                                        */
;/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
;/*  10-15-2021     William E. Lamie         Modified comment(s), added    */
;/*                                            execution profile support,  */
;/*                                            resulting in version 6.1.9  */
;/*                                                                        */
;/**************************************************************************/
;VOID   _tx_thread_fiq_context_restore(VOID)
;{
    RSEG    .text:CODE:NOROOT(2)
    PUBLIC  _tx_thread_fiq_context_restore
    CODE32
_tx_thread_fiq_context_restore
;
;    /* Lockout interrupts.  */
;
    MRS     r3, CPSR                            ; Pickup current CPSR
    ORR     r0, r3, #DISABLE_INTS               ; Build interrupt disable value
    MSR     CPSR_cxsf, r0                       ; Lockout interrupts

#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
;
;    /* Call the ISR exit function to indicate an ISR is complete.  */
;
    BL      _tx_execution_isr_exit              ; Call the ISR exit function
#endif
;
;    /* Determine if interrupts are nested.  */
;    if (--_tx_thread_system_state)
;    {
;
    LDR     r3, =_tx_thread_system_state        ; Pickup address of system state var
    LDR     r2, [r3]                            ; Pickup system state
    SUB     r2, r2, #1                          ; Decrement the counter
    STR     r2, [r3]                            ; Store the counter 
    CMP     r2, #0                              ; Was this the first interrupt?
    BEQ     __tx_thread_fiq_not_nested_restore  ; If so, not a nested restore
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
__tx_thread_fiq_not_nested_restore
;
;    /* Determine if a thread was interrupted and no preemption is required.  */
;    else if (((_tx_thread_current_ptr) && (_tx_thread_current_ptr == _tx_thread_execute_ptr) 
;               || (_tx_thread_preempt_disable))
;    {
;
    LDR     r1, [sp]                            ; Pickup the saved SPSR
    MOV     r2, #MODE_MASK                      ; Build mask to isolate the interrupted mode
    AND     r1, r1, r2                          ; Isolate mode bits
    CMP     r1, #IRQ_MODE_BITS                  ; Was an interrupt taken in IRQ mode before we
                                                ;   got to context save? */
    BEQ     __tx_thread_fiq_no_preempt_restore  ; Yes, just go back to point of interrupt


    LDR     r1, =_tx_thread_current_ptr         ; Pickup address of current thread ptr
    LDR     r0, [r1]                            ; Pickup actual current thread pointer
    CMP     r0, #0                              ; Is it NULL?
    BEQ     __tx_thread_fiq_idle_system_restore ; Yes, idle system was interrupted

    LDR     r3, =_tx_thread_preempt_disable     ; Pickup preempt disable address
    LDR     r2, [r3]                            ; Pickup actual preempt disable flag
    CMP     r2, #0                              ; Is it set?
    BNE     __tx_thread_fiq_no_preempt_restore  ; Yes, don't preempt this thread
    LDR     r3, =_tx_thread_execute_ptr         ; Pickup address of execute thread ptr
    LDR     r2, [r3]                            ; Pickup actual execute thread pointer
    CMP     r0, r2                              ; Is the same thread highest priority?
    BNE     __tx_thread_fiq_preempt_restore     ; No, preemption needs to happen


__tx_thread_fiq_no_preempt_restore
;
;    /* Restore interrupted thread or ISR.  */
;
;    /* Pickup the saved stack pointer.  */
;    tmp_ptr =  _tx_thread_current_ptr -> tx_thread_stack_ptr; 
;
;    /* Recover the saved context and return to the point of interrupt.  */
;
    LDMIA   sp!, {r0, lr}                       ; Recover SPSR, POI, and scratch regs
    MSR     SPSR_cxsf, r0                       ; Put SPSR back
    LDMIA   sp!, {r0-r3}                        ; Recover r0-r3
    MOVS    pc, lr                              ; Return to point of interrupt
;
;    }
;    else
;    {
__tx_thread_fiq_preempt_restore
;
    LDMIA   sp!, {r3, lr}                       ; Recover temporarily saved registers
    MOV     r1, lr                              ; Save lr (point of interrupt)
    MOV     r2, #SVC_MODE                       ; Build SVC mode CPSR
    MSR     CPSR_cxsf, r2                       ; Enter SVC mode
    STR     r1, [sp, #-4]!                      ; Save point of interrupt
    STMDB   sp!, {r4-r12, lr}                   ; Save upper half of registers
    MOV     r4, r3                              ; Save SPSR in r4
    MOV     r2, #FIQ_MODE                       ; Build FIQ mode CPSR
    MSR     CPSR_cxsf, r2                       ; Re-enter FIQ mode
    LDMIA   sp!, {r0-r3}                        ; Recover r0-r3
    MOV     r5, #SVC_MODE                       ; Build SVC mode CPSR
    MSR     CPSR_cxsf, r5                       ; Enter SVC mode
    STMDB   sp!, {r0-r3}                        ; Save r0-r3 on thread's stack

    LDR     r1, =_tx_thread_current_ptr         ; Pickup address of current thread ptr
    LDR     r0, [r1]                            ; Pickup current thread pointer

#ifdef __ARMVFP__
    LDR     r2, [r0, #144]                      ; Pickup the VFP enabled flag
    CMP     r2, #0                              ; Is the VFP enabled?
    BEQ     _tx_skip_irq_vfp_save               ; No, skip VFP IRQ save
    VMRS    r2, FPSCR                           ; Pickup the FPSCR
    STR     r2, [sp, #-4]!                      ; Save FPSCR
    VSTMDB  sp!, {D16-D31}                      ; Save D16-D31
    VSTMDB  sp!, {D0-D15}                       ; Save D0-D15
_tx_skip_irq_vfp_save:
#endif

    MOV     r3, #1                              ; Build interrupt stack type
    STMDB   sp!, {r3, r4}                       ; Save interrupt stack type and SPSR
    STR     sp, [r0, #8]                        ; Save stack pointer in thread control
                                                ;   block  */
    BIC     r4, r4, #THUMB_MASK                 ; Clear the Thumb bit of CPSR
    ORR     r3, r4, #DISABLE_INTS               ; Or-in interrupt lockout bit(s)
    MSR     CPSR_cxsf, r3                       ; Lockout interrupts
;
;    /* Save the remaining time-slice and disable it.  */
;    if (_tx_timer_time_slice)
;    {
;
    LDR     r3, =_tx_timer_time_slice           ; Pickup time-slice variable address
    LDR     r2, [r3]                            ; Pickup time-slice
    CMP     r2, #0                              ; Is it active?
    BEQ     __tx_thread_fiq_dont_save_ts        ; No, don't save it
;
;        _tx_thread_current_ptr -> tx_thread_time_slice =  _tx_timer_time_slice;
;        _tx_timer_time_slice =  0; 
;
    STR     r2, [r0, #24]                       ; Save thread's time-slice
    MOV     r2, #0                              ; Clear value
    STR     r2, [r3]                            ; Disable global time-slice flag
;
;    }
__tx_thread_fiq_dont_save_ts
;
;
;    /* Clear the current task pointer.  */
;    _tx_thread_current_ptr =  TX_NULL;
;
    MOV     r0, #0                              ; NULL value
    STR     r0, [r1]                            ; Clear current thread pointer
;
;    /* Return to the scheduler.  */
;    _tx_thread_schedule();
;
    B       _tx_thread_schedule                 ; Return to scheduler
;    }
;
__tx_thread_fiq_idle_system_restore
;
;    /* Just return back to the scheduler!  */
;
    ADD     sp, sp, #24                         ; Recover FIQ stack space
    MRS     r3, CPSR                            ; Pickup current CPSR
    BIC     r3, r3, #MODE_MASK                  ; Clear the mode portion of the CPSR
    ORR     r3, r3, #SVC_MODE_BITS              ; Or-in new interrupt lockout bit
    MSR     CPSR_cxsf, r3                       ; Lockout interrupts
    B       _tx_thread_schedule                 ; Return to scheduler
;
;}
;
;
    END

