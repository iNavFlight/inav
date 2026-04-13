/**
  ******************************************************************************
  * @file    stm32h7b3i_eval_sram.c
  * @author  MCD Application Team
  * @brief   This file includes the SRAM driver for the IS61WV102416BLL-10M memory
  *          device mounted on STM32H7B3I_EVAL boards.
  @verbatim
  How To use this driver:
  -----------------------
   - This driver is used to drive the IS61WV102416BLL-10M SRAM external memory mounted
     on STM32H7B3I_EVAL board.
   - This driver does not need a specific component driver for the SRAM device
     to be included with.

  Driver description:
  ------------------
  + Initialization steps:
     o Initialize the SRAM external memory using the BSP_SRAM_Init() function. This
       function includes the MSP layer hardware resources initialization and the
       FMC controller configuration to interface with the external SRAM memory.

  + SRAM read/write operations
     o SRAM external memory can be accessed with read/write operations once it is
       initialized.
       Read/write operation can be performed with AHB access using the functions
       BSP_SRAM_ReadData()/BSP_SRAM_WriteData(), or by DMA transfer using the functions
       BSP_SRAM_ReadData_DMA()/BSP_SRAM_WriteData_DMA().
     o The AHB access is performed with 16-bit width transaction, the DMA transfer
       configuration is fixed at single (no burst) halfword transfer
       (see the SRAM_MspInit() static function).
     o User can implement his own functions for read/write access with his desired
       configurations.
     o If interrupt mode is used for DMA transfer, the function BSP_SRAM_DMA_IRQHandler()
       is called in IRQ handler file, to serve the generated interrupt once the DMA
       transfer is complete.
  @endverbatim
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32h7b3i_eval_sram.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H7B3I_EVAL
  * @{
  */

/** @addtogroup STM32H7B3I_EVAL_SRAM
  * @{
  */

/** @defgroup STM32H7B3I_EVAL_SRAM_Exported_Variables SRAM Exported Variables
  * @{
  */
SRAM_HandleTypeDef hsram[SRAM_INSTANCES_NBR];
/**
  * @}
  */

/** @defgroup STM32H7B3I_EVAL_SRAM_Private_Variables SRAM Private Variables
  * @{
  */
#if (USE_HAL_SRAM_REGISTER_CALLBACKS == 1)
static uint32_t IsMspCallbacksValid = 0;
#endif
/**
  * @}
  */

/** @defgroup STM32H7B3I_EVAL_SRAM_Private_Functions_Prototypes SRAM Private Functions Prototypes
  * @{
  */
static void SRAM_MspInit(SRAM_HandleTypeDef  *hSram);
static void SRAM_MspDeInit(SRAM_HandleTypeDef  *hSram);
/**
  * @}
  */

/** @defgroup STM32H7B3I_EVAL_SRAM_Exported_Functions SRAM Exported Functions
  * @{
  */

/**
  * @brief  Initializes the SRAM device.
  * @param  Instance SRAM instance
  * @retval BSP status
  */
int32_t BSP_SRAM_Init(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if (Instance >= SRAM_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_HAL_SRAM_REGISTER_CALLBACKS == 0)
    /* Msp SRAM initialization */
    SRAM_MspInit(&hsram[Instance]);
#else
    /* Register the SRAM MSP Callbacks */
    if (IsMspCallbacksValid == 0U)
    {
      if (BSP_SRAM_RegisterDefaultMspCallbacks(Instance) != BSP_ERROR_NONE)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
    }
    if (ret == BSP_ERROR_NONE)
    {
#endif /* USE_HAL_SRAM_REGISTER_CALLBACKS */
    /* __weak function can be rewritten by the application */
    if (MX_SRAM_BANK3_Init(&hsram[Instance]) != HAL_OK)
    {
      ret = BSP_ERROR_NO_INIT;
    }
#if (USE_HAL_SRAM_REGISTER_CALLBACKS == 1)
  }
#endif
}

return ret;
}

/**
  * @brief  DeInitializes the SRAM device.
  * @param  Instance SRAM instance
  * @retval BSP status
  */
int32_t BSP_SRAM_DeInit(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if (Instance >= SRAM_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* SRAM device de-initialization */
    hsram[Instance].Instance = FMC_NORSRAM_DEVICE;
    hsram[Instance].Extended = FMC_NORSRAM_EXTENDED_DEVICE;

    (void)HAL_SRAM_DeInit(&hsram[Instance]);
#if (USE_HAL_SRAM_REGISTER_CALLBACKS == 0)
    /* SRAM controller de-initialization */
    SRAM_MspDeInit(&hsram[Instance]);
#endif /* (USE_HAL_SRAM_REGISTER_CALLBACKS == 0) */
  }

  return ret;
}

/**
  * @brief  Initializes the SRAM peripheral.
  * @param  hSram SRAM handle
  * @retval HAL status
  */
__weak HAL_StatusTypeDef MX_SRAM_BANK3_Init(SRAM_HandleTypeDef *hSram)
{
  static FMC_NORSRAM_TimingTypeDef sram_timing;

  /* SRAM device configuration */
  hSram->Instance = FMC_NORSRAM_DEVICE;
  hSram->Extended = FMC_NORSRAM_EXTENDED_DEVICE;

  /* SRAM device configuration */
  hSram->Init.NSBank             = FMC_NORSRAM_BANK3;
  hSram->Init.DataAddressMux     = FMC_DATA_ADDRESS_MUX_DISABLE;
  hSram->Init.MemoryType         = FMC_MEMORY_TYPE_SRAM;
  hSram->Init.MemoryDataWidth    = FMC_NORSRAM_MEM_BUS_WIDTH_16;
  hSram->Init.BurstAccessMode    = FMC_BURST_ACCESS_MODE_DISABLE;
  hSram->Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
  hSram->Init.WaitSignalActive   = FMC_WAIT_TIMING_BEFORE_WS;
  hSram->Init.WriteOperation     = FMC_WRITE_OPERATION_ENABLE;
  hSram->Init.WaitSignal         = FMC_WAIT_SIGNAL_DISABLE;
  hSram->Init.ExtendedMode       = FMC_EXTENDED_MODE_DISABLE;
  hSram->Init.AsynchronousWait   = FMC_ASYNCHRONOUS_WAIT_DISABLE;
  hSram->Init.WriteBurst         = FMC_WRITE_BURST_DISABLE;
  hSram->Init.ContinuousClock    = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
  hSram->Init.WriteFifo          = FMC_WRITE_FIFO_DISABLE;
  hSram->Init.PageSize           = FMC_PAGE_SIZE_NONE;

  /* Timing configuration derived from system clock (up to 280)
  for 93,33Mhz as SRAM clock frequency */
  sram_timing.AddressSetupTime      = 2;
  sram_timing.AddressHoldTime       = 1;
  sram_timing.DataSetupTime         = 4;
  sram_timing.BusTurnAroundDuration = 1;
  sram_timing.CLKDivision           = 3;
  sram_timing.DataLatency           = 2;
  sram_timing.AccessMode            = FMC_ACCESS_MODE_A;

  /* SRAM controller initialization */
  if (HAL_SRAM_Init(hSram, &sram_timing, NULL) != HAL_OK)
  {
    return  HAL_ERROR;
  }
  return HAL_OK;
}

#if (USE_HAL_SRAM_REGISTER_CALLBACKS == 1)
/**
  * @brief Default BSP SRAM Msp Callbacks
  * @param Instance      SRAM Instance
  * @retval BSP status
  */
int32_t BSP_SRAM_RegisterDefaultMspCallbacks(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if (Instance >= SRAM_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Register MspInit/MspDeInit Callbacks */
    if (HAL_SRAM_RegisterCallback(&hsram[Instance], HAL_SRAM_MSP_INIT_CB_ID, SRAM_MspInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if (HAL_SRAM_RegisterCallback(&hsram[Instance], HAL_SRAM_MSP_DEINIT_CB_ID, SRAM_MspDeInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      IsMspCallbacksValid = 1U;
    }
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief BSP SRAM Msp Callback registering
  * @param Instance     SRAM Instance
  * @param CallBacks    pointer to MspInit/MspDeInit callbacks functions
  * @retval BSP status
  */
int32_t BSP_SRAM_RegisterMspCallbacks(uint32_t Instance, BSP_SRAM_Cb_t *CallBacks)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if (Instance >= SRAM_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Register MspInit/MspDeInit Callbacks */
    if (HAL_SRAM_RegisterCallback(&hsram[Instance], HAL_SRAM_MSP_INIT_CB_ID, CallBacks->pMspInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if (HAL_SRAM_RegisterCallback(&hsram[Instance], HAL_SRAM_MSP_DEINIT_CB_ID, CallBacks->pMspDeInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      IsMspCallbacksValid = 1U;
    }
  }
  /* Return BSP status */
  return ret;
}
#endif /* (USE_HAL_SRAM_REGISTER_CALLBACKS == 1) */

/**
  * @brief  This function handles SRAM MDMA interrupt request.
  * @param  Instance SDRAM instance
  * @retval None
  */
void BSP_SRAM_IRQHandler(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);

  HAL_MDMA_IRQHandler(hsram[Instance].hmdma);
}

/**
  * @}
  */

/** @defgroup STM32H7B3I_EVAL_SRAM_Private_Functions SRAM Private Functions
  * @{
  */
/**
  * @brief  Initializes SRAM MSP.
  * @param  hSram  SRAM handle
  * @retval None
  */
static void SRAM_MspInit(SRAM_HandleTypeDef  *hSram)
{
  static MDMA_HandleTypeDef mdma_handle;
  GPIO_InitTypeDef gpio_init_structure;

  /* Prevent unused argument(s) compilation warning */
  UNUSED(hSram);

  /* Enable FMC clock */
  __HAL_RCC_FMC_CLK_ENABLE();

  /* Enable chosen MDMAx clock */
  SRAM_MDMAx_CLK_ENABLE();

  /* Enable GPIOs clock */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /* Common GPIO configuration */
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init_structure.Alternate = GPIO_AF12_FMC;

  /* GPIOD configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 | \
                              GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | \
                              GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOD, &gpio_init_structure);

  /* GPIOE configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | \
                              GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | \
                              GPIO_PIN_15;
  HAL_GPIO_Init(GPIOE, &gpio_init_structure);

  /* GPIOF configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | \
                              GPIO_PIN_5 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOF, &gpio_init_structure);

  /* GPIOG configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | \
                              GPIO_PIN_5 | GPIO_PIN_10;
  HAL_GPIO_Init(GPIOG, &gpio_init_structure);

  /* Configure common MDMA parameters */
  mdma_handle.Init.Request                  = MDMA_REQUEST_SW;
  mdma_handle.Init.TransferTriggerMode      = MDMA_BLOCK_TRANSFER;
  mdma_handle.Init.Priority                 = MDMA_PRIORITY_HIGH;
  mdma_handle.Init.SourceInc                = MDMA_SRC_INC_WORD;
  mdma_handle.Init.DestinationInc           = MDMA_DEST_INC_WORD;
  mdma_handle.Init.SourceDataSize           = MDMA_SRC_DATASIZE_WORD;
  mdma_handle.Init.DestDataSize             = MDMA_DEST_DATASIZE_WORD;
  mdma_handle.Init.DataAlignment            = MDMA_DATAALIGN_PACKENABLE;
  mdma_handle.Init.SourceBurst              = MDMA_SOURCE_BURST_SINGLE;
  mdma_handle.Init.DestBurst                = MDMA_DEST_BURST_SINGLE;
  mdma_handle.Init.BufferTransferLength     = 128;
  mdma_handle.Init.SourceBlockAddressOffset = 0;
  mdma_handle.Init.DestBlockAddressOffset   = 0;

  mdma_handle.Instance = SRAM_MDMAx_CHANNEL;

  /* Associate the DMA handle */
  __HAL_LINKDMA(hSram, hmdma, mdma_handle);

  /* De-initialize the Stream for new transfer */
  (void)HAL_MDMA_DeInit(&mdma_handle);

  /* Configure the DMA Stream */
  (void)HAL_MDMA_Init(&mdma_handle);

  /* NVIC configuration for DMA transfer complete interrupt */
  HAL_NVIC_SetPriority(SRAM_MDMAx_IRQn, BSP_SRAM_IT_PRIORITY, 0);
  HAL_NVIC_EnableIRQ(SRAM_MDMAx_IRQn);

}


/**
  * @brief  DeInitializes SRAM MSP.
  * @param  hSram  SRAM handle
  * @retval None
  */
static void SRAM_MspDeInit(SRAM_HandleTypeDef  *hSram)
{
  GPIO_InitTypeDef gpio_init_structure;
  static MDMA_HandleTypeDef mdma_handle;

  /* Prevent unused argument(s) compilation warning */
  UNUSED(hSram);

  /* GPIOD configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 | \
                              GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | \
                              GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_DeInit(GPIOD, gpio_init_structure.Pin);

  /* GPIOE configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | \
                              GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | \
                              GPIO_PIN_15;
  HAL_GPIO_DeInit(GPIOE, gpio_init_structure.Pin);

  /* GPIOF configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | \
                              GPIO_PIN_5 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_DeInit(GPIOF, gpio_init_structure.Pin);

  /* GPIOG configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | \
                              GPIO_PIN_5 | GPIO_PIN_10;
  HAL_GPIO_DeInit(GPIOG, gpio_init_structure.Pin);

  /* Disable NVIC configuration for MDMA interrupt */
  HAL_NVIC_DisableIRQ(SRAM_MDMAx_IRQn);

  /* De-initialize the channel for new transfer */
  mdma_handle.Instance = SRAM_MDMAx_CHANNEL;
  (void)HAL_MDMA_DeInit(&mdma_handle);

  /* Disable chosen MDMAx clock */
  SRAM_MDMAx_CLK_DISABLE();

  /* Disable FMC clock */
  __HAL_RCC_FMC_CLK_DISABLE();
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
