/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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

/**
 * @file    DMAv2/stm32_dma.c
 * @brief   Enhanced DMA helper driver code.
 *
 * @addtogroup STM32_DMA
 * @details DMA sharing helper driver. In the STM32 the DMA streams are a
 *          shared resource, this driver allows to allocate and free DMA
 *          streams at runtime in order to allow all the other device
 *          drivers to coordinate the access to the resource.
 * @note    The DMA ISR handlers are all declared into this module because
 *          sharing, the various device drivers can associate a callback to
 *          ISRs when allocating streams.
 * @{
 */

#include "hal.h"

/* The following macro is only defined if some driver requiring DMA services
   has been enabled.*/
#if defined(STM32_DMA_REQUIRED) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/**
 * @brief   Mask of the DMA1 streams in @p dma_streams_mask.
 */
#define STM32_DMA1_STREAMS_MASK     0x000000FFU

/**
 * @brief   Mask of the DMA2 streams in @p dma_streams_mask.
 */
#define STM32_DMA2_STREAMS_MASK     0x0000FF00U

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   DMA streams descriptors.
 * @details This table keeps the association between an unique stream
 *          identifier and the involved physical registers.
 * @note    Don't use this array directly, use the appropriate wrapper macros
 *          instead: @p STM32_DMA1_STREAM0, @p STM32_DMA1_STREAM1 etc.
 */
const stm32_dma_stream_t _stm32_dma_streams[STM32_DMA_STREAMS] = {
  {DMA1_Stream0, &DMA1->LIFCR, 0, 0, STM32_DMA1_CH0_NUMBER},
  {DMA1_Stream1, &DMA1->LIFCR, 6, 1, STM32_DMA1_CH1_NUMBER},
  {DMA1_Stream2, &DMA1->LIFCR, 16, 2, STM32_DMA1_CH2_NUMBER},
  {DMA1_Stream3, &DMA1->LIFCR, 22, 3, STM32_DMA1_CH3_NUMBER},
  {DMA1_Stream4, &DMA1->HIFCR, 0, 4, STM32_DMA1_CH4_NUMBER},
  {DMA1_Stream5, &DMA1->HIFCR, 6, 5, STM32_DMA1_CH5_NUMBER},
  {DMA1_Stream6, &DMA1->HIFCR, 16, 6, STM32_DMA1_CH6_NUMBER},
  {DMA1_Stream7, &DMA1->HIFCR, 22, 7, STM32_DMA1_CH7_NUMBER},
  {DMA2_Stream0, &DMA2->LIFCR, 0, 8, STM32_DMA2_CH0_NUMBER},
  {DMA2_Stream1, &DMA2->LIFCR, 6, 9, STM32_DMA2_CH1_NUMBER},
  {DMA2_Stream2, &DMA2->LIFCR, 16, 10, STM32_DMA2_CH2_NUMBER},
  {DMA2_Stream3, &DMA2->LIFCR, 22, 11, STM32_DMA2_CH3_NUMBER},
  {DMA2_Stream4, &DMA2->HIFCR, 0, 12, STM32_DMA2_CH4_NUMBER},
  {DMA2_Stream5, &DMA2->HIFCR, 6, 13, STM32_DMA2_CH5_NUMBER},
  {DMA2_Stream6, &DMA2->HIFCR, 16, 14, STM32_DMA2_CH6_NUMBER},
  {DMA2_Stream7, &DMA2->HIFCR, 22, 15, STM32_DMA2_CH7_NUMBER},
};

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   DMA ISR redirector type.
 */
typedef struct {
  stm32_dmaisr_t        dma_func;       /**< @brief DMA callback function.  */
  void                  *dma_param;     /**< @brief DMA callback parameter. */
} dma_isr_redir_t;

/**
 * @brief   Mask of the allocated streams.
 */
static uint32_t dma_streams_mask;

/**
 * @brief   DMA IRQ redirectors.
 */
static dma_isr_redir_t dma_isr_redir[STM32_DMA_STREAMS];

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/**
 * @brief   DMA1 stream 0 shared interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA1_CH0_HANDLER) {
  uint32_t flags;

  OSAL_IRQ_PROLOGUE();

  flags = (DMA1->LISR >> 0U) & STM32_DMA_ISR_MASK;
  DMA1->LIFCR = flags << 0U;
  if (dma_isr_redir[0].dma_func)
    dma_isr_redir[0].dma_func(dma_isr_redir[0].dma_param, flags);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   DMA1 stream 1 shared interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA1_CH1_HANDLER) {
  uint32_t flags;

  OSAL_IRQ_PROLOGUE();

  flags = (DMA1->LISR >> 6U) & STM32_DMA_ISR_MASK;
  DMA1->LIFCR = flags << 6U;
  if (dma_isr_redir[1].dma_func)
    dma_isr_redir[1].dma_func(dma_isr_redir[1].dma_param, flags);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   DMA1 stream 2 shared interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA1_CH2_HANDLER) {
  uint32_t flags;

  OSAL_IRQ_PROLOGUE();

  flags = (DMA1->LISR >> 16U) & STM32_DMA_ISR_MASK;
  DMA1->LIFCR = flags << 16U;
  if (dma_isr_redir[2].dma_func)
    dma_isr_redir[2].dma_func(dma_isr_redir[2].dma_param, flags);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   DMA1 stream 3 shared interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA1_CH3_HANDLER) {
  uint32_t flags;

  OSAL_IRQ_PROLOGUE();

  flags = (DMA1->LISR >> 22U) & STM32_DMA_ISR_MASK;
  DMA1->LIFCR = flags << 22U;
  if (dma_isr_redir[3].dma_func)
    dma_isr_redir[3].dma_func(dma_isr_redir[3].dma_param, flags);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   DMA1 stream 4 shared interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA1_CH4_HANDLER) {
  uint32_t flags;

  OSAL_IRQ_PROLOGUE();

  flags = (DMA1->HISR >> 0U) & STM32_DMA_ISR_MASK;
  DMA1->HIFCR = flags << 0U;
  if (dma_isr_redir[4].dma_func)
    dma_isr_redir[4].dma_func(dma_isr_redir[4].dma_param, flags);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   DMA1 stream 5 shared interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA1_CH5_HANDLER) {
  uint32_t flags;

  OSAL_IRQ_PROLOGUE();

  flags = (DMA1->HISR >> 6U) & STM32_DMA_ISR_MASK;
  DMA1->HIFCR = flags << 6U;
  if (dma_isr_redir[5].dma_func)
    dma_isr_redir[5].dma_func(dma_isr_redir[5].dma_param, flags);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   DMA1 stream 6 shared interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA1_CH6_HANDLER) {
  uint32_t flags;

  OSAL_IRQ_PROLOGUE();

  flags = (DMA1->HISR >> 16U) & STM32_DMA_ISR_MASK;
  DMA1->HIFCR = flags << 16U;
  if (dma_isr_redir[6].dma_func)
    dma_isr_redir[6].dma_func(dma_isr_redir[6].dma_param, flags);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   DMA1 stream 7 shared interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA1_CH7_HANDLER) {
  uint32_t flags;

  OSAL_IRQ_PROLOGUE();

  flags = (DMA1->HISR >> 22U) & STM32_DMA_ISR_MASK;
  DMA1->HIFCR = flags << 22U;
  if (dma_isr_redir[7].dma_func)
    dma_isr_redir[7].dma_func(dma_isr_redir[7].dma_param, flags);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   DMA2 stream 0 shared interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA2_CH0_HANDLER) {
  uint32_t flags;

  OSAL_IRQ_PROLOGUE();

  flags = (DMA2->LISR >> 0U) & STM32_DMA_ISR_MASK;
  DMA2->LIFCR = flags << 0U;
  if (dma_isr_redir[8].dma_func)
    dma_isr_redir[8].dma_func(dma_isr_redir[8].dma_param, flags);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   DMA2 stream 1 shared interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA2_CH1_HANDLER) {
  uint32_t flags;

  OSAL_IRQ_PROLOGUE();

  flags = (DMA2->LISR >> 6U) & STM32_DMA_ISR_MASK;
  DMA2->LIFCR = flags << 6U;
  if (dma_isr_redir[9].dma_func)
    dma_isr_redir[9].dma_func(dma_isr_redir[9].dma_param, flags);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   DMA2 stream 2 shared interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA2_CH2_HANDLER) {
  uint32_t flags;

  OSAL_IRQ_PROLOGUE();

  flags = (DMA2->LISR >> 16U) & STM32_DMA_ISR_MASK;
  DMA2->LIFCR = flags << 16U;
  if (dma_isr_redir[10].dma_func)
    dma_isr_redir[10].dma_func(dma_isr_redir[10].dma_param, flags);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   DMA2 stream 3 shared interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA2_CH3_HANDLER) {
  uint32_t flags;

  OSAL_IRQ_PROLOGUE();

  flags = (DMA2->LISR >> 22U) & STM32_DMA_ISR_MASK;
  DMA2->LIFCR = flags << 22U;
  if (dma_isr_redir[11].dma_func)
    dma_isr_redir[11].dma_func(dma_isr_redir[11].dma_param, flags);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   DMA2 stream 4 shared interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA2_CH4_HANDLER) {
  uint32_t flags;

  OSAL_IRQ_PROLOGUE();

  flags = (DMA2->HISR >> 0U) & STM32_DMA_ISR_MASK;
  DMA2->HIFCR = flags << 0U;
  if (dma_isr_redir[12].dma_func)
    dma_isr_redir[12].dma_func(dma_isr_redir[12].dma_param, flags);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   DMA2 stream 5 shared interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA2_CH5_HANDLER) {
  uint32_t flags;

  OSAL_IRQ_PROLOGUE();

  flags = (DMA2->HISR >> 6U) & STM32_DMA_ISR_MASK;
  DMA2->HIFCR = flags << 6U;
  if (dma_isr_redir[13].dma_func)
    dma_isr_redir[13].dma_func(dma_isr_redir[13].dma_param, flags);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   DMA2 stream 6 shared interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA2_CH6_HANDLER) {
  uint32_t flags;

  OSAL_IRQ_PROLOGUE();

  flags = (DMA2->HISR >> 16U) & STM32_DMA_ISR_MASK;
  DMA2->HIFCR = flags << 16U;
  if (dma_isr_redir[14].dma_func)
    dma_isr_redir[14].dma_func(dma_isr_redir[14].dma_param, flags);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   DMA2 stream 7 shared interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA2_CH7_HANDLER) {
  uint32_t flags;

  OSAL_IRQ_PROLOGUE();

  flags = (DMA2->HISR >> 22U) & STM32_DMA_ISR_MASK;
  DMA2->HIFCR = flags << 22U;
  if (dma_isr_redir[15].dma_func)
    dma_isr_redir[15].dma_func(dma_isr_redir[15].dma_param, flags);

  OSAL_IRQ_EPILOGUE();
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   STM32 DMA helper initialization.
 *
 * @init
 */
void dmaInit(void) {
  unsigned i;

  dma_streams_mask = 0U;
  for (i = 0U; i < STM32_DMA_STREAMS; i++) {
    _stm32_dma_streams[i].stream->CR = 0U;
    dma_isr_redir[i].dma_func = NULL;
  }
  DMA1->LIFCR = 0xFFFFFFFFU;
  DMA1->HIFCR = 0xFFFFFFFFU;
  DMA2->LIFCR = 0xFFFFFFFFU;
  DMA2->HIFCR = 0xFFFFFFFFU;
}

/**
 * @brief   Allocates a DMA stream.
 * @details The stream is allocated and, if required, the DMA clock enabled.
 *          The function also enables the IRQ vector associated to the stream
 *          and initializes its priority.
 * @pre     The stream must not be already in use or an error is returned.
 * @post    The stream is allocated and the default ISR handler redirected
 *          to the specified function.
 * @post    The stream ISR vector is enabled and its priority configured.
 * @post    The stream must be freed using @p dmaStreamRelease() before it can
 *          be reused with another peripheral.
 * @post    The stream is in its post-reset state.
 * @note    This function can be invoked in both ISR or thread context.
 *
 * @param[in] dmastp    pointer to a stm32_dma_stream_t structure
 * @param[in] priority  IRQ priority mask for the DMA stream
 * @param[in] func      handling function pointer, can be @p NULL
 * @param[in] param     a parameter to be passed to the handling function
 * @return              The operation status.
 * @retval false        no error, stream taken.
 * @retval true         error, stream already taken.
 *
 * @special
 */
bool dmaStreamAllocate(const stm32_dma_stream_t *dmastp,
                       uint32_t priority,
                       stm32_dmaisr_t func,
                       void *param) {

  osalDbgCheck(dmastp != NULL);

  /* Checks if the stream is already taken.*/
  if ((dma_streams_mask & (1U << dmastp->selfindex)) != 0U)
    return true;

  /* Marks the stream as allocated.*/
  dma_isr_redir[dmastp->selfindex].dma_func  = func;
  dma_isr_redir[dmastp->selfindex].dma_param = param;
  dma_streams_mask |= (1U << dmastp->selfindex);

  /* Enabling DMA clocks required by the current streams set.*/
  if ((dma_streams_mask & STM32_DMA1_STREAMS_MASK) != 0U) {
    rccEnableDMA1(false);
  }
  if ((dma_streams_mask & STM32_DMA2_STREAMS_MASK) != 0U) {
    rccEnableDMA2(false);
  }

  /* Putting the stream in a safe state.*/
  dmaStreamDisable(dmastp);
  dmastp->stream->CR = STM32_DMA_CR_RESET_VALUE;
  dmastp->stream->FCR = STM32_DMA_FCR_RESET_VALUE;

  /* Enables the associated IRQ vector if a callback is defined.*/
  if (func != NULL) {
    nvicEnableVector(dmastp->vector, priority);
  }

  return false;
}

/**
 * @brief   Releases a DMA stream.
 * @details The stream is freed and, if required, the DMA clock disabled.
 *          Trying to release a unallocated stream is an illegal operation
 *          and is trapped if assertions are enabled.
 * @pre     The stream must have been allocated using @p dmaStreamAllocate().
 * @post    The stream is again available.
 * @note    This function can be invoked in both ISR or thread context.
 *
 * @param[in] dmastp    pointer to a stm32_dma_stream_t structure
 *
 * @special
 */
void dmaStreamRelease(const stm32_dma_stream_t *dmastp) {

  osalDbgCheck(dmastp != NULL);

  /* Check if the streams is not taken.*/
  osalDbgAssert((dma_streams_mask & (1U << dmastp->selfindex)) != 0U,
                "not allocated");

  /* Disables the associated IRQ vector.*/
  nvicDisableVector(dmastp->vector);

  /* Marks the stream as not allocated.*/
  dma_streams_mask &= ~(1U << dmastp->selfindex);

  /* Shutting down clocks that are no more required, if any.*/
  if ((dma_streams_mask & STM32_DMA1_STREAMS_MASK) == 0U) {
    rccDisableDMA1(false);
  }
  if ((dma_streams_mask & STM32_DMA2_STREAMS_MASK) == 0U) {
    rccDisableDMA2(false);
  }
}

#endif /* STM32_DMA_REQUIRED */

/** @} */
