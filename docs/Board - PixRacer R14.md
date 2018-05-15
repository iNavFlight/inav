# Board - PixRacer

PinOut diagrams are only for reference. See Ports and interfaces for true locations and mappings.

## Top View PinOut
![PixRacer](https://pixhawk.org/_media/modules/pixracer_r09_top_pinouts.png?cache=)

## Bottom View PinOut
![PixRacer](https://pixhawk.org/_media/modules/pixracer_r09_bot_pinouts.png?cache=&w=900&h=660&tok=a821d6)

Schematic : https://pixhawk.org/_media/modules/pixracer-r14.pdf

## How to Flash
PixRacer comes with NuttX Bootloader installed.
To flash inav follow the steps below
* Short 3.3 V pad and GND pad located at Top(near Motor Pins)
* Plug in via USB
* Either use ziadag to get the correct the drivers for inav based firmware flashing or use Dfuse to flash the correct firmware.
If you want to revert back then PixRacer factory Loaded Bootloader Bin File for Dfuse : https://github.com/mkschreder/ardupilot/tree/master/mk/PX4/bootloader (download px4fmu4_bl.bin) or Build your own from :https://github.com/PX4/Bootloader
Then follow this : https://pixhawk.org/dev/bootloader_update

## Features

### Processor and Sensors

* 32-bit STM32F427 Cortex M4 core with FPU rev. 3 168 MHz/256 KB RAM/2 MB Flash
* ICM-20608-G 3-axis accelerometer/gyroscope
* MPU-9250 3-axis accelerometer/gyroscope/magnetometer
* MEAS MS5611 barometer
* Honeywell HMC5983 magnetometer temperature compensated

### Interface and Ports

Total UARTS Recognized by iNav -> 8

#### USART1
* Location : Top
* Use: ESP8266 for Wifi OTA updates  in Arducopter and PX4 AutoPilot
* See Top View  to find TX & RX  
* Only 3.3 volt peripherals will work unless your provide your own 5V BEC

#### USART2
* Location : Top
* Use :Free Port
* Runcam Device/Smart Audio and any other UART Peripheral should be supported

#### USART3
* Location : Top
* Use :Free port
* Runcam Device/Smart Audio and any other UART Peripheral should be supported

#### USART 4
* Location :
* Use : I2C/GPS Port (WIP if you want to just use I2C)
* Recommended to actually leave this port as is and not modify wires
* GPS devices/ magnetometer and other I2C devices should be connected to this port
* Till I2C is made available use as Free UART

#### USART 5
* Location : Top?
* Use :CAN Bus?
* Recommendation :CAN ESC?

#### USART 6
* Location : Top
* Use : RC IN  
* Has RSS IN and and supports only PPM
* RSS IN board is connected to PC1 pin of the F4 controller
* RC input has been separated
* PPM RC input is on PB0
* Serial RC input is on PC7

#### UART 7
* Location : Bottom
* Use :JTAG connector for debugging and bootloader/firmware flash
* Unless you really know what you are doing leave this port alone
* If you _really_ need another UART be aware, this port is limited to 3.3 volt

#### USART 8
* Location : Top
* Use : Native FrSky port
* Recommendation : Use for native FrSky telemetry

## Buzzer
You can connect the buzzer provide from factory to the default buzzer port


### WIP
* SD card (SDIO Bus)(Pins used are  PC8, PC9, PC10, P11, PC12, PD 2)
* Voltage Sensing (Connected to PA2)
* Current Sensing (Connected to PA3)
* HMC5983 compass (DRDY : PE12 , CS : PE15) (SPI bus pins : SCK on PB10 , MISO on PB14, MOSI on PB15) shares SPI  bus with FRAM
* FM25V02-G FRAM memory  support (SPI bus pins : SCK on PB10 , MISO on PB14, MOSI on PB15)
* I2C support  (SCL on PB8 ,SDA on PB9)
