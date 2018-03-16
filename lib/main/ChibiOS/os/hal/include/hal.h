/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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
 * @file    hal.h
 * @brief   HAL subsystem header.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef _HAL_H_
#define _HAL_H_

#include "osal.h"
#include "board.h"
#include "halconf.h"

#include "hal_lld.h"

/* Abstract interfaces.*/
#include "hal_streams.h"
#include "hal_channels.h"
#include "hal_files.h"
#include "hal_ioblock.h"
#include "hal_mmcsd.h"

/* Shared headers.*/
#include "hal_buffers.h"
#include "hal_queues.h"

/* Normal drivers.*/
#include "pal.h"
#include "adc.h"
#include "can.h"
#include "dac.h"
#include "ext.h"
#include "gpt.h"
#include "i2c.h"
#include "i2s.h"
#include "icu.h"
#include "mac.h"
#include "mii.h"
#include "pwm.h"
#include "rtc.h"
#include "serial.h"
#include "sdc.h"
#include "spi.h"
#include "uart.h"
#include "usb.h"
#include "wdg.h"

/*
 *  The ST driver is a special case, it is only included if the OSAL is
 *  configured to require it.
 */
#if OSAL_ST_MODE != OSAL_ST_MODE_NONE
#include "st.h"
#endif

/* Complex drivers.*/
#include "mmc_spi.h"
#include "serial_usb.h"

/* Community drivers.*/
#if defined(HAL_USE_COMMUNITY) || defined(__DOXYGEN__)
#if (HAL_USE_COMMUNITY == TRUE) || defined(__DOXYGEN__)
#include "hal_community.h"
#endif
#endif

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   ChibiOS/HAL identification macro.
 */
#define _CHIBIOS_HAL_

/**
 * @brief   Stable release flag.
 */
#define CH_HAL_STABLE           1

/**
 * @name    ChibiOS/HAL version identification
 * @{
 */
/**
 * @brief   HAL version string.
 */
#define HAL_VERSION             "4.0.5"

/**
 * @brief   HAL version major number.
 */
#define CH_HAL_MAJOR            4

/**
 * @brief   HAL version minor number.
 */
#define CH_HAL_MINOR            0

/**
 * @brief   HAL version patch number.
 */
#define CH_HAL_PATCH            5
/** @} */

/**
 * @name    Return codes
 * @{
 */
#define HAL_SUCCESS             false
#define HAL_FAILED              true
/** @} */

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
  void halInit(void);
#ifdef __cplusplus
}
#endif

#endif /* _HAL_H_ */

/** @} */
