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
;#include "tx_timer.h"
;
#ifdef TX_ENABLE_FIQ_SUPPORT
DISABLE_INTS    DEFINE  0xC0                    ; Disable IRQ & FIQ interrupts
#else
DISABLE_INTS    DEFINE  0x80                    ; Disable IRQ interrupts
#endif
MODE_MASK       DEFINE  0x1F                    ; Mode mask 
FIQ_MODE_BITS   DEFINE  0x11                    ; FIQ mode bits
;
;
;/**************************************************************************/ 
;/*                                                                        */ 
;/*  FUNCTION                                               RELEASE        */ 
;/*                                                                        */ 
;/*    _tx_thread_fiq_nesting_end                         Cortex-A7/IAR    */ 
;/*                                                           6.1          */
;/*  AUTHOR                                                                */
;/*                                                                        */
;/*    William E. Lamie, Microsoft Corporation                             */
;/*                                                                        */
;/*  DESCRIPTION                                                           */
;/*                                                                        */ 
;/*    This function is called by the application from FIQ mode after      */ 
;/*    _tx_thread_fiq_nesting_start has been called and switches the FIQ   */ 
;/*    processing from system mode back to FIQ mode prior to the ISR       */ 
;/*    calling _tx_thread_fiq_context_restore.  Note that this function    */ 
;/*    assumes the system stack pointer is in the same position after      */ 
;/*    nesting start function was called.                                  */ 
;/*                                                                        */ 
;/*    This function assumes that the system mode stack pointer was setup  */ 
;/*    during low-level initialization (tx_initialize_low_level.s79).      */ 
;/*                                                                        */ 
;/*    This function returns with FIQ interrupts disabled.                 */ 
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
;VOID   _tx_thread_fiq_nesting_end(VOID)
;{
    RSEG    .text:CODE:NOROOT(2)
    PUBLIC  _tx_thread_fiq_nesting_end
    CODE32
_tx_thread_fiq_nesting_end
    MOV     r3,lr                               ; Save ISR return address
    MRS     r0, CPSR                            ; Pickup the CPSR
    ORR     r0, r0, #DISABLE_INTS               ; Build disable interrupt value
    MSR     CPSR_cxsf, r0                       ; Disable interrupts
    LDR     lr, [sp]                            ; Pickup saved lr
    ADD     sp, sp, #4                          ; Adjust stack pointer
    BIC     r0, r0, #MODE_MASK                  ; Clear mode bits
    ORR     r0, r0, #FIQ_MODE_BITS              ; Build IRQ mode CPSR
    MSR     CPSR_cxsf, r0                       ; Re-enter IRQ mode
    MOV     pc, r3                              ; Return to ISR
;}
;
;
    END

