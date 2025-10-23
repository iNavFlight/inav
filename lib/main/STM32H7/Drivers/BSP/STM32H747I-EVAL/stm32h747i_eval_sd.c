/**
  ******************************************************************************
  * @file    stm32h747i_eval_sd.c
  * @author  MCD Application Team
  * @brief   This file includes the uSD card driver mounted on STM32H747I_EVAL
  *          boards.
  @verbatim
  How To use this driver:
  -----------------------
   - This driver is used to drive the micro SD external cards mounted on STM32H747I_EVAL
     board.
   - This driver does not need a specific component driver for the micro SD device
     to be included with.

  Driver description:
  ------------------
  + Initialization steps:
     o Initialize the micro SD card using the BSP_SD_Init() function. This
       function includes the MSP layer hardware resources initialization and the
       SDIO interface configuration to interface with the external micro SD. It
       also includes the micro SD initialization sequence.

     o To check the SD card presence you can use the function BSP_SD_IsDetected() which
       returns the detection status.

     o If SD presence detection interrupt mode is desired, you must configure the
       SD detection interrupt mode by calling the function BSP_SD_DetectITConfig().
       The interrupt is generated as an external interrupt whenever the micro SD card is
       plugged/unplugged in/from the evaluation board.
       The SD detection is managed by MFX, so the SD detection interrupt has to be
       treated by MFX_IRQOUT gpio pin IRQ handler. BSP_SD_DetectCallback() is called when
       SD is detected.

     o The function BSP_SD_GetCardInfo()are used to get the micro SD card information
       which is stored in the structure "HAL_SD_CardInfoTypedef".

  + Micro SD card operations
     o The micro SD card can be accessed with read/write block(s) operations once
       it is ready for access. The access can be performed whether
       using the polling mode by calling the functions BSP_SD_ReadBlocks()/BSP_SD_WriteBlocks(),
       using the interrupt mode by calling the functions BSP_SD_ReadBlocks_IT()/BSP_SD_WriteBlocks_IT(),
       or by DMA transfer using the functions BSP_SD_ReadBlocks_DMA()/BSP_SD_WriteBlocks_DMA().
     o The DMA transfer complete is used with interrupt mode. Once the SD transfer
       is complete, the SD interrupt is handled using the function BSP_SDMMC1_IRQHandler()
       The DMA Tx/Rx transfer complete are handled using the functions
       SD_SDMMC1_DMA_Tx_IRQHandler(), SD_SDMMC1_DMA_Rx_IRQHandler().The corresponding
       user callbacks are implemented by the user at application level.
     o The SD erase block(s) is performed using the functions BSP_SD_Erase() with specifying
       the number of blocks to erase.
     o The SD runtime status is returned when calling the function BSP_SD_GetCardState().

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
#include "stm32h747i_eval_sd.h"
#include "stm32h747i_eval_bus.h"
#include "stm32h747i_eval_io.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H747I_EVAL
  * @{
  */

/** @addtogroup STM32H747I_EVAL_SD
  * @{
  */

/** @defgroup STM32H747I_EVAL_SD_Private_TypesDefinitions Private TypesDefinitions
  * @{
  */
/**
  * @}
  */

/** @defgroup STM32H747I_EVAL_SD_Private_Defines Private Defines
  * @{
  */
/**
  * @}
  */

/** @defgroup STM32H747I_EVAL_SD_Private_Macros Private Macros
  * @{
  */
/**
  * @}
  */
/** @defgroup STM32H747I_EVAL_SD_Private_TypesDefinitions Private TypesDefinitions
  * @{
  */
typedef void (* BSP_EXTI_LineCallback) (void);
/**
  * @}
  */

/** @defgroup STM32H747I_EVAL_SD_Exported_Variables Exported Variables
  * @{
  */
SD_HandleTypeDef hsd_sdmmc[SD_INSTANCES_NBR];
EXTI_HandleTypeDef hsd_exti[SD_INSTANCES_NBR];
/**
  * @}
  */

/** @defgroup STM32H747I_EVAL_SD_Private_Variables Private Variables
  * @{
  */
#if (USE_BSP_IO_CLASS > 0U)
static uint32_t PinDetect[SD_INSTANCES_NBR]  = {SD_DETECT_PIN};
#endif
#if (USE_HAL_SD_REGISTER_CALLBACKS == 1)
/* Is Msp Callbacks registered   */
static uint32_t   IsMspCallbacksValid[SD_INSTANCES_NBR] = {0};
#endif
/**
  * @}
  */
/** @defgroup STM32H747I_EVAL_SD_Private_Functions_Prototypes Private Functions Prototypes
  * @{
  */
static void SD_MspInit(SD_HandleTypeDef *hsd);
static void SD_MspDeInit(SD_HandleTypeDef *hsd);
#if (USE_HAL_SD_REGISTER_CALLBACKS == 1)
static void SD_AbortCallback(SD_HandleTypeDef *hsd);
static void SD_TxCpltCallback(SD_HandleTypeDef *hsd);
static void SD_RxCpltCallback(SD_HandleTypeDef *hsd);
#if (USE_SD_TRANSCEIVER != 0U)
static void SD_DriveTransceiver_1_8V_Callback(FlagStatus status);
#endif
#endif /* (USE_HAL_SD_REGISTER_CALLBACKS == 1)   */
static void SD_EXTI_Callback(void);
/**
  * @}
  */

/** @defgroup STM32H747I_EVAL_SD_Exported_Functions Exported Functions
  * @{
  */

/**
  * @brief  Initializes the SD card device.
  * @param  Instance      SD Instance
  * @retval BSP status
  */
int32_t BSP_SD_Init(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

#if (USE_BSP_IO_CLASS > 0U)
  BSP_IO_Init_t io_init_structure;
#endif

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_BSP_IO_CLASS > 0U)

    /* Configure SD pin detect   */
    io_init_structure.Pin  =PinDetect[Instance];
    io_init_structure.Pull = IO_PULLUP;
    io_init_structure.Mode = IO_MODE_INPUT;

    if(BSP_IO_Init(0, &io_init_structure) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_BUS_FAILURE;
    }
    else
    {
      /* Initialise Transciver MFXPIN to enable 1.8V Switch mode */
      io_init_structure.Pin  = SD_LDO_SEL_PIN;
      io_init_structure.Pull = IO_PULLDOWN;
      io_init_structure.Mode = IO_MODE_OUTPUT_PP;

      if(BSP_IO_Init(0, &io_init_structure) != BSP_ERROR_NONE)
      {
        ret = BSP_ERROR_BUS_FAILURE;
      }
    }
#endif
    /* Check if SD card is present   */

    if((uint32_t)BSP_SD_IsDetected(Instance) != SD_PRESENT)
    {
      ret = BSP_ERROR_UNKNOWN_COMPONENT;
    }
    else
    {

#if (USE_HAL_SD_REGISTER_CALLBACKS == 1)
      /* Register the SD MSP Callbacks   */
      if(IsMspCallbacksValid[Instance] == 0UL)
      {
        if(BSP_SD_RegisterDefaultMspCallbacks(Instance) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
      }
#else
      /* Msp SD initialization   */
      SD_MspInit(&hsd_sdmmc[Instance]);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS   */

      if(ret == BSP_ERROR_NONE)
      {
        /* HAL SD initialization and Enable wide operation   */
        if(MX_SDMMC1_SD_Init(&hsd_sdmmc[Instance]) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
        else if(HAL_SD_ConfigWideBusOperation(&hsd_sdmmc[Instance], SDMMC_BUS_WIDE_4B) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
        else
        {
          /* Switch to High Speed mode if the card support this mode */
          (void)HAL_SD_ConfigSpeedBusOperation(&hsd_sdmmc[Instance], SDMMC_SPEED_MODE_HIGH);

#if (USE_HAL_SD_REGISTER_CALLBACKS == 1)
          /* Register SD TC, HT and Abort callbacks */
          if(HAL_SD_RegisterCallback(&hsd_sdmmc[Instance], HAL_SD_TX_CPLT_CB_ID, SD_TxCpltCallback) != HAL_OK)
          {
            ret = BSP_ERROR_PERIPH_FAILURE;
          }
          else if(HAL_SD_RegisterCallback(&hsd_sdmmc[Instance], HAL_SD_RX_CPLT_CB_ID, SD_RxCpltCallback) != HAL_OK)
          {
            ret = BSP_ERROR_PERIPH_FAILURE;
          }
          else if(HAL_SD_RegisterCallback(&hsd_sdmmc[Instance], HAL_SD_ABORT_CB_ID, SD_AbortCallback) != HAL_OK)
          {
            ret = BSP_ERROR_PERIPH_FAILURE;
          }
          else
          {
#if (USE_SD_TRANSCEIVER != 0U)
            if(HAL_SD_RegisterTransceiverCallback(&hsd_sdmmc[Instance], SD_DriveTransceiver_1_8V_Callback) != HAL_OK)
            {
              ret = BSP_ERROR_PERIPH_FAILURE;
            }
#endif
          }
#endif /* USE_HAL_SD_REGISTER_CALLBACKS   */
        }
      }
    }
  }


  return ret;
}

/**
  * @brief  DeInitializes the SD card device.
  * @param Instance      SD Instance
  * @retval SD status
  */
int32_t BSP_SD_DeInit(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

#if (USE_BSP_IO_CLASS > 0U)
  BSP_IO_Init_t io_init_structure;
#endif
  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(HAL_SD_DeInit(&hsd_sdmmc[Instance]) != HAL_OK)/* HAL SD de-initialization   */
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    /* Msp SD de-initialization   */
#if (USE_HAL_SD_REGISTER_CALLBACKS == 0)
    SD_MspDeInit(&hsd_sdmmc[Instance]);
#endif /* (USE_HAL_SD_REGISTER_CALLBACKS == 0)   */
#if (USE_BSP_IO_CLASS > 0U)
    io_init_structure.Pin  = PinDetect[Instance];
    io_init_structure.Pull = IO_PULLUP;
    io_init_structure.Mode = IO_MODE_INPUT;

    if(BSP_IO_Init(0, &io_init_structure) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_BUS_FAILURE;
    }
#endif
  }

  return ret;
}

/**
  * @brief  Initializes the SDMMC1 peripheral.
  * @param  hsd SD handle
  * @retval HAL status
  */
__weak HAL_StatusTypeDef MX_SDMMC1_SD_Init(SD_HandleTypeDef *hsd)
{
  HAL_StatusTypeDef ret = HAL_OK;
  /* uSD device interface configuration */
  hsd->Instance                 = SDMMC1;
  hsd->Init.ClockEdge           = SDMMC_CLOCK_EDGE_RISING;
  hsd->Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd->Init.BusWide             = SDMMC_BUS_WIDE_4B;
  hsd->Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd->Init.TranceiverPresent   = SDMMC_TRANSCEIVER_PRESENT;
#if ( USE_SD_HIGH_PERFORMANCE > 0 )
  hsd->Init.ClockDiv            = SDMMC_HSpeed_CLK_DIV;
#else
  hsd->Init.ClockDiv            = SDMMC_NSpeed_CLK_DIV;
#endif

  /* HAL SD initialization   */
  if(HAL_SD_Init(hsd) != HAL_OK)
  {
    ret = HAL_ERROR;
  }

  return ret;
}

#if (USE_HAL_SD_REGISTER_CALLBACKS == 1)
/**
  * @brief Default BSP SD Msp Callbacks
  * @param Instance      SD Instance
  * @retval BSP status
  */
int32_t BSP_SD_RegisterDefaultMspCallbacks(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Register MspInit/MspDeInit Callbacks */
    if(HAL_SD_RegisterCallback(&hsd_sdmmc[Instance], HAL_SD_MSP_INIT_CB_ID, SD_MspInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(HAL_SD_RegisterCallback(&hsd_sdmmc[Instance], HAL_SD_MSP_DEINIT_CB_ID, SD_MspDeInit) != HAL_OK)
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
  * @brief BSP SD Msp Callback registering
  * @param Instance     SD Instance
  * @param CallBacks    pointer to MspInit/MspDeInit callbacks functions
  * @retval BSP status
  */
int32_t BSP_SD_RegisterMspCallbacks(uint32_t Instance, BSP_SD_Cb_t *CallBacks)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Register MspInit/MspDeInit Callbacks */
    if(HAL_SD_RegisterCallback(&hsd_sdmmc[Instance], HAL_SD_MSP_INIT_CB_ID, CallBacks->pMspInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(HAL_SD_RegisterCallback(&hsd_sdmmc[Instance], HAL_SD_MSP_DEINIT_CB_ID, CallBacks->pMspDeInitCb) != HAL_OK)
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
#endif /* (USE_HAL_SD_REGISTER_CALLBACKS == 1) */

/**
  * @brief  Configures Interrupt mode for SD detection pin.
  * @param  Instance      SD Instance
  * @retval BSP status
  */
int32_t BSP_SD_DetectITConfig(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;
  const uint32_t SD_EXTI_LINE[SD_INSTANCES_NBR]   = {SD_DETECT_EXTI_LINE};
  static BSP_EXTI_LineCallback SdCallback[SD_INSTANCES_NBR] = {SD_EXTI_Callback};

  if(Instance> SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_BSP_IO_CLASS > 0U)
    BSP_IO_Init_t io_init_structure;

    io_init_structure.Pin  = PinDetect[Instance];
    io_init_structure.Pull = IO_PULLUP;
    /* Configure IO interrupt acquisition mode   */
    if(((uint32_t)BSP_IO_ReadPin(0, PinDetect[Instance]) && PinDetect[Instance]) != PinDetect[Instance])
    {
      io_init_structure.Mode = IO_MODE_IT_RISING_EDGE;
    }
    else
    {
      io_init_structure.Mode = IO_MODE_IT_FALLING_EDGE;
    }

    if(BSP_IO_Init(0, &io_init_structure) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_BUS_FAILURE;
    }
#endif
    if(ret == BSP_ERROR_NONE)
    {
      if(HAL_EXTI_GetHandle(&hsd_exti[Instance], SD_EXTI_LINE[Instance]) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else
      {
        if(HAL_EXTI_RegisterCallback(&hsd_exti[Instance],  HAL_EXTI_COMMON_CB_ID, SdCallback[Instance]) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
      }
    }
  }
   return ret;
}

/**
  * @brief  BSP SD Callback.
  * @param  Instance SD Instance
  * @param  Status   Pin status
  * @retval None.
  */
__weak void BSP_SD_DetectCallback(uint32_t Instance, uint32_t Status)
{
  /* Prevent unused argument(s) compilation warning   */
  UNUSED(Instance);
  UNUSED(Status);

  /* This function should be implemented by the user application.
  It is called into this driver when an event on JoyPin is triggered.   */
}

/**
  * @brief  Detects if SD card is correctly plugged in the memory slot or not.
  * @param Instance  SD Instance
  * @retval Returns if SD is detected or not
  */
int32_t BSP_SD_IsDetected(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_UNKNOWN_FAILURE;

  if(Instance >= SD_INSTANCES_NBR)
  {
    return BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Check SD card detect pin */
#if (USE_BSP_IO_CLASS > 0)
   if((BSP_IO_ReadPin(0, PinDetect[Instance]) && PinDetect[Instance]) != 0UL)
    {
      ret = (int32_t)SD_NOT_PRESENT;
    }
    else
    {
      ret = (int32_t)SD_PRESENT;
    }
#endif
  }

  return ret;
}

/**
  * @brief  Reads block(s) from a specified address in an SD card, in polling mode.
  * @param  Instance   SD Instance
  * @param  pData      Pointer to the buffer that will contain the data to transmit
  * @param  BlockIdx   Block index from where data is to be read
  * @param  BlocksNbr  Number of SD blocks to read
  * @retval BSP status
  */
int32_t BSP_SD_ReadBlocks(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t timeout = SD_READ_TIMEOUT*BlocksNbr;

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(HAL_SD_ReadBlocks(&hsd_sdmmc[Instance], (uint8_t *)pData, BlockIdx, BlocksNbr, timeout) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  /* Return BSP status   */
  return ret;
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in polling mode.
  * @param  Instance   SD Instance
  * @param  pData      Pointer to the buffer that will contain the data to transmit
  * @param  BlockIdx   Block index from where data is to be written
  * @param  BlocksNbr  Number of SD blocks to write
  * @retval BSP status
  */
int32_t BSP_SD_WriteBlocks(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t timeout = SD_READ_TIMEOUT*BlocksNbr;

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(HAL_SD_WriteBlocks(&hsd_sdmmc[Instance], (uint8_t *)pData, BlockIdx, BlocksNbr, timeout) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  /* Return BSP status   */
  return ret;
}

/**
  * @brief  Reads block(s) from a specified address in an SD card, in DMA mode.
  * @param  Instance   SD Instance
  * @param  pData      Pointer to the buffer that will contain the data to transmit
  * @param  BlockIdx   Block index from where data is to be read
  * @param  BlocksNbr  Number of SD blocks to read
  * @retval BSP status
  */
int32_t BSP_SD_ReadBlocks_DMA(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(HAL_SD_ReadBlocks_DMA(&hsd_sdmmc[Instance], (uint8_t *)pData, BlockIdx, BlocksNbr) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  /* Return BSP status   */
  return ret;
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in DMA mode.
  * @param  Instance   SD Instance
  * @param  pData      Pointer to the buffer that will contain the data to transmit
  * @param  BlockIdx   Block index from where data is to be written
  * @param  BlocksNbr  Number of SD blocks to write
  * @retval BSP status
  */
int32_t BSP_SD_WriteBlocks_DMA(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(HAL_SD_WriteBlocks_DMA(&hsd_sdmmc[Instance], (uint8_t *)pData, BlockIdx, BlocksNbr) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  /* Return BSP status   */
  return ret;
}

/**
  * @brief  Reads block(s) from a specified address in an SD card, in DMA mode.
  * @param  Instance   SD Instance
  * @param  pData      Pointer to the buffer that will contain the data to transmit
  * @param  BlockIdx   Block index from where data is to be read
  * @param  BlocksNbr  Number of SD blocks to read
  * @retval SD status
  */
int32_t BSP_SD_ReadBlocks_IT(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(HAL_SD_ReadBlocks_IT(&hsd_sdmmc[Instance], (uint8_t *)pData, BlockIdx, BlocksNbr) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  /* Return BSP status   */
  return ret;
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in DMA mode.
  * @param  Instance   SD Instance
  * @param  pData      Pointer to the buffer that will contain the data to transmit
  * @param  BlockIdx   Block index from where data is to be written
  * @param  BlocksNbr  Number of SD blocks to write
  * @retval SD status
  */
int32_t BSP_SD_WriteBlocks_IT(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(HAL_SD_WriteBlocks_IT(&hsd_sdmmc[Instance], (uint8_t *)pData, BlockIdx, BlocksNbr) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  /* Return BSP status   */
  return ret;
}


/**
  * @brief  Erases the specified memory area of the given SD card.
  * @param  Instance   SD Instance
  * @param  BlockIdx   Block index from where data is to be
  * @param  BlocksNbr  Number of SD blocks to erase
  * @retval SD status
  */
int32_t BSP_SD_Erase(uint32_t Instance, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(HAL_SD_Erase(&hsd_sdmmc[Instance], BlockIdx, BlockIdx + BlocksNbr) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  /* Return BSP status   */
  return ret;
}

/**
  * @brief  Gets the current SD card data status.
  * @param  Instance  SD Instance
  * @retval Data transfer state.
  *          This value can be one of the following values:
  *            @arg  SD_TRANSFER_OK: No data transfer is acting
  *            @arg  SD_TRANSFER_BUSY: Data transfer is acting
  */
int32_t BSP_SD_GetCardState(uint32_t Instance)
{
  return (int32_t)((HAL_SD_GetCardState(&hsd_sdmmc[Instance]) == HAL_SD_CARD_TRANSFER ) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY);
}

/**
  * @brief  Get SD information about specific SD card.
  * @param  Instance  SD Instance
  * @param  CardInfo  Pointer to HAL_SD_CardInfoTypedef structure
  * @retval BSP status
  */
int32_t BSP_SD_GetCardInfo(uint32_t Instance, BSP_SD_CardInfo *CardInfo)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(HAL_SD_GetCardInfo(&hsd_sdmmc[Instance], CardInfo) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }
  /* Return BSP status */
  return ret;
}

#if !defined (USE_HAL_SD_REGISTER_CALLBACKS) || (USE_HAL_SD_REGISTER_CALLBACKS == 0)
/**
  * @brief SD Abort callbacks
  * @param hsd  SD handle
  * @retval None
  */
void HAL_SD_AbortCallback(SD_HandleTypeDef *hsd)
{
  BSP_SD_AbortCallback((hsd == &hsd_sdmmc[0]) ? 0UL : 1UL);
}

/**
  * @brief Tx Transfer completed callbacks
  * @param hsd  SD handle
  * @retval None
  */
void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
  BSP_SD_WriteCpltCallback((hsd == &hsd_sdmmc[0]) ? 0UL : 1UL);
}

/**
  * @brief Rx Transfer completed callbacks
  * @param hsd  SD handle
  * @retval None
  */
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
  BSP_SD_ReadCpltCallback((hsd == &hsd_sdmmc[0]) ? 0UL : 1UL);
}

#if (USE_SD_TRANSCEIVER != 0U)
/**
  * @brief  Enable the SD Transceiver 1.8V Mode Callback.
  */
void HAL_SD_DriveTransciver_1_8V_Callback(FlagStatus status)
{
#if (USE_BSP_IO_CLASS > 0U)
  if(status == SET)
  {
    BSP_IO_WritePin(0, SD_LDO_SEL_PIN, IO_PIN_SET);
  }
  else
  {
    BSP_IO_WritePin(0, SD_LDO_SEL_PIN, IO_PIN_RESET);
  }
#endif
}
#endif
#endif /* !defined (USE_HAL_SD_REGISTER_CALLBACKS) || (USE_HAL_SD_REGISTER_CALLBACKS == 0)   */

/**
  * @brief  This function handles pin detection interrupt request.
  * @param  Instance  SD Instance
  * @retval None
  */
void BSP_SD_DETECT_IRQHandler(uint32_t Instance)
{
  HAL_EXTI_IRQHandler(&hsd_exti[Instance]);
}

/**
  * @brief  This function handles SDMMC interrupt requests.
  * @param  Instance  SD Instance
  * @retval None
  */
void BSP_SD_IRQHandler(uint32_t Instance)
{
  HAL_SD_IRQHandler(&hsd_sdmmc[Instance]);
}

/**
  * @brief BSP SD Abort callbacks
  * @param  Instance     SD Instance
  * @retval None
  */
__weak void BSP_SD_AbortCallback(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning   */
  UNUSED(Instance);
}

/**
  * @brief BSP Tx Transfer completed callbacks
  * @param  Instance     SD Instance
  * @retval None
  */
__weak void BSP_SD_WriteCpltCallback(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning   */
  UNUSED(Instance);
}

/**
  * @brief BSP Rx Transfer completed callbacks
  * @param  Instance     SD Instance
  * @retval None
  */
__weak void BSP_SD_ReadCpltCallback(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning   */
  UNUSED(Instance);
}

/**
  * @}
  */

/** @defgroup STM32H747I_EVAL_SD_Private_Functions Private Functions
  * @{
  */
#if (USE_HAL_SD_REGISTER_CALLBACKS == 1)
/**
  * @brief SD Abort callbacks
  * @param hsd  SD handle
  * @retval None
  */
static void SD_AbortCallback(SD_HandleTypeDef *hsd)
{
  BSP_SD_AbortCallback((hsd == &hsd_sdmmc[0]) ? 0UL : 1UL);
}

/**
  * @brief Tx Transfer completed callbacks
  * @param hsd  SD handle
  * @retval None
  */
static void SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
  BSP_SD_WriteCpltCallback((hsd == &hsd_sdmmc[0]) ? 0UL : 1UL);
}

/**
  * @brief Rx Transfer completed callbacks
  * @param hsd  SD handle
  * @retval None
  */
static void SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
  BSP_SD_ReadCpltCallback((hsd == &hsd_sdmmc[0]) ? 0UL : 1UL);
}

#if (USE_SD_TRANSCEIVER != 0U)
/**
  * @brief  Enable the SD Transciver 1.8V Mode Callback.
  * @param  status Transceiver 1.8V status
  * @retval None
  */
static void SD_DriveTransceiver_1_8V_Callback(FlagStatus status)
{
#if (USE_BSP_IO_CLASS > 0U)
  if(status == SET)
  {
    BSP_IO_WritePin(0, SD_LDO_SEL_PIN, IO_PIN_SET);
  }
  else
  {
    BSP_IO_WritePin(0, SD_LDO_SEL_PIN, IO_PIN_RESET);
  }
#endif
}
#endif
#endif/* (USE_HAL_SD_REGISTER_CALLBACKS == 1) */

/**
  * @brief  SD EXTI line detection callbacks.
  * @retval None
  */
static void SD_EXTI_Callback(void)
{
#if (USE_BSP_IO_CLASS > 0)
  uint32_t sd_pin, sd_status = SD_PRESENT;
  BSP_IO_Init_t io_init_structure;

  sd_pin = (uint32_t)BSP_IO_GetIT(0, SD_DETECT_PIN);
  io_init_structure.Pin  = sd_pin;
  io_init_structure.Pull = IO_PULLUP;
  /* Check SD card detect pin   */
  if(((uint32_t)BSP_IO_ReadPin(0, sd_pin) & sd_pin) != sd_pin)
  {
    io_init_structure.Mode = IO_MODE_IT_RISING_EDGE;
    (void)BSP_IO_Init(0, &io_init_structure);
  }
  else
  {
    sd_status = SD_NOT_PRESENT;
    io_init_structure.Mode = IO_MODE_IT_FALLING_EDGE;
    (void)BSP_IO_Init(0, &io_init_structure);
  }
  if(sd_pin == SD_DETECT_PIN)
  {
    BSP_SD_DetectCallback(0, sd_status);
  }

  (void)BSP_IO_ClearIT(0, sd_pin);
#endif
}

/**
  * @brief  Initializes the SD MSP.
  * @param  hsd  SD handle
  * @retval None
  */
static void SD_MspInit(SD_HandleTypeDef *hsd)
{
  GPIO_InitTypeDef gpio_init_structure;

  if(hsd == &hsd_sdmmc[0])
  {
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

    /* GPIOB configuration */
    gpio_init_structure.Alternate = GPIO_AF7_SDIO1;
    gpio_init_structure.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    HAL_GPIO_Init(GPIOB, &gpio_init_structure);

    /* GPIOC configuration */
    gpio_init_structure.Alternate = GPIO_AF12_SDIO1;
    gpio_init_structure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
    HAL_GPIO_Init(GPIOC, &gpio_init_structure);

    /* GPIOC configuration */
    gpio_init_structure.Alternate = GPIO_AF8_SDIO1;
    gpio_init_structure.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    HAL_GPIO_Init(GPIOC, &gpio_init_structure);

    /* GPIOD configuration */
    gpio_init_structure.Alternate = GPIO_AF12_SDIO1;
    gpio_init_structure.Pin = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOD, &gpio_init_structure);

    /* NVIC configuration for SDIO interrupts */
    HAL_NVIC_SetPriority(SDMMC1_IRQn, BSP_SD_IT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(SDMMC1_IRQn);

  }
}

/**
  * @brief  DeInitializes the SD MSP.
  * @param  hsd  SD handle
  * @retval None
  */
static void SD_MspDeInit(SD_HandleTypeDef *hsd)
{
  GPIO_InitTypeDef gpio_init_structure;

  if(hsd == &hsd_sdmmc[0])
  {
    /* Disable NVIC for SDIO interrupts */
    HAL_NVIC_DisableIRQ(SDMMC1_IRQn);

    /* GPIOB configuration */
    gpio_init_structure.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    HAL_GPIO_DeInit(GPIOB, gpio_init_structure.Pin);

    /* GPIOC configuration */
    gpio_init_structure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
    HAL_GPIO_DeInit(GPIOC, gpio_init_structure.Pin);

    /* GPIOC configuration */
    gpio_init_structure.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    HAL_GPIO_DeInit(GPIOC, gpio_init_structure.Pin);

    /* GPIOD configuration */
    gpio_init_structure.Pin = GPIO_PIN_2;
    HAL_GPIO_DeInit(GPIOD, gpio_init_structure.Pin);

    /* Disable SDMMC1 clock */
    __HAL_RCC_SDMMC1_CLK_DISABLE();
  }
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
