/*
    Copyright (C) 2014..2016 Marco Veeneman

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
 * @{
 */

#ifndef _CMPARAMS_H_
#define _CMPARAMS_H_

/**
 * @brief   Cortex core model.
 */
#define CORTEX_MODEL            4

/**
 * @brief   Memory Protection unit presence.
 */
#define CORTEX_HAS_MPU          1

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
#if !defined(TM4C1230C3PM) && !defined(TM4C1230D5PM) &&                       \
  !defined(TM4C1230E6PM) && !defined(TM4C1230H6PM) &&                         \
  !defined(TM4C1231C3PM) && !defined(TM4C1231D5PM) &&                         \
  !defined(TM4C1231D5PZ) && !defined(TM4C1231E6PM) &&                         \
  !defined(TM4C1231E6PZ) && !defined(TM4C1231H6PGE) &&                        \
  !defined(TM4C1231H6PM) && !defined(TM4C1231H6PZ) &&                         \
  !defined(TM4C1232C3PM) && !defined(TM4C1232D5PM) &&                         \
  !defined(TM4C1232E6PM) && !defined(TM4C1232H6PM) &&                         \
  !defined(TM4C1233C3PM) && !defined(TM4C1233D5PM) &&                         \
  !defined(TM4C1233D5PZ) && !defined(TM4C1233E6PM) &&                         \
  !defined(TM4C1233E6PZ) && !defined(TM4C1233H6PGE) &&                        \
  !defined(TM4C1233H6PM) && !defined(TM4C1233H6PZ) &&                         \
  !defined(TM4C1236D5PM) && !defined(TM4C1236E6PM) &&                         \
  !defined(TM4C1236H6PM) && !defined(TM4C1237D5PM) &&                         \
  !defined(TM4C1237D5PZ) && !defined(TM4C1237E6PM) &&                         \
  !defined(TM4C1237E6PZ) && !defined(TM4C1237H6PGE) &&                        \
  !defined(TM4C1237H6PM) && !defined(TM4C1237H6PZ) &&                         \
  !defined(TM4C123AE6PM) && !defined(TM4C123AH6PM) &&                         \
  !defined(TM4C123BE6PM) && !defined(TM4C123BE6PZ) &&                         \
  !defined(TM4C123BH6PGE) && !defined(TM4C123BH6PM) &&                        \
  !defined(TM4C123BH6PZ) && !defined(TM4C123BH6ZRB) &&                        \
  !defined(TM4C123FE6PM) && !defined(TM4C123FH6PM) &&                         \
  !defined(TM4C123GE6PM) && !defined(TM4C123GE6PZ) &&                         \
  !defined(TM4C123GH6PGE) && !defined(TM4C123GH6PM) &&                        \
  !defined(TM4C123GH6PZ) && !defined(TM4C123GH6ZRB) &&                        \
  !defined(TM4C123GH5ZXR)
#include "board.h"
#endif

/* Including the device CMSIS header. Note, we are not using the definitions
   from this header because we need this file to be usable also from
   assembler source files. We verify that the info matches instead.*/
#include "tm4c123x.h"

#if !CORTEX_HAS_MPU != !__MPU_PRESENT
#error "CMSIS __MPU_PRESENT mismatch"
#endif

#if !CORTEX_HAS_FPU != !__FPU_PRESENT
#error "CMSIS __FPU_PRESENT mismatch"
#endif

#if CORTEX_PRIORITY_BITS != __NVIC_PRIO_BITS
#error "CMSIS __NVIC_PRIO_BITS mismatch"
#endif

#endif /* !defined(_FROM_ASM_) */

#endif /* _CMPARAMS_H_ */

/**
 * @}
 */
