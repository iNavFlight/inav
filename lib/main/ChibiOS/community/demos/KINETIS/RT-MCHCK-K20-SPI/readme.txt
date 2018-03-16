*****************************************************************************
** ChibiOS/RT port for ARM-Cortex-M4 MCHCK K20 .                           **
*****************************************************************************

** TARGET **

The demo runs on an MKHCK K20 board. It use the SPI bus to send the
Manufacturer and Device ID Read command (0x9f) and to read the returned
data from a standard SPI data flash device. It has been tested with
an AT45DB081.

The pin connections are

  C5 is connected to SCK
  C6 is connected to MOSI
  D3 is connected to MISO
  C0 is connected to SS
