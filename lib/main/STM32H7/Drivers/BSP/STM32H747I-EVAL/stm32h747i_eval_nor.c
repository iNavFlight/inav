/**
  ******************************************************************************
  * @file    stm32h747i_eval_nor.c
  * @author  MCD Application Team
  * @brief   This file includes a standard driver for the MT28EW128ABA1LPC-0SIT NOR flash memory
  *          device mounted on STM32H747I_EVAL boards.
  @verbatim
  How To use this driver:
  -----------------------
   - This driver is used to drive the MT28EW128ABA1LPC-0SIT NOR flash external memory mounted
     on STM32H747I_EVAL board.
   - This driver does not need a specific component driver for the NOR device
     to be included with.

  Driver description:
  ------------------
  + Initialization steps:
     o Initialize the NOR external memory using the BSP_NOR_Init() function. This
       function includes the MSP layer hardware resources initialization and the
       FMC controller configuration to interface with the external NOR memory.

  + NOR flash operations
     o NOR external memory can be accessed with read/write operations once it is
       initialized.
       Read/write operation can be performed with AHB access using the functions
       BSP_NOR_ReadData()/BSP_NOR_WriteData(). The BSP_NOR_WriteData() performs write operation
       of an amount of data by unit (halfword). You can also perform a program data
       operation of an amount of data using the function BSP_NOR_ProgramData().
     o The function BSP_NOR_Read_ID() returns the chip IDs stored in the structure
       "NOR_IDTypeDef". (see the NOR IDs in the memory data sheet)
     o Perform erase block operation using the function BSP_NOR_Erase_Block() and by
       specifying the block address. You can perform an erase operation of the whole
       chip by calling the function BSP_NOR_Erase_Chip().
     o After other operations, the function BSP_NOR_ReturnToReadMode() allows the NOR
       flash to return to read mode to perform read operations on it.
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
#include "stm32h747i_eval_nor.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H747I_EVAL
  * @{
  */

/** @defgroup STM32H747I_EVAL_NOR NOR
  * @{
  */

/** @defgroup STM32H747I_EVAL_NOR_Exported_Variables Exported Variables
  * @{
  */
NOR_HandleTypeDef hnor[NOR_INSTANCES_NBR];
/**
  * @}
  */

/** @defgroup STM32H747I_EVAL_NOR_Private_Variables Private Variables
  * @{
  */

#if (USE_HAL_NOR_REGISTER_CALLBACKS == 1)
static uint32_t IsMspCallbacksValid = 0;
#endif

/**
  * @}
  */

/** @defgroup STM32H747I_EVAL_NOR_Private_Functions_Prototypes Private Functions Prototypes
  * @{
  */
static void NOR_MspInit(NOR_HandleTypeDef  *hnor);
static void NOR_MspDeInit(NOR_HandleTypeDef  *hnor);
static void NOR_MspWait(NOR_HandleTypeDef *hnor, uint32_t Timeout);
static int32_t NOR_GetStatus(uint32_t Instance, uint32_t Address, uint32_t Timeout, uint32_t ProgramTimeout);
/**
  * @}
  */

/** @defgroup STM32H747I_EVAL_NOR_Private_Functions Private Functions
  * @{
  */

/**
  * @brief  Initializes the NOR device.
  * @param  Instance  NOR Instance
  * @retval BSP status
  */
int32_t BSP_NOR_Init(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= NOR_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_HAL_NOR_REGISTER_CALLBACKS == 1)
    /* Register the NOR MSP Callbacks */
    if(IsMspCallbacksValid == 0UL)
    {
      if(BSP_NOR_RegisterDefaultMspCallbacks(Instance) != BSP_ERROR_NONE)
      {
        return BSP_ERROR_PERIPH_FAILURE;
      }
      else
      {
        /* Do nothing */
      }
    }
    else
    {
      /* Msp callbacks already registered */
    }
#else
    /* Msp NOR initialization */
    NOR_MspInit(&hnor[Instance]);
#endif /* USE_HAL_NOR_REGISTER_CALLBACKS */

    /* __weak function can be rewritten by the application */
    if(MX_NOR_Init(&hnor[Instance]) != HAL_OK)
    {
      ret = BSP_ERROR_NO_INIT;
    }
    else
    {
      ret = BSP_ERROR_NONE;
    }
  }

  return ret;
}

/**
  * @brief  DeInitializes the NOR device.
  * @param  Instance  NOR Instance
  * @retval BSP status
  */
int32_t BSP_NOR_DeInit(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= NOR_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(HAL_NOR_DeInit(&hnor[Instance]) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
#if (USE_HAL_NOR_REGISTER_CALLBACKS == 0)
      /* NOR controller de-initialization */
      NOR_MspDeInit(&hnor[Instance]);
#endif /* (USE_HAL_NOR_REGISTER_CALLBACKS == 0) */

    }
  }

  return ret;
}

/**
  * @brief  Initializes the NOR periperal.
  * @param  hnor NOR handle
  * @retval HAL status
  */
__weak HAL_StatusTypeDef MX_NOR_Init(NOR_HandleTypeDef *hnor)
{
  static FMC_NORSRAM_TimingTypeDef Timing;

  hnor->Instance  = FMC_NORSRAM_DEVICE;
  hnor->Extended  = FMC_NORSRAM_EXTENDED_DEVICE;

  /* NOR device configuration */
  hnor->Init.NSBank             = FMC_NORSRAM_BANK1;
  hnor->Init.DataAddressMux     = FMC_DATA_ADDRESS_MUX_DISABLE;
  hnor->Init.MemoryType         = FMC_MEMORY_TYPE_NOR;
  hnor->Init.MemoryDataWidth    = FMC_NORSRAM_MEM_BUS_WIDTH_16;
  hnor->Init.BurstAccessMode    = FMC_BURST_ACCESS_MODE_DISABLE;
  hnor->Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
  hnor->Init.WaitSignalActive   = FMC_WAIT_TIMING_BEFORE_WS;
  hnor->Init.WriteOperation     = FMC_WRITE_OPERATION_ENABLE;
  hnor->Init.WaitSignal         = FMC_WAIT_SIGNAL_ENABLE;
  hnor->Init.ExtendedMode       = FMC_EXTENDED_MODE_DISABLE;
  hnor->Init.AsynchronousWait   = FMC_ASYNCHRONOUS_WAIT_ENABLE;
  hnor->Init.WriteBurst         = FMC_WRITE_BURST_DISABLE;
  hnor->Init.ContinuousClock    = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;

  /* Timing configuration derived from system clock (up to 216Mhz)
     for 108Mhz as NOR clock frequency */
  Timing.AddressSetupTime      = 9;
  Timing.AddressHoldTime       = 1;
  Timing.DataSetupTime         = 5;
  Timing.BusTurnAroundDuration = 4;
  Timing.CLKDivision           = 4;
  Timing.DataLatency           = 2;
  Timing.AccessMode            = FMC_ACCESS_MODE_B;

  /* NOR controller initialization */
  if(HAL_NOR_Init(hnor, &Timing, &Timing) != HAL_OK)
  {
    return  HAL_ERROR;
  }
  return HAL_OK;
}

#if (USE_HAL_NOR_REGISTER_CALLBACKS == 1)
/**
  * @brief Default BSP NOR Msp Callbacks
  * @param  Instance    NOR instance
  * @retval BSP status
  */
int32_t BSP_NOR_RegisterDefaultMspCallbacks(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);

  __HAL_NOR_RESET_HANDLE_STATE(&hnor[Instance]);

  /* Register default MspInit/MspDeInit Callbacks */
  if(HAL_NOR_RegisterCallback(&hnor[Instance], HAL_NOR_MSP_INIT_CB_ID, NOR_MspInit) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else if(HAL_NOR_RegisterCallback(&hnor[Instance], HAL_NOR_MSP_DEINIT_CB_ID, NOR_MspDeInit)!= HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    IsMspCallbacksValid = 1UL;
  }

  return ret;
}

/**
  * @brief BSP NOR Msp Callback registering
  * @param Instance    NOR instance
  * @param Callbacks   pointer to MspInit/MspDeInit callback functions
  * @retval BSP status
  */
int32_t BSP_NOR_RegisterMspCallbacks(uint32_t Instance, BSP_NOR_Cb_t *Callbacks)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);

  __HAL_NOR_RESET_HANDLE_STATE(&hnor[Instance]);

  /* Register MspInit/MspDeInit Callbacks */
  if(HAL_NOR_RegisterCallback(&hnor[Instance], HAL_NOR_MSP_INIT_CB_ID, Callbacks->pMspInitCb)!= HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else if(HAL_NOR_RegisterCallback(&hnor[Instance], HAL_NOR_MSP_DEINIT_CB_ID, Callbacks->pMspDeInitCb)!= HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    IsMspCallbacksValid = 1UL;
  }

  return ret;
}
#endif /* (USE_HAL_NOR_REGISTER_CALLBACKS == 1) */
/**
  * @brief  Reads an amount of data from the NOR device.
  * @param  Instance  NOR Instance
  * @param  uwStartAddress  Read start address
  * @param  pData  Pointer to data to be read
  * @param  uwDataSize  Size of data to read
  * @retval BSP status
  */
int32_t BSP_NOR_ReadData(uint32_t Instance, uint32_t uwStartAddress, uint16_t* pData, uint32_t uwDataSize)
{
  int32_t ret;

  if(Instance >= NOR_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }
  else if(HAL_NOR_ReadBuffer(&hnor[Instance], NOR_DEVICE_ADDR + uwStartAddress, pData, uwDataSize) != HAL_OK)
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
  * @brief  Returns the NOR memory to read mode.
  * @param  Instance  NOR Instance
  * @retval BSP status
  */
int32_t BSP_NOR_ReturnToReadMode(uint32_t Instance)
{
  int32_t ret;

  if(Instance >= NOR_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }
  else if(HAL_NOR_ReturnToReadMode(&hnor[Instance]) != HAL_OK)
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
  * @brief  Writes an amount of data to the NOR device.
  * @param  Instance  NOR Instance
  * @param  uwStartAddress  Write start address
  * @param  pData  Pointer to data to be written
  * @param  uwDataSize  Size of data to write
  * @retval BSP status
  */
int32_t BSP_NOR_WriteData(uint32_t Instance, uint32_t uwStartAddress, uint16_t* pData, uint32_t uwDataSize)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t index = uwDataSize;
  uint32_t write_addr;
  uint16_t* write_data;

  if(Instance >= NOR_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    write_data = pData;
    write_addr = uwStartAddress;
    while((index > 0UL) && (ret == BSP_ERROR_NONE))
    {
      /* Write data to NOR */
      if(HAL_NOR_Program(&hnor[Instance], (uint32_t *)(NOR_DEVICE_ADDR + write_addr), write_data) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else if(NOR_GetStatus(Instance, NOR_DEVICE_ADDR, PROGRAM_TIMEOUT, PROGRAM_TIMEOUT) != BSP_ERROR_NONE)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else
      {
        /* Update the counters */
        index--;
        write_addr += 2UL;
        write_data++;
      }
    }
  }

  return ret;
}

/**
  * @brief  Programs an amount of data to the NOR device.
  * @param  Instance  NOR Instance
  * @param  uwStartAddress  Write start address
  * @param  pData  Pointer to data to be written
  * @param  uwDataSize  Size of data to write
  * @retval BSP status
  */
int32_t BSP_NOR_ProgramData(uint32_t Instance, uint32_t uwStartAddress, uint16_t* pData, uint32_t uwDataSize)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= NOR_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }/* Send NOR program buffer operation */
  else if(HAL_NOR_ProgramBuffer(&hnor[Instance], uwStartAddress, pData, uwDataSize) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    if(NOR_GetStatus(Instance, NOR_DEVICE_ADDR, PROGRAM_TIMEOUT, PROGRAM_TIMEOUT) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  return ret;
}

/**
  * @brief  Erases the specified block of the NOR device.
  * @param  Instance  NOR Instance
  * @param  BlockAddress  Block address to erase
  * @retval BSP status
  */
int32_t BSP_NOR_EraseBlock(uint32_t Instance, uint32_t BlockAddress)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= NOR_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }/* Send NOR erase block operation */
  else if(HAL_NOR_Erase_Block(&hnor[Instance], BlockAddress, NOR_DEVICE_ADDR) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    if(NOR_GetStatus(Instance, NOR_DEVICE_ADDR, PROGRAM_TIMEOUT, BLOCKERASE_TIMEOUT) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  return ret;
}

/**
  * @brief  Erases the entire NOR chip.
  * @param  Instance  NOR Instance
  * @retval BSP status
  */
int32_t BSP_NOR_EraseChip(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= NOR_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }/* Send NOR Erase chip operation */
  else if(HAL_NOR_Erase_Chip(&hnor[Instance], NOR_DEVICE_ADDR) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    if(NOR_GetStatus(Instance, NOR_DEVICE_ADDR, PROGRAM_TIMEOUT, CHIPERASE_TIMEOUT) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  return ret;
}

/**
  * @brief  Reads NOR flash IDs.
  * @param  Instance  NOR Instance
  * @param  pNOR_ID   Pointer to NOR ID structure
  * @retval BSP status
  */
int32_t BSP_NOR_ReadID(uint32_t Instance, NOR_IDTypeDef *pNOR_ID)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= NOR_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(HAL_NOR_Read_ID(&hnor[Instance], pNOR_ID) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  return ret;
}
/**
  * @}
  */

/** @addtogroup STM32H747I_EVAL_NOR_Private_Functions_Prototypes
  * @{
  */

/**
  * @brief  Initializes the NOR MSP.
  * @param  hnor NOR handle
  * @retval None
  */
static void NOR_MspInit(NOR_HandleTypeDef *hnor)
{
  GPIO_InitTypeDef gpio_init_structure;
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hnor);

  /* Enable FMC clock */
  __HAL_RCC_FMC_CLK_ENABLE();
  /* Enable GPIOs clock */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /* Common GPIO configuration */
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;

  /* GPIOD configuration */
  gpio_init_structure.Alternate = GPIO_AF12_FMC;
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5| GPIO_PIN_6 | GPIO_PIN_7|\
                              GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 |\
                              GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOD, &gpio_init_structure);

  /* GPIOE configuration */
  gpio_init_structure.Alternate = GPIO_AF12_FMC;
  gpio_init_structure.Pin   = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 |GPIO_PIN_6| GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 |\
                              GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 |\
                              GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOE, &gpio_init_structure);

  /* GPIOF configuration */
  gpio_init_structure.Alternate = GPIO_AF12_FMC;
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 | GPIO_PIN_4     |\
                              GPIO_PIN_5 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOF, &gpio_init_structure);

  /* GPIOG configuration */
  gpio_init_structure.Alternate = GPIO_AF12_FMC;
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 | GPIO_PIN_4     |\
                              GPIO_PIN_5;
  HAL_GPIO_Init(GPIOG, &gpio_init_structure);
}

/**
  * @brief  DeInitializes NOR MSP.
  * @param  hnor NOR handle
  * @retval None
  */
static void NOR_MspDeInit(NOR_HandleTypeDef  *hnor)
{
  GPIO_InitTypeDef gpio_init_structure;
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hnor);

  /* GPIOC configuration */
  gpio_init_structure.Pin   = GPIO_PIN_6 | GPIO_PIN_7;
  HAL_GPIO_DeInit(GPIOC, gpio_init_structure.Pin);

  /* GPIOD configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5|\
                              GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 |\
                              GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_DeInit(GPIOD, gpio_init_structure.Pin);

  /* GPIOE configuration */
  gpio_init_structure.Pin   = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 |\
                              GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 |\
                              GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_DeInit(GPIOE, gpio_init_structure.Pin);

  /* GPIOF configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 | GPIO_PIN_4     |\
                              GPIO_PIN_5 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_DeInit(GPIOF, gpio_init_structure.Pin);

  /* GPIOG configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 | GPIO_PIN_4     |\
                              GPIO_PIN_5;
  HAL_GPIO_DeInit(GPIOG, gpio_init_structure.Pin);

    /* Disable FMC clock */
  __HAL_RCC_FMC_CLK_DISABLE();
}

/**
  * @brief  NOR BSP Wait for Ready/Busy signal.
  * @param  hnor     Pointer to NOR handle
  * @param  Timeout  Timeout duration
  * @retval None
  */
static void NOR_MspWait(NOR_HandleTypeDef *hnor, uint32_t Timeout)
{
  uint32_t timeout = Timeout;
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hnor);

  /* Polling on Ready/Busy signal */
  while((HAL_GPIO_ReadPin(NOR_READY_BUSY_GPIO, NOR_READY_BUSY_PIN) != NOR_BUSY_STATE) && (timeout > 0U))
  {
    timeout--;
  }

  timeout = Timeout;

  /* Polling on Ready/Busy signal */
  while((HAL_GPIO_ReadPin(NOR_READY_BUSY_GPIO, NOR_READY_BUSY_PIN) != NOR_READY_STATE) && (timeout > 0U))
  {
    timeout--;
  }
}

/**
  * @brief  Returns the NOR operation status.
  * @param  Instance  NOR Instance
  * @param  Address Device address
  * @param  Timeout NOR Timeout for Ready/Busy signal
  * @param  ProgramTimeout NOR programming Timeout
  * @retval BSP status
  */
static int32_t NOR_GetStatus(uint32_t Instance, uint32_t Address, uint32_t Timeout, uint32_t ProgramTimeout)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= NOR_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* NOR BSP Wait for Ready/Busy signal */
    NOR_MspWait(&hnor[Instance], Timeout);

    /* Return the NOR memory status */
    if(HAL_NOR_GetStatus(&hnor[Instance], Address, ProgramTimeout) != HAL_NOR_STATUS_SUCCESS)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  return ret;
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
