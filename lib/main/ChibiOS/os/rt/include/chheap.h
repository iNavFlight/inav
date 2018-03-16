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
 * @file    chheap.h
 * @brief   Heaps macros and structures.
 *
 * @addtogroup heaps
 * @{
 */

#ifndef _CHHEAP_H_
#define _CHHEAP_H_

#if (CH_CFG_USE_HEAP == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if CH_CFG_USE_MEMCORE == FALSE
#error "CH_CFG_USE_HEAP requires CH_CFG_USE_MEMCORE"
#endif

#if (CH_CFG_USE_MUTEXES == FALSE) && (CH_CFG_USE_SEMAPHORES == FALSE)
#error "CH_CFG_USE_HEAP requires CH_CFG_USE_MUTEXES and/or CH_CFG_USE_SEMAPHORES"
#endif

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a memory heap.
 */
typedef struct memory_heap memory_heap_t;

/**
 * @brief   Memory heap block header.
 */
union heap_header {
  stkalign_t align;
  struct {
    union {
      union heap_header *next;      /**< @brief Next block in free list.    */
      memory_heap_t     *heap;      /**< @brief Block owner heap.           */
    } u;                            /**< @brief Overlapped fields.          */
    size_t              size;       /**< @brief Size of the memory block.   */
  } h;
};

/**
 * @brief   Structure describing a memory heap.
 */
struct memory_heap {
  memgetfunc_t          h_provider; /**< @brief Memory blocks provider for
                                                this heap.                  */
  union heap_header     h_free;     /**< @brief Free blocks list header.    */
#if CH_CFG_USE_MUTEXES == TRUE
  mutex_t               h_mtx;      /**< @brief Heap access mutex.          */
#else
  semaphore_t           h_sem;      /**< @brief Heap access semaphore.      */
#endif
};

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void _heap_init(void);
  void chHeapObjectInit(memory_heap_t *heapp, void *buf, size_t size);
  void *chHeapAlloc(memory_heap_t *heapp, size_t size);
  void chHeapFree(void *p);
  size_t chHeapStatus(memory_heap_t *heapp, size_t *sizep);
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

#endif /* CH_CFG_USE_HEAP == TRUE */

#endif /* _CHHEAP_H_ */

/** @} */
