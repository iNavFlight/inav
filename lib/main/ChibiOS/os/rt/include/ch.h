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
 * @file    ch.h
 * @brief   ChibiOS/RT main include file.
 * @details This header includes all the required kernel headers so it is the
 *          only kernel header you usually want to include in your application.
 *
 * @addtogroup kernel_info
 * @details Kernel related info.
 * @{
 */

#ifndef _CH_H_
#define _CH_H_

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
#define CH_KERNEL_VERSION       "3.1.4"

/**
 * @brief   Kernel version major number.
 */
#define CH_KERNEL_MAJOR         3

/**
 * @brief   Kernel version minor number.
 */
#define CH_KERNEL_MINOR         1

/**
 * @brief   Kernel version patch number.
 */
#define CH_KERNEL_PATCH         4
/** @} */

/* Core headers.*/
#include "chtypes.h"
#include "chconf.h"
#include "chlicense.h"
#include "chsystypes.h"
#include "chcore.h"
#include "chdebug.h"
#include "chtm.h"
#include "chstats.h"
#include "chschd.h"
#include "chsys.h"
#include "chvt.h"
#include "chthreads.h"

/* Optional subsystems headers.*/
#include "chregistry.h"
#include "chsem.h"
#include "chbsem.h"
#include "chmtx.h"
#include "chcond.h"
#include "chevents.h"
#include "chmsg.h"
#include "chmboxes.h"
#include "chmemcore.h"
#include "chheap.h"
#include "chmempools.h"
#include "chdynamic.h"
#include "chqueues.h"
#include "chstreams.h"

#endif /* _CH_H_ */

/** @} */
