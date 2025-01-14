/**
  ******************************************************************************
  * @file    stm32h747i_discovery_qspi.c
  * @author  MCD Application Team
  * @brief   This file includes a standard driver for the MT25TL01G QSPI
  *          memory mounted on STM32H747I-DISCO board.
    @verbatim
  How To use this driver:
  -----------------------
   - This driver is used to drive the MT25TL01G QSPI external
       memory mounted on STM32H747I-DISCO board.

   - This driver need a specific component driver (MT25TL01G) to be included with.

  Driver description:
  -------------------
   - Initialization steps:
       + Initialize the QPSI external memory using the BSP_QSPI_Init() function. This
            function includes the MSP layer hardware resources initialization and the
            QSPI interface with the external memory.
         STR and DTR transfer rates are supported.
         SPI, SPI 2-IO, SPI-4IO and QPI modes are supported

   - QSPI memory operations
       + QSPI memory can be accessed with read/write operations once it is
            initialized.
            Read/write operation can be performed with AHB access using the functions
            BSP_QSPI_Read()/BSP_QSPI_Write().
       + The function BSP_QSPI_GetInfo() returns the configuration of the QSPI memory.
            (see the QSPI memory data sheet)
       + Perform erase block operation using the function BSP_QSPI_EraseBlock() and by
            specifying the block address. You can perform an erase operation of the whole
            chip by calling the function BSP_QSPI_EraseChip().
       + The function BSP_QSPI_GetStatus() returns the current status of the QSPI memory.
            (see the QSPI memory data sheet)
       + The function BSP_QSPI_EnableMemoryMappedMode enables the QSPI memory mapped mode
       + The function BSP_QSPI_DisableMemoryMappedMode disables the QSPI memory mapped mode
       + The function BSP_QSPI_ConfigFlash() allow to configure the QSPI mode and transfer rate

  Note:
  --------
    Regarding the "Instance" parameter, needed for all functions, it is used to select
    an QSPI instance. On the STM32H747I_DISCO board, there's one instance. Then, this
    parameter should be 0.

  @endverbatim
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

/* Includes ------------------------------------------------------------------*/
#include "stm32h747i_discovery_qspi.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H747I_DISCO
  * @{
  */

/** @defgroup STM32H747I_DISCO_QSPI QSPI
  * @{
  */

/* Private variables ---------------------------------------------------------*/

/** @defgroup STM32H747I_DISCO_QSPI_Exported_Variables Exported Variables
  * @{
  */
QSPI_HandleTypeDef hqspi;
BSP_QSPI_Ctx_t     QSPI_Ctx[QSPI_INSTANCES_NUMBER];
/**
  * @}
  */

/* Private functions ---------------------------------------------------------*/

/** @defgroup STM32H747I_DISCO_QSPI_Private_Functions Private Functions
  * @{
  */
static void QSPI_MspInit(QSPI_HandleTypeDef *hQspi);
static void QSPI_MspDeInit(QSPI_HandleTypeDef *hSspi);
static int32_t QSPI_ResetMemory(uint32_t Instance);
static int32_t QSPI_DummyCyclesCfg(uint32_t Instance);

/**
  * @}
  */

/** @defgroup STM32H747I_DISCO_QSPI_Exported_Functions Exported Functions
  * @{
  */

/**
  * @brief  Initializes the QSPI interface.
  *         Instance management :
  *         Instance can running at Indirect or MMP mode.
  *         Every indirect instance can switch between.
  *         Once MMP instance active then Instance switch function disabled.
  *         Only 1 MMP instance can run.
  *         Instance switch function enabled after MMP instance exit to Indirect mode.
  *
  *         MT25TL01G action :
  *         QE(Quad Enable, Non-volatile) bit of Status Register
  *         QE = 0; WP# & RESET# pin active
  *                 Accept 1-1-1, 1-1-2, 1-2-2 commands
  *         QE = 1; WP# become SIO2 pin, RESET# become SIO3 pin
  *                 Accept 1-1-1, 1-1-2, 1-2-2, 1-1-4, 1-4-4 commands
  *         Enter QPI mode by issue EQIO(0x35) command from 1-1-1 mode
  *                 Accept 4-4-4 commands
  *         Exit QPI mode by issue RSTQIO(0xF5) command from 4-4-4 mode
  *                 Accept commands, dependent QE bit status
  *         Memory Read commands support STR(Single Transfer Rate) & DTR(Double Transfer Rate) mode
  *
  *         Force QE = 1, Enter 4-Byte address mode
  *         Configure Dummy cycle & ODS(Out Driver Strength)
  *         STR/DTR effect read commands only
  * @param  Instance   QSPI Instance
  * @param  Init       QSPI Init structure
  * @retval BSP status
  */
int32_t BSP_QSPI_Init(uint32_t Instance, BSP_QSPI_Init_t *Init)
{
  int32_t ret = BSP_ERROR_NONE;
  BSP_QSPI_Info_t pInfo;
  MX_QSPI_Init_t qspi_init;
  /* Table to handle clock prescalers:
  1: For STR mode to reach max 108Mhz
  3: For DTR mode to reach max 54Mhz
  */
  static const uint32_t PrescalerTab[2] = {1, 3};

  /* Check if the instance is supported */
  if(Instance >= QSPI_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Check if instance is already initialized */
    if(QSPI_Ctx[Instance].IsInitialized == QSPI_ACCESS_NONE)
    {
#if (USE_HAL_QSPI_REGISTER_CALLBACKS == 1)
      /* Register the QSPI MSP Callbacks */
      if(QSPI_Ctx[Instance].IsMspCallbacksValid == 0UL)
      {
        if(BSP_QSPI_RegisterDefaultMspCallbacks(Instance) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
      }
#else
      /* Msp QSPI initialization */
      QSPI_MspInit(&hqspi);
#endif /* USE_HAL_QSPI_REGISTER_CALLBACKS */

      if(ret == BSP_ERROR_NONE)
      {
        /* STM32 QSPI interface initialization */
        (void)MT25TL01G_GetFlashInfo(&pInfo);
        qspi_init.ClockPrescaler = PrescalerTab[Init->TransferRate];
        qspi_init.DualFlashMode  = QSPI_DUALFLASH_ENABLE;
        qspi_init.FlashSize      = (uint32_t)POSITION_VAL((uint32_t)pInfo.FlashSize) - 1U;
        qspi_init.SampleShifting = (Init->TransferRate == BSP_QSPI_STR_TRANSFER) ? QSPI_SAMPLE_SHIFTING_HALFCYCLE : QSPI_SAMPLE_SHIFTING_NONE;

        if(MX_QSPI_Init(&hqspi, &qspi_init) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }/* QSPI memory reset */
        else if(QSPI_ResetMemory(Instance) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }/* Force Flash enter 4 Byte address mode */
        else if(MT25TL01G_AutoPollingMemReady(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != MT25TL01G_OK)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }
        else if(MT25TL01G_Enter4BytesAddressMode(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != MT25TL01G_OK)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }/* Configuration of the dummy cycles on QSPI memory side */
        else if(QSPI_DummyCyclesCfg(Instance) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }
        else
        {
          /* Configure Flash to desired mode */
          if(BSP_QSPI_ConfigFlash(Instance, Init->InterfaceMode, Init->TransferRate) != BSP_ERROR_NONE)
          {
            ret = BSP_ERROR_COMPONENT_FAILURE;
          }
        }
      }
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  De-Initializes the QSPI interface.
  * @param  Instance   QSPI Instance
  * @retval BSP status
  */
int32_t BSP_QSPI_DeInit(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if(Instance >= QSPI_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(QSPI_Ctx[Instance].IsInitialized == QSPI_ACCESS_MMP)
    {
      if(BSP_QSPI_DisableMemoryMappedMode(Instance) != BSP_ERROR_NONE)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
    }

    if(ret == BSP_ERROR_NONE)
    {
      /* Set default QSPI_Ctx values */
      QSPI_Ctx[Instance].IsInitialized = QSPI_ACCESS_NONE;
      QSPI_Ctx[Instance].InterfaceMode = BSP_QSPI_SPI_MODE;
      QSPI_Ctx[Instance].TransferRate  = BSP_QSPI_STR_TRANSFER;
      QSPI_Ctx[Instance].DualFlashMode = QSPI_DUALFLASH_ENABLE;

#if (USE_HAL_QSPI_REGISTER_CALLBACKS == 0)
      QSPI_MspDeInit(&hqspi);
#endif /* (USE_HAL_QSPI_REGISTER_CALLBACKS == 0) */

      /* Call the DeInit function to reset the driver */
      if (HAL_QSPI_DeInit(&hqspi) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Initializes the QSPI interface.
  * @param  hQspi       QSPI handle
  * @param  Config      QSPI configuration structure
  * @retval BSP status
  */
__weak HAL_StatusTypeDef MX_QSPI_Init(QSPI_HandleTypeDef *hQspi, MX_QSPI_Init_t *Config)
{
  /* QSPI initialization */
  /* QSPI freq = SYSCLK /(1 + ClockPrescaler) Mhz */
  hQspi->Instance                = QUADSPI;
  hQspi->Init.ClockPrescaler     = Config->ClockPrescaler;
  hQspi->Init.FifoThreshold      = 1;
  hQspi->Init.SampleShifting     = Config->SampleShifting;
  hQspi->Init.FlashSize          = Config->FlashSize;
  hQspi->Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_4_CYCLE; /* Min 50ns for nonRead */
  hQspi->Init.ClockMode          = QSPI_CLOCK_MODE_0;
  hQspi->Init.FlashID            = QSPI_FLASH_ID_1;
  hQspi->Init.DualFlash          = Config->DualFlashMode;

  return HAL_QSPI_Init(hQspi);
}

#if (USE_HAL_QSPI_REGISTER_CALLBACKS == 1)
/**
  * @brief Default BSP QSPI Msp Callbacks
  * @param Instance      QSPI Instance
  * @retval BSP status
  */
int32_t BSP_QSPI_RegisterDefaultMspCallbacks (uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if(Instance >= QSPI_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Register MspInit/MspDeInit Callbacks */
    if(HAL_QSPI_RegisterCallback(&hqspi, HAL_QSPI_MSPINIT_CB_ID, QSPI_MspInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(HAL_QSPI_RegisterCallback(&hqspi, HAL_QSPI_MSPDEINIT_CB_ID, QSPI_MspDeInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      QSPI_Ctx[Instance].IsMspCallbacksValid = 1U;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief BSP QSPI Msp Callback registering
  * @param Instance     QSPI Instance
  * @param CallBacks    pointer to MspInit/MspDeInit callbacks functions
  * @retval BSP status
  */
int32_t BSP_QSPI_RegisterMspCallbacks (uint32_t Instance, BSP_QSPI_Cb_t *CallBacks)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if(Instance >= QSPI_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Register MspInit/MspDeInit Callbacks */
    if(HAL_QSPI_RegisterCallback(&hqspi, HAL_QSPI_MSPINIT_CB_ID, CallBacks->pMspInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(HAL_QSPI_RegisterCallback(&hqspi, HAL_QSPI_MSPDEINIT_CB_ID, CallBacks->pMspDeInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      QSPI_Ctx[Instance].IsMspCallbacksValid = 1U;
    }
  }

  /* Return BSP status */
  return ret;
}
#endif /* (USE_HAL_QSPI_REGISTER_CALLBACKS == 1) */

/**
  * @brief  Reads an amount of data from the QSPI memory.
  * @param  Instance  QSPI instance
  * @param  pData     Pointer to data to be read
  * @param  ReadAddr  Read start address
  * @param  Size      Size of data to read
  * @retval BSP status
  */
int32_t BSP_QSPI_Read(uint32_t Instance, uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if(Instance >= QSPI_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(QSPI_Ctx[Instance].TransferRate == BSP_QSPI_STR_TRANSFER)
    {
      if(MT25TL01G_ReadSTR(&hqspi, QSPI_Ctx[Instance].InterfaceMode, pData, ReadAddr, Size) != MT25TL01G_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
    }
    else
    {
      if(MT25TL01G_ReadDTR(&hqspi, QSPI_Ctx[Instance].InterfaceMode, pData, ReadAddr, Size) != MT25TL01G_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Writes an amount of data to the QSPI memory.
  * @param  Instance   QSPI instance
  * @param  pData      Pointer to data to be written
  * @param  WriteAddr  Write start address
  * @param  Size       Size of data to write
  * @retval BSP status
  */
int32_t BSP_QSPI_Write(uint32_t Instance, uint8_t *pData, uint32_t WriteAddr, uint32_t Size)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t end_addr, current_size, current_addr;
  uint8_t *write_data;

  /* Check if the instance is supported */
  if(Instance >= QSPI_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Calculation of the size between the write address and the end of the page */
    current_size = MT25TL01G_PAGE_SIZE - (WriteAddr % MT25TL01G_PAGE_SIZE);

    /* Check if the size of the data is less than the remaining place in the page */
    if (current_size > Size)
    {
      current_size = Size;
    }

    /* Initialize the address variables */
    current_addr = WriteAddr;
    end_addr = WriteAddr + Size;
    write_data = pData;

    /* Perform the write page by page */
    do
    {
      /* Check if Flash busy ? */
      if(MT25TL01G_AutoPollingMemReady(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != MT25TL01G_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }/* Enable write operations */
      else if(MT25TL01G_WriteEnable(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != MT25TL01G_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }/* Issue page program command */
      else if(MT25TL01G_PageProgram(&hqspi, QSPI_Ctx[Instance].InterfaceMode, write_data, current_addr, current_size) != MT25TL01G_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }/* Configure automatic polling mode to wait for end of program */
      else if (MT25TL01G_AutoPollingMemReady(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != MT25TL01G_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else
      {
        /* Update the address and size variables for next page programming */
        current_addr += current_size;
        write_data += current_size;
        current_size = ((current_addr + MT25TL01G_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : MT25TL01G_PAGE_SIZE;
      }
    } while ((current_addr < end_addr) && (ret == BSP_ERROR_NONE));
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Erases the specified block of the QSPI memory.
  *         MT25TL01G support 4K, 32K, 64K size block erase commands for each Die.
  *           i.e 8K, 64K, 128K at BSP level (see BSP_QSPI_Erase_t type definition)
  * @param  Instance     QSPI instance
  * @param  BlockAddress Block address to erase
  * @param  BlockSize    Erase Block size
  * @retval BSP status
  */
int32_t BSP_QSPI_EraseBlock(uint32_t Instance, uint32_t BlockAddress, BSP_QSPI_Erase_t BlockSize)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if(Instance >= QSPI_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Check Flash busy ? */
    if(MT25TL01G_AutoPollingMemReady(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != MT25TL01G_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }/* Enable write operations */
    else if(MT25TL01G_WriteEnable(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != MT25TL01G_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      /* Issue Block Erase command */
      if(MT25TL01G_BlockErase(&hqspi, QSPI_Ctx[Instance].InterfaceMode, BlockAddress, (MT25TL01G_Erase_t)BlockSize) != MT25TL01G_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Erases the entire QSPI memory.
  * @param  Instance  QSPI instance
  * @retval BSP status
  */
int32_t BSP_QSPI_EraseChip(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if(Instance >= QSPI_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Check Flash busy ? */
    if(MT25TL01G_AutoPollingMemReady(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != MT25TL01G_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }/* Enable write operations */
    else if (MT25TL01G_WriteEnable(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != MT25TL01G_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      /* Issue Chip erase command */
      if(MT25TL01G_ChipErase(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != MT25TL01G_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Reads current status of the QSPI memory.
  *         If WIP != 0 then return busy.
  * @param  Instance  QSPI instance
  * @retval QSPI memory status: whether busy or not
  */
int32_t BSP_QSPI_GetStatus(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;
  uint8_t reg;

  /* Check if the instance is supported */
  if(Instance >= QSPI_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(MT25TL01G_ReadStatusRegister(&hqspi, QSPI_Ctx[Instance].InterfaceMode, &reg) != MT25TL01G_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      /* Check the value of the register */
      if ((reg & MT25TL01G_SR_WIP) != 0U)
      {
        ret = BSP_ERROR_BUSY;
      }
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Return the configuration of the QSPI memory.
  * @param  Instance  QSPI instance
  * @param  pInfo     pointer on the configuration structure
  * @retval BSP status
  */
int32_t BSP_QSPI_GetInfo(uint32_t Instance, BSP_QSPI_Info_t *pInfo)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if(Instance >= QSPI_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    (void)MT25TL01G_GetFlashInfo(pInfo);
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Configure the QSPI in memory-mapped mode
  *         Only 1 Instance can running MMP mode. And it will lock system at this mode.
  * @param  Instance  QSPI instance
  * @retval BSP status
  */
int32_t BSP_QSPI_EnableMemoryMappedMode(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if(Instance >= QSPI_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(QSPI_Ctx[Instance].TransferRate == BSP_QSPI_STR_TRANSFER)
    {
      if(MT25TL01G_EnableMemoryMappedModeSTR(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != MT25TL01G_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else /* Update QSPI context if all operations are well done */
      {
        QSPI_Ctx[Instance].IsInitialized = QSPI_ACCESS_MMP;
      }
    }
    else
    {
      if(MT25TL01G_EnableMemoryMappedModeDTR(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != MT25TL01G_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else /* Update QSPI context if all operations are well done */
      {
        QSPI_Ctx[Instance].IsInitialized = QSPI_ACCESS_MMP;
      }
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Exit form memory-mapped mode
  *         Only 1 Instance can running MMP mode. And it will lock system at this mode.
  * @param  Instance  QSPI instance
  * @retval BSP status
  */
int32_t BSP_QSPI_DisableMemoryMappedMode(uint32_t Instance)
{
  uint8_t Dummy;
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if(Instance >= QSPI_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(QSPI_Ctx[Instance].IsInitialized != QSPI_ACCESS_MMP)
    {
      ret = BSP_ERROR_QSPI_MMP_UNLOCK_FAILURE;
    }/* Abort MMP back to indirect mode */
    else if(HAL_QSPI_Abort(&hqspi) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      /* Force QSPI interface Sampling Shift to half cycle */
      hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;

      if(HAL_QSPI_Init(&hqspi)!= HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      /* Dummy read for exit from Performance Enhance mode */
      else if(MT25TL01G_ReadSTR(&hqspi, QSPI_Ctx[Instance].InterfaceMode, &Dummy, 0, 1) != MT25TL01G_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else /* Update QSPI context if all operations are well done */
      {
        QSPI_Ctx[Instance].IsInitialized = QSPI_ACCESS_INDIRECT;
      }
    }
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get flash ID, 3 Byte
  *         Manufacturer ID, Memory type, Memory density
  * @param  Instance  QSPI instance
  * @param  Id QSPI Identifier
  * @retval BSP status
  */
int32_t BSP_QSPI_ReadID(uint32_t Instance, uint8_t *Id)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if(Instance >= QSPI_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(MT25TL01G_ReadID(&hqspi, QSPI_Ctx[Instance].InterfaceMode, Id) != MT25TL01G_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set Flash to desired Interface mode. And this instance becomes current instance.
  *         If current instance running at MMP mode then this function isn't work.
  *         Indirect -> Indirect
  * @param  Instance  QSPI instance
  * @param  Mode      QSPI mode
  * @param  Rate      QSPI transfer rate
  * @retval BSP status
  */
int32_t BSP_QSPI_ConfigFlash(uint32_t Instance, BSP_QSPI_Interface_t Mode, BSP_QSPI_Transfer_t Rate)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if(Instance >= QSPI_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Check if MMP mode locked ************************************************/
    if(QSPI_Ctx[Instance].IsInitialized == QSPI_ACCESS_MMP)
    {
      ret = BSP_ERROR_QSPI_MMP_LOCK_FAILURE;
    }
    else
    {
      /* Setup MCU transfer rate setting ***************************************************/
      hqspi.Init.SampleShifting = (Rate == BSP_QSPI_STR_TRANSFER) ? QSPI_SAMPLE_SHIFTING_HALFCYCLE : QSPI_SAMPLE_SHIFTING_NONE;

      if(HAL_QSPI_Init(&hqspi)!= HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else
      {
        /* Setup Flash interface ***************************************************/
        switch(QSPI_Ctx[Instance].InterfaceMode)
        {
        case MT25TL01G_QPI_MODE :               /* 4-4-4 commands */
          if(Mode != MT25TL01G_QPI_MODE)
          {
            if(MT25TL01G_ExitQPIMode(&hqspi) != MT25TL01G_OK)
            {
              ret = BSP_ERROR_COMPONENT_FAILURE;
            }
          }
          break;

        case BSP_QSPI_SPI_MODE :               /* 1-1-1 commands, Power on H/W default setting */
        case BSP_QSPI_SPI_2IO_MODE :           /* 1-2-2 read commands */
        case BSP_QSPI_SPI_4IO_MODE :           /* 1-4-4 read commands */
        default :
          if(Mode == MT25TL01G_QPI_MODE)
          {
            if(MT25TL01G_EnterQPIMode(&hqspi) != MT25TL01G_OK)
            {
              ret = BSP_ERROR_COMPONENT_FAILURE;
            }
          }
          break;
        }

        /* Update QSPI context if all operations are well done */
        if(ret == BSP_ERROR_NONE)
        {
          /* Update current status parameter *****************************************/
          QSPI_Ctx[Instance].IsInitialized = QSPI_ACCESS_INDIRECT;
          QSPI_Ctx[Instance].InterfaceMode = Mode;
          QSPI_Ctx[Instance].TransferRate  = Rate;
        }
      }
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @}
  */

/** @defgroup STM32H747I_DISCO_QSPI_Private_Functions Private Functions
  * @{
  */

/**
  * @brief QSPI MSP Initialization
  * @param hQspi : QSPI handle
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  *           - NVIC configuration for QSPI interrupt
  * @retval None
  */
static void QSPI_MspInit(QSPI_HandleTypeDef *hQspi)
{
  GPIO_InitTypeDef gpio_init_structure;

  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable the QuadSPI memory interface clock */
  QSPI_CLK_ENABLE();
  /* Reset the QuadSPI memory interface */
  QSPI_FORCE_RESET();
  QSPI_RELEASE_RESET();
  /* Enable GPIO clocks */
  QSPI_CLK_GPIO_CLK_ENABLE();
  QSPI_BK1_CS_GPIO_CLK_ENABLE();
  QSPI_BK1_D0_GPIO_CLK_ENABLE();
  QSPI_BK1_D1_GPIO_CLK_ENABLE();
  QSPI_BK1_D2_GPIO_CLK_ENABLE();
  QSPI_BK1_D3_GPIO_CLK_ENABLE();

  QSPI_BK2_CS_GPIO_CLK_ENABLE();
  QSPI_BK2_D0_GPIO_CLK_ENABLE();
  QSPI_BK2_D1_GPIO_CLK_ENABLE();
  QSPI_BK2_D2_GPIO_CLK_ENABLE();
  QSPI_BK2_D3_GPIO_CLK_ENABLE();

  /*##-2- Configure peripheral GPIO ##########################################*/
  /* QSPI CLK GPIO pin configuration  */
  gpio_init_structure.Pin       = QSPI_CLK_PIN;
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init_structure.Pull      = GPIO_NOPULL;
  gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(QSPI_CLK_GPIO_PORT, &gpio_init_structure);

  /* QSPI CS GPIO pin configuration  */
  gpio_init_structure.Pin       = QSPI_BK1_CS_PIN;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Alternate = GPIO_AF10_QUADSPI;
  HAL_GPIO_Init(QSPI_BK1_CS_GPIO_PORT, &gpio_init_structure);

  /* QSPI D0 GPIO pin configuration  */
  gpio_init_structure.Pin       = QSPI_BK1_D0_PIN;
  gpio_init_structure.Pull      = GPIO_NOPULL;
  gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(QSPI_BK1_D0_GPIO_PORT, &gpio_init_structure);

  gpio_init_structure.Pin       = QSPI_BK2_D0_PIN;
  gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(QSPI_BK2_D0_GPIO_PORT, &gpio_init_structure);

  /* QSPI D1 GPIO pin configuration  */
  gpio_init_structure.Pin       = QSPI_BK1_D1_PIN;
  gpio_init_structure.Alternate = GPIO_AF10_QUADSPI;
  HAL_GPIO_Init(QSPI_BK1_D1_GPIO_PORT, &gpio_init_structure);

  gpio_init_structure.Pin       = QSPI_BK2_D1_PIN;
  gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(QSPI_BK2_D1_GPIO_PORT, &gpio_init_structure);

  /* QSPI D2 GPIO pin configuration  */
  gpio_init_structure.Pin       = QSPI_BK1_D2_PIN;
  gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(QSPI_BK1_D2_GPIO_PORT, &gpio_init_structure);

  gpio_init_structure.Pin       = QSPI_BK2_D2_PIN;
  HAL_GPIO_Init(QSPI_BK2_D2_GPIO_PORT, &gpio_init_structure);

  /* QSPI D3 GPIO pin configuration  */
  gpio_init_structure.Pin       = QSPI_BK1_D3_PIN;
  HAL_GPIO_Init(QSPI_BK1_D3_GPIO_PORT, &gpio_init_structure);

  gpio_init_structure.Pin       = QSPI_BK2_D3_PIN;
  HAL_GPIO_Init(QSPI_BK2_D3_GPIO_PORT, &gpio_init_structure);

  /*##-3- Configure the NVIC for QSPI #########################################*/
  /* NVIC configuration for QSPI interrupt */
  HAL_NVIC_SetPriority(QUADSPI_IRQn, 0x0F, 0);
  HAL_NVIC_EnableIRQ(QUADSPI_IRQn);
}

/**
  * @brief QSPI MSP De-Initialization
  * @param hQspi  QSPI handle
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO and NVIC configuration to their default state
  * @retval None
  */
static void QSPI_MspDeInit(QSPI_HandleTypeDef *hQspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hQspi);

  /*##-2- Disable peripherals and GPIO Clocks ################################*/
  /* De-Configure QSPI pins */
  HAL_GPIO_DeInit(QSPI_CLK_GPIO_PORT, QSPI_CLK_PIN);
  HAL_GPIO_DeInit(QSPI_BK1_CS_GPIO_PORT, QSPI_BK1_CS_PIN);
  HAL_GPIO_DeInit(QSPI_BK1_D0_GPIO_PORT, QSPI_BK1_D0_PIN);
  HAL_GPIO_DeInit(QSPI_BK1_D1_GPIO_PORT, QSPI_BK1_D1_PIN);
  HAL_GPIO_DeInit(QSPI_BK1_D2_GPIO_PORT, QSPI_BK1_D2_PIN);
  HAL_GPIO_DeInit(QSPI_BK1_D3_GPIO_PORT, QSPI_BK1_D3_PIN);

  HAL_GPIO_DeInit(QSPI_BK2_CS_GPIO_PORT, QSPI_BK2_CS_PIN);
  HAL_GPIO_DeInit(QSPI_BK2_D0_GPIO_PORT, QSPI_BK2_D0_PIN);
  HAL_GPIO_DeInit(QSPI_BK2_D1_GPIO_PORT, QSPI_BK2_D1_PIN);
  HAL_GPIO_DeInit(QSPI_BK2_D2_GPIO_PORT, QSPI_BK2_D2_PIN);
  HAL_GPIO_DeInit(QSPI_BK2_D3_GPIO_PORT, QSPI_BK2_D3_PIN);

  /*##-3- Reset peripherals ##################################################*/
  /* Reset the QuadSPI memory interface */
  QSPI_FORCE_RESET();
  QSPI_RELEASE_RESET();

  /* Disable the QuadSPI memory interface clock */
  QSPI_CLK_DISABLE();
}

/**
  * @brief  This function reset the QSPI Flash memory.
  *         Fore QPI+SPI reset to avoid system come from unknown status.
  *         Flash accept 1-1-1, 1-1-2, 1-2-2 commands after reset.
  * @param  Instance  QSPI instance
  * @retval BSP status
  */
static int32_t QSPI_ResetMemory(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Send RESET ENABLE command in QPI mode (QUAD I/Os, 4-4-4) */
  if(MT25TL01G_ResetEnable(&hqspi, MT25TL01G_QPI_MODE) != MT25TL01G_OK)
  {
    ret =BSP_ERROR_COMPONENT_FAILURE;
  }/* Send RESET memory command in QPI mode (QUAD I/Os, 4-4-4) */
  else if(MT25TL01G_ResetMemory(&hqspi, MT25TL01G_QPI_MODE) != MT25TL01G_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }/* Wait Flash ready */
  else if(MT25TL01G_AutoPollingMemReady(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != MT25TL01G_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }/* Send RESET ENABLE command in SPI mode (1-1-1) */
  else if(MT25TL01G_ResetEnable(&hqspi, BSP_QSPI_SPI_MODE) != MT25TL01G_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }/* Send RESET memory command in SPI mode (1-1-1) */
  else if(MT25TL01G_ResetMemory(&hqspi, BSP_QSPI_SPI_MODE) != MT25TL01G_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    QSPI_Ctx[Instance].IsInitialized = QSPI_ACCESS_INDIRECT;  /* After reset S/W setting to indirect access   */
    QSPI_Ctx[Instance].InterfaceMode = BSP_QSPI_SPI_MODE;     /* After reset H/W back to SPI mode by default  */
    QSPI_Ctx[Instance].TransferRate  = BSP_QSPI_STR_TRANSFER; /* After reset S/W setting to STR mode          */
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  This function configure the dummy cycles on memory side.
  *         Dummy cycle bit locate in Configuration Register[7:6]
  * @param  Instance  QSPI instance
  * @retval BSP status
  */
static int32_t QSPI_DummyCyclesCfg(uint32_t Instance)
{
    int32_t ret= BSP_ERROR_NONE;
    QSPI_CommandTypeDef s_command;
  uint16_t reg=0;

  /* Initialize the read volatile configuration register command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = MT25TL01G_READ_VOL_CFG_REG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.DummyCycles       = 0;
  s_command.NbData            = 2;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return BSP_ERROR_COMPONENT_FAILURE;
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive(&hqspi, (uint8_t *)(&reg), HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return BSP_ERROR_COMPONENT_FAILURE;
  }

  /* Enable write operations */
  if (MT25TL01G_WriteEnable(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != MT25TL01G_OK)
  {
    return BSP_ERROR_COMPONENT_FAILURE;
  }

  /* Update volatile configuration register (with new dummy cycles) */
  s_command.Instruction = MT25TL01G_WRITE_VOL_CFG_REG_CMD;
  MODIFY_REG(reg, 0xF0F0, ((MT25TL01G_DUMMY_CYCLES_READ_QUAD << 4) |
                               (MT25TL01G_DUMMY_CYCLES_READ_QUAD << 12)));

  /* Configure the write volatile configuration register command */
  if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return BSP_ERROR_COMPONENT_FAILURE;
  }

  /* Transmission of the data */
  if (HAL_QSPI_Transmit(&hqspi, (uint8_t *)(&reg), HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return BSP_ERROR_COMPONENT_FAILURE;
  }

  /* Return BSP status */
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
