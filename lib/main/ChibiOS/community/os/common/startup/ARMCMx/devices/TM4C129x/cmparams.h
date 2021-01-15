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
 * @file    TM4C129x/cmparams.h
 * @brief   ARM Cortex-M4 parameters for the TM4C129x.
 *
 * @defgroup ARMCMx_TM4C129x TM4C129x Specific Parameters
 * @ingroup ARMCMx_SPECIFIC
 * @details This file contains the Cortex-M4 specific parameters for the
 *          TM4C129x platform.
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
#define CORTEX_NUM_VECTORS      120

/* The following code is not processed when the file is included from an
   asm module.*/
#if !defined(_FROM_ASM_)

/* If the device type is not externally defined, for example from the Makefile,
   then a file named board.h is included. This file must contain a device
   definition compatible with the include file.*/
#if !defined (PART_TM4C1290NCPDT) && !defined (PART_TM4C1290NCZAD) &&       \
    !defined (PART_TM4C1292NCPDT) && !defined (PART_TM4C1292NCZAD) &&       \
    !defined (PART_TM4C1294KCPDT) && !defined (PART_TM4C1294NCPDT) &&       \
    !defined (PART_TM4C1294NCZAD) && !defined (PART_TM4C1297NCZAD) &&       \
    !defined (PART_TM4C1299KCZAD) && !defined (PART_TM4C1299NCZAD) &&       \
    !defined (PART_TM4C129CNCPDT) && !defined (PART_TM4C129CNCZAD) &&       \
    !defined (PART_TM4C129DNCPDT) && !defined (PART_TM4C129DNCZAD) &&       \
    !defined (PART_TM4C129EKCPDT) && !defined (PART_TM4C129ENCPDT) &&       \
    !defined (PART_TM4C129ENCZAD) && !defined (PART_TM4C129LNCZAD) &&       \
    !defined (PART_TM4C129XKCZAD) && !defined (PART_TM4C129XNCZAD)
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
#include "inc/hw_emac.h"
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
