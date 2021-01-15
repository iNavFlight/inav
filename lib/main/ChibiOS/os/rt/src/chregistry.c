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
 * @file    chregistry.c
 * @brief   Threads registry code.
 *
 * @addtogroup registry
 * @details Threads Registry related APIs and services.
 *          <h2>Operation mode</h2>
 *          The Threads Registry is a double linked list that holds all the
 *          active threads in the system.<br>
 *          Operations defined for the registry:
 *          - <b>First</b>, returns the first, in creation order, active thread
 *            in the system.
 *          - <b>Next</b>, returns the next, in creation order, active thread
 *            in the system.
 *          .
 *          The registry is meant to be mainly a debug feature, for example,
 *          using the registry a debugger can enumerate the active threads
 *          in any given moment or the shell can print the active threads
 *          and their state.<br>
 *          Another possible use is for centralized threads memory management,
 *          terminating threads can pulse an event source and an event handler
 *          can perform a scansion of the registry in order to recover the
 *          memory.
 * @pre     In order to use the threads registry the @p CH_CFG_USE_REGISTRY
 *          option must be enabled in @p chconf.h.
 * @{
 */

#include <string.h>

#include "ch.h"

#if (CH_CFG_USE_REGISTRY == TRUE) || defined(__DOXYGEN__)

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

#define _offsetof(st, m)                                                    \
  /*lint -save -e9005 -e9033 -e413 [11.8, 10.8 1.3] Normal pointers
    arithmetic, it is safe.*/                                               \
  ((size_t)((char *)&((st *)0)->m - (char *)0))                             \
  /*lint -restore*/

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/*
 * OS signature in ROM plus debug-related information.
 */
ROMCONST chdebug_t ch_debug = {
  {'m', 'a', 'i', 'n'},
  (uint8_t)0,
  (uint8_t)sizeof (chdebug_t),
  (uint16_t)(((unsigned)CH_KERNEL_MAJOR << 11U) |
             ((unsigned)CH_KERNEL_MINOR << 6U) |
             ((unsigned)CH_KERNEL_PATCH << 0U)),
  (uint8_t)sizeof (void *),
  (uint8_t)sizeof (systime_t),
  (uint8_t)sizeof (thread_t),
  (uint8_t)_offsetof(thread_t, prio),
  (uint8_t)_offsetof(thread_t, ctx),
  (uint8_t)_offsetof(thread_t, newer),
  (uint8_t)_offsetof(thread_t, older),
  (uint8_t)_offsetof(thread_t, name),
#if (CH_DBG_ENABLE_STACK_CHECK == TRUE) || (CH_CFG_USE_DYNAMIC == TRUE)
  (uint8_t)_offsetof(thread_t, wabase),
#else
  (uint8_t)0,
#endif
  (uint8_t)_offsetof(thread_t, state),
  (uint8_t)_offsetof(thread_t, flags),
#if CH_CFG_USE_DYNAMIC == TRUE
  (uint8_t)_offsetof(thread_t, refs),
#else
  (uint8_t)0,
#endif
#if CH_CFG_TIME_QUANTUM > 0
  (uint8_t)_offsetof(thread_t, ticks),
#else
  (uint8_t)0,
#endif
#if CH_DBG_THREADS_PROFILING == TRUE
  (uint8_t)_offsetof(thread_t, time)
#else
  (uint8_t)0
#endif
};

/**
 * @brief   Returns the first thread in the system.
 * @details Returns the most ancient thread in the system, usually this is
 *          the main thread unless it terminated. A reference is added to the
 *          returned thread in order to make sure its status is not lost.
 * @note    This function cannot return @p NULL because there is always at
 *          least one thread in the system.
 *
 * @return              A reference to the most ancient thread.
 *
 * @api
 */
thread_t *chRegFirstThread(void) {
  thread_t *tp;

  chSysLock();
  tp = ch.rlist.newer;
#if CH_CFG_USE_DYNAMIC == TRUE
  tp->refs++;
#endif
  chSysUnlock();

  return tp;
}

/**
 * @brief   Returns the thread next to the specified one.
 * @details The reference counter of the specified thread is decremented and
 *          the reference counter of the returned thread is incremented.
 *
 * @param[in] tp        pointer to the thread
 * @return              A reference to the next thread.
 * @retval NULL         if there is no next thread.
 *
 * @api
 */
thread_t *chRegNextThread(thread_t *tp) {
  thread_t *ntp;

  chSysLock();
  ntp = tp->newer;
  /*lint -save -e9087 -e740 [11.3, 1.3] Cast required by list handling.*/
  if (ntp == (thread_t *)&ch.rlist) {
  /*lint -restore*/
    ntp = NULL;
  }
#if CH_CFG_USE_DYNAMIC == TRUE
  else {
    chDbgAssert(ntp->refs < (trefs_t)255, "too many references");
    ntp->refs++;
  }
#endif
  chSysUnlock();
#if CH_CFG_USE_DYNAMIC == TRUE
  chThdRelease(tp);
#endif

  return ntp;
}

/**
 * @brief   Retrieves a thread pointer by name.
 * @note    The reference counter of the found thread is increased by one so
 *          it cannot be disposed incidentally after the pointer has been
 *          returned.
 *
 * @param[in] name      the thread name
 * @return              A pointer to the found thread.
 * @retval NULL         if a matching thread has not been found.
 *
 * @api
 */
thread_t *chRegFindThreadByName(const char *name) {
  thread_t *ctp;

  /* Scanning registry.*/
  ctp = chRegFirstThread();
  do {
    if (strcmp(chRegGetThreadNameX(ctp), name) == 0) {
      return ctp;
    }
    ctp = chRegNextThread(ctp);
  } while (ctp != NULL);

  return NULL;
}

/**
 * @brief   Confirms that a pointer is a valid thread pointer.
 * @note    The reference counter of the found thread is increased by one so
 *          it cannot be disposed incidentally after the pointer has been
 *          returned.
 *
 * @param[in] tp        pointer to the thread
 * @return              A pointer to the found thread.
 * @retval NULL         if a matching thread has not been found.
 *
 * @api
 */
thread_t *chRegFindThreadByPointer(thread_t *tp) {
  thread_t *ctp;

  /* Scanning registry.*/
  ctp = chRegFirstThread();
  do {
    if (ctp == tp) {
      return ctp;
    }
    ctp = chRegNextThread(ctp);
  } while (ctp != NULL);

  return NULL;
}

#if (CH_DBG_ENABLE_STACK_CHECK == TRUE) || (CH_CFG_USE_DYNAMIC == TRUE) ||  \
    defined(__DOXYGEN__)
/**
 * @brief   Confirms that a working area is being used by some active thread.
 * @note    The reference counter of the found thread is increased by one so
 *          it cannot be disposed incidentally after the pointer has been
 *          returned.
 *
 * @param[in] wa        pointer to a static working area
 * @return              A pointer to the found thread.
 * @retval NULL         if a matching thread has not been found.
 *
 * @api
 */
thread_t *chRegFindThreadByWorkingArea(stkalign_t *wa) {
  thread_t *ctp;

  /* Scanning registry.*/
  ctp = chRegFirstThread();
  do {
    if (chThdGetWorkingAreaX(ctp) == wa) {
      return ctp;
    }
    ctp = chRegNextThread(ctp);
  } while (ctp != NULL);

  return NULL;
}
#endif

#endif /* CH_CFG_USE_REGISTRY == TRUE */

/** @} */
