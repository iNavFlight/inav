/**
  ******************************************************************************
  * @file    readme.txt
  * @author  MCD Application Team
  * @brief   This file lists the main changes done by STMicroelectronics on
  *          NetXDuo low level drivers for STM32 devices.
  ******************************************************************************
  */

### V3.2.0 / 09-December-2022 ###
=================================
Main changes
-------------
- Add rtl8211eg PHY driver interface
- LAN8742 PHY driver interface and Ethernet driver: Add support for 1000MBITS configuration
- Ethernet driver: Set Promiscuous Mode based on a Preprocessor switch


Dependencies:
-------------
- Azure RTOS NetXDuo V6.1.10 or higher
- STM32CubeH7 Ethernet HAL driver V1.11.0
- STM32CubeF4 Ethernet HAL driver V1.8.0
- STM32CubeF7 Ethernet HAL driver V1.3.0
- Cypress_WHD middleware 1.70.0
- MX_WIFI component driver 2.3.4

### V3.1.1 / 11-November-2022 ###
=================================
Main changes
-------------
- Fix spelling errors

Dependencies:
-------------
- Azure RTOS NetXDuo V6.1.10 or higher
- STM32CubeH7 Ethernet HAL driver V1.11.0
- STM32CubeF4 Ethernet HAL driver V1.8.0
- STM32CubeF7 Ethernet HAL driver V1.3.0
- Cypress_WHD middleware 1.70.0
- MX_WIFI component driver 2.3.4

### V3.1.0 / 30-September-2022 ###
=================================
Main changes
-------------
- Ethernet driver: Calculate the packet data offset based on header size and alignment type
- MXCHIP wifi driver: implement AP mode.
- MXCHIP wifi driver: remove the extra wait after the reset of the WiFi module.
- MXCHIP wifi driver: fix some errors seen with MISRA.
- MXCHIP wifi driver: avoid re-declaration of external function
- MXCHIP wifi driver: implement nx_driver_get_status() API


Dependencies:
-------------
- Azure RTOS NetXDuo V6.1.10 or higher
- STM32CubeH7 Ethernet HAL driver V1.11.0
- STM32CubeF4 Ethernet HAL driver V1.8.0
- STM32CubeF7 Ethernet HAL driver V1.3.0
- Cypress_WHD middleware 1.70.0
- MX_WIFI component driver 2.3.4

### V3.0.1 / 10-June-2022 ###
=================================
Main changes
-------------
- Ethernet driver: Fix NetXDuo STM32 Ethernet driver to correctly set packet data offset.
- Ethernet driver: Remove unused code related to obsolete flag NX_DRIVER_INTERNAL_TRANSMIT_QUEUE.
- Ethernet driver: Replace incorrect specific device defines with generic ones.

Dependencies:
-------------
- Azure RTOS NetXDuo V6.1.10 or higher
- STM32CubeH7 Ethernet HAL driver V1.11.0
- STM32CubeF4 Ethernet HAL driver V1.8.0
- STM32CubeF7 Ethernet HAL driver V1.3.0
- Cypress_WHD middleware 1.70.0
- MX_WIFI component driver 2.1.11

### V3.0.0 / 01-April-2022 ###
=================================
Main changes
-------------
- Add Cypress WiFi module driver.
- Align the NetXDuo STM32 Ethernet driver against new HAL Ethernet driver APIs.
- Fix NetXDuo STM32 Ethernet driver to correctly check the phy link down/up status.

Dependencies:
-------------
- Azure RTOS NetXDuo V6.1.10 or higher
- STM32CubeH7 Ethernet HAL driver V1.11.0
- STM32CubeF4 Ethernet HAL driver V1.8.0
- STM32CubeF7 Ethernet HAL driver V1.3.0
- Cypress_WHD middleware 1.70.0
- MX_WIFI component driver 2.1.11

### V2.1.2 / 28-January-2022 ###
=================================
Main changes
-------------
- Use correct license header for template files
  + mx_wifi_azure_rtos_conf.h

### V2.1.1 / 05-November-2021 ###
=================================
Main changes
-------------
- Invalidate the cache when filling the ETH/DMA descriptors in the hardware initialize function

Dependencies:
-------------
- Azure RTOS NetXDuo V6.1.7 or higher
- STM32CubeH7 Ethernet HAL driver V1.10.0
- STM32CubeF4 Ethernet HAL driver delivered within X-CUBE-AZRTOS-F4
- STM32CubeF7 Ethernet HAL driver delivered within X-CUBE-AZRTOS-F7
- MX_WIFI component driver  2.1.11


### V2.1.0 / 27-August-2021 ###
=================================
Main changes
-------------
- Update ethernet driver to support both STM32F4 and STM32H7 families

Dependencies:
-------------
- Azure RTOS NetXDuo V6.1.7
- STM32CubeH7 Ethernet HAL driver V1.10.0
- STM32CubeF4 Ethernet HAL driver delivered within X-CUBE-AZRTOS-F4
- MX_WIFI component driver  2.1.11


### V2.0.0 / 21-June-2021 ###
=================================
Main changes
-------------
- restructure  the folder tree to split ethernet and wifi drivers
- Add MXCHIP wifi module driver

Dependencies:
-------------
- Azure RTOS NetXDuo V6.1.7
- STM32CubeH7 Ethernet HAL driver V1.10.0
- MX_WIFI component driver  2.1.11

### V1.0.0 / 25-February-2021 ###
=================================
Main changes
-------------
- First official release of Azure RTOS NetXDuo low level drivers for STM32 MCU series

Dependencies:
-------------
- Azure RTOS NetXDuo V6.1.3
- STM32CubeH7 Ethernet HAL driver V1.10.0
