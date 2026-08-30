                                                 /**
 ******************************************************************************
 * @file    is42s32800j.c
 * @author  MCD Application Team
 * @brief   is42s32800j sdram 8MBx32 driver file
 ******************************************************************************
 * @attention
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "is42s32800j.h"

/** @addtogroup BSP
  * @{
  */
  
/** @addtogroup Components
  * @{
  */ 

/** @defgroup IS42S32800J IS42S32800J
  * @brief     This file provides a set of functions needed to drive the 
  *            IS42S32800J SDRAM memory.
  * @{
  */
 
/** @defgroup IS42S32800J_Private_Variables IS42S32800J Private Variables
  * @{
  */
static FMC_SDRAM_CommandTypeDef Command;
/**
  * @}
  */ 

/** @defgroup IS42S32800J_Function_Prototypes IS42S32800J Function Prototypes
  * @{
  */
static int32_t IS42S32800J_Delay(uint32_t Delay);

/**
  * @}
  */ 

/** @defgroup IS42S32800J_Exported_Functions IS42S32800J Exported Functions
  * @{
  */
/**
  * @brief  Initializes the IS42S32800J SDRAm memory
  * @param  Ctx Component object pointer
  * @param  pRegMode : Pointer to Register Mode structure
  * @retval error status
  */
int32_t IS42S32800J_Init(SDRAM_HandleTypeDef *Ctx, IS42S32800J_Context_t *pRegMode) 
{
  int32_t ret = IS42S32800J_ERROR;
  
  /* Step 1: Configure a clock configuration enable command */
  if(IS42S32800J_ClockEnable(Ctx, pRegMode->TargetBank) == IS42S32800J_OK)
  {
    /* Step 2: Insert 100 us minimum delay */ 
    /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
    (void)IS42S32800J_Delay(1);
    
    /* Step 3: Configure a PALL (precharge all) command */ 
    if(IS42S32800J_Precharge(Ctx, pRegMode->TargetBank) == IS42S32800J_OK)
    {
      /* Step 4: Configure a Refresh command */ 
      if(IS42S32800J_RefreshMode(Ctx, pRegMode->TargetBank, pRegMode->RefreshMode) == IS42S32800J_OK)
      {
        /* Step 5: Program the external memory mode register */
        if(IS42S32800J_ModeRegConfig(Ctx, pRegMode) == IS42S32800J_OK)
        {
          /* Step 6: Set the refresh rate counter */
          if(IS42S32800J_RefreshRate(Ctx, pRegMode->RefreshRate) == IS42S32800J_OK)
          {
            ret = IS42S32800J_OK;
          }
        }
      }
    }
  } 
  return ret;
}

/**
  * @brief  Enable SDRAM clock
  * @param  Ctx Component object pointer
  * @param  Interface Could be FMC_SDRAM_CMD_TARGET_BANK1 or FMC_SDRAM_CMD_TARGET_BANK2
  * @retval error status
  */
int32_t IS42S32800J_ClockEnable(SDRAM_HandleTypeDef *Ctx, uint32_t Interface) 
{
  Command.CommandMode            = IS42S32800J_CLK_ENABLE_CMD;
  Command.CommandTarget          = Interface;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  if(HAL_SDRAM_SendCommand(Ctx, &Command, IS42S32800J_TIMEOUT) != HAL_OK)
  {
    return IS42S32800J_ERROR;
  }
  else
  {
    return IS42S32800J_OK;
  }
}

/**
  * @brief  Precharge all sdram banks
  * @param  Ctx Component object pointer
  * @param  Interface Could be FMC_SDRAM_CMD_TARGET_BANK1 or FMC_SDRAM_CMD_TARGET_BANK2
  * @retval error status
  */
int32_t IS42S32800J_Precharge(SDRAM_HandleTypeDef *Ctx, uint32_t Interface) 
{
  Command.CommandMode            = IS42S32800J_PALL_CMD;
  Command.CommandTarget          = Interface;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  if(HAL_SDRAM_SendCommand(Ctx, &Command, IS42S32800J_TIMEOUT) != HAL_OK)
  {
    return IS42S32800J_ERROR;
  }
  else
  {
    return IS42S32800J_OK;
  }
}

/**
  * @brief  Program the external memory mode register
  * @param  Ctx Component object pointer
  * @param  pRegMode : Pointer to Register Mode structure
  * @retval error status
  */
int32_t IS42S32800J_ModeRegConfig(SDRAM_HandleTypeDef *Ctx, IS42S32800J_Context_t *pRegMode) 
{
  uint32_t tmpmrd;

  /* Program the external memory mode register */
  tmpmrd = (uint32_t)pRegMode->BurstLength   |\
                     pRegMode->BurstType     |\
                     pRegMode->CASLatency    |\
                     pRegMode->OperationMode |\
                     pRegMode->WriteBurstMode;
  
  Command.CommandMode            = IS42S32800J_LOAD_MODE_CMD;
  Command.CommandTarget          = pRegMode->TargetBank;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = tmpmrd;
  
  /* Send the command */
  if(HAL_SDRAM_SendCommand(Ctx, &Command, IS42S32800J_TIMEOUT) != HAL_OK)
  {
    return IS42S32800J_ERROR;
  }
  else
  {
    return IS42S32800J_OK;
  }
}

/**
  * @brief  Program the SDRAM timing
  * @param  Ctx Component object pointer
  * @param  pTiming Pointer to SDRAM timing configuration structure
  * @retval error status
  */
int32_t IS42S32800J_TimingConfig(SDRAM_HandleTypeDef *Ctx, FMC_SDRAM_TimingTypeDef *pTiming) 
{
  /* Program the SDRAM timing */
  if(HAL_SDRAM_Init(Ctx, pTiming) != HAL_OK)
  {
    return IS42S32800J_ERROR;
  }
  else
  {
    return IS42S32800J_OK;
  }
}

/**
  * @brief  Configure Refresh mode
  * @param  Ctx Component object pointer
  * @param  Interface Could be FMC_SDRAM_CMD_TARGET_BANK1 or FMC_SDRAM_CMD_TARGET_BANK2 
  * @param  RefreshMode Could be IS42S32800J_CMD_AUTOREFRESH_MODE or
  *                      IS42S32800J_CMD_SELFREFRESH_MODE
  * @retval error status
  */
int32_t IS42S32800J_RefreshMode(SDRAM_HandleTypeDef *Ctx, uint32_t Interface, uint32_t RefreshMode) 
{
  Command.CommandMode            = RefreshMode;
  Command.CommandTarget          = Interface;
  Command.AutoRefreshNumber      = 8;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  if(HAL_SDRAM_SendCommand(Ctx, &Command, IS42S32800J_TIMEOUT) != HAL_OK)
  {
    return IS42S32800J_ERROR;
  }
  else
  {
    return IS42S32800J_OK;
  }
}

/**
  * @brief  Set the device refresh rate
  * @param  Ctx Component object pointer
  * @param  RefreshCount The refresh rate to be programmed
  * @retval error status
  */
int32_t IS42S32800J_RefreshRate(SDRAM_HandleTypeDef *Ctx, uint32_t RefreshCount) 
{
  /* Set the device refresh rate */
  if(HAL_SDRAM_ProgramRefreshRate(Ctx, RefreshCount) != HAL_OK)
  {
    return IS42S32800J_ERROR;
  }
  else
  {
    return IS42S32800J_OK;
  }
}

/**
  * @brief  Enter Power mode
  * @param  Ctx Component object pointer
  * @param  Interface Could be FMC_SDRAM_CMD_TARGET_BANK1 or FMC_SDRAM_CMD_TARGET_BANK2
  * @retval error status
  */
int32_t IS42S32800J_EnterPowerMode(SDRAM_HandleTypeDef *Ctx, uint32_t Interface) 
{
  Command.CommandMode            = IS42S32800J_POWERDOWN_MODE_CMD;
  Command.CommandTarget          = Interface;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  if(HAL_SDRAM_SendCommand(Ctx, &Command, IS42S32800J_TIMEOUT) != HAL_OK)
  {
    return IS42S32800J_ERROR;
  }
  else
  {
    return IS42S32800J_OK;
  }
}

/**
  * @brief  Exit Power mode
  * @param  Ctx Component object pointer
  * @param  Interface Could be FMC_SDRAM_CMD_TARGET_BANK1 or FMC_SDRAM_CMD_TARGET_BANK2
  * @retval error status
  */
int32_t IS42S32800J_ExitPowerMode(SDRAM_HandleTypeDef *Ctx, uint32_t Interface) 
{
  Command.CommandMode            = IS42S32800J_NORMAL_MODE_CMD;
  Command.CommandTarget          = Interface;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  if(HAL_SDRAM_SendCommand(Ctx, &Command, IS42S32800J_TIMEOUT) != HAL_OK)
  {
    return IS42S32800J_ERROR;
  }
  else
  {
    return IS42S32800J_OK;
  }
}

/**
  * @brief  Sends command to the SDRAM bank.
  * @param  Ctx Component object pointer
  * @param  SdramCmd : Pointer to SDRAM command structure 
  * @retval SDRAM status
  */  
int32_t IS42S32800J_Sendcmd(SDRAM_HandleTypeDef *Ctx, FMC_SDRAM_CommandTypeDef *SdramCmd)
{
  if(HAL_SDRAM_SendCommand(Ctx, SdramCmd, IS42S32800J_TIMEOUT) != HAL_OK)
  {
    return IS42S32800J_ERROR;
  }
  else
  {
    return IS42S32800J_OK;
  }
}

/**
  * @}
  */ 

/** @defgroup IS42S32800J_Private_Functions IS42S32800J Private Functions
  * @{
  */ 

/**
  * @brief This function provides accurate delay (in milliseconds)
  * @param Delay : specifies the delay time length, in milliseconds
  * @retval IS42S32800J_OK
  */
static int32_t IS42S32800J_Delay(uint32_t Delay)
{  
  uint32_t tickstart;
  tickstart = HAL_GetTick();
  while((HAL_GetTick() - tickstart) < Delay)
  {
  }
  return IS42S32800J_OK;
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
