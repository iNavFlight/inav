*****************************************************************************
** ChibiOS/RT HAL - 1-Wire driver demo for STM32F1xx.                      **
*****************************************************************************

** TARGET **

The demo will on an Olimex STM32_103STK board.

** The Demo **

The application demonstrates the use of the STM32F1xx 1-Wire driver.

** Board Setup **

To use demo you have to power your 1-wire device from 5V bus on board
and connect DQ line to PB0 pin. Do not forget about external pullup
resistor to 5V (4k7 recommended).

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
