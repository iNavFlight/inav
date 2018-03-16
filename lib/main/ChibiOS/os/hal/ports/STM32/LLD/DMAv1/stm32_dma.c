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
 * @file    STM32F3xx/stm32_dma.c
 * @brief   DMA helper driver code.
 *
 * @addtogroup STM32F3xx_DMA
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
#define STM32_DMA1_STREAMS_MASK     ((1U << STM32_DMA1_NUM_CHANNELS) - 1U)

/**
 * @brief   Mask of the DMA2 streams in @p dma_streams_mask.
 */
#define STM32_DMA2_STREAMS_MASK     (((1U << STM32_DMA2_NUM_CHANNELS) -     \
                                     1U) << STM32_DMA1_NUM_CHANNELS)

/**
 * @brief   Post-reset value of the stream CCR register.
 */
#define STM32_DMA_CCR_RESET_VALUE   0x00000000U

#if STM32_DMA_SUPPORTS_CSELR == TRUE

#if defined(DMA1_CSELR)
#define ADDR_DMA1_CSELR             &DMA1_CSELR->CSELR
#else
#define ADDR_DMA1_CSELR             &DMA1->CSELR
#endif

#if defined(DMA2_CSELR)
#define ADDR_DMA2_CSELR             &DMA2_CSELR->CSELR
#else
#define ADDR_DMA2_CSELR             &DMA2->CSELR
#endif

#else /* !defined(DMA1_CSELR) */

#define ADDR_DMA1_CSELR             NULL
#define ADDR_DMA2_CSELR             NULL

#endif /* !defined(DMA1_CSELR) */

/*
 * Default ISR collision masks.
 */
#if !defined(DMA1_CH1_CMASK)
#define DMA1_CH1_CMASK              0x00000001U
#endif

#if !defined(DMA1_CH2_CMASK)
#define DMA1_CH2_CMASK              0x00000002U
#endif

#if !defined(DMA1_CH3_CMASK)
#define DMA1_CH3_CMASK              0x00000004U
#endif

#if !defined(DMA1_CH4_CMASK)
#define DMA1_CH4_CMASK              0x00000008U
#endif

#if !defined(DMA1_CH5_CMASK)
#define DMA1_CH5_CMASK              0x00000010U
#endif

#if !defined(DMA1_CH6_CMASK)
#define DMA1_CH6_CMASK              0x00000020U
#endif

#if !defined(DMA1_CH7_CMASK)
#define DMA1_CH7_CMASK              0x00000040U
#endif

#if !defined(DMA2_CH1_CMASK)
#define DMA2_CH1_CMASK              0x00000080U
#endif

#if !defined(DMA2_CH2_CMASK)
#define DMA2_CH2_CMASK              0x00000100U
#endif

#if !defined(DMA2_CH3_CMASK)
#define DMA2_CH3_CMASK              0x00000200U
#endif

#if !defined(DMA2_CH4_CMASK)
#define DMA2_CH4_CMASK              0x00000400U
#endif

#if !defined(DMA2_CH5_CMASK)
#define DMA2_CH5_CMASK              0x00000800U
#endif

#if !defined(DMA2_CH6_CMASK)
#define DMA2_CH6_CMASK              0x00001000U
#endif

#if !defined(DMA2_CH7_CMASK)
#define DMA2_CH7_CMASK              0x00002000U
#endif

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   DMA streams descriptors.
 * @details This table keeps the association between an unique stream
 *          identifier and the involved physical registers.
 * @note    Don't use this array directly, use the appropriate wrapper macros
 *          instead: @p STM32_DMA1_STREAM1, @p STM32_DMA1_STREAM2 etc.
 */
const stm32_dma_stream_t _stm32_dma_streams[STM32_DMA_STREAMS] = {
  {DMA1, DMA1_Channel1, DMA1_CH1_CMASK, ADDR_DMA1_CSELR,  0,  0, STM32_DMA1_CH1_NUMBER},
  {DMA1, DMA1_Channel2, DMA1_CH2_CMASK, ADDR_DMA1_CSELR,  4,  1, STM32_DMA1_CH2_NUMBER},
  {DMA1, DMA1_Channel3, DMA1_CH3_CMASK, ADDR_DMA1_CSELR,  8,  2, STM32_DMA1_CH3_NUMBER},
  {DMA1, DMA1_Channel4, DMA1_CH4_CMASK, ADDR_DMA1_CSELR, 12,  3, STM32_DMA1_CH4_NUMBER},
  {DMA1, DMA1_Channel5, DMA1_CH5_CMASK, ADDR_DMA1_CSELR, 16,  4, STM32_DMA1_CH5_NUMBER},
#if STM32_DMA1_NUM_CHANNELS > 5
  {DMA1, DMA1_Channel6, DMA1_CH6_CMASK, ADDR_DMA1_CSELR, 20,  5, STM32_DMA1_CH6_NUMBER},
#if STM32_DMA1_NUM_CHANNELS > 6
  {DMA1, DMA1_Channel7, DMA1_CH7_CMASK, ADDR_DMA1_CSELR, 24,  6, STM32_DMA1_CH7_NUMBER},
#if STM32_DMA2_NUM_CHANNELS > 0
  {DMA2, DMA2_Channel1, DMA2_CH1_CMASK, ADDR_DMA2_CSELR,  0,  7, STM32_DMA2_CH1_NUMBER},
  {DMA2, DMA2_Channel2, DMA2_CH2_CMASK, ADDR_DMA2_CSELR,  4,  8, STM32_DMA2_CH2_NUMBER},
  {DMA2, DMA2_Channel3, DMA2_CH3_CMASK, ADDR_DMA2_CSELR,  8,  9, STM32_DMA2_CH3_NUMBER},
  {DMA2, DMA2_Channel4, DMA2_CH4_CMASK, ADDR_DMA2_CSELR, 12, 10, STM32_DMA2_CH4_NUMBER},
  {DMA2, DMA2_Channel5, DMA2_CH5_CMASK, ADDR_DMA2_CSELR, 16, 11, STM32_DMA2_CH5_NUMBER},
#if STM32_DMA2_NUM_CHANNELS > 5
  {DMA2, DMA2_Channel6, DMA2_CH6_CMASK, ADDR_DMA2_CSELR, 20, 12, STM32_DMA2_CH6_NUMBER},
#if STM32_DMA2_NUM_CHANNELS > 6
  {DMA2, DMA2_Channel7, DMA2_CH7_CMASK, ADDR_DMA2_CSELR, 24, 13, STM32_DMA2_CH7_NUMBER},
#endif
#endif
#endif
#endif
#endif
};

/**
 * @brief   DMA IRQ redirectors.
 */
dma_isr_redir_t _stm32_dma_isr_redir[STM32_DMA_STREAMS];

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   Mask of the allocated streams.
 */
static uint32_t dma_streams_mask;

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if defined(STM32_DMA1_CH1_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA1 stream 1 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA1_CH1_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  dmaServeInterrupt(STM32_DMA1_STREAM1);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if defined(STM32_DMA1_CH2_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA1 stream 2 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA1_CH2_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  dmaServeInterrupt(STM32_DMA1_STREAM2);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if defined(STM32_DMA1_CH3_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA1 stream 3 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA1_CH3_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  dmaServeInterrupt(STM32_DMA1_STREAM3);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if defined(STM32_DMA1_CH4_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA1 stream 4 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA1_CH4_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  dmaServeInterrupt(STM32_DMA1_STREAM4);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if defined(STM32_DMA1_CH5_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA1 stream 5 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA1_CH5_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  dmaServeInterrupt(STM32_DMA1_STREAM5);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if defined(STM32_DMA1_CH6_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA1 stream 6 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA1_CH6_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  dmaServeInterrupt(STM32_DMA1_STREAM6);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if defined(STM32_DMA1_CH7_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA1 stream 7 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA1_CH7_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  dmaServeInterrupt(STM32_DMA1_STREAM7);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if defined(STM32_DMA2_CH1_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA2 stream 1 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA2_CH1_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  dmaServeInterrupt(STM32_DMA2_STREAM1);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if defined(STM32_DMA2_CH2_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA2 stream 2 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA2_CH2_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  dmaServeInterrupt(STM32_DMA2_STREAM2);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if defined(STM32_DMA2_CH3_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA2 stream 3 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA2_CH3_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  dmaServeInterrupt(STM32_DMA2_STREAM3);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if defined(STM32_DMA2_CH4_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA2 stream 4 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA2_CH4_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  dmaServeInterrupt(STM32_DMA2_STREAM4);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if defined(STM32_DMA2_CH5_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA2 stream 5 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA2_CH5_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  dmaServeInterrupt(STM32_DMA2_STREAM5);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if defined(STM32_DMA2_CH6_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA2 stream 6 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA2_CH6_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  dmaServeInterrupt(STM32_DMA2_STREAM6);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if defined(STM32_DMA2_CH7_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA2 stream 7 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA2_CH7_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  dmaServeInterrupt(STM32_DMA2_STREAM7);

  OSAL_IRQ_EPILOGUE();
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   STM32 DMA helper initialization.
 *
 * @init
 */
void dmaInit(void) {
  int i;

  dma_streams_mask = 0U;
  for (i = 0; i < STM32_DMA_STREAMS; i++) {
    _stm32_dma_streams[i].channel->CCR = 0U;
    _stm32_dma_isr_redir[i].dma_func = NULL;
  }
  DMA1->IFCR = 0xFFFFFFFFU;
#if STM32_DMA2_NUM_CHANNELS > 0
  DMA2->IFCR = 0xFFFFFFFFU;
#endif
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

  /* Installs the DMA handler.*/
  _stm32_dma_isr_redir[dmastp->selfindex].dma_func  = func;
  _stm32_dma_isr_redir[dmastp->selfindex].dma_param = param;

  /* Enabling DMA clocks required by the current streams set.*/
  if ((dma_streams_mask & STM32_DMA1_STREAMS_MASK) == 0U) {
    rccEnableDMA1(false);
  }
#if STM32_DMA2_NUM_CHANNELS > 0
  if ((dma_streams_mask & STM32_DMA2_STREAMS_MASK) == 0U) {
    rccEnableDMA2(false);
  }
#endif

  /* Putting the stream in a safe state.*/
  dmaStreamDisable(dmastp);
  dmastp->channel->CCR = STM32_DMA_CCR_RESET_VALUE;

  /* Enables the associated IRQ vector if not alread enabled and if a
     callback is defined.*/
  if (((dma_streams_mask & dmastp->cmask) == 0U) &&
      (func != NULL)) {
    nvicEnableVector(dmastp->vector, priority);
  }

  /* Marks the stream as allocated.*/
  dma_streams_mask |= (1U << dmastp->selfindex);

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
  osalDbgAssert((dma_streams_mask & (1 << dmastp->selfindex)) != 0U,
                "not allocated");

  /* Marks the stream as not allocated.*/
  dma_streams_mask &= ~(1U << dmastp->selfindex);

  /* Disables the associated IRQ vector if it is no more in use.*/
  if ((dma_streams_mask & dmastp->cmask) == 0U) {
    nvicDisableVector(dmastp->vector);
  }

  /* Removes the DMA handler.*/
  _stm32_dma_isr_redir[dmastp->selfindex].dma_func  = NULL;
  _stm32_dma_isr_redir[dmastp->selfindex].dma_param = NULL;

  /* Shutting down clocks that are no more required, if any.*/
  if ((dma_streams_mask & STM32_DMA1_STREAMS_MASK) == 0U) {
    rccDisableDMA1(false);
  }
#if STM32_DMA2_NUM_CHANNELS > 0
  if ((dma_streams_mask & STM32_DMA2_STREAMS_MASK) == 0U) {
    rccDisableDMA2(false);
  }
#endif
}

#endif /* STM32_DMA_REQUIRED */

/** @} */
