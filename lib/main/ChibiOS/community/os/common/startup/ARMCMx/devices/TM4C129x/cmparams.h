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
 * @file    TM4C129x/cmparams.h
 * @brief   ARM Cortex-M4 parameters for the TM4C129x.
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
#define CORTEX_NUM_VECTORS      112

/* The following code is not processed when the file is included from an
   asm module.*/
#if !defined(_FROM_ASM_)

/* If the device type is not externally defined, for example from the Makefile,
   then a file named board.h is included. This file must contain a device
   definition compatible with the include file.*/
#if !defined(TM4C1290NCPDT) && !defined(TM4C1290NCZAD)                        \
  && !defined(TM4C1292NCPDT) && !defined(TM4C1292NCZAD)                       \
  && !defined(TM4C1294KCPDT) && !defined(TM4C1294NCPDT)                       \
  && !defined(TM4C1294NCZAD) && !defined(TM4C1297NCZAD)                       \
  && !defined(TM4C1299KCZAD) && !defined(TM4C1299NCZAD)                       \
  && !defined(TM4C129CNCPDT) && !defined(TM4C129CNCZAD)                       \
  && !defined(TM4C129DNCPDT) && !defined(TM4C129DNCZAD)                       \
  && !defined(TM4C129EKCPDT) && !defined(TM4C129ENCPDT)                       \
  && !defined(TM4C129ENCZAD) && !defined(TM4C129LNCZAD)                       \
  && !defined(TM4C129XKCZAD) && !defined(TM4C129XNCZAD)
#include "board.h"
#endif

/* Including the device CMSIS header. Note, we are not using the definitions
   from this header because we need this file to be usable also from
   assembler source files. We verify that the info matches instead.*/
#include "tm4c129x.h"

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
