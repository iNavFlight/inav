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
 * @file    hal_community.h
 * @brief   HAL subsystem header (community part).
 *
 * @addtogroup HAL_COMMUNITY
 * @{
 */

#ifndef HAL_COMMUNITY_H
#define HAL_COMMUNITY_H


/* Error checks on the configuration header file.*/
#if !defined(HAL_USE_CRC)
#define HAL_USE_CRC                         FALSE
#endif

#if !defined(HAL_USE_EEPROM)
#define HAL_USE_EEPROM                      FALSE
#endif

#if !defined(HAL_USE_EICU)
#define HAL_USE_EICU                        FALSE
#endif

#if !defined(HAL_USE_NAND)
#define HAL_USE_NAND                        FALSE
#endif

#if !defined(HAL_USE_ONEWIRE)
#define HAL_USE_ONEWIRE                     FALSE
#endif

#if !defined(HAL_USE_QEI)
#define HAL_USE_QEI                         FALSE
#endif

#if !defined(HAL_USE_RNG)
#define HAL_USE_RNG                         FALSE
#endif

#if !defined(HAL_USE_TIMCAP)
#define HAL_USE_TIMCAP                      FALSE
#endif

#if !defined(HAL_USE_USBH)
#define HAL_USE_USBH                        FALSE
#endif

#if !defined(HAL_USE_USB_HID)
#define HAL_USE_USB_HID                     FALSE
#endif

#if !defined(HAL_USE_USB_MSD)
#define HAL_USE_USB_MSD                     FALSE
#endif

#if !defined(HAL_USE_COMP)
#define HAL_USE_COMP                        FALSE
#endif

#if !defined(HAL_USE_OPAMP)
#define HAL_USE_OPAMP                        FALSE
#endif

/* Abstract interfaces.*/

/* Shared headers.*/

/* Normal drivers.*/
#include "hal_nand.h"
#include "hal_eicu.h"
#include "hal_rng.h"
#include "hal_usbh.h"
#include "hal_timcap.h"
#include "hal_qei.h"
#include "hal_comp.h"
#include "hal_opamp.h"

/* Complex drivers.*/
#include "hal_onewire.h"
#include "hal_crc.h"
#include "hal_eeprom.h"
#include "hal_usb_hid.h"
#include "hal_usb_msd.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void halCommunityInit(void);
#ifdef __cplusplus
}
#endif

#endif /* HAL_COMMUNITY_H */

/** @} */
