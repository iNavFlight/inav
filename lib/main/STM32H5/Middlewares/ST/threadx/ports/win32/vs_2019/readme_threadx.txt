                         Microsoft's Azure RTOS ThreadX for Win32 

                              Using the Visual Studio Tools


1. Open the ThreadX Project Workspace

In order to build the ThreadX library and the ThreadX demonstration first load 
the Azure RTOS Workspace azure_rtos.sln, which is located inside the "example_build"
directory. 


2. Building the ThreadX run-time Library

Building the ThreadX library is easy; simply make the "tx" project active and 
then select the build button. You should now observe the compilation of the 
ThreadX library source. This project build produces the ThreadX library file 
tx.lib.


3.  Building the Demonstration System

You are now ready to run the ThreadX Win32 demonstration. Simply make the 
"sample_thread" project active and then select the build button. When the build
is finished, select the run button from Visual Studio and observe various
demonstration statistics being printed to the console window. You may also set
breakpoints, single step, perform data watches, etc.


4.  System Initialization

The system entry point is at main(), which is defined in the application. 
Once the application calls tx_kernel_enter, ThreadX starts running and 
performs various initialization duties prior to starting the scheduler. The 
Win32-specific initialization is done in the function _tx_initialize_low_level,
which is located in the file tx_initialize_low_level.c. This function is 
responsible for setting up various system data structures and simulated 
interrupts - including the periodic timer interrupt source for ThreadX.

In addition, _tx_initialize_low_level determines the first available 
address for use by the application. In Win32, this is basically done
by using malloc to get a big block of memory from Windows.


5.  Win32 Implementation

ThreadX for Win32 is implemented using Win32 threads. Each application
thread in ThreadX actually runs as a Win32 thread. The determination of
which application thread to run is made by the ThreadX scheduler, which 
itself is a Win32 thread. The ThreadX scheduler is the highest priority 
thread in the system.

Interrupts in ThreadX/Win32 are also simulated by threads. A good example
is the ThreadX system timer interrupt, which can be found in 
tx_initialize_low_level.c.

5.1  ThreadX Limitations

ThreadX for Win32 behaves in the same manner as ThreadX in an embedded 
environment EXCEPT in the case of thread termination. Unfortunately, the 
Win32 API does not have a good mechanism to terminate threads and instead
must rely on the thread itself terminating. Hence, threads in the ThreadX
Win32 implementation must have some ThreadX call periodically in their 
processing if they can be terminated by another ThreadX thread.


6.  Improving Performance

The distribution version of ThreadX is built without any compiler 
optimizations. This makes it easy to debug because you can trace or set 
breakpoints inside of ThreadX itself. Of course, this costs some 
performance. To make it run faster, you can change the tx project file to 
enable all compiler optimizations. In addition, you can eliminate the 
ThreadX basic API error checking by compiling your application code with the 
symbol TX_DISABLE_ERROR_CHECKING defined.


7.  Interrupt Handling

ThreadX provides simulated interrupt handling with Win32 threads. Simulated
interrupt threads may be created by the application or may be added to the
simulated timer interrupt defined in tx_initialize_low_level.c. The following 
format for creating simulated interrupts should be used:

7.1  Data structures

Here is an example of how to define the Win32 data structures and prototypes
necessary to create a simulated interrupt thread:

HANDLE       _sample_win32_interrupt_handle;
DWORD        _sample_win32_interrupt_id;
DWORD WINAPI _sample_win32_interrupt(LPVOID);


7.2  Creating a Simulated Interrupt Thread

Here is an example of how to create a simulated interrupt thread in Win32.
This may be done inside of tx_initialize_low_level.c or from your application code

        _sample_win32_interrupt_handle =
            CreateThread(NULL, 0, _sample_win32_interrupt, (LPVOID) &_sample_win32_interrupt_handle,
                    CREATE_SUSPENDED, &_sample_win32_interrupt_id);

        SetThreadPriority(_sample_win32_interrupt_handle, THREAD_PRIORITY_BELOW_NORMAL);


7.3  Activating the Simulated Interrupt Thread

Simulated interrupt threads should not be activated until initialization is complete, i.e. until
ThreadX is ready to schedule threads. The following activation code may be added to the routine
in tx_initialize_low_level.c called _tx_initialize_start_interrupts or into the application code directly:

        ResumeThread(_sample_win32_interrupt_handle);


7.4  Simulated Interrupt Thread Template

The following is a template for the simulated interrupt thread. This interrupt will occur on
a periodic basis. 

DWORD WINAPI _sample_win32_interrupt(LPVOID *ptr)
{


    while(1)
    {

        /* Sleep for the desired time.  */
        Sleep(18);

        /* Call ThreadX context save for interrupt preparation.  */
        _tx_thread_context_save();

        /* Call application ISR here!  */

        /* Call ThreadX context restore for interrupt completion.  */
        _tx_thread_context_restore();
    } 
}


8.  Revision History

For generic code revision information, please refer to the readme_threadx_generic.txt
file, which is included in your distribution. The following details the revision
information associated with this specific port of ThreadX:

09-30-2020  Initial ThreadX version for Win32 using Microsoft Visual C/C++.


Copyright(c) 1996-2020 Microsoft Corporation


https://azure.com/rtos

