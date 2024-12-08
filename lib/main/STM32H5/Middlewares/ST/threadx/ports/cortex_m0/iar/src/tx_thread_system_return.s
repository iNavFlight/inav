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
    SECTION `.text`:CODE:NOROOT(2)
    THUMB
;/**************************************************************************/
;/*                                                                        */
;/*  FUNCTION                                               RELEASE        */
;/*                                                                        */
;/*    _tx_thread_system_return                          Cortex-M0/IAR     */
;/*                                                           6.1          */
;/*  AUTHOR                                                                */
;/*                                                                        */
;/*    William E. Lamie, Microsoft Corporation                             */
;/*                                                                        */
;/*  DESCRIPTION                                                           */
;/*                                                                        */
;/*    This function is target processor specific.  It is used to transfer */
;/*    control from a thread back to the ThreadX system.  Only a           */
;/*    minimal context is saved since the compiler assumes temp registers  */
;/*    are going to get slicked by a function call anyway.                 */
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
;/*    _tx_thread_schedule                   Thread scheduling loop        */
;/*                                                                        */
;/*  CALLED BY                                                             */
;/*                                                                        */
;/*    ThreadX components                                                  */
;/*                                                                        */
;/*  RELEASE HISTORY                                                       */
;/*                                                                        */
;/*    DATE              NAME                      DESCRIPTION             */
;/*                                                                        */
;/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
;/*                                                                        */
;/**************************************************************************/
;VOID   _tx_thread_system_return(VOID)
;{
    PUBLIC  _tx_thread_system_return
_tx_thread_system_return??rA:
_tx_thread_system_return:
;
;    /* Return to real scheduler via PendSV. Note that this routine is often
;       replaced with in-line assembly in tx_port.h to improved performance.  */
;
    LDR     r0, =0x10000000                         ; Load PENDSVSET bit
    LDR     r1, =0xE000ED04                         ; Load NVIC base
    STR     r0, [r1]                                ; Set PENDSVBIT in ICSR
    MRS     r0, IPSR                                ; Pickup IPSR
    CMP     r0, #0                                  ; Is it a thread returning?
    BNE     _isr_context                            ; If ISR, skip interrupt enable
    MRS     r1, PRIMASK                             ; Thread context returning, pickup PRIMASK
    CPSIE   i                                       ; Enable interrupts
    MSR     PRIMASK, r1                             ; Restore original interrupt posture
_isr_context:
    BX      lr                                      ; Return to caller
;}
    END
