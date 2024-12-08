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
    IMPORT  _tx_thread_current_ptr
    IMPORT  _tx_thread_execute_ptr
    IMPORT  _tx_timer_time_slice
    IMPORT  _tx_thread_system_stack_ptr
    IMPORT  _tx_thread_preempt_disable
#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))           
    IMPORT  _tx_execution_thread_enter
    IMPORT  _tx_execution_thread_exit
#endif
    IF :DEF:TX_LOW_POWER
    IMPORT  tx_low_power_enter
    IMPORT  tx_low_power_exit
    ENDIF
;
;
    AREA    ||.text||, CODE, READONLY
    PRESERVE8
;/**************************************************************************/
;/*                                                                        */
;/*  FUNCTION                                               RELEASE        */
;/*                                                                        */
;/*    _tx_thread_schedule                               Cortex-M0/AC5     */
;/*                                                           6.1.5        */
;/*  AUTHOR                                                                */
;/*                                                                        */
;/*    William E. Lamie, Microsoft Corporation                             */
;/*                                                                        */
;/*  DESCRIPTION                                                           */
;/*                                                                        */
;/*    This function waits for a thread control block pointer to appear in */
;/*    the _tx_thread_execute_ptr variable.  Once a thread pointer appears */
;/*    in the variable, the corresponding thread is resumed.               */
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
;/*    _tx_initialize_kernel_enter          ThreadX entry function         */
;/*    _tx_thread_system_return             Return to system from thread   */
;/*    _tx_thread_context_restore           Restore thread's context       */
;/*                                                                        */
;/*  RELEASE HISTORY                                                       */
;/*                                                                        */
;/*    DATE              NAME                      DESCRIPTION             */
;/*                                                                        */
;/*  09-30-2020     William E. Lamie        Initial Version 6.1            */
;/*  03-02-2021     Scott Larson            Modified comment(s), add       */
;/*                                           low power code,              */
;/*                                           resulting in version 6.1.5   */
;/*                                                                        */
;/**************************************************************************/
;VOID   _tx_thread_schedule(VOID)
;{
    EXPORT  _tx_thread_schedule
_tx_thread_schedule
;
;    /* This function should only ever be called on Cortex-M0
;       from the first schedule request. Subsequent scheduling occurs
;       from the PendSV handling routines below. */
;
;    /* Clear the preempt-disable flag to enable rescheduling after initialization on Cortex-M targets.  */
;
    MOVS    r0, #0                                  ; Build value for TX_FALSE
    LDR     r2, =_tx_thread_preempt_disable         ; Build address of preempt disable flag
    STR     r0, [r2, #0]                            ; Clear preempt disable flag
;
;    /* Enable interrupts */
;
    CPSIE   i
;
;    /* Enter the scheduler for the first time.  */
;
    LDR     r0, =0x10000000                         ; Load PENDSVSET bit
    LDR     r1, =0xE000ED04                         ; Load ICSR address
    STR     r0, [r1]                                ; Set PENDSVBIT in ICSR
    DSB                                             ; Complete all memory accesses
    ISB                                             ; Flush pipeline
;
;    /* Wait here for the PendSV to take place.  */
;
__tx_wait_here
    B       __tx_wait_here                          ; Wait for the PendSV to happen
;}
;
;    /* Generic context switch-out switch-in handler...  Note that this handler is
;       common for both PendSV and SVCall.  */
;
    EXPORT  PendSV_Handler
    EXPORT  __tx_PendSVHandler
PendSV_Handler
__tx_PendSVHandler
;
;    /* Get current thread value and new thread pointer.  */
;
__tx_ts_handler

#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))           
;
;    /* Call the thread exit function to indicate the thread is no longer executing.  */
;
    CPSID   i                                       ; Disable interrupts
    PUSH    {r0, lr}                                ; Save LR (and r0 just for alignment)
    BL      _tx_execution_thread_exit               ; Call the thread exit function
    POP     {r0, r1}                                ; Recover LR
    MOV     lr, r1                                  ;
    CPSIE   i                                       ; Enable interrupts
#endif
    LDR     r0, =_tx_thread_current_ptr             ; Build current thread pointer address
    LDR     r2, =_tx_thread_execute_ptr             ; Build execute thread pointer address
    MOVS    r3, #0                                  ; Build NULL value
    LDR     r1, [r0]                                ; Pickup current thread pointer
;
;    /* Determine if there is a current thread to finish preserving.  */
;
    CMP     r1,#0                                   ; If NULL, skip preservation
    BEQ     __tx_ts_new                             ;
;
;    /* Recover PSP and preserve current thread context.  */
;
    STR     r3, [r0]                                ; Set _tx_thread_current_ptr to NULL
    MRS     r3, PSP                                 ; Pickup PSP pointer (thread's stack pointer)
    SUBS    r3, r3, #16                             ; Allocate stack space
    STM     r3!, {r4-r7}                            ; Save its remaining registers (M3 Instruction: STMDB r12!, {r4-r11})
    MOV     r4,r8                                   ;
    MOV     r5,r9                                   ;
    MOV     r6,r10                                  ;
    MOV     r7,r11                                  ;
    SUBS    r3, r3, #32                             ; Allocate stack space
    STM     r3!, {r4-r7}                            ;
    SUBS    r3, r3, #20                             ; Allocate stack space
    MOV     r5, LR                                  ;
    STR     r5, [r3]                                ; Save LR on the stack
    STR     r3, [r1, #8]                            ; Save its stack pointer
;
;    /* Determine if time-slice is active. If it isn't, skip time handling processing.  */
;
    LDR     r4, =_tx_timer_time_slice               ; Build address of time-slice variable
    LDR     r5, [r4]                                ; Pickup current time-slice
    CMP     r5, #0                                  ; If not active, skip processing
    BEQ     __tx_ts_new                             ;
;
;    /* Time-slice is active, save the current thread's time-slice and clear the global time-slice variable.  */
;
    STR     r5, [r1, #24]                           ; Save current time-slice
;
;    /* Clear the global time-slice.  */
;
    MOVS    r5, #0                                  ; Build clear value
    STR     r5, [r4]                                ; Clear time-slice
;
;
;    /* Executing thread is now completely preserved!!!  */
;
__tx_ts_new
;
;    /* Now we are looking for a new thread to execute!  */
;
    CPSID   i                                       ; Disable interrupts
    LDR     r1, [r2]                                ; Is there another thread ready to execute?
    CMP     r1, #0                                  ;
    BEQ     __tx_ts_wait                            ; No, skip to the wait processing
;
;    /* Yes, another thread is ready for else, make the current thread the new thread.  */
;
    STR     r1, [r0]                                ; Setup the current thread pointer to the new thread
    CPSIE   i                                       ; Enable interrupts
;
;    /* Increment the thread run count.  */
;
__tx_ts_restore
    LDR     r7, [r1, #4]                            ; Pickup the current thread run count
    LDR     r4, =_tx_timer_time_slice               ; Build address of time-slice variable
    LDR     r5, [r1, #24]                           ; Pickup thread's current time-slice
    ADDS    r7, r7, #1                              ; Increment the thread run count
    STR     r7, [r1, #4]                            ; Store the new run count
;
;    /* Setup global time-slice with thread's current time-slice.  */
;
    STR     r5, [r4]                                ; Setup global time-slice

#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))           
;
;    /* Call the thread entry function to indicate the thread is executing.  */
;
    PUSH    {r0, r1}                                ; Save r0/r1
    BL      _tx_execution_thread_enter              ; Call the thread execution enter function
    POP     {r0, r1}                                ; Recover r0/r1
#endif
;
;    /* Restore the thread context and PSP.  */
;
    LDR     r3, [r1, #8]                            ; Pickup thread's stack pointer
    LDR     r5, [r3]                                ; Recover saved LR
    ADDS    r3, r3, #4                              ; Position past LR
    MOV     lr, r5                                  ; Restore LR
    LDM     r3!,{r4-r7}                             ; Recover thread's registers (r4-r11)
    MOV     r11,r7                                  ;
    MOV     r10,r6                                  ;
    MOV     r9,r5                                   ;
    MOV     r8,r4                                   ;
    LDM     r3!,{r4-r7}                             ;
    MSR     PSP, r3                                 ; Setup the thread's stack pointer
;
;    /* Return to thread.  */
;
    BX      lr                                      ; Return to thread!
;
;    /* The following is the idle wait processing... in this case, no threads are ready for execution and the
;       system will simply be idle until an interrupt occurs that makes a thread ready. Note that interrupts
;       are disabled to allow use of WFI for waiting for a thread to arrive.  */
;
__tx_ts_wait
    CPSID   i                                       ; Disable interrupts
    LDR     r1, [r2]                                ; Pickup the next thread to execute pointer
    STR     r1, [r0]                                ; Store it in the current pointer
    CMP     r1, #0                                  ; If non-NULL, a new thread is ready!
    BNE     __tx_ts_ready                           ;

    IF :DEF:TX_LOW_POWER
    PUSH    {r0-r3}
    BL      tx_low_power_enter                      ; Possibly enter low power mode
    POP     {r0-r3}
    ENDIF

    IF :DEF:TX_ENABLE_WFI
    DSB                                             ; Ensure no outstanding memory transactions
    WFI                                             ; Wait for interrupt
    ISB                                             ; Ensure pipeline is flushed
    ENDIF

    IF :DEF:TX_LOW_POWER
    PUSH    {r0-r3}
    BL      tx_low_power_exit                       ; Exit low power mode
    POP     {r0-r3}
    ENDIF

__tx_ts_ISB
    CPSIE   i                                       ; Enable interrupts
    B       __tx_ts_wait                            ; Loop to continue waiting
;
;    /* At this point, we have a new thread ready to go. Clear any newly pended PendSV - since we are
;       already in the handler!  */
;
__tx_ts_ready
    LDR     r7, =0x08000000                         ; Build clear PendSV value
    LDR     r5, =0xE000ED04                         ; Build base NVIC address
    STR     r7, [r5]                                ; Clear any PendSV
;
;    /* Re-enable interrupts and restore new thread.  */
;
    CPSIE   i                                       ; Enable interrupts
    B       __tx_ts_restore                         ; Restore the thread

    ALIGN
    LTORG
    END
