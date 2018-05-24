# Flight controller hardware

The current focus is geared towards flight controller hardware that use the STM32F3, STM32F4, STM32F7 series processors. The core logic is separated from the hardware drivers, porting to other processors is possible.

Current list of supported boards can be obtained using [INAV release page](https://github.com/iNavFlight/inav/releases) 

### Boards based on F1 CPUs

These boards are no longer supported including Naze32, CC3D and all derivatives. 

### Boards based on F3 CPUs

These boards are supported, but not recommeneded for modern setups.  

### Boards based on F4/F7 CPUs

These boards are powerfull and in general support everything INAV is capable of. Limitations are quite rare and are usually caused by hardware design issues.

### Recommended boards

These boards are well tested with INAV and are known to be of good quality and reliability.

| Board name                | CPU Family | Target name(s)            | GPS  | Compass | Barometer      | Telemetry | RX                             | Blackbox             |
|---------------------------|:----------:|:-------------------------:|:----:|:-------:|:--------------:|:---------:|:------------------------------:|:--------------------:|
| [PARIS Sirius™ AIR3](http://www.multiwiicopter.com/products/inav-air3-fixed-wing)                 | F3         | AIRHEROF3, AIRHEROF3_QUAD | All  | All     | All            | All       | All                            | SERIAL               |
| [Airbot OMNIBUS AIO F3](http://shop.myairbot.com/index.php/flight-control/cleanflight-baseflight/omnibusv11.html) | F3         | OMNIBUS                   | All  | All     | All            | All       | All                            | SERIAL, SD           |
| [Airbot OMNIBUS AIO F4](http://shop.myairbot.com/index.php/flight-control/cleanflight-baseflight/omnibusf4v2.html)| F4         | OMNIBUSF4, OMNIBUSF4PRO   | All  | All     | All            | All       | All                            | SERIAL, SD, SPIFLASH |
| [Airbot F4 / Flip F4](http://shop.myairbot.com/index.php/flight-control/apm/airbotf4v1.html)                      | F4         | AIRBOTF4                  | All  | All     | All            | All       | All                            | SERIAL, SPIFLASH     |