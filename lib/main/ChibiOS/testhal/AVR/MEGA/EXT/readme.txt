*****************************************************************************
** ChibiOS/RT port for Atmel AVR ATmega2560.                               **
*****************************************************************************

** TARGET **

The demo runs on an Arduino Mega2560 board.

** The Demo **

The demo currently just prints the number of External Interruption detected
on the Serial0, which is available on the board USB connector
(FT232 converter). The LED on PB7 (pin 13 on Arduino IDE) is also toggle on
every interruption. A switch is connected to INT4, the D2 pin on the Arduino
Mega2560 board.

** Build Procedure **

The demo was built using the GCC AVR toolchain.
