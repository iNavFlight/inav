/**
  ******************************************************************************
  * @file    readme.txt
  * @author  MCD Application Team
  * @brief   This file lists the main changes done by STMicroelectronics on
  *          FileX low level drivers for STM32 devices.
  ******************************************************************************
  */

### V2.1.4 / 11-November-2022 ###
=================================
Main changes
-------------
- Fix spelling errors

### V2.1.3 / 01-April-2022 ###
=================================
Main changes
-------------
- Fix FX_UINIT requests in the SD/MMC drivers when the FX_STM32_XXX_INIT flags are set to 0
  + fx_stm32_sd_driver.c
  + fx_stm32_mmc_driver.c

### V2.1.2 / 28-January-2022 ###
=================================
Main changes
-------------
- Add "sd driver" template files for standalone mode.
  + "fx_stm32_sd_driver_glue_dma_standalone.c"
  + "fx_stm32_sd_driver_dma_standalone.h"

- Add "sd driver" template files for rtos mode
  + fx_stm32_sd_driver_dma_rtos.h
  + fx_stm32_sd_driver_glue_dma_rtos.c

- Update "sd driver" and "mmc driver" generic templates
  + fx_stm32_mmc_driver.h
  + fx_stm32_mmc_driver_glue.c
  + fx_stm32_sd_driver.h
  + fx_stm32_sd_driver_glue.c

- Remove duplicated templates
  + fx_stm32_sd_driver_polling.h
  + fx_stm32_sd_driver_glue_hal.c
  + fx_stm32_sd_driver_dma_it_rtos.h
  + fx_stm32_sd_driver_dma_it_standalone.h
  + fx_stm32_mmc_driver_glue_hal.c
  + fx_stm32_mmc_driver_dma_it_standalone.h
  + fx_stm32_mmc_driver_dma_it_rtos.h

- Use correct license header for template files
  + fx_stm32_sram_driver.h

### V2.1.1 / 20-September-2021 ###
=================================
Main changes
-------------
- Use correct defines for the mmc driver inherited from fx_stm32_mmc_driver.h file

Dependencies:
-------------
- Azure RTOS FileX V6.1.7
- Azure RTOS LevelX V6.1.7
- STM32Cube SD, MMC HAL drivers

### V2.1.0 / 27-August-2021 ###
=================================
Main changes
-------------
- Update scratch buffer declaration using the "__attribute__" routine
- Add explicit "()" in the FX_STM32_SD_READ_CPLT_NOTIFY FX_STM32_SD_WRITE_CPLT_NOTIFY macros calls.
- Remove the explicit references to STM32 products in the template files

Dependencies:
-------------
- Azure RTOS FileX V6.1.7
- Azure RTOS LevelX V6.1.7
- STM32Cube SD, MMC HAL drivers


### V2.0.0 / 21-June-2021 ###
=================================
Main changes
-------------
- Decouple the SD/MMC drivers from explicit dependency to BSP API
- Add 'fx_stm32_xxx_driver_glue.c' files for HAL API (DMA, IT, Polling)
- Add 'fx_stm32_xx_driver.h' templates for RTOS and Baremetal modes
- Fix GNU GCC warnings

Dependencies:
-------------
- Azure RTOS FileX V6.1.7
- Azure RTOS LevelX V6.1.7

### V1.0.0 / 25-February-2021 ###
=================================
Main changes
-------------
- First official release of Azure RTOS FileX low level drivers for STM32 MCU series

Dependencies:
-------------
- Azure RTOS FileX V6.1.3
- Azure RTOS LevelX V6.1.3
- STM32Cube SD, MMC BSP drivers
