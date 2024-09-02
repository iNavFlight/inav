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

THUMB_MASK      EQU     0x20                    ; Thumb bit (5) of CPSR/SPSR
FIQ_MODE        EQU     0x11                    ; FIQ mode
IRQ_MODE        EQU     0x12                    ; IRQ mode
SVC_MODE        EQU     0x13                    ; SVC mode
ABT_MODE        EQU     0x17                    ; ABT mode
SYS_MODE        EQU     0x1F                    ; SYS mode

HEAP_SIZE       EQU     4096                    ; Heap size
FIQ_STACK_SIZE  EQU     512                     ; FIQ stack size
SYS_STACK_SIZE  EQU     1024                    ; SYS stack size
IRQ_STACK_SIZE  EQU     1024                    ; IRQ stack size
SVC_STACK_SIZE  EQU     512                     ; SVC stack size
ABT_STACK_SIZE  EQU     512                     ; ABT stack size

VFPEnable       EQU     0x40000000              ; VFP enable value

;
;
    IMPORT      __tx_swi_interrupt
    IMPORT      _tx_thread_system_stack_ptr
    IMPORT      _tx_initialize_unused_memory
    IMPORT      _tx_thread_context_save
    IMPORT      _tx_thread_context_restore
    IF  :DEF:TX_ENABLE_FIQ_SUPPORT
    IMPORT      _tx_thread_fiq_context_save
    IMPORT      _tx_thread_fiq_context_restore
    ENDIF
    IF  :DEF:TX_ENABLE_IRQ_NESTING
    IMPORT      _tx_thread_irq_nesting_start
    IMPORT      _tx_thread_irq_nesting_end
    ENDIF
    IF  :DEF:TX_ENABLE_FIQ_NESTING
    IMPORT      _tx_thread_fiq_nesting_start
    IMPORT      _tx_thread_fiq_nesting_end
    ENDIF
    IMPORT      _tx_timer_interrupt
    IMPORT      __main
    IMPORT      _tx_version_id
    IMPORT      _tx_build_options
    IMPORT      |Image$$ZI$$Limit|
;
;
    AREA  VECTORS, CODE, READONLY
    PRESERVE8
;
;/* Define the default Cortex-A7 vector area.  This should be located or copied to 0.  */
;
    EXPORT  __vectors
__vectors
    LDR     pc,=Reset_Vector                    ; Reset goes to startup function
    LDR     pc,=__tx_undefined                  ; Undefined handler
    LDR     pc,=__tx_swi_interrupt              ; Software interrupt handler
    LDR     pc,=__tx_prefetch_handler           ; Prefetch exception handler
    LDR     pc,=__tx_abort_handler              ; Abort exception handler
    LDR     pc,=__tx_reserved_handler           ; Reserved exception handler
    LDR     pc,=__tx_irq_handler                ; IRQ interrupt handler
    LDR     pc,=__tx_fiq_handler                ; FIQ interrupt handler
;
;
        EXPORT  Reset_Vector
Reset_Vector

    IF  {TARGET_FPU_VFP} = {TRUE}
    MRC     p15, 0, r1, c1, c0, 2               ; r1 = Access Control Register
    ORR     r1, r1, #(0xf << 20)                ; Enable full access for p10,11
    MCR     p15, 0, r1, c1, c0, 2               ; Access Control Register = r1
    MOV     r1, #0
    MCR     p15, 0, r1, c7, c5, 4               ; Flush prefetch buffer because of FMXR below and
                                                ; CP 10 & 11 were only just enabled
    MOV     r0, #VFPEnable                      ; Enable VFP itself
    FMXR    FPEXC, r0                           ; FPEXC = r0
    ENDIF

    B       __main
;
;
        AREA ||.text||, CODE, READONLY
;/**************************************************************************/ 
;/*                                                                        */ 
;/*  FUNCTION                                               RELEASE        */ 
;/*                                                                        */ 
;/*    _tx_initialize_low_level                        Cortex-A7/MMU/AC5   */ 
;/*                                                           6.1          */
;/*  AUTHOR                                                                */
;/*                                                                        */
;/*    Scott Larson, Microsoft Corporation                                 */
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
;/*  09-30-2020      Scott Larson            Initial Version 6.1           */
;/*                                                                        */
;/**************************************************************************/
;VOID   _tx_initialize_low_level(VOID)
;{
    EXPORT  _tx_initialize_low_level
_tx_initialize_low_level
;
;
;    /****** NOTE ****** We must be in SVC MODE at this point.  Some monitors
;       enter this routine in USER mode and require a software interrupt to
;       change into SVC mode.  */
;

    ; Set vector table.
    LDR     r0, = __vectors
    MCR     p15, 0, r0, c12, c0, 0


    LDR     r1, =|Image$$ZI$$Limit|             ; Get end of non-initialized RAM area
    LDR     r2, =HEAP_SIZE                      ; Pickup the heap size
    ADD     r1, r2, r1                          ; Setup heap limit
    ADD     r1, r1, #4                          ; Setup stack limit
    MOV     r3, lr                              ; Save LR

    LDR     r2, =SYS_STACK_SIZE                 ; Pickup stack size
    CPS     #SYS_MODE                           ; Enter SYS mode
    ADD     r1, r1, r2                          ; Calculate start of SYS stack
    BIC     r1, r1, #7                          ; Ensure 8-byte alignment
    MOV     sp, r1                              ; Setup SYS stack pointer
    MOV     lr, r3                              ; Restore LR

    LDR     r2, =SVC_STACK_SIZE                 ; Pickup SVC stack size
    CPS     #SVC_MODE                           ; Enter SVC mode
    ADD     r1, r1, r2                          ; Calculate start of SVC stack
    BIC     r1, r1, #7                          ; Ensure 8-byte alignment
    MOV     sp, r1                              ; Setup SVC stack pointer

    LDR     r2, =IRQ_STACK_SIZE                 ; Pickup IRQ stack size
    CPS     #IRQ_MODE                           ; Enter IRQ mode
    ADD     r1, r1, r2                          ; Calculate start of IRQ stack
    BIC     r1, r1, #7                          ; Ensure 8-byte alignment
    MOV     sp, r1                              ; Setup IRQ stack pointer

    LDR     r2, =FIQ_STACK_SIZE                 ; Pickup FIQ stack size
    CPS     #FIQ_MODE                           ; Enter FIQ mode
    ADD     r1, r1, r2                          ; Calculate start of FIQ stack
    BIC     r1, r1, #7                          ; Ensure 8-byte alignment
    MOV     sp, r1                              ; Setup FIQ stack pointer
    MOV     sl, #0                              ; Clear sl
    MOV     fp, #0                              ; Clear fp

    LDR     r2, =ABT_STACK_SIZE                 ; Pickup ABT stack size
    CPS     #ABT_MODE                           ; Enter ABT mode
    ADD     r1, r1, r2                          ; Calculate start of ABT stack
    BIC     r1, r1, #7                          ; Ensure 8-byte alignment
    MOV     sp, r1                              ; Setup ABT stack pointer

    CPS     #SYS_MODE                           ; Enter SYS mode
;
;    /* Save the first available memory address.  */
;    _tx_initialize_unused_memory =  (VOID_PTR) |Image$$ZI$$Limit| + HEAP + SYS_STACK + SVC_STACK + IRQ_STACK + FIQ_STACK + ABT_STACK;
;
    ADD     r0, r1, #4                          ; Increment to next free word
    LDR     r2, =_tx_initialize_unused_memory   ; Pickup unused memory ptr address
    STR     r0, [r2, #0]                        ; Save first free memory address
;
;    /* Setup Timer for periodic interrupts.  */
;

GIC1_CPU_INTERFACE_BASE     EQU 0x2C002000
GIC1_DIST_INTERFACE_BASE    EQU 0x2C001000

    PUSH    {r4-r12, lr}
    ldr     r2, =GIC1_CPU_INTERFACE_BASE
    ldr     r7, =GIC1_DIST_INTERFACE_BASE
    ; Enable GIC
    mov     r3, #0x1
    str     r3, [r2, #0x0000]
    ; Enable GIC forwarding
    str     r3, [r7, #0x000]
    ; Set Binary Point Register to 0
    eor     r3, r3, r3
    str     r3, [r2, #0x0008]

    ; At this point GIC is enabled
    ; All INTS are disabled / not configured

    ; r0 - interrupt number
    ; r1 - priority
    ; r2 - interrupt type  (edge / level trig.)
    ldr     r0, =34
    ldr     r1, =0xF0
    ldr     r2, =1

    ldr     r7, =GIC1_DIST_INTERFACE_BASE

    ; enable the interrupt in isenable register
    mov     r4, #0x100              ; ISEN REG offset base
    mov     r5, r0, LSR #5          ; Interrupt_Number DIV 5
    add     r4, r4, r5, LSL #2      ; final offset from GIC DIST BASE
    and     r5, r0, #31             ; bit number
    mov     r8, #1


    ldr     r6, [r7, r4]
    orr     r6, r6, r8, LSL r5
    str     r6, [r7, r4]

    ; setup priority
    mov     r4, #0x400
    mov     r5, r0, LSR #2          ; Interrupt_Number DIV 4
    add     r4, r4, r5, LSL #2      ; final offset from GIC DIST BASE
    and     r5, r0, #3              ; Int_Num MOD 4
    lsl     r5, #3
    lsl     r1, r5

    ldr     r6, [r7, r4]
    orr     r1, r6, r1
    str     r1, [r7, r4]

    ; set up processor target
    mov     r4, #0x800
    mov     r5, r0, LSR #2          ; Interrupt_Number DIV 4
    add     r4, r4, r5, LSL #2      ; final offset from GIC DIST BASE
    and     r5, r0, #3              ; Int_Num MOD 4
    lsl     r5, #3
    mov     r1, #0xff
    lsl     r1, r5

    ldr     r6, [r7, r4]
    orr     r1, r6, r1
    str     r1, [r7, r4]

    ; set up interrupt type
    mov     r4, #0xC00
    mov     r5, r0, LSR #4
    add     r4, r4, r5, LSL #2  ; offset from base

    ; field
    and     r5, r0, #15
    lsl     r5, #1
    lsl     r2, r5

    ldr     r6, [r7, r4]
    orr     r2, r6, r2
    str     r2, [r7, r4]

    ldr     r2, =GIC1_CPU_INTERFACE_BASE

    ; set the interrupt id prio mask
    ; Max Priorities = 32.
    ; mask = (32 - 1) << 3
    mov     r3, #0xF8
    str     r3, [r2, #0x0004]

    CPSIE   if

    ; Timer base
    ldr     r0, =0x1C110000

    ; get the timer id
    ldr     r1, [r0, #0xfe0]

    ; set count value in load register
    ldr     r1, =0x00000020
    str     r1, [r0, #00]

    ; enable the timer
    ; periodic mode
    mov     r1, #0xe2
    str     r1, [r0, #08]

    POP     {r4-r12, lr}






;
;    /* Done, return to caller.  */
;
    BX      lr                                  ; Return to caller
;}
;
;
;/* Define initial heap/stack routine for the ARM RealView (and ADS) startup code.  This
;   routine will set the initial stack to use the ThreadX IRQ & FIQ & 
;   (optionally SYS) stack areas.  */
;
    EXPORT  __user_initial_stackheap
__user_initial_stackheap
    LDR     r0, =|Image$$ZI$$Limit|             ; Get end of non-initialized RAM area
    LDR     r2, =HEAP_SIZE                      ; Pickup the heap size
    ADD     r2, r2, r0                          ; Setup heap limit
    ADD     r3, r2, #4                          ; Setup stack limit
    MOV     r1, r3                              ; Setup start of stack
    IF :DEF:TX_ENABLE_IRQ_NESTING
    LDR     r12, =SYS_STACK_SIZE                ; Pickup IRQ system stack
    ADD     r1, r1, r12                         ; Setup the return system stack
    BIC     r1, r1, #7                          ; Ensure 8-byte alignment
    ENDIF
    LDR     r12, =FIQ_STACK_SIZE                ; Pickup FIQ stack size
    ADD     r1, r1, r12                         ; Setup the return system stack
    BIC     r1, r1, #7                          ; Ensure 8-byte alignment
    LDR     r12, =IRQ_STACK_SIZE                ; Pickup IRQ system stack
    ADD     r1, r1, r12                         ; Setup the return system stack
    BIC     r1, r1, #7                          ; Ensure 8-byte alignment
    IF  {INTER} = {TRUE}
    BX      lr                                  ; Return to caller
    ELSE
    MOV     pc, lr                              ; Return to caller
    ENDIF
;
;
;/* Define shells for each of the interrupt vectors.  */
;
    EXPORT  __tx_undefined
__tx_undefined
    B       __tx_undefined                      ; Undefined handler
;
    EXPORT  __tx_reserved_handler
__tx_reserved_handler
    B       __tx_reserved_handler               ; Reserved exception handler
;
;
    EXPORT  __tx_irq_handler
    EXPORT  __tx_irq_processing_return      
__tx_irq_handler
;
;    /* Jump to context save to save system context.  */
    B       _tx_thread_context_save
__tx_irq_processing_return
;
;    /* At this point execution is still in the IRQ mode.  The CPSR, point of
;       interrupt, and all C scratch registers are available for use.  In 
;       addition, IRQ interrupts may be re-enabled - with certain restrictions -
;       if nested IRQ interrupts are desired.  Interrupts may be re-enabled over
;       small code sequences where lr is saved before enabling interrupts and 
;       restored after interrupts are again disabled.  */
;
;
    BL      _tx_timer_interrupt                 ; Timer interrupt handler
    
    ; clear timer interrupt
    ldr     r0, =0x1C110000
    eor     r1, r1, r1
    str     r1, [r0, #0x0C]

_tx_not_timer_interrupt
;
;    /* Interrupt nesting is allowed after calling _tx_thread_irq_nesting_start 
;       from IRQ mode with interrupts disabled.  This routine switches to the
;       system mode and returns with IRQ interrupts enabled.  
;       
;       NOTE:  It is very important to ensure all IRQ interrupts are cleared 
;       prior to enabling nested IRQ interrupts.  */
    IF :DEF:TX_ENABLE_IRQ_NESTING
    BL      _tx_thread_irq_nesting_start
    ENDIF
;
;
;    /* Application IRQ handlers can be called here!  */
;
;    /* If interrupt nesting was started earlier, the end of interrupt nesting
;       service must be called before returning to _tx_thread_context_restore.  
;       This routine returns in processing in IRQ mode with interrupts disabled.  */
    IF :DEF:TX_ENABLE_IRQ_NESTING
    BL      _tx_thread_irq_nesting_end
    ENDIF
;
;    /* Jump to context restore to restore system context.  */
    B       _tx_thread_context_restore
;
;
;    /* This is an example of a vectored IRQ handler.  */
;
    EXPORT  __tx_example_vectored_irq_handler
__tx_example_vectored_irq_handler
;
;
;    /* Save initial context and call context save to prepare for 
;       vectored ISR execution.  */
;
;    STMDB   sp!, {r0-r3}                        ; Save some scratch registers
;    MRS     r0, SPSR                            ; Pickup saved SPSR
;    SUB     lr, lr, #4                          ; Adjust point of interrupt 
;    STMDB   sp!, {r0, r10, r12, lr}             ; Store other scratch registers
;    BL      _tx_thread_vectored_context_save    ; Vectored context save
;
;    /* At this point execution is still in the IRQ mode.  The CPSR, point of
;       interrupt, and all C scratch registers are available for use.  In 
;       addition, IRQ interrupts may be re-enabled - with certain restrictions -
;       if nested IRQ interrupts are desired.  Interrupts may be re-enabled over
;       small code sequences where lr is saved before enabling interrupts and 
;       restored after interrupts are again disabled.  */
;
;
;    /* Interrupt nesting is allowed after calling _tx_thread_irq_nesting_start 
;       from IRQ mode with interrupts disabled.  This routine switches to the
;       system mode and returns with IRQ interrupts enabled.  
;       
;       NOTE:  It is very important to ensure all IRQ interrupts are cleared 
;       prior to enabling nested IRQ interrupts.  */
;    IF :DEF:TX_ENABLE_IRQ_NESTING
;    BL      _tx_thread_irq_nesting_start
;    ENDIF
;
;    /* Application IRQ handlers can be called here!  */
;
;    /* If interrupt nesting was started earlier, the end of interrupt nesting
;       service must be called before returning to _tx_thread_context_restore.  
;       This routine returns in processing in IRQ mode with interrupts disabled.  */
;    IF :DEF:TX_ENABLE_IRQ_NESTING
;    BL      _tx_thread_irq_nesting_end
;    ENDIF
;
;    /* Jump to context restore to restore system context.  */
;    B       _tx_thread_context_restore
;
;
    IF  :DEF:TX_ENABLE_FIQ_SUPPORT
    EXPORT  __tx_fiq_handler
    EXPORT  __tx_fiq_processing_return
__tx_fiq_handler
;
;    /* Jump to fiq context save to save system context.  */
    B       _tx_thread_fiq_context_save
__tx_fiq_processing_return
;
;    /* At this point execution is still in the FIQ mode.  The CPSR, point of
;       interrupt, and all C scratch registers are available for use.  */
;
;    /* Interrupt nesting is allowed after calling _tx_thread_fiq_nesting_start 
;       from FIQ mode with interrupts disabled.  This routine switches to the
;       system mode and returns with FIQ interrupts enabled. 
;
;       NOTE:  It is very important to ensure all FIQ interrupts are cleared 
;       prior to enabling nested FIQ interrupts.  */
    IF  :DEF:TX_ENABLE_FIQ_NESTING
    BL      _tx_thread_fiq_nesting_start
    ENDIF
;
;    /* Application FIQ handlers can be called here!  */
;
;    /* If interrupt nesting was started earlier, the end of interrupt nesting
;       service must be called before returning to _tx_thread_fiq_context_restore.  */
    IF  :DEF:TX_ENABLE_FIQ_NESTING
    BL      _tx_thread_fiq_nesting_end
    ENDIF
;
;    /* Jump to fiq context restore to restore system context.  */
    B       _tx_thread_fiq_context_restore
;
;
    ELSE
    EXPORT  __tx_fiq_handler
__tx_fiq_handler
    B       __tx_fiq_handler                    ; FIQ interrupt handler
    ENDIF

    
    
;/**************************************************************************/ 
;/*                                                                        */ 
;/*  FUNCTION                                               RELEASE        */ 
;/*                                                                        */ 
;/*    __tx_prefetch_handler & __tx_abort_handler      Cortex-A7/MMU/AC5   */ 
;/*                                                           6.1          */
;/*  AUTHOR                                                                */
;/*                                                                        */
;/*    Scott Larson, Microsoft Corporation                                 */
;/*                                                                        */
;/*  DESCRIPTION                                                           */ 
;/*                                                                        */ 
;/*    This function handles MMU exceptions and fills the                  */ 
;/*    _txm_module_manager_memory_fault_info struct.                       */ 
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
;/*    _txm_module_manager_memory_fault_handler                            */ 
;/*    _tx_execution_thread_exit                                           */ 
;/*    _tx_thread_schedule                                                 */ 
;/*                                                                        */ 
;/*  CALLED BY                                                             */ 
;/*                                                                        */ 
;/*    MMU exceptions                                                      */ 
;/*                                                                        */ 
;/*  RELEASE HISTORY                                                       */ 
;/*                                                                        */ 
;/*    DATE              NAME                      DESCRIPTION             */
;/*                                                                        */
;/*  09-30-2020      Scott Larson            Initial Version 6.1           */
;/*                                                                        */
;/**************************************************************************/

; *******************************************************************
; MMU Exception Handling
; *******************************************************************
    EXTERN  _tx_thread_system_state
    EXTERN  _txm_module_manager_memory_fault_info
    EXTERN  _tx_thread_current_ptr
    EXTERN  _txm_module_manager_memory_fault_handler
    EXTERN  _tx_execution_thread_exit
    EXTERN  _tx_thread_schedule
    
    EXPORT  __tx_prefetch_handler
    EXPORT  __tx_abort_handler
__tx_prefetch_handler
__tx_abort_handler
    STMDB   sp!, {r0-r3}                        ; Save some working registers
    LDR     r3, =_tx_thread_system_state        ; Pickup address of system state var
    LDR     r2, [r3, #0]                        ; Pickup system state
    ADD     r2, r2, #1                          ; Increment the interrupt counter
    STR     r2, [r3, #0]                        ; Store it back in the variable
    SUB     lr, lr, #4                          ; Adjust point of exception
;
;    /* Now pickup and store all the fault related information.  */
;
    ; Pickup the memory fault info struct
    LDR     r3, =_txm_module_manager_memory_fault_info
    LDR     r0, =_tx_thread_current_ptr     ; Build current thread pointer address
    LDR     r1, [r0]                        ; Pickup the current thread pointer
    STR     r1, [r3, #0]                    ; Save current thread pointer
    STR     lr, [r3, #4]                    ; Save point of fault
    MRC     p15, 0, r0, c6, c0, 0           ; Read DFAR
    STR     r0, [r3, #8]                    ; Save DFAR
    MRC     p15, 0, r0, c5, c0, 0           ; Read DFSR
    STR     r0, [r3, #12]                   ; Save DFSR
    MRC     p15, 0, r0, c6, c0, 2           ; Read IFAR
    STR     r0, [r3, #16]                   ; Save IFAR
    MRC     p15, 0, r0, c5, c0, 1           ; Read IFSR
    STR     r0, [r3, #20]                   ; Save IFSR
    
    ; Save registers r0-r12
    POP     {r0-r2}
    STR     r0, [r3, #28]                   ; Save r0
    STR     r1, [r3, #32]                   ; Save r1
    STR     r2, [r3, #36]                   ; Save r2
    POP     {r0}
    STR     r0, [r3, #40]                   ; Save r3
    STR     r4, [r3, #44]                   ; Save r4
    STR     r5, [r3, #48]                   ; Save r5
    STR     r6, [r3, #52]                   ; Save r6
    STR     r7, [r3, #56]                   ; Save r7
    STR     r8, [r3, #60]                   ; Save r8
    STR     r9, [r3, #64]                   ; Save r9
    STR     r10,[r3, #68]                   ; Save r10
    STR     r11,[r3, #72]                   ; Save r11
    STR     r12,[r3, #76]                   ; Save r12
    
    CPS     #SYS_MODE                       ; Enter SYS mode
    MOV     r0, lr                          ; Pickup lr
    MOV     r1, sp                          ; Pickup sp
    CPS     #ABT_MODE                       ; Back to ABT mode
    STR     r0, [r3, #80]                   ; Save lr
    STR     r1, [r3, #24]                   ; Save sp
    MRS     r0, SPSR                        ; Pickup SPSR
    STR     r0, [r3, #84]                   ; Save SPSR
    ORR     r0, r0, #SYS_MODE               ; Return into SYS mode
    BIC     r0, r0, #THUMB_MASK             ; Clear THUMB mode
    MSR     SPSR_c, r0                      ; Save SPSR
    
    ; Call memory manager fault handler
    BL      _txm_module_manager_memory_fault_handler

    IF  :DEF:TX_ENABLE_EXECUTION_CHANGE_NOTIFY
;
;    /* Call the thread exit function to indicate the thread is no longer executing.  */
;
    BL      _tx_execution_thread_exit       ; Call the thread exit function
    ENDIF

    LDR     r0, =_tx_thread_system_state    ; Pickup address of system state
    LDR     r1, [r0]                        ; Pickup system state
    SUB     r1, r1, #1                      ; Decrement
    STR     r1, [r0]                        ; Store new system state
    
    MOV     r1, #0                          ; Build NULL value
    LDR     r0, =_tx_thread_current_ptr     ; Pickup address of current thread pointer
    STR     r1, [r0]                        ; Clear current thread pointer
    
    ; Return from exception
    LDR     lr, =_tx_thread_schedule        ; Load scheduler address
    MOVS    pc, lr                          ; Return to scheduler
; *******************************************************************
; End of MMU exception handling.
; *******************************************************************

;
;    /* Reference build options and version ID to ensure they come in.  */
;
    LDR     r2, =_tx_build_options              ; Pickup build options variable address
    LDR     r0, [r2, #0]                        ; Pickup build options content
    LDR     r2, =_tx_version_id                 ; Pickup version ID variable address
    LDR     r0, [r2, #0]                        ; Pickup version ID content
;
;
    END

