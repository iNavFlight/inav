                     Microsoft's Azure RTOS ThreadX for Cortex-M0 

                             Using ARM Compiler 6 & DS

1. Import the ThreadX Projects

In order to build the ThreadX library and the ThreadX demonstration, first import 
the 'tx' and 'sample_threadx' projects (located in the "example_build" directory) 
into your DS workspace.


2.  Building the ThreadX run-time Library

Building the ThreadX library is easy; simply right-click the Eclipse project 
"tx" and then select the "Build Project" button. You should now observe the compilation 
and assembly of the ThreadX library. This project build produces the ThreadX
library file tx.a.


3.  Demonstration System

The ThreadX demonstration is designed to execute under the DS debugger on the
MPS2_Cortex_M0 Bare Metal simulator.

Building the demonstration is easy; simply right-click the Eclipse project 
"sample_threadx" and then select the "Build Project" button. You should now observe 
the compilation and assembly of the ThreadX demonstration. This project build produces 
the ThreadX library file sample_threadx.axf. Next, expand the demo ThreadX project folder 
in the Project Explorer window, right-click on the 'cortex-m0_tx.launch' file, click
'Debug As', and then click 'cortex-m0_tx' from the submenu. This will cause the
debugger to load the sample_threadx.axf ELF file and run to main. You are now ready 
to execute the ThreadX demonstration.


4.  System Initialization

The entry point in ThreadX for the Cortex-M0 using AC6 tools uses the standard AC6 
Cortex-M0 reset sequence. From the reset vector the C runtime will be initialized.

The ThreadX tx_initialize_low_level.S file is responsible for setting up 
various system data structures, the vector area, and a periodic timer interrupt 
source. 

In addition, _tx_initialize_low_level determines the first available 
address for use by the application, which is supplied as the sole input 
parameter to your application definition function, tx_application_define.


5.  Register Usage and Stack Frames

The following defines the saved context stack frames for context switches
that occur as a result of interrupt handling or from thread-level API calls.
All suspended threads have the same stack frame in the Cortex-M0 version of
ThreadX. The top of the suspended thread's stack is pointed to by 
tx_thread_stack_ptr in the associated thread control block TX_THREAD.


  Stack Offset     Stack Contents 

     0x00               r8
     0x04               r9
     0x08               r10
     0x0C               r11
     0x10               r4
     0x14               r5
     0x18               r6
     0x1C               r7
     0x20               r0          (Hardware stack starts here!!)
     0x24               r1
     0x28               r2
     0x2C               r3
     0x30               r12
     0x34               lr
     0x38               pc
     0x3C               xPSR


6.  Improving Performance

The distribution version of ThreadX is built without any compiler optimizations. 
This makes it easy to debug because you can trace or set breakpoints inside of 
ThreadX itself. Of course, this costs some performance. To make it run faster, 
you can change the build_threadx.bat file to remove the -g option and enable 
all compiler optimizations. 

In addition, you can eliminate the ThreadX basic API error checking by 
compiling your application code with the symbol TX_DISABLE_ERROR_CHECKING 
defined. 


7.  Interrupt Handling

ThreadX provides complete and high-performance interrupt handling for Cortex-M0
targets. There are a certain set of requirements that are defined in the 
following sub-sections:


7.1  Vector Area

The Cortex-M0 vectors start at the label __tx_vectors or similar. The application may modify
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

        POP     {r0, r1}
        MOV     lr, r1
        BX      lr
; }


Note: the Cortex-M0 requires exception handlers to be thumb labels, this implies bit 0 set.
To accomplish this, the declaration of the label has to be preceded by the assembler directive
.thumb_func to instruct the linker to create thumb labels. The label __tx_IntHandler needs to 
be inserted in the correct location in the interrupt vector table. This table is typically 
located in either your runtime startup file or in the tx_initialize_low_level.S file.


8.  Revision History

For generic code revision information, please refer to the readme_threadx_generic.txt
file, which is included in your distribution. The following details the revision
information associated with this specific port of ThreadX:

04-02-2021  Release 6.1.6 changes:
            tx_port.h                           Updated macro definition

03-02-2021  The following files were changed/added for version 6.1.5:
            tx_thread_schedule.s            Added low power feature

09-30-2020  Initial ThreadX 6.1 version for Cortex-M0 using AC6 tools.


Copyright(c) 1996-2020 Microsoft Corporation


https://azure.com/rtos

