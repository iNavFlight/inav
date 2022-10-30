# Flight controller hardware

### Recommended boards

These boards are well tested with INAV and are known to be of good quality and reliability.

| Board name                | CPU Family | Target name(s)            | GPS  | Compass | Barometer      | Telemetry | RX                             | Blackbox             |
|---------------------------|:----------:|:-------------------------:|:----:|:-------:|:--------------:|:---------:|:------------------------------:|:--------------------:|
| [Diatone Mamba H743](https://inavflight.com/shop/s/bg/1929033)       | H7         | MAMBAH743                | All  | All     | All            | All       | All   | SERIAL, SD     |
| [Matek F765-WSE](https://inavflight.com/shop/s/bg/1890404)       | F7         | MATEKF765SE                | All  | All     | All            | All       | All     | SERIAL, SD     |
| [Matek F722-SE](https://inavflight.com/shop/p/MATEKF722SE)       | F7         | MATEKF722SE               | All  | All     | All            | All       | All      | SERIAL, SD     |
| [Holybro Kakute H7](https://inavflight.com/shop/s/bg/1914066)       | H7         | KAKUTEH7               | All  | All     | All            | All       | All      | SERIAL, SD     |

It's possible to find more supported and tested boards [here](https://github.com/iNavFlight/inav/wiki/Welcome-to-INAV,-useful-links-and-products)
### Boards documentation

See the [docs/boards](https://github.com/iNavFlight/inav/tree/master/docs/boards) folder for additional information regards to many targets in INAV, to example help in finding pinout and features. _Feel free to help improve the docs._

### Boards based on F4/F7 CPUs

These boards are powerful and in general support everything INAV is capable of. Limitations are quite rare and are usually caused by hardware design issues.

### Boards based on F3 CPUs

Boards based on STM32F3 MCUs are no longer supported by latest INAV version. Last release is 2.6.1.

### Boards based on F1 CPUs

Boards based on STM32F1 CPUs are no longer supported by latest INAV version. Last release is 1.7.3

### Not recommended for new setups

F1 and F3 boards are no longer recommended. Users should choose a board from the supported F4 or F7 devices available in the latest release.
