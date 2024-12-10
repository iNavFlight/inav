# Middleware Open Bootloader MCU Component

![latest tag](https://img.shields.io/github/v/tag/STMicroelectronics/stm32-mw-openbl.svg?color=brightgreen)

## Overview

**Open bootloader** is an IAP that is provided in the STM32Cube Firmware package and GitHub. This is an example that can be used by any customer that wants to build its own bootloader. Its main task is to download the application program to the internal user memory (Flash/SRAM/OTP) without the need for a debugger, by using one of the available communication interface (USART/I2C/SPI/USB-DFU & FDCAN).
**Open bootloader** supplies services to the Host (STM32 CubeProgrammer…) in order to download firmware to the device via an interface link (USART/I2C/SPI/USB-DFU/FDCAN & I3C) and to install this firmware in the needed user memory.
**Open bootloader** relies on STM32CubeFirmware HAL/LL drivers for hardware system initialization like the different clocks and the interfaces configuration.
**Open bootloader** is executed by Cortex-M processor on the non-secure domain and uses following resources:
   * Non secure user flash memory/SRAM1
   * Interrupts
   * RCC and power
   * IP Peripherals (USART/I2C/SPI/USB-DFU/FDCAN and I3C)
   * GPIOs
   * Systick
   * IWDG

## Description

This **stm32-mw-openbl** MCU component repository is one element **common to all** STM32Cube MCU embedded software packages, providing the **Open Bootloader MCU Middleware** part.

## Release note

Details about the content of this release are available in the release note [here](https://htmlpreview.github.io/?https://github.com/STMicroelectronics/stm32-mw-openbl/blob/main/Release_Notes.html).

## Troubleshooting

Please refer to the [CONTRIBUTING.md](CONTRIBUTING.md) guide.
