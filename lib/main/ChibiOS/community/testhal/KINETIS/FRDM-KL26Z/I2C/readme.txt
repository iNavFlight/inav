*********************************************************************
** ChibiOS/RT I2C test demo for Freedom Board KL26Z.               **
*********************************************************************

** TARGET **

The test runs on an Freescale Freedom KL26Z board.

** The Demo **

This test tries to access the onboard FXOS8700CQ chip using the I2C bus.
It sends the command WHO_AM_I which has a standard answer that can be
verified. If the correct answer is received the GREEN led will blink.
If no answer or invalid answer is received the RED led will blink.

** Build Procedure **

This test was built using the ARM GCC toolchain available at:

https://launchpad.net/gcc-arm-embedded
