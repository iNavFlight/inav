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

@/* Include macros for modifying the wait list.  */
#include "tx_thread_smp_protection_wait_list_macros.h"

    .global    _tx_thread_current_ptr
    .global    _tx_thread_smp_protection
    .global    _tx_thread_smp_protect_wait_counts
    .global    _tx_thread_smp_protect_wait_list
    .global    _tx_thread_smp_protect_wait_list_lock_protect_in_force
    .global    _tx_thread_smp_protect_wait_list_head
    .global    _tx_thread_smp_protect_wait_list_tail
    .global    _tx_thread_smp_protect_wait_list_size

    .arm
    .text
    .align 2
@/**************************************************************************/ 
@/*                                                                        */ 
@/*  FUNCTION                                               RELEASE        */ 
@/*                                                                        */ 
@/*    _tx_thread_smp_protect                          SMP/Cortex-A7/GNU   */ 
@/*                                                            6.1.12      */
@/*  AUTHOR                                                                */
@/*                                                                        */
@/*    William E. Lamie, Microsoft Corporation                             */
@/*                                                                        */
@/*  DESCRIPTION                                                           */
@/*                                                                        */ 
@/*    This function gets protection for running inside the ThreadX        */ 
@/*    source. This is ccomplished by a combination of a test-and-set      */ 
@/*    flag and periodically disabling interrupts.                         */ 
@/*                                                                        */ 
@/*  INPUT                                                                 */ 
@/*                                                                        */ 
@/*    None                                                                */
@/*                                                                        */ 
@/*  OUTPUT                                                                */ 
@/*                                                                        */ 
@/*    Previous Status Register                                            */ 
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
@/*  09-30-2020      William E. Lamie        Initial Version 6.1           */
@/*  07-29-2022      Scott Larson            Fixed preprocessor statement, */
@/*                                            resulting in version 6.1.12 */
@/*                                                                        */
@/**************************************************************************/
    .global _tx_thread_smp_protect
    .type   _tx_thread_smp_protect,function
_tx_thread_smp_protect:
@VOID  _tx_thread_smp_protect(VOID)
@{
@
    PUSH    {r4-r6}                             @ Save registers we'll be using
@
@    /* Disable interrupts so we don't get preempted.  */
@
    MRS     r0, CPSR                            @ Pickup current CPSR

#ifdef TX_ENABLE_FIQ_SUPPORT
    CPSID   if                                  @ Disable IRQ and FIQ interrupts
#else
    CPSID   i                                   @ Disable IRQ interrupts
#endif
@
@    /* Do we already have protection?  */
@    if (this_core == _tx_thread_smp_protection.tx_thread_smp_protect_core)
@    {
@
    MRC     p15, 0, r1, c0, c0, 5               @ Read CPU ID register
    AND     r1, r1, #0x03                       @ Mask off, leaving the CPU ID field
    LDR     r2, =_tx_thread_smp_protection      @ Build address to protection structure
    LDR     r3, [r2, #8]                        @ Pickup the owning core
    CMP     r1, r3                              @ Is it not this core?
    BNE     _protection_not_owned               @ No, the protection is not already owned
@
@    /* We already have protection. */
@
@    /* Increment the protection count. */
@    _tx_thread_smp_protection.tx_thread_smp_protect_count++;
@
    LDR     r3, [r2, #12]                       @ Pickup ownership count
    ADD     r3, r3, #1                          @ Increment ownership count
    STR     r3, [r2, #12]                       @ Store ownership count
    DMB

    B       _return

_protection_not_owned:
@
@    /* Is the lock available?  */
@    if (_tx_thread_smp_protection.tx_thread_smp_protect_in_force == 0)
@    {
@
    LDREX   r3, [r2, #0]                        @ Pickup the protection flag
    CMP     r3, #0
    BNE     _start_waiting                      @ No, protection not available
@
@    /* Is the list empty?  */
@    if (_tx_thread_smp_protect_wait_list_head == _tx_thread_smp_protect_wait_list_tail)
@    {
@
    LDR     r3, =_tx_thread_smp_protect_wait_list_head
    LDR     r3, [r3]
    LDR     r4, =_tx_thread_smp_protect_wait_list_tail
    LDR     r4, [r4]
    CMP     r3, r4
    BNE     _list_not_empty
@
@    /* Try to get the lock.  */
@    if (write_exclusive(&_tx_thread_smp_protection.tx_thread_smp_protect_in_force, 1) == SUCCESS)
@    {
@
    MOV     r3, #1                              @ Build lock value
    STREX   r4, r3, [r2, #0]                    @ Attempt to get the protection
    CMP     r4, #0
    BNE     _start_waiting                      @ Did it fail?
@
@    /* We got the lock! */
@    _tx_thread_smp_protect_lock_got();
@
    DMB                                         @ Ensure write to protection finishes
    _tx_thread_smp_protect_lock_got             @ Call the lock got function

    B       _return

_list_not_empty:
@
@    /* Are we at the front of the list?  */
@    if (this_core == _tx_thread_smp_protect_wait_list[_tx_thread_smp_protect_wait_list_head])
@    {
@
    LDR     r3, =_tx_thread_smp_protect_wait_list_head @ Get the address of the head
    LDR     r3, [r3]                            @ Get the value of the head
    LDR     r4, =_tx_thread_smp_protect_wait_list @ Get the address of the list
    LDR     r4, [r4, r3, LSL #2]                @ Get the value at the head index

    CMP     r1, r4
    BNE     _start_waiting
@
@    /* Is the lock still available?  */
@    if (_tx_thread_smp_protection.tx_thread_smp_protect_in_force == 0)
@    {
@
    LDR     r3, [r2, #0]                        @ Pickup the protection flag
    CMP     r3, #0
    BNE     _start_waiting                      @ No, protection not available
@
@    /* Get the lock.  */
@    _tx_thread_smp_protection.tx_thread_smp_protect_in_force = 1;
@
    MOV     r3, #1                              @ Build lock value
    STR     r3, [r2, #0]                        @ Store lock value
    DMB                                         @
@
@    /* Got the lock.  */
@    _tx_thread_smp_protect_lock_got();
@
    _tx_thread_smp_protect_lock_got
@
@    /* Remove this core from the wait list.  */
@    _tx_thread_smp_protect_remove_from_front_of_list();
@
    _tx_thread_smp_protect_remove_from_front_of_list

    B       _return

_start_waiting:
@
@    /* For one reason or another, we didn't get the lock.  */
@
@    /* Increment wait count. */
@    _tx_thread_smp_protect_wait_counts[this_core]++;
@
    LDR     r3, =_tx_thread_smp_protect_wait_counts @ Load wait list counts
    LDR     r4, [r3, r1, LSL #2]                @ Load waiting value for this core
    ADD     r4, r4, #1                          @ Increment wait value
    STR     r4, [r3, r1, LSL #2]                @ Store new wait value
@
@    /* Have we not added ourselves to the list yet? */
@    if (_tx_thread_smp_protect_wait_counts[this_core] == 1)
@    {
@
    CMP     r4, #1
    BNE     _already_in_list0                   @ Is this core already waiting?
@
@    /* Add ourselves to the list.  */
@    _tx_thread_smp_protect_wait_list_add(this_core);
@
    _tx_thread_smp_protect_wait_list_add @ Call macro to add ourselves to the list
@
@    }
@
_already_in_list0:
@
@    /* Restore interrupts. */
@
    MSR     CPSR_c, r0                          @ Restore CPSR
#ifdef TX_ENABLE_WFE
    WFE                                         @ Go into standby
#endif
@
@    /* We do this until we have the lock. */
@    while (1)
@    {
@
_try_to_get_lock:
@
@    /* Disable interrupts so we don't get preempted.  */
@
#ifdef TX_ENABLE_FIQ_SUPPORT
    CPSID   if                                  @ Disable IRQ and FIQ interrupts
#else
    CPSID   i                                   @ Disable IRQ interrupts
#endif

    MRC     p15, 0, r1, c0, c0, 5               @ Read CPU ID register
    AND     r1, r1, #0x03                       @ Mask off, leaving the CPU ID field
@
@    /* Do we already have protection?  */
@    if (this_core == _tx_thread_smp_protection.tx_thread_smp_protect_core)
@    {
@
    LDR     r3, [r2, #8]                        @ Pickup the owning core
    CMP     r3, r1                              @ Is it this core?
    BEQ     _got_lock_after_waiting             @ Yes, the protection is already owned. This means
                                                @ an ISR preempted us and got protection
@
@    }
@
@    /* Are we at the front of the list?  */
@    if (this_core == _tx_thread_smp_protect_wait_list[_tx_thread_smp_protect_wait_list_head])
@    {
@
    LDR     r3, =_tx_thread_smp_protect_wait_list_head @ Get the address of the head
    LDR     r3, [r3]                            @ Get the value of the head
    LDR     r4, =_tx_thread_smp_protect_wait_list @ Get the address of the list
    LDR     r4, [r4, r3, LSL #2]                @ Get the value at the head index

    CMP     r1, r4
    BNE     _did_not_get_lock
@
@    /* Is the lock still available? */
@    if (_tx_thread_smp_protection.tx_thread_smp_protect_in_force == 0)
@    {
@
    LDR     r3, [r2, #0]                        @ Pickup the protection flag
    CMP     r3, #0
    BNE     _did_not_get_lock                   @ No, protection not available
@
@    /* Get the lock.  */
@    _tx_thread_smp_protection.tx_thread_smp_protect_in_force = 1;
@
    MOV     r3, #1                              @ Build lock value
    STR     r3, [r2, #0]                        @ Store lock value
    DMB                                         @
@
@    /* Got the lock.  */
@    _tx_thread_smp_protect_lock_got();
@
    _tx_thread_smp_protect_lock_got
@
@    /* Remove this core from the wait list.  */
@    _tx_thread_smp_protect_remove_from_front_of_list();
@
    _tx_thread_smp_protect_remove_from_front_of_list

    B       _got_lock_after_waiting

_did_not_get_lock:
@
@    /* For one reason or another, we didn't get the lock.  */
@
@    /* Were we removed from the list? This can happen if we're a thread
@       and we got preempted. */
@    if (_tx_thread_smp_protect_wait_counts[this_core] == 0)
@    {
@
    LDR     r3, =_tx_thread_smp_protect_wait_counts @ Load wait list counts
    LDR     r4, [r3, r1, LSL #2]                @ Load waiting value for this core
    CMP     r4, #0
    BNE     _already_in_list1                   @ Is this core already in the list?
@
@    /* Add ourselves to the list.  */
@    _tx_thread_smp_protect_wait_list_add(this_core);
@
    _tx_thread_smp_protect_wait_list_add @ Call macro to add ourselves to the list
@
@    /* Our waiting count was also reset when we were preempted. Increment it again. */
@    _tx_thread_smp_protect_wait_counts[this_core]++;
@
    LDR     r3, =_tx_thread_smp_protect_wait_counts @ Load wait list counts
    LDR     r4, [r3, r1, LSL #2]                @ Load waiting value for this core
    ADD     r4, r4, #1                          @ Increment wait value
    STR     r4, [r3, r1, LSL #2]                @ Store new wait value value
@
@    }
@
_already_in_list1:
@
@    /* Restore interrupts and try again.  */
@
    MSR     CPSR_c, r0                          @ Restore CPSR
#ifdef TX_ENABLE_WFE
    WFE                                         @ Go into standby
#endif
    B       _try_to_get_lock                    @ On waking, restart the protection attempt

_got_lock_after_waiting:
@
@    /* We're no longer waiting.  */
@    _tx_thread_smp_protect_wait_counts[this_core]--;
@
    LDR     r3, =_tx_thread_smp_protect_wait_counts @ Load waiting list
    LDR     r4, [r3, r1, LSL #2]                @ Load current wait value
    SUB     r4, r4, #1                          @ Decrement wait value
    STR     r4, [r3, r1, LSL #2]                @ Store new wait value value

@
@    /* Restore link register and return.  */
@
_return:

    POP     {r4-r6}                             @ Restore registers

#ifdef __THUMB_INTERWORK
    BX      lr                                  @ Return to caller
#else
    MOV     pc, lr                              @ Return to caller
#endif

