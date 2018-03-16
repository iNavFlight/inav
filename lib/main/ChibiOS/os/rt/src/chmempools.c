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
 * @file    chmempools.c
 * @brief   Memory Pools code.
 *
 * @addtogroup pools
 * @details Memory Pools related APIs and services.
 *          <h2>Operation mode</h2>
 *          The Memory Pools APIs allow to allocate/free fixed size objects in
 *          <b>constant time</b> and reliably without memory fragmentation
 *          problems.<br>
 *          Memory Pools do not enforce any alignment constraint on the
 *          contained object however the objects must be properly aligned
 *          to contain a pointer to void.
 * @pre     In order to use the memory pools APIs the @p CH_CFG_USE_MEMPOOLS option
 *          must be enabled in @p chconf.h.
 * @{
 */

#include "ch.h"

#if (CH_CFG_USE_MEMPOOLS == TRUE) || defined(__DOXYGEN__)

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
 * @brief   Initializes an empty memory pool.
 *
 * @param[out] mp       pointer to a @p memory_pool_t structure
 * @param[in] size      the size of the objects contained in this memory pool,
 *                      the minimum accepted size is the size of a pointer to
 *                      void.
 * @param[in] provider  memory provider function for the memory pool or
 *                      @p NULL if the pool is not allowed to grow
 *                      automatically
 *
 * @init
 */
void chPoolObjectInit(memory_pool_t *mp, size_t size, memgetfunc_t provider) {

  chDbgCheck((mp != NULL) && (size >= sizeof(void *)));

  mp->mp_next = NULL;
  mp->mp_object_size = size;
  mp->mp_provider = provider;
}

/**
 * @brief   Loads a memory pool with an array of static objects.
 * @pre     The memory pool must be already been initialized.
 * @pre     The array elements must be of the right size for the specified
 *          memory pool.
 * @post    The memory pool contains the elements of the input array.
 *
 * @param[in] mp        pointer to a @p memory_pool_t structure
 * @param[in] p         pointer to the array first element
 * @param[in] n         number of elements in the array
 *
 * @api
 */
void chPoolLoadArray(memory_pool_t *mp, void *p, size_t n) {

  chDbgCheck((mp != NULL) && (n != 0U));

  while (n != 0U) {
    chPoolAdd(mp, p);
    /*lint -save -e9087 [11.3] Safe cast.*/
    p = (void *)(((uint8_t *)p) + mp->mp_object_size);
    /*lint -restore*/
    n--;
  }
}

/**
 * @brief   Allocates an object from a memory pool.
 * @pre     The memory pool must be already been initialized.
 *
 * @param[in] mp        pointer to a @p memory_pool_t structure
 * @return              The pointer to the allocated object.
 * @retval NULL         if pool is empty.
 *
 * @iclass
 */
void *chPoolAllocI(memory_pool_t *mp) {
  void *objp;

  chDbgCheckClassI();
  chDbgCheck(mp != NULL);

  objp = mp->mp_next;
  /*lint -save -e9013 [15.7] There is no else because it is not needed.*/
  if (objp != NULL) {
    mp->mp_next = mp->mp_next->ph_next;
  }
  else if (mp->mp_provider != NULL) {
    objp = mp->mp_provider(mp->mp_object_size);
  }
  /*lint -restore*/

  return objp;
}

/**
 * @brief   Allocates an object from a memory pool.
 * @pre     The memory pool must be already been initialized.
 *
 * @param[in] mp        pointer to a @p memory_pool_t structure
 * @return              The pointer to the allocated object.
 * @retval NULL         if pool is empty.
 *
 * @api
 */
void *chPoolAlloc(memory_pool_t *mp) {
  void *objp;

  chSysLock();
  objp = chPoolAllocI(mp);
  chSysUnlock();

  return objp;
}

/**
 * @brief   Releases an object into a memory pool.
 * @pre     The memory pool must be already been initialized.
 * @pre     The freed object must be of the right size for the specified
 *          memory pool.
 * @pre     The object must be properly aligned to contain a pointer to void.
 *
 * @param[in] mp        pointer to a @p memory_pool_t structure
 * @param[in] objp      the pointer to the object to be released
 *
 * @iclass
 */
void chPoolFreeI(memory_pool_t *mp, void *objp) {
  struct pool_header *php = objp;

  chDbgCheckClassI();
  chDbgCheck((mp != NULL) && (objp != NULL));

  php->ph_next = mp->mp_next;
  mp->mp_next = php;
}

/**
 * @brief   Releases an object into a memory pool.
 * @pre     The memory pool must be already been initialized.
 * @pre     The freed object must be of the right size for the specified
 *          memory pool.
 * @pre     The object must be properly aligned to contain a pointer to void.
 *
 * @param[in] mp        pointer to a @p memory_pool_t structure
 * @param[in] objp      the pointer to the object to be released
 *
 * @api
 */
void chPoolFree(memory_pool_t *mp, void *objp) {

  chSysLock();
  chPoolFreeI(mp, objp);
  chSysUnlock();
}

#endif /* CH_CFG_USE_MEMPOOLS == TRUE */

/** @} */
