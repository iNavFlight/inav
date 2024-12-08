                       Microsoft's Azure RTOS ThreadX for ARMv7-M
                            (Cortex-M3, Cortex-M4, Cortex-M7)
                              Using ARM Compiler 6 (AC6)


1.  Building the ThreadX run-time Library

In order to build the ThreadX library and the ThreadX demonstration, first import 
the 'tx' and 'sample_threadx' projects (located in the "example_build" directory) 
into your DS workspace.

Building the ThreadX library is easy; simply right-click the Eclipse project 
"tx" and then select the "Build Project" button. You should now observe the compilation 
and assembly of the ThreadX library. This project build produces the ThreadX
library file tx.a.


2.  Demonstration System

The ThreadX demonstration is designed to execute under the DS debugger on the
MPS2_Cortex_Mx Bare Metal simulator.

Building the demonstration is easy; simply right-click the Eclipse project 
"sample_threadx" and then select the "Build Project" button. You should now observe 
the compilation and assembly of the ThreadX demonstration. This project build produces 
the ThreadX library file sample_threadx.axf. Next, expand the demo ThreadX project folder 
in the Project Explorer window, right-click on the 'cortex-mx_tx.launch' file, click
'Debug As', and then click 'cortex-mx_tx' from the submenu. This will cause the
debugger to load the sample_threadx.axf ELF file and run to main. You are now ready 
to execute the ThreadX demonstration.


3.  System Initialization

The entry point in ThreadX for the Cortex-M using AC6 tools uses the standard GNU 
Cortex-M reset sequence. From the reset vector the C runtime will be initialized.

The ThreadX tx_initialize_low_level.S file is responsible for setting up 
various system data structures, the vector area, and a periodic timer interrupt 
source. 

In addition, _tx_initialize_low_level determines the first available 
address for use by the application, which is supplied as the sole input 
parameter to your application definition function, tx_application_define.


4.  Register Usage and Stack Frames

The following defines the saved context stack frames for context switches
that occur as a result of interrupt handling or from thread-level API calls.
All suspended threads have the same stack frame in the Cortex-M version of
ThreadX. The top of the suspended thread's stack is pointed to by 
tx_thread_stack_ptr in the associated thread control block TX_THREAD.

Non-FPU Stack Frame:

    Stack Offset    Stack Contents

    0x00            lr          Interrupted lr (lr at time of PENDSV)
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

    0x00            lr          Interrupted lr (lr at time of PENDSV)
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


5.  Improving Performance

The distribution version of ThreadX is built without any compiler 
optimizations. This makes it easy to debug because you can trace or set 
breakpoints inside of ThreadX itself. Of course, this costs some 
performance. To make it run faster, you can change the ThreadX library
project to enable various compiler optimizations.

In addition, you can eliminate the ThreadX basic API error checking by 
compiling your application code with the symbol TX_DISABLE_ERROR_CHECKING 
defined. 


6.  Interrupt Handling

ThreadX provides complete and high-performance interrupt handling for Cortex-M
targets. There are a certain set of requirements that are defined in the 
following sub-sections:


6.1  Vector Area

The Cortex-M vectors start at the label __tx_vectors or similar. The application may modify
the vector area according to its needs. There is code in tx_initialize_low_level() that will 
configure the vector base register. 


6.2 Managed Interrupts

A ThreadX managed interrupt is defined below. By following these conventions, the 
application ISR is then allowed access to various ThreadX services from the ISR.
Here is the standard template for managed ISRs in ThreadX:


        .global  __tx_IntHandler
        .thumb_func
__tx_IntHandler:
; VOID InterruptHandler (VOID)
; {
        PUSH    {r0, lr}

;    /* Do interrupt handler work here */
;    /* BL <your interrupt routine in C> */

        POP     {r0, lr}
        BX      lr
; }


Note: the Cortex-M requires exception handlers to be thumb labels, this implies bit 0 set.
To accomplish this, the declaration of the label has to be preceded by the assembler directive
.thumb_func to instruct the linker to create thumb labels. The label __tx_IntHandler needs to 
be inserted in the correct location in the interrupt vector table. This table is typically 
located in either your runtime startup file or in the tx_initialize_low_level.S file.


7. FPU Support

ThreadX for Cortex-M supports automatic ("lazy") VFP support, which means that applications threads 
can simply use the VFP and ThreadX automatically maintains the VFP registers as part of the thread 
context - no additional setup by the application.


8.  Revision History

For generic code revision information, please refer to the readme_threadx_generic.txt
file, which is included in your distribution. The following details the revision
information associated with this specific port of ThreadX:

06-02-2021  Initial ThreadX version 6.1.7 for Cortex-M using AC6 tools.


Copyright(c) 1996-2021 Microsoft Corporation


https://azure.com/rtos

