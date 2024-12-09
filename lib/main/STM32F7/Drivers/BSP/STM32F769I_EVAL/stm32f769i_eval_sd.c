/**
  ******************************************************************************
  * @file    stm32f769i_eval_sd.c
  * @author  MCD Application Team
  * @brief   This file includes the uSD card driver mounted on STM32F769I-EVAL 
  *          evaluation boards.
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  @verbatim
  How To use this driver:
  -----------------------
   - This driver is used to drive the micro SD external cards mounted on STM32F769I-EVAL
     evaluation board.
   - This driver does not need a specific component driver for the micro SD device
     to be included with.

  Driver description:
  ------------------
  + Initialization steps:
     o Initialize the micro SD card using the BSP_SD_InitEx() function. This 
       function includes the MSP layer hardware resources initialization and the
       SDIO interface configuration to interface with the external micro SD. It 
       also includes the micro SD initialization sequence for SDCard1 or SDCard2.
       When BSP_SD_Init() is called, SDCard1 is by default initialized.
     o To check the SD card presence you can use the function BSP_SD_IsDetectedEx() which 
       returns the detection status for SDCard1 or SDCard2.
       the function BSP_SD_IsDetected() returns the detection status for SDCard1.
     o If SD presence detection interrupt mode is desired, you must configure the 
       SD detection interrupt mode by calling the functions BSP_SD_ITConfig() for
       SDCard1 or BSP_SD_ITConfigEx() for SDCard2 . The interrupt is generated as 
       an external interrupt whenever the micro SD card is plugged/unplugged
       in/from the evaluation board. The SD detection is managed by MFX, so the 
       SD detection interrupt has to be treated by MFX_IRQOUT gpio pin IRQ handler.
     o The function BSP_SD_GetCardInfo()/BSP_SD_GetCardInfoEx() are used to get 
       the micro SD card information which is stored in the structure 
       "HAL_SD_CardInfoTypedef".

  + Micro SD card operations
     o The micro SD card can be accessed with read/write block(s) operations once 
       it is ready for access. The access, by default to SDCard1, can be performed whether 
       using the polling mode by calling the functions BSP_SD_ReadBlocks()/BSP_SD_WriteBlocks(),  
       or by DMA transfer using the functions BSP_SD_ReadBlocks_DMA()/BSP_SD_WriteBlocks_DMA().
       The access can be performed to SDCard1 or SDCard2 by calling BSP_SD_ReadBlocksEx(),
       BSP_SD_WriteBlocksEx() or by calling BSP_SD_ReadBlocks_DMAEx()/BSP_SD_WriteBlocks_DMAEx().
     o The DMA transfer complete is used with interrupt mode. Once the SD transfer
       is complete, the SD interrupt is handled using the function BSP_SDMMC1_IRQHandler()
       when SDCard1 is used or BSP_SDMMC2_IRQHandler() when SDCard2 is used.
       The DMA Tx/Rx transfer complete are handled using the functions
       BSP_SDMMC1_DMA_Tx_IRQHandler(), BSP_SDMMC1_DMA_Rx_IRQHandler(), 
       BSP_SDMMC2_DMA_Tx_IRQHandler(), BSP_SDMMC2_DMA_Rx_IRQHandler(). The corresponding
       user callbacks are implemented by the user at application level. 
     o The SD erase block(s) is performed using the functions BSP_SD_Erase()/BSP_SD_EraseEx() 
       with specifying the number of blocks to erase.
     o The SD runtime status is returned when calling the function BSP_SD_GetCardState()
       BSP_SD_GetCardStateEx().

  @endverbatim
  ******************************************************************************
  */ 

/* Dependencies
- stm32f769i_eval.c
- stm32f7xx_hal_sd.c
- stm32f7xx_ll_sdmmc.c
- stm32f7xx_hal_dma.c  
- stm32f7xx_hal_gpio.c
- stm32f7xx_hal_cortex.c
- stm32f7xx_hal_rcc_ex.h
EndDependencies */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f769i_eval_sd.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32F769I_EVAL
  * @{
  */ 
  
/** @defgroup STM32F769I_EVAL_SD STM32F769I_EVAL SD
  * @{
  */ 


/** @defgroup STM32F769I_EVAL_SD_Private_TypesDefinitions SD Private TypesDefinitions
  * @{
  */
/**
  * @}
  */ 

/** @defgroup STM32F769I_EVAL_SD_Private_Defines SD Private Defines
  * @{
  */
/**
  * @}
  */ 
  
/** @defgroup STM32F769I_EVAL_SD_Private_Macros SD Private Macros
  * @{
  */    
/**
  * @}
  */  

/** @defgroup STM32F769I_EVAL_SD_Private_Variables SD Private Variables
  * @{
  */
SD_HandleTypeDef uSdHandle;
SD_HandleTypeDef uSdHandle2;
static uint8_t UseExtiModeDetection = 0;

/**
  * @}
  */ 
  
/** @defgroup STM32F769I_EVAL_SD_Private_Functions_Prototypes SD Private Functions Prototypes
  * @{
  */
/**
  * @}
  */ 
  
/** @defgroup STM32F769I_EVAL_SD_Private_Functions SD Private Functions
  * @{
  */

/**
  * @brief  Initializes the SD card device.
  * @retval SD status
  */
uint8_t BSP_SD_Init(void)
{ 
  /* By default, initialize SDMMC1 */
  return BSP_SD_InitEx(SD_CARD1);
}

/**
  * @brief  Initializes the SD card device.
  * @param  SdCard: SD card to be used, that should be SD_CARD1 or SD_CARD2 
  * @retval SD status
  */
uint8_t BSP_SD_InitEx(uint32_t SdCard)
{ 
  uint8_t sd_state = MSD_OK;
  
  /* uSD device interface configuration */
  if(SdCard == SD_CARD1)
  {  
    uSdHandle.Instance = SDMMC1;
    uSdHandle.Init.ClockEdge           = SDMMC_CLOCK_EDGE_RISING;
    uSdHandle.Init.ClockBypass         = SDMMC_CLOCK_BYPASS_DISABLE;
    uSdHandle.Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    uSdHandle.Init.BusWide             = SDMMC_BUS_WIDE_1B;
    uSdHandle.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    uSdHandle.Init.ClockDiv            = SDMMC_TRANSFER_CLK_DIV;
  }
  else
  {
    uSdHandle2.Instance = SDMMC2;
    uSdHandle2.Init.ClockEdge           = SDMMC_CLOCK_EDGE_RISING;
    uSdHandle2.Init.ClockBypass         = SDMMC_CLOCK_BYPASS_DISABLE;
    uSdHandle2.Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    uSdHandle2.Init.BusWide             = SDMMC_BUS_WIDE_1B;
    uSdHandle2.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    uSdHandle2.Init.ClockDiv            = SDMMC_TRANSFER_CLK_DIV;   
  }
  /* Initialize IO functionalities (MFX) used by SD detect pin */
  BSP_IO_Init(); 
  
  /* Check if the SD card is plugged in the slot */
  if(SdCard == SD_CARD1)
  {
    BSP_IO_ConfigPin(SD1_DETECT_PIN, IO_MODE_INPUT_PU);
    
    if(BSP_SD_IsDetected() != SD_PRESENT)
    {
      return MSD_ERROR_SD_NOT_PRESENT;
    }
    
    /* Msp SD initialization */
    BSP_SD_MspInit(&uSdHandle, NULL);
    
    /* HAL SD initialization */
    if(HAL_SD_Init(&uSdHandle) != HAL_OK)
    {
      sd_state = MSD_ERROR;
    }  
  }
  else
  {
    BSP_IO_ConfigPin(SD2_DETECT_PIN, IO_MODE_INPUT_PU);
    
    if(BSP_SD_IsDetectedEx(SD_CARD2) != SD_PRESENT)
    {
      return MSD_ERROR_SD_NOT_PRESENT;
    }
    /* Msp SD initialization */
    BSP_SD_MspInit(&uSdHandle2, NULL);
    
    /* HAL SD initialization */
    if(HAL_SD_Init(&uSdHandle2) != HAL_OK)
    {
      sd_state = MSD_ERROR;
    }    
  }
  
  
  /* Configure SD Bus width */
  if(sd_state == MSD_OK)
  {
    if(SdCard == SD_CARD1)
    {    
      /* Enable wide operation */
      sd_state = HAL_SD_ConfigWideBusOperation(&uSdHandle, SDMMC_BUS_WIDE_4B); 
    }
    else
    {
      /* Enable wide operation */    
      sd_state = HAL_SD_ConfigWideBusOperation(&uSdHandle2, SDMMC_BUS_WIDE_4B);
    }
    if(sd_state != HAL_OK)
    {
      sd_state = MSD_ERROR;
    }
    else
    {
      sd_state = MSD_OK;
    }
  }
  
  return  sd_state;
}


/**
  * @brief  DeInitializes the SD card device.
  * @retval SD status
  */
uint8_t BSP_SD_DeInit(void)
{
  /* By default, DeInitialize SDMMC1 */
  return BSP_SD_DeInitEx(SD_CARD1);
}

/**
  * @brief  DeInitializes the SD card device.
  * @param  SdCard: SD card to be used, that should be SD_CARD1 or SD_CARD2 
  * @retval SD status
  */
uint8_t BSP_SD_DeInitEx(uint32_t SdCard)
{ 
  uint8_t sd_state = MSD_OK;
  
  /* Set back Mfx pin to INPUT mode in case it was in exti */
  UseExtiModeDetection = 0;
  if(SdCard == SD_CARD1)
  {
    uSdHandle.Instance = SDMMC1;    
    /* HAL SD deinitialization */
    if(HAL_SD_DeInit(&uSdHandle) != HAL_OK)
    {
      sd_state = MSD_ERROR;
    }
    
    /* Msp SD deinitialization */
    BSP_SD_MspDeInit(&uSdHandle, NULL);    
    BSP_IO_ConfigPin(SD1_DETECT_PIN, IO_MODE_INPUT_PU);
  }
  else
  {
    uSdHandle2.Instance = SDMMC2;    
    BSP_IO_ConfigPin(SD2_DETECT_PIN, IO_MODE_INPUT_PU); 
    
    /* HAL SD deinitialization */
    if(HAL_SD_DeInit(&uSdHandle2) != HAL_OK)
    {
      sd_state = MSD_ERROR;
    }
    
    /* Msp SD deinitialization */
    BSP_SD_MspDeInit(&uSdHandle2, NULL);
  }  
  return  sd_state;
}

/**
  * @brief  Configures Interrupt mode for SD1 detection pin.
  * @retval Returns 0
  */
uint8_t BSP_SD_ITConfig(void)
{  
  /* Configure Interrupt mode for SD1 detection pin */  
  /* Note: disabling exti mode can be done calling BSP_SD_DeInit() */
  UseExtiModeDetection = 1;  
  BSP_SD_IsDetected();

  return 0;
}

/**
  * @brief  Configures Interrupt mode for SD detection pin.
  * @param  SdCard: SD card to be used, that should be SD_CARD1 or SD_CARD2
  * @retval Returns 0
  */
uint8_t BSP_SD_ITConfigEx(uint32_t SdCard)
{  
  /* Configure Interrupt mode for SD2 detection pin */  
  /* Note: disabling exti mode can be done calling BSP_SD_DeInitEx(SD_CARD2) */
  UseExtiModeDetection = 1;  
  BSP_SD_IsDetectedEx(SdCard);

  return 0;
}

/**
 * @brief  Detects if SD card is correctly plugged in the memory slot or not.
 * @retval Returns if SD is detected or not
 */
uint8_t BSP_SD_IsDetected(void)
{
  return BSP_SD_IsDetectedEx(SD_CARD1);
}

/**
 * @brief  Detects if SD card is correctly plugged in the memory slot or not.
 * @param  SdCard: SD card to be used, that should be SD_CARD1 or SD_CARD2 
 * @retval Returns if SD is detected or not
 */
uint8_t BSP_SD_IsDetectedEx(uint32_t SdCard)
{
  __IO uint8_t status = SD_PRESENT;
  
  if(SdCard == SD_CARD1)
  {
    /* Check SD card detect pin */
    if((BSP_IO_ReadPin(SD1_DETECT_PIN)&SD1_DETECT_PIN) != SD1_DETECT_PIN)
    {
      if (UseExtiModeDetection)
      {
        BSP_IO_ConfigPin(SD1_DETECT_PIN, IO_MODE_IT_RISING_EDGE_PU);
      }
    }
    else
    {
      status = SD_NOT_PRESENT;
      if (UseExtiModeDetection)
      {
        BSP_IO_ConfigPin(SD1_DETECT_PIN, IO_MODE_IT_FALLING_EDGE_PU);
      }
    }
  }
  else
  {
    /* Check SD card detect pin */
    if((BSP_IO_ReadPin(SD2_DETECT_PIN)&SD2_DETECT_PIN) != SD2_DETECT_PIN)
    {
      if (UseExtiModeDetection)
      {
        BSP_IO_ConfigPin(SD2_DETECT_PIN, IO_MODE_IT_RISING_EDGE_PU);
      }
    }
    else
    {
      status = SD_NOT_PRESENT;
      if (UseExtiModeDetection)
      {
        BSP_IO_ConfigPin(SD2_DETECT_PIN, IO_MODE_IT_FALLING_EDGE_PU);
      }
    }    
  }
  return status;
}

/**
  * @brief  Reads block(s) from a specified address in an SD card, in polling mode.
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  ReadAddr: Address from where data is to be read
  * @param  NumOfBlocks: Number of SD blocks to read
  * @param  Timeout: Timeout for read operation
  * @retval SD status
  */
uint8_t BSP_SD_ReadBlocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
  return BSP_SD_ReadBlocksEx(SD_CARD1, pData, ReadAddr, NumOfBlocks, Timeout);
}

/**
  * @brief  Reads block(s) from a specified address in an SD card, in polling mode.
  * @param  SdCard: SD card to be used, that should be SD_CARD1 or SD_CARD2 
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  ReadAddr: Address from where data is to be read  
  * @param  NumOfBlocks: Number of SD blocks to read
  * @param  Timeout: Timeout for read operation
  * @retval SD status
  */
uint8_t BSP_SD_ReadBlocksEx(uint32_t SdCard, uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
  HAL_StatusTypeDef  sd_state = HAL_OK;
  
  if(SdCard == SD_CARD1)
  {
    sd_state = HAL_SD_ReadBlocks(&uSdHandle, (uint8_t *)pData, ReadAddr, NumOfBlocks, Timeout);
  }
  else
  {
    sd_state = HAL_SD_ReadBlocks(&uSdHandle2, (uint8_t *)pData, ReadAddr, NumOfBlocks, Timeout);
  }
  
  if( sd_state == HAL_OK)
  {
    return MSD_OK;
  }
  else
  {
    return MSD_ERROR;
  }
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in polling mode. 
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  WriteAddr: Address from where data is to be written
  * @param  NumOfBlocks: Number of SD blocks to write
  * @param  Timeout: Timeout for write operation
  * @retval SD status
  */
uint8_t BSP_SD_WriteBlocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
    return BSP_SD_WriteBlocksEx(SD_CARD1, pData, WriteAddr, NumOfBlocks, Timeout);
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in polling mode.
  * @param  SdCard: SD card to be used, that should be SD_CARD1 or SD_CARD2
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  WriteAddr: Address from where data is to be written  
  * @param  NumOfBlocks: Number of SD blocks to write
  * @param  Timeout: Timeout for write operation
  * @retval SD status
  */
uint8_t BSP_SD_WriteBlocksEx(uint32_t SdCard, uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
  HAL_StatusTypeDef  sd_state = HAL_OK;
  
  if(SdCard == SD_CARD1)
  {
    sd_state = HAL_SD_WriteBlocks(&uSdHandle, (uint8_t *)pData, WriteAddr, NumOfBlocks, Timeout);
  }
  else
  {
    sd_state = HAL_SD_WriteBlocks(&uSdHandle2, (uint8_t *)pData, WriteAddr, NumOfBlocks, Timeout);
  }
  
  if( sd_state == HAL_OK)
  {
    return MSD_OK;
  }
  else
  {
    return MSD_ERROR;
  }  
}

/**
  * @brief  Reads block(s) from a specified address in an SD card, in DMA mode.
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  ReadAddr: Address from where data is to be read
  * @param  NumOfBlocks: Number of SD blocks to read 
  * @retval SD status
  */
uint8_t BSP_SD_ReadBlocks_DMA(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks)
{
  return BSP_SD_ReadBlocks_DMAEx(SD_CARD1, pData, ReadAddr, NumOfBlocks);
}

/**
  * @brief  Reads block(s) from a specified address in an SD card, in DMA mode.
  * @param  SdCard: SD card to be used, that should be SD_CARD1 or SD_CARD2
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  ReadAddr: Address from where data is to be read
  * @param  NumOfBlocks: Number of SD blocks to read 
  * @retval SD status
  */
uint8_t BSP_SD_ReadBlocks_DMAEx(uint32_t SdCard, uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks)
{
  HAL_StatusTypeDef  sd_state = HAL_OK;
  
  if(SdCard == SD_CARD1)
  {
    /* Read block(s) in DMA transfer mode */
    sd_state = HAL_SD_ReadBlocks_DMA(&uSdHandle, (uint8_t *)pData, ReadAddr, NumOfBlocks);
  }
  else
  {
    /* Read block(s) in DMA transfer mode */
    sd_state = HAL_SD_ReadBlocks_DMA(&uSdHandle2, (uint8_t *)pData, ReadAddr, NumOfBlocks);
  }
  
  if( sd_state == HAL_OK)
  {
    return MSD_OK;
  }
  else
  {
    return MSD_ERROR;
  }
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in DMA mode.
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  WriteAddr: Address from where data is to be written
  * @param  NumOfBlocks: Number of SD blocks to write 
  * @retval SD status
  */
uint8_t BSP_SD_WriteBlocks_DMA(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks)
{
  return BSP_SD_WriteBlocks_DMAEx(SD_CARD1, pData, WriteAddr, NumOfBlocks);
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in DMA mode.
  * @param  SdCard: SD card to be used, that should be SD_CARD1 or SD_CARD2
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  WriteAddr: Address from where data is to be written
  * @param  NumOfBlocks: Number of SD blocks to write 
  * @retval SD status
  */
uint8_t BSP_SD_WriteBlocks_DMAEx(uint32_t SdCard, uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks)
{
  HAL_StatusTypeDef  sd_state = HAL_OK;
  
  if(SdCard == SD_CARD1)
  {  
    /* Write block(s) in DMA transfer mode */
    sd_state = HAL_SD_WriteBlocks_DMA(&uSdHandle, (uint8_t *)pData, WriteAddr, NumOfBlocks);
  }
  else
  {
    /* Write block(s) in DMA transfer mode */ 
    sd_state = HAL_SD_WriteBlocks_DMA(&uSdHandle2, (uint8_t *)pData, WriteAddr, NumOfBlocks);
  }
  
  if( sd_state == HAL_OK)
  {
    return MSD_OK;
  }
  else
  {
    return MSD_ERROR;
  }
}

/**
  * @brief  Erases the specified memory area of the given SD card. 
  * @param  StartAddr: Start byte address
  * @param  EndAddr: End byte address
  * @retval SD status
  */
uint8_t BSP_SD_Erase(uint32_t StartAddr, uint32_t EndAddr)
{
  return BSP_SD_EraseEx(SD_CARD1, StartAddr, EndAddr);
}

/**
  * @brief  Erases the specified memory area of the given SD card. 
  * @param  SdCard: SD card to be used, that should be SD_CARD1 or SD_CARD2
  * @param  StartAddr: Start byte address
  * @param  EndAddr: End byte address
  * @retval SD status
  */
uint8_t BSP_SD_EraseEx(uint32_t SdCard, uint32_t StartAddr, uint32_t EndAddr)
{
  HAL_StatusTypeDef  sd_state = HAL_OK;
  
  if(SdCard == SD_CARD1)
  {
    sd_state = HAL_SD_Erase(&uSdHandle, StartAddr, EndAddr);
  }
  else
  {
    sd_state = HAL_SD_Erase(&uSdHandle2, StartAddr, EndAddr); 
  }

  if( sd_state == HAL_OK)
  {
    return MSD_OK;
  }
  else
  {
    return MSD_ERROR;
  } 
}

/**
  * @brief  Initializes the SD MSP.
  * @param  hsd: SD handle
  * @param  Params  
  * @retval None
  */
__weak void BSP_SD_MspInit(SD_HandleTypeDef *hsd, void *Params)
{
  static DMA_HandleTypeDef dma_rx_handle;
  static DMA_HandleTypeDef dma_tx_handle;
  static DMA_HandleTypeDef dma_rx_handle2;
  static DMA_HandleTypeDef dma_tx_handle2;  
  GPIO_InitTypeDef gpio_init_structure;
  
  /* Camera has to be powered down as some signals use same GPIOs between 
  * SD card and camera bus. Camera drives its signals to low impedance 
  * when powered ON. So the camera is powered off to let its signals
  * in high impedance */
  
  /* Camera power down sequence */
  BSP_IO_ConfigPin(RSTI_PIN, IO_MODE_OUTPUT);
  BSP_IO_ConfigPin(XSDN_PIN, IO_MODE_OUTPUT);
  /* De-assert the camera STANDBY pin (active high) */
  BSP_IO_WritePin(XSDN_PIN, BSP_IO_PIN_RESET);
  /* Assert the camera RSTI pin (active low) */
  BSP_IO_WritePin(RSTI_PIN, BSP_IO_PIN_RESET);
  if(hsd->Instance == SDMMC1)
  {
    /* Enable SDIO clock */
    __HAL_RCC_SDMMC1_CLK_ENABLE();
    
    /* Enable DMA2 clocks */
    __DMAx_TxRx_CLK_ENABLE();
    
    /* Enable GPIOs clock */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    
    /* Common GPIO configuration */
    gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
    gpio_init_structure.Pull      = GPIO_PULLUP;
    gpio_init_structure.Speed     = GPIO_SPEED_HIGH;
    gpio_init_structure.Alternate = GPIO_AF12_SDMMC1;
    
    /* GPIOC configuration: SD1_D0, SD1_D1, SD1_D2, SD1_D3 and SD1_CLK pins */
    gpio_init_structure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
    
    HAL_GPIO_Init(GPIOC, &gpio_init_structure);
    
    /* GPIOD configuration: SD1_CMD pin */
    gpio_init_structure.Pin = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOD, &gpio_init_structure);
    
    /* NVIC configuration for SDIO interrupts */
    HAL_NVIC_SetPriority(SDMMC1_IRQn, 0x0E, 0);
    HAL_NVIC_EnableIRQ(SDMMC1_IRQn);
  
    dma_rx_handle.Init.Channel             = SD1_DMAx_Rx_CHANNEL;
    dma_rx_handle.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    dma_rx_handle.Init.PeriphInc           = DMA_PINC_DISABLE;
    dma_rx_handle.Init.MemInc              = DMA_MINC_ENABLE;
    dma_rx_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    dma_rx_handle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    dma_rx_handle.Init.Mode                = DMA_PFCTRL;
    dma_rx_handle.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
    dma_rx_handle.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
    dma_rx_handle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    dma_rx_handle.Init.MemBurst            = DMA_MBURST_INC4;
    dma_rx_handle.Init.PeriphBurst         = DMA_PBURST_INC4;
    dma_rx_handle.Instance                 = SD1_DMAx_Rx_STREAM;
    
    /* Associate the DMA handle */
    __HAL_LINKDMA(hsd, hdmarx, dma_rx_handle);
    
    /* Deinitialize the stream for new transfer */
    HAL_DMA_DeInit(&dma_rx_handle);
    
    /* Configure the DMA stream */    
    HAL_DMA_Init(&dma_rx_handle);
         
    dma_tx_handle.Init.Channel             = SD1_DMAx_Tx_CHANNEL;
    dma_tx_handle.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    dma_tx_handle.Init.PeriphInc           = DMA_PINC_DISABLE;
    dma_tx_handle.Init.MemInc              = DMA_MINC_ENABLE;
    dma_tx_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    dma_tx_handle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    dma_tx_handle.Init.Mode                = DMA_PFCTRL;
    dma_tx_handle.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
    dma_tx_handle.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
    dma_tx_handle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    dma_tx_handle.Init.MemBurst            = DMA_MBURST_INC4;
    dma_tx_handle.Init.PeriphBurst         = DMA_PBURST_INC4;
    dma_tx_handle.Instance                 = SD1_DMAx_Tx_STREAM; 
    
    /* Associate the DMA handle */
    __HAL_LINKDMA(hsd, hdmatx, dma_tx_handle);
    
    /* Deinitialize the stream for new transfer */
    HAL_DMA_DeInit(&dma_tx_handle);
    
    /* Configure the DMA stream */
    HAL_DMA_Init(&dma_tx_handle);  

    /* NVIC configuration for DMA transfer complete interrupt */
    HAL_NVIC_SetPriority(SD1_DMAx_Rx_IRQn, 0x0F, 0);
    HAL_NVIC_EnableIRQ(SD1_DMAx_Rx_IRQn);
    
    /* NVIC configuration for DMA transfer complete interrupt */
    HAL_NVIC_SetPriority(SD1_DMAx_Tx_IRQn, 0x0F, 0);
    HAL_NVIC_EnableIRQ(SD1_DMAx_Tx_IRQn);  
  }
  else
  {
    /* Enable SDIO clock */
    __HAL_RCC_SDMMC2_CLK_ENABLE();
    
    /* Enable DMA2 clocks */
    __DMAx_TxRx_CLK_ENABLE();
    
    /* Enable GPIOs clock */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    
    /* Common GPIO configuration */
    gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
    gpio_init_structure.Pull      = GPIO_PULLUP;
    gpio_init_structure.Speed     = GPIO_SPEED_HIGH;
    gpio_init_structure.Alternate = GPIO_AF10_SDMMC2;
    
    /* GPIOB configuration: SD2_D2 and SD2_D3 pins */
    gpio_init_structure.Pin = GPIO_PIN_3 | GPIO_PIN_4;
    
    HAL_GPIO_Init(GPIOB, &gpio_init_structure);
    
    /* GPIOD configuration: SD2_CLK and SD2_CMD pins */
    gpio_init_structure.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gpio_init_structure.Alternate = GPIO_AF11_SDMMC2;
    HAL_GPIO_Init(GPIOD, &gpio_init_structure);
    
    /* GPIOG configuration: SD2_D0 and SD2_D1 pins */
    gpio_init_structure.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    
    HAL_GPIO_Init(GPIOG, &gpio_init_structure);
    
    /* NVIC configuration for SDIO interrupts */
    HAL_NVIC_SetPriority(SDMMC2_IRQn, 0x0E, 0);
    HAL_NVIC_EnableIRQ(SDMMC2_IRQn);
    
   
    dma_rx_handle2.Init.Channel             = SD2_DMAx_Rx_CHANNEL;
    dma_rx_handle2.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    dma_rx_handle2.Init.PeriphInc           = DMA_PINC_DISABLE;
    dma_rx_handle2.Init.MemInc              = DMA_MINC_ENABLE;
    dma_rx_handle2.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    dma_rx_handle2.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    dma_rx_handle2.Init.Mode                = DMA_PFCTRL;
    dma_rx_handle2.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
    dma_rx_handle2.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
    dma_rx_handle2.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    dma_rx_handle2.Init.MemBurst            = DMA_MBURST_INC16;
    dma_rx_handle2.Init.PeriphBurst         = DMA_PBURST_INC4;
    dma_rx_handle2.Instance                 = SD2_DMAx_Rx_STREAM;     
    
    /* Associate the DMA handle */
    __HAL_LINKDMA(hsd, hdmarx, dma_rx_handle2);
    
    /* Deinitialize the stream for new transfer */
    HAL_DMA_DeInit(&dma_rx_handle2);
    
    /* Configure the DMA stream */    
    HAL_DMA_Init(&dma_rx_handle2);

    dma_tx_handle2.Init.Channel             = SD2_DMAx_Tx_CHANNEL;
    dma_tx_handle2.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    dma_tx_handle2.Init.PeriphInc           = DMA_PINC_DISABLE;
    dma_tx_handle2.Init.MemInc              = DMA_MINC_ENABLE;
    dma_tx_handle2.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    dma_tx_handle2.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    dma_tx_handle2.Init.Mode                = DMA_PFCTRL;
    dma_tx_handle2.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
    dma_tx_handle2.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
    dma_tx_handle2.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    dma_tx_handle2.Init.MemBurst            = DMA_MBURST_INC16;
    dma_tx_handle2.Init.PeriphBurst         = DMA_PBURST_INC4;
    dma_tx_handle2.Instance                 = SD2_DMAx_Tx_STREAM;    
    
    /* Associate the DMA handle */
    __HAL_LINKDMA(hsd, hdmatx, dma_tx_handle2);
    
    /* Deinitialize the stream for new transfer */
    HAL_DMA_DeInit(&dma_tx_handle2);
    
    /* Configure the DMA stream */
    HAL_DMA_Init(&dma_tx_handle2);  
 
    /* NVIC configuration for DMA transfer complete interrupt */
    HAL_NVIC_SetPriority(SD2_DMAx_Rx_IRQn, 0x0F, 0);
    HAL_NVIC_EnableIRQ(SD2_DMAx_Rx_IRQn);
    
    /* NVIC configuration for DMA transfer complete interrupt */
    HAL_NVIC_SetPriority(SD2_DMAx_Tx_IRQn, 0x0F, 0);
    HAL_NVIC_EnableIRQ(SD2_DMAx_Tx_IRQn);     
  }
}

/**
  * @brief  DeInitializes the SD MSP.
  * @param  hsd: SD handle
  * @param  Params  
  * @retval None
  */
__weak void BSP_SD_MspDeInit(SD_HandleTypeDef *hsd, void *Params)
{
  static DMA_HandleTypeDef dma_rx_handle;
  static DMA_HandleTypeDef dma_tx_handle;
  static DMA_HandleTypeDef dma_rx_handle2;
  static DMA_HandleTypeDef dma_tx_handle2;
  
  if(hsd->Instance == SDMMC1)
  {
    /* Disable NVIC for DMA transfer complete interrupts */
    HAL_NVIC_DisableIRQ(SD1_DMAx_Rx_IRQn);
    HAL_NVIC_DisableIRQ(SD1_DMAx_Tx_IRQn);
    
    /* Deinitialize the stream for new transfer */
    dma_rx_handle.Instance = SD1_DMAx_Rx_STREAM;
    HAL_DMA_DeInit(&dma_rx_handle);
    
    /* Deinitialize the stream for new transfer */
    dma_tx_handle.Instance = SD1_DMAx_Tx_STREAM;
    HAL_DMA_DeInit(&dma_tx_handle);
    
    /* Disable NVIC for SDIO interrupts */
    HAL_NVIC_DisableIRQ(SDMMC1_IRQn);
    
    /* DeInit GPIO pins can be done in the application 
    (by surcharging this __weak function) */
    
    /* Disable SDMMC1 clock */
    __HAL_RCC_SDMMC1_CLK_DISABLE();
  }
  else
  {    /* Disable NVIC for DMA transfer complete interrupts */
    HAL_NVIC_DisableIRQ(SD2_DMAx_Rx_IRQn);
    HAL_NVIC_DisableIRQ(SD2_DMAx_Tx_IRQn);
    
    /* Deinitialize the stream for new transfer */
    dma_rx_handle2.Instance = SD2_DMAx_Rx_STREAM;
    HAL_DMA_DeInit(&dma_rx_handle2);
    
    /* Deinitialize the stream for new transfer */
    dma_tx_handle2.Instance = SD2_DMAx_Tx_STREAM;
    HAL_DMA_DeInit(&dma_tx_handle2);
    
    /* Disable NVIC for SDIO interrupts */
    HAL_NVIC_DisableIRQ(SDMMC2_IRQn);
    
    /* DeInit GPIO pins can be done in the application 
    (by surcharging this __weak function) */
    
    /* Disable SDMMC2 clock */
    __HAL_RCC_SDMMC2_CLK_DISABLE();
  }
  /* GPIO pins clock and DMA clocks can be shut down in the application
  by surcharging this __weak function */ 
}

/**
  * @brief  Gets the current SD card data status.
  * @retval Data transfer state.
  *          This value can be one of the following values:
  *            @arg  SD_TRANSFER_OK: No data transfer is acting
  *            @arg  SD_TRANSFER_BUSY: Data transfer is acting
  */
uint8_t BSP_SD_GetCardState(void)
{
  return BSP_SD_GetCardStateEx(SD_CARD1);
}

/**
  * @brief  Gets the current SD card data status.
  * @param  SdCard: SD_CARD1 or SD_CARD2
  * @retval Data transfer state.
  *          This value can be one of the following values:
  *            @arg  SD_TRANSFER_OK: No data transfer is acting
  *            @arg  SD_TRANSFER_BUSY: Data transfer is acting
  */
uint8_t BSP_SD_GetCardStateEx(uint32_t SdCard)
{
  if(SdCard == SD_CARD1)
  {
    return((HAL_SD_GetCardState(&uSdHandle) == HAL_SD_CARD_TRANSFER ) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY);
  }
  else
  {
    return((HAL_SD_GetCardState(&uSdHandle2) == HAL_SD_CARD_TRANSFER ) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY);
  }
}

/**
  * @brief  Get SD information about specific SD card.
  * @param  CardInfo: Pointer to HAL_SD_CardInfoTypedef structure
  * @retval None 
  */
void BSP_SD_GetCardInfo(BSP_SD_CardInfo *CardInfo)
{
  BSP_SD_GetCardInfoEx(SD_CARD1, CardInfo);
}

/**
  * @brief  Get SD information about specific SD card.
  * @param  SdCard: SD card to be used, that should be SD_CARD1 or SD_CARD2  
  * @param  CardInfo: Pointer to HAL_SD_CardInfoTypedef structure
  * @retval None 
  */
void BSP_SD_GetCardInfoEx(uint32_t SdCard, BSP_SD_CardInfo *CardInfo)
{
  /* Get SD card Information */
  if(SdCard == SD_CARD1)
  {
    HAL_SD_GetCardInfo(&uSdHandle, CardInfo);
  }
  else
  {
    HAL_SD_GetCardInfo(&uSdHandle2, CardInfo);
  }
}

/**
  * @brief SD Abort callbacks
  * @param hsd: SD handle
  * @retval None
  */
void HAL_SD_AbortCallback(SD_HandleTypeDef *hsd)
{
  BSP_SD_AbortCallback((hsd == &uSdHandle) ? SD_CARD1 : SD_CARD2);
}

/**
  * @brief Tx Transfer completed callbacks
  * @param hsd: SD handle
  * @retval None
  */
void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
  BSP_SD_WriteCpltCallback((hsd == &uSdHandle) ? SD_CARD1 : SD_CARD2);
}

/**
  * @brief Rx Transfer completed callbacks
  * @param hsd: SD handle
  * @retval None
  */
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
  BSP_SD_ReadCpltCallback((hsd == &uSdHandle) ? SD_CARD1 : SD_CARD2);
}

/**
  * @brief BSP SD Abort callbacks
  * @param SdCard: SD_CARD1 or SD_CARD2
  * @retval None
  */
__weak void BSP_SD_AbortCallback(uint32_t SdCard)
{

}

/**
  * @brief BSP Tx Transfer completed callbacks
  * @param SdCard: SD_CARD1 or SD_CARD2
  * @retval None
  */
__weak void BSP_SD_WriteCpltCallback(uint32_t SdCard)
{

}

/**
  * @brief BSP Rx Transfer completed callbacks
  * @param SdCard: SD_CARD1 or SD_CARD2
  * @retval None
  */
__weak void BSP_SD_ReadCpltCallback(uint32_t SdCard)
{

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
 
