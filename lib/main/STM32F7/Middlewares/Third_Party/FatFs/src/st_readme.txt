  @verbatim
  ******************************************************************************
  * @file    st_readme.txt
  * @author  MCD Application Team
  * @brief   This file lists the main modification done by STMicroelectronics on
  *          FatFs for integration with STM32Cube solution.
  *          For more details on FatFs implementation on STM32Cube, please refer
  *          to UM1721 "Developing Applications on STM32Cube with FatFs"
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics. All rights reserved.
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                       opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  @endverbatim

### V2.1.4/18-10-2019 ###
============================
+ Fix wrong usage of the "memcpy" in the SD_Write() function
  - drivers/sd_diskio_dma_template_bspv1.c
  - drivers/sd_diskio_dma_template_bspv2.c
  - drivers/sd_diskio_dma_rtos_template_bspv1.c
  - drivers/sd_diskio_dma_rtos_template_bspv2.c

+ correct the usage of the "_USE_MUTEX" config flag
  - syscall.c

### V2.1.3/26-07-2019 ###
============================
+ add new BSPv2 templates:
  - drivers/sd_diskio_dma_rtos_template_bspv2.c
  - drivers/sd_diskio_dma_template_bspv2.c
  - drivers/sd_diskio_template_bspv2.c
  - drivers/sdram_diskio_template_bspv2.c

+ rename old template to "xxxx_diskio_template_bspv1.c":
  - drivers/sd_diskio_dma_rtos_template_bspv1.c
  - drivers/sd_diskio_dma_template_bspv1.c
  - drivers/sd_diskio_template_bspv1.c
  - drivers/sdram_diskio_template_bspv1.c

+ Add CMSIS-OSv2 support in templates, syscall.c and ff_conf_template.h
  - syscall.c
  - ff_conf_template.h
  - drivers/sd_diskio_dma_rtos_template_bspv2.c

+ support usage of "osMutex" alongside "osSemaphore" as _SYNC_t type in fatfs
  - syscall.c
  - ff_conf_template.h


### V2.1.2/29-03-2019 ###
============================
+ add st_license.txt in the root directory
+ src/drivers/xxx_diskio_template.[c/h], src/ff_gen_drv.[c/h], src/option/syscall.c: update the license terms to BSD-3-Clause

### V2.1.1/25-01-2019 ###
============================
+ sd_diskio_dma_rtos_template.c
  - Fix memory leak in the SD_Initialize()
  - Disable the ENABLE_SD_DMA_CACHE_MAINTENANCE flag by default to fix a build error for CM4
  - include correct diskio header file

+ sd_diskio_dma_template.c
  - Correct the SD_read() function when enabling the ENABLE_SCRATCH_BUFFER flag

+ sd_diskio_dma_rtos_template.c sd_diskio_dma_template.c
  - fix potential overflow when using SysTick.


### V2.1.0/21-09-2018 ###
============================
+ ff.c
  - back-port a fix from 0.13, to correct an issue when using multi-threading
    access in the same device due to a missing semaphore lock when calling
    disk_status() API.

+ sd_diskio_dma_rtos_template.c
  - Add support to CMSIS-RTOS V2 API

+ sd_diskio_dma_rtos_template.c sd_diskio_dma_template.c
  - Add a compile flag "ENABLE_SCRATCH_BUFFER" to avoid misaligned access
    caused buffer alignment constraint in some DMA versions.
  - Add BSP_SD_ErrorCallback() and BSP_SD_AbortCallback() template functions.

### V2.0.2/17-November-2017 ###
============================
+ sdram_diskio_template.c  sram_diskio_template.c
   Fix wrong buffer size in the (SRAM/SDRAM)DISK_read(), (SRAM/SDRAM)DISK_write()

+ sd_diskio_template.c
  - define a generic 'SD_TIMEOUT' based on the BSP drivers defines. This fixes
    a build issue when using this driver with the Adafruit shield.

+ sd_diskio_dma_rtos_template.c
  - add a check via  osKernelRunning(), to avoid runtime errors due to
    osMessageXXX calls that needs the "osKernelStart()" call done first.

+ sd_diskio_dma_template.c, sd_diskio_dma_rtos_template.c
  - fix wrong address alignment when calling SCB_InvalidateDCache_by_Addr() and
    SCB_CleanDCache_by_Addr(), the address has to be 32-Byte and not
    32-bit aligned.

  - fix BSP_SD_ReadCpltCallback() and BSP_SD_WriteCpltCallback() prototypes by
    adding 'void' as argument to avoid IAR compiler errors


+ sd_diskio_template.c sd_diskio_dma_template.c, sd_diskio_dma_rtos_template.c
  - add the  flag "DISABLE_SD_INIT" to give the user the choice to initialize the SD
    either in the application or in the FatFs diskio driver.

+ all xxx_diskio_template.c
  - fix GET_BLOCK_SIZE ioctl call; the return value is in unit of sectors.


### V2.0.1/10-July-2017 ###
============================
+ sd_diskio_dma_template.c, sd_diskio_dma_rtos_template.c
  - add the  flag "ENABLE_SD_DMA_CACHE_MAINTENANCE", to enable cache maintenance  at each read write operation.
    This is useful for STM32F7/STM32H7 based platforms when using a cachable memory region.
  - add timeout checks in SD_Read() and SD_Write() to give the control back to the application to decide in case of errors.

+ ff_gen_drv.c: fix a wrong check that causes an out of bound array access.


### V2.0.0/07-March-2017 ###
============================
  + Upgrade to use FatFS R0.12c. The R0.12c breaks the API compatibility with R0.11b.
  - f_mkfs() API has a new signature.
  - The _CODE_PAGE got new values.
  - For more details check the files (doc/updates.txt) and the following urls:
       http://elm-chan.org/fsw/ff/en/mkfs.html
       http://elm-chan.org/fsw/ff/en/config.html

  + Add USB, RAMDISK and uSD template drivers under src/drivers.
    - The diskio drivers aren't part of fatfs anymore, they are just templates instead.
    - User has to copy the suitable template .c/.h file under the project, rename them by
      removing the "_template" suffix then link them into the final application.
    - The diskio driver .c/.h files have to be edited according to the used platform.

  + Define the macros "ff_malloc" and "ff_free" in the ff_conf_template.h and use
    them in the syscall.c instead of direct calls to stdlib malloc and free functions.
  + Define the "__weak" attribute in diskio.c for the GNU GCC compiler


### V1.4.0/09-September-2016 ###
================================
  + Upgrade to use FatFs R0.12b.
  + ff_conf.h: remove the use of define "_USE_BUFF_WO_ALIGNMENT".


### V1.3.0/08-May-2015 ###
==========================
  + Upgrade to use FatFs R0.11.
  + Add new APIs FATFS_LinkDriverEx() and FATFS_UnLinkDriverEx() to manage USB Key Disk having
     multi-lun capability. These APIs are equivalent to FATFS_LinkDriver() and FATFS_UnLinkDriver()
     with "lun" parameter set to 0.
  + ff_conf.h: add new define "_USE_BUFF_WO_ALIGNMENT".
     This option is available only for usbh diskio interface and allow to disable
     the management of the unaligned buffer.
     When STM32 USB OTG HS or FS IP is used with internal DMA enabled, this define
     must be set to 0 to align data into 32bits through an internal scratch buffer
     before being processed by the DMA . Otherwise (DMA not used), this define must
     be set to 1 to avoid Data alignment and improve the performance.
     Please note that if _USE_BUFF_WO_ALIGNMENT is set to 1 and an unaligned 32bits
     buffer is forwarded to the FatFs Write/Read functions, an error will be returned.
     (0: default value or 1: unaligned buffer return an error).


  + Important note:
      For application code based on previous FatFs version; when moving to R0.11
      the changes that need to be done is to update ffconf.h file, taking
      ffconf_template.h file as reference.


### V1.2.1/20-November-2014 ###
===============================
  + Disk I/O drivers; change count argument type from BYTE to UINT

  + Important note:
      For application code based on previous FatFs version; when moving to R0.10b
      the only change that need to be done is to update ffconf.h file, taking
      ffconf_template.h file as reference.


### V1.2.0/04-November-2014 ###
===============================
  + Upgrade to use FatFs R0.10b.
  + diskio.c: update disk_read() and disk_write() argument's type.

  + Important note:
      For application code based on previous FatFs version; when moving to R0.10b
      the only change that need to be done is to update ffconf.h file, taking
      ffconf_template.h file as reference.


### V1.1.1/12-September-2014 ###
================================
  + ff_gen_drv.c: Update the Disk_drvTypeDef disk variable initialization to avoid
    warnings detected with Atollic TrueSTUDIO Complier.


### V1.1.0/22-April-2014 ###
============================
  + Update sd_diskio to use SD BSP in polling mode instead of DMA mode (the scratch
    buffer needed for DMA alignment is removed as well).
  + diskio.c and ff_gen_drv.c/.h: update to prevent multiple initialization.


### V1.0.0/18-February-2014 ###
===============================
   + First R0.10 customized version for STM32Cube solution.


 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
