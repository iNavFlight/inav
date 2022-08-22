# Board - [MATEKSYS F405-Wing](https://inavflight.com/shop/p/MATEKF405WING)

## Specification:

* STM32F405 CPU
* OSD
* BMP280 barometer
* SD slot
* 6 UART serial ports
* I2C
* PDB for 2 motors
* 7 servos
* 5A BEC for the servos

## Details

* For Matek F405-WING use `MATEKF405SE` firmware.
* SBUS pin is connected to UART2
* I2C OLED display is connected to I2C2
* SmartPort telemetry can be setup with `SoftwareSerial` feature turned on, SmartPort configured in SoftwareSerial1 and receiver connected to UART2 TX pad

### Alternate targets

#### MATEKF045SE_PINIO
`MATEKF045SE_PINIO` replaces UART 6 (TX) with a pad that can be used for PINIO

## Where to buy:

* [Banggood](https://inavflight.com/shop/p/MATEKF405WING)
