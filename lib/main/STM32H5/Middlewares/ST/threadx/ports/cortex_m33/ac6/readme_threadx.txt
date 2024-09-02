                     Microsoft's Azure RTOS ThreadX for Cortex-M33 

                              Using the AC6 Tools in Keil uVision

1. Import the ThreadX Projects

In order to build the ThreadX library and the ThreadX demonstration, first open 
the AzureRTOS.uvmpw workspace (located in the "example_build" directory) 
into Keil.


2.  Building the ThreadX run-time Library

Building the ThreadX library is easy; simply set the ThreadX_Library project
as active, then then build the library. You should now observe the compilation 
and assembly of the ThreadX library. This project build produces the ThreadX
library file ThreadX_Library.lib.
Files tx_thread_stack_error_handler.c and tx_thread_stack_error_notify.c 
replace the common files of the same name. 

3.  Demonstration System

The ThreadX demonstration is designed to execute under the Keil debugger on the
FVP_MPS2_Cortex-M33_MDK simulator.

Building the demonstration is easy; simply select the "Batch Build" button. 
You should now observe the compilation and assembly of the ThreadX demonstration of 
both the demo_secure_zone and demo_threadx_non-secure_zone projects. 
Then click the Start/Stop Debug Session button to start the simulator and begin debugging.
You are now ready to execute the ThreadX demonstration.


4.  System Initialization

The entry point in ThreadX for the Cortex-M33 using AC6 tools uses the standard AC6 
Cortex-M33 reset sequence. From the reset vector the C runtime will be initialized.

The ThreadX tx_initialize_low_level.s file is responsible for setting up 
various system data structures, the vector area, and a periodic timer interrupt 
source. 

In addition, _tx_initialize_low_level determines the first available 
address for use by the application, which is supplied as the sole input 
parameter to your application definition function, tx_application_define.


5.  Register Usage and Stack Frames

The following defines the saved context stack frames for context switches
that occur as a result of interrupt handling or from thread-level API calls.
All suspended threads have the same stack frame in the Cortex-M33 version of
ThreadX. The top of the suspended thread's stack is pointed to by 
tx_thread_stack_ptr in the associated thread control block TX_THREAD.

Non-FPU Stack Frame:

    Stack Offset    Stack Contents

    0x00            LR          Interrupted LR (LR at time of PENDSV)
    0x04            r4          Software stacked GP registers
    0x08            r5
    0x0C            r6
    0x10            r7
    0x14            r8
    0x18            r9
    0x1C            r10
    0x20            r11
    0x24            r0          Hardware stacked registers
    0x28            r1
    0x2C            r2
    0x30            r3
    0x34            r12
    0x38            lr
    0x3C            pc
    0x40            xPSR

FPU Stack Frame (only interrupted thread with FPU enabled):

    Stack Offset    Stack Contents

    0x00            LR          Interrupted LR (LR at time of PENDSV)
    0x04            s16         Software stacked FPU registers
    0x08            s17
    0x0C            s18
    0x10            s19
    0x14            s20
    0x18            s21
    0x1C            s22
    0x20            s23
    0x24            s24
    0x28            s25
    0x2C            s26
    0x30            s27
    0x34            s28
    0x38            s29
    0x3C            s30
    0x40            s31
    0x44            r4          Software stacked registers
    0x48            r5
    0x4C            r6
    0x50            r7
    0x54            r8
    0x58            r9
    0x5C            r10
    0x60            r11
    0x64            r0          Hardware stacked registers
    0x68            r1
    0x6C            r2
    0x70            r3
    0x74            r12
    0x78            lr
    0x7C            pc
    0x80            xPSR
    0x84            s0          Hardware stacked FPU registers
    0x88            s1
    0x8C            s2
    0x90            s3
    0x94            s4
    0x98            s5
    0x9C            s6
    0xA0            s7
    0xA4            s8
    0xA8            s9
    0xAC            s10
    0xB0            s11
    0xB4            s12
    0xB8            s13
    0xBC            s14
    0xC0            s15
    0xC4            fpscr


6.  Improving Performance

To make ThreadX and the application(s) run faster, you can enable 
all compiler optimizations. 

In addition, you can eliminate the ThreadX basic API error checking by 
compiling your application code with the symbol TX_DISABLE_ERROR_CHECKING 
defined. 


7.  Interrupt Handling

ThreadX provides complete and high-performance interrupt handling for Cortex-M33
targets. There are a certain set of requirements that are defined in the 
following sub-sections:


7.1  Vector Area

The Cortex-M33 vectors start at the label __Vectors or similar. The application may modify
the vector area according to its needs. There is code in tx_initialize_low_level() that will 
configure the vector base register. 


7.2 Managed Interrupts

ISRs can be written completely in C (or assembly language) without any calls to
_tx_thread_context_save or _tx_thread_context_restore. These ISRs are allowed access to the
ThreadX API that is available to ISRs.

ISRs written in C will take the form (where "your_C_isr" is an entry in the vector table):

void    your_C_isr(void)
{

    /* ISR processing goes here, including any needed function calls.  */
}

ISRs written in assembly language will take the form:


    .global  your_assembly_isr
    .thumb_func
your_assembly_isr:
; VOID your_assembly_isr(VOID)
; {
    PUSH    {r0, lr}
;       
;    /* Do interrupt handler work here */
;    /* BL <your interrupt routine in C> */

    POP     {r0, lr}
    BX      lr
; }

Note: the Cortex-M33 requires exception handlers to be thumb labels, this implies bit 0 set.
To accomplish this, the declaration of the label has to be preceded by the assembler directive
.thumb_func to instruct the linker to create thumb labels. The label __tx_IntHandler needs to 
be inserted in the correct location in the interrupt vector table. This table is typically 
located in either your runtime startup file or in the tx_initialize_low_level.s file.


8. FPU Support

ThreadX for Cortex-M33 supports automatic ("lazy") VFP support, which means that applications threads 
can simply use the VFP and ThreadX automatically maintains the VFP registers as part of the thread 
context.


9.  Revision History

For generic code revision information, please refer to the readme_threadx_generic.txt
file, which is included in your distribution. The following details the revision
information associated with this specific port of ThreadX:

06-02-2021  Release 6.1.7 changes:
            tx_port.h                           Remove unneeded include file
            tx_thread_secure_stack_initialize.S New file
            tx_thread_schedule.S                Added secure stack initialize to SVC hander
            tx_thread_secure_stack.c            Fixed stack pointer save, initialize in handler mode

04-02-2021  Release 6.1.6 changes:
            tx_port.h                           Updated macro definition
            tx_thread_schedule.s                Added low power support

03-02-2021  The following files were changed/added for version 6.1.5:
            tx_port.h                       Added ULONG64_DEFINED

09-30-2020  Initial ThreadX 6.1 version for Cortex-M33 using AC6 tools.


Copyright(c) 1996-2020 Microsoft Corporation


https://azure.com/rtos

