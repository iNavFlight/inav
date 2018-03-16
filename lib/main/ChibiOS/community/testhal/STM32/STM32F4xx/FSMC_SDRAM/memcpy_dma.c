/*
    ChibiOS/RT - Copyright (C) 2013-2014 Uladzimir Pylinsky aka barthess

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <string.h>

#include "ch.h"
#include "hal.h"

#include "memcpy_dma.h"

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define STM32_MEMCPY_DMA_PRIORITY       0
#define STM32_MEMCPY_DMA_STREAM         STM32_DMA_STREAM_ID(2, 6)

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */

/*
 ******************************************************************************
 * PROTOTYPES
 ******************************************************************************
 */

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
static memcpy_dma_engine_t engine;

/*
 ******************************************************************************
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 */

/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */
/*
 *
 */
void memcpy_dma_start(void) {
  bool b;

  engine.dma = STM32_DMA_STREAM(STM32_MEMCPY_DMA_STREAM);
  b = dmaStreamAllocate(engine.dma, STM32_MEMCPY_DMA_PRIORITY, NULL, NULL);
  osalDbgAssert(!b, "stream already allocated");
}

/*
 *
 */
void memcpy_dma_stop(void) {
  dmaStreamRelease(engine.dma);
}

/*
 *
 */
void memcpy_dma(void *dest, const void *src, size_t size) {

  size_t words = size / 4;
  size_t remainder = size % 4;
  size_t max_block = 0xFFFF; /* DMA limitation */

  uint32_t cr = STM32_DMA_CR_PSIZE_WORD | STM32_DMA_CR_MSIZE_WORD;

  while (words > max_block) {
    dmaStartMemCopy(engine.dma, cr, src, dest, max_block)
    dmaWaitCompletion(engine.dma);
    words -= max_block;
  }

  dmaStartMemCopy(engine.dma, cr, src, dest, words)
  dmaWaitCompletion(engine.dma);

  if (remainder > 0)
    memcpy(dest+size-remainder, src+size-remainder, remainder);
}

