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

/**
 * @brief   Adds a reference to a thread object.
 * @pre     The configuration option @p CH_CFG_USE_DYNAMIC must be enabled in
 *          order to use this function.
 *
 * @param[in] tp        pointer to the thread
 * @return              The same thread pointer passed as parameter
 *                      representing the new reference.
 *
 * @api
 */
thread_t *chThdAddRef(thread_t *tp) {

  chSysLock();
  chDbgAssert(tp->p_refs < (trefs_t)255, "too many references");
  tp->p_refs++;
  chSysUnlock();

  return tp;
}

/**
 * @brief   Releases a reference to a thread object.
 * @details If the references counter reaches zero <b>and</b> the thread
 *          is in the @p CH_STATE_FINAL state then the thread's memory is
 *          returned to the proper allocator.
 * @pre     The configuration option @p CH_CFG_USE_DYNAMIC must be enabled in
 *          order to use this function.
 * @note    Static threads are not affected.
 *
 * @param[in] tp        pointer to the thread
 *
 * @api
 */
void chThdRelease(thread_t *tp) {
  trefs_t refs;

  chSysLock();
  chDbgAssert(tp->p_refs > (trefs_t)0, "not referenced");
  tp->p_refs--;
  refs = tp->p_refs;

  /* If the references counter reaches zero and the thread is in its
     terminated state then the memory can be returned to the proper
     allocator. Of course static threads are not affected.*/
  if ((refs == (trefs_t)0) && (tp->p_state == CH_STATE_FINAL)) {
    switch (tp->p_flags & CH_FLAG_MODE_MASK) {
#if CH_CFG_USE_HEAP == TRUE
    case CH_FLAG_MODE_HEAP:
#if CH_CFG_USE_REGISTRY == TRUE
      REG_REMOVE(tp);
#endif
      chSysUnlock();
      chHeapFree(tp);
      return;
#endif
#if CH_CFG_USE_MEMPOOLS == TRUE
    case CH_FLAG_MODE_MPOOL:
#if CH_CFG_USE_REGISTRY == TRUE
      REG_REMOVE(tp);
#endif
      chSysUnlock();
      chPoolFree(tp->p_mpool, tp);
      return;
#endif
    default:
      /* Nothing to do for static threads, those are removed from the
         registry on exit.*/
      break;
    }
  }
  chSysUnlock();
}

#if (CH_CFG_USE_HEAP == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   Creates a new thread allocating the memory from the heap.
 * @pre     The configuration options @p CH_CFG_USE_DYNAMIC and
 *          @p CH_CFG_USE_HEAP must be enabled in order to use this function.
 * @note    A thread can terminate by calling @p chThdExit() or by simply
 *          returning from its main function.
 * @note    The memory allocated for the thread is not released when the thread
 *          terminates but when a @p chThdWait() is performed.
 *
 * @param[in] heapp     heap from which allocate the memory or @p NULL for the
 *                      default heap
 * @param[in] size      size of the working area to be allocated
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
                              tprio_t prio, tfunc_t pf, void *arg) {
  void *wsp;
  thread_t *tp;

  wsp = chHeapAlloc(heapp, size);
  if (wsp == NULL) {
    return NULL;
  }

#if CH_DBG_FILL_THREADS == TRUE
  _thread_memfill((uint8_t *)wsp,
                  (uint8_t *)wsp + sizeof(thread_t),
                  CH_DBG_THREAD_FILL_VALUE);
  _thread_memfill((uint8_t *)wsp + sizeof(thread_t),
                  (uint8_t *)wsp + size,
                  CH_DBG_STACK_FILL_VALUE);
#endif

  chSysLock();
  tp = chThdCreateI(wsp, size, prio, pf, arg);
  tp->p_flags = CH_FLAG_MODE_HEAP;
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
 * @note    A thread can terminate by calling @p chThdExit() or by simply
 *          returning from its main function.
 * @note    The memory allocated for the thread is not released when the thread
 *          terminates but when a @p chThdWait() is performed.
 *
 * @param[in] mp        pointer to the memory pool object
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
thread_t *chThdCreateFromMemoryPool(memory_pool_t *mp, tprio_t prio,
                                    tfunc_t pf, void *arg) {
  void *wsp;
  thread_t *tp;

  chDbgCheck(mp != NULL);

  wsp = chPoolAlloc(mp);
  if (wsp == NULL) {
    return NULL;
  }

#if CH_DBG_FILL_THREADS == TRUE
  _thread_memfill((uint8_t *)wsp,
                  (uint8_t *)wsp + sizeof(thread_t),
                  CH_DBG_THREAD_FILL_VALUE);
  _thread_memfill((uint8_t *)wsp + sizeof(thread_t),
                  (uint8_t *)wsp + mp->mp_object_size,
                  CH_DBG_STACK_FILL_VALUE);
#endif

  chSysLock();
  tp = chThdCreateI(wsp, mp->mp_object_size, prio, pf, arg);
  tp->p_flags = CH_FLAG_MODE_MPOOL;
  tp->p_mpool = mp;
  chSchWakeupS(tp, MSG_OK);
  chSysUnlock();

  return tp;
}
#endif /* CH_CFG_USE_MEMPOOLS == TRUE */

#endif /* CH_CFG_USE_DYNAMIC == TRUE */

/** @} */
