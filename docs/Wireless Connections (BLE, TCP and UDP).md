# Wireless connections

From iNav 5 onwards, the Configurator supports wireless connections via Bluetooth Low Energy (BLE) and Wifi (UDP and TCP).

## BLE

The following adapters are supported:

- CC2541 based modules (HM1X, HC08/09)
- Nordic Semiconductor NRF5340 (Adafruit BLE Shield)
- SpeedyBee adapter

Flightcontrollers with BLE should also work, if you have an adapter/FC that doesn't work, open an issue here on Github and we will add it.

### Configuring the BLE modules
Activate MSP in iNav on a free UART port and set the Bluetooth module to the appropriate baud rate.

Example for a HM-10 module:

Connect the module to a USB/UART adapter (remember: RX to TX, TX to RX), and connect it to a serial terminal (e.g. from the Arduino IDE),
Standard baud rate is 115200 baud, CR+LF

```
AT+BAUD4
AT+NAMEiNav
```

The baud rate values: 
| Value | Baud |
|------|------|
| 1 | 9600 |
| 2 | 19200 |
| 3 | 38400 |
| 4 | 115200 |

There are many counterfeits of the HC08/09 modules on the market, which work unreliably at high baud rates.
However, it is recommended to avoid these modules and to use an original HM-10.

### SpeedyBee adapter

Just connect it to the USB port, no further configuration needed.

## TCP and UDP

Allows connections via Wifi.

Hardware:
- DIY, ESP8266 based:
  This project can be used to make iNav Wifi enabled: https://github.com/Scavanger/MSPWifiBridge 
  A small ESP01S module should still fit anywhere.

- ExpressLRS Wifi:
  Should work (via TCP, port 5761), but untested due to lack of hardware from the developer. CLI and presets do not work here, problem in ELRS, not in iNav.
