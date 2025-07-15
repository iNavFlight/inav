# BSP STM32H745I-DISCO Component

![latest tag](https://img.shields.io/github/v/tag/STMicroelectronics/stm32h745i-disco.svg?color=brightgreen)

## Overview

**STM32Cube** is an STMicroelectronics original initiative to ease developers' life by reducing efforts, time and cost.

**STM32Cube** covers the overall STM32 products portfolio. It includes a comprehensive embedded software platform delivered for each STM32 series.
   * The CMSIS modules (core and device) corresponding to the ARM(tm) core implemented in this STM32 product.
   * The STM32 HAL-LL drivers, an abstraction layer offering a set of APIs ensuring maximized portability across the STM32 portfolio.
   * The BSP drivers of each evaluation, demonstration or nucleo board provided for this STM32 series.
   * A consistent set of middleware libraries such as RTOS, USB, FatFS, graphics, touch sensing library...
   * A full set of software projects (basic examples, applications, and demonstrations) for each board provided for this STM32 series.

Two models of publication are proposed for the STM32Cube embedded software:
   * The monolithic **MCU Package**: all STM32Cube software modules of one STM32 series are present (Drivers, Middleware, Projects, Utilities) in the repository (usual name **STM32Cubexx**, xx corresponding to the STM32 series).
   * The **MCU component**: each STM32Cube software module being part of the STM32Cube MCU Package, is delivered as an individual repository, allowing the user to select and get only the required software functions.

## Description

This **stm32h745i-disco** MCU component repository is one element of the **STM32CubeH7** MCU embedded software package, providing the **STM32H745I-DISCO** BSP BOARD component part.

## Release note

Details about the content of this release are available in the release note [here](https://htmlpreview.github.io/?https://github.com/STMicroelectronics/stm32h745i-disco/blob/main/Release_Notes.html).

## Compatibility information

Below is the list of the BSP *component* drivers to be used with this BSP *board* driver. It is **crucial** that you use a consistent set of CMSIS - HAL - BSP versions, as mentioned in [this](https://htmlpreview.github.io/?https://github.com/STMicroelectronics/STM32CubeH7/blob/master/Release_Notes.html) release note.

* [stm32-bsp-common](https://github.com/STMicroelectronics/stm32-bsp-common)
* [stm32-ft5336](https://github.com/STMicroelectronics/stm32-ft5336)
* [stm32-mt25tl01g](https://github.com/STMicroelectronics/stm32-mt25tl01g)
* [stm32-mt48lc4m32b2](https://github.com/STMicroelectronics/stm32-mt48lc4m32b2)
* [stm32-rk043fn48h](https://github.com/STMicroelectronics/stm32-rk043fn48h)
* [stm32-wm8994](https://github.com/STMicroelectronics/stm32-wm8994)

## Troubleshooting

Please refer to the [CONTRIBUTING.md](CONTRIBUTING.md) guide.
