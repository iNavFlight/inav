/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio.

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
 * @file    ARM/compilers/GCC/chtypes.h
 * @brief   ARM port system types.
 *
 * @addtogroup ARM_GCC_CORE
 * @{
 */

#ifndef _CHTYPES_H_
#define _CHTYPES_H_

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
#define FALSE               0
#endif

/**
 * @brief   Generic 'true' boolean constant.
 */
#if !defined(TRUE) || defined(__DOXYGEN__)
#define TRUE                1
#endif
/** @} */

/**
 * @name    Kernel types
 * @{
 */
typedef uint32_t            rtcnt_t;        /**< Realtime counter.          */
typedef uint64_t            rttime_t;       /**< Realtime accumulator.      */
typedef uint32_t            syssts_t;       /**< System status word.        */
typedef uint8_t             tmode_t;        /**< Thread flags.              */
typedef uint8_t             tstate_t;       /**< Thread state.              */
typedef uint8_t             trefs_t;        /**< Thread references counter. */
typedef uint8_t             tslices_t;      /**< Thread time slices counter.*/
typedef uint32_t            tprio_t;        /**< Thread priority.           */
typedef int32_t             msg_t;          /**< Inter-thread message.      */
typedef int32_t             eventid_t;      /**< Numeric event identifier.  */
typedef uint32_t            eventmask_t;    /**< Mask of event identifiers. */
typedef uint32_t            eventflags_t;   /**< Mask of event flags.       */
typedef int32_t             cnt_t;          /**< Generic signed counter.    */
typedef uint32_t            ucnt_t;         /**< Generic unsigned counter.  */
/** @} */

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
#define PORT_THD_FUNCTION(tname, arg) void tname(void *arg)

/**
 * @brief   Packed variable specifier.
 */
#define PACKED_VAR __attribute__((packed))

#endif /* _CHTYPES_H_ */

/** @} */
