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

    .text 32
    .align 4
    .syntax unified
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_system_return                         Cortex-M7/GNU      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is target processor specific.  It is used to transfer */
/*    control from a thread back to the ThreadX system.  Only a           */
/*    minimal context is saved since the compiler assumes temp registers  */
/*    are going to get slicked by a function call anyway.                 */
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
/*    _tx_thread_schedule                   Thread scheduling loop        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    ThreadX components                                                  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      Scott Larson            Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
// VOID   _tx_thread_system_return(VOID)
// {
    .thumb_func
    .global  _tx_thread_system_return
_tx_thread_system_return:

    /* Return to real scheduler via PendSV. Note that this routine is often
       replaced with in-line assembly in tx_port.h to improved performance.  */

    MOV     r0, #0x10000000                         // Load PENDSVSET bit
    MOV     r1, #0xE000E000                         // Load NVIC base
    STR     r0, [r1, #0xD04]                        // Set PENDSVBIT in ICSR
    MRS     r0, IPSR                                // Pickup IPSR
    CMP     r0, #0                                  // Is it a thread returning?
    BNE     _isr_context                            // If ISR, skip interrupt enable
#ifdef TX_PORT_USE_BASEPRI
    MRS     r1, BASEPRI                             // Thread context returning, pickup BASEPRI
    MOV     r0, #0
    MSR     BASEPRI, r0                             // Enable interrupts
    MSR     BASEPRI, r1                             // Restore original interrupt posture
#else
    MRS     r1, PRIMASK                             // Thread context returning, pickup PRIMASK
    CPSIE   i                                       // Enable interrupts
    MSR     PRIMASK, r1                             // Restore original interrupt posture
#endif
_isr_context:
    BX      lr                                      // Return to caller
// }
