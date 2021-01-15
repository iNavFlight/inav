/*
    ChibiOS/HAL - Copyright (C) 2016 Andrew Wygle aka awygle

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
 * @file    MSP430X/compilers/GCC/chtypes.h
 * @brief   MSP430X port system types.
 *
 * @addtogroup MSP430X_GCC_CORE
 * @{
 */

#ifndef CHTYPES_H
#define CHTYPES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @name    Common constants
 */
/**
 * @brief   Generic 'false' boolean constant.
 */
#if !defined(FALSE) || defined(__DOXYGEN__)
#define FALSE                   0
#endif

/**
 * @brief   Generic 'true' boolean constant.
 */
#if !defined(TRUE) || defined(__DOXYGEN__)
#define TRUE                    1
#endif
/** @} */

/**
 * @name    Kernel types
 * @{
 */
typedef uint16_t            rtcnt_t;        /**< Realtime counter.          */
typedef uint64_t            rttime_t;       /**< Realtime accumulator.      */
typedef uint16_t            syssts_t;       /**< System status word.        */
typedef uint8_t             tmode_t;        /**< Thread flags.              */
typedef uint8_t             tstate_t;       /**< Thread state.              */
typedef uint8_t             trefs_t;        /**< Thread references counter. */
typedef uint8_t             tslices_t;      /**< Thread time slices counter.*/
typedef uint8_t             tprio_t;        /**< Thread priority.           */
typedef int16_t             msg_t;          /**< Inter-thread message.      */
typedef int32_t             eventid_t;      /**< Numeric event identifier.  */
typedef uint8_t             eventmask_t;    /**< Mask of event identifiers. */
typedef uint16_t            eventflags_t;   /**< Mask of event flags.       */
typedef int16_t             cnt_t;          /**< Generic signed counter.    */
typedef uint16_t            ucnt_t;         /**< Generic unsigned counter.  */

/**
 * @brief   ROM constant modifier.
 * @note    It is set to use the "const" keyword in this port.
 */
#define ROMCONST const

/**
 * @brief   Makes functions not inlineable.
 * @note    If the compiler does not support such attribute then the
 *          realtime counter precision could be degraded.
 */
#define NOINLINE __attribute__((noinline))

/**
 * @brief   Optimized thread function declaration macro.
 */
#define PORT_THD_FUNCTION(tname, arg)                                         \
  void tname(void *arg)

/**
 * @brief   Packed variable specifier.
 */
#define PACKED_VAR __attribute__((packed))

/**
 * @brief   Memory alignment enforcement for variables.
 */
#define ALIGNED_VAR(n) __attribute__((aligned(n)))

/**
 * @brief   Size of a pointer.
 * @note    To be used where the sizeof operator cannot be used, preprocessor
 *          expressions for example.
 */
#define SIZEOF_PTR          4

/**
 * @brief   True if alignment is low-high in current architecture.
 */
#define REVERSE_ORDER       1

#endif /* CHTYPES_H */

/** @} */
