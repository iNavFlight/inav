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
@/**   Timer                                                               */
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
@#include "tx_timer.h"
@#include "tx_thread.h"
@
@
@Define Assembly language external references...
@
    .global     _tx_timer_time_slice
    .global     _tx_timer_system_clock
    .global     _tx_timer_current_ptr
    .global     _tx_timer_list_start
    .global     _tx_timer_list_end
    .global     _tx_timer_expired_time_slice
    .global     _tx_timer_expired
    .global     _tx_thread_time_slice
    .global     _tx_timer_expiration_process
    .global     _tx_timer_interrupt_active
    .global     _tx_thread_smp_protect
    .global     _tx_thread_smp_unprotect
    .global     _tx_trace_isr_enter_insert
    .global     _tx_trace_isr_exit_insert
@
@
    .arm
    .text
    .align 2
@/**************************************************************************/ 
@/*                                                                        */ 
@/*  FUNCTION                                               RELEASE        */ 
@/*                                                                        */ 
@/*    _tx_timer_interrupt                             SMP/Cortex-A7/GNU   */ 
@/*                                                            6.1         */
@/*  AUTHOR                                                                */
@/*                                                                        */
@/*    William E. Lamie, Microsoft Corporation                             */
@/*                                                                        */
@/*  DESCRIPTION                                                           */
@/*                                                                        */ 
@/*    This function processes the hardware timer interrupt.  This         */ 
@/*    processing includes incrementing the system clock and checking for  */ 
@/*    time slice and/or timer expiration.  If either is found, the        */ 
@/*    interrupt context save/restore functions are called along with the  */ 
@/*    expiration functions.                                               */ 
@/*                                                                        */ 
@/*  INPUT                                                                 */ 
@/*                                                                        */ 
@/*    None                                                                */ 
@/*                                                                        */ 
@/*  OUTPUT                                                                */ 
@/*                                                                        */ 
@/*    None                                                                */ 
@/*                                                                        */ 
@/*  CALLS                                                                 */ 
@/*                                                                        */ 
@/*    _tx_thread_time_slice                 Time slice interrupted thread */ 
@/*    _tx_thread_smp_protect                Get SMP protection            */ 
@/*    _tx_thread_smp_unprotect              Releast SMP protection        */ 
@/*    _tx_timer_expiration_process          Timer expiration processing   */ 
@/*                                                                        */ 
@/*  CALLED BY                                                             */ 
@/*                                                                        */ 
@/*    interrupt vector                                                    */ 
@/*                                                                        */ 
@/*  RELEASE HISTORY                                                       */ 
@/*                                                                        */ 
@/*    DATE              NAME                      DESCRIPTION             */
@/*                                                                        */
@/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
@/*                                                                        */
@/**************************************************************************/
@VOID   _tx_timer_interrupt(VOID)
@{
    .global _tx_timer_interrupt
    .type   _tx_timer_interrupt,function
_tx_timer_interrupt:
@
@    /* Upon entry to this routine, it is assumed that context save has already
@       been called, and therefore the compiler scratch registers are available
@       for use.  */
@
    MRC     p15, 0, r0, c0, c0, 5               @ Read CPU ID register
    AND     r0, r0, #0x03                       @ Mask off, leaving the CPU ID field
    CMP     r0, #0                              @ Only process timer interrupts from core 0 (to change this simply change the constant!)
    BEQ     __tx_process_timer                  @ If the same process the interrupt
    BX      lr                                  @ Return to caller if not matched
__tx_process_timer:

    STMDB   sp!, {r4, lr}                       @ Save the lr and r4 register on the stack
    BL      _tx_thread_smp_protect              @ Get protection
    MOV     r4, r0                              @ Save the return value in preserved register

    LDR     r1, =_tx_timer_interrupt_active     @ Pickup address of timer interrupt active count
    LDR     r0, [r1, #0]                        @ Pickup interrupt active count
    ADD     r0, r0, #1                          @ Increment interrupt active count
    STR     r0, [r1, #0]                        @ Store new interrupt active count
    DMB                                         @ Ensure that accesses to shared resource have completed
@
@    /* Increment the system clock.  */
@    _tx_timer_system_clock++;
@
    LDR     r1, =_tx_timer_system_clock         @ Pickup address of system clock
    LDR     r0, [r1, #0]                        @ Pickup system clock
    ADD     r0, r0, #1                          @ Increment system clock
    STR     r0, [r1, #0]                        @ Store new system clock
@
@    /* Test for timer expiration.  */
@    if (*_tx_timer_current_ptr)
@    {
@
    LDR     r1, =_tx_timer_expired              @ Pickup addr of expired flag
    LDR     r0, [r1, #0]                        @ Pickup timer expired flag
    CMP     r0, #0                              @ Check for previous timer expiration still active
    BNE     __tx_timer_done                     @ If so, skip timer processing
    LDR     r1, =_tx_timer_current_ptr          @ Pickup current timer pointer addr
    LDR     r0, [r1, #0]                        @ Pickup current timer
    LDR     r2, [r0, #0]                        @ Pickup timer list entry
    CMP     r2, #0                              @ Is there anything in the list?
    BEQ     __tx_timer_no_timer                 @ No, just increment the timer
@
@        /* Set expiration flag.  */
@        _tx_timer_expired =  TX_TRUE;
@
    LDR     r3, =_tx_timer_expired              @ Pickup expiration flag address
    MOV     r2, #1                              @ Build expired value
    STR     r2, [r3, #0]                        @ Set expired flag
    B       __tx_timer_done                     @ Finished timer processing
@
@    }
@    else
@    {
__tx_timer_no_timer:
@
@        /* No timer expired, increment the timer pointer.  */
@        _tx_timer_current_ptr++;
@
    ADD     r0, r0, #4                          @ Move to next timer
@
@        /* Check for wrap-around.  */
@        if (_tx_timer_current_ptr == _tx_timer_list_end)
@
    LDR     r3, =_tx_timer_list_end             @ Pickup addr of timer list end
    LDR     r2, [r3, #0]                        @ Pickup list end
    CMP     r0, r2                              @ Are we at list end?
    BNE     __tx_timer_skip_wrap                @ No, skip wrap-around logic
@
@            /* Wrap to beginning of list.  */
@            _tx_timer_current_ptr =  _tx_timer_list_start;
@
    LDR     r3, =_tx_timer_list_start           @ Pickup addr of timer list start
    LDR     r0, [r3, #0]                        @ Set current pointer to list start
@
__tx_timer_skip_wrap:
@
    STR     r0, [r1, #0]                        @ Store new current timer pointer
@    }
@
__tx_timer_done:
@
@
@    /* Did a timer expire?  */
@    if (_tx_timer_expired)
@    {
@
    LDR     r1, =_tx_timer_expired              @ Pickup addr of expired flag
    LDR     r0, [r1, #0]                        @ Pickup timer expired flag
    CMP     r0, #0                              @ Check for timer expiration
    BEQ     __tx_timer_dont_activate            @ If not set, skip timer activation
@
@        /* Process timer expiration.  */
@        _tx_timer_expiration_process();
@
    BL      _tx_timer_expiration_process        @ Call the timer expiration handling routine
@
@    }
__tx_timer_dont_activate:
@
@    /* Call time-slice processing.  */
@    _tx_thread_time_slice(); 

    BL      _tx_thread_time_slice               @ Call time-slice processing
@
@    }
@
    LDR     r1, =_tx_timer_interrupt_active     @ Pickup address of timer interrupt active count
    LDR     r0, [r1, #0]                        @ Pickup interrupt active count
    SUB     r0, r0, #1                          @ Decrement interrupt active count
    STR     r0, [r1, #0]                        @ Store new interrupt active count
    DMB                                         @ Ensure that accesses to shared resource have completed
@
@    /* Release protection.  */
@
    MOV     r0, r4                              @ Pass the previous status register back
    BL      _tx_thread_smp_unprotect            @ Release protection

    LDMIA   sp!, {r4, lr}                       @ Recover lr register and r4
#ifdef __THUMB_INTERWORK
    BX      lr                                  @ Return to caller
#else
    MOV     pc, lr                              @ Return to caller
#endif
@
@}


