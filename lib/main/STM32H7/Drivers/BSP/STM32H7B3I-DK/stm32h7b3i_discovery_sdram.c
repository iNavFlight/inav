/**
  ******************************************************************************
  * @file    stm32h7b3i_discovery_sdram.c
  * @author  MCD Application Team
  * @brief   This file includes the SDRAM driver for the IS42S16800J-6BLI memory
  *          device mounted on STM32H7B3I_DK boards.
  @verbatim
  How To use this driver:
  -----------------------
   - This driver is used to drive the IS42S16800J-6BLI SDRAM external memory mounted
     on STM32H7B3I_DK board.
   - This driver does not need a specific component driver for the SDRAM device
     to be included with.

  Driver description:
  ------------------
  + Initialization steps:
     o Initialize the SDRAM external memory using the BSP_SDRAM_Init() function. This
       function includes the MSP layer hardware resources initialization and the
       FMC controller configuration to interface with the external SDRAM memory.
     o It contains the SDRAM initialization sequence to program the SDRAM external
       device using the function BSP_SDRAM_Initialization_sequence(). Note that this
       sequence is standard for all SDRAM devices, but can include some differences
       from a device to another. If it is the case, the right sequence should be
       implemented separately.

  + SDRAM read/write operations
     o SDRAM external memory can be accessed with read/write operations once it is
       initialized.
       Read/write operation can be performed with AHB access using the functions
       BSP_SDRAM_ReadData()/BSP_SDRAM_WriteData(), or by DMA transfer using the functions
       BSP_SDRAM_ReadData_DMA()/BSP_SDRAM_WriteData_DMA().
     o The AHB access is performed with 32-bit width transaction, the DMA transfer
       configuration is fixed at single (no burst) word transfer (see the
       SDRAM_MspInit() static function).
     o User can implement his own functions for read/write access with his desired
       configurations.
     o If interrupt mode is used for DMA transfer, the function BSP_SDRAM_DMA_IRQHandler()
       is called in IRQ handler file, to serve the generated interrupt once the DMA
       transfer is complete.
     o You can send a command to the SDRAM device in runtime using the function
       BSP_SDRAM_Sendcmd(), and giving the desired command as parameter chosen between
       the predefined commands of the "FMC_SDRAM_CommandTypeDef" structure.
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
#include "stm32h7b3i_discovery_sdram.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H7B3I_DK
  * @{
  */

/** @addtogroup STM32H7B3I_DK_SDRAM
  * @{
  */

/** @defgroup STM32H7B3I_DK_SDRAM_Exported_Variables Exported Variables
  * @{
  */
SDRAM_HandleTypeDef hsdram[SDRAM_INSTANCES_NBR];
/**
  * @}
  */

/** @defgroup STM32H7B3I_DK_SDRAM_Private_Variables Private Variables
  * @{
  */
#if (USE_HAL_SDRAM_REGISTER_CALLBACKS == 1)
static uint32_t IsMspCallbacksValid = 0;
#endif
/**
  * @}
  */

/** @defgroup STM32H7B3I_DK_SDRAM_Private_Functions_Prototypes Private Functions Prototypes
  * @{
  */
static void SDRAM_MspInit(SDRAM_HandleTypeDef  *hSdram);
static void SDRAM_MspDeInit(SDRAM_HandleTypeDef  *hSdram);
/**
  * @}
  */

/** @defgroup STM32H7B3I_DK_SDRAM_Exported_Functions Exported Functions
  * @{
  */

/**
  * @brief  Initializes the SDRAM device.
  * @param Instance  SDRAM Instance
  * @retval BSP status
  */
int32_t BSP_SDRAM_Init(uint32_t Instance)
{
  int32_t ret;
  static IS42S16800J_Context_t pRegMode;
  uint32_t row_bits_num, mem_bus_width;

  if(Instance >= SDRAM_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_HAL_SDRAM_REGISTER_CALLBACKS == 1)
    /* Register the SDRAM MSP Callbacks */
    if(IsMspCallbacksValid == 0)
    {
      if(BSP_SDRAM_RegisterDefaultMspCallbacks(Instance) != BSP_ERROR_NONE)
      {
        return BSP_ERROR_PERIPH_FAILURE;
      }
    }
#else
    /* Msp SDRAM initialization */
    SDRAM_MspInit(&hsdram[Instance]);
#endif /* USE_HAL_SDRAM_REGISTER_CALLBACKS */

    row_bits_num  = FMC_SDRAM_ROW_BITS_NUM_12;
    mem_bus_width = FMC_SDRAM_MEM_BUS_WIDTH_16;

    if(MX_SDRAM_BANK2_Init(&hsdram[Instance], row_bits_num, mem_bus_width) != HAL_OK)
    {
      ret = BSP_ERROR_NO_INIT;
    }
    else
    {
      /* External memory mode register configuration */
      pRegMode.TargetBank      = FMC_SDRAM_CMD_TARGET_BANK2;
      pRegMode.RefreshMode     = IS42S16800J_AUTOREFRESH_MODE_CMD;
      pRegMode.RefreshRate     = REFRESH_COUNT;
      pRegMode.BurstLength     = IS42S16800J_BURST_LENGTH_1;
      pRegMode.BurstType       = IS42S16800J_BURST_TYPE_SEQUENTIAL;
      pRegMode.CASLatency      = IS42S16800J_CAS_LATENCY_2;
      pRegMode.OperationMode   = IS42S16800J_OPERATING_MODE_STANDARD;
      pRegMode.WriteBurstMode  = IS42S16800J_WRITEBURST_MODE_SINGLE;

      /* SDRAM initialization sequence */
      if(IS42S16800J_Init(&hsdram[Instance], &pRegMode) != IS42S16800J_OK)
      {
        ret =  BSP_ERROR_COMPONENT_FAILURE;
      }
      else
      {
        ret = BSP_ERROR_NONE;
      }
    }
  }

  return ret;
}

/**
  * @brief  DeInitializes the SDRAM device.
  * @param Instance  SDRAM Instance
  * @retval BSP status
  */
int32_t BSP_SDRAM_DeInit(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= SDRAM_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    (void)HAL_SDRAM_DeInit(&hsdram[Instance]);
#if (USE_HAL_SDRAM_REGISTER_CALLBACKS == 0)
    /* SDRAM controller de-initialization */
    SDRAM_MspDeInit(&hsdram[Instance]);
#endif /* (USE_HAL_SDRAM_REGISTER_CALLBACKS == 0) */
  }

  return ret;
}

/**
  * @brief  Initializes the SDRAM periperal.
  * @param  hSdram SDRAM handle
  * @param  RowBitsNumber Number of row to set
  * @param  MemoryDataWidth The momory width 16 or 32bits
  * @retval HAL status
  */
__weak HAL_StatusTypeDef MX_SDRAM_BANK2_Init(SDRAM_HandleTypeDef *hSdram, uint32_t RowBitsNumber, uint32_t MemoryDataWidth)
{
  FMC_SDRAM_TimingTypeDef sdram_timing;

  /* SDRAM device configuration */
  hsdram->Instance = FMC_SDRAM_DEVICE;

  /* SDRAM handle configuration */
  hSdram->Init.SDBank             = FMC_SDRAM_BANK2;
  hSdram->Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_9;
  hSdram->Init.RowBitsNumber      = RowBitsNumber;
  hSdram->Init.MemoryDataWidth    = MemoryDataWidth;
  hSdram->Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hSdram->Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_2;
  hSdram->Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hSdram->Init.SDClockPeriod      = FMC_SDRAM_CLOCK_PERIOD_3;
  hSdram->Init.ReadBurst          = FMC_SDRAM_RBURST_ENABLE;
  hSdram->Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_2;

  /* tick = 1/(HCLK/3) = 1/93,3Mhz = 10 ns */
  /* Timing configuration for as SDRAM */
  sdram_timing.LoadToActiveDelay    = 2;   /* Load Mode Register to Active  */
  sdram_timing.ExitSelfRefreshDelay = 7;  /* Exit Self-refresh delay       */
  sdram_timing.SelfRefreshTime      = 4;   /* Self refresh time             */
  sdram_timing.RowCycleDelay        = 7;  /* Row cycle delay               */
  sdram_timing.WriteRecoveryTime    = 2;   /* Recovery delay                */
  sdram_timing.RPDelay              = 2;   /* Row precharge delay           */
  sdram_timing.RCDDelay             = 2;   /* Row to column delay           */

  /* SDRAM controller initialization */
  if(HAL_SDRAM_Init(hSdram, &sdram_timing) != HAL_OK)
  {
    return  HAL_ERROR;
  }
  return HAL_OK;
}


#if (USE_HAL_SDRAM_REGISTER_CALLBACKS == 1)
/**
  * @brief Default BSP SDRAM Msp Callbacks
  * @param Instance      SDRAM Instance
  * @retval BSP status
  */
int32_t BSP_SDRAM_RegisterDefaultMspCallbacks (uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if(Instance >=SDRAM_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Register MspInit/MspDeInit Callbacks */
    if(HAL_SDRAM_RegisterCallback(&hsdram[Instance], HAL_SDRAM_MSP_INIT_CB_ID, SDRAM_MspInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    if(HAL_SDRAM_RegisterCallback(&hsdram[Instance], HAL_SDRAM_MSP_DEINIT_CB_ID, SDRAM_MspDeInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      IsMspCallbacksValid = 1;
    }
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief BSP SDRAM Msp Callback registering
  * @param Instance     SDRAM Instance
  * @param CallBacks    pointer to MspInit/MspDeInit callbacks functions
  * @retval BSP status
  */
int32_t BSP_SDRAM_RegisterMspCallbacks (uint32_t Instance, BSP_SDRAM_Cb_t *CallBacks)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if(Instance >=SDRAM_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Register MspInit/MspDeInit Callbacks */
    if(HAL_SDRAM_RegisterCallback(&hsdram[Instance], HAL_SDRAM_MSP_INIT_CB_ID, CallBacks->pMspInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    if(HAL_SDRAM_RegisterCallback(&hsdram[Instance], HAL_SDRAM_MSP_DEINIT_CB_ID, CallBacks->pMspDeInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      IsMspCallbacksValid = 1;
    }
  }
  /* Return BSP status */
  return ret;
}
#endif /* (USE_HAL_SDRAM_REGISTER_CALLBACKS == 1) */

/**
  * @brief  Sends command to the SDRAM bank.
  * @param  Instance  SDRAM Instance
  * @param  SdramCmd  Pointer to SDRAM command structure
  * @retval BSP status
  */
int32_t BSP_SDRAM_SendCmd(uint32_t Instance, FMC_SDRAM_CommandTypeDef *SdramCmd)
{
  int32_t ret;

  if(Instance >=SDRAM_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }
  else if(IS42S16800J_Sendcmd(&hsdram[Instance], SdramCmd) != IS42S16800J_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  return ret;
}

/**
  * @brief  This function handles SDRAM interrupt request.
  * @param  Instance  SDRAM Instance
  * @retval None
  */
void BSP_SDRAM_IRQHandler(uint32_t Instance)
{
  HAL_MDMA_IRQHandler(hsdram[Instance].hmdma);
}

/**
  * @}
  */

/** @defgroup STM32H7B3I_DK_SDRAM_Private_Functions Private Functions
  * @{
  */
/**
  * @brief  Initializes SDRAM MSP.
  * @param  hSdram SDRAM handle
  * @retval None
  */
static void SDRAM_MspInit(SDRAM_HandleTypeDef  *hSdram)
{
  static MDMA_HandleTypeDef mdma_handle;
  GPIO_InitTypeDef gpio_init_structure;

  /* Enable FMC clock */
  __HAL_RCC_FMC_CLK_ENABLE();

  /* Enable chosen DMAx clock */
  SDRAM_MDMAx_CLK_ENABLE();

  /* Enable GPIOs clock */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /* Common GPIO configuration */
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_NOPULL;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init_structure.Alternate = GPIO_AF12_FMC;

  /* GPIOD configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8| GPIO_PIN_9 | GPIO_PIN_10 |\
                              GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOD, &gpio_init_structure);

  /* GPIOE configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7| GPIO_PIN_8 | GPIO_PIN_9 |\
                              GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |\
                              GPIO_PIN_15;
  HAL_GPIO_Init(GPIOE, &gpio_init_structure);

  /* GPIOF configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 | GPIO_PIN_4 |\
                              GPIO_PIN_5 | GPIO_PIN_11  | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOF, &gpio_init_structure);

  /* GPIOG configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4| GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOG, &gpio_init_structure);

    /* GPIOH configuration */
  gpio_init_structure.Pin   = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
  HAL_GPIO_Init(GPIOH, &gpio_init_structure);

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

  mdma_handle.Instance = SDRAM_MDMAx_CHANNEL;

   /* Associate the DMA handle */
  __HAL_LINKDMA(hSdram, hmdma, mdma_handle);

  /* De-initialize the Stream for new transfer */
  (void)HAL_MDMA_DeInit(&mdma_handle);

  /* Configure the DMA Stream */
  (void)HAL_MDMA_Init(&mdma_handle);

  /* NVIC configuration for DMA transfer complete interrupt */
  HAL_NVIC_SetPriority(SDRAM_MDMAx_IRQn, BSP_SDRAM_IT_PRIORITY, 0);
  HAL_NVIC_EnableIRQ(SDRAM_MDMAx_IRQn);
}

/**
  * @brief  DeInitializes SDRAM MSP.
  * @param  hSdram SDRAM handle
  * @retval None
  */
static void SDRAM_MspDeInit(SDRAM_HandleTypeDef  *hSdram)
{
  static DMA_HandleTypeDef mdma_handle;

  /* Prevent unused argument(s) compilation warning */
  UNUSED(hSdram);

  /* Disable NVIC configuration for DMA interrupt */
  HAL_NVIC_DisableIRQ(SDRAM_MDMAx_IRQn);

  /* Deinitialize the stream for new transfer */
  mdma_handle.Instance = SDRAM_MDMAx_CHANNEL;
  (void)HAL_DMA_DeInit(&mdma_handle);

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
