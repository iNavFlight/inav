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

    IMPORT  _tx_thread_current_ptr
    IMPORT  _tx_thread_execute_ptr
    IMPORT  _tx_timer_time_slice
    IMPORT  _tx_thread_system_stack_ptr
    IMPORT  _tx_thread_preempt_disable
#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
    IMPORT  _tx_execution_thread_enter
    IMPORT  _tx_execution_thread_exit
#endif

    AREA ||.text||, CODE, READONLY
    PRESERVE8
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_schedule                              Cortex-M4/AC5      */
/*                                                           6.1.11       */
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
/*  06-02-2021      Scott Larson            Initial Version 6.1.7         */
/*  04-25-2022      Scott Larson            Added BASEPRI support,        */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
// VOID   _tx_thread_schedule(VOID)
// {
    EXPORT  _tx_thread_schedule
_tx_thread_schedule

    /* This function should only ever be called on Cortex-M
       from the first schedule request. Subsequent scheduling occurs
       from the PendSV handling routine below. */

    /* Clear the preempt-disable flag to enable rescheduling after initialization on Cortex-M targets.  */

    MOV     r0, #0                                  // Build value for TX_FALSE
    LDR     r2, =_tx_thread_preempt_disable         // Build address of preempt disable flag
    STR     r0, [r2, #0]                            // Clear preempt disable flag

    /* Clear CONTROL.FPCA bit so VFP registers aren't unnecessarily stacked.  */

#ifdef __TARGET_FPU_VFP
    MRS     r0, CONTROL                             // Pickup current CONTROL register
    BIC     r0, r0, #4                              // Clear the FPCA bit
    MSR     CONTROL, r0                             // Setup new CONTROL register
#endif

    /* Enable interrupts */
    CPSIE   i

    /* Enter the scheduler for the first time.  */

    MOV     r0, #0x10000000                         // Load PENDSVSET bit
    MOV     r1, #0xE000E000                         // Load NVIC base
    STR     r0, [r1, #0xD04]                        // Set PENDSVBIT in ICSR
    DSB                                             // Complete all memory accesses
    ISB                                             // Flush pipeline

    /* Wait here for the PendSV to take place.  */

__tx_wait_here
    B       __tx_wait_here                          // Wait for the PendSV to happen
// }

    /* Generic context switching PendSV handler.  */

    EXPORT  __tx_PendSVHandler
    EXPORT  PendSV_Handler
__tx_PendSVHandler
PendSV_Handler

    /* Get current thread value and new thread pointer.  */

__tx_ts_handler

#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
    /* Call the thread exit function to indicate the thread is no longer executing.  */
#ifdef TX_PORT_USE_BASEPRI
    LDR     r1, =TX_PORT_BASEPRI                    // Mask interrupt priorities =< TX_PORT_BASEPRI
    MSR     BASEPRI, r1
#else
    CPSID   i                                       // Disable interrupts
#endif  /* TX_PORT_USE_BASEPRI */
    PUSH    {r0, lr}                                // Save LR (and r0 just for alignment)
    BL      _tx_execution_thread_exit               // Call the thread exit function
    POP     {r0, lr}                                // Recover LR
#ifdef TX_PORT_USE_BASEPRI
    MOV     r0, 0                                   // Disable BASEPRI masking (enable interrupts)
    MSR     BASEPRI, r0
#else
    CPSIE   i                                       // Enable interrupts
#endif  /* TX_PORT_USE_BASEPRI */
#endif  /* EXECUTION PROFILE */

    LDR     r0, =_tx_thread_current_ptr             // Build current thread pointer address
    LDR     r2, =_tx_thread_execute_ptr             // Build execute thread pointer address
    MOV     r3, #0                                  // Build NULL value
    LDR     r1, [r0]                                // Pickup current thread pointer

    /* Determine if there is a current thread to finish preserving.  */

    CBZ     r1, __tx_ts_new                         // If NULL, skip preservation

    /* Recover PSP and preserve current thread context.  */

    STR     r3, [r0]                                // Set _tx_thread_current_ptr to NULL
    MRS     r12, PSP                                // Pickup PSP pointer (thread's stack pointer)
    STMDB   r12!, {r4-r11}                          // Save its remaining registers
#ifdef __TARGET_FPU_VFP
    TST     LR, #0x10                               // Determine if the VFP extended frame is present
    BNE     _skip_vfp_save
    VSTMDB  r12!,{s16-s31}                          // Yes, save additional VFP registers
_skip_vfp_save
#endif
    LDR     r4, =_tx_timer_time_slice               // Build address of time-slice variable
    STMDB   r12!, {LR}                              // Save LR on the stack

    /* Determine if time-slice is active. If it isn't, skip time handling processing.  */

    LDR     r5, [r4]                                // Pickup current time-slice
    STR     r12, [r1, #8]                           // Save the thread stack pointer
    CBZ     r5, __tx_ts_new                         // If not active, skip processing

    /* Time-slice is active, save the current thread's time-slice and clear the global time-slice variable.  */

    STR     r5, [r1, #24]                           // Save current time-slice

    /* Clear the global time-slice.  */

    STR     r3, [r4]                                // Clear time-slice

    /* Executing thread is now completely preserved!!!  */

__tx_ts_new

    /* Now we are looking for a new thread to execute!  */

#ifdef TX_PORT_USE_BASEPRI
    LDR     r1, =TX_PORT_BASEPRI                    // Mask interrupt priorities =< TX_PORT_BASEPRI
    MSR     BASEPRI, r1
#else
    CPSID   i                                       // Disable interrupts
#endif
    LDR     r1, [r2]                                // Is there another thread ready to execute?
    CBZ     r1, __tx_ts_wait                        // No, skip to the wait processing

    /* Yes, another thread is ready for else, make the current thread the new thread.  */

    STR     r1, [r0]                                // Setup the current thread pointer to the new thread
#ifdef TX_PORT_USE_BASEPRI
    MOV     r4, #0                                  // Disable BASEPRI masking (enable interrupts)
    MSR     BASEPRI, r4
#else
    CPSIE   i                                       // Enable interrupts
#endif

    /* Increment the thread run count.  */

__tx_ts_restore
    LDR     r7, [r1, #4]                            // Pickup the current thread run count
    LDR     r4, =_tx_timer_time_slice               // Build address of time-slice variable
    LDR     r5, [r1, #24]                           // Pickup thread's current time-slice
    ADD     r7, r7, #1                              // Increment the thread run count
    STR     r7, [r1, #4]                            // Store the new run count

    /* Setup global time-slice with thread's current time-slice.  */

    STR     r5, [r4]                                // Setup global time-slice

#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
    /* Call the thread entry function to indicate the thread is executing.  */
    PUSH    {r0, r1}                                // Save r0 and r1
    BL      _tx_execution_thread_enter              // Call the thread execution enter function
    POP     {r0, r1}                                // Recover r0 and r1
#endif

    /* Restore the thread context and PSP.  */

    LDR     r12, [r1, #8]                           // Pickup thread's stack pointer
    LDMIA   r12!, {LR}                              // Pickup LR
#ifdef __TARGET_FPU_VFP
    TST     LR, #0x10                               // Determine if the VFP extended frame is present
    BNE     _skip_vfp_restore                       // If not, skip VFP restore
    VLDMIA  r12!, {s16-s31}                         // Yes, restore additional VFP registers
_skip_vfp_restore
#endif
    LDMIA   r12!, {r4-r11}                          // Recover thread's registers
    MSR     PSP, r12                                // Setup the thread's stack pointer

    /* Return to thread.  */

    BX      lr                                      // Return to thread!

    /* The following is the idle wait processing... in this case, no threads are ready for execution and the
       system will simply be idle until an interrupt occurs that makes a thread ready. Note that interrupts
       are disabled to allow use of WFI for waiting for a thread to arrive.  */

__tx_ts_wait
#ifdef TX_PORT_USE_BASEPRI
    LDR     r1, =TX_PORT_BASEPRI                    // Mask interrupt priorities =< TX_PORT_BASEPRI
    MSR     BASEPRI, r1
#else
    CPSID   i                                       // Disable interrupts
#endif
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

#ifdef TX_PORT_USE_BASEPRI
    MOV     r4, #0                                  // Disable BASEPRI masking (enable interrupts)
    MSR     BASEPRI, r4
#else
    CPSIE   i                                       // Enable interrupts
#endif
    B       __tx_ts_wait                            // Loop to continue waiting

    /* At this point, we have a new thread ready to go. Clear any newly pended PendSV - since we are
       already in the handler!  */

__tx_ts_ready
    MOV     r7, #0x08000000                         // Build clear PendSV value
    MOV     r8, #0xE000E000                         // Build base NVIC address
    STR     r7, [r8, #0xD04]                        // Clear any PendSV

    /* Re-enable interrupts and restore new thread.  */
#ifdef TX_PORT_USE_BASEPRI
    MOV     r4, #0                                  // Disable BASEPRI masking (enable interrupts)
    MSR     BASEPRI, r4
#else
    CPSIE   i                                       // Enable interrupts
#endif
    B       __tx_ts_restore                         // Restore the thread
// }

#ifdef __TARGET_FPU_VFP
    EXPORT  tx_thread_fpu_enable
tx_thread_fpu_enable
    EXPORT  tx_thread_fpu_disable
tx_thread_fpu_disable

    /* Automatic VPF logic is supported, this function is present only for
       backward compatibility purposes and therefore simply returns.  */

    BX      LR                                      // Return to caller

    EXPORT  _tx_vfp_access
_tx_vfp_access
    VMOV.F32 s0, s0                                 // Simply access the VFP
    BX       lr                                     // Return to caller

#endif

    ALIGN
    LTORG
    END
