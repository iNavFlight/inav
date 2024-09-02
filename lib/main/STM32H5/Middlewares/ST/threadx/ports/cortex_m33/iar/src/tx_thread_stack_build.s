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

    SECTION `.text`:CODE:NOROOT(2)
    THUMB
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_stack_build                            Cortex-M33/IAR    */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function builds a stack frame on the supplied thread's stack.  */
/*    The stack frame results in a fake interrupt return to the supplied  */
/*    function pointer.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Pointer to thread control blk */
/*    function_ptr                          Pointer to return function    */
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
/*    _tx_thread_create                     Create thread service         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
// VOID   _tx_thread_stack_build(TX_THREAD *thread_ptr, VOID (*function_ptr)(VOID))
// {
    PUBLIC  _tx_thread_stack_build
_tx_thread_stack_build:
    /* Build a fake interrupt frame.  The form of the fake interrupt stack
       on the Cortex-M should look like the following after it is built:

       Stack Top:
                       LR          Interrupted LR (LR at time of PENDSV)
                       r4          Initial value for r4
                       r5          Initial value for r5
                       r6          Initial value for r6
                       r7          Initial value for r7
                       r8          Initial value for r8
                       r9          Initial value for r9
                       r10         Initial value for r10
                       r11         Initial value for r11
                       r0          Initial value for r0    (Hardware stack starts here!!)
                       r1          Initial value for r1
                       r2          Initial value for r2
                       r3          Initial value for r3
                       r12         Initial value for r12
                       lr          Initial value for lr
                       pc          Initial value for pc
                       xPSR        Initial value for xPSR

    Stack Bottom: (higher memory address)  */

    LDR     r2, [r0, #16]                           // Pickup end of stack area
    BIC     r2, r2, #0x7                            // Align frame for 8-byte alignment
    SUB     r2, r2, #68                             // Subtract frame size
#ifdef TX_SINGLE_MODE_SECURE
    LDR     r3, =0xFFFFFFFD                         // Build initial LR value for secure mode
#else
    LDR     r3, =0xFFFFFFBC                         // Build initial LR value to return to non-secure PSP
#endif
    STR     r3, [r2, #0]                            // Save on the stack

    /* Actually build the stack frame.  */

    MOV     r3, #0                                  // Build initial register value
    STR     r3, [r2, #4]                            // Store initial r4
    STR     r3, [r2, #8]                            // Store initial r5
    STR     r3, [r2, #12]                           // Store initial r6
    STR     r3, [r2, #16]                           // Store initial r7
    STR     r3, [r2, #20]                           // Store initial r8
    STR     r3, [r2, #24]                           // Store initial r9
    STR     r3, [r2, #28]                           // Store initial r10
    STR     r3, [r2, #32]                           // Store initial r11

    /* Hardware stack follows.  */

    STR     r3, [r2, #36]                           // Store initial r0
    STR     r3, [r2, #40]                           // Store initial r1
    STR     r3, [r2, #44]                           // Store initial r2
    STR     r3, [r2, #48]                           // Store initial r3
    STR     r3, [r2, #52]                           // Store initial r12
    MOV     r3, #0xFFFFFFFF                         // Poison EXC_RETURN value
    STR     r3, [r2, #56]                           // Store initial lr
    STR     r1, [r2, #60]                           // Store initial pc
    MOV     r3, #0x01000000                         // Only T-bit need be set
    STR     r3, [r2, #64]                           // Store initial xPSR

    /* Setup stack pointer.  */
    // thread_ptr -> tx_thread_stack_ptr =  r2;

    STR     r2, [r0, #8]                            // Save stack pointer in thread's
                                                    //   control block
    BX      lr                                      // Return to caller
// }
    END
