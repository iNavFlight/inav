# OpenFC Lite 30x30

Open hardware flight controller by OpenDrone, 30.5 x 30.5 mm mounting, 3S-8S.
Hardware files: https://github.com/OpenDrone-hw/OpenFC-Lite

INAV target: `OPENFC_LITE`

> The target has not been flown yet. Check the IMU orientation on the bench
> (Setup tab: nose down, roll right, yaw) before the first arming.

## Hardware

* MCU: RP2354B (RP2350B, QFN-80, 2 MB flash in the package), 12 MHz crystal
* IMU: BMI270 on SPI. The footprint also takes an ICM-426xx, which is detected as well
* No barometer and no compass on board, I2C pads for external ones
* 4 motor outputs (DSHOT), no servo outputs
* 10 V rail for the VTX, switchable
* Beeper pad, status LED, LED strip pad
* ADC: battery voltage, current signal from the ESC connector, RSSI, one spare pad
* microSD socket (SPI) for the blackbox
* Analog OSD: not supported by INAV yet, see below

## Pinout

| GPIO | INAV name | Function |
|---|---|---|
| 0 / 1 | PA0 / PA1 | UART1 TX / RX, digital VTX connector |
| 2 / 3 | PA2 / PA3 | UART3 TX / RX |
| 4, 5, 6 | | analog OSD, not used |
| 7 | PA7 | Beeper |
| 11 | PA11 | 10 V VTX rail enable (PINIO1) |
| 12 | PA12 | Status LED |
| 13 | | IMU interrupt, not used |
| 14 | PA14 | IMU chip select |
| 15 | | IMU CLKIN, not used |
| 16 / 17 | PB0 / PB1 | I2C SDA / SCL |
| 18 / 19 / 20 | PB2 / PB3 / PB4 | IMU SPI SCK / MOSI / MISO |
| 22 / 21 | PB6 / PB5 | UART2 TX / RX, receiver pads |
| 23 | PB7 | LED strip |
| 26 / 27 | PB10 / PB11 | UART4 TX / RX, RX is also the telemetry pin of the ESC connector |
| 31, 30, 29, 28 | PB15, PB14, PB13, PB12 | Motors M1, M2, M3, M4 |
| 40 | PC8 | ADC: current |
| 41 | PC9 | ADC: battery voltage |
| 45 | PC13 | ADC: RSSI |
| 47 | PC15 | ADC: spare pad |
| 42 / 43 / 44 | PC10 / PC11 / PC12 | microSD SPI SCK / MOSI / MISO |
| 46 | PC14 | microSD chip select |

## Serial ports

| Port | Silk | Hardware | Default function |
|---|---|---|---|
| USB | USB-C | USB VCP | MSP |
| UART1 | TX0 / RX0, VTX connector | uart0 | MSP DisplayPort (HD OSD) |
| UART2 | TX1 / RX1 | uart1 | Serial receiver, CRSF |
| UART3 | TP0 / RP0 | PIO | GPS (enable the GPS feature) |
| UART4 | TP1 / RP1, ESC connector | PIO | ESC telemetry |

UART3 and UART4 are software (PIO) UARTs. None of the ports can work in
single-wire half-duplex mode, so SmartPort, FPort and similar protocols are
not available. Inverted signals (SBUS) need no external inverter.

## Defaults

* Motor protocol DSHOT300, PID loop 1 kHz
* Battery voltage scale 1100 (100k / 10k divider)
* Current scale 100 (10 mV/A, OpenESC 30x30). Change `current_meter_scale` for another ESC
* The 10 V VTX rail is on after boot. The `USER1` mode switches it **off**
* Blackbox is enabled and logs to the microSD card (`blackbox_device = SDCARD`)

## USB mass storage

The CLI command `msc` reboots the board as a USB disk with the microSD card.
Unplug USB to return to normal operation.

Some cards do not cope with the way a computer writes to the disk: one block
at a time, where the blackbox writes long runs. A 4 GB no-name card (it reports
itself as `APPSD` in `sd_info`) failed about every 30th read and stopped
responding on the second write, the disk then disappeared from the computer
and the card stayed dead until the board was powered off. Blackbox logging
worked on the same card. If deleting or copying files to the card fails, use another card,
or take the logs off with a card reader.

## Not supported yet

* Analog OSD (GPIO 4, 5, 6)
* Bidirectional DSHOT, use the ESC telemetry wire instead
* Dynamic gyro notch, gyro Kalman and adaptive filter
* Servo outputs: the board has none

## Flashing

Hold the BOOT button while plugging in USB, the board shows up as a mass
storage device. Copy `inav_x.y.z_OPENFC_LITE.uf2` onto it.
