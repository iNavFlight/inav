@/**************************************************************************/
@/*                                                                        */
@/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
@/*                                                                        */
@/*       This software is licensed under the Microsoft Software License   */
@/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
@/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
@/*       and in the root directory of this software.                      */
@/*                                                                        */
@/**************************************************************************/
@
@
@/**************************************************************************/
@/**************************************************************************/
@/**                                                                       */ 
@/** ThreadX Component                                                     */ 
@/**                                                                       */
@/**   Thread                                                              */
@/**                                                                       */
@/**************************************************************************/
@/**************************************************************************/
@
@#define TX_SOURCE_CODE
@
@
@/* Include necessary system files.  */
@
@#include "tx_api.h"
@#include "tx_thread.h"
@
@
#ifdef TX_ENABLE_FIQ_SUPPORT
INT_MASK        =           0xC0                @ Interrupt bit mask
#else
INT_MASK        =           0x80                @ Interrupt bit mask
#endif
@
@
    .arm
    .text
    .align 2
@/**************************************************************************/ 
@/*                                                                        */ 
@/*  FUNCTION                                               RELEASE        */ 
@/*                                                                        */ 
@/*    _tx_thread_interrupt_control                    SMP/Cortex-A7/GNU   */ 
@/*                                                            6.1         */
@/*  AUTHOR                                                                */
@/*                                                                        */
@/*    William E. Lamie, Microsoft Corporation                             */
@/*                                                                        */
@/*  DESCRIPTION                                                           */
@/*                                                                        */ 
@/*    This function is responsible for changing the interrupt lockout     */ 
@/*    posture of the system.                                              */ 
@/*                                                                        */ 
@/*  INPUT                                                                 */ 
@/*                                                                        */ 
@/*    new_posture                           New interrupt lockout posture */ 
@/*                                                                        */ 
@/*  OUTPUT                                                                */ 
@/*                                                                        */ 
@/*    old_posture                           Old interrupt lockout posture */ 
@/*                                                                        */ 
@/*  CALLS                                                                 */ 
@/*                                                                        */ 
@/*    None                                                                */ 
@/*                                                                        */ 
@/*  CALLED BY                                                             */ 
@/*                                                                        */ 
@/*    Application Code                                                    */ 
@/*                                                                        */ 
@/*  RELEASE HISTORY                                                       */ 
@/*                                                                        */ 
@/*    DATE              NAME                      DESCRIPTION             */
@/*                                                                        */
@/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
@/*                                                                        */
@/**************************************************************************/
@UINT   _tx_thread_interrupt_control(UINT new_posture)
@{
    .global _tx_thread_interrupt_control
    .type   _tx_thread_interrupt_control,function
_tx_thread_interrupt_control:
@
@    /* Pickup current interrupt lockout posture.  */
@
    MRS     r3, CPSR                            @ Pickup current CPSR
    BIC     r1, r3, #INT_MASK                   @ Clear interrupt lockout bits
    ORR     r1, r1, r0                          @ Or-in new interrupt lockout bits
@
@    /* Apply the new interrupt posture.  */
@
    MSR     CPSR_c, r1                          @ Setup new CPSR
    AND     r0, r3, #INT_MASK                   @ Return previous interrupt mask
#ifdef __THUMB_INTERWORK
    BX      lr                          @ Return to caller
#else
    MOV     pc, lr                      @ Return to caller
#endif
@
@}
@


