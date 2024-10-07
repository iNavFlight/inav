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
/**   Timer                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

    .arm


/* Define Assembly language external references...  */

    .global     _tx_timer_time_slice
    .global     _tx_timer_system_clock
    .global     _tx_timer_current_ptr
    .global     _tx_timer_list_start
    .global     _tx_timer_list_end
    .global     _tx_timer_expired_time_slice
    .global     _tx_timer_expired
    .global     _tx_thread_time_slice



/* Define the 16-bit Thumb mode veneer for _tx_timer_interrupt for
   applications calling this function from to 16-bit Thumb mode.  */

    .text
    .align 2
    .thumb
    .global $_tx_timer_interrupt
    .type   $_tx_timer_interrupt,function
$_tx_timer_interrupt:
     BX        pc                               // Switch to 32-bit mode
     NOP                                        //
    .arm
     STMFD     sp!, {lr}                        // Save return address
     BL        _tx_timer_interrupt              // Call _tx_timer_interrupt function
     LDMFD     sp!, {lr}                        // Recover saved return address
     BX        lr                               // Return to 16-bit caller


    .text
    .align 2
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_timer_interrupt                                  ARMv7-A        */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the hardware timer interrupt.  This         */
/*    processing includes incrementing the system clock and checking for  */
/*    time slice and/or timer expiration.  If either is found, the        */
/*    interrupt context save/restore functions are called along with the  */
/*    expiration functions.                                               */
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
/*    _tx_thread_time_slice                 Time slice interrupted thread */
/*    _tx_timer_expiration_process          Timer expiration processing   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    interrupt vector                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*  04-25-2022     Zhen Kong                Updated comments,             */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
    .global _tx_timer_interrupt
    .type   _tx_timer_interrupt,function
_tx_timer_interrupt:

    /* Upon entry to this routine, it is assumed that context save has already
       been called, and therefore the compiler scratch registers are available
       for use.  */

    /* Increment the system clock.  */

    LDR     r1, =_tx_timer_system_clock         // Pickup address of system clock
    LDR     r0, [r1]                            // Pickup system clock
    ADD     r0, r0, #1                          // Increment system clock
    STR     r0, [r1]                            // Store new system clock

    /* Test for time-slice expiration.  */

    LDR     r3, =_tx_timer_time_slice           // Pickup address of time-slice
    LDR     r2, [r3]                            // Pickup time-slice
    CMP     r2, #0                              // Is it non-active?
    BEQ     __tx_timer_no_time_slice            // Yes, skip time-slice processing

    /* Decrement the time_slice.  */

    SUB     r2, r2, #1                          // Decrement the time-slice
    STR     r2, [r3]                            // Store new time-slice value

    /* Check for expiration.  */

    CMP     r2, #0                              // Has it expired?
    BNE     __tx_timer_no_time_slice            // No, skip expiration processing

    /* Set the time-slice expired flag.  */

    LDR     r3, =_tx_timer_expired_time_slice   // Pickup address of expired flag
    MOV     r0, #1                              // Build expired value
    STR     r0, [r3]                            // Set time-slice expiration flag

__tx_timer_no_time_slice:

    /* Test for timer expiration.  */

    LDR     r1, =_tx_timer_current_ptr          // Pickup current timer pointer address
    LDR     r0, [r1]                            // Pickup current timer
    LDR     r2, [r0]                            // Pickup timer list entry
    CMP     r2, #0                              // Is there anything in the list?
    BEQ     __tx_timer_no_timer                 // No, just increment the timer

    /* Set expiration flag.  */

    LDR     r3, =_tx_timer_expired              // Pickup expiration flag address
    MOV     r2, #1                              // Build expired value
    STR     r2, [r3]                            // Set expired flag
    B       __tx_timer_done                     // Finished timer processing

__tx_timer_no_timer:

    /* No timer expired, increment the timer pointer.  */
    ADD     r0, r0, #4                          // Move to next timer

    /* Check for wraparound.  */

    LDR     r3, =_tx_timer_list_end             // Pickup address of timer list end
    LDR     r2, [r3]                            // Pickup list end
    CMP     r0, r2                              // Are we at list end?
    BNE     __tx_timer_skip_wrap                // No, skip wraparound logic

    /* Wrap to beginning of list.  */

    LDR     r3, =_tx_timer_list_start           // Pickup address of timer list start
    LDR     r0, [r3]                            // Set current pointer to list start

__tx_timer_skip_wrap:

    STR     r0, [r1]                            // Store new current timer pointer

__tx_timer_done:

    /* See if anything has expired.  */

    LDR     r3, =_tx_timer_expired_time_slice   // Pickup address of expired flag
    LDR     r2, [r3]                            // Pickup time-slice expired flag
    CMP     r2, #0                              // Did a time-slice expire?
    BNE     __tx_something_expired              // If non-zero, time-slice expired
    LDR     r1, =_tx_timer_expired              // Pickup address of other expired flag
    LDR     r0, [r1]                            // Pickup timer expired flag
    CMP     r0, #0                              // Did a timer expire?
    BEQ     __tx_timer_nothing_expired          // No, nothing expired

__tx_something_expired:

    STMDB   sp!, {r0, lr}                       // Save the lr register on the stack
                                                //   and save r0 just to keep 8-byte alignment

    /* Did a timer expire?  */

    LDR     r1, =_tx_timer_expired              // Pickup address of expired flag
    LDR     r0, [r1]                            // Pickup timer expired flag
    CMP     r0, #0                              // Check for timer expiration
    BEQ     __tx_timer_dont_activate            // If not set, skip timer activation

    /* Process timer expiration.  */
    BL      _tx_timer_expiration_process        // Call the timer expiration handling routine

__tx_timer_dont_activate:

    /* Did time slice expire?  */

    LDR     r3, =_tx_timer_expired_time_slice   // Pickup address of time-slice expired
    LDR     r2, [r3]                            // Pickup the actual flag
    CMP     r2, #0                              // See if the flag is set
    BEQ     __tx_timer_not_ts_expiration        // No, skip time-slice processing

    /* Time slice interrupted thread.  */

    BL      _tx_thread_time_slice               // Call time-slice processing

__tx_timer_not_ts_expiration:

    LDMIA   sp!, {r0, lr}                       // Recover lr register (r0 is just there for
                                                //   the 8-byte stack alignment

__tx_timer_nothing_expired:

#ifdef __THUMB_INTERWORK
    BX      lr                                  // Return to caller
#else
    MOV     pc, lr                              // Return to caller
#endif
