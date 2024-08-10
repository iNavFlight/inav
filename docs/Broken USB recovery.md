# Broken USB recovery 

It is possible to flash INAV without USB over UART 1 or 3.

## Prerequisites:
- USB/UART adapter (FT232, CP2102, etc.)
- STM32 Cube Programmer (https://www.st.com/en/development-tools/stm32cubeprog.html)

To gain access to the FC via Configurator, MSP must be activated on a UART as standard. Some FCs already have this enabled by default, if not a custom firmware must be created.

The following targets have MSP activated on a UART by default:

| Target | Standard MSP Port |
|-----------| ----------- |
| AOCODARCF4V3 | UART5 |
| ATOMRCF405NAVI_DELUXE | UART1 |
| FF_F35_LIGHTNING | UART1 |
| FLYCOLORF7V2 | UART4 |
| GEPRCF405_BT_HD | UART5* |
| GEPRCF722_BT_HD | UART4* |
| IFLIGHT_BLITZ_F7_AIO | UART1 |
| JHEMCUF405WING | UART6 |
| JHEMCUH743HD | UART4 |
| KAKUTEH7 | UART1 and UART2* |
| KAKUTEH7WING | UART6 |
| MAMBAF405_2022A | UART4 |
| MAMBAF405US | UART4 |
| MAMBAF722 | UART4 |
| MAMBAF722 APP | UART4*|
| MAMBAF722WING | UART4 |
| MAMBAF722_X8 | UART4 |
| MAMBAH743 | UART4* |
| MATEKF405SE | UART1 |
| NEUTRONRCH743BT | UART3* |
| SDMODELH7V1 | UART1 and UART2 |
| SKYSTARSH743HD | UART4 |
| SPEEDYBEEF4 | UART5* |
| SPEEDYBEEF405MINI | UART4* |
| SPEEDYBEEF405V3 | UART4* |
| SPEEDYBEEF405V4 | UART4* |
| SPEEDYBEEF405WING | UART6 |
| SPEEDYBEEF7 | UART6 |
| SPRACINGF4EVO | UART1 |
| TMOTORF7V2 | UART5 |

(*) No Pads/Pins, Port is used interally (Bluetooth)

## Custom firmware:

If the FC does not have MSP activated on a UART by default or does not have a connector for it, a custom firmware must be built. 
The following procedure describes the process under Windows 10/11:

Please read [Building in Windows 2010 or 11 with Linux Subsystem](https://github.com/iNavFlight/inav/blob/master/docs/development/Building%20in%20Windows%2010%20or%2011%20with%20Linux%20Subsystem.md)
and follow the instructions up to "Building with Make".

In the step 'prepare build environment' add the option `-DMSP_UART=SERIAL_PORT_USARTX` to `cmake`

Replace the X in SERIAL_PORT_USARTX with the number of UART/serial port on which MSP is to be activated.

Example:
For UART 2: `cmake -DMSP_UART=SERIAL_PORT_USART2 ..`
For UART 3: `cmake -DMSP_UART=SERIAL_PORT_USART3 ..`
etc.

Build the firmware as described in the document above (`make [YOUR_TARGET]`).

## Flashing via Uart:

1. Disconnect ALL peripherals and the USB Cable from the FC. To power the FC use a battery or use the 5V provided from the USB/Serial Converter. 
2. Connect UART 1 or 3 (other UARTS will not work) and GND to the USB/Serial converter (RX -> TX, TX -> RX)
3. Keep the boot/dfu button pressed
4. Switch on the FC / supply with power
5. Start STM32 CubeProgrammer and go to "Erasing & Programming", second option in the menu.
6. Select UART (blue dropdown field) and select the COM port of the USB/Serial adapter and press "Connect". The corresponding processor should now be displayed below.
7. Click on "Full flash erase". This is also necessary if you are flashing the same firmware version that was previously on the FC, otherwise MSP may not be activated on the UART.
8. Under "Download" load the previously created firmware (`INAV_X.X.X_[Your Target].hex`) or the standard firmware if UART is already activated there. The option "Verify programming" is optional but recommended. Make sure that "Skip flash erase while programming" is NOT activated.
9. Click "Start Programming"

After the process is completed, switch the FC off and on again and then the Configurator can connect to the FC via USB/serial adapter and the previously configured UART.
