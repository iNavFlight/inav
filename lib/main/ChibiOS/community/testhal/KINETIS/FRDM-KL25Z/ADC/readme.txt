*****************************************************************************
** ChibiOS/RT port for ARM-Cortex-M0+ Freedom Board KL25Z.                 **
*****************************************************************************

The demo runs on an Freescale Freedom KL25Z board and demonstrates
the usage of the ADC.

It reads the internal temperature sensor. If the temperature drops
below 20C (68F) it turns on the blue LED. If the temperature rises
above 27C (81F) it turns on the red LED. Otherwise the green LED is
illuminated.

The internal bandgap voltage reference is used to calibrate the
results returned from the temperature sensor.
