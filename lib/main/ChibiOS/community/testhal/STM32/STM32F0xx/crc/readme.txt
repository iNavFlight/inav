*****************************************************************************
** ChibiOS/HAL - CRC driver demo for STM32F0xx (also sw driver)            **
*****************************************************************************

** TARGET **

The demo runs on an ST STM32F0-Discovery board.

** The Demo **

The application demonstrates the use of the STM32F0xx CRC driver.  There are
many different ways to configure and setup the CRC.  This demo has be
configured to test the following:
  * ST hardware block configured with CRC32 with or without DMA
  * ST hardware block configured with CRC16 with or without DMA
  * Software CRC32
  * Software CRC16

** Board Setup **

- No requirements

** Build Procedure **

The demo has been tested using the free Codesourcery GCC-based toolchain
and YAGARTO.
Just modify the TRGT line in the makefile in order to use different GCC ports.

** Notes **

Some files used by the demo are not part of ChibiOS/RT but are copyright of
ST Microelectronics and are licensed under a different license.
Also note that not all the files present in the ST library are distributed
with ChibiOS/RT, you can find the whole library on the ST web site:

                             http://www.st.com
