*****************************************************************************
** ChibiOS/RT port for ARM-Cortex-M0 OSHChip (nRF51822).                  **
*****************************************************************************

** TARGET **

The demo runs on an OSHChip_V1.0 board. This board is powered by a Nordic
Semiconductor nRF51822 processor which is an ARM Cortex-M0 with bluetooth radio
hardware. For information about the board, see http://oshchip.org/

** The Demo **

This demo will print the standard TestThread output over UART (TX is DIP Pin 1) and sequentially blink all three LEDs.

** Build Procedure **

The demo has been tested using the freely available GCC ARM Embedded toolchain.

** Notes **

If the Mass Storage mechanism of the official OSHChip CMSIS-DAP debugger is desired for flashing the firmware, the hex file may need patching to be accepted by the Mass Storage flasher; see `patch_hex.sh`.
