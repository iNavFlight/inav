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
@/**   Thread - Low Level SMP Support                                      */
@/**                                                                       */
@/**************************************************************************/
@/**************************************************************************/
@
@
@#define TX_SOURCE_CODE
@#define TX_THREAD_SMP_SOURCE_CODE
@
@/* Include necessary system files.  */
@
@#include "tx_api.h"
@#include "tx_thread.h"
@#include "tx_timer.h"  */
@
@
    .global    _tx_thread_current_ptr
    .global    _tx_thread_smp_protection
    .global    _tx_thread_preempt_disable
    .global    _tx_thread_smp_protect_wait_counts

    .arm
    .text
    .align 2
@/**************************************************************************/ 
@/*                                                                        */ 
@/*  FUNCTION                                               RELEASE        */ 
@/*                                                                        */ 
@/*    _tx_thread_smp_unprotect                        SMP/Cortex-A7/GNU   */ 
@/*                                                            6.1         */
@/*  AUTHOR                                                                */
@/*                                                                        */
@/*    William E. Lamie, Microsoft Corporation                             */
@/*                                                                        */
@/*  DESCRIPTION                                                           */
@/*                                                                        */ 
@/*    This function releases previously obtained protection. The supplied */ 
@/*    previous SR is restored. If the value of _tx_thread_system_state    */ 
@/*    and _tx_thread_preempt_disable are both zero, then multithreading   */ 
@/*    is enabled as well.                                                 */ 
@/*                                                                        */ 
@/*  INPUT                                                                 */ 
@/*                                                                        */ 
@/*    Previous Status Register                                            */ 
@/*                                                                        */ 
@/*  OUTPUT                                                                */ 
@/*                                                                        */ 
@/*    None                                                                */
@/*                                                                        */ 
@/*  CALLS                                                                 */ 
@/*                                                                        */ 
@/*    None                                                                */
@/*                                                                        */ 
@/*  CALLED BY                                                             */ 
@/*                                                                        */ 
@/*    ThreadX Source                                                      */
@/*                                                                        */ 
@/*  RELEASE HISTORY                                                       */ 
@/*                                                                        */ 
@/*    DATE              NAME                      DESCRIPTION             */
@/*                                                                        */
@/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
@/*                                                                        */
@/**************************************************************************/
    .global _tx_thread_smp_unprotect
    .type   _tx_thread_smp_unprotect,function
_tx_thread_smp_unprotect:
@
@    /* Lockout interrupts.  */
@
#ifdef TX_ENABLE_FIQ_SUPPORT
    CPSID   if                                  @ Disable IRQ and FIQ interrupts
#else
    CPSID   i                                   @ Disable IRQ interrupts
#endif

    MRC     p15, 0, r1, c0, c0, 5               @ Read CPU ID register
    AND     r1, r1, #0x03                       @ Mask off, leaving the CPU ID field

    LDR     r2,=_tx_thread_smp_protection       @ Build address of protection structure
    LDR     r3, [r2, #8]                        @ Pickup the owning core
    CMP     r1, r3                              @ Is it this core?
    BNE     _still_protected                    @ If this is not the owning core, protection is in force elsewhere

    LDR     r3, [r2, #12]                       @ Pickup the protection count
    CMP     r3, #0                              @ Check to see if the protection is still active
    BEQ     _still_protected                    @ If the protection count is zero, protection has already been cleared

    SUB     r3, r3, #1                          @ Decrement the protection count
    STR     r3, [r2, #12]                       @ Store the new count back
    CMP     r3, #0                              @ Check to see if the protection is still active
    BNE     _still_protected                    @ If the protection count is non-zero, protection is still in force
    LDR     r2,=_tx_thread_preempt_disable      @ Build address of preempt disable flag
    LDR     r3, [r2]                            @ Pickup preempt disable flag
    CMP     r3, #0                              @ Is the preempt disable flag set?
    BNE     _still_protected                    @ Yes, skip the protection release

    LDR     r2,=_tx_thread_smp_protect_wait_counts @ Build build address of wait counts
    LDR     r3, [r2, r1, LSL #2]                @ Pickup wait list value
    CMP     r3, #0                              @ Are any entities on this core waiting?
    BNE     _still_protected                    @ Yes, skip the protection release

    LDR     r2,=_tx_thread_smp_protection       @ Build address of protection structure
    MOV     r3, #0xFFFFFFFF                     @ Build invalid value
    STR     r3, [r2, #8]                        @ Mark the protected core as invalid
#ifdef TX_MPCORE_DEBUG_ENABLE
    STR     LR, [r2, #16]                       @ Save caller's return address
#endif
    DMB                                         @ Ensure that accesses to shared resource have completed
    MOV     r3, #0                              @ Build release protection value
    STR     r3, [r2, #0]                        @ Release the protection
    DSB                                         @ To ensure update of the protection occurs before other CPUs awake
#ifdef TX_ENABLE_WFE
    SEV                                         @ Send event to other CPUs, wakes anyone waiting on the protection (using WFE)
#endif

_still_protected:
    MSR     CPSR_c, r0                          @ Restore CPSR

#ifdef __THUMB_INTERWORK
    BX      lr                                  @ Return to caller
#else
    MOV     pc, lr                              @ Return to caller
#endif

