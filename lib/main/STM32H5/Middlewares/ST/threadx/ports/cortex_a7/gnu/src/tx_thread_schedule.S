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

    .global     _tx_thread_execute_ptr
    .global     _tx_thread_current_ptr
    .global     _tx_timer_time_slice


/* Define the 16-bit Thumb mode veneer for _tx_thread_schedule for
   applications calling this function from to 16-bit Thumb mode.  */

    .text
    .align  2
    .global $_tx_thread_schedule
    .type   $_tx_thread_schedule,function
$_tx_thread_schedule:
         .thumb
     BX        pc                               // Switch to 32-bit mode
     NOP                                        //
    .arm
     STMFD     sp!, {lr}                        // Save return address
     BL        _tx_thread_schedule              // Call _tx_thread_schedule function
     LDMFD     sp!, {lr}                        // Recover saved return address
     BX        lr                               // Return to 16-bit caller


    .text
    .align 2
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_schedule                                  ARMv7-A        */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
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
/*    _tx_thread_context_restore           Restore thread's context       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*  10-15-2021     William E. Lamie         Modified comment(s), added    */
/*                                            execution profile support,  */
/*                                            resulting in version 6.1.9  */
/*  04-25-2022     Zhen Kong                Updated comments,             */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
    .global _tx_thread_schedule
    .type  _tx_thread_schedule,function
_tx_thread_schedule:

    /* Enable interrupts.  */

#ifdef TX_ENABLE_FIQ_SUPPORT
    CPSIE   if                              // Enable IRQ and FIQ interrupts
#else
    CPSIE   i                               // Enable IRQ interrupts
#endif

    /* Wait for a thread to execute.  */
    LDR     r1, =_tx_thread_execute_ptr     // Address of thread execute ptr

__tx_thread_schedule_loop:

    LDR     r0, [r1]                        // Pickup next thread to execute
    CMP     r0, #0                          // Is it NULL?
    BEQ     __tx_thread_schedule_loop       // If so, keep looking for a thread
    /* Yes! We have a thread to execute.  Lockout interrupts and
       transfer control to it.  */

#ifdef TX_ENABLE_FIQ_SUPPORT
    CPSID   if                              // Disable IRQ and FIQ interrupts
#else
    CPSID   i                               // Disable IRQ interrupts
#endif

    /* Setup the current thread pointer.  */

    LDR     r1, =_tx_thread_current_ptr     // Pickup address of current thread
    STR     r0, [r1]                        // Setup current thread pointer

    /* Increment the run count for this thread.  */

    LDR     r2, [r0, #4]                    // Pickup run counter
    LDR     r3, [r0, #24]                   // Pickup time-slice for this thread
    ADD     r2, r2, #1                      // Increment thread run-counter
    STR     r2, [r0, #4]                    // Store the new run counter

    /* Setup time-slice, if present.  */

    LDR     r2, =_tx_timer_time_slice       // Pickup address of time-slice
                                            //   variable
    LDR     sp, [r0, #8]                    // Switch stack pointers
    STR     r3, [r2]                        // Setup time-slice

#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))

    /* Call the thread entry function to indicate the thread is executing.  */

    MOV     r5, r0                          // Save r0
    BL      _tx_execution_thread_enter      // Call the thread execution enter function
    MOV     r0, r5                          // Restore r0
#endif

    /* Determine if an interrupt frame or a synchronous task suspension frame
       is present.  */

    LDMIA   sp!, {r4, r5}                   // Pickup the stack type and saved CPSR
    CMP     r4, #0                          // Check for synchronous context switch
    BEQ     _tx_solicited_return
    MSR     SPSR_cxsf, r5                   // Setup SPSR for return
#ifdef TX_ENABLE_VFP_SUPPORT
    LDR     r1, [r0, #144]                  // Pickup the VFP enabled flag
    CMP     r1, #0                          // Is the VFP enabled?
    BEQ     _tx_skip_interrupt_vfp_restore  // No, skip VFP interrupt restore
    VLDMIA  sp!, {D0-D15}                   // Recover D0-D15
    VLDMIA  sp!, {D16-D31}                  // Recover D16-D31
    LDR     r4, [sp], #4                    // Pickup FPSCR
    VMSR    FPSCR, r4                       // Restore FPSCR
_tx_skip_interrupt_vfp_restore:
#endif
    LDMIA   sp!, {r0-r12, lr, pc}^          // Return to point of thread interrupt

_tx_solicited_return:

#ifdef TX_ENABLE_VFP_SUPPORT
    LDR     r1, [r0, #144]                  // Pickup the VFP enabled flag
    CMP     r1, #0                          // Is the VFP enabled?
    BEQ     _tx_skip_solicited_vfp_restore  // No, skip VFP solicited restore
    VLDMIA  sp!, {D8-D15}                   // Recover D8-D15
    VLDMIA  sp!, {D16-D31}                  // Recover D16-D31
    LDR     r4, [sp], #4                    // Pickup FPSCR
    VMSR    FPSCR, r4                       // Restore FPSCR
_tx_skip_solicited_vfp_restore:
#endif
    MSR     CPSR_cxsf, r5                   // Recover CPSR
    LDMIA   sp!, {r4-r11, lr}               // Return to thread synchronously
#ifdef __THUMB_INTERWORK
    BX      lr                              // Return to caller
#else
    MOV     pc, lr                          // Return to caller
#endif

#ifdef TX_ENABLE_VFP_SUPPORT

    .global tx_thread_vfp_enable
    .type  tx_thread_vfp_enable,function
tx_thread_vfp_enable:
    MRS     r2, CPSR                        // Pickup the CPSR
#ifdef TX_ENABLE_FIQ_SUPPORT
    CPSID   if                              // Enable IRQ and FIQ interrupts
#else
    CPSID   i                               // Enable IRQ interrupts
#endif
    LDR     r0, =_tx_thread_current_ptr     // Build current thread pointer address
    LDR     r1, [r0]                        // Pickup current thread pointer
    CMP     r1, #0                          // Check for NULL thread pointer
    BEQ     __tx_no_thread_to_enable        // If NULL, skip VFP enable
    MOV     r0, #1                          // Build enable value
    STR     r0, [r1, #144]                  // Set the VFP enable flag (tx_thread_vfp_enable field in TX_THREAD)
__tx_no_thread_to_enable:
    MSR     CPSR_cxsf, r2                   // Recover CPSR
    BX      LR                              // Return to caller

    .global tx_thread_vfp_disable
    .type  tx_thread_vfp_disable,function
tx_thread_vfp_disable:
    MRS     r2, CPSR                        // Pickup the CPSR
#ifdef TX_ENABLE_FIQ_SUPPORT
    CPSID   if                              // Enable IRQ and FIQ interrupts
#else
    CPSID   i                               // Enable IRQ interrupts
#endif
    LDR     r0, =_tx_thread_current_ptr     // Build current thread pointer address
    LDR     r1, [r0]                        // Pickup current thread pointer
    CMP     r1, #0                          // Check for NULL thread pointer
    BEQ     __tx_no_thread_to_disable       // If NULL, skip VFP disable
    MOV     r0, #0                          // Build disable value
    STR     r0, [r1, #144]                  // Clear the VFP enable flag (tx_thread_vfp_enable field in TX_THREAD)
__tx_no_thread_to_disable:
    MSR     CPSR_cxsf, r2                   // Recover CPSR
    BX      LR                              // Return to caller

#endif
