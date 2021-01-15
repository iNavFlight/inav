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
 * @file    SAMA5D2/armparams.h
 * @brief   ARM parameters for the SAMA5D2x.
 *
 * @defgroup ARM_SAMA5D2x Specific Parameters
 * @ingroup ARM_SPECIFIC
 * @details This file contains the ARM specific parameters for the
 *          SAMA5D2x platform.
 * @{
 */

#ifndef ARMPARAMS_H
#define ARMPARAMS_H

/**
 * @brief   ARM core model.
 */
#define ARM_CORE                ARM_CORE_CORTEX_A5

/**
 * @brief   Cortex core model.
 */
#define CORTEX_MODEL            5

/**
 * @brief   Thumb-capable.
 */
#define ARM_SUPPORTS_THUMB      1

/**
 * @brief   Thumb2-capable.
 */
#define ARM_SUPPORTS_THUMB2     1

/**
 * @brief   VFPv4-D16 FPU.
 */
#define TARGET_FEATURE_EXTENSION_REGISTER_COUNT    16

/**
 * @brief   Implementation of the wait-for-interrupt state enter.
 */
#define ARM_WFI_IMPL            asm volatile ("wfi")

#if !defined(_FROM_ASM_) || defined(__DOXYGEN__)
/* If the device type is not externally defined, for example from the Makefile,
   then a file named board.h is included. This file must contain a device
   definition compatible with the vendor include file.*/
#if !defined (SAMA5D21) && !defined (SAMA5D22) &&  !defined (SAMA5D23) &&   \
    !defined (SAMA5D24) && !defined (SAMA5D25) &&  !defined (SAMA5D26) &&   \
    !defined (SAMA5D27) && !defined (SAMA5D28)
#include "board.h"
#endif

/* Including the device CMSIS header. Note, we are not using the definitions
   from this header because we need this file to be usable also from
   assembler source files. We verify that the info matches instead.*/
#include "sama5d2x.h"

/*lint -save -e9029 [10.4] Signedness comes from external files, it is
  unpredictable but gives no problems.*/
#if CORTEX_MODEL != __CORTEX_A
#error "CMSIS __CORTEX_A mismatch"
#endif

/**
 * @brief   Address of the IRQ vector register in the interrupt controller.
 */
#define ARM_IRQ_VECTOR_REG      0xF803C010U
#else
#define ARM_IRQ_VECTOR_REG      0xF803C010
#endif

#define ARM_ENABLE_WFI_IDLE     TRUE

#define ARM_SUPPORTS_L2CC       1

#endif /* ARMPARAMS_H */

/** @} */
