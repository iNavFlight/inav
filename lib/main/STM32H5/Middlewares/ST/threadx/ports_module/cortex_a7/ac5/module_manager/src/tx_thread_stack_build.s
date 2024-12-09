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
SVC_MODE        EQU     0x13                    ; SVC mode
SYS_MODE        EQU     0x1F                    ; SYS mode
    IF :DEF:TX_ENABLE_FIQ_SUPPORT
CPSR_MASK       EQU     0xDF                    ; Mask initial CPSR, IRQ & FIQ ints enabled
    ELSE
CPSR_MASK       EQU     0x9F                    ; Mask initial CPSR, IRQ ints enabled
    ENDIF

THUMB_MASK      EQU     0x20                    ; Thumb bit (5) of CPSR/SPSR

;
;
        AREA ||.text||, CODE, READONLY
;/**************************************************************************/ 
;/*                                                                        */ 
;/*  FUNCTION                                               RELEASE        */ 
;/*                                                                        */ 
;/*    _tx_thread_stack_build                          Cortex-A7/MMU/AC5   */ 
;/*                                                           6.1          */
;/*  AUTHOR                                                                */
;/*                                                                        */
;/*    Scott Larson, Microsoft Corporation                                 */
;/*                                                                        */
;/*  DESCRIPTION                                                           */ 
;/*                                                                        */ 
;/*    This function builds a stack frame on the supplied thread's stack.  */ 
;/*    The stack frame results in a fake interrupt return to the supplied  */ 
;/*    function pointer.                                                   */ 
;/*                                                                        */ 
;/*  INPUT                                                                 */ 
;/*                                                                        */ 
;/*    thread_ptr                            Pointer to thread control blk */ 
;/*    function_ptr                          Pointer to return function    */ 
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
;/*    _tx_thread_create                     Create thread service         */ 
;/*                                                                        */ 
;/*  RELEASE HISTORY                                                       */ 
;/*                                                                        */ 
;/*    DATE              NAME                      DESCRIPTION             */
;/*                                                                        */
;/*  09-30-2020      Scott Larson            Initial Version 6.1           */
;/*                                                                        */
;/**************************************************************************/
;VOID   _tx_thread_stack_build(TX_THREAD *thread_ptr, VOID (*function_ptr)(VOID))
;{
    EXPORT  _tx_thread_stack_build
_tx_thread_stack_build
;
;       
;    /* Build a fake interrupt frame.  The form of the fake interrupt stack
;       on the Cortex-A7 should look like the following after it is built:
;       
;       Stack Top:      1           Interrupt stack frame type
;                       CPSR        Initial value for CPSR
;                       a1 (r0)     Initial value for a1
;                       a2 (r1)     Initial value for a2
;                       a3 (r2)     Initial value for a3
;                       a4 (r3)     Initial value for a4
;                       v1 (r4)     Initial value for v1
;                       v2 (r5)     Initial value for v2
;                       v3 (r6)     Initial value for v3
;                       v4 (r7)     Initial value for v4
;                       v5 (r8)     Initial value for v5
;                       sb (r9)     Initial value for sb
;                       sl (r10)    Initial value for sl
;                       fp (r11)    Initial value for fp
;                       ip (r12)    Initial value for ip
;                       lr (r14)    Initial value for lr
;                       pc (r15)    Initial value for pc
;                       0           For stack backtracing
;
;    Stack Bottom: (higher memory address)  */
;
    LDR     r2, [r0, #16]                       ; Pickup end of stack area
    BIC     r2, r2, #7                          ; Ensure 8-byte alignment
    SUB     r2, r2, #76                         ; Allocate space for the stack frame
;
;    /* Actually build the stack frame.  */
;
    MOV     r3, #1                              ; Build interrupt stack type
    STR     r3, [r2, #0]                        ; Store stack type
    MOV     r3, #0                              ; Build initial register value
    STR     r3, [r2, #8]                        ; Store initial r0
    STR     r3, [r2, #12]                       ; Store initial r1
    STR     r3, [r2, #16]                       ; Store initial r2
    STR     r3, [r2, #20]                       ; Store initial r3
    STR     r3, [r2, #24]                       ; Store initial r4
    STR     r3, [r2, #28]                       ; Store initial r5
    STR     r3, [r2, #32]                       ; Store initial r6
    STR     r3, [r2, #36]                       ; Store initial r7
    STR     r3, [r2, #40]                       ; Store initial r8
    STR     r3, [r2, #44]                       ; Store initial r9
    LDR     r3, [r0, #12]                       ; Pickup stack starting address
    STR     r3, [r2, #48]                       ; Store initial r10 (sl)
    MOV     r3, #0                              ; Build initial register value
    STR     r3, [r2, #52]                       ; Store initial r11
    STR     r3, [r2, #56]                       ; Store initial r12
    STR     r3, [r2, #60]                       ; Store initial lr
    STR     r1, [r2, #64]                       ; Store initial pc
    STR     r3, [r2, #68]                       ; 0 for back-trace

    MRS     r3, CPSR                            ; Pickup CPSR
    BIC     r3, r3, #CPSR_MASK                  ; Mask mode bits of CPSR
    ORR     r3, r3, #SYS_MODE                   ; Build CPSR, SYS mode, interrupts enabled
    BIC     r3, r3, #THUMB_MASK                 ; Clear Thumb bit by default
    AND     r1, r1, #1                          ; Determine if the entry function is in Thumb mode
    CMP     r1, #1                              ; Is the Thumb bit set?
    ORREQ   r3, r3, #THUMB_MASK                 ; Yes, set the Thumb bit
    STR     r3, [r2, #4]                        ; Store initial CPSR
;
;    /* Setup stack pointer.  */
;    thread_ptr -> tx_thread_stack_ptr =  r2;
;
    STR     r2, [r0, #8]                        ; Save stack pointer in thread's
                                                ;   control block
    IF  {INTER} = {TRUE}
    BX      lr                                  ; Return to caller
    ELSE
    MOV     pc, lr                              ; Return to caller
    ENDIF

;}
    END

