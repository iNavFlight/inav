/**
  ******************************************************************************
  * @file    stm32h750b_discovery_mmc.c
  * @author  MCD Application Team
  * @brief   This file includes the EMMC driver mounted on STM32H750B-DK
  *          board.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* File Info : -----------------------------------------------------------------
                                   User NOTES
1. How To use this driver:
--------------------------
   - This driver is used to drive the EMMC mounted on STM32H750B-DK board.
   - This driver does not need a specific component driver for the EMMC device
     to be included with.

2. Driver description:
---------------------
  + Initialization steps:
     o Initialize the external EMMC memory using the BSP_MMC_Init() function. This
       function includes the MSP layer hardware resources initialization and the
       SDIO interface configuration to interface with the external EMMC. It
       also includes the EMMC initialization sequence.
     o The function BSP_MMC_GetCardInfo() is used to get the MMC information
       which is stored in the structure "HAL_MMC_CardInfoTypedef".

  + Micro MMC card operations
     o The micro MMC card can be accessed with read/write block(s) operations once
       it is ready for access. The access can be performed whether using the polling
       mode by calling the functions BSP_MMC_ReadBlocks()/BSP_MMC_WriteBlocks(), or by DMA
       transfer using the functions BSP_MMC_ReadBlocks_DMA()/BSP_MMC_WriteBlocks_DMA()
     o The DMA transfer complete is used with interrupt mode. Once the MMC transfer
       is complete, the MMC interrupt is handled using the function BSP_MMC_IRQHandler(),
       the DMA Tx/Rx transfer complete are handled using the functions
       MMC_DMA_Tx_IRQHandler()/MMC_DMA_Rx_IRQHandler() that should be defined by user.
       The corresponding user callbacks are implemented by the user at application level.
     o The MMC erase block(s) is performed using the function BSP_MMC_Erase() with specifying
       the number of blocks to erase.
     o The MMC runtime status is returned when calling the function BSP_MMC_GetStatus().

------------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include "stm32h750b_discovery_mmc.h"


/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H750B_DK
  * @{
  */

/** @defgroup STM32H750B_DK_MMC MMC
  * @{
  */


/** @defgroup STM32H750B_DK_MMC_Exported_Variables Exported Variables
  * @{
  */
MMC_HandleTypeDef hsd_sdmmc[MMC_INSTANCES_NBR];
/**
  * @}
  */

/** @defgroup STM32H750B_DK_MMC_Private_FunctionsPrototypes Private Functions Prototypes
  * @{
  */
static void MMC_MspInit(MMC_HandleTypeDef *hmmc);
static void MMC_MspDeInit(MMC_HandleTypeDef *hmmc);
#if (USE_HAL_MMC_REGISTER_CALLBACKS == 1)
static void MMC_AbortCallback(MMC_HandleTypeDef *hmmc);
static void MMC_TxCpltCallback(MMC_HandleTypeDef *hmmc);
static void MMC_RxCpltCallback(MMC_HandleTypeDef *hmmc);
#endif
/**
  * @}
  */

/** @defgroup STM32H750B_DK_MMC_Exported_Functions Exported Functions
  * @{
  */

/**
  * @brief  Initializes the MMC card device.
  * @param  Instance      SDMMC Instance
  * @retval BSP status
  */
int32_t BSP_MMC_Init(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= MMC_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_HAL_MMC_REGISTER_CALLBACKS == 0)
    /* Msp MMC initialization */
    MMC_MspInit(&hsd_sdmmc[Instance]);
#else
    /* Register the MMC MSP Callbacks */
    if(IsMspCallbacksValid[Instance] == 0UL)
    {
      if(BSP_MMC_RegisterDefaultMspCallbacks(Instance) != BSP_ERROR_NONE)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
    }
    if(ret == BSP_ERROR_NONE)
    {
#endif
      /* HAL MMC initialization */
      if(MX_MMC_SD_Init(&hsd_sdmmc[Instance]) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
#if (USE_HAL_MMC_REGISTER_CALLBACKS == 1)
      /* Register MMC TC, HT and Abort callbacks */
      else if(HAL_MMC_RegisterCallback(&hsd_sdmmc[Instance], HAL_MMC_TX_CPLT_CB_ID, MMC_TxCpltCallback) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else if(HAL_MMC_RegisterCallback(&hsd_sdmmc[Instance], HAL_MMC_RX_CPLT_CB_ID, MMC_RxCpltCallback) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else
      {
        if(HAL_MMC_RegisterCallback(&hsd_sdmmc[Instance], HAL_MMC_ABORT_CB_ID, MMC_AbortCallback) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
      }
    }
#endif /* USE_HAL_MMC_REGISTER_CALLBACKS */
  }

  return  ret;
}

/**
  * @brief  DeInitializes the MMC card device.
  * @param  Instance      SDMMC Instance
  * @retval BSP status
  */
int32_t BSP_MMC_DeInit(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= MMC_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }/* HAL MMC deinitialization */
  else if(HAL_MMC_DeInit(&hsd_sdmmc[Instance]) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    /* Msp MMC de-initialization */
#if (USE_HAL_MMC_REGISTER_CALLBACKS == 0)
    MMC_MspDeInit(&hsd_sdmmc[Instance]);
#endif /* (USE_HAL_MMC_REGISTER_CALLBACKS == 0) */
  }

  return  ret;
}

/**
  * @brief  Initializes the SDMMC1 peripheral.
  * @param  hmmc SD handle
  * @retval HAL status
  */
__weak HAL_StatusTypeDef MX_MMC_SD_Init(MMC_HandleTypeDef *hmmc)
{
  HAL_StatusTypeDef ret = HAL_OK;

  hmmc->Instance                 = SDMMC1;
  hmmc->Init.ClockDiv            = 2;
  hmmc->Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hmmc->Init.ClockEdge           = SDMMC_CLOCK_EDGE_RISING;
  hmmc->Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
  hmmc->Init.BusWide             = SDMMC_BUS_WIDE_8B;

  /* HAL SD initialization */
  if(HAL_MMC_Init(hmmc) != HAL_OK)
  {
    ret = HAL_ERROR;
  }

  return ret;
}

#if (USE_HAL_MMC_REGISTER_CALLBACKS == 1)
/**
  * @brief Default BSP MMC Msp Callbacks
  * @param  Instance      SDMMC Instance
  * @retval BSP status
  */
int32_t BSP_MMC_RegisterDefaultMspCallbacks(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= MMC_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Register MspInit/MspDeInit Callbacks */
    if(HAL_MMC_RegisterCallback(&hsd_sdmmc[Instance], HAL_MMC_MSP_INIT_CB_ID, MMC_MspInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(HAL_MMC_RegisterCallback(&hsd_sdmmc[Instance], HAL_MMC_MSP_DEINIT_CB_ID, MMC_MspDeInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      IsMspCallbacksValid[Instance] = 1U;
    }
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief BSP MMC Msp Callback registering
  * @param  Instance   SDMMC Instance
  * @param  CallBacks  pointer to MspInit/MspDeInit callbacks functions
  * @retval BSP status
  */
int32_t BSP_MMC_RegisterMspCallbacks(uint32_t Instance, BSP_MMC_Cb_t *CallBacks)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= MMC_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Register MspInit/MspDeInit Callbacks */
    if(HAL_MMC_RegisterCallback(&hsd_sdmmc[Instance], HAL_MMC_MSP_INIT_CB_ID, CallBacks->pMspInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(HAL_MMC_RegisterCallback(&hsd_sdmmc[Instance], HAL_MMC_MSP_DEINIT_CB_ID, CallBacks->pMspDeInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      IsMspCallbacksValid[Instance] = 1U;
    }
  }

  /* Return BSP status */
  return ret;
}
#endif /* (USE_HAL_MMC_REGISTER_CALLBACKS == 1) */

/**
  * @brief  Reads block(s) from a specified address in an SD card, in polling mode.
  * @param  Instance   MMC Instance
  * @param  pData      Pointer to the buffer that will contain the data to transmit
  * @param  BlockIdx   Block index from where data is to be read
  * @param  BlocksNbr  Number of MMC blocks to read
  * @retval BSP status
  */
int32_t BSP_MMC_ReadBlocks(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  uint32_t timeout = MMC_READ_TIMEOUT*BlocksNbr;
  int32_t ret;

  if(Instance >= MMC_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(HAL_MMC_ReadBlocks(&hsd_sdmmc[Instance], (uint8_t *)pData, BlockIdx, BlocksNbr, timeout) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in polling mode.
  * @param  Instance   MMC Instance
  * @param  pData      Pointer to the buffer that will contain the data to transmit
  * @param  BlockIdx   Block index from where data is to be written
  * @param  BlocksNbr  Number of MMC blocks to write
  * @retval BSP status
  */
int32_t BSP_MMC_WriteBlocks(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  uint32_t timeout = MMC_READ_TIMEOUT*BlocksNbr;
  int32_t ret;

  if(Instance >= MMC_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(HAL_MMC_WriteBlocks(&hsd_sdmmc[Instance], (uint8_t *)pData, BlockIdx, BlocksNbr, timeout) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Reads block(s) from a specified address in an SD card, in DMA mode.
  * @param  Instance   MMC Instance
  * @param  pData      Pointer to the buffer that will contain the data to transmit
  * @param  BlockIdx   Block index from where data is to be read
  * @param  BlocksNbr  Number of MMC blocks to read
  * @retval BSP status
  */
int32_t BSP_MMC_ReadBlocks_DMA(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  int32_t ret;

  if(Instance >= MMC_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(HAL_MMC_ReadBlocks_DMA(&hsd_sdmmc[Instance], (uint8_t *)pData, BlockIdx, BlocksNbr) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in DMA mode.
  * @param  Instance   MMC Instance
  * @param  pData      Pointer to the buffer that will contain the data to transmit
  * @param  BlockIdx   Block index from where data is to be written
  * @param  BlocksNbr  Number of MMC blocks to write
  * @retval BSP status
  */
int32_t BSP_MMC_WriteBlocks_DMA(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  int32_t ret;

  if(Instance >= MMC_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(HAL_MMC_WriteBlocks_DMA(&hsd_sdmmc[Instance], (uint8_t *)pData, BlockIdx, BlocksNbr) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Erases the specified memory area of the given MMC card.
  * @param  Instance   MMC Instance
  * @param  StartAddr : Start byte address
  * @param  EndAddr : End byte address
  * @retval BSP status
  */
int32_t BSP_MMC_Erase(uint32_t Instance, uint32_t StartAddr, uint32_t EndAddr)
{
  int32_t ret;

  if(Instance >= MMC_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if( HAL_MMC_Erase(&hsd_sdmmc[Instance], StartAddr, EndAddr) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  return ret ;
}

/**
  * @brief  Handles MMC card interrupt request.
  * @param  Instance   MMC Instance
  * @retval None
  */
void BSP_MMC_IRQHandler(uint32_t Instance)
{
  HAL_MMC_IRQHandler(&hsd_sdmmc[Instance]);
}

/**
  * @brief  Gets the current MMC card data status.
  * @param  Instance   MMC Instance
  * @retval Data transfer state.
  *          This value can be one of the following values:
  *            @arg  MMC_TRANSFER_OK: No data transfer is acting
  *            @arg  MMC_TRANSFER_BUSY: Data transfer is acting
  *            @arg  MMC_TRANSFER_ERROR: Data transfer error
  */
int32_t BSP_MMC_GetCardState(uint32_t Instance)
{
  return((HAL_MMC_GetCardState(&hsd_sdmmc[Instance]) == HAL_MMC_CARD_TRANSFER ) ? MMC_TRANSFER_OK : MMC_TRANSFER_BUSY);
}

/**
  * @brief  Get MMC information about specific MMC card.
  * @param  Instance   MMC Instance
  * @param  CardInfo : Pointer to HAL_MMC_CardInfoTypedef structure
  * @retval None
  */
int32_t BSP_MMC_GetCardInfo(uint32_t Instance, BSP_MMC_CardInfo *CardInfo)
{
  int32_t ret;

  if(Instance >= MMC_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(HAL_MMC_GetCardInfo(&hsd_sdmmc[Instance], CardInfo) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief BSP MMC Abort callbacks
  * @param  Instance   MMC Instance
  * @retval None
  */
__weak void BSP_MMC_AbortCallback(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);
}

/**
  * @brief BSP Tx Transfer completed callbacks
  * @param  Instance   MMC Instance
  * @retval None
  */
__weak void BSP_MMC_WriteCpltCallback(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);
}

/**
  * @brief BSP Rx Transfer completed callbacks
  * @param  Instance   MMC Instance
  * @retval None
  */
__weak void BSP_MMC_ReadCpltCallback(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);
}

#if (USE_HAL_MMC_REGISTER_CALLBACKS == 0)
/**
  * @brief MMC Abort callbacks
  * @param hmmc : MMC handle
  * @retval None
  */
void HAL_MMC_AbortCallback(MMC_HandleTypeDef *hmmc)
{
  BSP_MMC_AbortCallback(0);
}

/**
  * @brief Tx Transfer completed callbacks
  * @param hmmc: MMC handle
  * @retval None
  */
void HAL_MMC_TxCpltCallback(MMC_HandleTypeDef *hmmc)
{
  BSP_MMC_WriteCpltCallback(0);
}

/**
  * @brief Rx Transfer completed callbacks
  * @param hmmc: MMC handle
  * @retval None
  */
void HAL_MMC_RxCpltCallback(MMC_HandleTypeDef *hmmc)
{
  BSP_MMC_ReadCpltCallback(0);
}
#endif
/**
  * @}
  */

/** @defgroup STM32H750B_DK_MMC_Private_Functions Private Functions
  * @{
  */

/**
  * @brief  Initializes the MMC MSP.
  * @param  hmmc  MMC handle
  * @retval None
  */
static void MMC_MspInit(MMC_HandleTypeDef *hmmc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hmmc);

  /* __weak function can be modified by the application */

  GPIO_InitTypeDef gpio_init_structure;

  /* Enable SDIO clock */
  __HAL_RCC_SDMMC1_CLK_ENABLE();

  /* Enable GPIOs clock */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();


  /* Common GPIO configuration */
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init_structure.Alternate = GPIO_AF12_SDIO1;

  /* SDMMC GPIO CLKIN PB8, D0 PC8, D1 PC9, D2 PC10, D3 PC11, CK PC12, CMD PD2 */
  /* GPIOC configuration */
  gpio_init_structure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
  HAL_GPIO_Init(GPIOC, &gpio_init_structure);

  /* GPIOD configuration */
  gpio_init_structure.Pin = GPIO_PIN_2;
  HAL_GPIO_Init(GPIOD, &gpio_init_structure);

  gpio_init_structure.Pin = GPIO_PIN_8 | GPIO_PIN_9;
  HAL_GPIO_Init(GPIOB, &gpio_init_structure);


  /* NVIC configuration for SDIO interrupts */
  HAL_NVIC_SetPriority(SDMMC1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(SDMMC1_IRQn);
}

/**
  * @brief  DeInitializes the MMC MSP.
  * @param  hmmc : MMC handle
  * @retval None
  */
static void MMC_MspDeInit(MMC_HandleTypeDef *hmmc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hmmc);

  /* Disable NVIC for SDIO interrupts */
  HAL_NVIC_DisableIRQ(SDMMC1_IRQn);

  /* DeInit GPIO pins can be done in the application
  (by surcharging this __weak function) */

  /* Disable SDMMC1 clock */
  __HAL_RCC_SDMMC1_CLK_DISABLE();

  /* GPIO pins clock and DMA clocks can be shut down in the application
  by surcharging this __weak function */
}

#if (USE_HAL_MMC_REGISTER_CALLBACKS == 1)
/**
  * @brief MMC Abort callbacks
  * @param hmmc : MMC handle
  * @retval None
  */
static void MMC_AbortCallback(MMC_HandleTypeDef *hmmc)
{
  BSP_MMC_AbortCallback(0);
}

/**
  * @brief Tx Transfer completed callbacks
  * @param hmmc : MMC handle
  * @retval None
  */
static void MMC_TxCpltCallback(MMC_HandleTypeDef *hmmc)
{
  BSP_MMC_WriteCpltCallback(0);
}

/**
  * @brief Rx Transfer completed callbacks
  * @param hmmc : MMC handle
  * @retval None
  */
static void MMC_RxCpltCallback(MMC_HandleTypeDef *hmmc)
{
  BSP_MMC_ReadCpltCallback(0);
}
#endif

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
