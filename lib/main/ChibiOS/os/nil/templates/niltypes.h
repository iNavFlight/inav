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
 * @file    templates/niltypes.h
 * @brief   Port system types.
 *
 * @addtogroup NIL_TYPES
 * @{
 */

#ifndef _NILTYPES_H_
#define _NILTYPES_H_

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

typedef uint32_t        syssts_t;       /**< System status word.            */
typedef uint32_t        rtcnt_t;        /**< Realtime counter.              */
typedef uint8_t         tstate_t;       /**< Thread state.                  */
typedef int32_t         msg_t;          /**< Inter-thread message.          */
typedef uint32_t        eventmask_t;    /**< Mask of event identifiers.     */
typedef int32_t         cnt_t;          /**< Generic signed counter.        */
typedef uint32_t        ucnt_t;         /**< Generic unsigned counter.      */

/**
 * @brief   Type of system time.
 */
#if (NIL_CFG_ST_RESOLUTION == 32) || defined(__DOXYGEN__)
typedef uint32_t systime_t;
#else
typedef uint16_t systime_t;
#endif

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
#define PORT_THD_FUNCTION(tname, arg)                                       \
  __attribute__((noreturn)) void tname(void *arg)

/**
 * @brief   Packed variable specifier.
 */
#define PACKED_VAR __attribute__((packed))

#endif /* _NILTYPES_H_ */

/** @} */
