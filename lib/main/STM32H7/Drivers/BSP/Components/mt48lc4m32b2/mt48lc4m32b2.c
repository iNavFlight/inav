                                                 /**
 ******************************************************************************
 * @file    mt48lc4m32b2.c
 * @author  MCD Application Team
 * @brief   mt48lc4m32b2 sdram 128Mb driver file
 ******************************************************************************
 * @attention
 *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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
#include "mt48lc4m32b2.h"

/** @addtogroup BSP
  * @{
  */
  
/** @addtogroup Components
  * @{
  */ 

/** @defgroup MT48LC4M32B2 MT48LC4M32B2
  * @brief     This file provides a set of functions needed to drive the 
  *            MT48LC4M32B2 SDRAM memory.
  * @{
  */
 
/** @defgroup MT48LC4M32B2_Private_Variables MT48LC4M32B2 Private Variables
  * @{
  */
static FMC_SDRAM_CommandTypeDef Command;
/**
  * @}
  */ 

/** @defgroup MT48LC4M32B2_Function_Prototypes MT48LC4M32B2 Function Prototypes
  * @{
  */
static int32_t MT48LC4M32B2_Delay(uint32_t Delay);

/**
  * @}
  */ 

/** @defgroup MT48LC4M32B2_Exported_Functions MT48LC4M32B2 Exported Functions
  * @{
  */
/**
  * @brief  Initializes the MT48LC4M32B2 SDRAM memory
  * @param  Ctx : Component object pointer
  * @param  pRegMode : Pointer to Register Mode stucture
  * @retval error status
  */
int32_t MT48LC4M32B2_Init(SDRAM_HandleTypeDef *Ctx, MT48LC4M32B2_Context_t *pRegMode) 
{
  int32_t ret = MT48LC4M32B2_ERROR;
  
  /* Step 1: Configure a clock configuration enable command */
  if(MT48LC4M32B2_ClockEnable(Ctx, pRegMode->TargetBank) == MT48LC4M32B2_OK)
  {
    /* Step 2: Insert 100 us minimum delay */ 
    /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
    (void)MT48LC4M32B2_Delay(1);
    
    /* Step 3: Configure a PALL (precharge all) command */ 
    if(MT48LC4M32B2_Precharge(Ctx, pRegMode->TargetBank) == MT48LC4M32B2_OK)
    {
      /* Step 4: Configure a Refresh command */ 
      if(MT48LC4M32B2_RefreshMode(Ctx, pRegMode->TargetBank, pRegMode->RefreshMode) == MT48LC4M32B2_OK)
      {
        /* Step 5: Program the external memory mode register */
        if(MT48LC4M32B2_ModeRegConfig(Ctx, pRegMode) == MT48LC4M32B2_OK)
        {
          /* Step 6: Set the refresh rate counter */
          if(MT48LC4M32B2_RefreshRate(Ctx, pRegMode->RefreshRate) == MT48LC4M32B2_OK)
          {
            ret = MT48LC4M32B2_OK;
          }
        }
      }
    }
  } 
  return ret;
}

/**
  * @brief  Enable SDRAM clock
  * @param  Ctx : Component object pointer
  * @param  Interface : Could be FMC_SDRAM_CMD_TARGET_BANK1 or FMC_SDRAM_CMD_TARGET_BANK2
  * @retval error status
  */
int32_t MT48LC4M32B2_ClockEnable(SDRAM_HandleTypeDef *Ctx, uint32_t Interface) 
{
  Command.CommandMode            = MT48LC4M32B2_CLK_ENABLE_CMD;
  Command.CommandTarget          = Interface;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  if(HAL_SDRAM_SendCommand(Ctx, &Command, MT48LC4M32B2_TIMEOUT) != HAL_OK)
  {
    return MT48LC4M32B2_ERROR;
  }
  else
  {
    return MT48LC4M32B2_OK;
  }
}

/**
  * @brief  Precharge all sdram banks
  * @param  Ctx : Component object pointer
  * @param  Interface : Could be FMC_SDRAM_CMD_TARGET_BANK1 or FMC_SDRAM_CMD_TARGET_BANK2
  * @retval error status
  */
int32_t MT48LC4M32B2_Precharge(SDRAM_HandleTypeDef *Ctx, uint32_t Interface) 
{
  Command.CommandMode            = MT48LC4M32B2_PALL_CMD;
  Command.CommandTarget          = Interface;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  if(HAL_SDRAM_SendCommand(Ctx, &Command, MT48LC4M32B2_TIMEOUT) != HAL_OK)
  {
    return MT48LC4M32B2_ERROR;
  }
  else
  {
    return MT48LC4M32B2_OK;
  }
}

/**
  * @brief  Program the external memory mode register
  * @param  Ctx : Component object pointer
  * @param  pRegMode : Pointer to Register Mode stucture
  * @retval error status
  */
int32_t MT48LC4M32B2_ModeRegConfig(SDRAM_HandleTypeDef *Ctx, MT48LC4M32B2_Context_t *pRegMode) 
{
  uint32_t tmpmrd;

  /* Program the external memory mode register */
  tmpmrd = (uint32_t)pRegMode->BurstLength   |\
                     pRegMode->BurstType     |\
                     pRegMode->CASLatency    |\
                     pRegMode->OperationMode |\
                     pRegMode->WriteBurstMode;
  
  Command.CommandMode            = MT48LC4M32B2_LOAD_MODE_CMD;
  Command.CommandTarget          = pRegMode->TargetBank;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = tmpmrd;
  
  /* Send the command */
  if(HAL_SDRAM_SendCommand(Ctx, &Command, MT48LC4M32B2_TIMEOUT) != HAL_OK)
  {
    return MT48LC4M32B2_ERROR;
  }
  else
  {
    return MT48LC4M32B2_OK;
  }
}

/**
  * @brief  Program the SDRAM timing
  * @param  Ctx : Component object pointer
  * @param  pTiming : Pointer to SDRAM timing configuration stucture
  * @retval error status
  */
int32_t MT48LC4M32B2_TimingConfig(SDRAM_HandleTypeDef *Ctx, FMC_SDRAM_TimingTypeDef *pTiming) 
{
  /* Program the SDRAM timing */
  if(HAL_SDRAM_Init(Ctx, pTiming) != HAL_OK)
  {
    return MT48LC4M32B2_ERROR;
  }
  else
  {
    return MT48LC4M32B2_OK;
  }
}

/**
  * @brief  Configure Refresh mode
  * @param  Ctx : Component object pointer
  * @param  Interface : Could be FMC_SDRAM_CMD_TARGET_BANK1 or FMC_SDRAM_CMD_TARGET_BANK2 
  * @param  RefreshMode : Could be MT48LC4M32B2_CMD_AUTOREFRESH_MODE or
  *                      MT48LC4M32B2_CMD_SELFREFRESH_MODE
  * @retval error status
  */
int32_t MT48LC4M32B2_RefreshMode(SDRAM_HandleTypeDef *Ctx, uint32_t Interface, uint32_t RefreshMode) 
{
  Command.CommandMode            = RefreshMode;
  Command.CommandTarget          = Interface;
  Command.AutoRefreshNumber      = 8;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  if(HAL_SDRAM_SendCommand(Ctx, &Command, MT48LC4M32B2_TIMEOUT) != HAL_OK)
  {
    return MT48LC4M32B2_ERROR;
  }
  else
  {
    return MT48LC4M32B2_OK;
  }
}

/**
  * @brief  Set the device refresh rate
  * @param  Ctx : Component object pointer
  * @param  RefreshCount : The refresh rate to be programmed
  * @retval error status
  */
int32_t MT48LC4M32B2_RefreshRate(SDRAM_HandleTypeDef *Ctx, uint32_t RefreshCount) 
{
  /* Set the device refresh rate */
  if(HAL_SDRAM_ProgramRefreshRate(Ctx, RefreshCount) != HAL_OK)
  {
    return MT48LC4M32B2_ERROR;
  }
  else
  {
    return MT48LC4M32B2_OK;
  }
}

/**
  * @brief  Enter Power mode
  * @param  Ctx : Component object pointer
  * @param  Interface : Could be FMC_SDRAM_CMD_TARGET_BANK1 or FMC_SDRAM_CMD_TARGET_BANK2
  * @retval error status
  */
int32_t MT48LC4M32B2_EnterPowerMode(SDRAM_HandleTypeDef *Ctx, uint32_t Interface) 
{
  Command.CommandMode            = MT48LC4M32B2_POWERDOWN_MODE_CMD;
  Command.CommandTarget          = Interface;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  if(HAL_SDRAM_SendCommand(Ctx, &Command, MT48LC4M32B2_TIMEOUT) != HAL_OK)
  {
    return MT48LC4M32B2_ERROR;
  }
  else
  {
    return MT48LC4M32B2_OK;
  }
}

/**
  * @brief  Exit Power mode
  * @param  Ctx : Component object pointer
  * @param  Interface : Could be FMC_SDRAM_CMD_TARGET_BANK1 or FMC_SDRAM_CMD_TARGET_BANK2
  * @retval error status
  */
int32_t MT48LC4M32B2_ExitPowerMode(SDRAM_HandleTypeDef *Ctx, uint32_t Interface) 
{
  Command.CommandMode            = MT48LC4M32B2_NORMAL_MODE_CMD;
  Command.CommandTarget          = Interface;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  if(HAL_SDRAM_SendCommand(Ctx, &Command, MT48LC4M32B2_TIMEOUT) != HAL_OK)
  {
    return MT48LC4M32B2_ERROR;
  }
  else
  {
    return MT48LC4M32B2_OK;
  }
}

/**
  * @brief  Sends command to the SDRAM bank.
  * @param  Ctx : Component object pointer
  * @param  SdramCmd : Pointer to SDRAM command structure 
  * @retval SDRAM status
  */  
int32_t MT48LC4M32B2_Sendcmd(SDRAM_HandleTypeDef *Ctx, FMC_SDRAM_CommandTypeDef *SdramCmd)
{
  if(HAL_SDRAM_SendCommand(Ctx, SdramCmd, MT48LC4M32B2_TIMEOUT) != HAL_OK)
  {
    return MT48LC4M32B2_ERROR;
  }
  else
  {
    return MT48LC4M32B2_OK;
  }
}

/**
  * @}
  */ 

/** @defgroup MT48LC4M32B2_Private_Functions MT48LC4M32B2 Private Functions
  * @{
  */ 

/**
  * @brief This function provides accurate delay (in milliseconds)
  * @param Delay: specifies the delay time length, in milliseconds
  * @retval MT48LC4M32B2_OK
  */
static int32_t MT48LC4M32B2_Delay(uint32_t Delay)
{  
  uint32_t tickstart;
  tickstart = HAL_GetTick();
  while((HAL_GetTick() - tickstart) < Delay)
  {
  }
  return MT48LC4M32B2_OK;
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
