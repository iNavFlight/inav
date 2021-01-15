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
 * @file    rt/include/ch.h
 * @brief   ChibiOS/RT main include file.
 *
 * @addtogroup kernel_info
 * @details This header includes all the required kernel headers so it is the
 *          only kernel header you usually want to include in your application.
 * @details Kernel related info.
 * @{
 */

#ifndef CH_H
#define CH_H

/**
 * @brief   ChibiOS/RT identification macro.
 */
#define _CHIBIOS_RT_

/**
 * @brief   Stable release flag.
 */
#define CH_KERNEL_STABLE        1

/**
 * @name    ChibiOS/RT version identification
 * @{
 */
/**
 * @brief   Kernel version string.
 */
#define CH_KERNEL_VERSION       "6.0.3"

/**
 * @brief   Kernel version major number.
 */
#define CH_KERNEL_MAJOR         6

/**
 * @brief   Kernel version minor number.
 */
#define CH_KERNEL_MINOR         0

/**
 * @brief   Kernel version patch number.
 */
#define CH_KERNEL_PATCH         3
/** @} */

/**
 * @name    Constants for configuration options
 */
/**
 * @brief   Generic 'false' preprocessor boolean constant.
 * @note    It is meant to be used in configuration files as switch.
 */
#if !defined(FALSE) || defined(__DOXYGEN__)
#define FALSE                   0
#endif

/**
 * @brief   Generic 'true' preprocessor boolean constant.
 * @note    It is meant to be used in configuration files as switch.
 */
#if !defined(TRUE) || defined(__DOXYGEN__)
#define TRUE                    1
#endif
/** @} */

/* Configuration headers, checks and licensing restrictions.*/
#include "chconf.h"
#include "chchecks.h"
#include "chlicense.h"
#include "chrestrictions.h"

/* Early function prototype required by the following headers.*/
#ifdef __cplusplus
extern "C" {
#endif
  void chSysHalt(const char *reason);
#ifdef __cplusplus
}
#endif

/* Base kernel headers.*/
#include "chtypes.h" /* TODO: Rename and rework.*/
#include "chsystypes.h"
#include "chdebug.h"
#include "chtime.h"
#include "chalign.h"
#include "chcore.h"
#include "chtrace.h"
#include "chtm.h"
#include "chstats.h"
#include "chschd.h"
#include "chsys.h"
#include "chvt.h"
#include "chthreads.h"

/* Optional subsystems headers.*/
#include "chregistry.h"
#include "chsem.h"
#include "chmtx.h"
#include "chcond.h"
#include "chevents.h"
#include "chmsg.h"

/* OSLIB.*/
#include "chlib.h"

/* Headers dependent on the OSLIB.*/
#include "chdynamic.h"

#endif /* CH_H */

/** @} */
