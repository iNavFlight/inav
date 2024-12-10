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

    IMPORT      _tx_timer_time_slice
    IMPORT      _tx_timer_system_clock
    IMPORT      _tx_timer_current_ptr
    IMPORT      _tx_timer_list_start
    IMPORT      _tx_timer_list_end
    IMPORT      _tx_timer_expired_time_slice
    IMPORT      _tx_timer_expired
    IMPORT      _tx_thread_time_slice
    IMPORT      _tx_timer_expiration_process
    IMPORT      _tx_thread_preempt_disable
    IMPORT      _tx_thread_current_ptr
    IMPORT      _tx_thread_execute_ptr

    AREA ||.text||, CODE, READONLY
    PRESERVE8
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_timer_interrupt                              Cortex-M4/AC5      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the hardware timer interrupt.  This         */
/*    processing includes incrementing the system clock and checking for  */
/*    time slice and/or timer expiration.  If either is found, the        */
/*    expiration functions are called.                                    */
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
/*    _tx_timer_expiration_process          Timer expiration processing   */
/*    _tx_thread_time_slice                 Time slice interrupted thread */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    interrupt vector                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      Scott Larson            Initial Version 6.1.7         */
/*  01-31-2022      Scott Larson            Modified comment(s), added    */
/*                                            TX_NO_TIMER support,        */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
// VOID   _tx_timer_interrupt(VOID)
// {
#ifndef TX_NO_TIMER
    EXPORT  _tx_timer_interrupt
_tx_timer_interrupt

    /* Upon entry to this routine, it is assumed that the compiler scratch registers are available
       for use.  */

    /* Increment the system clock.  */
    // _tx_timer_system_clock++;

    LDR     r1, =_tx_timer_system_clock             // Pickup address of system clock
    LDR     r0, [r1, #0]                            // Pickup system clock
    ADD     r0, r0, #1                              // Increment system clock
    STR     r0, [r1, #0]                            // Store new system clock

    /* Test for time-slice expiration.  */
    // if (_tx_timer_time_slice)
    // {

    LDR     r3, =_tx_timer_time_slice               // Pickup address of time-slice
    LDR     r2, [r3, #0]                            // Pickup time-slice
    CBZ     r2, __tx_timer_no_time_slice            // Is it non-active?
                                                    // Yes, skip time-slice processing

       /* Decrement the time_slice.  */
       // _tx_timer_time_slice--;

    SUB     r2, r2, #1                              // Decrement the time-slice
    STR     r2, [r3, #0]                            // Store new time-slice value

       /* Check for expiration.  */
       // if (__tx_timer_time_slice == 0)

    CBNZ    r2, __tx_timer_no_time_slice            // Has it expired?
                                                    // No, skip expiration processing

       /* Set the time-slice expired flag.  */
       // _tx_timer_expired_time_slice =  TX_TRUE;

    LDR     r3, =_tx_timer_expired_time_slice       // Pickup address of expired flag
    MOV     r0, #1                                  // Build expired value
    STR     r0, [r3, #0]                            // Set time-slice expiration flag

    // }

__tx_timer_no_time_slice

    /* Test for timer expiration.  */
    // if (*_tx_timer_current_ptr)
    // {

    LDR     r1, =_tx_timer_current_ptr              // Pickup current timer pointer address
    LDR     r0, [r1, #0]                            // Pickup current timer
    LDR     r2, [r0, #0]                            // Pickup timer list entry
    CBZ     r2, __tx_timer_no_timer                 // Is there anything in the list?
                                                    // No, just increment the timer

        /* Set expiration flag.  */
        // _tx_timer_expired =  TX_TRUE;

    LDR     r3, =_tx_timer_expired                  // Pickup expiration flag address
    MOV     r2, #1                                  // Build expired value
    STR     r2, [r3, #0]                            // Set expired flag
    B       __tx_timer_done                         // Finished timer processing

    // }
    // else
    // {
__tx_timer_no_timer

        /* No timer expired, increment the timer pointer.  */
        // _tx_timer_current_ptr++;

    ADD     r0, r0, #4                              // Move to next timer

        /* Check for wrap-around.  */
        // if (_tx_timer_current_ptr == _tx_timer_list_end)

    LDR     r3, =_tx_timer_list_end                 // Pickup addr of timer list end
    LDR     r2, [r3, #0]                            // Pickup list end
    CMP     r0, r2                                  // Are we at list end?
    BNE     __tx_timer_skip_wrap                    // No, skip wrap-around logic

            /* Wrap to beginning of list.  */
            // _tx_timer_current_ptr =  _tx_timer_list_start;

    LDR     r3, =_tx_timer_list_start               // Pickup addr of timer list start
    LDR     r0, [r3, #0]                            // Set current pointer to list start

__tx_timer_skip_wrap

    STR     r0, [r1, #0]                            // Store new current timer pointer
    // }

__tx_timer_done

    /* See if anything has expired.  */
    // if ((_tx_timer_expired_time_slice) || (_tx_timer_expired))
    // {

    LDR     r3, =_tx_timer_expired_time_slice       // Pickup addr of expired flag
    LDR     r2, [r3, #0]                            // Pickup time-slice expired flag
    CBNZ    r2, __tx_something_expired              // Did a time-slice expire?
                                                    // If non-zero, time-slice expired
    LDR     r1, =_tx_timer_expired                  // Pickup addr of other expired flag
    LDR     r0, [r1, #0]                            // Pickup timer expired flag
    CBZ     r0, __tx_timer_nothing_expired          // Did a timer expire?
                                                    // No, nothing expired

__tx_something_expired

    STMDB   sp!, {r0, lr}                           // Save the lr register on the stack
                                                    //   and save r0 just to keep 8-byte alignment

    /* Did a timer expire?  */
    // if (_tx_timer_expired)
    // {

    LDR     r1, =_tx_timer_expired                  // Pickup addr of expired flag
    LDR     r0, [r1, #0]                            // Pickup timer expired flag
    CBZ     r0, __tx_timer_dont_activate            // Check for timer expiration
                                                    // If not set, skip timer activation

        /* Process timer expiration.  */
        // _tx_timer_expiration_process();

    BL      _tx_timer_expiration_process            // Call the timer expiration handling routine

    // }
__tx_timer_dont_activate

    /* Did time slice expire?  */
    // if (_tx_timer_expired_time_slice)
    // {

    LDR     r3, =_tx_timer_expired_time_slice       // Pickup addr of time-slice expired
    LDR     r2, [r3, #0]                            // Pickup the actual flag
    CBZ     r2, __tx_timer_not_ts_expiration        // See if the flag is set
                                                    // No, skip time-slice processing

        /* Time slice interrupted thread.  */
        // _tx_thread_time_slice();

    BL      _tx_thread_time_slice                   // Call time-slice processing
    LDR     r0, =_tx_thread_preempt_disable         // Build address of preempt disable flag
    LDR     r1, [r0]                                // Is the preempt disable flag set?
    CBNZ    r1, __tx_timer_skip_time_slice          // Yes, skip the PendSV logic
    LDR     r0, =_tx_thread_current_ptr             // Build current thread pointer address
    LDR     r1, [r0]                                // Pickup the current thread pointer
    LDR     r2, =_tx_thread_execute_ptr             // Build execute thread pointer address
    LDR     r3, [r2]                                // Pickup the execute thread pointer
    LDR     r0, =0xE000ED04                         // Build address of control register
    LDR     r2, =0x10000000                         // Build value for PendSV bit
    CMP     r1, r3                                  // Are they the same?
    BEQ     __tx_timer_skip_time_slice              // If the same, there was no time-slice performed
    STR     r2, [r0]                                // Not the same, issue the PendSV for preemption
__tx_timer_skip_time_slice

    // }

__tx_timer_not_ts_expiration

    LDMIA   sp!, {r0, lr}                           // Recover lr register (r0 is just there for
                                                    //   the 8-byte stack alignment

    // }

__tx_timer_nothing_expired

    DSB                                             // Complete all memory access
    BX      lr                                      // Return to caller
// }
#endif
    ALIGN
    LTORG
    END
