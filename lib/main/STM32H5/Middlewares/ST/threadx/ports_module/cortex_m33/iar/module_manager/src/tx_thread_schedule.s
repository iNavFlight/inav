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

    EXTERN  _tx_thread_current_ptr
    EXTERN  _tx_thread_execute_ptr
    EXTERN  _tx_timer_time_slice
    EXTERN  _tx_thread_system_stack_ptr
    EXTERN  _tx_thread_preempt_disable
    EXTERN  _tx_execution_thread_enter
    EXTERN  _tx_execution_thread_exit
    EXTERN  _txm_module_manager_memory_fault_handler
    EXTERN  _txm_module_manager_memory_fault_info
    EXTERN  _tx_thread_secure_stack_context_restore
    EXTERN  _tx_thread_secure_stack_context_save
    EXTERN  _tx_thread_secure_mode_stack_allocate
    EXTERN  _tx_thread_secure_mode_stack_free
    EXTERN  _tx_alloc_return
    EXTERN  _tx_free_return
    SECTION `.text`:CODE:NOROOT(2)
    THUMB
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_schedule                               Cortex-M33/IAR    */
/*                                                           6.2.0        */
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
/*  03-02-2021      Scott Larson            Initial Version 6.1.5         */
/*  04-02-2021      Scott Larson            Modified comments and fixed   */
/*                                            MPU region configuration,   */
/*                                            resulting in version 6.1.6  */
/*  06-02-2021      Scott Larson            Fixed extended stack handling */
/*                                            when calling kernel APIs,   */
/*                                            resulting in version 6.1.7  */
/*  04-25-2022      Scott Larson            Optimized MPU configuration,  */
/*                                            added BASEPRI support,      */
/*                                            resulting in version 6.1.11 */
/*  07-29-2022      Scott Larson            Removed the code path to skip */
/*                                            MPU reloading,              */
/*                                            resulting in version 6.1.12 */
/*  10-31-2022      Scott Larson            Added low power support,      */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
// VOID   _tx_thread_schedule(VOID)
// {
    PUBLIC  _tx_thread_schedule
_tx_thread_schedule:
    /* This function should only ever be called on Cortex-M
       from the first schedule request. Subsequent scheduling occurs
       from the PendSV handling routine below. */

    /* Clear the preempt-disable flag to enable rescheduling after initialization on Cortex-M targets.  */
    MOV     r0, #0                                  // Build value for TX_FALSE
    LDR     r2, =_tx_thread_preempt_disable         // Build address of preempt disable flag
    STR     r0, [r2, #0]                            // Clear preempt disable flag

#ifdef __ARMVFP__
    /* Clear CONTROL.FPCA bit so VFP registers aren't unnecessarily stacked.  */
    MRS     r0, CONTROL                             // Pickup current CONTROL register
    BIC     r0, r0, #4                              // Clear the FPCA bit
    MSR     CONTROL, r0                             // Setup new CONTROL register
#endif

    /* Enable memory fault registers.  */
    LDR     r0, =0xE000ED24                         // Build SHCSR address
    LDR     r1, =0x70000                            // Enable Usage, Bus, and MemManage faults
    STR     r1, [r0]                                //

    /* Enable interrupts */
    CPSIE   i

    /* Enter the scheduler for the first time.  */
    MOV     r0, #0x10000000                         // Load PENDSVSET bit
    MOV     r1, #0xE000E000                         // Load NVIC base
    STR     r0, [r1, #0xD04]                        // Set PENDSVBIT in ICSR
    DSB                                             // Complete all memory accesses
    ISB                                             // Flush pipeline

    /* Wait here for the PendSV to take place.  */

__tx_wait_here:
    B       __tx_wait_here                          // Wait for the PendSV to happen
// }


    /* Memory Exception Handler.  */

    PUBLIC  MemManage_Handler
    PUBLIC  BusFault_Handler
MemManage_Handler:
BusFault_Handler:

#ifdef TX_PORT_USE_BASEPRI
    LDR     r1, =TX_PORT_BASEPRI                    // Mask interrupt priorities =< TX_PORT_BASEPRI
    MSR     BASEPRI, r1
#else
    CPSID   i                                       // Disable interrupts
#endif  /* TX_PORT_USE_BASEPRI */

    /* Now pickup and store all the fault related information.  */

    LDR     r12,=_txm_module_manager_memory_fault_info  // Pickup fault info struct
    LDR     r0, =_tx_thread_current_ptr             // Build current thread pointer address
    LDR     r1, [r0]                                // Pickup the current thread pointer
    STR     r1, [r12, #0]                           // Save current thread pointer in fault info structure
    LDR     r0, =0xE000ED24                         // Build SHCSR address
    LDR     r1, [r0]                                // Pickup SHCSR
    STR     r1, [r12, #8]                           // Save SHCSR
    LDR     r0, =0xE000ED28                         // Build CFSR address
    LDR     r1, [r0]                                // Pickup CFSR
    STR     r1, [r12, #12]                          // Save CFSR
    LDR     r0, =0xE000ED34                         // Build MMFAR address
    LDR     r1, [r0]                                // Pickup MMFAR
    STR     r1, [r12, #16]                          // Save MMFAR
    LDR     r0, =0xE000ED38                         // Build BFAR address
    LDR     r1, [r0]                                // Pickup BFAR
    STR     r1, [r12, #20]                          // Save BFAR
    MRS     r0, CONTROL                             // Pickup current CONTROL register
    STR     r0, [r12, #24]                          // Save CONTROL
    MRS     r1, PSP                                 // Pickup thread stack pointer
    STR     r1, [r12, #28]                          // Save thread stack pointer
    LDR     r0, [r1]                                // Pickup saved r0
    STR     r0, [r12, #32]                          // Save r0
    LDR     r0, [r1, #4]                            // Pickup saved r1
    STR     r0, [r12, #36]                          // Save r1
    STR     r2, [r12, #40]                          // Save r2
    STR     r3, [r12, #44]                          // Save r3
    STR     r4, [r12, #48]                          // Save r4
    STR     r5, [r12, #52]                          // Save r5
    STR     r6, [r12, #56]                          // Save r6
    STR     r7, [r12, #60]                          // Save r7
    STR     r8, [r12, #64]                          // Save r8
    STR     r9, [r12, #68]                          // Save r9
    STR     r10,[r12, #72]                          // Save r10
    STR     r11,[r12, #76]                          // Save r11
    LDR     r0, [r1, #16]                           // Pickup saved r12
    STR     r0, [r12, #80]                          // Save r12
    LDR     r0, [r1, #20]                           // Pickup saved lr
    STR     r0, [r12, #84]                          // Save lr
    LDR     r0, [r1, #24]                           // Pickup instruction address at point of fault
    STR     r0, [r12, #4]                           // Save point of fault
    LDR     r0, [r1, #28]                           // Pickup xPSR
    STR     r0, [r12, #88]                          // Save xPSR

    MRS     r0, CONTROL                             // Pickup current CONTROL register
    BIC     r0, r0, #1                              // Clear the UNPRIV bit
    MSR     CONTROL, r0                             // Setup new CONTROL register

    LDR     r0, =0xE000ED28                         // Build the Memory Management Fault Status Register (MMFSR)
    LDRB    r1, [r0]                                // Pickup the MMFSR, with the following bit definitions:
                                                    //     Bit 0 = 1 -> Instruction address violation
                                                    //     Bit 1 = 1 -> Load/store address violation
                                                    //     Bit 7 = 1 -> MMFAR is valid
    STRB    r1, [r0]                                // Clear the MMFSR

#ifdef __ARMVFP__
    LDR     r0, =0xE000EF34                         // Cleanup FPU context: Load FPCCR address
    LDR     r1, [r0]                                // Load FPCCR
    BIC     r1, r1, #1                              // Clear the lazy preservation active bit
    STR     r1, [r0]                                // Store the value
#endif

    BL      _txm_module_manager_memory_fault_handler    // Call memory manager fault handler

#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
    /* Call the thread exit function to indicate the thread is no longer executing.  */
    CPSID   i                                       // Disable interrupts
    BL      _tx_execution_thread_exit               // Call the thread exit function
    CPSIE   i                                       // Enable interrupts
#endif

    MOV     r1, #0                                  // Build NULL value
    LDR     r0, =_tx_thread_current_ptr             // Pickup address of current thread pointer
    STR     r1, [r0]                                // Clear current thread pointer

    // Return from MemManage_Handler exception
    LDR     r0, =0xE000ED04                         // Load ICSR
    LDR     r1, =0x10000000                         // Set PENDSVSET bit
    STR     r1, [r0]                                // Store ICSR
    DSB                                             // Wait for memory access to complete
#ifdef TX_PORT_USE_BASEPRI
    MOV     r0, 0                                   // Disable BASEPRI masking (enable interrupts)
    MSR     BASEPRI, r0
#else
    CPSIE   i                                       // Enable interrupts
#endif
#ifdef TX_SINGLE_MODE_SECURE
    LDR     lr, =0xFFFFFFFD                         // Exception return to secure
#else
    LDR     lr, =0xFFFFFFBC                         // Exception return to non-secure
#endif
    BX      lr                                      // Return from exception


    /* Generic context switching PendSV handler.  */

    PUBLIC  PendSV_Handler
PendSV_Handler:
__tx_ts_handler:

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
#ifdef __ARMVFP__
    TST     LR, #0x10                               // Determine if the VFP extended frame is present
    BNE     _skip_vfp_save
    VSTMDB  r12!,{s16-s31}                          // Yes, save additional VFP registers
_skip_vfp_save:
#endif
    LDR     r4, =_tx_timer_time_slice               // Build address of time-slice variable
    STMDB   r12!, {LR}                              // Save LR on the stack
    STR     r12, [r1, #8]                           // Save the thread stack pointer

#if (!defined(TX_SINGLE_MODE_SECURE) && !defined(TX_SINGLE_MODE_NON_SECURE))
    // Save secure context
    LDR     r5, [r1,#0xC4]                          // Load secure stack index
    CBZ     r5, _skip_secure_save                   // Skip save if there is no secure context
    PUSH    {r0,r1,r2,r3}                           // Save scratch registers
    MOV     r0, r1                                  // Move thread ptr to r0
    BL      _tx_thread_secure_stack_context_save    // Save secure stack
    POP     {r0,r1,r2,r3}                           // Restore secure registers
_skip_secure_save:
#endif

    /* Determine if time-slice is active. If it isn't, skip time handling processing.  */

    LDR     r5, [r4]                                // Pickup current time-slice
    CBZ     r5, __tx_ts_new                         // If not active, skip processing

    /* Time-slice is active, save the current thread's time-slice and clear the global time-slice variable.  */

    STR     r5, [r1, #24]                           // Save current time-slice

    /* Clear the global time-slice.  */

    STR     r3, [r4]                                // Clear time-slice

    /* Executing thread is now completely preserved!!!  */

__tx_ts_new:

    /* Now we are looking for a new thread to execute!  */

#ifdef TX_PORT_USE_BASEPRI
    LDR     r1, =TX_PORT_BASEPRI                    // Mask interrupt priorities =< TX_PORT_BASEPRI
    MSR     BASEPRI, r1
#else
    CPSID   i                                       // Disable interrupts
#endif
    LDR     r1, [r2]                                // Is there another thread ready to execute?
    CBNZ    r1, __tx_ts_restore                     // Yes, schedule it

    /* The following is the idle wait processing... in this case, no threads are ready for execution and the
       system will simply be idle until an interrupt occurs that makes a thread ready. Note that interrupts
       are disabled to allow use of WFI for waiting for a thread to arrive.  */

__tx_ts_wait:
#ifdef TX_PORT_USE_BASEPRI
    LDR     r1, =TX_PORT_BASEPRI                    // Mask interrupt priorities =< TX_PORT_BASEPRI
    MSR     BASEPRI, r1
#else
    CPSID   i                                       // Disable interrupts
#endif
    LDR     r1, [r2]                                // Pickup the next thread to execute pointer
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

__tx_ts_ready:
    MOV     r7, #0x08000000                         // Build clear PendSV value
    MOV     r8, #0xE000E000                         // Build base NVIC address
    STR     r7, [r8, #0xD04]                        // Clear any PendSV

__tx_ts_restore:

    /* A thread is ready, make the current thread the new thread
       and enable interrupts.  */

    STR     r1, [r0]                                // Setup the current thread pointer to the new thread
#ifdef TX_PORT_USE_BASEPRI
    MOV     r4, #0                                  // Disable BASEPRI masking (enable interrupts)
    MSR     BASEPRI, r4
#else
    CPSIE   i                                       // Enable interrupts
#endif

    /* Increment the thread run count.  */

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

#if (!defined(TX_SINGLE_MODE_SECURE) && !defined(TX_SINGLE_MODE_NON_SECURE))
    // Restore secure context
    LDR     r0, [r1,#0xC4]                          // Load secure stack index
    CBZ     r0, _skip_secure_restore                // Skip restore if there is no secure context
    PUSH    {r0,r1}                                 // Save r1 (and dummy r0)
    MOV     r0, r1                                  // Move thread ptr to r0
    BL      _tx_thread_secure_stack_context_restore // Restore secure stack
    POP     {r0,r1}                                 // Restore r1 (and dummy r0)
_skip_secure_restore:
#endif

    /* Restore the thread context and PSP.  */
    LDR     r12, [r1, #12]                          // Get stack start
    MSR     PSPLIM, r12                             // Set stack limit
    LDR     r12, [r1, #8]                           // Pickup thread's stack pointer

    MRS     r5, CONTROL                             // Pickup current CONTROL register
    LDR     r4, [r1, #0x98]                         // Pickup current user mode flag
    BIC     r5, r5, #1                              // Clear the UNPRIV bit
    ORR     r4, r4, r5                              // Build new CONTROL register
    MSR     CONTROL, r4                             // Setup new CONTROL register

    LDR     r0, =0xE000ED94                         // Build MPU control reg address
    MOV     r3, #0                                  // Build disable value
    STR     r3, [r0]                                // Disable MPU
    LDR     r0, [r1, #0x90]                         // Pickup the module instance pointer
    CBZ     r0, skip_mpu_setup                      // Is this thread owned by a module? No, skip MPU setup
    LDR     r2, [r0, #0x74]                         // Pickup MPU address of data region
    CBZ     r2, skip_mpu_setup                      // Is protection required for this module? No, skip MPU setup

    LDR     r1, =0xE000ED9C                         // MPU_RBAR register address

    // Use alias registers to quickly load MPU
    LDR     r2, =0xE000ED98                         // Get region register
    STR     r3, [r2]                                // Set region to 0
    ADD     r0, r0, #0x64                           // Build address of MPU register start in thread control block
    LDM     r0!, {r2-r9}                            // Load first four MPU regions
    STM     r1, {r2-r9}                             // Store first four MPU regions
    MOV     r2, #4                                  // Select region 4
    LDR     r3, =0xE000ED98                         // Get region register
    STR     r2, [r3]                                // Set region to 4
    LDM     r0, {r2-r9}                             // Load second four MPU regions
    STM     r1, {r2-r9}                             // Store second four MPU regions
_tx_enable_mpu:
    LDR     r0, =0xE000ED94                         // Build MPU control reg address
    MOV     r1, #5                                  // Build enable value with background region enabled
    STR     r1, [r0]                                // Enable MPU
skip_mpu_setup:
    LDMIA   r12!, {LR}                              // Pickup LR
#ifdef __ARMVFP__
    TST     LR, #0x10                               // Determine if the VFP extended frame is present
    BNE     _skip_vfp_restore                       // If not, skip VFP restore
    VLDMIA  r12!, {s16-s31}                         // Yes, restore additional VFP registers
_skip_vfp_restore:
#endif
    LDMIA   r12!, {r4-r11}                          // Recover thread's registers
    MSR     PSP, r12                                // Setup the thread's stack pointer

    BX      lr                                      // Return to thread!


    /* SVC Handler.  */
    PUBLIC  SVC_Handler
SVC_Handler:
    TST     lr, #0x04                               // Determine return stack from EXC_RETURN bit 2
    ITE     EQ
    MRSEQ   r0, MSP                                 // Get MSP if return stack is MSP
    MRSNE   r0, PSP                                 // Get PSP if return stack is PSP

    LDR     r1, [r0,#24]                            // Load saved PC from stack
    LDRB    r2, [r1,#-2]                            // Load SVC number

#if (!defined(TX_SINGLE_MODE_SECURE) && !defined(TX_SINGLE_MODE_NON_SECURE))
    CMP     r2, #1                                  // Is it a secure stack allocate request?
    BEQ     _tx_svc_secure_alloc                    // Yes, go there

    CMP     r2, #2                                  // Is it a secure stack free request?
    BEQ     _tx_svc_secure_free                     // Yes, go there
#endif  // End of ifndef TX_SINGLE_MODE_SECURE, TX_SINGLE_MODE_NON_SECURE


    CMP     r2, #3                                  // Is it the entry into ThreadX?
    BNE     _tx_thread_user_return                  // No, return to user mode

    /* At this point we have an SVC 3, which means we are entering
       the kernel from a module thread with user mode selected. */

    LDR     r2, =_txm_module_priv-1                 // Load address of where we should have come from
    CMP     r1, r2                                  // Did we come from user_mode_entry?
    IT      NE                                      // If no (not equal), then...
    BXNE    lr                                      // return from where we came.

    LDR     r3, [r0, #20]                           // This is the saved LR
    LDR     r1, =_tx_thread_current_ptr             // Build current thread pointer address
    LDR     r2, [r1]                                // Pickup current thread pointer
    MOV     r1, #0                                  // Build clear value
    STR     r1, [r2, #0x98]                         // Clear the current user mode selection for thread
    STR     r3, [r2, #0xA0]                         // Save the original LR in thread control block

    /* If there is memory protection, use kernel stack */
    LDR     r0, [r2, #0x90]                         // Load the module instance ptr
    LDR     r0, [r0, #0x0C]                         // Load the module property flags
    TST     r0, #2                                  // Check if memory protected
    BEQ     _tx_skip_kernel_stack_enter

    /* Switch to the module thread's kernel stack */
    LDR     r0, [r2, #0xA8]                         // Load the module kernel stack end
    LDR     r1, [r2, #0xA4]                         // Load the module kernel stack start
    MSR     PSPLIM, r1                              // Set stack limit
#ifndef TXM_MODULE_KERNEL_STACK_MAINTENANCE_DISABLE
    LDR     r3, [r2, #0xAC]                         // Load the module kernel stack size
    STR     r1, [r2, #12]                           // Set stack start
    STR     r0, [r2, #16]                           // Set stack end
    STR     r3, [r2, #20]                           // Set stack size
#endif
    MRS     r3, PSP                                 // Pickup thread stack pointer
    TST     lr, #0x10                               // Test for extended module stack
    ITT     EQ
    ORREQ   r3, r3, #1                              // If so, set LSB in thread stack pointer to indicate extended frame
    ORREQ   lr, lr, #0x10                           // Set bit, return with standard frame
    STR     r3, [r2, #0xB0]                         // Save thread stack pointer
    BIC     r3, #1                                  // Clear possibly OR'd bit
    
    /* Build kernel stack by copying thread stack two registers at a time */
    ADD     r3, r3, #32                             // Start at bottom of hardware stack
    LDMDB   r3!, {r1-r2}
    STMDB   r0!, {r1-r2}
    LDMDB   r3!, {r1-r2}
    STMDB   r0!, {r1-r2}
    LDMDB   r3!, {r1-r2}
    STMDB   r0!, {r1-r2}
    LDMDB   r3!, {r1-r2}
    STMDB   r0!, {r1-r2}

    MSR     PSP, r0                                 // Set kernel stack pointer

_tx_skip_kernel_stack_enter:
    MRS     r0, CONTROL                             // Pickup current CONTROL register
    BIC     r0, r0, #1                              // Clear the UNPRIV bit
    MSR     CONTROL, r0                             // Setup new CONTROL register
    BX      lr                                      // Return to thread


_tx_thread_user_return:
    LDR     r2, =_txm_module_user_mode_exit-1       // Load address of where we should have come from
    CMP     r1, r2                                  // Did we come from user_mode_exit?
    IT      NE                                      // If no (not equal), then...
    BXNE    lr                                      // return from where we came

    LDR     r1, =_tx_thread_current_ptr             // Build current thread pointer address
    LDR     r2, [r1]                                // Pickup current thread pointer
    LDR     r1, [r2, #0x9C]                         // Pick up user mode
    STR     r1, [r2, #0x98]                         // Set the current user mode selection for thread

    /* If there is memory protection, use kernel stack */
    LDR     r0, [r2, #0x90]                         // Load the module instance ptr
    LDR     r0, [r0, #0x0C]                         // Load the module property flags
    TST     r0, #2                                  // Check if memory protected
    BEQ     _tx_skip_kernel_stack_exit


    LDR     r0, [r2, #0xB4]                         // Load the module thread stack start
    MSR     PSPLIM, r0                              // Set stack limit
#ifndef TXM_MODULE_KERNEL_STACK_MAINTENANCE_DISABLE
    LDR     r1, [r2, #0xB8]                         // Load the module thread stack end
    LDR     r3, [r2, #0xBC]                         // Load the module thread stack size
    STR     r0, [r2, #12]                           // Set stack start
    STR     r1, [r2, #16]                           // Set stack end
    STR     r3, [r2, #20]                           // Set stack size
#endif

    /* If lazy stacking is pending, check if it can be cleared.
       if(LSPACT && tx_thread_module_stack_start < FPCAR && FPCAR < tx_thread_module_stack_end)
       then clear LSPACT. */
    LDR     r3, =0xE000EF34                         // Address of FPCCR
    LDR     r3, [r3]                                // Load FPCCR
    TST     r3, #1                                  // Check if LSPACT is set
    BEQ     _tx_no_lazy_clear                       // if clear, move on
    LDR     r1, =0xE000EF38                         // Address of FPCAR
    LDR     r1, [r1]                                // Load FPCAR
    LDR     r0, [r2, #0xA4]                         // Load kernel stack start
    CMP     r1, r0                                  // If FPCAR < start, move on
    BLO     _tx_no_lazy_clear
    LDR     r0, [r2, #0xA8]                         // Load kernel stack end
    CMP     r0, r1                                  // If end < FPCAR, move on
    BLO     _tx_no_lazy_clear
    BIC     r3, #1                                  // Clear LSPACT
    LDR     r1, =0xE000EF34                         // Address of FPCCR
    STR     r3, [r1]                                // Save updated FPCCR
_tx_no_lazy_clear:

    LDR     r0, [r2, #0xB0]                         // Load the module thread stack pointer
    MRS     r3, PSP                                 // Pickup kernel stack pointer
    TST     r0, #1                                  // Is module stack extended?
    ITTE    NE                                      // If so...
    BICNE   lr, #0x10                               // Clear bit, return with extended frame
    BICNE   r0, #1                                  // Clear bit that indicates extended module frame
    ORREQ   lr, lr, #0x10                           // Else set bit, return with standard frame

    /* Copy kernel hardware stack to module thread stack. */
    LDM     r3!, {r1-r2}                            // Get r0, r1 from kernel stack
    STM     r0!, {r1-r2}                            // Insert r0, r1 into thread stack
    LDM     r3!, {r1-r2}                            // Get r2, r3 from kernel stack
    STM     r0!, {r1-r2}                            // Insert r2, r3 into thread stack
    LDM     r3!, {r1-r2}                            // Get r12, lr from kernel stack
    STM     r0!, {r1-r2}                            // Insert r12, lr into thread stack
    LDM     r3!, {r1-r2}                            // Get pc, xpsr from kernel stack
    STM     r0!, {r1-r2}                            // Insert pc, xpsr into thread stack
    SUB     r0, r0, #32                             // Subtract 32 to get back to top of stack
    MSR     PSP, r0                                 // Set thread stack pointer

    LDR     r1, =_tx_thread_current_ptr             // Build current thread pointer address
    LDR     r2, [r1]                                // Pickup current thread pointer
    LDR     r1, [r2, #0x9C]                         // Pick up user mode

_tx_skip_kernel_stack_exit:
    MRS     r0, CONTROL                             // Pickup current CONTROL register
    ORR     r0, r0, r1                              // OR in the user mode bit
    MSR     CONTROL, r0                             // Setup new CONTROL register
    BX      lr                                      // Return to thread


#if (!defined(TX_SINGLE_MODE_SECURE) && !defined(TX_SINGLE_MODE_NON_SECURE))
_tx_svc_secure_alloc:
    LDR     r2, =_tx_alloc_return-1                 // Load address of where we should have come from
    CMP     r1, r2                                  // Did we come from _tx_thread_secure_stack_allocate?
    IT      NE                                      // If no (not equal), then...
    BXNE    lr                                      // return from where we came.
    
    PUSH    {r0, lr}                                // Save SP and EXC_RETURN
    LDM     r0, {r0-r3}                             // Load function parameters from stack
    BL      _tx_thread_secure_mode_stack_allocate
    POP     {r12, lr}                               // Restore SP and EXC_RETURN
    STR     r0, [r12]                               // Store function return value
    BX      lr
    
_tx_svc_secure_free:
    LDR     r2, =_tx_free_return-1                  // Load address of where we should have come from
    CMP     r1, r2                                  // Did we come from _tx_thread_secure_stack_free?
    IT      NE                                      // If no (not equal), then...
    BXNE    lr                                      // return from where we came.
    
    PUSH    {r0, lr}                                // Save SP and EXC_RETURN
    LDM     r0, {r0-r3}                             // Load function parameters from stack
    BL      _tx_thread_secure_mode_stack_free
    POP     {r12, lr}                               // Restore SP and EXC_RETURN
    STR     r0, [r12]                               // Store function return value
    BX      lr
#endif  // End of ifndef TX_SINGLE_MODE_SECURE, TX_SINGLE_MODE_NON_SECURE



    /* Kernel entry function from user mode.  */

    EXTERN  _txm_module_manager_kernel_dispatch
    SECTION `.text`:CODE:NOROOT(5)
    THUMB
    ALIGNROM   5
// VOID   _txm_module_manager_user_mode_entry(VOID)
// {
    PUBLIC  _txm_module_manager_user_mode_entry
_txm_module_manager_user_mode_entry:
    SVC     3                                       // Enter kernel
_txm_module_priv:
    /* At this point, we are out of user mode. The original LR has been saved in the
       thread control block. Simply call the kernel dispatch function. */
    BL      _txm_module_manager_kernel_dispatch

    /* Pickup the original LR value while still in privileged mode */
    LDR     r2, =_tx_thread_current_ptr             // Build current thread pointer address
    LDR     r3, [r2]                                // Pickup current thread pointer
    LDR     lr, [r3, #0xA0]                         // Pickup saved LR from original call

    SVC     4                                       // Exit kernel and return to user mode
_txm_module_user_mode_exit:
    BX      lr                                      // Return to the caller
    NOP
    NOP
    NOP
    NOP
// }


    PUBLIC  _tx_vfp_access
_tx_vfp_access:
    VMOV.F32 s0, s0                                 // Simply access the VFP
    BX       lr                                     // Return to caller
    END
