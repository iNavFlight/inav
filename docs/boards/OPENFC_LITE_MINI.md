# OpenFC Lite Mini 20x20

Open hardware flight controller by OpenDrone, 20 x 20 mm mounting, 3S-6S.
Hardware files: https://github.com/OpenDrone-hw/OpenFC-Lite-Mini

INAV target: `OPENFC_LITE_MINI`

> Check the IMU orientation on the bench (Setup tab: nose down, roll right,
> yaw) before the first arming.

## Hardware

* MCU: RP2354A (RP2350A, QFN-60, 2 MB flash in the package), 12 MHz crystal
* IMU: BMI270 on SPI. The footprint also takes an ICM-426xx or ICM-45686, which are detected as well
* No barometer and no compass on board, I2C pads for external ones
* 4 motor outputs (DSHOT), no servo outputs
* 10 V rail for the VTX, switchable
* Beeper pad, status LED, LED strip pad
* ADC: battery voltage, current signal from the ESC connector
* microSD socket (SPI) for the blackbox
* Analog OSD: not supported by INAV yet, see below

## Pinout

| GPIO | INAV name | Function |
|---|---|---|
| 0 / 1 | PA0 / PA1 | UART1 TX / RX, digital VTX connector |
| 2 / 3 | PA2 / PA3 | UART3 TX / RX, RX is also the telemetry pin of the ESC connector |
| 4 / 5 | PA4 / PA5 | I2C SDA / SCL |
| 6 / 7 | PA6 / PA7 | UART2 TX / RX, receiver pads |
| 8 | PA8 | LED strip |
| 9 | | IMU interrupt, not used |
| 10 / 11 / 12 | PA10 / PA11 / PA12 | IMU SPI SCK / MOSI / MISO |
| 13 | PA13 | IMU chip select |
| 14, 15, 16 | | analog OSD, not used |
| 17 | PB1 | Beeper |
| 18 / 19 / 20 | PB2 / PB3 / PB4 | microSD SPI SCK / MOSI / MISO |
| 21 | PB5 | microSD chip select |
| 25, 24, 23, 22 | PB9, PB8, PB7, PB6 | Motors M1, M2, M3, M4 |
| 26 | PB10 | Status LED |
| 27 | PB11 | 10 V VTX rail enable (PINIO1) |
| 28 | PB12 | ADC: current |
| 29 | PB13 | ADC: battery voltage |

## Serial ports

| Port | Hardware | Default function |
|---|---|---|
| USB | USB VCP | MSP |
| UART1, VTX connector | uart0 | MSP DisplayPort (HD OSD) |
| UART2, receiver pads | uart1 | Serial receiver, CRSF |
| UART3 | PIO | GPS (enable the GPS feature) |

None of the ports can work in single-wire half-duplex mode, so SmartPort, FPort
and similar protocols are not available. Inverted signals need no external
inverter on any port.

The RX pin of UART3 is shared by the GPS pads and the telemetry pin of the ESC
connector, so the port serves one of the two: leave it on GPS or set it to ESC
telemetry.

UART3 is a software (PIO) UART and less capable than UART1 and UART2:

* 8N1 only, no parity and no second stop bit. SBUS (8E2) works on UART1 and
  UART2 only, a PIO UART does not open for it.
* Each bit is sampled once, and a byte with a framing error is dropped without
  a trace.
* One interrupt per received byte and a short hardware FIFO.

It is fine for GPS, ESC telemetry and MSP. Keep the receiver on UART2.

## Beeper and LED strip

* Beeper: an active 5 V beeper between the B+ and B- pads. A passive beeper
  does not work, `beeper_pwm_mode` has to stay `OFF`.
* LED strip: WS2812 on the LED pad, the signal is level-shifted to 5 V on the
  board. All LED pads carry the same signal. The strip needs a DSHOT motor
  protocol: with a PWM protocol (Standard, OneShot, Multishot) motors 1 and 2
  use the same PWM slice and the strip stays dark, `status` then shows
  `LED strip not started`.

## Defaults

* Motor protocol DSHOT300, PID loop 1 kHz
* Battery voltage scale 1100 (100k / 10k divider)
* Current scale 200 (20 mV/A, OpenESC 20x20). Change `current_meter_scale` for another ESC
* The 10 V VTX rail is on after boot. The `USER1` mode switches it **off**
* Blackbox is enabled and logs to the microSD card (`blackbox_device = SDCARD`)

## USB mass storage, CPU clock

The `msc` CLI command and the `cpu_overclock` setting work as on the 30x30
board, see [OPENFC_LITE.md](OPENFC_LITE.md).

## Not supported yet

* Analog OSD (GPIO 14, 15, 16)
* Bidirectional DSHOT, use the ESC telemetry wire instead
* Dynamic gyro notch, gyro Kalman and adaptive filter
* Servo outputs: the board has none

## Flashing

Hold the BOOT button while plugging in USB, or run `dfu` in the CLI: the board
shows up as a mass storage device. Copy `inav_x.y.z_OPENFC_LITE_MINI.uf2` onto
it.
