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
    IMPORT      _tx_thread_current_ptr
    IMPORT      _tx_timer_time_slice
    IMPORT      _tx_thread_schedule
    IMPORT      _tx_thread_preempt_disable
    IMPORT      _tx_thread_smp_protection
    IF :DEF:TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    IMPORT      _tx_execution_thread_exit        
    ENDIF
;
;
        AREA ||.text||, CODE, READONLY
        PRESERVE8
;/**************************************************************************/ 
;/*                                                                        */ 
;/*  FUNCTION                                               RELEASE        */ 
;/*                                                                        */ 
;/*    _tx_thread_system_return                        SMP/Cortex-A7/AC5   */
;/*                                                            6.1         */
;/*  AUTHOR                                                                */
;/*                                                                        */
;/*    William E. Lamie, Microsoft Corporation                             */
;/*                                                                        */
;/*  DESCRIPTION                                                           */
;/*                                                                        */ 
;/*    This function is target processor specific.  It is used to transfer */ 
;/*    control from a thread back to the ThreadX system.  Only a           */ 
;/*    minimal context is saved since the compiler assumes temp registers  */ 
;/*    are going to get slicked by a function call anyway.                 */ 
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
;/*    _tx_thread_schedule                   Thread scheduling loop        */ 
;/*                                                                        */ 
;/*  CALLED BY                                                             */ 
;/*                                                                        */ 
;/*    ThreadX components                                                  */ 
;/*                                                                        */ 
;/*  RELEASE HISTORY                                                       */ 
;/*                                                                        */ 
;/*    DATE              NAME                      DESCRIPTION             */
;/*                                                                        */
;/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
;/*                                                                        */
;/**************************************************************************/
;VOID   _tx_thread_system_return(VOID)
;{
    EXPORT  _tx_thread_system_return
_tx_thread_system_return
;
;    /* Save minimal context on the stack.  */
;
    STMDB   sp!, {r4-r11, lr}                   ; Save minimal context
;
;    /* Pickup the CPU ID.   */
;
    MRC     p15, 0, r10, c0, c0, 5              ; Read CPU ID register
    AND     r10, r10, #0x03                     ; Mask off, leaving the CPU ID field
    LSL     r12, r10, #2                        ; Build offset to array indexes

    LDR     r3, =_tx_thread_current_ptr         ; Pickup address of current ptr
    ADD     r3, r3, r12                         ; Build index into current ptr array
    LDR     r0, [r3, #0]                        ; Pickup current thread pointer
    IF  {TARGET_FPU_VFP} = {TRUE}
    LDR     r1, [r0, #160]                      ; Pickup the VFP enabled flag
    CMP     r1, #0                              ; Is the VFP enabled?
    BEQ     _tx_skip_solicited_vfp_save         ; No, skip VFP solicited save
    VMRS    r4, FPSCR                           ; Pickup the FPSCR
    STR     r4, [sp, #-4]!                      ; Save FPSCR
    VSTMDB  sp!, {D16-D31}                      ; Save D16-D31
    VSTMDB  sp!, {D8-D15}                       ; Save D8-D15
_tx_skip_solicited_vfp_save
    ENDIF
    MOV     r4, #0                              ; Build a solicited stack type
    MRS     r5, CPSR                            ; Pickup the CPSR
    STMDB   sp!, {r4-r5}                        ; Save type and CPSR
;   
;   /* Lockout interrupts.  */
;
    IF :DEF:TX_ENABLE_FIQ_SUPPORT
    CPSID   if                                  ; Disable IRQ and FIQ interrupts
    ELSE
    CPSID   i                                   ; Disable IRQ interrupts
    ENDIF

    IF :DEF:TX_ENABLE_EXECUTION_CHANGE_NOTIFY
;
;    /* Call the thread exit function to indicate the thread is no longer executing.  */
;
    MOV     r4, r0                              ; Save r0
    MOV     r5, r3                              ; Save r3
    MOV     r6, r12                             ; Save r12
    BL      _tx_execution_thread_exit           ; Call the thread exit function
    MOV     r3, r5                              ; Recover r3
    MOV     r0, r4                              ; Recover r4
    MOV     r12,r6                              ; Recover r12
    ENDIF
;
    LDR     r2, =_tx_timer_time_slice           ; Pickup address of time slice
    ADD     r2, r2, r12                         ; Build index into time-slice array
    LDR     r1, [r2, #0]                        ; Pickup current time slice
;
;    /* Save current stack and switch to system stack.  */
;    _tx_thread_current_ptr[core] -> tx_thread_stack_ptr =  sp;
;    sp = _tx_thread_system_stack_ptr[core];
;
    STR     sp, [r0, #8]                        ; Save thread stack pointer
;
;    /* Determine if the time-slice is active.  */
;    if (_tx_timer_time_slice[core])
;    {
;
    MOV     r4, #0                              ; Build clear value
    CMP     r1, #0                              ; Is a time-slice active?
    BEQ     __tx_thread_dont_save_ts            ; No, don't save the time-slice
;
;       /* Save time-slice for the thread and clear the current time-slice.  */
;       _tx_thread_current_ptr[core] -> tx_thread_time_slice =  _tx_timer_time_slice[core];
;       _tx_timer_time_slice[core] =  0;
;
    STR     r4, [r2, #0]                        ; Clear time-slice
    STR     r1, [r0, #24]                       ; Save current time-slice
;
;    }
__tx_thread_dont_save_ts
;
;    /* Clear the current thread pointer.  */
;    _tx_thread_current_ptr[core] =  TX_NULL;
;
    STR     r4, [r3, #0]                        ; Clear current thread pointer
;
;    /* Set ready bit in thread control block.  */
;
    LDR     r2, [r0, #152]                      ; Pickup word with ready bit
    ORR     r2, r2, #0x8000                     ; Build ready bit set
    DMB                                         ; Ensure that accesses to shared resource have completed
    STR     r2, [r0, #152]                      ; Set ready bit
;
;    /* Now clear protection. It is assumed that protection is in force whenever this routine is called.  */
;
    LDR     r3, =_tx_thread_smp_protection      ; Pickup address of protection structure

    IF  :DEF:TX_MPCORE_DEBUG_ENABLE
    STR     lr, [r3, #24]                       ; Save last caller
    LDR     r2, [r3, #4]                        ; Pickup owning thread
    CMP     r0, r2                              ; Is it the same as the current thread?
__error_loop
    BNE     __error_loop                        ; If not, we have a problem!!
    ENDIF    
    
    LDR     r1, =_tx_thread_preempt_disable     ; Build address to preempt disable flag
    MOV     r2, #0                              ; Build clear value
    STR     r2, [r1, #0]                        ; Clear preempt disable flag
    STR     r2, [r3, #12]                       ; Clear protection count
    MOV     r1, #0xFFFFFFFF                     ; Build invalid value
    STR     r1, [r3, #8]                        ; Set core to an invalid value
    DMB                                         ; Ensure that accesses to shared resource have completed
    STR     r2, [r3]                            ; Clear protection
    DSB                                         ; To ensure update of the shared resource occurs before other CPUs awake
    SEV                                         ; Send event to other CPUs, wakes anyone waiting on a mutex (using WFE)

    B       _tx_thread_schedule                 ; Jump to scheduler!
;
;}
    END

