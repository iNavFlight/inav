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
 * @file    ARMCMx/compilers/GCC/vectors.h
 * @brief   Interrupt vectors for Cortex-Mx devices.
 *
 * @defgroup ARMCMx_VECTORS Cortex-Mx Interrupt Vectors
 * @{
 */

#ifndef _VECTORS_H_
#define _VECTORS_H_

#include "cmparams.h"

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

#if !defined(_FROM_ASM_)
/**
 * @brief   Type of an IRQ vector.
 */
typedef void  (*irq_vector_t)(void);

/**
 * @brief   Type of a structure representing the whole vectors table.
 */
typedef struct {
  uint32_t      *init_stack;
  irq_vector_t  reset_handler;
  irq_vector_t  nmi_handler;
  irq_vector_t  hardfault_handler;
  irq_vector_t  memmanage_handler;
  irq_vector_t  busfault_handler;
  irq_vector_t  usagefault_handler;
  irq_vector_t  vector1c;
  irq_vector_t  vector20;
  irq_vector_t  vector24;
  irq_vector_t  vector28;
  irq_vector_t  svc_handler;
  irq_vector_t  debugmonitor_handler;
  irq_vector_t  vector34;
  irq_vector_t  pendsv_handler;
  irq_vector_t  systick_handler;
  irq_vector_t  vectors[CORTEX_NUM_VECTORS];
} vectors_t;
#endif /* !defined(_FROM_ASM_) */

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if !defined(_FROM_ASM_)
extern vectors_t _vectors;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

#endif /* _VECTORS_H_ */

/** @} */
