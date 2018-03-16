*****************************************************************************
** ChibiOS/RT port for ARM-Cortex-M4 STM32F429.                            **
*****************************************************************************

** TARGET **

The demo runs on an ST STM32F429I-Discovery board.

** The Demo **

A simple command shell is activated on virtual serial port SD1.
The demo makes use of FMC, LTDC, and DMA2D peripherals to show graphical
contents on the display of the board, composed both on the on-chip SRAM
and the on-board SDRAM.

** Build Procedure **

The demo has been tested by using the free Codesourcery GCC-based toolchain
and YAGARTO. just modify the TRGT line in the makefile in order to use
different GCC toolchains.

** Notes **

Some files used by the demo are not part of ChibiOS/RT but are copyright of
ST Microelectronics and are licensed under a different license.
Also note that not all the files present in the ST library are distributed
with ChibiOS/RT, you can find the whole library on the ST web site:

                             http://www.st.com
