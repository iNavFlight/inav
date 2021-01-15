/*
    ChibiOS/RT - Copyright (C) 2014 Uladzimir Pylinsky aka barthess

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    hal_community.c
 * @brief   HAL subsystem code.
 *
 * @addtogroup HAL
 * @{
 */

#include "hal.h"

#if (HAL_USE_COMMUNITY == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   HAL initialization (community part).
 *
 * @init
 */
void halCommunityInit(void) {

#if HAL_USE_NAND || defined(__DOXYGEN__)
  nandInit();
#endif

#if HAL_USE_EICU || defined(__DOXYGEN__)
  eicuInit();
#endif

#if HAL_USE_CRC || defined(__DOXYGEN__)
  crcInit();
#endif

#if HAL_USE_RNG || defined(__DOXYGEN__)
  rngInit();
#endif

#if HAL_USE_USBH || defined(__DOXYGEN__)
  usbhInit();
#endif

#if HAL_USE_TIMCAP || defined(__DOXYGEN__)
  timcapInit();
#endif

#if HAL_USE_QEI || defined(__DOXYGEN__)
  qeiInit();
#endif

#if HAL_USE_COMP || defined(__DOXYGEN__)
  compInit();
#endif
}

#endif /* HAL_USE_COMMUNITY */

/** @} */
