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
 * @file    AVR/compilers/GCC/chtypes.h
 * @brief   AVR architecture port system types.
 *
 * @addtogroup AVR_CORE
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
#define TRUE                (!FALSE)
#endif
/** @} */

typedef bool            bool_t;         /**< Fast boolean type.             */
typedef uint8_t         syssts_t;       /**< System status word.            */
typedef uint8_t         tmode_t;        /**< Thread flags.                  */
typedef uint8_t         tstate_t;       /**< Thread state.                  */
typedef uint8_t         trefs_t;        /**< Thread references counter.     */
typedef uint8_t         tslices_t;      /**< Thread time slices counter.    */
typedef uint8_t         tprio_t;        /**< Thread priority.               */
typedef int16_t         msg_t;          /**< Inter-thread message.          */
typedef uint8_t         eventid_t;      /**< Event Id.                      */
typedef uint8_t         eventmask_t;    /**< Event mask.                    */
typedef uint8_t         eventflags_t;   /**< Event flags.                   */
typedef int8_t          cnt_t;          /**< Resources counter.             */

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
