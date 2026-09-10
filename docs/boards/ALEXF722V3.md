# ALEX F722-V3 Flight Controller

The **ALEX F722-V3** is a flight controller manufactured by **INDIAN ROBOTICS SOLUTION PRIVATE LIMITED**, built around the STM32F722RET6 MCU.

## Hardware Specifications

- **Manufacturer:** INDIAN ROBOTICS SOLUTION PRIVATE LIMITED
- **Target Name:** ALEXF722V3
- **MCU:** STM32F722RET6 (216MHz ARM Cortex-M7)
- **IMU:** ICM42688P / ICM42605 (SPI1, CW90_DEG)
- **Barometer:** SPL06-001 (I2C1)
- **OSD:** AT7456E / MAX7456 (SPI2)
- **Blackbox:** SPI Flash on SPI3 (16MB W25Q128FV / 2MB M25P16)
- **Compass:** External QMC5883P / IST8310 / HMC5883 on I2C1
- **Camera Switch:** Onboard dual-camera switching (CAM1 / CAM2) via PINIO1 (PC0)
- **BEC:** 5V and 12V outputs
- **LED:** WS2812 programmable LED pads (PB3)
- **Buzzer:** Active buzzer pad (PC13, inverted)
- **Voltage / Current Sensing:** Onboard ADC channels (PC1 for VBAT, PC3 for Current)

---

## Pinout & Port Mapping

### Serial Ports (UARTs)

| Pin Markings | Function | INAV Serial Port | Typical Usage |
|:---:|:---:|:---:|:---|
| **USB** | Virtual COM Port | VCP | Configurator / CLI |
| **TX1 / RX1** | UART1 (PB6 / PB7) | UART1 | MSP / Telemetry |
| **T2 / R2** | UART2 (PA2 / PA3) | UART2 | Serial Receiver (CRSF / ELRS / SBUS) |
| **T4** | UART4 (PA0 / PA1) | UART4 | GPS Module (UBLOX) |
| **TX5 / RX5** | UART5 (PC12 / PD2)| UART5 | ESC Telemetry / Peripherals |
| **T6 / R6** | UART6 (PC6 / PC7) | UART6 | DJI O3 / Walksnail / HD VTX |

### I2C Bus

| Pin Markings | MCU Pin | Function |
|:---:|:---:|:---|
| **SCL** | PB8 | I2C1 Clock (Barometer & External Compass) |
| **SDA** | PB9 | I2C1 Data (Barometer & External Compass) |

### Camera Switch (PINIO)

The board features a built-in camera switcher between **CAM1** and **CAM2**:
- Controlled via **PINIO 1** (MCU Pin PC0).
- Configured in INAV as USER1 mode in the **Modes** tab.
- Assign USER1 to an AUX channel switch on your radio transmitter to switch cameras mid-flight.
