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
    .global    _tx_thread_system_state
    .global    _tx_thread_current_ptr
    .global    _tx_thread_smp_release_cores_flag
    .global    _tx_thread_schedule

    .arm
    .text
    .align 2
@/**************************************************************************/ 
@/*                                                                        */ 
@/*  FUNCTION                                               RELEASE        */ 
@/*                                                                        */ 
@/*    _tx_thread_smp_initialize_wait                  SMP/Cortex-A7/GNU   */ 
@/*                                                            6.1         */
@/*  AUTHOR                                                                */
@/*                                                                        */
@/*    William E. Lamie, Microsoft Corporation                             */
@/*                                                                        */
@/*  DESCRIPTION                                                           */
@/*                                                                        */ 
@/*    This function is the place where additional cores wait until        */ 
@/*    initialization is complete before they enter the thread scheduling  */ 
@/*    loop.                                                               */ 
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
@/*    _tx_thread_schedule                   Thread scheduling loop        */
@/*                                                                        */ 
@/*  CALLED BY                                                             */ 
@/*                                                                        */ 
@/*    Hardware                                                            */ 
@/*                                                                        */ 
@/*  RELEASE HISTORY                                                       */ 
@/*                                                                        */ 
@/*    DATE              NAME                      DESCRIPTION             */
@/*                                                                        */
@/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
@/*                                                                        */
@/**************************************************************************/
    .global _tx_thread_smp_initialize_wait
    .type   _tx_thread_smp_initialize_wait,function
_tx_thread_smp_initialize_wait:

@    /* Lockout interrupts.  */
@
#ifdef TX_ENABLE_FIQ_SUPPORT
    CPSID   if                                  @ Disable IRQ and FIQ interrupts
#else
    CPSID   i                                   @ Disable IRQ interrupts
#endif
@
@    /* Pickup the CPU ID.   */
@
    MRC     p15, 0, r10, c0, c0, 5              @ Read CPU ID register
    AND     r10, r10, #0x03                     @ Mask off, leaving the CPU ID field
    LSL     r10, r10, #2                        @ Build offset to array indexes
@
@    /* Make sure the system state for this core is TX_INITIALIZE_IN_PROGRESS before we check the release 
@       flag.  */
@
    LDR     r3, =_tx_thread_system_state        @ Build address of system state variable
    ADD     r3, r3, r10                         @ Build index into the system state array
    LDR     r2, =0xF0F0F0F0                     @ Build TX_INITIALIZE_IN_PROGRESS flag
wait_for_initialize:
    LDR     r1, [r3]                            @ Pickup system state
    CMP     r1, r2                              @ Has initialization completed?
    BNE     wait_for_initialize                 @ If different, wait here!
@
@    /* Pickup the release cores flag.  */
@
    LDR     r2, =_tx_thread_smp_release_cores_flag  @ Build address of release cores flag

wait_for_release:
    LDR     r3, [r2]                            @ Pickup the flag
    CMP     r3, #0                              @ Is it set?
    BEQ     wait_for_release                    @ Wait for the flag to be set 
@
@    /* Core 0 has released this core.  */
@    
@    /* Clear this core's system state variable.  */
@
    LDR     r3, =_tx_thread_system_state        @ Build address of system state variable
    ADD     r3, r3, r10                         @ Build index into the system state array
    MOV     r0, #0                              @ Build clear value
    STR     r0, [r3]                            @ Clear this core's entry in the system state array
@
@    /* Now wait for core 0 to finish it's initialization.  */
@
    LDR     r3, =_tx_thread_system_state        @ Build address of system state variable of logical 0 

core_0_wait_loop:
    LDR     r2, [r3]                            @ Pickup system state for core 0
    CMP     r2, #0                              @ Is it 0?
    BNE     core_0_wait_loop                    @ No, keep waiting for core 0 to finish its initialization
@
@    /* Initialize is complete, enter the scheduling loop!  */
@
    B       _tx_thread_schedule                 @ Enter scheduling loop for this core!



