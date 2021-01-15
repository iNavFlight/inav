/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

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

#ifndef HAL_H
#define HAL_H

#include "osal.h"
#include "board.h"
#include "halconf.h"

/* Error checks on the configuration header file.*/
#if !defined(HAL_USE_PAL)
#define HAL_USE_PAL                         FALSE
#endif

#if !defined(HAL_USE_ADC)
#define HAL_USE_ADC                         FALSE
#endif

#if !defined(HAL_USE_CAN)
#define HAL_USE_CAN                         FALSE
#endif

#if !defined(HAL_USE_CRY)
#define HAL_USE_CRY                         FALSE
#endif

#if !defined(HAL_USE_DAC)
#define HAL_USE_DAC                         FALSE
#endif

#if !defined(HAL_USE_GPT)
#define HAL_USE_GPT                         FALSE
#endif

#if !defined(HAL_USE_I2C)
#define HAL_USE_I2C                         FALSE
#endif

#if !defined(HAL_USE_I2S)
#define HAL_USE_I2S                         FALSE
#endif

#if !defined(HAL_USE_ICU)
#define HAL_USE_ICU                         FALSE
#endif

#if !defined(HAL_USE_MAC)
#define HAL_USE_MAC                         FALSE
#endif

#if !defined(HAL_USE_PWM)
#define HAL_USE_PWM                         FALSE
#endif

#if !defined(HAL_USE_RTC)
#define HAL_USE_RTC                         FALSE
#endif

#if !defined(HAL_USE_SERIAL)
#define HAL_USE_SERIAL                      FALSE
#endif

#if !defined(HAL_USE_SDC)
#define HAL_USE_SDC                         FALSE
#endif

#if !defined(HAL_USE_SIO)
#define HAL_USE_SIO                         FALSE
#endif

#if !defined(HAL_USE_SPI)
#define HAL_USE_SPI                         FALSE
#endif

#if !defined(HAL_USE_TRNG)
#define HAL_USE_TRNG                        FALSE
#endif

#if !defined(HAL_USE_UART)
#define HAL_USE_UART                        FALSE
#endif

#if !defined(HAL_USE_USB)
#define HAL_USE_USB                         FALSE
#endif

#if !defined(HAL_USE_WDG)
#define HAL_USE_WDG                         FALSE
#endif

#if !defined(HAL_USE_WSPI)
#define HAL_USE_WSPI                        FALSE
#endif

/* Low Level HAL support.*/
#include "hal_lld.h"

/* Abstract interfaces.*/
#include "hal_objects.h"
#include "hal_streams.h"
#include "hal_channels.h"
#include "hal_files.h"
#include "hal_ioblock.h"
#include "hal_mmcsd.h"
#include "hal_persistent.h"

/* Shared headers.*/
#include "hal_buffers.h"
#include "hal_queues.h"

/* Normal drivers.*/
#include "hal_pal.h"
#include "hal_adc.h"
#include "hal_can.h"
#include "hal_crypto.h"
#include "hal_dac.h"
#include "hal_gpt.h"
#include "hal_i2c.h"
#include "hal_i2s.h"
#include "hal_icu.h"
#include "hal_mac.h"
#include "hal_pwm.h"
#include "hal_rtc.h"
#include "hal_serial.h"
#include "hal_sdc.h"
#include "hal_sio.h"
#include "hal_spi.h"
#include "hal_trng.h"
#include "hal_uart.h"
#include "hal_usb.h"
#include "hal_wdg.h"
#include "hal_wspi.h"

/*
 *  The ST driver is a special case, it is only included if the OSAL is
 *  configured to require it.
 */
#if OSAL_ST_MODE != OSAL_ST_MODE_NONE
#include "hal_st.h"
#endif

/* Complex drivers.*/
#include "hal_mmc_spi.h"
#include "hal_serial_usb.h"

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
#define HAL_VERSION             "7.0.3"

/**
 * @brief   HAL version major number.
 */
#define CH_HAL_MAJOR            7

/**
 * @brief   HAL version minor number.
 */
#define CH_HAL_MINOR            0

/**
 * @brief   HAL version patch number.
 */
#define CH_HAL_PATCH            3
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

/* Configuration file checks.*/
#if !defined(_CHIBIOS_HAL_CONF_)
#error "invalid configuration file"
#endif

#if !defined(_CHIBIOS_HAL_CONF_VER_7_0_)
#error "obsolete or unknown configuration file"
#endif

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

#endif /* HAL_H */

/** @} */
