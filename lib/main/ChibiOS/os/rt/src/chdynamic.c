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
 * @file    chdynamic.c
 * @brief   Dynamic threads code.
 *
 * @addtogroup dynamic_threads
 * @details Dynamic threads related APIs and services.
 * @{
 */

#include "ch.h"

#if (CH_CFG_USE_DYNAMIC == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Module local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

#if (CH_CFG_USE_HEAP == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   Creates a new thread allocating the memory from the heap.
 * @pre     The configuration options @p CH_CFG_USE_DYNAMIC and
 *          @p CH_CFG_USE_HEAP must be enabled in order to use this function.
 * @note    A thread can terminate by calling @p chThdExit() or by simply
 *          returning from its main function.
 * @note    The memory allocated for the thread is not released automatically,
 *          it is responsibility of the creator thread to call @p chThdWait()
 *          and then release the allocated memory.
 *
 * @param[in] heapp     heap from which allocate the memory or @p NULL for the
 *                      default heap
 * @param[in] size      size of the working area to be allocated
 * @param[in] name      thread name
 * @param[in] prio      the priority level for the new thread
 * @param[in] pf        the thread function
 * @param[in] arg       an argument passed to the thread function. It can be
 *                      @p NULL.
 * @return              The pointer to the @p thread_t structure allocated for
 *                      the thread into the working space area.
 * @retval NULL         if the memory cannot be allocated.
 *
 * @api
 */
thread_t *chThdCreateFromHeap(memory_heap_t *heapp, size_t size,
                              const char *name, tprio_t prio,
                              tfunc_t pf, void *arg) {
  thread_t *tp;
  void *wsp;

  wsp = chHeapAllocAligned(heapp, size, PORT_WORKING_AREA_ALIGN);
  if (wsp == NULL) {
    return NULL;
  }

  thread_descriptor_t td = {
    name,
    wsp,
    (stkalign_t *)((uint8_t *)wsp + size),
    prio,
    pf,
    arg
  };

#if CH_DBG_FILL_THREADS == TRUE
  _thread_memfill((uint8_t *)wsp,
                  (uint8_t *)wsp + size,
                  CH_DBG_STACK_FILL_VALUE);
#endif

  chSysLock();
  tp = chThdCreateSuspendedI(&td);
  tp->flags = CH_FLAG_MODE_HEAP;
  chSchWakeupS(tp, MSG_OK);
  chSysUnlock();

  return tp;
}
#endif /* CH_CFG_USE_HEAP == TRUE */

#if (CH_CFG_USE_MEMPOOLS == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   Creates a new thread allocating the memory from the specified
 *          memory pool.
 * @pre     The configuration options @p CH_CFG_USE_DYNAMIC and
 *          @p CH_CFG_USE_MEMPOOLS must be enabled in order to use this
 *          function.
 * @pre     The pool must be initialized to contain only objects with
 *          alignment @p PORT_WORKING_AREA_ALIGN.
 * @note    A thread can terminate by calling @p chThdExit() or by simply
 *          returning from its main function.
 * @note    The memory allocated for the thread is not released automatically,
 *          it is responsibility of the creator thread to call @p chThdWait()
 *          and then release the allocated memory.
 *
 * @param[in] mp        pointer to the memory pool object
 * @param[in] name      thread name
 * @param[in] prio      the priority level for the new thread
 * @param[in] pf        the thread function
 * @param[in] arg       an argument passed to the thread function. It can be
 *                      @p NULL.
 * @return              The pointer to the @p thread_t structure allocated for
 *                      the thread into the working space area.
 * @retval  NULL        if the memory pool is empty.
 *
 * @api
 */
thread_t *chThdCreateFromMemoryPool(memory_pool_t *mp, const char *name,
                                    tprio_t prio, tfunc_t pf, void *arg) {
  thread_t *tp;
  void *wsp;

  chDbgCheck(mp != NULL);

  wsp = chPoolAlloc(mp);
  if (wsp == NULL) {
    return NULL;
  }

  thread_descriptor_t td = {
    name,
    wsp,
    (stkalign_t *)((uint8_t *)wsp + mp->object_size),
    prio,
    pf,
    arg
  };

#if CH_DBG_FILL_THREADS == TRUE
  _thread_memfill((uint8_t *)wsp,
                  (uint8_t *)wsp + mp->object_size,
                  CH_DBG_STACK_FILL_VALUE);
#endif

  chSysLock();
  tp = chThdCreateSuspendedI(&td);
  tp->flags = CH_FLAG_MODE_MPOOL;
  tp->mpool = mp;
  chSchWakeupS(tp, MSG_OK);
  chSysUnlock();

  return tp;
}
#endif /* CH_CFG_USE_MEMPOOLS == TRUE */

#endif /* CH_CFG_USE_DYNAMIC == TRUE */

/** @} */
