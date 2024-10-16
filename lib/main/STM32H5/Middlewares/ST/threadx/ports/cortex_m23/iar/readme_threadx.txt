                       Microsoft's Azure RTOS ThreadX for Cortex-M23 

                                   Using the IAR Tools


1.  Building the ThreadX run-time Library

Import all ThreadX common and port-specific source files into an IAR project.
Configure the project to build a library rather than an executable. This 
results in the ThreadX run-time library file tx.a, which is needed by 
the application.
Files tx_thread_stack_error_handler.c and tx_thread_stack_error_notify.c 
replace the common files of the same name. 

2.  Demonstration System

No demonstration is provided because the IAR EWARM 8.50 simulator does
not simulate the Cortex-M23 correctly. 


3.  System Initialization

The entry point in ThreadX for the Cortex-M23 using IAR tools is at label 
__iar_program_start. This is defined within the IAR compiler's startup code. 
In addition, this is where all static and global preset C variable 
initialization processing takes place.

The ThreadX tx_initialize_low_level.s file is responsible for setting up 
various system data structures, and a periodic timer interrupt source. 

The _tx_initialize_low_level function inside of tx_initialize_low_level.s
also determines the first available address for use by the application, which 
is supplied as the sole input parameter to your application definition function, 
tx_application_define. To accomplish this, a section is created in 
tx_initialize_low_level.s called FREE_MEM, which must be located after all 
other RAM sections in memory.


4.  Register Usage and Stack Frames

The following defines the saved context stack frames for context switches
that occur as a result of interrupt handling or from thread-level API calls.
All suspended threads have the same stack frame in the Cortex-M23 version of
ThreadX. The top of the suspended thread's stack is pointed to by 
tx_thread_stack_ptr in the associated thread control block TX_THREAD.

  Stack Offset     Stack Contents 

     0x00               LR          Interrupted LR (LR at time of PENDSV)
     0x04               r8
     0x08               r9
     0x0C               r10
     0x10               r11
     0x14               r4
     0x18               r5
     0x1C               r6
     0x20               r7
     0x24               r0          (Hardware stack starts here!!)
     0x28               r1
     0x2C               r2
     0x30               r3
     0x34               r12
     0x38               lr
     0x3C               pc
     0x40               xPSR


5.  Improving Performance

To make ThreadX and the application(s) run faster, you can enable 
all compiler optimizations. 

In addition, you can eliminate the ThreadX basic API error checking by 
compiling your application code with the symbol TX_DISABLE_ERROR_CHECKING 
defined. 


6.  Interrupt Handling

The Cortex-M23 vectors start at the label __vector_table and is typically defined in a 
startup.s file (or similar). The application may modify the vector area according to its needs.


6.1 Managed Interrupts

ISRs for Cortex-M using the IAR tools can be written completely in C (or assembly
language) without any calls to _tx_thread_context_save or _tx_thread_context_restore.
These ISRs are allowed access to the ThreadX API that is available to ISRs.

ISRs written in C will take the form (where "your_C_isr" is an entry in the vector table):

void    your_C_isr(void)
{

    /* ISR processing goes here, including any needed function calls.  */
}

ISRs written in assembly language will take the form:

    PUBLIC  your_assembly_isr
your_assembly_isr:

    PUSH    {r0, lr}

    ; ISR processing goes here, including any needed function calls.

    POP     {r0, r1}
    MOV     lr, r1
    BX      lr


7.  IAR Thread-safe Library Support

Thread-safe support for the IAR tools is easily enabled by building the ThreadX library
and the application with TX_ENABLE_IAR_LIBRARY_SUPPORT. Also, the linker control file
should have the following line added (if not already in place):

initialize by copy with packing = none { section __DLIB_PERTHREAD }; // Required in a multi-threaded application


7.  IAR Thread-safe Library Support

Thread-safe support for the IAR tools is easily enabled by building the ThreadX library
and the application with TX_ENABLE_IAR_LIBRARY_SUPPORT. Also, the linker control file
should have the following line added (if not already in place):

initialize by copy with packing = none { section __DLIB_PERTHREAD }; // Required in a multi-threaded application

The project options "General Options -> Library Configuration" should also have the 
"Enable thread support in library" box selected.


8.  Revision History

For generic code revision information, please refer to the readme_threadx_generic.txt
file, which is included in your distribution. The following details the revision
information associated with this specific port of ThreadX:

06-02-2021  Release 6.1.7 changes:
            tx_thread_secure_stack_initialize.s New file
            tx_thread_schedule.s                Added secure stack initialize to SVC hander
            tx_thread_secure_stack.c            Fixed stack pointer save, initialize in handler mode

04-02-2021  Release 6.1.6 changes:
            tx_port.h                           Updated macro definition
            tx_thread_schedule.s                Added low power support

03-02-2021  The following files were changed/added for version 6.1.5:
            tx_port.h                       Added ULONG64_DEFINED

09-30-2020  Initial ThreadX 6.1 version for Cortex-M23 using IAR's ARM tools.


Copyright(c) 1996-2020 Microsoft Corporation


https://azure.com/rtos

