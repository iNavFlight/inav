@; ------------------------------------------------------------
@; Cortex-A15 MPCore SMP Prime Number Generator Example
@;
@; Copyright (c) 2011-2012 ARM Ltd.  All rights reserved.
@; ------------------------------------------------------------
@
@    PRESERVE8
@
@  AREA  StartUp,CODE,READONLY
@
@; Standard definitions of mode bits and interrupt (I&F) flags in PSRs
@
Mode_USR          =     0x10
Mode_FIQ          =     0x11
Mode_IRQ          =     0x12
Mode_SVC          =     0x13
Mode_ABT          =     0x17
Mode_UNDEF        =     0x1B
Mode_SYS          =     0x1F

I_Bit             =     0x80 @ when I bit is set, IRQ is disabled
F_Bit             =     0x40 @ when F bit is set, FIQ is disabled

SYS_MODE        =     0xDF
SVC_MODE        =     0xD3
IRQ_MODE        =     0xD2

@; ------------------------------------------------------------
@; Porting defines
@; ------------------------------------------------------------
@
L1_COHERENT       =     0x00014c06  @ Template descriptor for coherent memory
L1_NONCOHERENT    =     0x00000c1e  @ Template descriptor for non-coherent memory
L1_DEVICE         =     0x00000c06  @ Template descriptor for device memory

.section  VECTORS, "ax"
.align 3
.cfi_sections .debug_frame  // put stack frame info into .debug_frame instead of .eh_frame

@; ------------------------------------------------------------
@
@  ENTRY
@
    .global Vectors
Vectors:
  B      Reset_Handler
  B      Undefined_Handler
  B      SVC_Handler
  B      Prefetch_Handler
  B      Abort_Handler
  B      Hypervisor_Handler
  B      IRQ_Handler
  B      FIQ_Handler

@; ------------------------------------------------------------
@; Handlers for unused exceptions
@; ------------------------------------------------------------
@
Undefined_Handler:
  B       Undefined_Handler
SVC_Handler:
  B       SVC_Handler
Prefetch_Handler:
  B       Prefetch_Handler
Abort_Handler:
  B       Abort_Handler
Hypervisor_Handler:
  B       Hypervisor_Handler
FIQ_Handler:
  B       FIQ_Handler

@; ------------------------------------------------------------
@; Imports
@; ------------------------------------------------------------
  .global   readIntAck
  .global   writeEOI
  .global   enableGIC
  .global   enableGICProcessorInterface
  .global   setPriorityMask
  .global   enableIntID
  .global   setIntPriority
  .global   joinSMP

  .global   invalidateCaches
  .global   disableHighVecs
  .global   _start
@; [Grape Change Start]
@;  IMPORT main_app

  .global   _tx_thread_smp_initialize_wait
  .global   _tx_thread_smp_release_cores_flag
  .global   _tx_thread_context_save
  .global   _tx_thread_context_restore
  .global   _tx_timer_interrupt
  .global   _tx_thread_smp_inter_core_interrupts

  .global   enableBranchPrediction
  .global   enableCaches

VFPEnable       =       0x40000000              @ VFP enable value

@;/*------------------------------------------------------------------------*/
@;/*--- Versatile Express(Timer0)                                        ---*/
GIC_DIST_CPUTARGET          =       0x2C001820
GIC_DIST_CPUTARGET_VALUE    =       0x000f0000

GIC_DIST_CONFIG             =       0x2C001C08
GIC_DIST_CONFIG_VALUE       =       0x00000000

GIC_DIST_PRIO               =       0x2C001420
GIC_DIST_PRIO_VALUE         =       0x00a00000

GIC_DIST_CONTROL            =       0x2C001000
GIC_DIST_CONTROL_VALUE      =       0x00000001

GIC_CPU_CONTROL             =       0x2C002000
GIC_CPU_CONTROL_VALUE       =       0x00000001

GIC_CPU_PRIO_MASK           =       0x2C002004
GIC_CPU_PRIO_MASK_VALUE     =       0x000000ff

GIC_DIST_ENABLE_SET         =       0x2C001104
GIC_DIST_ENABLE_SET_VALUE   =       0x00000004

GIC_CPU_INTACK              =       0x2C00200C
GIC_CPU_EOI                 =       0x2C002010
;
;
;
TIMCLK_CTRL                 =       0x1C020000
TIMCLK_CTRL_VALUE           =       0x00028000          @ Use EXTCLK (1MHz) for TIMCLK not REFCLK32KHZ

TIMER_LOAD                  =       0x1C110000
TIMER_LOAD_VALUE            =       0x00000140          @ 10ms

TIMER_CTRL                  =       0x1C110008
TIMER_CTRL_STOP             =       0x00000020
TIMER_CTRL_VALUE            =       0x000000E0
TIMER_ACK                   =       34                  @ Timer0
TIMER_INT_CLR               =       0x1C11000C
;
HANDLER_SET                 =       0x80000018
HANDLER_SET_VALUE           =       0xE59FF018
HANDLER_ADDRESS             =       0x80000038          @ irq

@;/*--- Versatile Express(Timer0)                                        ---*/
@;/*------------------------------------------------------------------------*/
@; [Grape Change End]

    .global     _page_table_top
    .global     _exec
    .global     _stack_init_irq
    .global     _stack_init_usr

@; ------------------------------------------------------------
@; Interrupt Handler
@; ------------------------------------------------------------
@
@  EXPORT IRQ_Handler
    .align 2
    .global IRQ_Handler
    .type   IRQ_Handler,function
IRQ_Handler:
@; [Grape Change Start]
  .global   __tx_irq_processing_return
@;  SUB     lr, lr, #4          ; Pre-adjust lr
@;  SRSFD   sp!, #Mode_IRQ      ; Save lr and SPRS to IRQ mode stack
@;  PUSH    {r0-r4, r12}        ; Save APCS corruptible registers to IRQ mode stack (and maintain 8 byte alignment)
@;    /* Jump to context save to save system context.  */
    B       _tx_thread_context_save
__tx_irq_processing_return:
    PUSH    {r4, r5}            @ Save some preserved registers (r5 is saved just for 8-byte alignment)
@; [Grape Change End]

  @ Acknowledge the interrupt
  BL      readIntAck
  MOV     r4, r0

@  ;
@  ; This example only uses (and enables) one.  At this point
@  ; you would normally check the ID, and clear the source.
@  ;

@; [Grape Change Start]
@;/*------------------------------------------------------------------------*/
@;/*--- Versatile Express(Timer0)                                        ---*/
    LDR     r0, =TIMER_ACK                      @ Setup timer acknowledge number
    CMP     r4, r0
    BNE     by_pass

    LDR     r0, =TIMER_INT_CLR                  @ Setup Timer interrupt clear register address
    LDR     r1, =1
    STR     r1, [r0, #0]                        @ Clear timer interrupt

    BL      _tx_timer_interrupt                 @ Timer interrupt handler
    B       by_pass2
@;/*--- Versatile Express(Timer0)                                        ---*/
@;/*------------------------------------------------------------------------*/

by_pass:
@;    /* Just increment the per-thread interrupt count for analysis purposes.  */
@;
    MRC     p15, 0, r0, c0, c0, 5                   @ Read CPU ID register
    AND     r0, r0, #0x03                           @ Mask off, leaving the CPU ID field
    LSL     r0, r0, #2                              @ Build offset to array indexes
    LDR     r1,=_tx_thread_smp_inter_core_interrupts @ Pickup base address of core interrupt counter array
    ADD     r1, r1, r0                              @ Build array index
    LDR     r0, [r1]                                @ Pickup counter
    ADD     r0, r0, #1                              @ Increment counter
    STR     r0, [r1]                                @ Store back counter

@;/* user handler */


@;/*------------------------------------------------------------------------*/
@; [Grape Change End]
@
@
@; [Grape Change Start]
by_pass2:
  @ Write end of interrupt reg
  MOV     r0, r4
  BL      writeEOI

@;  POP     {r0-r4, r12}        ; Restore stacked APCS registers
@;  MOV     r2, #0x01           ; Set r2 so CPU leaves holding pen
@;  RFEFD   sp!                 ; Return from exception
@;;;;;
@;    /* Jump to context restore to restore system context.  */
    POP     {r4, r5}                @ Recover preserved registers
    B       _tx_thread_context_restore
@; [Grape Change End]



@; ------------------------------------------------------------
@; Reset Handler - Generic initialization, run by all CPUs
@; ------------------------------------------------------------
@
@  EXPORT Reset_Handler
    .align 2
    .global $Reset_Handler
    .type   $Reset_Handler,function
Reset_Handler:

@  ;
@  ; Set ACTLR.SMP bit
@  ; ------------------
  BL      joinSMP

@;
@; Disable caches, MMU and branch prediction in case they were left enabled from an earlier run
@; This does not need to be done from a cold reset
@; ------------------------------------------------------------
  MRC     p15, 0, r0, c1, c0, 0       @ Read CP15 System Control register
  BIC     r0, r0, #(0x1 << 12)        @ Clear I bit 12 to disable I Cache
  BIC     r0, r0, #(0x1 <<  2)        @ Clear C bit  2 to disable D Cache
  BIC     r0, r0, #0x1                @ Clear M bit  0 to disable MMU
  BIC     r0, r0, #(0x1 << 11)        @ Clear Z bit 11 to disable branch prediction
  MCR     p15, 0, r0, c1, c0, 0       @ Write CP15 System Control register

@; The MMU is enabled later, before calling main().  Caches and branch prediction are enabled inside main(),
@; after the MMU has been enabled and scatterloading has been performed.
@
@  ;
@  ; Setup stacks
@  ;---------------

  MRC     p15, 0, r0, c0, c0, 5       @ Read CPU ID register
  ANDS    r0, r0, #0x03               @ Mask off, leaving the CPU ID field

@; [Grape Change Start]
@;  MSR     CPSR_c, #Mode_IRQ:OR:I_Bit:OR:F_Bit
@;  LDR     r1, =_stack_init_irq                  ; IRQ stacks for CPU 0,1,2,3
@;  SUB     r1, r1, r0, LSL #8                    ; 256 bytes of IRQ stack per CPU (0,1,2,3) - see scatter.scat
@;  MOV     sp, r1
@;
@;  MSR     CPSR_c, #Mode_SYS:OR:I_Bit:OR:F_Bit   ; Interrupts initially disabled
@;  LDR     r1, =_stack_init_usr                  ; App stacks for all CPUs
@;  SUB     r1, r1, r0, LSL #12                   ; 0x1000 bytes of App stack per CPU - see scatter.scat
@;  MOV     sp, r1


  MOV     r1, #IRQ_MODE                         @ Build IRQ mode CPSR
  MSR     CPSR_c, r1                            @ Enter IRQ mode
@  MSR     CPSR_c, #Mode_IRQ:OR:I_Bit:OR:F_Bit
  LDR     r1, =_stack_init_irq                  @ IRQ stacks for CPU 0,1,2,3
  SUB     r1, r1, r0, LSL #10                   @ 1024 bytes of IRQ stack per CPU (0,1,2,3) - see scatter.scat
  MOV     sp, r1

  MOV     r1, #SYS_MODE                         @ Build SYS mode CPSR
  MSR     CPSR_c, r1                            @ Enter SYS mode
@  MSR     CPSR_c, #Mode_SYS:OR:I_Bit:OR:F_Bit   @ Interrupts initially disabled
  LDR     r1, =_stack_init_usr                  @ App stacks for all CPUs
  SUB     r1, r1, r0, LSL #12                   @ 0x1000 bytes of App stack per CPU - see scatter.scat
  MOV     sp, r1

  MOV     r2, #SVC_MODE                         @ Build SVC mode CPSR
  MSR     CPSR_c, r2                            @ Enter SVC mode
@  MSR     CPSR_c, #Mode_SVC:OR:I_Bit:OR:F_Bit   @ Interrupts initially disabled
  MOV     sp, r1
@; [Grape Change End]
@
@  ;
@  ; Set vector base address
@  ; ------------------------
  LDR    r0, =Vectors
  MCR    p15, 0, r0, c12, c0, 0 @ Write Secure or Non-secure Vector Base Address
  BL     disableHighVecs @ Ensure that V-bit is cleared

@  ;
@  ; Invalidate caches
@  ; ------------------
  BL      invalidateCaches

@  ;
@  ; Clear Branch Prediction Array
@  ; ------------------------------
  MOV     r0, #0x0
  MCR     p15, 0, r0, c7, c5, 6     @ BPIALL - Invalidate entire branch predictor array

@; [Grape Change Start]
@;  ; Disable loop-buffer to fix errata on A15 r0p0
@;  MRC  p15, 0, r0, c0, c0, 0  ; Read main ID register MIDR
@;  MOV  r1, r0, lsr #4         ; Extract Primary Part Number
@;  LDR  r2, =0xFFF
@;  AND  r1, r1, r2
@;  LDR  r2, =0xC0F
@;  CMP  r1, r2                 ; Is this an A15?
@;  BNE  notA15r0p0             ; Jump if not A15
@;  AND  r5, r0, #0x00f00000    ; Variant
@;  AND  r6, r0, #0x0000000f    ; Revision
@;  ORRS r6, r6, r5             ; Combine variant and revision
@;  BNE  notA15r0p0             ; Jump if not r0p0
@;  MRC  p15, 0, r0, c1, c0, 1  ; Read Aux Ctrl Reg
@;  ORR  r0, r0, #(1 << 1)      ; Set bit 1 to Disable Loop Buffer
@;  MCR  p15, 0, r0, c1, c0, 1  ; Write Aux Ctrl Reg
@;  ISB
@;notA15r0p0
@; [Grape Change End]
@
@  ;
@  ; Invalidate TLBs
@  ;------------------
  MOV     r0, #0x0
  MCR     p15, 0, r0, c8, c7, 0     @ TLBIALL - Invalidate entire Unified TLB

@  ;
@  ; Set up Domain Access Control Reg
@  ; ----------------------------------
@  ; b00 - No Access (abort)
@  ; b01 - Client (respect table entry)
@  ; b10 - RESERVED
@  ; b11 - Manager (ignore access permissions)

  MRC     p15, 0, r0, c3, c0, 0      @ Read Domain Access Control Register
  LDR     r0, =0x55555555            @ Initialize every domain entry to b01 (client)
  MCR     p15, 0, r0, c3, c0, 0      @ Write Domain Access Control Register

@  ;;
@  ;; Enable L1 Preloader - Auxiliary Control
@  ;; -----------------------------------------
@  ;; Seems to undef on panda?
@  ;MRC     p15, 0, r0, c1, c0, 1      ; Read ACTLR
@  ;ORR     r0, r0, #0x4
@  ;MCR     p15, 0, r0, c1, c0, 1      ; Write ACTLR
@
@  ; Page tables
@  ; -------------------------
@  ; Each CPU will have its own L1 page table.  The
@  ; code reads the base address from the scatter file
@  ; the uses the CPUID to calculate an offset for each
@  ; CPU.
@  ;
@  ; The page tables are generated at boot time.  First
@  ; the table is zeroed.  Then the individual valid
@  ; entries are written in
@  ;
@
@  ; Calculate offset for this CPU
  LDR     r0, =_page_table_top
  MRC     p15, 0, r1, c0, c0, 5     @ Read Multiprocessor Affinity Register
  ANDS    r1, r1, #0x03             @ Mask off, leaving the CPU ID field
  MOV     r1, r1, LSL #14           @ Convert core ID into a 16K offset (this is the size of the table)
  ADD     r0, r1, r0                @ Add offset to current table location to get dst

  @ Fill table with zeros
  MOV     r2, #1024                 @ Set r3 to loop count (4 entries per iteration, 1024 iterations)
  MOV     r1, r0                    @ Make a copy of the base dst
  MOV     r3, #0
  MOV     r4, #0
  MOV     r5, #0
  MOV     r6, #0
ttb_zero_loop:
  STMIA   r1!, {r3-r6}              @ Store out four entries
  SUBS    r2, r2, #1                @ Decrement counter
  BNE     ttb_zero_loop

@  ;
@  ; STANDARD ENTRIES
@  ;
@
@  ; Entry for VA 0x0
@  ; This region must be coherent
@  ;LDR     r1, =PABASE_VA0           ; Physical address
@  ;LDR     r2, =L1_COHERENT          ; Descriptor template
@  ;ORR     r1, r1, r2                ; Combine address and template
@  ;STR     r1, [r0]
@
@
@  ; If not flat mapping, you need a page table entry covering
@  ; the physical address of the boot code.
@  ; This region must be coherent
  LDR     r1,=_exec                  @ Base physical address of code segment
  LSR     r1,#20                     @ Shift right to align to 1MB boundaries
  LDR     r3, =L1_COHERENT           @ Descriptor template
  ORR     r3, r1, LSL#20             @ Setup the initial level1 descriptor again
  STR     r3, [r0, r1, LSL#2]        @ str table entry

@; [Grape Change Start]
@;/*------------------------------------------------------------------------*/
@;/*--- Versatile Express(Timer0)                                        ---*/
  LDR     r1, =0x80000000           @ Physical address of HANDLER
  LSR     r1, r1, #20               @ Clear bottom 20 bits, to find which 1MB block it is in
  LSL     r2, r1, #2                @ Make a copy, and multiply by four.  This gives offset into the page tables
  LSL     r1, r1, #20               @ Put back in address format

  LDR     r3, =L1_COHERENT          @ Descriptor template
  ORR     r1, r1, r3                @ Combine address and template
  STR     r1, [r0, r2]

  LDR     r1, =0x2C000000           @ Physical address of GIC_DIST
  LSR     r1, r1, #20               @ Clear bottom 20 bits, to find which 1MB block it is in
  LSL     r2, r1, #2                @ Make a copy, and multiply by four.  This gives offset into the page tables
  LSL     r1, r1, #20               @ Put back in address format

  LDR     r3, =L1_DEVICE            @ Descriptor template
  ORR     r1, r1, r3                @ Combine address and template
  STR     r1, [r0, r2]

  LDR     r1, =0x1C000000           @ Physical address of TIMER
  LSR     r1, r1, #20               @ Clear bottom 20 bits, to find which 1MB block it is in
  LSL     r2, r1, #2                @ Make a copy, and multiply by four.  This gives offset into the page tables
  LSL     r1, r1, #20               @ Put back in address format

  LDR     r3, =L1_DEVICE            @ Descriptor template
  ORR     r1, r1, r3                @ Combine address and template
  STR     r1, [r0, r2]

  LDR     r1, =0x1C100000           @ Physical address of TIMER
  LSR     r1, r1, #20               @ Clear bottom 20 bits, to find which 1MB block it is in
  LSL     r2, r1, #2                @ Make a copy, and multiply by four.  This gives offset into the page tables
  LSL     r1, r1, #20               @ Put back in address format

  LDR     r3, =L1_DEVICE            @ Descriptor template
  ORR     r1, r1, r3                @ Combine address and template
  STR     r1, [r0, r2]
@;/*--- Versatile Express(Timer0)                                        ---*/
@;/*------------------------------------------------------------------------*/
@; [Grape Change End]
@
@  ; Entry for private address space
@  ; Needs to be marked as Device memory
  MRC     p15, 4, r1, c15, c0, 0    @ Get base address of private address space
  LSR     r1, r1, #20               @ Clear bottom 20 bits, to find which 1MB block it is in
  LSL     r2, r1, #2                @ Make a copy, and multiply by four.  This gives offset into the page tables
  LSL     r1, r1, #20               @ Put back in address format

  LDR     r3, =L1_DEVICE            @ Descriptor template
  ORR     r1, r1, r3                @ Combine address and template
  STR     r1, [r0, r2]

@  ;
@  ; OPTIONAL ENTRIES
@  ; You will need additional translations if:
@  ; - No RAM at zero, so cannot use flat mapping
@  ; - You wish to retarget
@  ;
@  ; If you wish to output to stdio to a UART you will need
@  ; an additional entry
@  ;LDR     r1, =PABASE_UART          ; Physical address of UART
@  ;LSR     r1, r1, #20               ; Mask off bottom 20 bits to find which 1MB it is within
@  ;LSL     r2, r1, #2                ; Make a copy and multiply by 4 to get table offset
@  ;LSL     r1, r1, #20               ; Put back into address format
@  ;LDR     r3, =L1_DEVICE            ; Descriptor template
@  ;ORR     r1, r1, r3                ; Combine address and template
@  ;STR     r1, [r0, r2]
@
@  ;
@  ; Barrier
@  ; --------
  DSB

@  ;
@  ; Set location of level 1 page table
@  ;------------------------------------
@  ; 31:14 - Base addr: 0x8050,0000 (CPU0), 0x8050,4000 (CPU1)
@  ; 13:5  - 0x0
@  ; 4:3   - RGN 0x0 (Outer Noncachable)
@  ; 2     - P   0x0
@  ; 1     - S   0x0 (Non-shared)
@  ; 0     - C   0x0 (Inner Noncachable)
  MCR     p15, 0, r0, c2, c0 ,0


@  ; Enable MMU
@  ;-------------
@  ; Leaving the caches disabled until after scatter loading.
  MRC     p15, 0, r0, c1, c0, 0       @ Read CP15 System Control register
  BIC     r0, r0, #(0x1 << 12)        @ Clear I bit 12 to disable I Cache
  BIC     r0, r0, #(0x1 <<  2)        @ Clear C bit  2 to disable D Cache
  BIC     r0, r0, #0x2                @ Clear A bit  1 to disable strict alignment fault checking
  ORR     r0, r0, #0x1                @ Set M bit 0 to enable MMU before scatter loading
  MCR     p15, 0, r0, c1, c0, 0       @ Write CP15 System Control register

@  ;
@  ; MMU now enabled - Virtual address system now active
@  ;
@; [Grape Change Start]
#ifdef TARGET_FPU_VFP
    MRC     p15, 0, r1, c1, c0, 2               @ r1 = Access Control Register
    ORR     r1, r1, #(0xf << 20)                @ Enable full access for p10,11
    MCR     p15, 0, r1, c1, c0, 2               @ Access Control Register = r1
    MOV     r1, #0
    MCR     p15, 0, r1, c7, c5, 4               @ Flush prefetch buffer because of FMXR below and
                                                @ CP 10 & 11 were only just enabled
    MOV     r0, #VFPEnable                      @ Enable VFP itself
    FMXR    FPEXC, r0                           @ FPEXC = r0
#endif

  LDR     r0, =_tx_thread_smp_release_cores_flag         @ Build address of release cores flag
  MOV     r1, #0
  STR     r1, [r0]
@; [Grape Change End]
@
@  ;
@  ; SMP initialization
@  ; -------------------
  MRC     p15, 0, r0, c0, c0, 5     @ Read CPU ID register
  ANDS    r0, r0, #0x03             @ Mask off, leaving the CPU ID field
  BEQ     primaryCPUInit
  BNE     secondaryCPUsInit




@; ------------------------------------------------------------
@; Initialization for PRIMARY CPU
@; ------------------------------------------------------------
@
@
@   EXPORT primaryCPUInit
    .align 2
    .global primaryCPUInit
    .type   primaryCPUInit,function
primaryCPUInit:

@  ;
@  ; GIC Init
@  ; ---------
  BL      enableGIC
  BL      enableGICProcessorInterface

    BL      enableCaches

@;    /* Setup Timer for periodic interrupts.  */
@;/*------------------------------------------------------------------------*/
@;/*--- Versatile Express(Timer0)                                        ---*/
    LDR     r0, =GIC_DIST_CPUTARGET
    LDR     r2, =GIC_DIST_CPUTARGET_VALUE
    STR     r2, [r0, #0]

    LDR     r0, =GIC_DIST_CONFIG
    LDR     r2, =GIC_DIST_CONFIG_VALUE
    STR     r2, [r0, #0]

    LDR     r0, =GIC_DIST_PRIO
    LDR     r2, =GIC_DIST_PRIO_VALUE
    STR     r2, [r0, #0]

    LDR     r0, =GIC_DIST_CONTROL
    LDR     r2, =GIC_DIST_CONTROL_VALUE
    STR     r2, [r0, #0]

    LDR     r0, =GIC_CPU_CONTROL
    LDR     r2, =GIC_CPU_CONTROL_VALUE
    STR     r2, [r0, #0]

    LDR     r0, =GIC_CPU_PRIO_MASK
    LDR     r2, =GIC_CPU_PRIO_MASK_VALUE
    STR     r2, [r0, #0]

    LDR     r0, =GIC_DIST_ENABLE_SET
    LDR     r2, =GIC_DIST_ENABLE_SET_VALUE
    STR     r2, [r0, #0]
@;;;;;
    LDR     r0, =HANDLER_SET
    LDR     r2, =HANDLER_SET_VALUE
    STR     r2, [r0, #0]
@;
@;    LDR     r0, =HANDLER_ADDRESS
@;    LDR     r2, =IRQ_Handler
@;    STR     r2, [r0, #0]
@;;;;;
    LDR     r0, =TIMER_CTRL
    LDR     r2, =TIMER_CTRL_STOP
    STR     r2, [r0, #0]

    LDR     r2, =TIMER_INT_CLR
    LDR     r0, =1
    STR     r0, [r2, #0]

    LDR     r0, =TIMER_LOAD
    LDR     r2, =TIMER_LOAD_VALUE
    STR     r2, [r0, #0]

    LDR     r0, =TIMER_CTRL
    LDR     r2, =TIMER_CTRL_VALUE
    STR     r2, [r0, #0]
@;/*--- Versatile Express(Timer0)                                        ---*/
@;/*------------------------------------------------------------------------*/

@  ;
@  ; Branch to C lib code
@  ; ----------------------
  B       _start

@; [Grape Change End]


@; ------------------------------------------------------------
@; Initialization for SECONDARY CPUs
@; ------------------------------------------------------------
@
@  EXPORT secondaryCPUsInit
    .align 2
    .global secondaryCPUsInit
    .type   secondaryCPUsInit,function
secondaryCPUsInit:

@  ;
@  ; GIC Init
@  ; ---------
  BL      enableGICProcessorInterface

  MOV     r0, #0x1F                 @ Priority
  BL      setPriorityMask

  MOV     r0, #0x0                  @ ID
  BL      enableIntID

  MOV     r0, #0x0                  @ ID
  MOV     r1, #0x0                  @ Priority
  BL      setIntPriority


@  ;
@  ; Holding Pen
@  ; ------------
@; [Grape Change Start]
@;  MOV     r2, #0x00                 ; Clear r2
@;  CPSIE   i                         ; Enable interrupts
@;holding_pen
@;  CMP     r2, #0x0                  ; r2 will be set to 0x1 by IRQ handler on receiving SGI
@;  WFIEQ
@;  BEQ     holding_pen
@;  CPSID   i                         ; IRQs not used in rest of example, so mask out interrupts
@; [Grape Change End]
@
@
@  ;
@  ; Branch to application
@  ; ----------------------
@; [Grape Change Start]
@;  B       main_app

@;    BL      enableBranchPrediction
    BL      enableCaches

    B        _tx_thread_smp_initialize_wait
@; [Grape Change End]
@


@; ------------------------------------------------------------
@; End of code
@; ------------------------------------------------------------
@
@  END
@
@; ------------------------------------------------------------
@; End of startup.s
@; ------------------------------------------------------------
