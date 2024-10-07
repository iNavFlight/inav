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


#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
    .global _tx_execution_thread_enter
    .global _tx_execution_thread_exit
#endif
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_schedule                               Cortex-M0+/AC6    */
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
/*  01-31-2022      Scott Larson            Initial Version 6.1.10        */
/*  04-25-2022      Scott Larson            Optimized MPU configuration,  */
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
    MOVS    r0, #0                                  // Build value for TX_FALSE
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


    /* Memory Exception Handler.  */
    .global  HardFaultException
    .thumb_func
HardFaultException:
    .global  MemManage_Handler
    .global  BusFault_Handler
    .global  UsageFault_Handler
    .thumb_func
MemManage_Handler:
    .thumb_func
BusFault_Handler:
    .thumb_func
UsageFault_Handler:

    CPSID   i                                       // Disable interrupts

    /* Now pickup and store all the fault related information.  */

    LDR     r2,=_txm_module_manager_memory_fault_info   // Pickup fault info struct
    LDR     r0, =_tx_thread_current_ptr             // Build current thread pointer address
    LDR     r1, [r0]                                // Pickup the current thread pointer
    STR     r1, [r2, #0]                            // Save current thread pointer in fault info structure
    MRS     r0, CONTROL                             // Pickup current CONTROL register
    STR     r0, [r2, #24]                           // Save CONTROL
    MRS     r1, PSP                                 // Pickup thread stack pointer
    STR     r1, [r2, #28]                           // Save thread stack pointer
    LDR     r0, [r1]                                // Pickup saved r0 (as r0-r3, r12, lr, pc, xpsr are automatically saved to stack)
    STR     r0, [r2, #32]                           // Save r0
    LDR     r0, [r1, #4]                            // Pickup saved r1
    STR     r0, [r2, #36]                           // Save r1
    LDR     r0, [r1, #8]                            // Pickup saved r2
    STR     r0, [r2, #40]                           // Save r2
    STR     r3, [r2, #44]                           // Save r3
    STR     r4, [r2, #48]                           // Save r4
    STR     r5, [r2, #52]                           // Save r5
    STR     r6, [r2, #56]                           // Save r6
    STR     r7, [r2, #60]                           // Save r7
    MOV     r0, r8                                  // Pickup current r8 register (high register)
    STR     r0, [r2, #64]                           // Save r8
    MOV     r0, r9                                  // Pickup current r9 register
    STR     r0, [r2, #68]                           // Save r9
    MOV     r0, r10                                 // Pickup current r10 register
    STR     r0, [r2, #72]                           // Save r10
    MOV     r0, r11                                 // Pickup current r11 register
    STR     r0, [r2, #76]                           // Save r11
    LDR     r0, [r1, #16]                           // Pickup saved r12
    STR     r0, [r2, #80]                           // Save r12
    LDR     r0, [r1, #20]                           // Pickup saved lr
    STR     r0, [r2, #84]                           // Save lr
    LDR     r0, [r1, #24]                           // Pickup instruction address at point of fault
    STR     r0, [r2, #4]                            // Save point of fault
    LDR     r0, [r1, #28]                           // Pickup xPSR
    STR     r0, [r2, #88]                           // Save xPSR

    MRS     r0, CONTROL                             // Pickup current CONTROL register
    MOVS    r1, #1
    BICS    r0, r0, r1                              // Clear the UNPRIV bit
    MSR     CONTROL, r0                             // Setup new CONTROL register

    BL      _txm_module_manager_memory_fault_handler    // Call memory manager fault handler

#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
    /* Call the thread exit function to indicate the thread is no longer executing.  */
    CPSID   i                                       // Disable interrupts
    BL      _tx_execution_thread_exit               // Call the thread exit function
    CPSIE   i                                       // Enable interrupts
#endif

    MOVS    r1, #0                                  // Build NULL value
    LDR     r0, =_tx_thread_current_ptr             // Pickup address of current thread pointer
    STR     r1, [r0]                                // Clear current thread pointer

    // Return from MemManage_Handler exception
    LDR     r0, =0xE000ED04                         // Load ICSR
    LDR     r1, =0x10000000                         // Set PENDSVSET bit
    STR     r1, [r0]                                // Store ICSR
    DSB                                             // Wait for memory access to complete
    CPSIE   i                                       // Enable interrupts
    LDR     r0, =0xFFFFFFFD                         // Exception return
    MOV     lr, r0                                  // Move exception return to lr
    BX      lr                                      // Return from exception


    /* Generic context switching PendSV handler.  */

    .section .text
    .balign 4
    .syntax unified
    .eabi_attribute Tag_ABI_align_preserved, 1
    .global  PendSV_Handler
    .global  __tx_PendSVHandler
    .syntax unified
    .thumb_func
PendSV_Handler:
    .thumb_func
__tx_PendSVHandler:

    /* Get current thread value and new thread pointer.  */

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
    MOVS    r3, #0                                  // Build NULL value
    LDR     r1, [r0]                                // Pickup current thread pointer

    /* Determine if there is a current thread to finish preserving.  */
    CMP     r1, #0
    BEQ     __tx_ts_new                             // If NULL, skip preservation

    /* Recover PSP and preserve current thread context.  */

    STR     r3, [r0]                                // Set _tx_thread_current_ptr to NULL
    MRS     r3, PSP                                 // Pickup PSP pointer (thread's stack pointer)
    SUBS    r3, r3, #16                             // Allocate stack space
    STM     r3!, {r4-r7}                            // Save its remaining registers
    MOV     r4, r8                                  // Pick up r8
    MOV     r5, r9                                  // Pick up r9
    MOV     r6, r10                                 // Pick up r10
    MOV     r7, r11                                 // Pick up r11
    SUBS    r3, r3, #32                             // Allocate stack space for r8-r11
    STM     r3!, {r4-r7}                            // Save r8-r11
    SUBS    r3, r3, #20                             // Allocate stack space for lr
    MOV     r5, lr                                  // Move lr into r5
    STR     r5, [r3]                                // Save lr
    STR     r3, [r1, #8]                            // Save the thread stack pointer

    /* Determine if time-slice is active. If it isn't, skip time handling processing.  */

    LDR     r4, =_tx_timer_time_slice               // Build address of time-slice variable
    LDR     r5, [r4]                                // Pickup current time-slice
    CMP     r5, #0
    BEQ     __tx_ts_new                            // If not active, skip processing

    /* Time-slice is active, save the current thread's time-slice and clear the global time-slice variable.  */

    STR     r5, [r1, #24]                           // Save current time-slice

    /* Clear the global time-slice.  */
    MOVS    r3, #0                                  // Build NULL value
    STR     r3, [r4]                                // Clear time-slice

    /* Executing thread is now completely preserved!!!  */

__tx_ts_new:

    /* Now we are looking for a new thread to execute!  */

    CPSID   i                                       // Disable interrupts
    LDR     r1, [r2]                                // Is there another thread ready to execute?
    CMP     r1, #0
    BNE     __tx_ts_restore                        // Yes, schedule it

    /* The following is the idle wait processing... in this case, no threads are ready for execution and the
       system will simply be idle until an interrupt occurs that makes a thread ready. Note that interrupts
       are disabled to allow use of WFI for waiting for a thread to arrive.  */

__tx_ts_wait:
    CPSID   i                                       // Disable interrupts
    LDR     r1, [r2]                                // Pickup the next thread to execute pointer
    CMP     r1, #0
    BNE     __tx_ts_ready                           // If non-NULL, a new thread is ready!

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
    LDR     r6, =0xE000ED04                         // Build base NVIC address
    STR     r7, [r6]                                // Clear any PendSV

__tx_ts_restore:

    /* A thread is ready, make the current thread the new thread
       and enable interrupts.  */

    STR     r1, [r0]                                // Setup the current thread pointer to the new thread
    CPSIE   i                                       // Enable interrupts

    /* Increment the thread run count.  */

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

    /* Restore the thread context and PSP.  */

    LDR     r3, [r1, #8]                            // Pickup thread's stack pointer

    // Set up CONTROL register based on user mode flag (privileged/unprivileged mode)
    MRS     r5, CONTROL                             // Pickup current CONTROL register
    MOVS    r6, #0x98
    LDR     r4, [r1, r6]                            // Pickup current user mode flag
    MOVS    r6, #1
    BICS    r5, r5, r6                              // Clear the UNPRIV bit
    ORRS    r4, r4, r5                              // Build new CONTROL register
    MSR     CONTROL, r4                             // Setup new CONTROL register

    // Determine if MPU needs to be configured
    LDR     r0, =0xE000ED94                         // Build MPU control reg address
    MOVS    r6, #0                                  // Build disable value
    STR     r6, [r0]                                // Disable MPU
    MOVS    r6, #0x90
    LDR     r0, [r1, r6]                            // Pickup the module instance pointer
    CMP     r0, #0
    BEQ     skip_mpu_setup                          // Is this thread owned by a module? No, skip MPU setup
    MOV     r8, r1                                  // Copy thread ptr
    MOVS	r6, #0x8C
    LDR     r2, [r0, r6]                            // Pickup MPU region 5 address
    CMP     r2, #0
    BEQ     skip_mpu_setup                          // Is protection required for this module? No, skip MPU setup

    // Initialize loop to configure MPU registers
    MOVS    r3, #0x64                               // Index of MPU register settings in thread control block
    ADD     r0, r0, r3                              // Build address of MPU register start in thread control block
    MOVS    r5, #0                                  // Select region 0
    LDR     r4, =0xE000ED98                         // Region register address
    // Loop to load MPU registers
_tx_mpu_loop:
    LDR     r1, =0xE000ED9C                         // Build address of MPU base register
    STR     r5, [r4]                                // Set region
    LDM     r0!, {r2-r3}                            // Get MPU settings from the module
    STM     r1!, {r2-r3}                            // Set MPU registers for region
    ADDS    r5, r5, #1                              // Increment to next region
    CMP     r5, #8                                  // Check if all regions have been set
    BNE     _tx_mpu_loop
_tx_enable_mpu:
    LDR     r0, =0xE000ED94                         // Build MPU control reg address
    MOVS    r1, #5                                  // Build enable value with background region enabled
    STR     r1, [r0]                                // Enable MPU
    MOV     r1, r8                                  // Get copied thread ptr

skip_mpu_setup:

    // Restore the thread context and PSP
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


    /* SVC Handler.  */
    .section .text
    .balign 4
    .syntax unified
    .eabi_attribute Tag_ABI_align_preserved, 1
    .global  SVC_Handler
    .thumb_func
.type SVC_Handler, function
SVC_Handler:
    .global  __tx_SVCallHandler
    .thumb_func
__tx_SVCallHandler:

    MRS     r0, PSP                                 // Pickup the PSP stack
    LDR     r1, [r0, #24]                           // Pickup the point of interrupt
    MOV     r3, r1
    SUBS    r3, r3, #2
    LDRB    r2, [r3]                                // Pickup the SVC parameter

    /* Determine which SVC trap we are processing */

    CMP     r2, #1                                  // Is it the entry into ThreadX?
    BNE     _tx_thread_user_return                  // No, return to user mode

    /* At this point we have an SVC 1, which means we are entering
       the kernel from a module thread with user mode selected. */

    LDR     r2, =_txm_module_priv                   // Load address of where we should have come from
    CMP     r1, r2                                  // Did we come from user_mode_entry?
    BEQ     _tx_entry_continue                      // If no (not equal), then...
    BX      lr                                      // return from where we came.
_tx_entry_continue:
    LDR     r3, [r0, #20]                           // This is the saved LR
    LDR     r1, =_tx_thread_current_ptr             // Build current thread pointer address
    LDR     r2, [r1]                                // Pickup current thread pointer
    MOVS    r1, #0                                  // Build clear value
    MOVS    r0, #0x98                               // Index of current user mode
    STR     r1, [r2, r0]                            // Clear the current user mode selection for thread
    MOVS    r0, #0xA0                               // Index of saved LR
    STR     r3, [r2, r0]                            // Save the original LR in thread control block

    /* If there is memory protection, use kernel stack */
    MOVS    r0, #0x90                               // Index of module instance ptr
    LDR     r0, [r2, r0]                            // Load the module instance ptr
    LDR     r0, [r0, #0x0C]                         // Load the module property flags
    MOVS    r1, #2                                  // MPU protection flag
    TST     r0, r1                                  // Check if memory protected
    BEQ     _tx_skip_kernel_stack_enter

    /* Switch to the module thread's kernel stack */
    MOVS    r0, #0xA8                               // Index of module kernel stack end
    LDR     r0, [r2, r0]                            // Load the module kernel stack end
    MOVS    r1, #0xA4                               // Index of module kernel stack start
    LDR     r1, [r2, r1]                            // Load the module kernel stack start

#ifndef TXM_MODULE_KERNEL_STACK_MAINTENANCE_DISABLE
    MOVS    r3, #0xAC                               // Index of module kernel stack size
    LDR     r3, [r2, r3]                            // Load the module kernel stack size
    STR     r1, [r2, #12]                           // Set stack start
    STR     r0, [r2, #16]                           // Set stack end
    STR     r3, [r2, #20]                           // Set stack size
#endif

    MRS     r3, PSP                                 // Pickup thread stack pointer
    MOVS    r1, #0xB0                               // Index of module stack pointer
    STR     r3, [r2, r1]                            // Save thread stack pointer

    /* Build kernel stack by copying thread stack two registers at a time */
    SUBS    r0, r0, #32                             // Start at top of hardware stack
    LDMIA   r3!, {r1,r2}                            // Get r0, r1 from thread stack
    STMIA   r0!, {r1,r2}                            // Insert r0, r1 into kernel stack
    LDMIA   r3!, {r1,r2}                            // Get r2, r3 from thread stack
    STMIA   r0!, {r1,r2}                            // Insert r2, r3 into kernel stack
    LDMIA   r3!, {r1,r2}                            // Get r12, lr from thread stack
    STMIA   r0!, {r1,r2}                            // Insert r12, lr into kernel stack
    LDMIA   r3!, {r1,r2}                            // Get pc, xpsr from thread stack
    STMIA   r0!, {r1,r2}                            // Insert pc, xpsr into kernel stack
    SUBS    r0, r0, #32                             // Go back to top of stack

    MSR     PSP, r0                                 // Set kernel stack pointer

_tx_skip_kernel_stack_enter:
    MRS     r0, CONTROL                             // Pickup current CONTROL register
    MOVS    r1, #1
    BICS    r0, r0, r1                              // Clear the UNPRIV bit
    MSR     CONTROL, r0                             // Setup new CONTROL register
    BX      lr                                      // Return to thread

_tx_thread_user_return:
    LDR     r2, =_txm_module_user_mode_exit         // Load address of where we should have come from
    CMP     r1, r2                                  // Did we come from user_mode_exit?
    BEQ     _tx_exit_continue                       // If no (not equal), then...
    BX      lr                                      // return from where we came.
_tx_exit_continue:
    LDR     r1, =_tx_thread_current_ptr             // Build current thread pointer address
    LDR     r2, [r1]                                // Pickup current thread pointer
    MOVS    r1, #0x9C                               // Index of user mode
    MOVS    r3, #0x98                               // Index of current user mode
    LDR     r1, [r2, r1]                            // Pick up user mode
    STR     r1, [r2, r3]                            // Set the current user mode selection for thread

    /* If there is memory protection, use kernel stack */
    MOVS    r0, #0x90                               // Index of module instance ptr
    LDR     r0, [r2, r0]                            // Load the module instance ptr
    LDR     r0, [r0, #0x0C]                         // Load the module property flags
    MOVS    r1, #2                                  // MPU protection flag
    TST     r0, r1                                  // Check if memory protected
    BEQ     _tx_skip_kernel_stack_exit

#ifndef TXM_MODULE_KERNEL_STACK_MAINTENANCE_DISABLE
    MOVS    r0, #0xB4                               // Index of module thread stack start
    LDR     r0, [r2, r0]                            // Load the module thread stack start
    MOVS    r1, #0xB8                               // Index of module thread stack end
    LDR     r1, [r2, r1]                            // Load the module thread stack end
    MOVS    r3, #0xBC                               // Index of module thread stack size
    LDR     r3, [r2, r3]                            // Load the module thread stack size
    STR     r0, [r2, #12]                           // Set stack start
    STR     r1, [r2, #16]                           // Set stack end
    STR     r3, [r2, #20]                           // Set stack size
#endif
    MOVS    r1, #0xB0                               // Index of module thread stack pointer
    LDR     r0, [r2, r1]                            // Load the module thread stack pointer
    MRS     r3, PSP                                 // Pickup kernel stack pointer

    /* Copy kernel hardware stack to module thread stack. */
    LDM     r3!, {r1-r2}                            // Get r0, r1 from kernel stack
    STM     r0!, {r1-r2}                            // Insert r0, r1 into thread stack
    LDM     r3!, {r1-r2}                            // Get r2, r3 from kernel stack
    STM     r0!, {r1-r2}                            // Insert r2, r3 into thread stack
    LDM     r3!, {r1-r2}                            // Get r12, lr from kernel stack
    STM     r0!, {r1-r2}                            // Insert r12, lr into thread stack
    LDM     r3!, {r1-r2}                            // Get pc, xpsr from kernel stack
    STM     r0!, {r1-r2}                            // Insert pc, xpsr into thread stack
    SUBS    r0, r0, #32                             // Subtract 32 to get back to top of stack
    MSR     PSP, r0                                 // Set thread stack pointer

    LDR     r1, =_tx_thread_current_ptr             // Build current thread pointer address
    LDR     r2, [r1]                                // Pickup current thread pointer
    MOVS    r1, #0x9C                               // Index of user mode
    LDR     r1, [r2, r1]                            // Pick up user mode

_tx_skip_kernel_stack_exit:
    MRS     r0, CONTROL                             // Pickup current CONTROL register
    ORRS    r0, r0, r1                              // OR in the user mode bit
    MSR     CONTROL, r0                             // Setup new CONTROL register
    BX      lr                                      // Return to thread

    /* Kernel entry function from user mode.  */

    .global  _txm_module_manager_kernel_dispatch
    .align  8
    .syntax unified
// VOID   _txm_module_manager_user_mode_entry(VOID)
// {
    .global  _txm_module_manager_user_mode_entry
    .thumb_func
_txm_module_manager_user_mode_entry:
    SVC     1                                       // Enter kernel
_txm_module_priv:
    /* At this point, we are out of user mode. The original LR has been saved in the
       thread control block. Simply call the kernel dispatch function. */
    BL      _txm_module_manager_kernel_dispatch

    /* Pickup the original LR value while still in privileged mode */
    LDR     r2, =_tx_thread_current_ptr             // Build current thread pointer address
    LDR     r3, [r2]                                // Pickup current thread pointer
    MOVS    r2, #0xA0
    LDR     r1, [r3, r2]                         // Pickup saved LR from original call
    MOV     lr, r1

    SVC     2                                       // Exit kernel and return to user mode
_txm_module_user_mode_exit:
    BX      lr                                      // Return to the caller
    NOP
    NOP
    NOP
    NOP
// }

