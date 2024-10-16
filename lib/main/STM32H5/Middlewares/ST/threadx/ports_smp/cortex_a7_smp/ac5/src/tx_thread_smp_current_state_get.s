;/**************************************************************************/
;/*                                                                        */
;/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
;/*                                                                        */
;/*       This software is licensed under the Microsoft Software License   */
;/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
;/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
;/*       and in the root directory of this software.                      */
;/*                                                                        */
;/**************************************************************************/
;
;
;/**************************************************************************/
;/**************************************************************************/
;/**                                                                       */ 
;/** ThreadX Component                                                     */ 
;/**                                                                       */
;/**   Thread - Low Level SMP Support                                      */
;/**                                                                       */
;/**************************************************************************/
;/**************************************************************************/
;
;
;#define TX_SOURCE_CODE
;#define TX_THREAD_SMP_SOURCE_CODE
;
;/* Include necessary system files.  */
;
;#include "tx_api.h"
;#include "tx_thread.h"
;#include "tx_timer.h"  */
;
;
    IMPORT     _tx_thread_system_state

        AREA ||.text||, CODE, READONLY
        PRESERVE8
;/**************************************************************************/ 
;/*                                                                        */ 
;/*  FUNCTION                                               RELEASE        */ 
;/*                                                                        */ 
;/*    _tx_thread_smp_current_state_get                SMP/Cortex-A7/AC5   */
;/*                                                            6.1         */
;/*  AUTHOR                                                                */
;/*                                                                        */
;/*    William E. Lamie, Microsoft Corporation                             */
;/*                                                                        */
;/*  DESCRIPTION                                                           */
;/*                                                                        */ 
;/*    This function is gets the current state of the calling core.        */ 
;/*                                                                        */ 
;/*  INPUT                                                                 */ 
;/*                                                                        */ 
;/*    None                                                                */
;/*                                                                        */ 
;/*  OUTPUT                                                                */ 
;/*                                                                        */ 
;/*    None                                                                */
;/*                                                                        */ 
;/*  CALLS                                                                 */ 
;/*                                                                        */ 
;/*    None                                                                */
;/*                                                                        */ 
;/*  CALLED BY                                                             */ 
;/*                                                                        */ 
;/*    ThreadX Components                                                  */ 
;/*                                                                        */ 
;/*  RELEASE HISTORY                                                       */ 
;/*                                                                        */ 
;/*    DATE              NAME                      DESCRIPTION             */
;/*                                                                        */
;/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
;/*                                                                        */
;/**************************************************************************/
    EXPORT  _tx_thread_smp_current_state_get
_tx_thread_smp_current_state_get

    MRS     r3, CPSR                            ; Pickup current CPSR

    IF  :DEF:TX_ENABLE_FIQ_SUPPORT
    CPSID   if                                  ; Disable IRQ and FIQ interrupts
    ELSE
    CPSID   i                                   ; Disable IRQ interrupts
    ENDIF
;
;    /* Pickup the CPU ID.   */
;
    MRC     p15, 0, r2, c0, c0, 5               ; Read CPU ID register
    AND     r2, r2, #0x03                       ; Mask off, leaving the CPU ID field
    LSL     r2, r2, #2                          ; Build offset to array indexes

    LDR     r1, =_tx_thread_system_state        ; Pickup start of the current state array
    ADD     r1, r1, r2                          ; Build index into the current state array
    LDR     r0, [r1]                            ; Pickup state for this core
    MSR     CPSR_c, r3                          ; Restore CPSR 
    IF  {INTER} = {TRUE}
    BX      lr                                  ; Return to caller
    ELSE
    MOV     pc, lr                              ; Return to caller
    ENDIF

    END

