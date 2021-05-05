
# Board - MATEKSYS F405 family

![Matek F405-AIO](http://www.mateksys.com/wp-content/uploads/2017/06/F405-AIO_2.jpg)

### [Matek F405-CTR](https://inavflight.com/shop/p/MATEKF405CTR)
### [Matek F405-STD](https://inavflight.com/shop/p/MATEKF405STD)
### [Matek F405-AIO (end-of-life)](http://www.mateksys.com/?portfolio=f405-aio)
### [Matek F405-OSD (end-of-life)](http://www.mateksys.com/?portfolio=f405-osd)

## Firmware

Due to differences on the board (I2C - see below), there are two firmware variants:

| Board  | Firmware |
| ------ | -------- |
| Matek F405-AIO, STD, CTR | inav_x.y.z_MATEKF405.hex<br/>inav_x.y.z_MATEKF405_SERVOS6.hex |
| Matek F405-OSD | inav_x.y.z_MATEKF405OSD.hex |

## Hardware features

* MCU: STM32F405RGT6, 168MHz
* IMU: ICM-20602 (OSD, AIO, STD), MPU6000 (CTR)
* OSD: BetaFlight OSD w/ AT7456E chip
* Baro: BMP280 (STD, CTR)
* Compass: No
* Blackbox: MicroSD card slot
* VCP, UART1, UART2, UART3, UART4, UART5
* Built in inverter for SBUS input (UART2-RX)
* PPM/UART Shared: UART2-RX
* Battery Voltage Sensor
* I2C SDA & SCL: Yes
* LDO: 3.3V Max. 300mA
* Current Sensor: Yes (AIO, CTR), otherwise FCHUB-6S option
* Integrated Power Distribution Board: Yes (AIO, CTR), otherwise FCHUB-6S option
* Integrated Voltage Regulator: 5V 2A & 9V 2A (AIO), 5V 2A (CTR), otherwise FCHUB-6S option
* 6 PWM / DSHOT capable outputs
* WS2812 Led Strip : Yes
* Beeper: Yes
* RSSI: Yes
* Side-press button for BOOT(DFU) mode
* Anti-vibration Standoffs

### I2C

The F405-AIO, STD, CTR boards expose dedicated I2C pads.
The F405-OSD does not expose I2C. For iNav there is a software I2C provision using the USART3 pads, as:

* SDA => RX3, SCL => TX3
* Do not assign any serial function to USART3

### PWM and Servos

Due to the available hardware, please note:

#### Flying Wing

* S1 : ESC
* S2 : LEFT elevon
* S3 : RIGHT elevon

#### Tricopter

* S1 : Tail Servo
* S2 : Motor 1
* S3 : Motor 2
* S4 : Motor 3

If you need servo connected to S6 pin while keeping motors on S1..S4 pins (e.g. camera tilt on quadcopter), please flash MATEKF405_SERVOS6 firmware.

I2C requires that the WS2812 led strip is moved to S5, thus WS2812 is not usable on hexcopter and similar.

### Soft Serial

Soft serial is available as an alternative to a hardware UART on RX4/TX4 and TX2. By default this is NOT inverted. In order to use this feature:

* Enable soft serial
* Do not assign any function to hardware UART4
* Assign the desired function to the soft-serial ports
* UART4 TX/RX pads will be used as SoftSerial 1 TX/RX pads
* UART2 TX pad will be used as SoftSerial 2 TX pad

RX2 and SBUS pads can be used as normal for receiver-only UART. If you need a full duplex UART (IE: TBS Crossfire) and SoftSerial, then use UART 1, 3 or 5 for Full Duplex.

#### Common scenarios for SoftSerial on this boards:
You need to wire a FrSky receiver (SBUS and SmartPort) to the Flight controller
* Connect SBUS from Receiver to SBUS pin on the Flight Controller
* Connect SmartPort from Receiver to TX2 pad on the Flight Controller
* Enable SoftSerial and set UART2 for Serial RX and SoftSerial 2 for SmartPort Telemetry

You need to wire a SmartAudio or Trump VTX to the Flight controller
* Connect SmartAudio/Tramp from VTX to the TX4 pad on the Flight Controller
* Enable SoftSerial and set SoftSerial 1 for SmartAudio or Tramp

### USB

This board uses STM32 VCP and does _not_ use a UART when USB is connected. STM32 VCP drivers might be required on some operating systems.

Flashing requires DFU mode and STM32 DFU drivers. On Linux, the configurator or `dfu-util` work with a `udev` rule.

````
# DFU (Internal bootloader for STM32 MCUs)
SUBSYSTEM=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="df11", MODE="0664", GROUP="plugdev"
ATTRS{idVendor}=="0483", ATTRS{idProduct}=="5740", ENV{ID_MM_DEVICE_IGNORE}="1"
````

On Windows, it may be necessary to use the [Zadig](http://zadig.akeo.ie) tool to install the WinUSB driver.

## Manufacturer

Matek Systems: http://www.mateksys.com/

## Distributors

* [Matek F405-CTR](https://inavflight.com/shop/p/MATEKF405CTR)
* [Matek F405-STD](https://inavflight.com/shop/p/MATEKF405STD)

## FAQ & Known Issues

Rcgroups Thread Matek F405: https://www.rcgroups.com/forums/showthread.php?2889298-MATEKSYS-Flight-Controller-F405-OSD-32K-Gyro-5xUARTs-SD-Slot

Rcgroups Thread Matek F405-AIO: https://www.rcgroups.com/forums/showthread.php?2912273-Matek-Flight-Controller-F405-AIO

This board doesn't have hardware inverters for UART pins. This page explains getting SmartPort telemetry working on F4 board:  https://github.com/iNavFlight/inav/blob/master/docs/Telemetry.md
