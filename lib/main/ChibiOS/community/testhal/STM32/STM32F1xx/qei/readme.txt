*****************************************************************************
** ChibiOS/HAL - qei driver demo for STM32F1xx.                         **
*****************************************************************************

** TARGET **

The demo runs on an Olimex STM32_103STK board.

** The Demo **

The application demonstrates the use of the STM32F1xx QEI encoder driver.

** Board Setup **

To use demo you have to connect an encoder to one of the timers that support 
the encoder mode to ch1 and ch2 and add an external pullup resistor to 3V3.
For good results add 100n capacitors to GND.


** Notes **

Some files used by the demo are not part of ChibiOS/RT but are copyright of
ST Microelectronics and are licensed under a different license.
Also note that not all the files present in the ST library are distributed
with ChibiOS/RT, you can find the whole library on the ST web site:

                             http://www.st.com
