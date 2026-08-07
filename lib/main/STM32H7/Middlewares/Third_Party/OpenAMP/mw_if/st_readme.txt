
  @verbatim
  ********************************************************************************
  * @file    st_readme.txt
  * @author  MCD Application Team
  * @brief   This file lists the main modification done by STMicroelectronics on
  *          OpenAMP/mw_if to work with the openAMP stack within the STM32 Cube_FW
  ********************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics. All rights reserved.
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                       opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  @endverbatim

### V1.0.4/10-January-2020 ###
===============================
   + openamp_template.c, openamp_template.h, mbox_hsem_template.c, mbox_hsem_template.h, mbox_ipcc_template.c, mbox_ipcc_template.h, rsc_table_template.h:
     - Add "User Code " sections

   + openamp_template.c:
     - initialize local variables to '0'

### V1.0.3/08-November-2019 ##
===============================
   + openamp_conf_template.h:
     - replace the "STM32MP157Cxx" define macro with "LINUX_RPROC_MASTER" to support all STM32MP1 varieties

### V1.0.2/29-July-2019 ###
============================

   + rsc_table_template.c:
     - fix a runtime issue when the "__LOG_TRACE_IO_" flag is not enabled.

### V1.0.1/15-June-2019 ###
============================
   + openamp_conf_template.h:
      - use "openamp_log.h" header name instead of "log.h" to avoid name clashing.

   + rsc_table_template.c:
      - correct declaration of the resource_table  for the STM32MP1


### V1.0.0/29-Mach-2019 ###
============================
   + First version compliant with the OpenAMP v2018.10 release


 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
