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
;/**   Initialize                                                          */
;/**                                                                       */
;/**************************************************************************/
;/**************************************************************************/
;
;#define TX_SOURCE_CODE
;
;
;/* Include necessary system files.  */
;
;#include "tx_api.h"
;#include "tx_initialize.h"
;#include "tx_thread.h"
;#include "tx_timer.h"
;
;
        EXTERN  _tx_thread_system_stack_ptr
        EXTERN  _tx_initialize_unused_memory
        EXTERN  _tx_timer_interrupt
        EXTERN  __vector_table
        EXTERN  _tx_execution_isr_enter
        EXTERN  _tx_execution_isr_exit
;
;
SYSTEM_CLOCK      EQU   7200000
SYSTICK_CYCLES    EQU   ((SYSTEM_CLOCK / 100) -1)
        
    RSEG    FREE_MEM:DATA
    PUBLIC  __tx_free_memory_start
__tx_free_memory_start
    DS32    4        
;
;
        SECTION `.text`:CODE:NOROOT(2)
        THUMB
;/**************************************************************************/
;/*                                                                        */
;/*  FUNCTION                                               RELEASE        */
;/*                                                                        */
;/*    _tx_initialize_low_level                          Cortex-M0+/IAR     */
;/*                                                           6.1.10       */
;/*  AUTHOR                                                                */
;/*                                                                        */
;/*    William E. Lamie, Microsoft Corporation                             */
;/*                                                                        */
;/*  DESCRIPTION                                                           */
;/*                                                                        */
;/*    This function is responsible for any low-level processor            */
;/*    initialization, including setting up interrupt vectors, setting     */
;/*    up a periodic timer interrupt source, saving the system stack       */
;/*    pointer for use in ISR processing later, and finding the first      */
;/*    available RAM memory address for tx_application_define.             */
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
;/*    _tx_initialize_kernel_enter           ThreadX entry function        */
;/*                                                                        */
;/*  RELEASE HISTORY                                                       */
;/*                                                                        */
;/*    DATE              NAME                      DESCRIPTION             */
;/*                                                                        */
/*  01-31-2022      Scott Larson            Initial Version 6.1.10        */
;/*                                                                        */
;/**************************************************************************/
;VOID   _tx_initialize_low_level(VOID)
;{
    PUBLIC  _tx_initialize_low_level
_tx_initialize_low_level:
    /* Disable interrupts during ThreadX initialization.  */

    CPSID   i

    /* Set base of available memory to end of non-initialised RAM area.  */

    LDR     r0, =_tx_initialize_unused_memory       // Build address of unused memory pointer
    LDR     r1, =__tx_free_memory_start             // Build first free address
    ADDS    r1, r1, #4                              //
    STR     r1, [r0]                                // Setup first unused memory pointer

    /* Setup Vector Table Offset Register.  */

    LDR     r0, =0xE000ED08                         // Build address of NVIC registers
    LDR     r1, =__vector_table                     // Pickup address of vector table
    STR     r1, [r0]                                // Set vector table address

    /* Set system stack pointer from vector value.  */

    LDR     r0, =_tx_thread_system_stack_ptr        // Build address of system stack pointer
    LDR     r1, =__vector_table                     // Pickup address of vector table
    LDR     r1, [r1]                                // Pickup reset stack pointer
    STR     r1, [r0]                                // Save system stack pointer

    /* Enable the cycle count register.  */

    LDR     r0, =0xE0001000                         // Build address of DWT register
    LDR     r1, [r0]                                // Pickup the current value
    MOVS    r2, #1
    ORRS    r1, r1, r2                              // Set the CYCCNTENA bit
    STR     r1, [r0]                                // Enable the cycle count register

    /* Configure SysTick for 100Hz clock, or 16384 cycles if no reference.  */

    LDR     r0, =0xE000E000                         // Build address of NVIC registers
    LDR     r1, =SYSTICK_CYCLES
    MOVS    r2, #0x14
    STR     r1, [r0, r2]                         // Setup SysTick Reload Value
    MOVS    r1, #0x7                                // Build SysTick Control Enable Value
    STR     r1, [r0, #0x10]                         // Setup SysTick Control

    /* Configure handler priorities.  */

    LDR     r1, =0x00000000                         // Rsrv, UsgF, BusF, MemM
    LDR     r2, =0xD18
    STR     r1, [r0, r2]                            // Setup System Handlers 4-7 Priority Registers

    LDR     r1, =0xFF000000                         // SVCl, Rsrv, Rsrv, Rsrv
    LDR     r2, =0xD1C
    STR     r1, [r0, r2]                            // Setup System Handlers 8-11 Priority Registers
                                                    // Note: SVC must be lowest priority, which is 0xFF

    LDR     r1, =0x40FF0000                         // SysT, PnSV, Rsrv, DbgM
    LDR     r2, =0xD20
    STR     r1, [r0, r2]                            // Setup System Handlers 12-15 Priority Registers
                                                    // Note: PnSV must be lowest priority, which is 0xFF

    /* Return to caller.  */

    BX      lr
// }
;
;/* Define SystTick Handler.  */
;

    PUBLIC  SysTick_Handler
    PUBLIC  __tx_SysTickHandler
SysTick_Handler:
__tx_SysTickHandler:
; VOID SysTickHandler (VOID)
; {
;
    PUSH    {r0, lr}
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    BL      _tx_execution_isr_enter             ; Call the ISR enter function
#endif
    BL      _tx_timer_interrupt
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    BL      _tx_execution_isr_exit              ; Call the ISR exit function
#endif
    POP     {r0, r1}
    MOV     lr, r1
    BX      lr
; }
        
    END
                

