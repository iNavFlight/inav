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
SYS_MODE        DEFINE  0xDF                    ; Disable irq,fiq SYS mode
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
    PUBLIC  __tx_prefetch_handler
__tx_prefetch_handler
    B       __tx_prefetch_handler               ; Prefetch exception handler
;
    RSEG    .text:CODE:NOROOT(2)
    PUBLIC  __tx_abort_handler
__tx_abort_handler
    B       __tx_abort_handler                  ; Abort exception handler
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
;
;
BUILD_OPTIONS
    DC32    _tx_build_options                   ; Reference to ensure it comes in
VERSION_ID
    DC32    _tx_version_id                      ; Reference to ensure it comes in
    END

