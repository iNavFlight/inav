                         Microsoft's Azure RTOS ThreadX for Linux 

                              Using the GNU GCC Tools

1.  Building the ThreadX run-time Library

First make sure you are in the "example_build" directory. Also, make sure that
you have setup your path and other environment variables necessary for the GNU
development environment. The following command retrieves and installs GCC
multilib on a Ubuntu system:
 
sudo apt-get install gcc-multilib

At this point you may run the GNU make command to build the ThreadX core 
library. This will build the ThreadX run-time environment in the 
"example_build" directory. 

   make tx.a

you should now observe the compilation of the ThreadX library source. At the 
end of the make, they are all combined into the run-time library file: tx.a.
This file must be linked with your application in order to use ThreadX.
 

2.  Demonstration System

Building the demonstration is easy; simply execute the GNU make command while 
inside the "example_build" directory. 

   make sample_threadx

You should observe the compilation of sample_threadx.c (which is the demonstration 
application) and linking with tx.a. The resulting file DEMO is a binary file 
that can be executed.


3.  System Initialization

The system entry point is at main(), which is defined in the application. 
Once the application calls tx_kernel_enter, ThreadX starts running and 
performs various initialization duties prior to starting the scheduler. The 
Linux-specific initialization is done in the function _tx_initialize_low_level,
which is located in the file tx_initialize_low_level.c. This function is 
responsible for setting up various system data structures and simulated 
interrupts - including the periodic timer interrupt source for ThreadX.

In addition, _tx_initialize_low_level determines the first available 
address for use by the application. In Linux, this is basically done
by using malloc to get a big block of memory from Linux.


4.  Linux Implementation

ThreadX for Linux is implemented using POSIX pthreads. Each application
thread in ThreadX actually runs as a Linux pthread. The determination of
which application thread to run is made by the ThreadX scheduler, which 
itself is a Linux pthread. The ThreadX scheduler is the highest priority 
thread in the system.

Interrupts in ThreadX/Linux are also simulated by pthreads. A good example
is the ThreadX system timer interrupt, which can be found in 
tx_initialize_low_level.c.

ThreadX for linux utilizes the API pthread_setschedparam() which requires
the ThreadX application running with privilege. The following command is used
to run a ThreadX application:

./sample_threadx

5.  Improving Performance

The distribution version of ThreadX is built without any compiler 
optimizations. This makes it easy to debug because you can trace or set 
breakpoints inside of ThreadX itself. Of course, this costs some 
performance. To make it run faster, you can change the makefile to 
enable all compiler optimizations. In addition, you can eliminate the 
ThreadX basic API error checking by compiling your application code with the 
symbol TX_DISABLE_ERROR_CHECKING defined.


6.  Interrupt Handling

ThreadX provides simulated interrupt handling with Linux pthreads. Simulated
interrupt threads may be created by the application or may be added to the
simulated timer interrupt defined in tx_initialize_low_level.c. The following 
format for creating simulated interrupts should be used:

6.1  Data structures

Here is an example of how to define the Linux data structures and prototypes
necessary to create a simulated interrupt thread:

pthread_t    _sample_linux_interrupt_thread;
void        *_sample_linux_interrupt_entry(void *p);

6.2  Creating a Simulated Interrupt Thread

Here is an example of how to create a simulated interrupt thread in Linux.
This may be done inside of tx_initialize_low_level.c or from your application code


struct sched_param sp;

       /* Create the ISR thread */
       pthread_create(&_sample_linux_interrupt_thread, NULL, _sample_linux_interrupt_entry, &_sample_linux_interrupt_thread);

       /* Set up the ISR priority */
       sp.sched_priority = TX_LINUX_PRIORITY_ISR;
       pthread_setschedparam(_sample_linux_interrupt_thread, SCHED_FIFO, &sp);



6.3  Simulated Interrupt Thread Template

The following is a template for the simulated interrupt thread. This interrupt will occur on
a periodic basis. 

void    *_sample_linux_interrupt_entry(void *p)
{
struct timespec ts;

    while(1)
    {

        ts.tv_sec = 0;
        ts.tv_nsec = 10000;
        while(nanosleep(&ts, &ts));

        /* Call ThreadX context save for interrupt preparation.  */
        _tx_thread_context_save();

        /* Call the real ISR routine */
        _sample_linux_interrupt_isr();

        /* Call ThreadX context restore for interrupt completion.  */
        _tx_thread_context_restore();
    } 
}



7.  Revision History

For generic code revision information, please refer to the readme_threadx_generic.txt
file, which is included in your distribution. The following details the revision
information associated with this specific port of ThreadX:

09-30-2020  Initial ThreadX 6.1 version for Linux using GNU GCC tools.


Copyright(c) 1996-2020 Microsoft Corporation


https://azure.com/rtos

