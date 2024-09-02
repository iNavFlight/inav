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
/**   Module Manager                                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


#define THUMB_MASK      0x20                    // THUMB bit
#define USR_MODE        0x10                    // USR mode
#define SYS_MODE        0x1F                    // SYS mode
#ifdef TX_ENABLE_FIQ_SUPPORT
#define CPSR_MASK       0xDF                    // Mask initial CPSR, IRQ & FIQ ints enabled
#else
#define CPSR_MASK       0x9F                    // Mask initial CPSR, IRQ ints enabled
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_thread_stack_build          Cortex-A7/MMU/GNU   */
/*                                                           6.x          */
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
/*  xx-xx-xxxx      Scott Larson            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
// VOID   _txm_module_manager_thread_stack_build(TX_THREAD *thread_ptr, VOID (*function_ptr)(TX_THREAD *, TXM_MODULE_INSTANCE *))
// {
    .text
    .global _txm_module_manager_thread_stack_build
    .type   _txm_module_manager_thread_stack_build, "function"
_txm_module_manager_thread_stack_build:


    /* Build a fake interrupt frame.  The form of the fake interrupt stack
       on the Cortex-A7 should look like the following after it is built:

       Stack Top:      1           Interrupt stack frame type
                       CPSR        Initial value for CPSR
                       r0          Initial value for r0
                       r1          Initial value for r1
                       r2          Initial value for r2
                       r3          Initial value for r3
                       r4          Initial value for r4
                       r5          Initial value for r5
                       r6          Initial value for r6
                       r7          Initial value for r7
                       r8          Initial value for r8
                       r9          Initial value for r9
                       r10         Initial value for r10
                       r11         Initial value for r11
                       r12         Initial value for r12
                       lr          Initial value for lr (r14)
                       pc          Initial value for pc (r15)
                       0           For stack backtracing

    Stack Bottom: (higher memory address)  */

    LDR     r2, [r0, #16]                       // Pickup end of stack area
    BIC     r2, r2, #7                          // Ensure 8-byte alignment
    SUB     r2, r2, #76                         // Allocate space for the stack frame

    /* Actually build the stack frame.  */

    MOV     r3, #1                              // Build interrupt stack type
    STR     r3, [r2, #0]                        // Store stack type
    STR     r0, [r2, #8]                        // Store initial r0 (thread pointer)
    LDR     r3, [r0, #8]                        // Pickup thread info pointer (it's in the stack pointer location right now)
    STR     r3, [r2, #12]                       // Store initial r1
    LDR     r3, [r3, #8]                        // Pickup data base register
    STR     r3, [r2, #44]                       // Store initial r9
    MOV     r3, #0                              // Build initial register value
    STR     r3, [r2, #16]                       // Store initial r2
    STR     r3, [r2, #20]                       // Store initial r3
    STR     r3, [r2, #24]                       // Store initial r4
    STR     r3, [r2, #28]                       // Store initial r5
    STR     r3, [r2, #32]                       // Store initial r6
    STR     r3, [r2, #36]                       // Store initial r7
    STR     r3, [r2, #40]                       // Store initial r8
    LDR     r3, [r0, #12]                       // Pickup stack starting address
    STR     r3, [r2, #48]                       // Store initial r10 (sl)
    MOV     r3, #0                              // Build initial register value
    STR     r3, [r2, #52]                       // Store initial r11
    STR     r3, [r2, #56]                       // Store initial r12
    STR     r3, [r2, #60]                       // Store initial lr
    STR     r1, [r2, #64]                       // Store initial pc
    STR     r3, [r2, #68]                       // 0 for back-trace
    MRS     r3, CPSR                            // Pickup CPSR
    BIC     r3, r3, #CPSR_MASK                  // Mask mode bits of CPSR
    TST     r1, #1                              // Test if THUMB bit set in initial PC
    ORRNE   r3, r3, #THUMB_MASK                 // Set T bit if set
    LDR     r1, [r0, #156]                      // Load tx_thread_module_current_user_mode
    TST     r1, #1                              // Test if the flag is set
    ORREQ   r3, r3, #SYS_MODE                   // Flag not set: Build CPSR, SYS mode, IRQ enabled
    ORRNE   r3, r3, #USR_MODE                   // Flag set: Build CPSR, USR mode, IRQ enabled
    STR     r3, [r2, #4]                        // Store initial CPSR

    /* Setup stack pointer.  */
    // thread_ptr -> tx_thread_stack_ptr =  r2;

    STR     r2, [r0, #8]                        // Save stack pointer in thread's control block
    BX      lr                                  // Return to caller
// }
