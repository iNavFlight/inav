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
/**   Thread                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_schedule                               Cortex-M23/GNU    */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function waits for a thread control block pointer to appear in */
/*    the _tx_thread_execute_ptr variable.  Once a thread pointer appears */
/*    in the variable, the corresponding thread is resumed.               */
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
/*    _tx_initialize_kernel_enter          ThreadX entry function         */
/*    _tx_thread_system_return             Return to system from thread   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*  04-02-2021      Scott Larson            Modified comment(s), added    */
/*                                            low power code,             */
/*                                            resulting in version 6.1.6  */
/*  06-02-2021      Scott Larson            Added secure stack initialize */
/*                                            in SVC handler,             */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/
// VOID   _tx_thread_schedule(VOID)
// {
    .section .text
    .balign 4
    .syntax unified
    .eabi_attribute Tag_ABI_align_preserved, 1
    .global  _tx_thread_schedule
    .thumb_func
.type _tx_thread_schedule, function
_tx_thread_schedule:
    /* This function should only ever be called on Cortex-M
       from the first schedule request. Subsequent scheduling occurs
       from the PendSV handling routine below. */

    /* Clear the preempt-disable flag to enable rescheduling after initialization on Cortex-M targets.  */
    MOVW    r0, #0                                  // Build value for TX_FALSE
    LDR     r2, =_tx_thread_preempt_disable         // Build address of preempt disable flag
    STR     r0, [r2, #0]                            // Clear preempt disable flag

    /* Enable interrupts */
    CPSIE   i

    /* Enter the scheduler for the first time.  */
    LDR     r0, =0x10000000                         // Load PENDSVSET bit
    LDR     r1, =0xE000ED04                         // Load ICSR address
    STR     r0, [r1]                                // Set PENDSVBIT in ICSR
    DSB                                             // Complete all memory accesses
    ISB                                             // Flush pipeline

    /* Wait here for the PendSV to take place.  */

__tx_wait_here:
    B       __tx_wait_here                          // Wait for the PendSV to happen
// }

    /* Generic context switching PendSV handler.  */

    .section .text
    .balign 4
    .syntax unified
    .eabi_attribute Tag_ABI_align_preserved, 1
    .global  PendSV_Handler
    .thumb_func
.type PendSV_Handler, function
    /* Get current thread value and new thread pointer.  */
PendSV_Handler:
__tx_ts_handler:

#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
    /* Call the thread exit function to indicate the thread is no longer executing.  */
    CPSID   i                                       // Disable interrupts
    PUSH    {r0, lr}                                // Save LR (and r0 just for alignment)
    BL      _tx_execution_thread_exit               // Call the thread exit function
    POP     {r0, r1}                                // Recover LR
    MOV     lr, r1                                  //
    CPSIE   i                                       // Enable interrupts
#endif

    LDR     r0, =_tx_thread_current_ptr             // Build current thread pointer address
    LDR     r2, =_tx_thread_execute_ptr             // Build execute thread pointer address
    MOVW    r3, #0                                  // Build NULL value
    LDR     r1, [r0]                                // Pickup current thread pointer

    /* Determine if there is a current thread to finish preserving.  */

    CBZ     r1, __tx_ts_new                         // If NULL, skip preservation

    /* Recover PSP and preserve current thread context.  */

    STR     r3, [r0]                                // Set _tx_thread_current_ptr to NULL
    MRS     r3, PSP                                 // Pickup PSP pointer (thread's stack pointer)
    SUBS    r3, r3, #16                             // Allocate stack space
    STM     r3!, {r4-r7}                            // Save r4-r7 (M4 Instruction: STMDB r12!, {r4-r11})
    MOV     r4, r8                                  // Copy r8-r11 to multisave registers
    MOV     r5, r9
    MOV     r6, r10
    MOV     r7, r11
    SUBS    r3, r3, #32                             // Allocate stack space
    STM     r3!, {r4-r7}                            // Save r8-r11
    SUBS    r3, r3, #20                             // Allocate stack space
    MOV     r5, lr                                  // Copy lr to saveable register
    STR     r5, [r3]                                // Save lr on the stack
    STR     r3, [r1, #8]                            // Save the thread stack pointer

#if (!defined(TX_SINGLE_MODE_SECURE) && !defined(TX_SINGLE_MODE_NON_SECURE))
    // Save secure context
    LDR     r5, =0x90                               // Secure stack index offset
    LDR     r5, [r1, r5]                            // Load secure stack index
    CBZ     r5, _skip_secure_save                   // Skip save if there is no secure context
    PUSH    {r0-r3}                                 // Save scratch registers
    MOV     r0, r1                                  // Move thread ptr to r0
    BL      _tx_thread_secure_stack_context_save    // Save secure stack
    POP     {r0-r3}                                 // Restore secure registers
_skip_secure_save:
#endif

    /* Determine if time-slice is active. If it isn't, skip time handling processing.  */

    LDR     r4, =_tx_timer_time_slice               // Build address of time-slice variable
    LDR     r5, [r4]                                // Pickup current time-slice
    CBZ     r5, __tx_ts_new                         // If not active, skip processing

    /* Time-slice is active, save the current thread's time-slice and clear the global time-slice variable.  */

    STR     r5, [r1, #24]                           // Save current time-slice

    /* Clear the global time-slice.  */

    MOVW    r5, #0                                  // Build clear value
    STR     r5, [r4]                                // Clear time-slice

    /* Executing thread is now completely preserved!!!  */

__tx_ts_new:

    /* Now we are looking for a new thread to execute!  */

    CPSID   i                                       // Disable interrupts
    LDR     r1, [r2]                                // Is there another thread ready to execute?
    CBZ     r1, __tx_ts_wait                        // No, skip to the wait processing

    /* Yes, another thread is ready for else, make the current thread the new thread.  */

    STR     r1, [r0]                                // Setup the current thread pointer to the new thread
    CPSIE   i                                       // Enable interrupts

    /* Increment the thread run count.  */

__tx_ts_restore:
    LDR     r7, [r1, #4]                            // Pickup the current thread run count
    LDR     r4, =_tx_timer_time_slice               // Build address of time-slice variable
    LDR     r5, [r1, #24]                           // Pickup thread's current time-slice
    ADDS    r7, r7, #1                              // Increment the thread run count
    STR     r7, [r1, #4]                            // Store the new run count

    /* Setup global time-slice with thread's current time-slice.  */

    STR     r5, [r4]                                // Setup global time-slice

#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
    /* Call the thread entry function to indicate the thread is executing.  */
    PUSH    {r0, r1}                                // Save r0 and r1
    BL      _tx_execution_thread_enter              // Call the thread execution enter function
    POP     {r0, r1}                                // Recover r0 and r1
#endif

#if (!defined(TX_SINGLE_MODE_SECURE) && !defined(TX_SINGLE_MODE_NON_SECURE))
    // Restore secure context
    LDR     r5, =0x90                               // Secure stack index offset
    LDR     r0, [r1, r5]                            // Load secure stack index
    CBZ     r0, _skip_secure_restore                // Skip restore if there is no secure context
    PUSH    {r0, r1}                                // Save r1 (and dummy r0)
    MOV     r0, r1                                  // Move thread ptr to r0
    BL      _tx_thread_secure_stack_context_restore // Restore secure stack
    POP     {r0, r1}                                // Restore r1 (and dummy r0)
_skip_secure_restore:
#endif

    /* Restore the thread context and PSP.  */
#ifdef TX_SINGLE_MODE_SECURE
    // There are only stack limit registers in secure mode on the M23
    LDR     r3, [r1, #12]                           // Get stack start
    MSR     PSPLIM, r3                              // Set stack limit
#endif
    LDR     r3, [r1, #8]                            // Pickup thread's stack pointer
    LDR     r5, [r3]                                // Recover saved LR
    ADDS    r3, r3, #4                              // Position past LR
    MOV     lr, r5                                  // Restore LR
    LDM     r3!, {r4-r7}                            // Recover thread's registers (r8-r11)
    MOV     r11, r7
    MOV     r10, r6
    MOV     r9, r5
    MOV     r8, r4
    LDM     r3!, {r4-r7}                            // Recover thread's registers (r4-r7)
    MSR     PSP, r3                                 // Setup the thread's stack pointer

    BX      lr                                      // Return to thread!

    /* The following is the idle wait processing... in this case, no threads are ready for execution and the
       system will simply be idle until an interrupt occurs that makes a thread ready. Note that interrupts
       are disabled to allow use of WFI for waiting for a thread to arrive.  */

__tx_ts_wait:
    CPSID   i                                       // Disable interrupts
    LDR     r1, [r2]                                // Pickup the next thread to execute pointer
    STR     r1, [r0]                                // Store it in the current pointer
    CBNZ    r1, __tx_ts_ready                       // If non-NULL, a new thread is ready!

#ifdef TX_LOW_POWER
    PUSH    {r0-r3}
    BL      tx_low_power_enter                      // Possibly enter low power mode
    POP     {r0-r3}
#endif

#ifdef TX_ENABLE_WFI
    DSB                                             // Ensure no outstanding memory transactions
    WFI                                             // Wait for interrupt
    ISB                                             // Ensure pipeline is flushed
#endif

#ifdef TX_LOW_POWER
    PUSH    {r0-r3}
    BL      tx_low_power_exit                       // Exit low power mode
    POP     {r0-r3}
#endif

    CPSIE   i                                       // Enable interrupts
    B       __tx_ts_wait                            // Loop to continue waiting

    /* At this point, we have a new thread ready to go. Clear any newly pended PendSV - since we are
       already in the handler!  */
__tx_ts_ready:
    LDR     r7, =0x08000000                         // Build clear PendSV value
    LDR     r5, =0xE000ED04                         // Build ICSR address
    STR     r7, [r5]                                // Clear any PendSV

    /* Re-enable interrupts and restore new thread.  */
    CPSIE   i                                       // Enable interrupts
    B       __tx_ts_restore                         // Restore the thread
// }


#if (!defined(TX_SINGLE_MODE_SECURE) && !defined(TX_SINGLE_MODE_NON_SECURE))
    // SVC_Handler is not needed when ThreadX is running in single mode.
    .section .text
    .balign 4
    .syntax unified
    .eabi_attribute Tag_ABI_align_preserved, 1
    .global  SVC_Handler
    .thumb_func
.type SVC_Handler, function
SVC_Handler:
    MOV     r0, lr
    MOVS    r1, #0x04
    TST     r1, r0                                  // Determine return stack from EXC_RETURN bit 2
    BEQ     _tx_load_msp
    MRS     r0, PSP                                 // Get PSP if return stack is PSP
    B       _tx_get_svc
_tx_load_msp:
    MRS     r0, MSP                                 // Get MSP if return stack is MSP
_tx_get_svc:
    LDR     r1, [r0,#24]                            // Load saved PC from stack
    LDR     r3, =-2
    LDRB    r1, [r1,r3]                             // Load SVC number

    CMP     r1, #1                                  // Is it a secure stack allocate request?
    BEQ     _tx_svc_secure_alloc                    // Yes, go there

    CMP     r1, #2                                  // Is it a secure stack free request?
    BEQ     _tx_svc_secure_free                     // Yes, go there
    
    CMP     r1, #3                                  // Is it a secure stack init request?
    BEQ     _tx_svc_secure_init                     // Yes, go there

    // Unknown SVC argument - just return
    BX      lr

_tx_svc_secure_alloc:
    PUSH    {r0, lr}                // Save SP and EXC_RETURN
    LDM     r0, {r0-r3}             // Load function parameters from stack
    BL      _tx_thread_secure_mode_stack_allocate
    POP     {r1, r2}                // Restore SP and EXC_RETURN
    STR     r0, [r1]                // Store function return value
    MOV     lr, r2
    BX      lr
_tx_svc_secure_free:
    PUSH    {r0, lr}                // Save SP and EXC_RETURN
    LDM     r0, {r0-r3}             // Load function parameters from stack
    BL      _tx_thread_secure_mode_stack_free
    POP     {r1, r2}                // Restore SP and EXC_RETURN
    STR     r0, [r1]                // Store function return value
    MOV     lr, r2
    BX      lr
_tx_svc_secure_init:
    PUSH    {r0,lr}                 // Save SP and EXC_RETURN
    BL      _tx_thread_secure_mode_stack_initialize
    POP     {r1, r2}                // Restore SP and EXC_RETURN
    MOV     lr, r2
    BX      lr
#endif  // End of ifndef TX_SINGLE_MODE_SECURE, TX_SINGLE_MODE_NON_SECURE

.end
