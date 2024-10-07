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

INT_MASK        =   0x03F


/* Define the 16-bit Thumb mode veneer for _tx_thread_interrupt_control for
   applications calling this function from to 16-bit Thumb mode.  */

    .text
    .align 2
    .global $_tx_thread_interrupt_control
$_tx_thread_interrupt_control:
        .thumb
     BX        pc                               // Switch to 32-bit mode
     NOP                                        //
    .arm
     STMFD     sp!, {lr}                        // Save return address
     BL        _tx_thread_interrupt_control     // Call _tx_thread_interrupt_control function
     LDMFD     sp!, {lr}                        // Recover saved return address
     BX        lr                               // Return to 16-bit caller


    .text
    .align 2
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_interrupt_control                         ARMv7-A        */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is responsible for changing the interrupt lockout     */
/*    posture of the system.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    new_posture                           New interrupt lockout posture */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    old_posture                           Old interrupt lockout posture */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
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
    .global _tx_thread_interrupt_control
    .type   _tx_thread_interrupt_control,function
_tx_thread_interrupt_control:

    /* Pickup current interrupt lockout posture.  */

    MRS     r3, CPSR                    // Pickup current CPSR
    MOV     r2, #INT_MASK               // Build interrupt mask
    AND     r1, r3, r2                  // Clear interrupt lockout bits
    ORR     r1, r1, r0                  // Or-in new interrupt lockout bits

    /* Apply the new interrupt posture.  */

    MSR     CPSR_c, r1                  // Setup new CPSR
    BIC     r0, r3, r2                  // Return previous interrupt mask
#ifdef __THUMB_INTERWORK
    BX      lr                          // Return to caller
#else
    MOV     pc, lr                      // Return to caller
#endif
