                      Microsoft's Azure RTOS ThreadX for Cortex-M23 

                                   Using the GNU Tools


1.  Building the ThreadX run-time Library

An example .bat file is in the example_build directory.
Files tx_thread_stack_error_handler.c and tx_thread_stack_error_notify.c 
replace the common files of the same name. 

2.  Demonstration System

No demonstration example is provided.


3.  System Initialization

The entry point in ThreadX for the Cortex-M23 using gnu tools uses the standard GNU 
Cortex-M23 reset sequence. From the reset vector the C runtime will be initialized.

The ThreadX tx_initialize_low_level.S file is responsible for setting up 
various system data structures, the vector area, and a periodic timer interrupt 
source. 

In addition, _tx_initialize_low_level determines the first available 
address for use by the application, which is supplied as the sole input 
parameter to your application definition function, tx_application_define.


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

ThreadX provides complete and high-performance interrupt handling for Cortex-M23
targets. There are a certain set of requirements that are defined in the 
following sub-sections:


6.1  Vector Area

The Cortex-M23 vectors start at the label __tx_vectors or similar. The application may modify
the vector area according to its needs. There is code in tx_initialize_low_level() that will 
configure the vector base register. 


6.2 Managed Interrupts

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


Note: the Cortex-M23 requires exception handlers to be thumb labels, this implies bit 0 set.
To accomplish this, the declaration of the label has to be preceded by the assembler directive
.thumb_func to instruct the linker to create thumb labels. The label __tx_IntHandler needs to 
be inserted in the correct location in the interrupt vector table. This table is typically 
located in either your runtime startup file or in the tx_initialize_low_level.S file.


7.  Revision History

For generic code revision information, please refer to the readme_threadx_generic.txt
file, which is included in your distribution. The following details the revision
information associated with this specific port of ThreadX:

06-02-2021  Release 6.1.7 changes:
            tx_thread_secure_stack_initialize.S New file
            tx_thread_schedule.S                Added secure stack initialize to SVC hander
            tx_thread_secure_stack.c            Fixed stack pointer save, initialize in handler mode

04-02-2021  Release 6.1.6 changes:
            tx_port.h                           Updated macro definition
            tx_thread_schedule.s                Added low power support

03-02-2021  The following files were changed/added for version 6.1.5:
            tx_port.h                       Added ULONG64_DEFINED

12-31-2020  The following files were 
            changed/added for port specific version 6.1.3:
            
            tx_port.h                       Remove unneeded include files, 
                                            use builtin functions,
                                            modified comments.
                                            
            tx_thread_secure_stack.c        Remove unneeded include file, 
                                            use inline get/set functions,
                                            modified comments.

09-30-2020  Initial ThreadX 6.1 version for Cortex-M23 using GNU tools.


Copyright(c) 1996-2020 Microsoft Corporation


https://azure.com/rtos

