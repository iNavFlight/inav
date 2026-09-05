# Board - RP2350 PICO (Raspberry Pi Pico 2)

The RP2350 PICO target runs INAV on an off-the-shelf **Raspberry Pi Pico 2**
(RP2350A) development board — a ~US$5, 40-pin DIP-format module. The Pico 2 is
not a ready-made flight controller: you assemble the FC yourself by wiring
sensors, ESCs/servos and power to its header pins. This guide covers the wiring
layout the firmware expects, how to build and flash the firmware, and how to
test the board once it is running.

> **Before you fly:** this is a self-assembled target. Read
> [Supported features](#supported-features) at the end of this guide — the
> current firmware is bench-test firmware: USB/MSP connectivity is confirmed,
> but several peripherals (real IMU, motor/servo outputs, sensors) are defined
> in the target but not yet confirmed working end to end. Do not connect
> propellers or attempt a real flight until the sections relevant to your build
> are confirmed.

## 1. What this gives you

* INAV running on a dual Cortex-M33 RP2350 at 192 MHz with 512 KB application
  RAM and 4 MB of onboard QSPI flash.
* A USB virtual COM port (VCP) with MSP and CLI over USB — the INAV Configurator
  connects directly over the USB cable, no USB-UART adapter needed.
* Settings stored in the Pico's own flash (top 64 KB of the 4 MB), so no
  external EEPROM/dataflash chip is required for configuration.
* Four serial ports in addition to the USB port, DShot/PWM motor outputs,
  servo PWM outputs, I2C, SPI, three ADC channels and a WS2812 LED-strip output
  — all mapped to specific header pins (see the wiring tables below).

### Board identity

| Property | Value |
|---|---|
| INAV board identifier | `RP2P` |
| USB product string / board name in Configurator | `RP2350_PICO` |
| MCU | RP2350A (dual Cortex-M33), 192 MHz default |
| Flash / RAM | 4 MB QSPI flash / 512 KB application SRAM |
| Logic level | 3.3 V (not 5 V tolerant) |
| Config storage | Onboard flash (`CONFIG_IN_FLASH`) |

This target is part of the official release firmware builds. During development,
test builds are also published by the CI for each pull request
(iNavFlight/pr-test-builds); you can build the firmware yourself from the INAV
source tree (Section 4) if you want a local build. The Configurator connects to
the resulting build normally.

## 2. What you need

### Minimum for a bench test (this works today)

* 1× Raspberry Pi Pico 2 (RP2350) — with or without headers
* 1× USB data cable (micro-USB)
* A PC with the INAV Configurator installed

### For wiring up a real vehicle (future firmware)

* **IMU (gyro/accelerometer):** the current firmware runs INAV's *simulated*
  IMU — no physical IMU driver is enabled yet, so **no IMU breakout can be
  detected at this time**. The SPI0 bus pins reserved for a future SPI IMU
  (and dataflash) are GP2/3/4/5 (see Section 3.3). Check the target's
  `target.h` (`src/main/target/RP2350_PICO/target.h` in the source tree) for
  the enabled IMU driver before buying an IMU breakout.
* **ESCs/servos:** 4× DShot-capable ESCs (or servos for fixed-wing surfaces)
* **Receiver:** any serial receiver (CRSF, SBUS, IBUS, … — protocols are
  compiled in) wired to UART2
* **GPS / barometer / compass (optional):** GPS on UART3; baro and mag
  auto-detect on the I2C1 bus (GP18/19)
* **Power:** 5 V BEC (or single LiPo cell — see voltage notes) to power the
  servos/ESCs; USB for bench power
* **A voltage divider** for battery voltage and any current sensor/RSSI analog
  inputs (Section 3.4)

### Voltage notes (read before wiring)

* All Pico 2 GPIO are **3.3 V logic and are not 5 V tolerant**. Do not feed
  5 V into any GPIO. Peripheral signals that are 5 V-logic (some ESCs, some
  servos, 5 V WS2812 strips) should be level-shifted or verified 3.3 V
  compatible.
* The three ADC inputs (VBAT/current/RSSI) must be **voltage-divided so the pin
  never exceeds 3.3 V**. Battery voltage must not be connected to GP26
  directly.
* Power the board from **VSYS (pin 39, 1.8–5.5 V)** or from USB. Do not exceed
  5.5 V on VSYS. Share a common ground between the Pico, ESCs, servos and
  receiver.
* The 3.3 V pin (pin 36) can power small external loads, but keep the total
  external draw low (the onboard regulator is rated for ~300 mA total).

## 3. Pinout and wiring

Physical pin numbers below are the **Pico 2 header pins (1–40)**. Orient the
board with the micro-USB connector at the top: pins 1–20 run down the left
edge (pin 1 top-left), pins 21–40 run up the right edge.

> GPIO numbers in this section are RP2350 GPIOs. The target expresses them in
> INAV pin syntax where **PA0–PA15 = GPIO0–15** and **PB0–PB13 = GPIO16–29**.

### 3.1 Serial ports (UARTs)

Five serial ports total: the USB VCP plus four UARTs. UART1/UART2 are RP2350
hardware UARTs (uart0/uart1); UART3/UART4 are PIO-implemented UARTs.

| INAV port | Intended role | Direction | INAV pin | GPIO | Pico 2 pin | Notes |
|---|---|---|---|---|---|---|
| UART1 | MSP / CLI / configurator | TX | `PA0` | GPIO0 | **1** | RP2350 uart0. USB VCP also carries MSP/CLI |
| | | RX | `PA1` | GPIO1 | **2** | |
| UART2 | Receiver (CRSF / SBUS) | TX | `PA6` | GPIO6 | **9** | RP2350 uart1. Uses function F11 (`GPIO_FUNC_UART_AUX`) — F2 on GP6/7 is only uart1 CTS/RTS. SBUS/CRSF can be wired directly — hardware inversion is available, no external inverter needed |
| | | RX | `PA7` | GPIO7 | **10** | |
| UART3 | GPS | TX | `PA8` | GPIO8 | **11** | PIO1-implemented UART. Pins double as servo outputs 1/2 — see Section 3.2 |
| | | RX | `PA9` | GPIO9 | **12** | |
| UART4 | Telemetry / extra | TX | `PA14` | GPIO14 | **19** | PIO1-implemented UART. Pins double as servo outputs 3/4 — see Section 3.2 |
| | | RX | `PA15` | GPIO15 | **20** | |

Assign a function to a UART in the Configurator **Ports** tab (e.g. GPS on
UART3, receiver on UART2). The default receiver input is MSP
(`FEATURE_RX_MSP`), so out of the box the Configurator's joystick can act as
your RC source for bench tests.

### 3.2 Motor and servo outputs

Motor outputs 1–4 sit on GP10–13 (DShot enabled; PWM also available), matching
the Betaflight RP2350A reference motor order. Servo outputs are on GP20/21
(dedicated) plus GP8/9 and GP14/15 (shared with UART3/UART4).
Outputs are assigned in the order listed — the first *N* (motor count) become
motors and the remainder become servos, per your airframe/mixer settings.

| Output | INAV pin | GPIO | Pico 2 pin | PWM slice | Notes |
|---|---|---|---|---|---|
| Motor 1 | `PA10` | GPIO10 | **14** | slice 5 | DShot-capable (PIO) |
| Motor 2 | `PA11` | GPIO11 | **15** | slice 5 | shares slice 5 with M1 → same update rate |
| Motor 3 | `PA12` | GPIO12 | **16** | slice 6 | |
| Motor 4 | `PA13` | GPIO13 | **17** | slice 6 | shares slice 6 with M3 |
| Servo 1 | `PA8` | GPIO8 | **11** | slice 4 | same pin as UART3 TX — use one or the other |
| Servo 2 | `PA9` | GPIO9 | **12** | slice 4 | same pin as UART3 RX |
| Servo 3 | `PA14` | GPIO14 | **19** | slice 7 | same pin as UART4 TX |
| Servo 4 | `PA15` | GPIO15 | **20** | slice 7 | same pin as UART4 RX |
| Servo 5 | `PB4` | GPIO20 | **26** | slice 10 | dedicated servo output, 50 Hz, 1000–2000 µs |
| Servo 6 | `PB5` | GPIO21 | **27** | slice 10 | shares slice 10 with S5 |

Notes:

* GP8/9 and GP14/15 are dual-purpose: assigning UART3/UART4 in the Ports tab
  uses them as serial pins; leave those ports unassigned to keep them available
  as servo outputs. Do not assign the same pin to both.
* Pins sharing a PWM slice must run at the same update rate, so keep paired
  outputs in the same output class (e.g. both motors on DShot, or both servos).

### 3.3 SPI, I2C, LED strip and status LED

| Function | INAV pin | GPIO | Pico 2 pin | Notes |
|---|---|---|---|---|
| SPI bus (INAV SPI device 1 = RP2350 spi0) | | | | reserved for a future SPI IMU + dataflash |
| SPI MISO | `PA4` | GPIO4 | **6** | |
| SPI SCK | `PA2` | GPIO2 | **4** | |
| SPI MOSI | `PA3` | GPIO3 | **5** | |
| I2C1 SDA | `PB2` | GPIO18 | **24** | baro + compass auto-detect bus |
| I2C1 SCL | `PB3` | GPIO19 | **25** | |
| LED strip (WS2812) | `PB6` | GPIO22 | **29** | PIO2-driven; use 3.3 V-logic strip or level-shift |
| LED0 (status) | `PB9` | GPIO25 | — | the Pico 2's **onboard user LED** (no header pin) |

Reserved pins (do not wire anything to them yet — no firmware function is
assigned): **GP5** (pin 7, planned SPI chip-select), **GP16** (pin 21, planned
dataflash chip-select), **GP17** (pin 22, planned beeper).

### 3.4 Analog inputs (ADC)

The RP2350's ADC inputs are hardware-fixed to GPIO26–28 (three channels on the
Pico 2; a fourth, GPIO29, is used onboard to measure VSYS).

| INAV function | INAV pin | GPIO | Pico 2 pin | Notes |
|---|---|---|---|---|
| Battery voltage (VBAT) | `PB10` | GPIO26 | **31** | feed a **divided** battery voltage, ≤ 3.3 V |
| Current sensor | `PB11` | GPIO27 | **32** | analog current-sensor output, ≤ 3.3 V |
| Analog RSSI | `PB12` | GPIO28 | **34** | analog RSSI from RX, ≤ 3.3 V |

Voltage monitoring (VBAT) is enabled by default. Use the usual resistor-divider
practice (e.g. ~10:1 for a LiPo) and calibrate the divider/scale in the
Configurator — there is no onboard divider on the Pico.

### 3.5 Power and utility pins

| Pin | Signal | Notes |
|---|---|---|
| 40 | VBUS | 5 V from the USB connector |
| 39 | VSYS | main input, 1.8–5.5 V (e.g. 5 V BEC or single LiPo cell) |
| 36 | 3V3 | 3.3 V output (external load budget ~300 mA total) |
| 37 | 3V3_EN | pull low to disable the 3.3 V supply |
| 35 | ADC_VREF | ADC reference (filtered 3.3 V) |
| 33 | AGND | analog ground for GPIO26–28 |
| 30 | RUN | reset (low to reset) |
| 3, 8, 13, 18, 23, 28, 38 | GND | ground |

GPIO23 (SMPS power-save control), GPIO24 (VBUS sense) and GPIO29 (VSYS sense)
are used by the Pico 2 board itself and are **not** exposed on the header — do
not try to use them.

## 4. Building the firmware

### Prerequisites

* Linux, macOS or WSL; `cmake` (≥ 3.13) and `make` (or Ninja)
* **GNU Arm Embedded Toolchain** (`arm-none-eabi`), a version with Cortex-M33
  support (the project's tooling targets the ARM GNU 13.2.rel1 release line)
* The **Pico SDK 2.1.0 + TinyUSB 0.17.0** are **vendored in-tree** at
  `lib/main/pico-sdk` (no git submodule step needed — the tree ships with the
  trimmed sources the build compiles)
* **picotool** (https://github.com/raspberrypi/picotool) installed and on your
  `PATH`. The build invokes `picotool uf2 convert … --family rp2350-arm-s` to
  produce the `.uf2` you drag onto the board.

### Configure and build

```bash
cd <inav-source>
cmake -B build_rp2350 -DTARGET=RP2350_PICO -DTOOLCHAIN=arm-none-eabi -DCMAKE_BUILD_TYPE=Release
cmake --build build_rp2350 --target RP2350_PICO
```

`TOOLCHAIN=arm-none-eabi` is required for this target. The build produces the
firmware artifacts in `build_rp2350/`:

* `inav_<version>_RP2350_PICO.uf2` — the file for BOOTSEL drag-and-drop
  flashing (e.g. `inav_9.0.0_RP2350_PICO.uf2`)
* plus `.elf`, `.hex` and `.bin` companions of the same base name

## 5. Flashing

### BOOTSEL + drag-and-drop (recommended)

1. Unplug the Pico 2.
2. Hold the **BOOTSEL** button on the Pico 2 and plug the USB cable in.
3. Release BOOTSEL. The Pico 2 mounts as a USB mass-storage drive named
   `RP2350`.
4. Copy `inav_<version>_RP2350_PICO.uf2` onto the drive. The onboard LED blinks
   while the firmware is written, then the board reboots and runs INAV.

### Alternative: picotool

With the board in BOOTSEL mode (steps 1–3 above):

```bash
picotool load -x inav_<version>_RP2350_PICO.uf2
```

To re-enter the bootloader at any time, hold BOOTSEL while the board resets or
powers up. A serial-wire (SWD) debug probe can also be used for flashing and
debugging.

## 6. First connect and test

The confirmed, working test path for the current firmware is the USB bench
path:

1. **Connect** the Pico 2 with a USB data cable. A serial port (USB CDC) should
   appear on your PC.
2. Open the **INAV Configurator**, select the Pico 2's serial port and connect.
   The board identifies itself as **RP2350_PICO** (board id `RP2P`) with the
   firmware version.
3. **Check status:** open the CLI and run `status` and `version` — board id,
   firmware version and sensor detection are reported.
4. **Sensors:** the **Sensors** tab shows the gyro/accelerometer. In the
   current firmware this is INAV's simulated IMU, so the values are synthetic
   and do not respond to physical movement. If you fit a barometer or compass
   on I2C1 (GP18/19), it is auto-detected at boot.
5. **Configuration persistence:** change a setting (e.g. in the CLI) and run
   `save` (or reboot after saving in the Configurator). Settings are stored in
   the Pico's own flash and survive power cycles.
6. **RC bench test:** the default receiver input is MSP, so the Configurator
   can act as the transmitter for bench checks of mixing/arming logic without
   any receiver hardware. For a real receiver, wire it to UART2 and configure
   the protocol in the Configurator.
7. **Outputs:** once the firmware build you are running supports them, use the
   Configurator **Motors**/**Servos** tabs to verify each output on GP10–13
   (motors) and GP8/9, GP14/15, GP20/21 (servos) **with no propellers fitted**.
   Check the current firmware status (Section 7) to see whether output support
   is confirmed in the build you flashed.

## 7. Supported features

### Confirmed working in the current firmware

* Boot from `.uf2`; runs at 192 MHz.
* **USB VCP (CDC) + MSP over USB** — the INAV Configurator connects and shows
  board name, firmware version and sensor status; CLI works over the same USB
  port.
* INAV's full task/control stack runs on the simulated sensors (gyro/acc,
  GPS, pitot, rangefinder) — useful for exercising the firmware on the bench
  without any sensors attached.
* Settings stored in the onboard flash; board identity `RP2P`.

### Defined in the target but not yet confirmed working end to end

These are pinned to specific GPIO in the firmware and drivers exist, but the
target's status does not yet list them as verified. Bench-test each one before
trusting it, and watch for new firmware builds that confirm them:

* **Real IMU (gyro/accelerometer): not yet — the firmware uses a simulated
  IMU.** No physical IMU driver is enabled, so no IMU breakout is detected
  yet. The SPI0 bus pins for a future SPI IMU/dataflash are reserved on
  GP2/3/4/5; chip-select lines are not yet assigned.
* Real GPS (simulated only), pitot and rangefinder (simulated only).
* Motor outputs (GP10–13, DShot/PWM) and servo outputs (GP8/9, GP14/15,
  GP20/21).
* LED strip output (GP22), ADC channels (GP26–28), I2C1 baro/compass
  auto-detection (GP18/19), UART2 receiver wiring.
* Beeper: no active-buzzer output is implemented (GP17 is reserved).
* **Blackbox logging:** not enabled (no dataflash driver). The USB port is
  CDC-only — no mass-storage/log drive is presented.
* **OSD:** MSP-OSD only (no analog video path on the Pico 2).

In short: treat the current firmware as a **development/bench build** — verify
each subsystem on the bench as firmware updates land before relying on it for a
real vehicle.

## 8. References

* INAV firmware: https://github.com/iNavFlight/inav (target sources:
  `src/main/target/RP2350_PICO/`, build support: `cmake/rp2350.cmake`)
* INAV Configurator: https://github.com/iNavFlight/inav-configurator
* Raspberry Pi Pico 2 datasheet: https://datasheets.raspberrypi.com/pico/pico-2-datasheet.pdf
* RP2350 microcontroller datasheet: https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf
* Pico SDK: https://github.com/raspberrypi/pico-sdk
* picotool: https://github.com/raspberrypi/picotool
