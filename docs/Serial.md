# Serial

INAV has enhanced serial port flexibility but configuration is slightly more complex as a result.

INAV has the concept of a function (MSP, GPS, Serial RX, etc) and a port (VCP, UARTx, SoftSerial x).
Not all functions can be used on all ports due to hardware pin mapping, conflicting features, hardware, and software
constraints.

## Serial port types

* USB Virtual Com Port (VCP) - USB pins on a USB port connected directly to the processor without requiring
a dedicated USB to UART adapter.  VCP does not 'use' a physical UART port.
* UART - A pair of dedicated hardware transmit and receive pins with signal detection and generation done in hardware.
* SoftSerial - A pair of hardware transmit and receive pins with signal detection and generation done in software.

UART is the most efficient in terms of CPU usage.
SoftSerial is the least efficient and slowest, SoftSerial should only be used for low-bandwidth usages, such as telemetry transmission.

UART ports are sometimes exposed via on-board USB to UART converters, such as the CP2102.
If the flight controller does not have an on-board USB to UART converter and doesn't support VCP then an external USB to UART board is required.
These are sometimes referred to as FTDI boards.  FTDI is just a common manufacturer of a chip (the FT232RL) used on USB to UART boards.

When selecting a USB to UART converter choose one that has DTR exposed as well as a selector for 3.3v and 5v since they are more useful.

Examples:

 * [FT232RL FTDI USB To TTL Serial Converter Adapter](https://inavflight.com/shop/s/bg/917226)
 * [USB To TTL / COM Converter Module buildin-in CP2102](https://inavflight.com/shop/s/bg/27989)

Both SoftSerial and UART ports can be connected to your computer via USB to UART converter boards.

## Serial Configuration

Serial port configuration is best done via the configurator.

Configure serial ports first, then enable/disable features that use the ports.  To configure SoftSerial ports the SOFTSERIAL feature must be also be enabled.

### Constraints

If the configuration is invalid the serial port configuration will reset to its defaults and features may be disabled.

* There must always be a port available to use for MSP/CLI.
* There is a maximum of 3 MSP ports.
* To use a port for a function, the function's corresponding feature must be also be enabled.
e.g. after configuring a port for GPS enable the GPS feature.
* If SoftSerial is used, then all SoftSerial ports must use the same baudrate.
* Softserial has no hard baud-rate cap, but because it is bit-banged it is most reliable at low baud rates (e.g. 19200 or below); high baud rates increase CPU load and timing jitter.
* All telemetry systems except MSP will ignore any attempts to override the baudrate.
* MSP/CLI can be shared with EITHER Blackbox OR telemetry (LTM or MAVlink, not RX telemetry).  In shared mode blackbox or telemetry will be output only when armed.
* Smartport telemetry cannot be shared with MSP.
* No other serial port sharing combinations are valid.
* You can use as many different telemetry systems as you like at the same time.
* You can only use each telemetry system once.  e.g.  FrSky telemetry cannot be used on two port, but LTN Telemetry and FrSky on two different ports is fine.

### Configuration via CLI

You can use the CLI for configuration but the commands are reserved for developers and advanced users.

The `serial` CLI command takes 6 arguments.

1. Identifier
2. Function bitmask (see serialPortFunction_e in the source)
3. MSP baud rate
4. GPS baud rate
5. Telemetry baud rate (auto baud allowed)
6. Blackbox baud rate

### UART pins on output pads

On many boards the processor can also connect a UART's TX or RX to one of the motor or servo output pads. That gives a port to a board whose UART pads are all taken, or puts a half-duplex device (SmartAudio, Tramp, a Smart ESC) on the servo connector it is already plugged into.

The Ports tab shows a Pins column for the UARTs that can move; choose the S pad for TX or RX there and save. In the CLI:

```
serialpad <port> <tx pad> <rx pad>
```

`port` is the identifier `serial` uses, the pads are numbered as in the Outputs tab (S1, S2, ...) and 0 keeps the UART's own pin. `serialpad` alone lists the current choices, and marks any the UART is not using now: the port has no function, the pad is not an output on this board, soft serial, an ADC input or another port uses that pin, or the change is waiting for a save. The change takes effect after `save`.

* The pad stops being a motor, servo, LED strip, beeper or PINIO output, and the other outputs keep their numbers. If the mixer drives a motor or servo on it, arming is blocked with a PWM output error (`status` shows "Motor or servo output used by a UART"): change the mixer or choose another pad.
* A pad carries one UART pin at a time, and the move happens only while the port has a function.
* F4 and AT32 boards invert SBUS and SmartPort with an inverter on the UART's own pin, which an output pad does not have. F7 and H7 invert inside the UART.
* A UART the board swaps (TX and RX exchanged) does not move.
* Available on boards with more than 512 KB of flash (F405, F745, F765, H7 and AT32), and on the NEXUS, NEXUS X and Vantac RF007, whose ESC connector UART1 can reach.


### Baud Rates

The allowable baud rates are as follows:

| Identifier | Baud rate |
| ---------- | --------- |
|  0         |    Auto   |
|  1         |    1200   |
|  2         |    2400   |
|  3         |    4800   |
|  4         |    9600   |
|  5         |   19200   |
|  6         |   38400   |
|  7         |   57600   |
|  8         |  115200   |
|  9         |  230400   |
| 10         |  250000   |
| 11         |  460800   |
| 12         |  921600   |
| 13         | 1000000   |
| 14         | 1500000   |
| 15         | 2000000   |
| 16         | 2470000   |


### Function numbers as of 2025

| Function                   | Number                                             |
| -------------------------- | -------------------------------------------------- |
| NONE                       | 0, |
| MSP                        | (1 << 0), // 1 |
| GPS                        | (1 << 1), // 2 |
| UNUSED_3                   | (1 << 2), // 4 //Was FUNCTION_TELEMETRY_FRSKY |
| TELEMETRY_HOTT             | (1 << 3), // 8 |
| TELEMETRY_LTM              | (1 << 4), // 16 |
| TELEMETRY_SMARTPORT        | (1 << 5), // 32 |
| RX_SERIAL                  | (1 << 6), // 64 |
| BLACKBOX                   | (1 << 7), // 128 |
| TELEMETRY_MAVLINK          | (1 << 8), // 256 |
| TELEMETRY_IBUS             | (1 << 9), // 512 |
| RCDEVICE                   | (1 << 10), // 1024 |
| VTX_SMARTAUDIO             | (1 << 11), // 2048 |
| VTX_TRAMP                  | (1 << 12), // 4096 |
| UNUSED_1                   | (1 << 13), // 8192: former\ UAV_INTERCONNECT |
| OPTICAL_FLOW               | (1 << 14), // 16384 |
| LOG                        | (1 << 15), // 32768 |
| RANGEFINDER                | (1 << 16), // 65536 |
| VTX_FFPV                   | (1 << 17), // 131072 |
| ESCSERIAL                  | (1 << 18), // 262144: this is used for both SERIALSHOT and ESC_SENSOR telemetry |
| TELEMETRY_SIM              | (1 << 19), // 524288 |
| FRSKY_OSD                  | (1 << 20), // 1048576 |
| DJI_HD_OSD                 | (1 << 21), // 2097152 |
| SERVO_SERIAL               | (1 << 22), // 4194304 |
| TELEMETRY_SMARTPORT_MASTER | (1 << 23), // 8388608 |
| UNUSED_2                   | (1 << 24), // 16777216 |
| MSP_OSD                    | (1 << 25), // 33554432 |

