/*
    Copyright (C) 2014..2017 Marco Veeneman

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
 * @file    TM4C123x/cmparams.h
 * @brief   ARM Cortex-M4 parameters for the TM4C123x.
 *
 * @defgroup ARMCMx_TM4C123x TM4C123x Specific Parameters
 * @ingroup ARMCMx_SPECIFIC
 * @details This file contains the Cortex-M4 specific parameters for the
 *          TM4C123x platform.
 * @{
 */

#ifndef CMPARAMS_H
#define CMPARAMS_H

/* Defines required for correct CMSIS header functioning */
#define __MPU_PRESENT           1       /**< MPU present                     */
#define __NVIC_PRIO_BITS        3       /**< Bits used for Priority Levels   */
#define __Vendor_SysTickConfig  1       /**< Use different SysTick Config    */
#define __FPU_PRESENT           1       /**< FPU present                     */

/* The following two defines are needed by ChibiOS */
#define SVCall_IRQn             -5
#define PendSV_IRQn             -3

/**
 * @brief   Cortex core model.
 */
#define CORTEX_MODEL            4

/**
 * @brief   Floating Point unit presence.
 */
#define CORTEX_HAS_FPU          1

/**
 * @brief   Number of bits in priority masks.
 */
#define CORTEX_PRIORITY_BITS    3

/**
 * @brief   Number of interrupt vectors.
 * @note    This number does not include the 16 system vectors and must be
 *          rounded to a multiple of 8.
 */
#define CORTEX_NUM_VECTORS      144

/* The following code is not processed when the file is included from an
   asm module.*/
#if !defined(_FROM_ASM_)

/* If the device type is not externally defined, for example from the Makefile,
   then a file named board.h is included. This file must contain a device
   definition compatible with the include file.*/
#if !defined (PART_TM4C1230C3PM)  && !defined (PART_TM4C1230D5PM)  &&       \
    !defined (PART_TM4C1230E6PM)  && !defined (PART_TM4C1230H6PM)  &&       \
    !defined (PART_TM4C1231C3PM)  && !defined (PART_TM4C1231D5PM)  &&       \
    !defined (PART_TM4C1231D5PZ)  && !defined (PART_TM4C1231E6PM)  &&       \
    !defined (PART_TM4C1231E6PZ)  && !defined (PART_TM4C1231H6PGE) &&       \
    !defined (PART_TM4C1231H6PM)  && !defined (PART_TM4C1231H6PZ)  &&       \
    !defined (PART_TM4C1232C3PM)  && !defined (PART_TM4C1232D5PM)  &&       \
    !defined (PART_TM4C1232E6PM)  && !defined (PART_TM4C1232H6PM)  &&       \
    !defined (PART_TM4C1233C3PM)  && !defined (PART_TM4C1233D5PM)  &&       \
    !defined (PART_TM4C1233D5PZ)  && !defined (PART_TM4C1233E6PM)  &&       \
    !defined (PART_TM4C1233E6PZ)  && !defined (PART_TM4C1233H6PGE) &&       \
    !defined (PART_TM4C1233H6PM)  && !defined (PART_TM4C1233H6PZ)  &&       \
    !defined (PART_TM4C1236D5PM)  && !defined (PART_TM4C1236E6PM)  &&       \
    !defined (PART_TM4C1236H6PM)  && !defined (PART_TM4C1237D5PM)  &&       \
    !defined (PART_TM4C1237D5PZ)  && !defined (PART_TM4C1237E6PM)  &&       \
    !defined (PART_TM4C1237E6PZ)  && !defined (PART_TM4C1237H6PGE) &&       \
    !defined (PART_TM4C1237H6PM)  && !defined (PART_TM4C1237H6PZ)  &&       \
    !defined (PART_TM4C123AE6PM)  && !defined (PART_TM4C123AH6PM)  &&       \
    !defined (PART_TM4C123BE6PM)  && !defined (PART_TM4C123BE6PZ)  &&       \
    !defined (PART_TM4C123BH6PGE) && !defined (PART_TM4C123BH6PM)  &&       \
    !defined (PART_TM4C123BH6PZ)  && !defined (PART_TM4C123BH6ZRB) &&       \
    !defined (PART_TM4C123FE6PM)  && !defined (PART_TM4C123FH6PM)  &&       \
    !defined (PART_TM4C123GE6PM)  && !defined (PART_TM4C123GE6PZ)  &&       \
    !defined (PART_TM4C123GH6PGE) && !defined (PART_TM4C123GH6PM)  &&       \
    !defined (PART_TM4C123GH6PZ)  && !defined (PART_TM4C123GH6ZRB) &&       \
    !defined (PART_TM4C123GH5ZXR)
#include "board.h"
#endif

typedef int IRQn_Type;

#include "core_cm4.h"

/* Including the TivaWare peripheral headers.*/
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_timer.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_gpio.h"
#include "inc/hw_uart.h"
#include "inc/hw_timer.h"
#include "inc/hw_i2c.h"
#include "inc/hw_watchdog.h"
#include "inc/hw_ssi.h"
#include "inc/hw_udma.h"
#include "inc/hw_pwm.h"
#include "inc/hw_adc.h"

#if CORTEX_NUM_VECTORS != ((((NUM_INTERRUPTS - 16) + 7) / 8) * 8)
#error "TivaWare NUM_INTERRUPTS mismatch"
#endif

#endif /* !defined(_FROM_ASM_) */

#endif /* CMPARAMS_H */

/** @} */
