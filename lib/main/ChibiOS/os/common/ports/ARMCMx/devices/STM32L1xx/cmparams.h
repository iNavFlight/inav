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
 * @file    STM32L1xx/cmparams.h
 * @brief   ARM Cortex-M3 parameters for the STM32L1xx.
 *
 * @defgroup ARMCMx_STM32L1xx STM32L1xx Specific Parameters
 * @ingroup ARMCMx_SPECIFIC
 * @details This file contains the Cortex-M3 specific parameters for the
 *          STM32L1xx platform.
 * @{
 */

#ifndef _CMPARAMS_H_
#define _CMPARAMS_H_

/**
 * @brief   Cortex core model.
 */
#define CORTEX_MODEL            3

/**
 * @brief   Floating Point unit presence.
 */
#define CORTEX_HAS_FPU          0

/**
 * @brief   Number of bits in priority masks.
 */
#define CORTEX_PRIORITY_BITS    4

/**
 * @brief   Number of interrupt vectors.
 * @note    This number does not include the 16 system vectors and must be
 *          rounded to a multiple of 8.
 */
#define CORTEX_NUM_VECTORS      64

/* The following code is not processed when the file is included from an
   asm module.*/
#if !defined(_FROM_ASM_)

/* If the device type is not externally defined, for example from the Makefile,
   then a file named board.h is included. This file must contain a device
   definition compatible with the vendor include file.*/
#if !defined(STM32L100xB)  && !defined(STM32L100xBA) &&                     \
    !defined(STM32L100xC)  && !defined(STM32L151xB)  &&                     \
    !defined(STM32L151xBA) && !defined(STM32L151xC)  &&                     \
    !defined(STM32L151xCA) && !defined(STM32L151xD)  &&                     \
    !defined(STM32L151xDX) && !defined(STM32L151xE)  &&                     \
    !defined(STM32L152xB)  && !defined(STM32L152xBA) &&                     \
    !defined(STM32L152xC)  && !defined(STM32L152xCA) &&                     \
    !defined(STM32L152xD)  && !defined(STM32L152xDX) &&                     \
    !defined(STM32L152xE)  && !defined(STM32L162xC)  &&                     \
    !defined(STM32L162xCA) && !defined(STM32L162xD)  &&                     \
    !defined(STM32L162xDX) && !defined(STM32L162xE)
#include "board.h"
#endif

/* Including the device CMSIS header. Note, we are not using the definitions
   from this header because we need this file to be usable also from
   assembler source files. We verify that the info matches instead.*/
#include "stm32l1xx.h"

#if CORTEX_MODEL != __CORTEX_M
#error "CMSIS __CORTEX_M mismatch"
#endif

#if CORTEX_PRIORITY_BITS != __NVIC_PRIO_BITS
#error "CMSIS __NVIC_PRIO_BITS mismatch"
#endif

/* Fix for yet another consistency error in ST headers.*/
#define SVCall_IRQn SVC_IRQn

#endif /* !defined(_FROM_ASM_) */

#endif /* _CMPARAMS_H_ */

/** @} */
