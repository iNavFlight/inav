# Middleware USB Host MCU Component

![supported tag](https://img.shields.io/badge/tag-v3.5.0-brightgreen.svg)

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

This **stm32_mw_usb_host** MCU component repository is one element **common to all** STM32Cube MCU embedded software packages, providing the **USB Host MCU Middleware** part.

It represents ST offer to ensure the support of USB Host role on STM32 MCUs.
It includes two main modules:
 * **Core** module for the USB host standard peripheral control APIs. It includes the files ensuring USB 2.0 standard code implementation for an USB host.
  These files’ APIs will be called within every USB Host application regardless the desired functionality.
 * **Class** module for the commonly supported classes APIs. it includes the files including different USB host classes. All STM32 USB classes are implemented according to the USB 2.0 and every class’s specifications. These files’ APIs will be called within USB Host applications according to the desired functionality.

## Release note

Details about the content of this release are available in the release note [here](https://htmlpreview.github.io/?https://github.com/STMicroelectronics/STM32CubeH7/blob/master/Middlewares/ST/STM32_USB_Host_Library/Release_Notes.html).

## Troubleshooting

Please refer to the [CONTRIBUTING.md](CONTRIBUTING.md) guide.