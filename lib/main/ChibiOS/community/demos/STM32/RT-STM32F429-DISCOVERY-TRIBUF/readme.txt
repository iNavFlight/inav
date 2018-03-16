*****************************************************************************
** ChibiOS/RT port for ARM-Cortex-M4 STM32F429.                            **
*****************************************************************************

** TARGET **

The demo runs on an ST STM32F429I-Discovery board.

** The Demo **

This demo shows how to use a triple buffer handler, with one writer thread and
one reader thread.
The writer thread puts a character into the current back buffer, thus swapping
the back buffer with the orphan buffer for a new write. The writer then sleeps
for a specified delay in milliseconds.
The reader thread gets waits (if there is a timeout) until the orphan buffer
contains available data, becoming the new front buffer. The character is read
from the new front buffer and printed. The reader then sleeps for a specified
delay in milliseconds.
A simple command shell is activated on virtual serial port SD1 or SDU1.
Via command line it is possible to start, stop, set the delay, and set the
thread priority of the reader and writer threads.
The reader can also be assigned a wait timeout in milliseconds, with special
cases of "*" for infinite timeout, and "-" (or 0 ms) for none.

** Build Procedure **

The demo has been tested by using the free GNU Tools ARM Embedded toolchain
and ChibiStudio. Just modify the TRGT line in the makefile in order to use
different GCC toolchains.
