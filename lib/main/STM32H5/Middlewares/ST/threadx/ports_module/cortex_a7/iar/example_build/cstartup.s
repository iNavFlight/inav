
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Part one of the system initialization code, 
;; contains low-level
;; initialization.
;;
;; Copyright 2007 IAR Systems. All rights reserved.
;;
;; $Revision: 14520 $
;;

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION IRQ_STACK:DATA:NOROOT(3)
        SECTION FIQ_STACK:DATA:NOROOT(3)
        SECTION CSTACK:DATA:NOROOT(3)

;
; The module in this file are included in the libraries, and may be
; replaced by any user-defined modules that define the PUBLIC symbol
; __iar_program_start or a user defined start symbol.
;
; To override the cstartup defined in the library, simply add your
; modified version to the workbench project.

        SECTION .intvec:CODE:NOROOT(2)

        PUBLIC  __vector
        PUBLIC  __vector_0x14
        PUBLIC  __iar_program_start
        EXTERN  __tx_undefined
        EXTERN  SWI_Handler
        EXTERN  __tx_prefetch_handler
        EXTERN  __tx_abort_handler
        EXTERN  __tx_irq_handler
        EXTERN  __tx_fiq_handler

        ARM
__vector:
        ; All default exception handlers (except reset) are
        ; defined as weak symbol definitions.
        ; If a handler is defined by the application it will take precedence.
        LDR     PC,Reset_Addr           ; Reset
        LDR     PC,Undefined_Addr       ; Undefined instructions
        LDR     PC,SWI_Addr             ; Software interrupt (SWI/SVC)
        LDR     PC,Prefetch_Addr        ; Prefetch abort
        LDR     PC,Abort_Addr           ; Data abort
__vector_0x14:
        DCD     0                       ; RESERVED
        LDR     PC,IRQ_Addr             ; IRQ
        LDR     PC,FIQ_Addr             ; FIQ

Reset_Addr:     DCD   __iar_program_start
Undefined_Addr: DCD   __tx_undefined
SWI_Addr:       DCD   SWI_Handler
Prefetch_Addr:  DCD   __tx_prefetch_handler
Abort_Addr:     DCD   __tx_abort_handler
IRQ_Addr:       DCD   __tx_irq_handler
FIQ_Addr:       DCD   __tx_fiq_handler

; --------------------------------------------------
; ?cstartup -- low-level system initialization code.
;
; After a reser execution starts here, the mode is ARM, supervisor
; with interrupts disabled.
;



        SECTION .text:CODE:NOROOT(2)

;        PUBLIC  ?cstartup        
        EXTERN  ?main
        REQUIRE __vector

        ARM        
        
__iar_program_start:        
?cstartup:

;
; Add initialization needed before setup of stackpointers here.
;

;
; Initialize the stack pointers.
; The pattern below can be used for any of the exception stacks:
; FIQ, IRQ, SVC, ABT, UND, SYS.
; The USR mode uses the same stack as SYS.
; The stack segments must be defined in the linker command file,
; and be declared above.
;


; --------------------
; Mode, correspords to bits 0-5 in CPSR

MODE_MSK DEFINE 0x1F            ; Bit mask for mode bits in CPSR

USR_MODE DEFINE 0x10            ; User mode
FIQ_MODE DEFINE 0x11            ; Fast Interrupt Request mode
IRQ_MODE DEFINE 0x12            ; Interrupt Request mode
SVC_MODE DEFINE 0x13            ; Supervisor mode
ABT_MODE DEFINE 0x17            ; Abort mode
UND_MODE DEFINE 0x1B            ; Undefined Instruction mode
SYS_MODE DEFINE 0x1F            ; System mode


        MRS     r0, cpsr                ; Original PSR value

        ;; Set up the interrupt stack pointer.

        BIC     r0, r0, #MODE_MSK       ; Clear the mode bits
        ORR     r0, r0, #IRQ_MODE       ; Set IRQ mode bits
        MSR     cpsr_c, r0              ; Change the mode
        LDR     sp, =SFE(IRQ_STACK)     ; End of IRQ_STACK

        ;; Set up the fast interrupt stack pointer.

        BIC     r0, r0, #MODE_MSK       ; Clear the mode bits
        ORR     r0, r0, #FIQ_MODE       ; Set FIR mode bits
        MSR     cpsr_c, r0              ; Change the mode
        LDR     sp, =SFE(FIQ_STACK)     ; End of FIQ_STACK

        ;; Set up the normal stack pointer.

        BIC     r0 ,r0, #MODE_MSK       ; Clear the mode bits
        ORR     r0 ,r0, #SYS_MODE       ; Set System mode bits
        MSR     cpsr_c, r0              ; Change the mode
        LDR     sp, =SFE(CSTACK)        ; End of CSTACK

#ifdef __ARMVFP__
        MRC     p15, 0, r1, c1, c0, 2   ; r1 = Access Control Register
        ORR     r1, r1, #(0xf << 20)    ; Enable full access for p10,11
        MCR     p15, 0, r1, c1, c0, 2   ; Access Control Register = r1
        MOV     r1, #0
        MCR     p15, 0, r1, c7, c5, 4   ; Flush prefetch buffer because of FMXR below and
                                        ; CP 10 & 11 were only just enabled
        MOV     r0, #0x40000000         ; Enable VFP itself
        FMXR    FPEXC, r0               ; FPEXC = r0
#endif

;
; Add more initialization here
;

; Continue to ?main for C-level initialization.

        B       ?main

        END



