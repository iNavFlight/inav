  @verbatim
  ******************************************************************************
  *
  *         Portions COPYRIGHT 2016-2023 STMicroelectronics, All Rights Reserved
  *         Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
  *
  * @file    st_readme.txt 
  * @author  MCD Application Team
  * @brief   This file lists the main modification done by STMicroelectronics on
  *          mcuboot for integration with STM32Cube solution.
  ******************************************************************************
  *
  * original licensing conditions
  * as issued by SPDX-License-Identifier: Apache-2.0
  *
  ******************************************************************************
  @endverbatim

### 1-February-2024 ###
========================
    + Update log of bootloader version
    + Remove imgtool script presence

### 30-January-2024 ###
========================
    + Add log for validation of primary slot(s)
    + Remove unused folders and restore imgtool script presence

### 16-January-2024 ###
========================
    + Add log when device is reset

### 19-December-2023 ###
========================
    + Improvement of swapping process

### 11-December-2023 ###
========================
    + Add security fix

### 29-November-2023 ###
========================
    + Add some security updates
    + Integrate features from the middleware 'trustedfirmware'
    + Remove unused parts
    + Add copyright information

### 17-October-2023 ###
========================
    + Remove extra validation of primary slot (MCUBOOT_BOOTSTRAP)

### 5-October-2023 ###
========================
    + Improve the support of encrypted security counters (MCUBOOT_ENC_SECURITY_CNT)
    + Add some security updates

### 16-June-2023 ###
========================
    + Add compliance with MCE
    + Remove the compliance with PKA2 V1

### 12-June-2023 ###
========================
    + Add compliance with PKA2 V1

### 30-May-2023 ###
========================
    + Add the feature "MCUBOOT_USE_HAL"

### 8-February-2023 ###
========================
    + After installation in overwrite mode, erase the complete secondary slot

### 2-February-2023 ###
========================
    + remove compilation warnings for GCC compiler

### 30-January-2023 ###
========================
    + fix public advisory about integer underflow in parsed TLV data
    + remove compilation warnings (for IDEs portability) : add required functions prototype, add an empty line in end of files
    + use RNG in fih_delay()
    + fix erase of invalid image in secondary slot
    + add setob command to manage spectic option byte processing

### 26-January-2022 ###
========================
    + add custom TMPBUF_SZ for devices with small ram
    + log the key during image installation
    + remove compilation warning

### 14-December-2021 ###
========================
    + remove compilation warning

### 11-October-2021 ###
========================
    + use mcuboot release v1.7.2
    + Branch FIH_PANIC macro to Error_Handler()
    + fix FIH support
    + imgtool: add option 'getprivbin' to produce key in binary format
    + imgtool: update windows version
    + Use hash reference (sha256) in flash to reduce boot time

### 2-Jun-2021 ###
========================
    + imgtool: Fix compatibility with click v8.0 package
    + imgtool: update windows executable with ST digital signature

### 18-May-2021 ###
========================
    + Add LICENSE.txt

### 1-March-2021 ###
==========================
    + Fix Keil compile warnings

### 29-January-2021 ###
==========================
    + Add logs for swap
    + Replace while (1) by Error_Handler() call for security
    + Fix compile warnings for no encryption config

### 11-December-2020 ###
==========================
    + Fix compile warnings for swap mode and no encryption mode
    + imgtool.exe with ST digital signature
    + imgtool : update window executable
    + Fix initial attestation key generation
    + Fix: swap support and without encrypt and primary_only config
    + mcuboot: fix no to decrypt empty slot
    + imgtool : add 16 bytes minimum write and clear option
    + Fix decrypt size during image copy when image TLV start in first chunk of 1 sector
    + Scripts: Fix after commit Remove key and evolution for regeneration of keys
    + Scripts: keygen ssl mode added
    + Scripts: Remove second batch of keys
    + Scripts: Remove key and evolution for regeneration of keys
    + Clean Encryption key, context cleaning after usage
    + Ensure cleanup of boot_status structure (inc. keys) at end of mcuboot process

### 20-October-2020 ###
==========================
    + fix buffer size for MUCBOOT_PRIMARY_ONLY
    + Add flow control check to ensure HW protections are enabled prior fw images access

### 25-August-2020 ###
==========================
    + imgtool: imgtool.exe is signed ST

### 23-June-2020 ###
==========================
    + imgtool: Cryptodome python module not required
    + imgtool: numpy python module added in requirements.txt

### 12-June-2020 ###
==========================
    + fix start offset of code injection check in padding area

### 11-June-2020 ###
==========================
    + fix to accept TLV DEPENDENCY in during TLV verification
### 02-June-2020 ###
==========================
    + mcuboot release v1.5.0
    + adaptation for BL2
    + imgtool: add flash and ass commands
    + imgtool: add otfdec support and primary-only image support
    + code adaptation for otfdec
    + fix don't check secure image with non secure key
    + fix for MCUBOOT_IMAGE_NUMBER = 1
    + Introduce switch to allow double signature check with single signature computation.
    + fix code injection in padding area.
    + fix compile warnings
