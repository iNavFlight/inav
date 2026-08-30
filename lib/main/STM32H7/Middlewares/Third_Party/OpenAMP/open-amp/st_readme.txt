
  @verbatim
  ******************************************************************************
  * @file    st_readme.txt
  * @author  MCD Application Team
  * @brief   This file lists the main modification done by STMicroelectronics on
  *          "OpenAMP/open-amp" for integration with STM32Cube solution.
  ******************************************************************************
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

### V1.0.1/11-October-2019 ###
===============================
   + Change RPMSG_ADDR_BMP_SIZE to increase the number of Virtual UART instances

     -lib/include/openamp/rpmsg.h
     -lib/rpmsg/rpmsg.c


### V1.0.0/29-March-2019 ###
===============================
   + Integrate official release v2018.10

   + Add a compiler WA to fix a crash in GCC and ARMCC. In the rpmsg_virtio.c
     the function rpmsg_virtio_ns_callback() is crashing when compiled with optimization.
     Until it is fixed we force that function to be compiled with "-O0" option

     -lib/rpmsg/rpmsg_virtio.c



 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
