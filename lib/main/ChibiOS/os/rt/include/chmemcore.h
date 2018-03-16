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
 * @file    chmemcore.h
 * @brief   Core memory manager macros and structures.
 *
 * @addtogroup memcore
 * @{
 */

#ifndef _CHMEMCORE_H_
#define _CHMEMCORE_H_

#if (CH_CFG_USE_MEMCORE == TRUE) || defined(__DOXYGEN__)

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

/**
 * @brief   Memory get function.
 */
typedef void *(*memgetfunc_t)(size_t size);

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/**
 * @name    Alignment support macros
 */
/**
 * @brief   Alignment size constant.
 */
#define MEM_ALIGN_SIZE      sizeof(stkalign_t)

/**
 * @brief   Alignment mask constant.
 */
#define MEM_ALIGN_MASK      (MEM_ALIGN_SIZE - 1U)

/**
 * @brief   Alignment helper macro.
 */
#define MEM_ALIGN_PREV(p)   ((size_t)(p) & ~MEM_ALIGN_MASK)

/**
 * @brief   Alignment helper macro.
 */
#define MEM_ALIGN_NEXT(p)   MEM_ALIGN_PREV((size_t)(p) + MEM_ALIGN_MASK)

/**
 * @brief   Returns whatever a pointer or memory size is aligned to
 *          the type @p stkalign_t.
 */
#define MEM_IS_ALIGNED(p)   (((size_t)(p) & MEM_ALIGN_MASK) == 0U)
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void _core_init(void);
  void *chCoreAlloc(size_t size);
  void *chCoreAllocI(size_t size);
  size_t chCoreGetStatusX(void);
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

#endif /* CH_CFG_USE_MEMCORE == TRUE */

#endif /* _CHMEMCORE_H_ */

/** @} */
