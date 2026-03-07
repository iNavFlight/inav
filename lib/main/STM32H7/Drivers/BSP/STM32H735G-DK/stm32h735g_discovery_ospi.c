/**
  ******************************************************************************
  * @file    stm32h735g_discovery_ospi_nor.c
  * @author  MCD Application Team
  * @brief   This file includes a standard driver for the MX25LM51245G OSPI
  *          memory mounted on STM32H735G-DK board.
  @verbatim
  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================
  [..]
   (#) This driver is used to drive the MX25LM51245G Octal NOR and the S70KL1281 HyperRAM
       external memories mounted on STM32H735G-DK board.

   (#) This driver need specific component driver (MX25LM51245G and S70KL1281) to be included with.

   (#) MX25LM51245G Initialization steps:
       (++) Initialize the OPSI external memory using the BSP_OSPI_NOR_Init() function. This
            function includes the MSP layer hardware resources initialization and the
            OSPI interface with the external memory.

   (#) MX25LM51245G Octal NOR memory operations
       (++) OSPI memory can be accessed with read/write operations once it is
            initialized.
            Read/write operation can be performed with AHB access using the functions
            BSP_OSPI_NOR_Read()/BSP_OSPI_NOR_Write().
       (++) The function BSP_OSPI_NOR_GetInfo() returns the configuration of the OSPI memory.
            (see the OSPI memory data sheet)
       (++) Perform erase block operation using the function BSP_OSPI_NOR_Erase_Block() and by
            specifying the block address. You can perform an erase operation of the whole
            chip by calling the function BSP_OSPI_NOR_Erase_Chip().
       (++) The function BSP_OSPI_NOR_GetStatus() returns the current status of the OSPI memory.
            (see the OSPI memory data sheet)
       (++) The memory access can be configured in memory-mapped mode with the call of
            function BSP_OSPI_NOR_EnableMemoryMapped(). To go back in indirect mode, the
            function BSP_OSPI_NOR_DisableMemoryMapped() should be used.
       (++) The erase operation can be suspend and resume with using functions
            BSP_OSPI_NOR_SuspendErase() and BSP_OSPI_NOR_ResumeErase()
       (++) The function BSP_OSPI_NOR_ReadID() returns the identifier of the memory
            (see the OSPI memory data sheet)
       (++) The configuration of the interface between peripheral and memory is done by
            the function BSP_OSPI_NOR_ConfigFlash(), three modes are possible :
            - SPI : instruction, address and data on one line
            - STR OPI : instruction, address and data on eight lines with sampling on one edge of clock
            - DTR OPI : instruction, address and data on eight lines with sampling on both edgaes of clock

   (#) S70KL1281 Initialization steps:
       (++) Initialize the HyperRAM external memory using the BSP_OSPI_RAM_Init() function. This
            function includes the MSP layer hardware resources initialization and the
            OSPI interface with the external memory.

   (#) S70KL1281 HyperRAM memory operations
       (++) HyperRAM memory can be accessed with read/write operations once it is
            initialized.
            Read/write operation can be performed with AHB access using the functions
            BSP_OSPI_RAM_Read()/BSP_OSPI_RAM_Write().
            Read/write operation can be performed with DMA using the functions
            BSP_OSPI_RAM_Read_DMA()/BSP_OSPI_RAM_Write_DMA().
       (++) The memory access can be configured in memory-mapped mode with the call of
            function BSP_OSPI_RAM_EnableMemoryMapped(). To go back in indirect mode, the
            function BSP_OSPI_RAM_DisableMemoryMapped() should be used.
       (++) The configuration of the interface between peripheral and memory is done by
            the function BSP_OSPI_RAM_ConfigHyperRAM() :
            - one latency mode is possible :
              - Fixed latency mode
            - five burst configuration are possible :
              - Linear burst access
              - 16-bytes hybrid burst access
              - 32-bytes hybrid burst access
              - 64-bytes hybrid burst access
              - 128-bytes hybrid burst access

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
#include "stm32h735g_discovery_ospi.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H735G_DK
  * @{
  */

/** @defgroup STM32H735G_DK_OSPI STM32H735G_DK OSPI
  * @{
  */

/* Exported variables --------------------------------------------------------*/
/** @addtogroup STM32H735G_DK_OSPI_NOR_Exported_Variables OSPI_NOR Exported Variables
  * @{
  */
OSPI_HandleTypeDef hospi_nor[OSPI_NOR_INSTANCES_NUMBER] = {0};;
OSPI_NOR_Ctx_t Ospi_Nor_Ctx[OSPI_NOR_INSTANCES_NUMBER] = {{
    OSPI_ACCESS_NONE,
    MX25LM51245G_SPI_MODE,
    MX25LM51245G_STR_TRANSFER
  }
};
/**
  * @}
  */
/** @addtogroup STM32H735G_DK_OSPI_RAM_Exported_Variables OSPI_RAM Exported Variables
  * @{
  */
OSPI_HandleTypeDef hospi_ram[OSPI_RAM_INSTANCES_NUMBER] = {0};
OSPI_RAM_Ctx_t Ospi_Ram_Ctx[OSPI_RAM_INSTANCES_NUMBER] = {{
    OSPI_ACCESS_NONE,
    BSP_OSPI_RAM_FIXED_LATENCY,
    BSP_OSPI_RAM_HYBRID_BURST,
    BSP_OSPI_RAM_BURST_16_BYTES
  }
};
/**
  * @}
  */

/** @defgroup STM32H735G_DK_OSPI_NOR_Private_Functions OSPI_NOR Private Functions
  * @{
  */
static void    OSPI_NOR_MspInit(OSPI_HandleTypeDef *hospi);
static void    OSPI_NOR_MspDeInit(OSPI_HandleTypeDef *hospi);
static int32_t OSPI_NOR_ResetMemory(uint32_t Instance);
static int32_t OSPI_NOR_EnterDOPIMode(uint32_t Instance);
static int32_t OSPI_NOR_EnterSOPIMode(uint32_t Instance);
static int32_t OSPI_NOR_ExitOPIMode(uint32_t Instance);
/**
  * @}
  */
/** @defgroup STM32H735G_DK_OSPI_RAM_Private_Functions OSPI_RAM Private Functions
  * @{
  */
static void OSPI_RAM_MspInit(OSPI_HandleTypeDef *hospi);
static void OSPI_RAM_MspDeInit(OSPI_HandleTypeDef *hospi);

/**
  * @}
  */

/* Exported functions ---------------------------------------------------------*/

/** @defgroup STM32H735G_DK_OSPI_NOR_Exported_Functions OSPI_NOR Exported Functions
  * @{
  */

/**
  * @brief  Initializes the OSPI interface.
  * @param  Instance   OSPI Instance
  * @param  Init       OSPI Init structure
  * @retval BSP status
  */
int32_t BSP_OSPI_NOR_Init(uint32_t Instance, BSP_OSPI_NOR_Init_t *Init)
{
  int32_t ret;
  BSP_OSPI_NOR_Info_t pInfo;
  MX_OSPI_InitTypeDef ospi_config;

  /* Check if the instance is supported */
  if (Instance >= OSPI_NOR_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Check if the instance is already initialized */
    if (Ospi_Nor_Ctx[Instance].IsInitialized == OSPI_ACCESS_NONE)
    {
#if (USE_HAL_OSPI_REGISTER_CALLBACKS == 1)
      /* Register the OSPI MSP Callbacks */
      if (Ospi_Nor_Ctx[Instance].IsMspCallbacksValid == 0UL)
      {
        if (BSP_OSPI_NOR_RegisterDefaultMspCallbacks(Instance) != BSP_ERROR_NONE)
        {
          return BSP_ERROR_PERIPH_FAILURE;
        }
      }
#else
      /* Msp OSPI initialization */
      OSPI_NOR_MspInit(&hospi_nor[Instance]);
#endif /* USE_HAL_OSPI_REGISTER_CALLBACKS */

      /* Get Flash information of one memory */
      (void)MX25LM51245G_GetFlashInfo(&pInfo);

      /* Fill config structure */
      ospi_config.ClockPrescaler = 2U;
      ospi_config.MemorySize     = (uint32_t)POSITION_VAL((uint32_t)pInfo.FlashSize);
      ospi_config.SampleShifting = HAL_OSPI_SAMPLE_SHIFTING_NONE;
      ospi_config.TransferRate   = Init->TransferRate;

      /* STM32 OSPI Clock configuration */
      if (MX_OSPI_ClockConfig(&hospi_nor[Instance]) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      /* STM32 OSPI interface initialization */
      else if (MX_OSPI_NOR_Init(&hospi_nor[Instance], &ospi_config) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      /* OSPI memory reset */
      else if (OSPI_NOR_ResetMemory(Instance) != BSP_ERROR_NONE)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      /* Check if memory is ready */
      else if (MX25LM51245G_AutoPollingMemReady(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                                Ospi_Nor_Ctx[Instance].TransferRate) != MX25LM51245G_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      /* Configure the memory */
      else if (BSP_OSPI_NOR_ConfigFlash(Instance, Init->InterfaceMode, Init->TransferRate) != BSP_ERROR_NONE)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else
      {
        ret = BSP_ERROR_NONE;
      }
    }
    else
    {
      ret = BSP_ERROR_NONE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  De-Initializes the OSPI interface.
  * @param  Instance   OSPI Instance
  * @retval BSP status
  */
int32_t BSP_OSPI_NOR_DeInit(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if (Instance >= OSPI_NOR_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Disable Memory mapped mode */
    if (Ospi_Nor_Ctx[Instance].IsInitialized == OSPI_ACCESS_MMP)
    {
      if (BSP_OSPI_NOR_DisableMemoryMappedMode(Instance) != BSP_ERROR_NONE)
      {
        return BSP_ERROR_COMPONENT_FAILURE;
      }
    }

    /* Set default Ospi_Nor_Ctx values */
    Ospi_Nor_Ctx[Instance].IsInitialized = OSPI_ACCESS_NONE;
    Ospi_Nor_Ctx[Instance].InterfaceMode = BSP_OSPI_NOR_SPI_MODE;
    Ospi_Nor_Ctx[Instance].TransferRate  = BSP_OSPI_NOR_STR_TRANSFER;

#if (USE_HAL_OSPI_REGISTER_CALLBACKS == 0)
    OSPI_NOR_MspDeInit(&hospi_nor[Instance]);
#endif /* (USE_HAL_OSPI_REGISTER_CALLBACKS == 0) */

    /* Call the DeInit function to reset the driver */
    if (HAL_OSPI_DeInit(&hospi_nor[Instance]) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Initializes the OSPI interface.
  * @param  hospi          OSPI handle
  * @param  Config         OSPI config structure
  * @retval BSP status
  */
__weak HAL_StatusTypeDef MX_OSPI_NOR_Init(OSPI_HandleTypeDef *hospi, MX_OSPI_InitTypeDef *Config)
{
  /* OctoSPI initialization */
  hospi->Instance = OCTOSPI1;

  hospi->Init.FifoThreshold      = 4;
  hospi->Init.DualQuad           = HAL_OSPI_DUALQUAD_DISABLE;
  hospi->Init.DeviceSize         = Config->MemorySize; /* 512 MBits */
  hospi->Init.ChipSelectHighTime = 2;
  hospi->Init.FreeRunningClock   = HAL_OSPI_FREERUNCLK_DISABLE;
  hospi->Init.ClockMode          = HAL_OSPI_CLOCK_MODE_0;
  hospi->Init.WrapSize           = HAL_OSPI_WRAP_NOT_SUPPORTED;
  hospi->Init.ClockPrescaler     = Config->ClockPrescaler;
  hospi->Init.SampleShifting     = Config->SampleShifting;
  hospi->Init.ChipSelectBoundary = 0;

  if (Config->TransferRate == BSP_OSPI_NOR_DTR_TRANSFER)
  {
    hospi->Init.MemoryType            = HAL_OSPI_MEMTYPE_MACRONIX;
    hospi->Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_ENABLE;
  }
  else
  {
    hospi->Init.MemoryType            = HAL_OSPI_MEMTYPE_MICRON;
    hospi->Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_DISABLE;
  }

  return HAL_OSPI_Init(hospi);
}

#if (USE_HAL_OSPI_REGISTER_CALLBACKS == 1)
/**
  * @brief Default BSP OSPI Msp Callbacks
  * @param Instance      OSPI Instance
  * @retval BSP status
  */
int32_t BSP_OSPI_NOR_RegisterDefaultMspCallbacks(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if (Instance >= OSPI_NOR_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Register MspInit/MspDeInit Callbacks */
    if (HAL_OSPI_RegisterCallback(&hospi_nor[Instance], HAL_OSPI_MSP_INIT_CB_ID, OSPI_NOR_MspInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if (HAL_OSPI_RegisterCallback(&hospi_nor[Instance], HAL_OSPI_MSP_DEINIT_CB_ID, OSPI_NOR_MspDeInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      Ospi_Nor_Ctx[Instance].IsMspCallbacksValid = 1U;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief BSP OSPI Msp Callback registering
  * @param Instance     OSPI Instance
  * @param CallBacks    pointer to MspInit/MspDeInit callbacks functions
  * @retval BSP status
  */
int32_t BSP_OSPI_NOR_RegisterMspCallbacks(uint32_t Instance, BSP_OSPI_Cb_t *CallBacks)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if (Instance >= OSPI_NOR_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Register MspInit/MspDeInit Callbacks */
    if (HAL_OSPI_RegisterCallback(&hospi_nor[Instance], HAL_OSPI_MSP_INIT_CB_ID, CallBacks->pMspInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if (HAL_OSPI_RegisterCallback(&hospi_nor[Instance], HAL_OSPI_MSP_DEINIT_CB_ID, CallBacks->pMspDeInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      Ospi_Nor_Ctx[Instance].IsMspCallbacksValid = 1U;
    }
  }

  /* Return BSP status */
  return ret;
}
#endif /* (USE_HAL_OSPI_REGISTER_CALLBACKS == 1) */

/**
  * @brief  Reads an amount of data from the OSPI memory.
  * @param  Instance  OSPI instance
  * @param  pData     Pointer to data to be read
  * @param  ReadAddr  Read start address
  * @param  Size      Size of data to read
  * @retval BSP status
  */
int32_t BSP_OSPI_NOR_Read(uint32_t Instance, uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if (Instance >= OSPI_NOR_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if (Ospi_Nor_Ctx[Instance].TransferRate == BSP_OSPI_NOR_STR_TRANSFER)
    {
      if (MX25LM51245G_ReadSTR(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode, MX25LM51245G_4BYTES_SIZE, pData,
                               ReadAddr, Size) != MX25LM51245G_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
    }
    else
    {
      if (MX25LM51245G_ReadDTR(&hospi_nor[Instance], pData, ReadAddr, Size) != MX25LM51245G_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Writes an amount of data to the OSPI memory.
  * @param  Instance  OSPI instance
  * @param  pData     Pointer to data to be written
  * @param  WriteAddr Write start address
  * @param  Size      Size of data to write
  * @retval BSP status
  */
int32_t BSP_OSPI_NOR_Write(uint32_t Instance, uint8_t *pData, uint32_t WriteAddr, uint32_t Size)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t end_addr, current_size, current_addr;
  uint32_t data_addr;

  /* Check if the instance is supported */
  if (Instance >= OSPI_NOR_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Calculation of the size between the write address and the end of the page */
    current_size = MX25LM51245G_PAGE_SIZE - (WriteAddr % MX25LM51245G_PAGE_SIZE);

    /* Check if the size of the data is less than the remaining place in the page */
    if (current_size > Size)
    {
      current_size = Size;
    }

    /* Initialize the address variables */
    current_addr = WriteAddr;
    end_addr = WriteAddr + Size;
    data_addr = (uint32_t)pData;

    /* Perform the write page by page */
    do
    {
      /* Check if Flash busy ? */
      if (MX25LM51245G_AutoPollingMemReady(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                           Ospi_Nor_Ctx[Instance].TransferRate) != MX25LM51245G_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }/* Enable write operations */
      else if (MX25LM51245G_WriteEnable(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                        Ospi_Nor_Ctx[Instance].TransferRate) != MX25LM51245G_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else
      {
        if (Ospi_Nor_Ctx[Instance].TransferRate == BSP_OSPI_NOR_STR_TRANSFER)
        {
          /* Issue page program command */
          if (MX25LM51245G_PageProgram(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode, MX25LM51245G_4BYTES_SIZE,
                                       (uint8_t *)data_addr, current_addr, current_size) != MX25LM51245G_OK)
          {
            ret = BSP_ERROR_COMPONENT_FAILURE;
          }
        }
        else
        {
          /* Issue page program command */
          if (MX25LM51245G_PageProgramDTR(&hospi_nor[Instance], (uint8_t *)data_addr, current_addr,
                                          current_size) != MX25LM51245G_OK)
          {
            ret = BSP_ERROR_COMPONENT_FAILURE;
          }
        }

        if (ret == BSP_ERROR_NONE)
        {
          /* Configure automatic polling mode to wait for end of program */
          if (MX25LM51245G_AutoPollingMemReady(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                               Ospi_Nor_Ctx[Instance].TransferRate) != MX25LM51245G_OK)
          {
            ret = BSP_ERROR_COMPONENT_FAILURE;
          }
          else
          {
            /* Update the address and size variables for next page programming */
            current_addr += current_size;
            data_addr += current_size;
            current_size = ((current_addr + MX25LM51245G_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : MX25LM51245G_PAGE_SIZE;
          }
        }
      }
    } while ((current_addr < end_addr) && (ret == BSP_ERROR_NONE));
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Erases the specified block of the OSPI memory.
  * @param  Instance     OSPI instance
  * @param  BlockAddress Block address to erase
  * @param  BlockSize    Erase Block size
  * @retval BSP status
  */
int32_t BSP_OSPI_NOR_Erase_Block(uint32_t Instance, uint32_t BlockAddress, BSP_OSPI_NOR_Erase_t BlockSize)
{
  int32_t ret;

  /* Check if the instance is supported */
  if (Instance >= OSPI_NOR_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Check Flash busy ? */
    if (MX25LM51245G_AutoPollingMemReady(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                         Ospi_Nor_Ctx[Instance].TransferRate) != MX25LM51245G_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }/* Enable write operations */
    else if (MX25LM51245G_WriteEnable(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                      Ospi_Nor_Ctx[Instance].TransferRate) != MX25LM51245G_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }/* Issue Block Erase command */
    else if (MX25LM51245G_BlockErase(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                     Ospi_Nor_Ctx[Instance].TransferRate, MX25LM51245G_4BYTES_SIZE, BlockAddress, BlockSize) != MX25LM51245G_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }/* Check Flash busy ? */
    else if (MX25LM51245G_AutoPollingMemReady(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                              Ospi_Nor_Ctx[Instance].TransferRate) != MX25LM51245G_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      ret = BSP_ERROR_NONE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Erases the entire OSPI memory.
  * @param  Instance  QSPI instance
  * @retval BSP status
  */
int32_t BSP_OSPI_NOR_Erase_Chip(uint32_t Instance)
{
  int32_t ret;

  /* Check if the instance is supported */
  if (Instance >= OSPI_NOR_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Check Flash busy ? */
    if (MX25LM51245G_AutoPollingMemReady(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                         Ospi_Nor_Ctx[Instance].TransferRate) != MX25LM51245G_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }/* Enable write operations */
    else if (MX25LM51245G_WriteEnable(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                      Ospi_Nor_Ctx[Instance].TransferRate) != MX25LM51245G_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }/* Issue Chip erase command */
    else if (MX25LM51245G_ChipErase(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                    Ospi_Nor_Ctx[Instance].TransferRate) != MX25LM51245G_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      ret = BSP_ERROR_NONE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Reads current status of the OSPI memory.
  * @param  Instance  QSPI instance
  * @retval OSPI memory status: whether busy or not
  */
int32_t BSP_OSPI_NOR_GetStatus(uint32_t Instance)
{
  static uint8_t reg[2];
  int32_t ret;

  /* Check if the instance is supported */
  if (Instance >= OSPI_NOR_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if (MX25LM51245G_ReadSecurityRegister(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                          Ospi_Nor_Ctx[Instance].TransferRate, reg) != MX25LM51245G_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }/* Check the value of the register */
    else if ((reg[0] & (MX25LM51245G_SECR_P_FAIL | MX25LM51245G_SECR_E_FAIL)) != 0U)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else if ((reg[0] & (MX25LM51245G_SECR_PSB | MX25LM51245G_SECR_ESB)) != 0U)
    {
      ret = BSP_ERROR_OSPI_SUSPENDED;
    }
    else if (MX25LM51245G_ReadStatusRegister(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                             Ospi_Nor_Ctx[Instance].TransferRate, reg) != MX25LM51245G_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }/* Check the value of the register */
    else if ((reg[0] & MX25LM51245G_SR_WIP) != 0U)
    {
      ret = BSP_ERROR_BUSY;
    }
    else
    {
      ret = BSP_ERROR_NONE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Return the configuration of the OSPI memory.
  * @param  Instance  OSPI instance
  * @param  pInfo     pointer on the configuration structure
  * @retval BSP status
  */
int32_t BSP_OSPI_NOR_GetInfo(uint32_t Instance, BSP_OSPI_NOR_Info_t *pInfo)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if (Instance >= OSPI_NOR_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    (void)MX25LM51245G_GetFlashInfo(pInfo);
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Configure the OSPI in memory-mapped mode
  * @param  Instance  OSPI instance
  * @retval BSP status
  */
int32_t BSP_OSPI_NOR_EnableMemoryMappedMode(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if (Instance >= OSPI_NOR_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if (Ospi_Nor_Ctx[Instance].TransferRate == BSP_OSPI_NOR_STR_TRANSFER)
    {
      if (MX25LM51245G_EnableMemoryMappedModeSTR(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                                 MX25LM51245G_4BYTES_SIZE) != MX25LM51245G_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else /* Update OSPI context if all operations are well done */
      {
        Ospi_Nor_Ctx[Instance].IsInitialized = OSPI_ACCESS_MMP;
      }
    }
    else
    {
      if (MX25LM51245G_EnableMemoryMappedModeDTR(&hospi_nor[Instance],
                                                 Ospi_Nor_Ctx[Instance].InterfaceMode) != MX25LM51245G_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else /* Update OSPI context if all operations are well done */
      {
        Ospi_Nor_Ctx[Instance].IsInitialized = OSPI_ACCESS_MMP;
      }
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Exit form memory-mapped mode
  *         Only 1 Instance can running MMP mode. And it will lock system at this mode.
  * @param  Instance  OSPI instance
  * @retval BSP status
  */
int32_t BSP_OSPI_NOR_DisableMemoryMappedMode(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if (Instance >= OSPI_NOR_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if (Ospi_Nor_Ctx[Instance].IsInitialized != OSPI_ACCESS_MMP)
    {
      ret = BSP_ERROR_OSPI_MMP_UNLOCK_FAILURE;
    }/* Abort MMP back to indirect mode */
    else if (HAL_OSPI_Abort(&hospi_nor[Instance]) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else /* Update OSPI NOR context if all operations are well done */
    {
      /* Update OSPI context if all operations are well done */
      Ospi_Nor_Ctx[Instance].IsInitialized = OSPI_ACCESS_INDIRECT;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get flash ID 3 Bytes:
  *         Manufacturer ID, Memory type, Memory density
  * @param  Instance  OSPI instance
  * @param  Id Pointer to flash ID bytes
  * @retval BSP status
  */
int32_t BSP_OSPI_NOR_ReadID(uint32_t Instance, uint8_t *Id)
{
  int32_t ret;

  /* Check if the instance is supported */
  if (Instance >= OSPI_NOR_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if (MX25LM51245G_ReadID(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                               Ospi_Nor_Ctx[Instance].TransferRate, Id) != MX25LM51245G_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set Flash to desired Interface mode. And this instance becomes current instance.
  *         If current instance running at MMP mode then this function doesn't work.
  *         Indirect -> Indirect
  * @param  Instance  OSPI instance
  * @param  Mode      OSPI mode
  * @param  Rate      OSPI transfer rate
  * @retval BSP status
  */
int32_t BSP_OSPI_NOR_ConfigFlash(uint32_t Instance, BSP_OSPI_NOR_Interface_t Mode, BSP_OSPI_NOR_Transfer_t Rate)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if (Instance >= OSPI_NOR_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Check if MMP mode locked ************************************************/
    if (Ospi_Nor_Ctx[Instance].IsInitialized == OSPI_ACCESS_MMP)
    {
      ret = BSP_ERROR_OSPI_MMP_LOCK_FAILURE;
    }
    else
    {
      /* Setup Flash interface ***************************************************/
      switch (Ospi_Nor_Ctx[Instance].InterfaceMode)
      {
        case BSP_OSPI_NOR_OPI_MODE :  /* 8-8-8 commands */
          if ((Mode != BSP_OSPI_NOR_OPI_MODE) || (Rate != Ospi_Nor_Ctx[Instance].TransferRate))
          {
            /* Exit OPI mode */
            ret = OSPI_NOR_ExitOPIMode(Instance);

            if ((ret == BSP_ERROR_NONE) && (Mode == BSP_OSPI_NOR_OPI_MODE))
            {

              if (Ospi_Nor_Ctx[Instance].TransferRate == BSP_OSPI_NOR_STR_TRANSFER)
              {
                /* Enter DTR OPI mode */
                ret = OSPI_NOR_EnterDOPIMode(Instance);
              }
              else
              {
                /* Enter STR OPI mode */
                ret = OSPI_NOR_EnterSOPIMode(Instance);
              }
            }
          }
          break;

        case BSP_OSPI_NOR_SPI_MODE :  /* 1-1-1 commands, Power on H/W default setting */
        default :
          if (Mode == BSP_OSPI_NOR_OPI_MODE)
          {
            if (Rate == BSP_OSPI_NOR_STR_TRANSFER)
            {
              /* Enter STR OPI mode */
              ret = OSPI_NOR_EnterSOPIMode(Instance);
            }
            else
            {
              /* Enter DTR OPI mode */
              ret = OSPI_NOR_EnterDOPIMode(Instance);
            }
          }
          break;
      }

      /* Update OSPI context if all operations are well done */
      if (ret == BSP_ERROR_NONE)
      {
        /* Update current status parameter *****************************************/
        Ospi_Nor_Ctx[Instance].IsInitialized = OSPI_ACCESS_INDIRECT;
        Ospi_Nor_Ctx[Instance].InterfaceMode = Mode;
        Ospi_Nor_Ctx[Instance].TransferRate  = Rate;
      }
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  This function suspends an ongoing erase command.
  * @param  Instance  QSPI instance
  * @retval BSP status
  */
int32_t BSP_OSPI_NOR_SuspendErase(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if (Instance >= OSPI_NOR_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  /* Check whether the device is busy (erase operation is in progress). */
  else if (BSP_OSPI_NOR_GetStatus(Instance) != BSP_ERROR_BUSY)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else if (MX25LM51245G_Suspend(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                Ospi_Nor_Ctx[Instance].TransferRate) != MX25LM51245G_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else if (BSP_OSPI_NOR_GetStatus(Instance) != BSP_ERROR_OSPI_SUSPENDED)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  This function resumes a paused erase command.
  * @param  Instance  QSPI instance
  * @retval BSP status
  */
int32_t BSP_OSPI_NOR_ResumeErase(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if (Instance >= OSPI_NOR_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  /* Check whether the device is busy (erase operation is in progress). */
  else if (BSP_OSPI_NOR_GetStatus(Instance) != BSP_ERROR_OSPI_SUSPENDED)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else if (MX25LM51245G_Resume(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                               Ospi_Nor_Ctx[Instance].TransferRate) != MX25LM51245G_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  /*
  When this command is executed, the status register write in progress bit is set to 1, and
  the flag status register program erase controller bit is set to 0. This command is ignored
  if the device is not in a suspended state.
  */
  else if (BSP_OSPI_NOR_GetStatus(Instance) != BSP_ERROR_BUSY)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @}
  */

/** @addtogroup STM32H735G_DK_OSPI_RAM_Exported_Functions
  * @{
  */

/**
  * @brief  Initializes the OSPI interface.
  * @param  Instance   OSPI Instance
  * @param  Init       OSPI Init structure
  * @retval BSP status
  */
int32_t BSP_OSPI_RAM_Init(uint32_t Instance, BSP_OSPI_RAM_Init_t *Init)
{
  int32_t ret;
  MX_OSPI_InitTypeDef ospi_init;

  /* Check if the instance is supported */
  if (Instance >= OSPI_RAM_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Check if the instance is already initialized */
    if (Ospi_Ram_Ctx[Instance].IsInitialized == OSPI_ACCESS_NONE)
    {
#if (USE_HAL_OSPI_REGISTER_CALLBACKS == 0)
      /* Msp OSPI initialization */
      OSPI_RAM_MspInit(&hospi_ram[Instance]);
#else
      /* Register the OSPI MSP Callbacks */
      if (Ospi_Ram_Ctx[Instance].IsMspCallbacksValid == 0UL)
      {
        if (BSP_OSPI_RAM_RegisterDefaultMspCallbacks(Instance) != BSP_ERROR_NONE)
        {
          return BSP_ERROR_PERIPH_FAILURE;
        }
      }
#endif /* USE_HAL_OSPI_REGISTER_CALLBACKS */

      /* Fill config structure */
      ospi_init.ClockPrescaler = 2U; /* OctoSPI clock = 200 MHz / ClockPrescaler = 100 MHz */
      ospi_init.MemorySize     = (uint32_t)POSITION_VAL(S70KL1281_RAM_SIZE);
      ospi_init.SampleShifting = HAL_OSPI_SAMPLE_SHIFTING_NONE;

      /* STM32 OSPI Clock configuration */
      if (MX_OSPI_ClockConfig(&hospi_ram[Instance]) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      /* STM32 OSPI interface initialization */
      else if (MX_OSPI_RAM_Init(&hospi_ram[Instance], &ospi_init) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      /* Configure the memory */
      else if (BSP_OSPI_RAM_ConfigHyperRAM(Instance, Init->LatencyType, Init->BurstType, Init->BurstLength) != BSP_ERROR_NONE)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else
      {
        ret = BSP_ERROR_NONE;
      }
    }
    else
    {
      ret = BSP_ERROR_NONE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  De-Initializes the OSPI interface.
  * @param  Instance   OSPI Instance
  * @retval BSP status
  */
int32_t BSP_OSPI_RAM_DeInit(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if (Instance >= OSPI_RAM_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Check if the instance is already initialized */
    if (Ospi_Ram_Ctx[Instance].IsInitialized != OSPI_ACCESS_NONE)
    {
      /* Disable Memory mapped mode */
      if (Ospi_Ram_Ctx[Instance].IsInitialized == OSPI_ACCESS_MMP)
      {
        if (BSP_OSPI_RAM_DisableMemoryMappedMode(Instance) != BSP_ERROR_NONE)
        {
          return BSP_ERROR_COMPONENT_FAILURE;
        }
      }

      /* Set default Ospi_Ram_Ctx values */
      Ospi_Ram_Ctx[Instance].IsInitialized = OSPI_ACCESS_NONE;
      Ospi_Ram_Ctx[Instance].LatencyType   = BSP_OSPI_RAM_FIXED_LATENCY;
      Ospi_Ram_Ctx[Instance].BurstType     = BSP_OSPI_RAM_LINEAR_BURST;
      Ospi_Ram_Ctx[Instance].BurstLength   = BSP_OSPI_RAM_BURST_32_BYTES;

#if (USE_HAL_OSPI_REGISTER_CALLBACKS == 0)
      OSPI_RAM_MspDeInit(&hospi_ram[Instance]);
#endif /* (USE_HAL_OSPI_REGISTER_CALLBACKS == 0) */

      /* Call the DeInit function to reset the driver */
      if (HAL_OSPI_DeInit(&hospi_ram[Instance]) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  OSPI Clock Config for OSPI Memories .
  * @param  hospi  OSPI Handle
  *         Being __weak it can be overwritten by the application
  * @retval HAL_status
  */
__weak HAL_StatusTypeDef MX_OSPI_ClockConfig(OSPI_HandleTypeDef *hospi)
{
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

  /* HyperRAM clock configuration */
  /* OSPI clock configuration */
  /* PLL2_VCO Input = HSE_VALUE/PLL3M = 5 Mhz */
  /* PLL2_VCO Output = PLL2_VCO Input * PLL3N = 400 Mhz */
  /* PLLOSPICLK = PLL2_VCO Output/PLL2R = 400/2 = 200 Mhz */
  /* OSPI clock frequency = PLLOSPICLK = 200 Mhz */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_OSPI ;
  PeriphClkInitStruct.OspiClockSelection = RCC_OSPICLKSOURCE_PLL2;
  PeriphClkInitStruct.PLL2.PLL2M = 5;
  PeriphClkInitStruct.PLL2.PLL2N = 80;
  PeriphClkInitStruct.PLL2.PLL2P = 5;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_0;

  return HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
}

/**
  * @brief  Initializes the OSPI interface.
  * @param  hospi          OSPI handle
  * @param  Init           OSPI config structure
  * @retval BSP status
  */
__weak HAL_StatusTypeDef MX_OSPI_RAM_Init(OSPI_HandleTypeDef *hospi, MX_OSPI_InitTypeDef *Init)
{
  OSPI_HyperbusCfgTypeDef sHyperbusCfg;
  HAL_StatusTypeDef status;

  hospi->Instance = OCTOSPI2;
  HAL_OSPI_DeInit(hospi);

  hospi->Init.FifoThreshold         = 4;
  hospi->Init.DualQuad              = HAL_OSPI_DUALQUAD_DISABLE;
  hospi->Init.MemoryType            = HAL_OSPI_MEMTYPE_HYPERBUS;
  hospi->Init.DeviceSize            = Init->MemorySize;
  hospi->Init.ChipSelectHighTime    = 4;
  hospi->Init.FreeRunningClock      = HAL_OSPI_FREERUNCLK_DISABLE;
  hospi->Init.ClockMode             = HAL_OSPI_CLOCK_MODE_0;
  hospi->Init.WrapSize              = HAL_OSPI_WRAP_NOT_SUPPORTED;
  hospi->Init.ClockPrescaler        = Init->ClockPrescaler;
  hospi->Init.SampleShifting        = Init->SampleShifting;
  hospi->Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_ENABLE;
  hospi->Init.Refresh               = 400; /*4us @100MHz*/
  hospi->Init.ChipSelectBoundary    = 23;  /* memory die boundary 2^23=8MBs*/

  status = HAL_OSPI_Init(hospi);
  if (status == HAL_OK)
  {
    sHyperbusCfg.RWRecoveryTime   = RW_RECOVERY_TIME;
    sHyperbusCfg.AccessTime       = DEFAULT_INITIAL_LATENCY;
    sHyperbusCfg.WriteZeroLatency = HAL_OSPI_LATENCY_ON_WRITE;
    sHyperbusCfg.LatencyMode      = HAL_OSPI_FIXED_LATENCY;

    status = HAL_OSPI_HyperbusCfg(hospi, &sHyperbusCfg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
  }

  return status;
}

#if (USE_HAL_OSPI_REGISTER_CALLBACKS == 1)
/**
  * @brief Default BSP OSPI Msp Callbacks
  * @param Instance      OSPI Instance
  * @retval BSP status
  */
int32_t BSP_OSPI_RAM_RegisterDefaultMspCallbacks(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if (Instance >= OSPI_RAM_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Register MspInit/MspDeInit Callbacks */
    if (HAL_OSPI_RegisterCallback(&hospi_ram[Instance], HAL_OSPI_MSP_INIT_CB_ID, OSPI_RAM_MspInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if (HAL_OSPI_RegisterCallback(&hospi_ram[Instance], HAL_OSPI_MSP_DEINIT_CB_ID, OSPI_RAM_MspDeInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      Ospi_Ram_Ctx[Instance].IsMspCallbacksValid = 1U;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief BSP OSPI Msp Callback registering
  * @param Instance     OSPI Instance
  * @param CallBacks    pointer to MspInit/MspDeInit callbacks functions
  * @retval BSP status
  */
int32_t BSP_OSPI_RAM_RegisterMspCallbacks(uint32_t Instance, BSP_OSPI_Cb_t *CallBacks)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if (Instance >= OSPI_RAM_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Register MspInit/MspDeInit Callbacks */
    if (HAL_OSPI_RegisterCallback(&hospi_ram[Instance], HAL_OSPI_MSP_INIT_CB_ID, CallBacks->pMspInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if (HAL_OSPI_RegisterCallback(&hospi_ram[Instance], HAL_OSPI_MSP_DEINIT_CB_ID, CallBacks->pMspDeInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      Ospi_Ram_Ctx[Instance].IsMspCallbacksValid = 1U;
    }
  }

  /* Return BSP status */
  return ret;
}
#endif /* (USE_HAL_OSPI_REGISTER_CALLBACKS == 1) */

/**
  * @brief  Reads an amount of data from the HyperRAM memory.
  * @param  Instance  OSPI instance
  * @param  pData     Pointer to data to be read
  * @param  ReadAddr  Read start address
  * @param  Size      Size of data to read
  * @retval BSP status
  */
int32_t BSP_OSPI_RAM_Read(uint32_t Instance, uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
  int32_t ret;

  /* Check if the instance is supported */
  if (Instance >= OSPI_RAM_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if (S70KL1281_Read(&hospi_ram[Instance], pData, ReadAddr, Size) != S70KL1281_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      ret = BSP_ERROR_NONE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Reads an amount of data from the HyperRAM memory in DMA mode.
  * @param  Instance  OSPI instance
  * @param  pData     Pointer to data to be read
  * @param  ReadAddr  Read start address
  * @param  Size      Size of data to read
  * @retval BSP status
  */
int32_t BSP_OSPI_RAM_Read_DMA(uint32_t Instance, uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
  int32_t ret;

  /* Check if the instance is supported */
  if (Instance >= OSPI_RAM_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if (S70KL1281_Read_DMA(&hospi_ram[Instance], pData, ReadAddr, Size) != S70KL1281_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      ret = BSP_ERROR_NONE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Writes an amount of data to the HyperRAM memory.
  * @param  Instance  OSPI instance
  * @param  pData     Pointer to data to be written
  * @param  WriteAddr Write start address
  * @param  Size      Size of data to write
  * @retval BSP status
  */
int32_t BSP_OSPI_RAM_Write(uint32_t Instance, uint8_t *pData, uint32_t WriteAddr, uint32_t Size)
{
  int32_t ret;

  /* Check if the instance is supported */
  if (Instance >= OSPI_RAM_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if (S70KL1281_Write(&hospi_ram[Instance], pData, WriteAddr, Size) != S70KL1281_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      ret = BSP_ERROR_NONE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Writes an amount of data to the HyperRAM memory in DMA mode.
  * @param  Instance  OSPI instance
  * @param  pData     Pointer to data to be written
  * @param  WriteAddr Write start address
  * @param  Size      Size of data to write
  * @retval BSP status
  */
int32_t BSP_OSPI_RAM_Write_DMA(uint32_t Instance, uint8_t *pData, uint32_t WriteAddr, uint32_t Size)
{
  int32_t ret;

  /* Check if the instance is supported */
  if (Instance >= OSPI_RAM_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if (S70KL1281_Write_DMA(&hospi_ram[Instance], pData, WriteAddr, Size) != S70KL1281_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      ret = BSP_ERROR_NONE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Configure the OSPI in memory-mapped mode
  * @param  Instance  OSPI instance
  * @retval BSP status
  */
int32_t BSP_OSPI_RAM_EnableMemoryMappedMode(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if (Instance >= OSPI_RAM_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if (S70KL1281_EnableMemoryMappedMode(&hospi_ram[Instance]) != S70KL1281_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else /* Update OSPI context if all operations are well done */
    {
      Ospi_Ram_Ctx[Instance].IsInitialized = OSPI_ACCESS_MMP;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Exit form memory-mapped mode
  *         Only 1 Instance can running MMP mode. And it will lock system at this mode.
  * @param  Instance  OSPI instance
  * @retval BSP status
  */
int32_t BSP_OSPI_RAM_DisableMemoryMappedMode(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if (Instance >= OSPI_RAM_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if (Ospi_Ram_Ctx[Instance].IsInitialized != OSPI_ACCESS_MMP)
    {
      ret = BSP_ERROR_OSPI_MMP_UNLOCK_FAILURE;
    }
    /* Abort MMP back to indirect mode */
    else if (HAL_OSPI_Abort(&hospi_ram[Instance]) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    /* Update OSPI HyperRAM context if all operations are well done */
    else
    {
      Ospi_Ram_Ctx[Instance].IsInitialized = OSPI_ACCESS_INDIRECT;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set HyperRAM to desired configuration. And this instance becomes current instance.
  *         If current instance running at MMP mode then this function doesn't work.
  *         Indirect -> Indirect
  * @param  Instance    OSPI instance
  * @param  Latency     OSPI latency mode
  * @param  BurstType   OSPI burst type
  * @param  BurstLength OSPI burst length
  * @retval BSP status
  */
int32_t BSP_OSPI_RAM_ConfigHyperRAM(uint32_t Instance, BSP_OSPI_RAM_Latency_t Latency,
                                    BSP_OSPI_RAM_BurstType_t BurstType, BSP_OSPI_RAM_BurstLength_t BurstLength)
{
  OSPI_HyperbusCfgTypeDef sHyperbusCfg;
  int32_t ret = BSP_ERROR_NONE;
  uint32_t initial_latency, latency_mode;
  uint16_t reg;

  /* Check if the instance is supported */
  if (Instance >= OSPI_RAM_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Check if MMP mode locked ***********************************************/
    if (Ospi_Ram_Ctx[Instance].IsInitialized == OSPI_ACCESS_MMP)
    {
      ret = BSP_ERROR_OSPI_MMP_LOCK_FAILURE;
    }
    else if ((Ospi_Ram_Ctx[Instance].IsInitialized == OSPI_ACCESS_NONE) ||
             (Ospi_Ram_Ctx[Instance].LatencyType   != Latency)              ||
             (Ospi_Ram_Ctx[Instance].BurstType     != BurstType)            ||
             (Ospi_Ram_Ctx[Instance].BurstLength   != BurstLength))
    {
      /* Reading the configuration of the HyperRAM ****************************/
      if (S70KL1281_ReadCfgReg0(&hospi_ram[Instance], &reg) != S70KL1281_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else
      {
        /* Configure the HyperRAM memory **************************************/
        /* Initial latency */
        if (Ospi_Ram_Ctx[Instance].IsInitialized == OSPI_ACCESS_NONE)
        {
          initial_latency = DEFAULT_INITIAL_LATENCY;
          latency_mode = HAL_OSPI_FIXED_LATENCY;
        }
        else
        {
          ret = BSP_ERROR_WRONG_PARAM;
        }

        /* check Latency Type */
        if (Latency != BSP_OSPI_RAM_FIXED_LATENCY)
        {
          ret = BSP_ERROR_WRONG_PARAM;
        }

        /* Burst type */
        if (BurstType == BSP_OSPI_RAM_HYBRID_BURST)
        {
          CLEAR_BIT(reg, S70KL1281_CR0_HBE);
        }
        else
        {
          SET_BIT(reg, S70KL1281_CR0_HBE);
        }

        /* Burst length */
        MODIFY_REG(reg, (uint16_t) S70KL1281_CR0_BLENGTH, (uint16_t) BurstLength);

        if (S70KL1281_WriteCfgReg0(&hospi_ram[Instance], reg, latency_mode, initial_latency) != S70KL1281_OK)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }
        else
        {
          /* Configure the peripheral *****************************************/
          if (BurstType == BSP_OSPI_RAM_HYBRID_BURST)
          {
            switch (BurstLength)
            {
              case BSP_OSPI_RAM_BURST_16_BYTES :
                hospi_ram[Instance].Init.WrapSize = HAL_OSPI_WRAP_16_BYTES;
                break;
              case BSP_OSPI_RAM_BURST_32_BYTES :
                hospi_ram[Instance].Init.WrapSize = HAL_OSPI_WRAP_32_BYTES;
                break;
              case BSP_OSPI_RAM_BURST_64_BYTES :
                hospi_ram[Instance].Init.WrapSize = HAL_OSPI_WRAP_64_BYTES;
                break;
              case BSP_OSPI_RAM_BURST_128_BYTES :
                hospi_ram[Instance].Init.WrapSize = HAL_OSPI_WRAP_128_BYTES;
                break;
              default :
                break;
            }
          }
          else
          {
            hospi_ram[Instance].Init.WrapSize = HAL_OSPI_WRAP_NOT_SUPPORTED;
          }
          if (HAL_OSPI_Init(&hospi_ram[Instance]) != HAL_OK)
          {
            ret = BSP_ERROR_PERIPH_FAILURE;
          }
          else
          {
            sHyperbusCfg.RWRecoveryTime   = RW_RECOVERY_TIME;
            sHyperbusCfg.AccessTime       = DEFAULT_INITIAL_LATENCY;
            sHyperbusCfg.WriteZeroLatency = HAL_OSPI_LATENCY_ON_WRITE;
            sHyperbusCfg.LatencyMode      = HAL_OSPI_FIXED_LATENCY;

            if (HAL_OSPI_HyperbusCfg(&hospi_ram[Instance], &sHyperbusCfg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
            {
              ret = BSP_ERROR_PERIPH_FAILURE;
            }
          }
        }
      }

      /* Update OSPI context if all operations are well done */
      if (ret == BSP_ERROR_NONE)
      {
        /* Update current status parameter *****************************************/
        Ospi_Ram_Ctx[Instance].IsInitialized = OSPI_ACCESS_INDIRECT;
        Ospi_Ram_Ctx[Instance].LatencyType   = BSP_OSPI_RAM_FIXED_LATENCY;
        Ospi_Ram_Ctx[Instance].BurstType     = BurstType;
        Ospi_Ram_Ctx[Instance].BurstLength   = BurstLength;
      }
    }
    else
    {
      /* Nothing to do */
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Handles OctoSPI HyperRAM DMA transfer interrupt request.
  * @param  Instance OSPI instance
  * @retval None
  */
void BSP_OSPI_RAM_DMA_IRQHandler(uint32_t Instance)
{
  HAL_MDMA_IRQHandler(hospi_ram[Instance].hmdma);
}

/**
  * @brief  Handles OctoSPI HyperRAM interrupt request.
  * @param  Instance OSPI instance
  * @retval None
  */
void BSP_OSPI_RAM_IRQHandler(uint32_t Instance)
{
  HAL_OSPI_IRQHandler(&hospi_ram[Instance]);
}
/**
  * @}
  */

/** @addtogroup STM32H735G_DKVERY_OSPI_NOR_Private_Functions
  * @{
  */

/**
  * @brief  Initializes the OSPI MSP.
  * @param  hospi OSPI handle
  * @retval None
  */
static void OSPI_NOR_MspInit(OSPI_HandleTypeDef *hospi)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* hospi unused argument(s) compilation warning */
  UNUSED(hospi);

  /* Enable the OctoSPI memory interface clock */
  OSPI_NOR_CLK_ENABLE();

  /* Reset the OctoSPI memory interface */
  OSPI_NOR_FORCE_RESET();
  OSPI_NOR_RELEASE_RESET();

  /* Enable GPIO clocks */
  OSPI_NOR_CLK_GPIO_CLK_ENABLE();
  OSPI_NOR_DQS_GPIO_CLK_ENABLE();
  OSPI_NOR_CS_GPIO_CLK_ENABLE();
  OSPI_NOR_D0_GPIO_CLK_ENABLE();
  OSPI_NOR_D1_GPIO_CLK_ENABLE();
  OSPI_NOR_D2_GPIO_CLK_ENABLE();
  OSPI_NOR_D3_GPIO_CLK_ENABLE();
  OSPI_NOR_D4_GPIO_CLK_ENABLE();
  OSPI_NOR_D5_GPIO_CLK_ENABLE();
  OSPI_NOR_D6_GPIO_CLK_ENABLE();
  OSPI_NOR_D7_GPIO_CLK_ENABLE();

  /* OctoSPI CS GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI_NOR_CS_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = OSPI_NOR_CS_PIN_AF;
  HAL_GPIO_Init(OSPI_NOR_CS_GPIO_PORT, &GPIO_InitStruct);

  /* OctoSPI DQS GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI_NOR_DQS_PIN;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Alternate = OSPI_NOR_DQS_PIN_AF;
  HAL_GPIO_Init(OSPI_NOR_DQS_GPIO_PORT, &GPIO_InitStruct);

  /* OctoSPI CLK GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI_NOR_CLK_PIN;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Alternate = OSPI_NOR_CLK_PIN_AF;
  HAL_GPIO_Init(OSPI_NOR_CLK_GPIO_PORT, &GPIO_InitStruct);

  /* OctoSPI D0 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI_NOR_D0_PIN;
  GPIO_InitStruct.Alternate = OSPI_NOR_D0_PIN_AF;
  HAL_GPIO_Init(OSPI_NOR_D0_GPIO_PORT, &GPIO_InitStruct);

  /* OctoSPI D1 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI_NOR_D1_PIN;
  GPIO_InitStruct.Alternate = OSPI_NOR_D1_PIN_AF;
  HAL_GPIO_Init(OSPI_NOR_D1_GPIO_PORT, &GPIO_InitStruct);

  /* OctoSPI D2 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI_NOR_D2_PIN;
  GPIO_InitStruct.Alternate = OSPI_NOR_D2_PIN_AF;
  HAL_GPIO_Init(OSPI_NOR_D2_GPIO_PORT, &GPIO_InitStruct);

  /* OctoSPI D3 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI_NOR_D3_PIN;
  GPIO_InitStruct.Alternate = OSPI_NOR_D3_PIN_AF;
  HAL_GPIO_Init(OSPI_NOR_D3_GPIO_PORT, &GPIO_InitStruct);

  /* OctoSPI D4 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI_NOR_D4_PIN;
  GPIO_InitStruct.Alternate = OSPI_NOR_D4_PIN_AF;
  HAL_GPIO_Init(OSPI_NOR_D4_GPIO_PORT, &GPIO_InitStruct);

  /* OctoSPI D5 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI_NOR_D5_PIN;
  GPIO_InitStruct.Alternate = OSPI_NOR_D5_PIN_AF;
  HAL_GPIO_Init(OSPI_NOR_D5_GPIO_PORT, &GPIO_InitStruct);

  /* OctoSPI D6 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI_NOR_D6_PIN;
  GPIO_InitStruct.Alternate = OSPI_NOR_D6_PIN_AF;
  HAL_GPIO_Init(OSPI_NOR_D6_GPIO_PORT, &GPIO_InitStruct);

  /* OctoSPI D7 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI_NOR_D7_PIN;
  GPIO_InitStruct.Alternate = OSPI_NOR_D7_PIN_AF;
  HAL_GPIO_Init(OSPI_NOR_D7_GPIO_PORT, &GPIO_InitStruct);
}

/**
  * @brief  De-Initializes the OSPI MSP.
  * @param  hospi OSPI handle
  * @retval None
  */
static void OSPI_NOR_MspDeInit(OSPI_HandleTypeDef *hospi)
{
  /* hospi unused argument(s) compilation warning */
  UNUSED(hospi);

  /* OctoSPI GPIO pins de-configuration  */
  HAL_GPIO_DeInit(OSPI_NOR_CLK_GPIO_PORT, OSPI_NOR_CLK_PIN);
  HAL_GPIO_DeInit(OSPI_NOR_DQS_GPIO_PORT, OSPI_NOR_DQS_PIN);
  HAL_GPIO_DeInit(OSPI_NOR_CS_GPIO_PORT, OSPI_NOR_CS_PIN);
  HAL_GPIO_DeInit(OSPI_NOR_D0_GPIO_PORT, OSPI_NOR_D0_PIN);
  HAL_GPIO_DeInit(OSPI_NOR_D1_GPIO_PORT, OSPI_NOR_D1_PIN);
  HAL_GPIO_DeInit(OSPI_NOR_D2_GPIO_PORT, OSPI_NOR_D2_PIN);
  HAL_GPIO_DeInit(OSPI_NOR_D3_GPIO_PORT, OSPI_NOR_D3_PIN);
  HAL_GPIO_DeInit(OSPI_NOR_D4_GPIO_PORT, OSPI_NOR_D4_PIN);
  HAL_GPIO_DeInit(OSPI_NOR_D5_GPIO_PORT, OSPI_NOR_D5_PIN);
  HAL_GPIO_DeInit(OSPI_NOR_D6_GPIO_PORT, OSPI_NOR_D6_PIN);
  HAL_GPIO_DeInit(OSPI_NOR_D7_GPIO_PORT, OSPI_NOR_D7_PIN);

  /* Reset the OctoSPI memory interface */
  OSPI_NOR_FORCE_RESET();
  OSPI_NOR_RELEASE_RESET();

  /* Disable the OctoSPI memory interface clock */
  OSPI_NOR_CLK_DISABLE();
}

/**
  * @brief  This function reset the OSPI memory.
  * @param  Instance  OSPI instance
  * @retval BSP status
  */
static int32_t OSPI_NOR_ResetMemory(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if (Instance >= OSPI_NOR_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if (MX25LM51245G_ResetEnable(&hospi_nor[Instance], BSP_OSPI_NOR_SPI_MODE,
                                    BSP_OSPI_NOR_STR_TRANSFER) != MX25LM51245G_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else if (MX25LM51245G_ResetMemory(&hospi_nor[Instance], BSP_OSPI_NOR_SPI_MODE,
                                    BSP_OSPI_NOR_STR_TRANSFER) != MX25LM51245G_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else if (MX25LM51245G_ResetEnable(&hospi_nor[Instance], BSP_OSPI_NOR_OPI_MODE,
                                    BSP_OSPI_NOR_STR_TRANSFER) != MX25LM51245G_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else if (MX25LM51245G_ResetMemory(&hospi_nor[Instance], BSP_OSPI_NOR_OPI_MODE,
                                    BSP_OSPI_NOR_STR_TRANSFER) != MX25LM51245G_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else if (MX25LM51245G_ResetEnable(&hospi_nor[Instance], BSP_OSPI_NOR_OPI_MODE,
                                    BSP_OSPI_NOR_DTR_TRANSFER) != MX25LM51245G_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else if (MX25LM51245G_ResetMemory(&hospi_nor[Instance], BSP_OSPI_NOR_OPI_MODE,
                                    BSP_OSPI_NOR_DTR_TRANSFER) != MX25LM51245G_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    Ospi_Nor_Ctx[Instance].IsInitialized = OSPI_ACCESS_INDIRECT;      /* After reset S/W setting to indirect access   */
    Ospi_Nor_Ctx[Instance].InterfaceMode = BSP_OSPI_NOR_SPI_MODE;         /* After reset H/W back to SPI mode by default  */
    Ospi_Nor_Ctx[Instance].TransferRate  = BSP_OSPI_NOR_STR_TRANSFER;     /* After reset S/W setting to STR mode          */

    /* After SWreset CMD, wait in case SWReset occurred during erase operation */
    HAL_Delay(MX25LM51245G_RESET_MAX_TIME);
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  This function enables the octal DTR mode of the memory.
  * @param  Instance  OSPI instance
  * @retval BSP status
  */
static int32_t OSPI_NOR_EnterDOPIMode(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;
  uint8_t reg[2];

  /* Check if the instance is supported */
  if (Instance >= OSPI_NOR_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  /* Enable write operations */
  else if (MX25LM51245G_WriteEnable(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                    Ospi_Nor_Ctx[Instance].TransferRate) != MX25LM51245G_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  /* Write Configuration register 2 (with new dummy cycles) */
  else if (MX25LM51245G_WriteCfg2Register(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                          Ospi_Nor_Ctx[Instance].TransferRate, MX25LM51245G_CR2_REG3_ADDR, CONF_OSPI_DUMMY_CYCLES) != MX25LM51245G_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  /* Enable write operations */
  else if (MX25LM51245G_WriteEnable(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                    Ospi_Nor_Ctx[Instance].TransferRate) != MX25LM51245G_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  /* Write Configuration register 2 (with Octal I/O SPI protocol) */
  else if (MX25LM51245G_WriteCfg2Register(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                          Ospi_Nor_Ctx[Instance].TransferRate, MX25LM51245G_CR2_REG1_ADDR, MX25LM51245G_CR2_DOPI) != MX25LM51245G_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    /* Wait that the configuration is effective and check that memory is ready */
    HAL_Delay(MX25LM51245G_WRITE_REG_MAX_TIME);

    /* Reconfigure the memory type of the peripheral */
    hospi_nor[Instance].Init.MemoryType            = HAL_OSPI_MEMTYPE_MACRONIX;
    hospi_nor[Instance].Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_ENABLE;
    if (HAL_OSPI_Init(&hospi_nor[Instance]) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    /* Check Flash busy ? */
    else if (MX25LM51245G_AutoPollingMemReady(&hospi_nor[Instance], BSP_OSPI_NOR_OPI_MODE,
                                              BSP_OSPI_NOR_DTR_TRANSFER) != MX25LM51245G_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    /* Check the configuration has been correctly done */
    else if (MX25LM51245G_ReadCfg2Register(&hospi_nor[Instance], BSP_OSPI_NOR_OPI_MODE, BSP_OSPI_NOR_DTR_TRANSFER,
                                           MX25LM51245G_CR2_REG1_ADDR, reg) != MX25LM51245G_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else if (reg[0] != MX25LM51245G_CR2_DOPI)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      ret = BSP_ERROR_NONE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  This function enables the octal STR mode of the memory.
  * @param  Instance  OSPI instance
  * @retval BSP status
  */
static int32_t OSPI_NOR_EnterSOPIMode(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;
  uint8_t reg[2];

  /* Check if the instance is supported */
  if (Instance >= OSPI_NOR_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  /* Enable write operations */
  else if (MX25LM51245G_WriteEnable(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                    Ospi_Nor_Ctx[Instance].TransferRate) != MX25LM51245G_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  /* Write Configuration register 2 (with new dummy cycles) */
  else if (MX25LM51245G_WriteCfg2Register(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                          Ospi_Nor_Ctx[Instance].TransferRate, MX25LM51245G_CR2_REG3_ADDR, CONF_OSPI_DUMMY_CYCLES) != MX25LM51245G_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  /* Enable write operations */
  else if (MX25LM51245G_WriteEnable(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                    Ospi_Nor_Ctx[Instance].TransferRate) != MX25LM51245G_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  /* Write Configuration register 2 (with Octal I/O SPI protocol) */
  else if (MX25LM51245G_WriteCfg2Register(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                          Ospi_Nor_Ctx[Instance].TransferRate, MX25LM51245G_CR2_REG1_ADDR, MX25LM51245G_CR2_SOPI) != MX25LM51245G_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    /* Wait that the configuration is effective and check that memory is ready */
    HAL_Delay(MX25LM51245G_WRITE_REG_MAX_TIME);

    /* Check Flash busy ? */
    if (MX25LM51245G_AutoPollingMemReady(&hospi_nor[Instance], BSP_OSPI_NOR_OPI_MODE,
                                         BSP_OSPI_NOR_STR_TRANSFER) != MX25LM51245G_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    /* Check the configuration has been correctly done */
    else if (MX25LM51245G_ReadCfg2Register(&hospi_nor[Instance], BSP_OSPI_NOR_OPI_MODE, BSP_OSPI_NOR_STR_TRANSFER,
                                           MX25LM51245G_CR2_REG1_ADDR, reg) != MX25LM51245G_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else if (reg[0] != MX25LM51245G_CR2_SOPI)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      ret = BSP_ERROR_NONE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  This function disables the octal DTR or STR mode of the memory.
  * @param  Instance  OSPI instance
  * @retval BSP status
  */
static int32_t OSPI_NOR_ExitOPIMode(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;
  uint8_t reg[2];

  /* Check if the instance is supported */
  if (Instance >= OSPI_NOR_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  /* Enable write operations */
  else if (MX25LM51245G_WriteEnable(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                    Ospi_Nor_Ctx[Instance].TransferRate) != MX25LM51245G_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    /* Write Configuration register 2 (with SPI protocol) */
    reg[0] = 0;
    reg[1] = 0;
    if (MX25LM51245G_WriteCfg2Register(&hospi_nor[Instance], Ospi_Nor_Ctx[Instance].InterfaceMode,
                                       Ospi_Nor_Ctx[Instance].TransferRate, MX25LM51245G_CR2_REG1_ADDR, reg[0]) != MX25LM51245G_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      /* Wait that the configuration is effective and check that memory is ready */
      HAL_Delay(MX25LM51245G_WRITE_REG_MAX_TIME);

      if (Ospi_Nor_Ctx[Instance].TransferRate == BSP_OSPI_NOR_DTR_TRANSFER)
      {
        /* Reconfigure the memory type of the peripheral */
        hospi_nor[Instance].Init.MemoryType            = HAL_OSPI_MEMTYPE_MICRON;
        hospi_nor[Instance].Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_DISABLE;
        if (HAL_OSPI_Init(&hospi_nor[Instance]) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
      }

      /* Check Flash busy ? */
      if (MX25LM51245G_AutoPollingMemReady(&hospi_nor[Instance], BSP_OSPI_NOR_SPI_MODE,
                                           BSP_OSPI_NOR_STR_TRANSFER) != MX25LM51245G_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      /* Check the configuration has been correctly done */
      else if (MX25LM51245G_ReadCfg2Register(&hospi_nor[Instance], BSP_OSPI_NOR_SPI_MODE, BSP_OSPI_NOR_STR_TRANSFER,
                                             MX25LM51245G_CR2_REG1_ADDR, reg) != MX25LM51245G_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else if (reg[0] != 0)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else
      {
        ret = BSP_ERROR_NONE;
      }
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @}
  */

/** @defgroup STM32H735G_DKVERY_OSPI_RAM_Private_Functions OSPI_RAM Private Functions
  * @{
  */

/**
  * @brief  Initializes the OSPI MSP.
  * @param  hospi OSPI handle
  * @retval None
  */
static void OSPI_RAM_MspInit(OSPI_HandleTypeDef *hospi)
{
  static MDMA_HandleTypeDef mdma_handle;
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Enable the OctoSPI memory interface, DMA and GPIO clocks */
  OSPI_RAM_CLK_ENABLE();
  OSPI_RAM_MDMAx_CLK_ENABLE();
  OSPI_RAM_CLK_GPIO_CLK_ENABLE();
  OSPI_RAM_DQS_GPIO_CLK_ENABLE();
  OSPI_RAM_CS_GPIO_CLK_ENABLE();
  OSPI_RAM_D0_GPIO_CLK_ENABLE();
  OSPI_RAM_D1_GPIO_CLK_ENABLE();
  OSPI_RAM_D2_GPIO_CLK_ENABLE();
  OSPI_RAM_D3_GPIO_CLK_ENABLE();
  OSPI_RAM_D4_GPIO_CLK_ENABLE();
  OSPI_RAM_D5_GPIO_CLK_ENABLE();
  OSPI_RAM_D6_GPIO_CLK_ENABLE();
  OSPI_RAM_D7_GPIO_CLK_ENABLE();

  /* Reset the OctoSPI memory interface */
  OSPI_RAM_FORCE_RESET();
  OSPI_RAM_RELEASE_RESET();

  /* OctoSPI CS GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI_RAM_CS_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = OSPI_RAM_CS_PIN_AF;
  HAL_GPIO_Init(OSPI_RAM_CS_GPIO_PORT, &GPIO_InitStruct);

  /* OctoSPI DQS GPIO pin configuration  */
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Pin       = OSPI_RAM_DQS_PIN;
  GPIO_InitStruct.Alternate = OSPI_RAM_DQS_PIN_AF;
  HAL_GPIO_Init(OSPI_RAM_DQS_GPIO_PORT, &GPIO_InitStruct);

  /* OctoSPI CLK GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI_RAM_CLK_PIN;
  GPIO_InitStruct.Alternate = OSPI_RAM_CLK_PIN_AF;
  HAL_GPIO_Init(OSPI_RAM_CLK_GPIO_PORT, &GPIO_InitStruct);

  /* OctoSPI D0 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI_RAM_D0_PIN;
  GPIO_InitStruct.Alternate = OSPI_RAM_D0_PIN_AF;
  HAL_GPIO_Init(OSPI_RAM_D0_GPIO_PORT, &GPIO_InitStruct);

  /* OctoSPI D1 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI_RAM_D1_PIN;
  GPIO_InitStruct.Alternate = OSPI_RAM_D1_PIN_AF;
  HAL_GPIO_Init(OSPI_RAM_D1_GPIO_PORT, &GPIO_InitStruct);

  /* OctoSPI D2 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI_RAM_D2_PIN;
  GPIO_InitStruct.Alternate = OSPI_RAM_D2_PIN_AF;
  HAL_GPIO_Init(OSPI_RAM_D2_GPIO_PORT, &GPIO_InitStruct);

  /* OctoSPI D3 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI_RAM_D3_PIN;
  GPIO_InitStruct.Alternate = OSPI_RAM_D3_PIN_AF;
  HAL_GPIO_Init(OSPI_RAM_D3_GPIO_PORT, &GPIO_InitStruct);

  /* OctoSPI D4 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI_RAM_D4_PIN;
  GPIO_InitStruct.Alternate = OSPI_RAM_D4_PIN_AF;
  HAL_GPIO_Init(OSPI_RAM_D4_GPIO_PORT, &GPIO_InitStruct);

  /* OctoSPI D5 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI_RAM_D5_PIN;
  GPIO_InitStruct.Alternate = OSPI_RAM_D5_PIN_AF;
  HAL_GPIO_Init(OSPI_RAM_D5_GPIO_PORT, &GPIO_InitStruct);

  /* OctoSPI D6 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI_RAM_D6_PIN;
  GPIO_InitStruct.Alternate = OSPI_RAM_D6_PIN_AF;
  HAL_GPIO_Init(OSPI_RAM_D6_GPIO_PORT, &GPIO_InitStruct);

  /* OctoSPI D7 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI_RAM_D7_PIN;
  GPIO_InitStruct.Alternate = OSPI_RAM_D7_PIN_AF;
  HAL_GPIO_Init(OSPI_RAM_D7_GPIO_PORT, &GPIO_InitStruct);

  /* Configure the OctoSPI DMA */
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

  mdma_handle.Instance = OSPI_RAM_MDMAx_CHANNEL;

  __HAL_LINKDMA(hospi, hmdma, mdma_handle);
  (void) HAL_MDMA_Init(&mdma_handle);

  /* Enable and set priority of the OctoSPI and DMA interrupts */
  HAL_NVIC_SetPriority(OCTOSPI2_IRQn, BSP_OSPI_RAM_IT_PRIORITY, 0);
  HAL_NVIC_SetPriority(OSPI_RAM_MDMAx_IRQn, BSP_OSPI_RAM_DMA_IT_PRIORITY, 0);

  HAL_NVIC_EnableIRQ(OCTOSPI2_IRQn);
  HAL_NVIC_EnableIRQ(OSPI_RAM_MDMAx_IRQn);

}

/**
  * @brief  De-Initializes the OSPI MSP.
  * @param  hospi OSPI handle
  * @retval None
  */
static void OSPI_RAM_MspDeInit(OSPI_HandleTypeDef *hospi)
{
  static MDMA_HandleTypeDef mdma_handle;

  /* hospi unused argument(s) compilation warning */
  UNUSED(hospi);

  /* Disable DMA and OctoSPI interrupts */
  HAL_NVIC_DisableIRQ(OSPI_RAM_MDMAx_IRQn);
  HAL_NVIC_DisableIRQ(OCTOSPI2_IRQn);

  /* De-configure the OctoSPI DMA */
  mdma_handle.Instance = OSPI_RAM_MDMAx_CHANNEL;
  (void) HAL_MDMA_DeInit(&mdma_handle);

  /* OctoSPI GPIO pins de-configuration  */
  HAL_GPIO_DeInit(OSPI_RAM_CLK_GPIO_PORT, OSPI_RAM_CLK_PIN);
  HAL_GPIO_DeInit(OSPI_RAM_DQS_GPIO_PORT, OSPI_RAM_DQS_PIN);
  HAL_GPIO_DeInit(OSPI_RAM_CS_GPIO_PORT, OSPI_RAM_CS_PIN);
  HAL_GPIO_DeInit(OSPI_RAM_D0_GPIO_PORT, OSPI_RAM_D0_PIN);
  HAL_GPIO_DeInit(OSPI_RAM_D1_GPIO_PORT, OSPI_RAM_D1_PIN);
  HAL_GPIO_DeInit(OSPI_RAM_D2_GPIO_PORT, OSPI_RAM_D2_PIN);
  HAL_GPIO_DeInit(OSPI_RAM_D3_GPIO_PORT, OSPI_RAM_D3_PIN);
  HAL_GPIO_DeInit(OSPI_RAM_D4_GPIO_PORT, OSPI_RAM_D4_PIN);
  HAL_GPIO_DeInit(OSPI_RAM_D5_GPIO_PORT, OSPI_RAM_D5_PIN);
  HAL_GPIO_DeInit(OSPI_RAM_D6_GPIO_PORT, OSPI_RAM_D6_PIN);
  HAL_GPIO_DeInit(OSPI_RAM_D7_GPIO_PORT, OSPI_RAM_D7_PIN);

  /* Reset the OctoSPI memory interface */
  OSPI_RAM_FORCE_RESET();
  OSPI_RAM_RELEASE_RESET();

  /* Disable the OctoSPI memory interface clock */
  OSPI_RAM_CLK_DISABLE();
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
