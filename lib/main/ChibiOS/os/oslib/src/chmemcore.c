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
 * @file    chmemcore.c
 * @brief   Core memory manager code.
 *
 * @addtogroup oslib_memcore
 * @details Core Memory Manager related APIs and services.
 *          <h2>Operation mode</h2>
 *          The core memory manager is a simplified allocator that only
 *          allows to allocate memory blocks without the possibility to
 *          free them.<br>
 *          This allocator is meant as a memory blocks provider for the
 *          other allocators such as:
 *          - C-Runtime allocator (through a compiler specific adapter module).
 *          - Heap allocator (see @ref oslib_memheaps).
 *          - Memory pools allocator (see @ref oslib_mempools).
 *          .
 *          By having a centralized memory provider the various allocators
 *          can coexist and share the main memory.<br>
 *          This allocator, alone, is also useful for very simple
 *          applications that just require a simple way to get memory
 *          blocks.
 * @pre     In order to use the core memory manager APIs the @p CH_CFG_USE_MEMCORE
 *          option must be enabled in @p chconf.h.
 * @note    Compatible with RT and NIL.
 * @{
 */

#include "ch.h"

#if (CH_CFG_USE_MEMCORE == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   Memory core descriptor.
 */
memcore_t ch_memcore;

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
 * @brief   Low level memory manager initialization.
 *
 * @notapi
 */
void _core_init(void) {
#if CH_CFG_MEMCORE_SIZE == 0
  extern uint8_t __heap_base__[];
  extern uint8_t __heap_end__[];

  /*lint -save -e9033 [10.8] Required cast operations.*/
  ch_memcore.nextmem = __heap_base__;
  ch_memcore.endmem  = __heap_end__;
  /*lint restore*/
#else
  static uint8_t static_heap[CH_CFG_MEMCORE_SIZE];

  ch_memcore.nextmem = &static_heap[0];
  ch_memcore.endmem  = &static_heap[CH_CFG_MEMCORE_SIZE];
#endif
}

/**
 * @brief   Allocates a memory block.
 * @details This function allocates a block of @p offset + @p size bytes. The
 *          returned pointer has @p offset bytes before its address and
 *          @p size bytes after.
 *
 * @param[in] size      the size of the block to be allocated.
 * @param[in] align     desired memory alignment
 * @param[in] offset    aligned pointer offset
 * @return              A pointer to the allocated memory block.
 * @retval NULL         allocation failed, core memory exhausted.
 *
 * @iclass
 */
void *chCoreAllocAlignedWithOffsetI(size_t size,
                                    unsigned align,
                                    size_t offset) {
  uint8_t *p, *next;

  chDbgCheckClassI();
  chDbgCheck(MEM_IS_VALID_ALIGNMENT(align));

  size = MEM_ALIGN_NEXT(size, align);
  p = (uint8_t *)MEM_ALIGN_NEXT(ch_memcore.nextmem + offset, align);
  next = p + size;

  /* Considering also the case where there is numeric overflow.*/
  if ((next > ch_memcore.endmem) || (next < ch_memcore.nextmem)) {
    return NULL;
  }

  ch_memcore.nextmem = next;

  return p;
}

/**
 * @brief   Allocates a memory block.
 * @details This function allocates a block of @p offset + @p size bytes. The
 *          returned pointer has @p offset bytes before its address and
 *          @p size bytes after.
 *
 * @param[in] size      the size of the block to be allocated.
 * @param[in] align     desired memory alignment
 * @param[in] offset    aligned pointer offset
 * @return              A pointer to the allocated memory block.
 * @retval NULL         allocation failed, core memory exhausted.
 *
 * @api
 */
void *chCoreAllocAlignedWithOffset(size_t size,
                                   unsigned align,
                                   size_t offset) {
  void *p;

  chSysLock();
  p = chCoreAllocAlignedWithOffsetI(size, align, offset);
  chSysUnlock();

  return p;
}

/**
 * @brief   Core memory status.
 *
 * @return              The size, in bytes, of the free core memory.
 *
 * @xclass
 */
size_t chCoreGetStatusX(void) {

  /*lint -save -e9033 [10.8] The cast is safe.*/
  return (size_t)(ch_memcore.endmem - ch_memcore.nextmem);
  /*lint -restore*/
}
#endif /* CH_CFG_USE_MEMCORE == TRUE */

/** @} */
