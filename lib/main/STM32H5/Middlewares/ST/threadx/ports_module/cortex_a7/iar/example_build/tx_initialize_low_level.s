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
SVC_MODE        DEFINE  0xD3                    ; Disable irq,fiq SVC mode
IRQ_MODE        DEFINE  0xD2                    ; Disable irq,fiq IRQ mode
FIQ_MODE        DEFINE  0xD1                    ; Disable irq,fiq FIQ mode
SYS_MODE        DEFINE  0x1F                    ; Disable irq,fiq SYS mode
;
;

    EXTERN      _tx_thread_system_stack_ptr
    EXTERN      _tx_initialize_unused_memory
    EXTERN      _tx_thread_context_save
;    EXTERN      _tx_thread_vectored_context_save
    EXTERN      _tx_thread_context_restore
#ifdef TX_ENABLE_FIQ_SUPPORT
    EXTERN      _tx_thread_fiq_context_save
    EXTERN      _tx_thread_fiq_context_restore
#endif
#ifdef TX_ENABLE_IRQ_NESTING
    EXTERN      _tx_thread_irq_nesting_start
    EXTERN      _tx_thread_irq_nesting_end
#endif
#ifdef TX_ENABLE_FIQ_NESTING
    EXTERN      _tx_thread_fiq_nesting_start
    EXTERN      _tx_thread_fiq_nesting_end
#endif
    EXTERN      _tx_timer_interrupt
    EXTERN      ?cstartup
    EXTERN      _tx_build_options
    EXTERN      _tx_version_id
;
;
;
;/* Define the FREE_MEM segment that will specify where free memory is 
;   defined.  This must also be located in at the end of other RAM segments
;   in the linker control file.  The value of this segment is what is passed
;   to tx_application_define.  */
;
    RSEG    FREE_MEM:DATA
    PUBLIC  __tx_free_memory_start
__tx_free_memory_start
    DS32    4
;
;
;
;
;/**************************************************************************/ 
;/*                                                                        */ 
;/*  FUNCTION                                               RELEASE        */ 
;/*                                                                        */ 
;/*    _tx_initialize_low_level                           Cortex-A7/IAR    */ 
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
    RSEG    .text:CODE:NOROOT(2)
    CODE32
    PUBLIC  _tx_initialize_low_level
_tx_initialize_low_level
;
;    /****** NOTE ****** The IAR 4.11a and above releases call main in SYS mode.  */
;
;    /* Remember the stack pointer, link register, and switch to SVC mode.  */
;
    MOV     r0, sp                              ; Remember the SP
    MOV     r1, lr                              ; Remember the LR
    MOV     r3, #SVC_MODE                       ; Build SVC mode CPSR
    MSR     CPSR_cxsf, r3                       ; Switch to SVC mode
    MOV     sp, r0                              ; Inherit the stack pointer setup by cstartup
    MOV     lr, r1                              ; Inherit the link register
;
;    /* Pickup the start of free memory.  */
;
    LDR     r0, =__tx_free_memory_start         ; Get end of non-initialized RAM area
;
;    /* Save the system stack pointer.  */
;    _tx_thread_system_stack_ptr = (VOID_PTR) (sp);
;
;    /* Save the first available memory address.  */
;    _tx_initialize_unused_memory =  (VOID_PTR) FREE_MEM;
;
    LDR     r2, =_tx_initialize_unused_memory   ; Pickup unused memory ptr address
    STR     r0, [r2, #0]                        ; Save first free memory address
;                      
;    /* Setup Timer for periodic interrupts.  */
;
;    /* Done, return to caller.  */
;
#ifdef TX_THUMB
    BX      lr                                  ; Return to caller
#else
    MOV     pc, lr                              ; Return to caller
#endif
;}
;
;/* Define shells for each of the interrupt vectors.  */
;
    RSEG    .text:CODE:NOROOT(2)
    PUBLIC  __tx_undefined
__tx_undefined
    B       __tx_undefined                      ; Undefined handler
;
    RSEG    .text:CODE:NOROOT(2)
    PUBLIC  __tx_swi_interrupt
__tx_swi_interrupt
    B       __tx_swi_interrupt                  ; Software interrupt handler
;
    RSEG    .text:CODE:NOROOT(2)
    PUBLIC  __tx_reserved_handler
__tx_reserved_handler
    B       __tx_reserved_handler               ; Reserved exception handler
;
    RSEG    .text:CODE:NOROOT(2)
    PUBLIC  __tx_irq_handler
    RSEG    .text:CODE:NOROOT(2)
    PUBLIC  __tx_irq_processing_return      
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
;    /* Interrupt nesting is allowed after calling _tx_thread_irq_nesting_start 
;       from IRQ mode with interrupts disabled.  This routine switches to the
;       system mode and returns with IRQ interrupts enabled.  
;       
;       NOTE:  It is very important to ensure all IRQ interrupts are cleared 
;       prior to enabling nested IRQ interrupts.  */
#ifdef TX_ENABLE_IRQ_NESTING
    BL      _tx_thread_irq_nesting_start
#endif
;
;    /* For debug purpose, execute the timer interrupt processing here.  In
;       a real system, some kind of status indication would have to be checked
;       before the timer interrupt handler could be called.  */
;
    BL      _tx_timer_interrupt                 ; Timer interrupt handler
;
;    /* Application IRQ handlers can be called here!  */
;
;    /* If interrupt nesting was started earlier, the end of interrupt nesting
;       service must be called before returning to _tx_thread_context_restore.  
;       This routine returns in processing in IRQ mode with interrupts disabled.  */
#ifdef TX_ENABLE_IRQ_NESTING
    BL      _tx_thread_irq_nesting_end
#endif
;
;    /* Jump to context restore to restore system context.  */
    B       _tx_thread_context_restore
;
;
;    /* This is an example of a vectored IRQ handler.  */
;
;    RSEG    .text:CODE:NOROOT(2)
;    PUBLIC  __tx_example_vectored_irq_handler
;__tx_example_vectored_irq_handler
;
;    /* Jump to context save to save system context.  */
;    STMDB   sp!, {r0-r3}                    ; Save some scratch registers
;    MRS     r0, SPSR                        ; Pickup saved SPSR
;    SUB     lr, lr, #4                      ; Adjust point of interrupt 
;    STMDB   sp!, {r0, r10, r12, lr}         ; Store other registers
;    BL      _tx_thread_vectored_context_save
;
;    /* At this point execution is still in the IRQ mode.  The CPSR, point of
;       interrupt, and all C scratch registers are available for use.  In 
;       addition, IRQ interrupts may be re-enabled - with certain restrictions -
;       if nested IRQ interrupts are desired.  Interrupts may be re-enabled over
;       small code sequences where lr is saved before enabling interrupts and 
;       restored after interrupts are again disabled.  */
;
;    /* Interrupt nesting is allowed after calling _tx_thread_irq_nesting_start 
;       from IRQ mode with interrupts disabled.  This routine switches to the
;       system mode and returns with IRQ interrupts enabled.  
;       
;       NOTE:  It is very important to ensure all IRQ interrupts are cleared 
;       prior to enabling nested IRQ interrupts.  */
;#ifdef TX_ENABLE_IRQ_NESTING
;    BL      _tx_thread_irq_nesting_start
;#endif
;
;    /* Application IRQ handler is called here!  */
;
;    /* If interrupt nesting was started earlier, the end of interrupt nesting
;       service must be called before returning to _tx_thread_context_restore.  
;       This routine returns in processing in IRQ mode with interrupts disabled.  */
;#ifdef TX_ENABLE_IRQ_NESTING
;    BL      _tx_thread_irq_nesting_end
;#endif
;
;    /* Jump to context restore to restore system context.  */
;    B       _tx_thread_context_restore
;
;
#ifdef TX_ENABLE_FIQ_SUPPORT
    RSEG    .text:CODE:NOROOT(2)
    PUBLIC  __tx_fiq_handler
    RSEG    .text:CODE:NOROOT(2)
    PUBLIC  __tx_fiq_processing_return
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
#ifdef TX_ENABLE_FIQ_NESTING
    BL      _tx_thread_fiq_nesting_start
#endif
;
;    /* Application FIQ handlers can be called here!  */
;
;    /* If interrupt nesting was started earlier, the end of interrupt nesting
;       service must be called before returning to _tx_thread_fiq_context_restore.  */
#ifdef TX_ENABLE_FIQ_NESTING
    BL      _tx_thread_fiq_nesting_end
#endif
;
;    /* Jump to fiq context restore to restore system context.  */
    B       _tx_thread_fiq_context_restore
;
;
#else
    RSEG    .text:CODE:NOROOT(2)
    PUBLIC  __tx_fiq_handler
__tx_fiq_handler
    B       __tx_fiq_handler                    ; FIQ interrupt handler
#endif


;/**************************************************************************/ 
;/*                                                                        */ 
;/*  FUNCTION                                               RELEASE        */ 
;/*                                                                        */ 
;/*    __tx_prefetch_handler & __tx_abort_handler      Cortex-A7/MMU/IAR   */ 
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
THUMB_MASK      DEFINE      0x20                ; Thumb bit (5) of CPSR/SPSR
ABT_MODE        DEFINE      0x17                ; ABORT mode

    EXTERN  _tx_thread_system_state
    EXTERN  _txm_module_manager_memory_fault_info
    EXTERN  _tx_thread_current_ptr
    EXTERN  _txm_module_manager_memory_fault_handler
    EXTERN  _tx_execution_thread_exit
    EXTERN  _tx_thread_schedule
    RSEG    .text:CODE:NOROOT(2)
    ARM
    PUBLIC  __tx_prefetch_handler
    PUBLIC  __tx_abort_handler
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

#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
;
;    /* Call the thread exit function to indicate the thread is no longer executing.  */
;
    BL      _tx_execution_thread_exit       ; Call the thread exit function
#endif

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


BUILD_OPTIONS
    DC32    _tx_build_options                   ; Reference to ensure it comes in
VERSION_ID
    DC32    _tx_version_id                      ; Reference to ensure it comes in

    END
