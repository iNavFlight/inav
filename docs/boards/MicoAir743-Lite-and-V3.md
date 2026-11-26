# MicoAir743-Lite and MicoAir743v3

The manufacturer manuals include connector diagrams and MCU pin assignments.

- [MicoAir743-Lite manual](https://micoair.com/en/docs/flight-controller/micoair743/micoair743-lite-flight-controller-manual)
- [MicoAir743v3 manual](https://micoair.com/en/docs/flight-controller/micoair743/micoair743v3-flight-controller)

Both boards use an STM32H743 and an SPA06 barometer. Blackbox logs to microSD by default.

| Target | Board | IMU | Compass | Analog OSD | DroneCAN |
| --- | --- | --- | --- | --- | --- |
| `MICOAIR743LITE` | MicoAir743-Lite | ICM45686 | External, I2C1 | None | — |
| `MICOAIR743V3` | MicoAir743v3 | BMI088 or ICM45686 | Onboard IST8310, I2C2 | AT7456E | CAN1 |
| `MICOAIR743V3_EXTMAG` | MicoAir743v3 | BMI088 or ICM45686 | External, I2C1 | AT7456E | CAN1 |

On V3, `gyro_to_use = 0` selects the BMI088 and `gyro_to_use = 1` selects the ICM45686. Save and reboot after changing it, then calibrate the selected accelerometer and verify the model orientation. With the EXTMAG target, set the external compass alignment for its mounting orientation.

## Default ports

The UART numbers match the board markings.

| Port | Default INAV function | Connection |
| --- | --- | --- |
| USB | MSP | USB Type-C, Configurator |
| UART1 | MSP, 115200 baud | TX1/RX1 |
| UART2 | MSP DisplayPort OSD | Digital VTX connector |
| UART3 | GPS | GPS connector, TX3/RX3 |
| UART4 | Unassigned | TX4/RX4 |
| UART5 | Unassigned (Lite only) | Lite TX5/RX5 |
| UART6 | Serial receiver, SBUS | TX6/RX6 |
| UART7 | ESC telemetry | RX7 only |
| UART8 | MSP, 115200 baud | Internal Bluetooth module |

For CRSF/ELRS, select the corresponding serial receiver protocol and connect receiver TX to RX6 and receiver RX to TX6. On V3, the DJI-VTX and receiver connectors share RX6. Connect only one receiver output to this signal.

On V3, the VTX (Analog) and DJI-VTX connectors share TX2. Choose either DisplayPort or analog VTX serial control for UART2.

## Outputs and LED strip

| Timer | Lite outputs | V3 outputs |
| --- | --- | --- |
| TIM1 | M1–M4 | M1–M4 |
| TIM3 | M5–M8 | M5–M6 |
| TIM15 | M9–M10 | M9–M10 |
| TIM4 | M11–M12 and the LED/PWM15 pad | M7–M8 and the LED/PWM11 pad |
| TIM12 | M13–M14 | — |

M10 is PWM-only on both boards. M13 and M14 are also PWM-only on Lite. Outputs on the same timer need compatible protocols and rates, so use PWM for the M9/M10 timer group when M10 is required.

`LED_STRIP` is disabled by default. Enabling it reserves TIM4, making Lite M11/M12 or V3 M7/M8 unavailable for motor/servo outputs. Keep it disabled when those outputs are needed. The buzzer uses a separate GPIO and does not reserve this timer.

Motor and servo assignment depends on the selected mixer and timer output modes. Check each physical output against the Configurator assignment before use.

## V3 camera and VTX power

The VTX (Analog), DJI-VTX, and FPV Camera connectors share a switched 9 V supply. With default settings and main power connected, the supply starts ON.

To control this supply, assign an AUX range to USER1 (PINIO1) in the Modes tab. **Activating USER1 turns the 9 V supply off for all three connectors, including the camera. Deactivating USER1 restores power.**
