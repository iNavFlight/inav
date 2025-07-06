
  @verbatim
  ******************************************************************************
  * @file    st_readme.txt
  * @author  MCD Application Team
  * @brief   This file lists the main modification done by STMicroelectronics on
  *          LibJPEG for integration with STM32Cube solution.
  ******************************************************************************
  * @attention
  * Copyright (c) 2019 STMicroelectronics. All rights reserved.
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                       opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  @endverbatim

### 29-March-2019 ###
========================
   + include/jdata_conf_template.h, source/jdata_conf_template.c: update license to the BSD-3-Clause

### 01-February-2019 ###
========================
   + Remove from LibJpeg the GPL-licensed "ansi2knr.c" file.

### 18-November-2016 ###
========================
   + Remove from LibJpeg all the links with FatFs
   + rename template files for read/write operations from 'jdatasrc_conf_template.c/.h'
   to 'jdata_conf_template.c/.h' .

### 23-September-2016 ###
========================
   + Remove from LibJpeg all the links with FatFs
   + Add template files for read/write operations 'jdatasrc_conf_template.c/.h', these
     files have to be copied to the application folder and modified as follows:
            - Rename them to 'jdatasrc_conf.c/.h'
            - Implement read/write functions (example of implementation is provided based on FatFs)

### 23-December-2014 ###
========================
   + jinclude.h: add newline at end of file to fix compilation warning.


### 19-June-2014 ###
====================
   + First customized version of LibJPEG V8d for STM32Cube solution.
   + LibJPEG is preconfigured to support by default the FatFs file system when
     media are processed from a FAT format media
   + The original “jmorecfg.h” and “jconfig.h” files was renamed into “jmorecfg_template.h”
     and “jconfig_template.h”, respectively. Two macros has been added to specify the memory
     allocation/Freeing methods.
     These two last files need to be copied to the application folder and customized
     depending on the application requirements.

 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
