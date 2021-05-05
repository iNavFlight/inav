# Board - [MATEKSYS F411-Wing](https://inavflight.com/shop/p/MATEKF411WING)

![Matek F411 Wing](https://quadmeup.com/wp-content/uploads/2018/12/DSC_0004.jpg)

## Specification:

* STM32F411 CPU
* OSD
* BMP280 barometer
* Integrated PDB for 2 motors
* 2 UART ports
* 5 servos
* no SD card slot
* 3 BECs

## Details

* [Full specification](http://www.mateksys.com/?portfolio=f411-wing)
* SBUS pad has a built-in inverter and is connected to UART1 RX

## Available TARGETS

* `MATEKF411` if you want to control LEDs and have SS1 TX on ST1 pad.
* `MATEKF411_FD_SFTSRL` if you need the softserial to be full-duplex (TX = ST1 pad, RX = LED pad), at the cost of losing the LED output.
* `MATEKF411_RSSI` if you want to have analog RSSI input on ST1 pad. SS1 TX will be available on the LED pad.
* `MATEKF411_SFTSRL2`if you want to use two softserials (TX only) at the same time. Eg. Smart Audio + S. Port,  Smart Audio + LTM. (SS1 TX will be on ST1 pad and SS2 TX will be on LED pad).

## Where to buy:

* [Banggood](https://inavflight.com/shop/p/MATEKF411WING)
