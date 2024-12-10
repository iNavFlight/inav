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
;/**   Thread                                                              */
;/**                                                                       */
;/**************************************************************************/
;/**************************************************************************/
;
;
;#define TX_SOURCE_CODE
;
;
;/* Include necessary system files.  */
;
;#include "tx_api.h"
;#include "tx_thread.h"
;
;
FIQ_DISABLE     EQU     0x40                    ; FIQ disable bit
MODE_MASK       EQU     0x1F                    ; Mode mask
SYS_MODE_BITS   EQU     0x1F                    ; System mode bits
;
;
        AREA ||.text||, CODE, READONLY
;/**************************************************************************/ 
;/*                                                                        */ 
;/*  FUNCTION                                               RELEASE        */ 
;/*                                                                        */ 
;/*    _tx_thread_fiq_nesting_start                       Cortex-A7/AC5    */ 
;/*                                                           6.1          */
;/*  AUTHOR                                                                */
;/*                                                                        */
;/*    William E. Lamie, Microsoft Corporation                             */
;/*                                                                        */
;/*  DESCRIPTION                                                           */
;/*                                                                        */ 
;/*    This function is called by the application from FIQ mode after      */ 
;/*    _tx_thread_fiq_context_save has been called and switches the FIQ    */ 
;/*    processing to the system mode so nested FIQ interrupt processing    */ 
;/*    is possible (system mode has its own "lr" register).  Note that     */ 
;/*    this function assumes that the system mode stack pointer was setup  */ 
;/*    during low-level initialization (tx_initialize_low_level.s).        */ 
;/*                                                                        */ 
;/*    This function returns with FIQ interrupts enabled.                  */ 
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
;/*    ISRs                                                                */ 
;/*                                                                        */ 
;/*  RELEASE HISTORY                                                       */ 
;/*                                                                        */ 
;/*    DATE              NAME                      DESCRIPTION             */
;/*                                                                        */
;/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
;/*                                                                        */
;/**************************************************************************/
;VOID   _tx_thread_fiq_nesting_start(VOID)
;{
    EXPORT  _tx_thread_fiq_nesting_start
_tx_thread_fiq_nesting_start
    MOV     r3,lr                               ; Save ISR return address
    MRS     r0, CPSR                            ; Pickup the CPSR
    BIC     r0, r0, #MODE_MASK                  ; Clear the mode bits
    ORR     r0, r0, #SYS_MODE_BITS              ; Build system mode CPSR
    MSR     CPSR_c, r0                          ; Enter system mode
    STMDB   sp!, {r1, lr}                       ; Push the system mode lr on the system mode stack
                                                ;   and push r1 just to keep 8-byte alignment
    BIC     r0, r0, #FIQ_DISABLE                ; Build enable FIQ CPSR
    MSR     CPSR_c, r0                          ; Enter system mode
    IF  {INTER} = {TRUE}
    BX      r3                                  ; Return to caller
    ELSE
    MOV     pc, r3                              ; Return to caller
    ENDIF
;}
;
    END

