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

    .global _tx_thread_current_ptr
    .global _tx_thread_execute_ptr
    .global _tx_timer_time_slice
    .global _tx_thread_preempt_disable
    .global _txm_module_manager_memory_fault_handler
    .global _txm_module_manager_memory_fault_info
#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
    .global _tx_execution_thread_enter
    .global _tx_execution_thread_exit
#endif
#ifdef TX_LOW_POWER
    .global tx_low_power_enter
    .global tx_low_power_exit
#endif
    .text
    .align 4
    .syntax unified
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_schedule                               Cortex-M4/AC6     */
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
/*  10-15-2021      Scott Larson            Initial Version 6.1.9         */
/*  04-25-2022      Scott Larson            Optimized MPU configuration,  */
/*                                            added BASEPRI support,      */
/*                                            resulting in version 6.1.11 */
/*  07-29-2022      Scott Larson            Removed the code path to skip */
/*                                            MPU reloading, optional     */
/*                                            default MPU settings,       */
/*                                            resulting in version 6.1.12 */
/*  10-31-2022      Scott Larson            Added low power support,      */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
// VOID   _tx_thread_schedule(VOID)
// {
    .global  _tx_thread_schedule
    .thumb_func
_tx_thread_schedule:

    /* This function should only ever be called on Cortex-M
       from the first schedule request. Subsequent scheduling occurs
       from the PendSV handling routine below. */

    /* Clear the preempt-disable flag to enable rescheduling after initialization on Cortex-M targets.  */

    MOV     r0, #0                                  // Build value for TX_FALSE
    LDR     r2, =_tx_thread_preempt_disable         // Build address of preempt disable flag
    STR     r0, [r2, #0]                            // Clear preempt disable flag

#ifdef __ARM_FP
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
    .global  MemManage_Handler
    .global  BusFault_Handler
    .global  UsageFault_Handler
    .thumb_func
MemManage_Handler:
    .thumb_func
BusFault_Handler:
    .thumb_func
UsageFault_Handler:

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

#ifdef __ARM_FP
    LDR     r0, =0xE000EF34                         // Cleanup FPU context: Load FPCCR address
    LDR     r1, [r0]                                // Load FPCCR
    BIC     r1, r1, #1                              // Clear the lazy preservation active bit
    STR     r1, [r0]                                // Save FPCCR
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
    MOV     lr, #0xFFFFFFFD                         // Load exception return code
    BX      lr                                      // Return from exception


    /* Generic context PendSV handler.  */

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
#ifdef __ARM_FP
    TST     LR, #0x10                               // Determine if the VFP extended frame is present
    BNE     _skip_vfp_save
    VSTMDB  r12!,{s16-s31}                          // Yes, save additional VFP registers
_skip_vfp_save:
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

    /* Restore the thread context and PSP.  */

    LDR     r12, [r1, #8]                           // Pickup thread's stack pointer

    MRS     r5, CONTROL                             // Pickup current CONTROL register
    LDR     r4, [r1, #0x98]                         // Pickup current user mode flag
    BIC     r5, r5, #1                              // Clear the UNPRIV bit
    ORR     r4, r4, r5                              // Build new CONTROL register
    MSR     CONTROL, r4                             // Setup new CONTROL register

    LDR     r0, =0xE000ED94                         // Build MPU control reg address
    MOV     r3, #0                                  // Build disable value
    CPSID   i                                       // Disable interrupts
    STR     r3, [r0]                                // Disable MPU
    LDR     r0, [r1, #0x90]                         // Pickup the module instance pointer
#ifdef TXM_MODULE_MPU_DEFAULT
    CBZ     r0, default_mpu                         // Is this thread owned by a module? No, default MPU setup
#else
    CBZ     r0, skip_mpu_setup                      // Is this thread owned by a module? No, skip MPU setup
#endif

    LDR     r2, [r0, #0x8C]                         // Pickup MPU region 5 address
#ifdef TXM_MODULE_MPU_DEFAULT
    CBZ     r2, default_mpu                         // Is protection required for this module? No, default MPU setup
#else
    CBZ     r2, skip_mpu_setup                      // Is protection required for this module? No, skip MPU setup
#endif
    LDR     r1, =0xE000ED9C                         // MPU_RBAR register address

    // Use alias registers to quickly load MPU
    ADD     r0, r0, #100                            // Build address of MPU register start in thread control block

#ifdef TXM_MODULE_MPU_DEFAULT
    B       config_mpu                              // configure MPU for module
default_mpu:
    LDR     r0, =txm_module_default_mpu_registers   // default MPU configuration
#endif

config_mpu:
    LDM     r0!,{r2-r9}                             // Load MPU regions 0-3
    STM     r1,{r2-r9}                              // Store MPU regions 0-3
    LDM     r0!,{r2-r9}                             // Load MPU regions 4-7
    STM     r1,{r2-r9}                              // Store MPU regions 4-7
#ifdef TXM_MODULE_MANAGER_16_MPU
    LDM     r0!,{r2-r9}                             // Load MPU regions 8-11
    STM     r1,{r2-r9}                              // Store MPU regions 8-11
    // Regions 12-15 are reserved for the user to define.
    LDM     r0,{r2-r9}                              // Load MPU regions 12-15
    STM     r1,{r2-r9}                              // Store MPU regions 12-15
#endif

_tx_enable_mpu:
    LDR     r0, =0xE000ED94                         // Build MPU control reg address
    MOV     r1, #5                                  // Build enable value with background region enabled
    STR     r1, [r0]                                // Enable MPU
skip_mpu_setup:
    CPSIE   i                                       // Enable interrupts
    LDMIA   r12!, {LR}                              // Pickup LR
#ifdef __ARM_FP
    TST     LR, #0x10                               // Determine if the VFP extended frame is present
    BNE     _skip_vfp_restore                       // If not, skip VFP restore
    VLDMIA  r12!, {s16-s31}                         // Yes, restore additional VFP registers
_skip_vfp_restore:
#endif
    LDMIA   r12!, {r4-r11}                          // Recover thread's registers
    MSR     PSP, r12                                // Setup the thread's stack pointer

    /* Return to thread.  */

    BX      lr                                      // Return to thread!


    /* SVC Handler.  */

    .global  SVC_Handler
    .thumb_func
SVC_Handler:
    .global  __tx_SVCallHandler
    .thumb_func
__tx_SVCallHandler:

    MRS     r0, PSP                                 // Pickup the PSP stack
    LDR     r1, [r0, #24]                           // Pickup the point of interrupt
    LDRB    r2, [r1, #-2]                           // Pickup the SVC parameter

    /* Determine which SVC trap we are processing */

    CMP     r2, #1                                  // Is it the entry into ThreadX?
    BNE     _tx_thread_user_return                  // No, return to user mode

    /* At this point we have an SVC 1, which means we are entering
       the kernel from a module thread with user mode selected. */

    LDR     r2, =_txm_module_priv                   // Load address of where we should have come from
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
#ifndef TXM_MODULE_KERNEL_STACK_MAINTENANCE_DISABLE
    LDR     r1, [r2, #0xA4]                         // Load the module kernel stack start
    LDR     r3, [r2, #0xAC]                         // Load the module kernel stack size
    STR     r1, [r2, #12]                           // Set stack start
    STR     r0, [r2, #16]                           // Set stack end
    STR     r3, [r2, #20]                           // Set stack size
#endif

    MRS     r3, PSP                                 // Pickup thread stack pointer
#ifdef __ARM_FP
    TST     lr, #0x10                               // Test for extended module stack
    ITT     EQ
    ORREQ   r3, r3, #1                              // If so, set LSB in thread stack pointer to indicate extended frame
    ORREQ   lr, lr, #0x10                           // Set bit, return with standard frame
#endif
    STR     r3, [r2, #0xB0]                         // Save thread stack pointer
#ifdef __ARM_FP
    BIC     r3, #1                                  // Clear possibly OR'd bit
#endif

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
    LDR     r2, =_txm_module_user_mode_exit         // Load address of where we should have come from
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

#ifndef TXM_MODULE_KERNEL_STACK_MAINTENANCE_DISABLE
    LDR     r0, [r2, #0xB4]                         // Load the module thread stack start
    LDR     r1, [r2, #0xB8]                         // Load the module thread stack end
    LDR     r3, [r2, #0xBC]                         // Load the module thread stack size
    STR     r0, [r2, #12]                           // Set stack start
    STR     r1, [r2, #16]                           // Set stack end
    STR     r3, [r2, #20]                           // Set stack size
#endif

#ifdef __ARM_FP
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
#endif

    LDR     r0, [r2, #0xB0]                         // Load the module thread stack pointer
    MRS     r3, PSP                                 // Pickup kernel stack pointer
#ifdef __ARM_FP
    TST     r0, #1                                  // Is module stack extended?
    ITTE    NE                                      // If so...
    BICNE   lr, #0x10                               // Clear bit, return with extended frame
    BICNE   r0, #1                                  // Clear bit that indicates extended module frame
    ORREQ   lr, lr, #0x10                           // Else set bit, return with standard frame
#endif

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


    /* Kernel entry function from user mode.  */

    .global  _txm_module_manager_kernel_dispatch
    .align  5
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
    LDR     lr, [r3, #0xA0]                         // Pickup saved LR from original call

    SVC     2                                       // Exit kernel and return to user mode
_txm_module_user_mode_exit:
    BX      lr                                      // Return to the caller
    NOP
    NOP
    NOP
    NOP
// }

#ifdef __ARM_FP

    .global  tx_thread_fpu_enable
    .thumb_func
tx_thread_fpu_enable:
    .global  tx_thread_fpu_disable
    .thumb_func
tx_thread_fpu_disable:

    /* Automatic VPF logic is supported, this function is present only for
       backward compatibility purposes and therefore simply returns.  */

    BX      LR                                      // Return to caller

#endif
