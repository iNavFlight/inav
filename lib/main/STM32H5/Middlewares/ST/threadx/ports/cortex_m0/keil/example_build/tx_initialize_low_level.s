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
;
    IMPORT  _tx_thread_system_stack_ptr
    IMPORT  _tx_initialize_unused_memory
    IMPORT  _tx_timer_interrupt
    IMPORT  __main
    IMPORT  |Image$$RO$$Limit|
    IMPORT  |Image$$RW$$Base|
    IMPORT  |Image$$ZI$$Base|
    IMPORT  |Image$$ZI$$Limit|
    IMPORT  __tx_PendSVHandler
;
;
SYSTEM_CLOCK        EQU     6000000
SYSTICK_CYCLES      EQU     ((SYSTEM_CLOCK / 100) -1)
;
;
;/* Setup the stack and heap areas.  */
;
STACK_SIZE          EQU     0x00000400
HEAP_SIZE           EQU     0x00000000

    AREA    STACK, NOINIT, READWRITE, ALIGN=3
StackMem
    SPACE   STACK_SIZE
__initial_sp


    AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
HeapMem
    SPACE   HEAP_SIZE
__heap_limit


    AREA    RESET, CODE, READONLY
;
    EXPORT  __tx_vectors
__tx_vectors
    DCD     __initial_sp                            ; Reset and system stack ptr
    DCD     Reset_Handler                           ; Reset goes to startup function
    DCD     __tx_NMIHandler                         ; NMI
    DCD     __tx_BadHandler                         ; HardFault
    DCD     0                                       ; MemManage
    DCD     0                                       ; BusFault
    DCD     0                                       ; UsageFault
    DCD     0                                       ; 7
    DCD     0                                       ; 8
    DCD     0                                       ; 9
    DCD     0                                       ; 10
    DCD     __tx_SVCallHandler                      ; SVCall
    DCD     __tx_DBGHandler                         ; Monitor
    DCD     0                                       ; 13
    DCD     __tx_PendSVHandler                      ; PendSV
    DCD     __tx_SysTickHandler                     ; SysTick
    DCD     __tx_IntHandler                         ; Int 0
    DCD     __tx_IntHandler                         ; Int 1
    DCD     __tx_IntHandler                         ; Int 2
    DCD     __tx_IntHandler                         ; Int 3
;
;
    AREA ||.text||, CODE, READONLY
    EXPORT  Reset_Handler
Reset_Handler
    CPSID   i
    LDR     R0, =__main
    BX      R0

;/**************************************************************************/
;/*                                                                        */
;/*  FUNCTION                                               RELEASE        */
;/*                                                                        */
;/*    _tx_initialize_low_level                          Cortex-M0/AC5     */
;/*                                                           6.1          */
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
;/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
;/*                                                                        */
;/**************************************************************************/
;VOID   _tx_initialize_low_level(VOID)
;{
    EXPORT  _tx_initialize_low_level
_tx_initialize_low_level
;
;    /* Ensure that interrupts are disabled.  */
;
    CPSID   i                                       ; Disable interrupts
;
;    /* Set base of available memory to end of non-initialised RAM area.  */
;
    LDR     r0, =_tx_initialize_unused_memory       ; Build address of unused memory pointer
    LDR     r1, =|Image$$ZI$$Limit|                 ; Build first free address
    ADDS    r1, r1, #4                              ;
    STR     r1, [r0]                                ; Setup first unused memory pointer
;
;    /* Setup Vector Table Offset Register.  */
;
    LDR     r0, =0xE000ED08                         ; Build address of NVIC registers
    LDR     r1, =__tx_vectors                       ; Pickup address of vector table
    STR     r1, [r0]                                ; Set vector table address
;
;    /* Enable the cycle count register.  */
;
;    LDR     r0, =0xE0001000                         ; Build address of DWT register
;    LDR     r1, [r0]                                ; Pickup the current value
;    MOVS    r2, #1
;    ORRS    r1, r1, r2                              ; Set the CYCCNTENA bit
;    STR     r1, [r0]                                ; Enable the cycle count register
;
;    /* Setup Vector Table Offset Register.  */
;
    LDR     r0, =0xE000E000                         ; Build address of NVIC registers
    LDR     r2, =0xD08                              ; Offset to vector base register
    ADD     r0, r0, r2                              ; Build vector base register
    LDR     r1, =__tx_vectors                       ; Pickup address of vector table
    STR     r1, [r0]                                ; Set vector table address
;
;    /* Set system stack pointer from vector value.  */
;
    LDR     r0, =_tx_thread_system_stack_ptr        ; Build address of system stack pointer
    LDR     r1, =__tx_vectors                       ; Pickup address of vector table
    LDR     r1, [r1]                                ; Pickup reset stack pointer
    STR     r1, [r0]                                ; Save system stack pointer
;
;    /* Configure SysTick.  */
;
    LDR     r0, =0xE000E000                         ; Build address of NVIC registers
    LDR     r1, =SYSTICK_CYCLES
    STR     r1, [r0, #0x14]                         ; Setup SysTick Reload Value
    MOVS    r1, #0x7                                ; Build SysTick Control Enable Value
    STR     r1, [r0, #0x10]                         ; Setup SysTick Control
;
;    /* Configure handler priorities.  */
;
    LDR     r1, =0x00000000                         ; Rsrv, UsgF, BusF, MemM
    LDR     r0, =0xE000E000                         ; Build address of NVIC registers
    LDR     r2, =0xD18                              ;
    ADD     r0, r0, r2                              ;
    STR     r1, [r0]                                ; Setup System Handlers 4-7 Priority Registers

    LDR     r1, =0xFF000000                         ; SVCl, Rsrv, Rsrv, Rsrv
    LDR     r0, =0xE000E000                         ; Build address of NVIC registers
    LDR     r2, =0xD1C                              ;
    ADD     r0, r0, r2                              ;
    STR     r1, [r0]                                ; Setup System Handlers 8-11 Priority Registers
                                                    ; Note: SVC must be lowest priority, which is 0xFF

    LDR     r1, =0x40FF0000                         ; SysT, PnSV, Rsrv, DbgM
    LDR     r0, =0xE000E000                         ; Build address of NVIC registers
    LDR     r2, =0xD20                              ;
    ADD     r0, r0, r2                              ;
    STR     r1, [r0]                                ; Setup System Handlers 12-15 Priority Registers
                                                    ; Note: PnSV must be lowest priority, which is 0xFF
;
;    /* Return to caller.  */
;
    BX      lr
;}
;
;
;/* Define initial heap/stack routine for the ARM RVCT startup code.
;   This routine will set the initial stack and heap locations */
;
    EXPORT  __user_initial_stackheap
__user_initial_stackheap
    LDR     R0, =HeapMem
    LDR     R1, =(StackMem + STACK_SIZE)
    LDR     R2, =(HeapMem + HEAP_SIZE)
    LDR     R3, =StackMem
    BX      LR
;
;
;/* Define shells for each of the unused vectors.  */
;
    EXPORT  __tx_BadHandler
__tx_BadHandler
    B       __tx_BadHandler

    EXPORT  __tx_SVCallHandler
__tx_SVCallHandler
    B       __tx_SVCallHandler

    EXPORT  __tx_IntHandler
__tx_IntHandler
; VOID InterruptHandler (VOID)
; {
    PUSH    {r0, lr}
#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
    BL      _tx_execution_isr_enter             // Call the ISR enter function
#endif
;    /* Do interrupt handler work here */
;    /* .... */
#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
    BL      _tx_execution_isr_exit              // Call the ISR exit function
#endif
    POP     {r0, r1}
    MOV     lr, r1
    BX      lr
; }

    EXPORT  SysTick_Handler
    EXPORT  __tx_SysTickHandler
__tx_SysTickHandler
SysTick_Handler
; VOID TimerInterruptHandler (VOID)
; {
;
    PUSH    {r0, lr}
#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
    BL      _tx_execution_isr_enter             // Call the ISR enter function
#endif
    BL      _tx_timer_interrupt
#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
    BL      _tx_execution_isr_exit              // Call the ISR exit function
#endif
    POP     {r0, r1}
    MOV     lr, r1
    BX      lr
; }

    EXPORT  __tx_NMIHandler
__tx_NMIHandler
    B       __tx_NMIHandler

    EXPORT  __tx_DBGHandler
__tx_DBGHandler
    B       __tx_DBGHandler

    ALIGN
    LTORG
    END
