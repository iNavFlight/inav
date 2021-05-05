# Flight controller hardware

### Sponsored and recommended boards

These boards come from companies that support INAV development. Buying one of these boards you make your small contribution for improving INAV as well. 

Also these boards are tested by INAV development team and usually flown on daily basis.

| Board name                | CPU Family | Target name(s)            | GPS  | Compass | Barometer      | Telemetry | RX                             | Blackbox             |
|---------------------------|:----------:|:-------------------------:|:----:|:-------:|:--------------:|:---------:|:------------------------------:|:--------------------:|
| [Airbot OMNIBUS F4 PRO](https://inavflight.com/shop/p/OMNIBUSF4PRO)| F4         | OMNIBUSF4PRO   | All  | All     | All            | All       | All                            | SERIAL, SD |
| [Airbot OMNIBUS F4](https://inavflight.com/shop/s/bg/1319176)| F4         | OMNIBUSF4  | All  | All     | All            | All       | All                            | SERIAL, SD |

Note: In the above and following tables, the sensor columns indicates firmware support for the sensor category; it does not necessarily mean there is an on-board sensor.

### Recommended boards

These boards are well tested with INAV and are known to be of good quality and reliability.

| Board name                | CPU Family | Target name(s)            | GPS  | Compass | Barometer      | Telemetry | RX                             | Blackbox             |
|---------------------------|:----------:|:-------------------------:|:----:|:-------:|:--------------:|:---------:|:------------------------------:|:--------------------:|
| [Matek F405-CTR](https://inavflight.com/shop/p/MATEKF405CTR)       | F4         | MATEKF405                | All  | All     | All            | All       | All                            | SERIAL, SD     |
| [Matek F405-STD](https://inavflight.com/shop/p/MATEKF405STD)       | F4         | MATEKF405                | All  | All     | All            | All       | All                            | SERIAL, SD     |
| [Matek F405-WING](https://inavflight.com/shop/p/MATEKF405WING)       | F4         | MATEKF405SE                | All  | All     | All            | All       | All                            | SERIAL, SD     |
| [Matek F722 WING](https://inavflight.com/shop/p/MATEKF722WING)       | F7         | MATEKF722SE                | All  | All     | All            | All       | All                            | SERIAL, SD     |
| [Matek F722-SE](https://inavflight.com/shop/p/MATEKF722SE)       | F7         | MATEKF722SE               | All  | All     | All            | All       | All                            | SERIAL, SD     |
| [Matek F722-STD](https://inavflight.com/shop/p/MATEKF722STD)       | F7         | MATEKF722               | All  | All     | All            | All       | All                            | SERIAL, SD     |
| [Matek F722-MINI](https://inavflight.com/shop/p/MATEKF722MINI)       | F7         | MATEKF722SE               | All  | All     | All            | All       | All                            | SPIFLASH    |

It's possible to find more supported and tested boards [here](https://github.com/iNavFlight/inav/wiki/Welcome-to-INAV,-useful-links-and-products)
### Boards documentation

See the [docs](https://github.com/iNavFlight/inav/tree/master/docs) folder for additional information regards to many targets in INAV, to example help in finding pinout and features. _Feel free to help improve the docs._

### Boards based on F4/F7 CPUs

These boards are powerful and in general support everything INAV is capable of. Limitations are quite rare and are usually caused by hardware design issues.

### Boards based on F3 CPUs

Boards based on F3 boards will be supported for as long as practical, sometimes with reduced features due to lack of resources. No new features will be added so F3 boards are not recommended for new builds.

### Boards based on F1 CPUs

Boards based on STM32F1 CPUs are no longer supported by latest INAV version. Last release is 1.7.3

### Not recommended for new setups

F1 and F3 boards are no longer recommended. Users should choose a board from the supported F4 or F7 devices available in the latest release.