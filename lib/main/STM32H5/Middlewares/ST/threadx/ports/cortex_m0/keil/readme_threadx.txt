                     Microsoft's Azure RTOS ThreadX for Cortex-M0 

             Using ARM Compiler 5 (AC5) and Keil Microcontroller Development Kit

1.  Building the ThreadX run-time Library

Building the ThreadX library is easy, simply load the project file 
tx.uvprojx, which is located inside the "example_build" directory. 

Once the ThreadX library files are displayed in the project window,
select the "Build Target" operation and observe the compilation and assembly
of the ThreadX library. This project build produces the ThreadX library 
file tx.lib.


2.  Demonstration System

The ThreadX demonstration is designed to execute under the Keil simulator or
Cortex-M0 hardware. This demonstration is slightly smaller than typical ThreadX 
demonstrations, and thus requires less than 7KB of Flash and less than 4KB of RAM.

Building the demonstration is easy; simply open the ThreadX demonstration 
project file sample_threadx.uvprojx, which is located inside the "example_build" 
directory. 

Once open, select the "Build Target" operation and observe the compilation of 
sample_threadx.c (which is the demonstration application) and linking with 
tx.lib. The resulting file sample_threadx.axf is a binary file that can be downloaded
and executed under the uVision simulator or Cortex-M0 hardware. 

For simulator execution, the following memory regions need to be defined via
the "Debug -> Memory Map" dialog:

0x20000000, 0x20080000 [check read and write access]
0xE0000000, 0xE8000000 [check read and write access]


3.  System Initialization

The entry point in ThreadX for the Cortex-M0 using AC5 tools is at label 
__main. This is defined within the AC5 compiler's startup code. In 
addition, this is where all static and global pre-set C variable 
initialization processing takes place.

The ThreadX tx_initialize_low_level.s file is responsible for setting up 
various system data structures, the vector area, and a periodic timer interrupt 
source. 

In addition, _tx_initialize_low_level determines the first available 
address for use by the application, which is supplied as the sole input 
parameter to your application definition function, tx_application_define.


4.  Register Usage and Stack Frames

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


5.  Improving Performance

The distribution version of ThreadX is built without any compiler 
optimizations. This makes it easy to debug because you can trace or set 
breakpoints inside of ThreadX itself. Of course, this costs some 
performance. To make it run faster, you can change the build_threadx.bat file to 
remove the -g option and enable all compiler optimizations. 

In addition, you can eliminate the ThreadX basic API error checking by 
compiling your application code with the symbol TX_DISABLE_ERROR_CHECKING 
defined. 


6.  Interrupt Handling

ThreadX provides complete and high-performance interrupt handling for Cortex-M0
targets. There are a certain set of requirements that are defined in the 
following sub-sections:


6.1  Vector Area

The Cortex-M0 vectors start at the label __tx_vectors. The application may modify
the vector area according to its needs.

6.2 Managed Interrupts

ISRs for Cortex-M can be written completely in C (or assembly language) without any
calls to _tx_thread_context_save or _tx_thread_context_restore. These ISRs are allowed
access to the ThreadX API that is available to ISRs.

ISRs written in C will take the form (where "your_C_isr" is an entry in the vector table):

void    your_C_isr(void)
{

    /* ISR processing goes here, including any needed function calls.  */
}

ISRs written in assembly language will take the form:

    EXPORT  your_assembly_isr
your_assembly_isr

    PUSH    {r0, lr}

    ; ISR processing goes here, including any needed function calls.

    POP     {r0, r1}
    MOV     lr, r1
    BX      lr


7.  Revision History

For generic code revision information, please refer to the readme_threadx_generic.txt
file, which is included in your distribution. The following details the revision
information associated with this specific port of ThreadX:

04-02-2021  Release 6.1.6 changes:
            tx_port.h                           Updated macro definition
            tx_thread_schedule.s                Fix compilation error

03-02-2021  The following files were changed/added for version 6.1.5:
            tx_thread_schedule.s            Added low power feature

09-30-2020  Initial ThreadX 6.1 version for Cortex-M0 using Keil tools.


Copyright(c) 1996-2020 Microsoft Corporation


https://azure.com/rtos

