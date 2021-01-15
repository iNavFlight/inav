/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio.

    This file is part of ChibiOS.

    ChibiOS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file    ARMCMx/chcore.h
 * @brief   ARM Cortex-Mx port macros and structures.
 *
 * @addtogroup ARMCMx_CORE
 * @{
 */

#ifndef CHCORE_H
#define CHCORE_H

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/**
 * @name    Architecture and Compiler
 * @{
 */
/**
 * @brief   Macro defining a generic ARM architecture.
 */
#define PORT_ARCHITECTURE_ARM

/* The following code is not processed when the file is included from an
   asm module because those intrinsic macros are not necessarily defined
   by the assembler too.*/
#if !defined(_FROM_ASM_)

/**
 * @brief   Compiler name and version.
 */
#if defined(__GNUC__) || defined(__DOXYGEN__)
#define PORT_COMPILER_NAME              "GCC " __VERSION__

#elif defined(__ICCARM__)
#define PORT_COMPILER_NAME              "IAR"

#elif defined(__CC_ARM)
#define PORT_COMPILER_NAME              "RVCT"

#else
#error "unsupported compiler"
#endif

#endif /* !defined(_FROM_ASM_) */

/** @} */

/* Inclusion of the Cortex-Mx implementation specific parameters.*/
#include "cmparams.h"

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   Enables an alternative timer implementation.
 * @details Usually the port uses a timer interface defined in the file
 *          @p chcore_timer.h, if this option is enabled then the file
 *          @p chcore_timer_alt.h is included instead.
 */
#if !defined(PORT_USE_ALT_TIMER)
#define PORT_USE_ALT_TIMER              FALSE
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/* The following code is not processed when the file is included from an
   asm module.*/
#if !defined(_FROM_ASM_)

/**
 * @brief   Type of a generic ARM register.
 */
typedef void *regarm_t;

/**
 * @brief   Type of stack and memory alignment enforcement.
 * @note    In this architecture the stack alignment is enforced to 64 bits,
 *          32 bits alignment is supported by hardware but deprecated by ARM,
 *          the implementation choice is to not offer the option.
 */
typedef uint64_t stkalign_t;

/* The following declarations are there just for Doxygen documentation, the
   real declarations are inside the sub-headers being specific for the
   sub-architectures.*/
#if defined(__DOXYGEN__)
/**
 * @brief   Interrupt saved context.
 * @details This structure represents the stack frame saved during a
 *          preemption-capable interrupt handler.
 * @note    It is implemented to match the Cortex-Mx exception context.
 */
struct port_extctx {};

/**
 * @brief   System saved context.
 * @details This structure represents the inner stack frame during a context
 *          switch.
 */
struct port_intctx {};
#endif /* defined(__DOXYGEN__) */

/**
 * @brief   Platform dependent part of the @p thread_t structure.
 * @details In this port the structure just holds a pointer to the
 *          @p port_intctx structure representing the stack pointer
 *          at context switch time.
 */
struct port_context {
  struct port_intctx *sp;
};

#endif /* !defined(_FROM_ASM_) */

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Total priority levels.
 */
#define CORTEX_PRIORITY_LEVELS          (1U << CORTEX_PRIORITY_BITS)

/**
 * @brief   Minimum priority level.
 * @details This minimum priority level is calculated from the number of
 *          priority bits supported by the specific Cortex-Mx implementation.
 */
#define CORTEX_MINIMUM_PRIORITY         (CORTEX_PRIORITY_LEVELS - 1)

/**
 * @brief   Maximum priority level.
 * @details The maximum allowed priority level is always zero.
 */
#define CORTEX_MAXIMUM_PRIORITY         0U

/**
 * @brief   Priority level to priority mask conversion macro.
 */
#define CORTEX_PRIO_MASK(n)                                                 \
  ((n) << (8U - (unsigned)CORTEX_PRIORITY_BITS))

/**
 * @brief   Priority level verification macro.
 */
#define PORT_IRQ_IS_VALID_PRIORITY(n)                                       \
  (((n) >= 0U) && ((n) < CORTEX_PRIORITY_LEVELS))

/**
 * @brief   Priority level verification macro.
 */
#define PORT_IRQ_IS_VALID_KERNEL_PRIORITY(n)                                \
  (((n) >= CORTEX_MAX_KERNEL_PRIORITY) && ((n) < CORTEX_PRIORITY_LEVELS))

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

/* Includes the sub-architecture-specific part.*/
#if (CORTEX_MODEL == 0) || (CORTEX_MODEL == 1)
#include "chcore_v6m.h"
#elif (CORTEX_MODEL == 3) || (CORTEX_MODEL == 4) || (CORTEX_MODEL == 7)
#include "mpu.h"
#include "chcore_v7m.h"
#else
#error "unknown Cortex-M variant"
#endif

#if !defined(_FROM_ASM_)

#if CH_CFG_ST_TIMEDELTA > 0
#if PORT_USE_ALT_TIMER == FALSE
#include "chcore_timer.h"
#else /* PORT_USE_ALT_TIMER != FALSE */
#include "chcore_timer_alt.h"
#endif /* PORT_USE_ALT_TIMER != FALSE */
#endif /* CH_CFG_ST_TIMEDELTA > 0 */

#endif /* !defined(_FROM_ASM_) */

#endif /* CHCORE_H */

/** @} */
