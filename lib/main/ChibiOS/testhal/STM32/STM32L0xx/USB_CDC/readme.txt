*****************************************************************************
** ChibiOS/HAL - USB-CDC driver demo for STM32F072.                        **
*****************************************************************************

** TARGET **

The demo runs on an ST STM32 Nycleo-64 L073RZ board and requires a USB 
break-out board having PA11 connected to USB_DM and PA12 connected to USB_DP.

** The Demo **

The application demonstrates the use of the STM32L073 USB driver.

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
