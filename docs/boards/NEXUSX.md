Radiomaster NEXUS X and NEXUS XR
================================

Flight controllers originally designed for helicopters using Rotorflight.
Based on STM32F722RET6. Both NEXUS X and XR share the same target in iNav.

The labels on the case (TAIL, ESC, RPM, TLM, AUX, SBUS, A, B and C) come from the helicopter
designs the board was made for. They do not match iNav's port and output numbering, so the
tables below give the translation.

| Item | Value |
|---|---|
| Target name | `NEXUSX` |
| MCU | STM32F722RE |
| Board identifier | `F7C5` |
| USB device name | `NEXUSX` |
| Serial ports | USB VCP plus UART1 to UART6 |
| Motor and servo outputs | 9 |
| Default features | telemetry, battery voltage, TX profile selection |

Built-in peripherals
-------------------

Both models contain a ICM42688P IMU, a SPL06 barometer and a W25N02KVZEIR blackbox memory chip.

The NEXUS XR also contains a serial ELRS receiver based on the RP4TD-M using ESP32 and two SX1281.
It is connected to the main STM32F7 Flight Controller on UART5.
None of the external connections route to the receiver, they are all connected to the STM32F7 Flight Controller.
The receiver can be disabled using USER1, which controls a pinio on pin PC8.

| Function | Hardware and connection |
|---|---|
| Gyro and accelerometer | ICM42688P on SPI1 (SCK PA5, MISO PA6, MOSI PA7), chip select PA4, interrupt PB8. Handled by the `ICM42605` driver, default alignment `CW0` |
| Barometer | SPL06 on I2C3 (SCL PA8, SDA PC9). That bus is internal, it is not available for external sensors |
| Compass | Not enabled by default. Auto detection of all supported compasses runs on I2C2, which is port "C", the bus an external compass connects to |
| Blackbox | W25N02K flash on SPI2 (SCK PB13, MISO PB14, MOSI PB15), chip select PB12. Logging to the onboard flash is enabled by default |
| Status LEDs | PC10 and PC11 |
| Battery voltage | ADC channel 3 on PC0, the "EXT-V" input. Default `vbat_scale` is 2474 |
| Further ADC inputs | PC2 ("BUS") and PC1 ("BEC") are wired to ADC channels 1 and 2, neither is used for battery voltage by default |
| Sensors enabled by default | Accelerometer and barometer |

Serial ports
------------

| iNav port | Marking on the case | TX pin | RX pin |
|-----------|------------------------------------------|--------|--------|
| USB VCP   | USB socket                               | n/a    | n/a    |
| UART1     | "AUX" (TX) and "SBUS" (RX)               | PB6    | PB7    |
| UART2     | "RPM" (TX) and "TLM" (RX)                | PA2    | PA3    |
| UART3     | "C"                                      | PB10   | PB11   |
| UART4     | "A"                                      | PA0    | PA1    |
| UART5     | internal ELRS receiver, NEXUS XR only    | PC12   | PD2    |
| UART6     | "B"                                      | PC6    | PC7    |

In short, the three four pin connectors are: "A" is UART4, "B" is UART6 and "C" is UART3.
Port "C" can be used either as UART3 or as the I2C2 bus, but not as both at the same time.

Default port functions
----------------------

After flashing, or after a settings reset, the ports have these functions:

| Port | Function |
|------------------------------------|-------------|
| USB VCP                            | MSP         |
| UART1 ("AUX" and "SBUS" pads)      | MSP         |
| UART5 (internal ELRS receiver)     | Serial RX   |
| UART2, UART3, UART4, UART6         | none        |

The receiver defaults are `receiver_type = SERIAL` and `serialrx_provider = CRSF`, with the
serial receiver function on UART5. On a NEXUS XR that is the internal ELRS receiver described
above, which is powered through the PC8 pinio and can be switched off with the USER1 mode.

The pad marked "SBUS" is not a serial receiver input in the default configuration, it is UART1
RX. To use a receiver on it, assign Serial RX to UART1 in the Ports tab and set
`serialrx_provider` to the protocol of the receiver. For a normal SBUS receiver leave
`serialrx_inverted` off, the inversion SBUS needs is already the default for that protocol.

A receiver can equally be connected to port "A" (UART4), "B" (UART6) or "C" (UART3) by moving
the Serial RX function to that port. That is the usual choice on a NEXUS X, which has no
internal receiver.

Pin configuration
-----------------

The RPM, TLM, AUX and SBUS pins are Servo/Motor outputs by default. However, when UART1 or UART2 are assigned a function in the ports tab, the pins will become a UART instead. See the table below.

| Marking on the case | Both UART1 and UART2 unused                                                            | UART1 in use                                                                           | UART2 in use                                                                           | Both UART1 and UART2 in use                                                            |
|---------------------|----------------------------------------------------------------------------------------|----------------------------------------------------------------------------------------|----------------------------------------------------------------------------------------|----------------------------------------------------------------------------------------|
| S1                  | Output S1                                                                              | Output S1                                                                              | Output S1                                                                              | Output S1                                                                              |
| S2                  | Output S2                                                                              | Output S2                                                                              | Output S2                                                                              | Output S2                                                                              |
| S3                  | Output S3                                                                              | Output S3                                                                              | Output S3                                                                              | Output S3                                                                              |
| TAIL                | Output S4                                                                              | Output S4                                                                              | Output S4                                                                              | Output S4                                                                              |
| ESC                 | Output S5                                                                              | Output S5                                                                              | Output S5                                                                              | Output S5                                                                              |
| RPM                 | Output S6                                                                              | Output S6                                                                              | UART2 TX                                                                               | UART2 TX                                                                               |
| TLM                 | Output S7                                                                              | Output S7                                                                              | UART2 RX                                                                               | UART2 RX                                                                               |
| AUX                 | Output S8                                                                              | UART1 TX                                                                               | Output S6                                                                              | UART1 TX                                                                               |
| SBUS                | Output S9                                                                              | UART1 RX                                                                               | Output S7                                                                              | UART1 RX                                                                               |
| A                   | UART4<br>pin order:<br>TX, RX, 5V, GND                                                 | UART4<br>pin order:<br>TX, RX, 5V, GND                                                 | UART4<br>pin order:<br>TX, RX, 5V, GND                                                 | UART4<br>pin order:<br>TX, RX, 5V, GND                                                 |
| B                   | UART6<br>pin order:<br>TX, RX, 5V, GND                                                 | UART6<br>pin order:<br>TX, RX, 5V, GND                                                 | UART6<br>pin order:<br>TX, RX, 5V, GND                                                 | UART6<br>pin order:<br>TX, RX, 5V, GND                                                 |
| C                   | I2C<br>pin order:<br>SCL, SDA, 5V, GND<br>or<br>UART3<br>pin order:<br>TX, RX, 5V, GND | I2C<br>pin order:<br>SCL, SDA, 5V, GND<br>or<br>UART3<br>pin order:<br>TX, RX, 5V, GND | I2C<br>pin order:<br>SCL, SDA, 5V, GND<br>or<br>UART3<br>pin order:<br>TX, RX, 5V, GND | I2C<br>pin order:<br>SCL, SDA, 5V, GND<br>or<br>UART3<br>pin order:<br>TX, RX, 5V, GND |
| EXT-V               | battery voltage<br>max 60V<br>pin order:<br>Vbat, GND                                  | battery voltage<br>max 60V<br>pin order:<br>Vbat, GND                                  | battery voltage<br>max 60V<br>pin order:<br>Vbat, GND                                  | battery voltage<br>max 60V<br>pin order:<br>Vbat, GND                                  |
| built-in ELRS       | UART5                                                                                  | UART5                                                                                  | UART5                                                                                  | UART5                                                                                  |

All pin orders are from left to right, when looking at the connector on the flight controller.
**Note that the pin order for "A", "B" and "C" is incorrect on radiomaster's website, it has RX and TX swapped.**

Outputs and timer groups
------------------------

The target defines nine outputs. Outputs that share a timer must all be motors or all be
servos. The mode is set per timer, in the Mixer tab of the Configurator or with the
`timer_output_mode` command on the CLI. See [ESC and servo outputs](../ESC%20and%20servo%20outputs.md).

| Timer | Outputs | Marking on the case | Mode set by the target |
|-------|---------|---------------------|------------------------|
| TIM3  | 1, 2, 3 | S1, S2, S3          | auto                   |
| TIM2  | 4, 6, 7 | TAIL, RPM, TLM      | auto                   |
| TIM1  | 5       | ESC                 | motors                 |
| TIM4  | 8, 9    | AUX, SBUS           | auto                   |

The output numbers are the positions in the output list of the target, which is the order the
Configurator shows them in. "ESC" is the only output the target forces to motor mode. Because
it is alone on TIM1 it can drive a motor while every other output stays a servo. DShot,
including DShot with burst DMA, ESC telemetry and the BLHeli passthrough interface are enabled
in this target.

I2C buses and external sensors
------------------------------

| Bus  | Pins                | Where             | Use |
|------|---------------------|-------------------|-----|
| I2C1 | PB6, PB7            | "AUX" and "SBUS"  | Not enabled in this target, the two pins are used for UART1 and for outputs |
| I2C2 | SCL PB10, SDA PB11  | port "C"          | The external bus: compass, rangefinder, pitot tube and temperature sensors |
| I2C3 | SCL PA8, SDA PC9    | internal          | The onboard SPL06 barometer, not broken out |

I2C2 and UART3 use the same two pins. The firmware only starts I2C2 when no function is
assigned to UART3, so as soon as UART3 is given a function the external I2C bus stops working,
and with it an external compass or rangefinder on port "C".

Hardware layout
---------------


| Marking on the case | STM32 pin |                         Servo |           UART |      I2C |
|---------------------|-----------|------------------------------:|---------------:|---------:|
| S1                  | PB4       |                       TIM3CH1 |            n/a |      n/a |
| S2                  | PB5       |                       TIM3CH2 |            n/a |      n/a |
| S3                  | PB0       |                       TIM3CH3 |            n/a |      n/a |
| TAIL                | PA15      |                       TIM2CH1 |            n/a |      n/a |
| ESC                 | PA9       |                       TIM1CH2 |       UART1 TX |      n/a |
| RPM                 | PA2       | TIM2CH3<br>TIM5CH3<br>TIM9CH1 |       UART2 TX |      n/a |
| TLM                 | PA3       | TIM2CH4<br>TIM5CH4<br>TIM9CH2 |       UART2 RX |      n/a |
| AUX                 | PB6       |                       TIM4CH1 |       UART1 TX | I2C1 SCL |
| SBUS                | PB7       |                       TIM4CH2 |       UART1 RX | I2C1 SDA |
| A                   | PA1/PA0   |                  TIM1<br>TIM5 | UART2<br>UART4 |      n/a |
| B                   | PC7/PC6   |                  TIM3<br>TIM8 |          UART6 |      n/a |
| C                   | PB11/PB10 |                          TIM2 |          UART3 |     I2C2 |
| EXT-V               | PC0       |                           n/a | n/a            | n/a      |
| built-in ELRS       | PC12/PD2  |                           n/a |          UART5 |      n/a |

The pinout is extremely similar to the F7C reference design from Rotorflight.
https://github.com/rotorflight/rotorflight-ref-design/blob/master/Reference-Design-F7C.md

Details not confirmed from the firmware source
----------------------------------------------

The following cannot be derived from the target definition in this repository. Please confirm
them with RadioMaster or a maintainer before wiring anything that depends on them.

- The pin order inside the "A", "B" and "C" connectors, including where 5V and GND sit. The
  order in the pin configuration table above comes from the board author, the firmware source
  says nothing about it.
- The 60V limit of the "EXT-V" input. The target only sets a default `vbat_scale` of 2474.
- Which pads, if any, carry the "BUS" and "BEC" ADC inputs (PC2 and PC1) and the two status
  LEDs (PC10 and PC11).
- Whether a NEXUS X, which has no internal receiver, exposes UART5 (PC12 and PD2) anywhere.
- Whether the board carries an onboard compass. The target looks for a compass on I2C2, the
  bus on port "C", which is where an external compass is connected.
- Whether assigning a function to UART1 really turns the "AUX" and "SBUS" pads from outputs
  into a serial port, as the pin configuration table states. The firmware gives up output pins
  in favour of UART2 to UART8, but UART1 is not part of that check, and UART1 has MSP assigned by
  default. This needs checking on hardware.
