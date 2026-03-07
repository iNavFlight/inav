/**
 ******************************************************************************
 * @file    s70kl1281.c
 * @modify  MCD Application Team
 * @brief   This file provides the S70KL1281 OSPI drivers.
 ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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
#include "s70kl1281.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @defgroup S70KL1281 S70KL1281
  * @{
  */

/** @defgroup S70KL1281_Exported_Functions S70KL1281 Exported Functions
  * @{
  */

/* Read/Write Array Commands *********************/
/**
  * @brief  Reads an amount of data from the OSPI memory.
  * @param  Ctx Component object pointer
  * @param  pData Pointer to data to be read
  * @param  ReadAddr Read start address
  * @param  Size Size of data to read
  * @retval OSPI memory status
  */
int32_t S70KL1281_Read(OSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
  OSPI_HyperbusCmdTypeDef  sCommand;

  /* Initialize the read command */
  sCommand.AddressSpace = HAL_OSPI_MEMORY_ADDRESS_SPACE;
  sCommand.AddressSize  = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.Address      = ReadAddr;
  sCommand.DQSMode      = HAL_OSPI_DQS_ENABLE;
  sCommand.NbData       = Size;

  /* Configure the command */
  if (HAL_OSPI_HyperbusCmd(Ctx, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive(Ctx, pData, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  return S70KL1281_OK;
}

/**
  * @brief  Reads an amount of data in DMA mode from the OSPI memory.
  * @param  Ctx Component object pointer
  * @param  pData Pointer to data to be read
  * @param  ReadAddr Read start address
  * @param  Size Size of data to read
  * @retval OSPI memory status
  */
int32_t S70KL1281_Read_DMA(OSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
  OSPI_HyperbusCmdTypeDef  sCommand;

  /* Initialize the read command */
  sCommand.AddressSpace = HAL_OSPI_MEMORY_ADDRESS_SPACE;
  sCommand.AddressSize  = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.Address      = ReadAddr;
  sCommand.DQSMode      = HAL_OSPI_DQS_ENABLE;
  sCommand.NbData       = Size;

  /* Configure the command */
  if (HAL_OSPI_HyperbusCmd(Ctx, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive_DMA(Ctx, pData) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  return S70KL1281_OK;
}

/**
  * @brief  Writes an amount of data to the OSPI memory.
  * @param  Ctx Component object pointer
  * @param  pData Pointer to data to be written
  * @param  WriteAddr Write start address
  * @param  Size Size of data to write
  * @retval OSPI memory status
  */
int32_t S70KL1281_Write(OSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t WriteAddr, uint32_t Size)
{
  OSPI_HyperbusCmdTypeDef  sCommand;

  /* Initialize the write command */
  sCommand.AddressSpace = HAL_OSPI_MEMORY_ADDRESS_SPACE;
  sCommand.AddressSize  = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.Address      = WriteAddr;
  sCommand.DQSMode      = HAL_OSPI_DQS_ENABLE;
  sCommand.NbData       = Size;

  /* Configure the command */
  if (HAL_OSPI_HyperbusCmd(Ctx, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  /* Transmission of the data */
  if (HAL_OSPI_Transmit(Ctx, pData, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  return S70KL1281_OK;
}

/**
  * @brief  Writes an amount of data in DMA mode to the OSPI memory.
  * @param  Ctx Component object pointer
  * @param  pData Pointer to data to be written
  * @param  WriteAddr Write start address
  * @param  Size Size of data to write
  * @retval OSPI memory status
  */
int32_t S70KL1281_Write_DMA(OSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t WriteAddr, uint32_t Size)
{
  OSPI_HyperbusCmdTypeDef  sCommand;

  /* Initialize the write command */
  sCommand.AddressSpace = HAL_OSPI_MEMORY_ADDRESS_SPACE;
  sCommand.AddressSize  = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.Address      = WriteAddr;
  sCommand.DQSMode      = HAL_OSPI_DQS_ENABLE;
  sCommand.NbData       = Size;

  /* Configure the command */
  if (HAL_OSPI_HyperbusCmd(Ctx, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  /* Transmission of the data */
  if (HAL_OSPI_Transmit_DMA(Ctx, pData) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  return S70KL1281_OK;
}

/**
  * @brief  Enable memory mapped mode for the OSPI memory.
  * @param  Ctx Component object pointer
  * @retval OSPI memory status
  */
int32_t S70KL1281_EnableMemoryMappedMode(OSPI_HandleTypeDef *Ctx)
{
  OSPI_HyperbusCmdTypeDef  sCommand;
  OSPI_MemoryMappedTypeDef sMemMappedCfg;

  /* OctoSPI Hyperbus command configuration */
  sCommand.AddressSpace = HAL_OSPI_MEMORY_ADDRESS_SPACE;
  sCommand.AddressSize  = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.Address      = 0;
  sCommand.DQSMode      = HAL_OSPI_DQS_ENABLE;
  sCommand.NbData       = 1;

  if (HAL_OSPI_HyperbusCmd(Ctx, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  /* OctoSPI activation of memory-mapped mode */
  sMemMappedCfg.TimeOutActivation = HAL_OSPI_TIMEOUT_COUNTER_DISABLE;

  if (HAL_OSPI_MemoryMapped(Ctx, &sMemMappedCfg) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  return S70KL1281_OK;
}

/* Register/Setting Commands **************************************************/
/**
  * @brief  Read configuration register 0 value
  * @param  Ctx Component object pointer
  * @param  Value Register value pointer
  * @retval error status
  */
int32_t S70KL1281_ReadCfgReg0(OSPI_HandleTypeDef *Ctx, uint16_t *Value)
{
  OSPI_HyperbusCmdTypeDef  sCommand;

  /* Initialize the read command */
  sCommand.AddressSpace = HAL_OSPI_REGISTER_ADDRESS_SPACE;
  sCommand.AddressSize  = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.Address      = S70KL1281_CR0_ADDRESS;
  sCommand.DQSMode      = HAL_OSPI_DQS_ENABLE;
  sCommand.NbData       = 2U;

  /* Configure the command */
  if (HAL_OSPI_HyperbusCmd(Ctx, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive(Ctx, (uint8_t *)Value, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  return S70KL1281_OK;
}

/**
  * @brief  Write configuration register 0
  * @param  Ctx Component object pointer
  * @param  Value Value to write to register
  * @retval error status
  */
int32_t S70KL1281_WriteCfgReg0(OSPI_HandleTypeDef *Ctx, uint16_t Value, uint32_t LatencyMode, uint32_t InitialLatency)
{
  OSPI_HyperbusCfgTypeDef sHyperbusCfg;
  OSPI_HyperbusCmdTypeDef sCommand;

  /* Reconfigure peripheral as no write latency to write in registers */
  sHyperbusCfg.RWRecoveryTime   = RW_RECOVERY_TIME;
  sHyperbusCfg.AccessTime       = InitialLatency;
  sHyperbusCfg.WriteZeroLatency = HAL_OSPI_NO_LATENCY_ON_WRITE;
  sHyperbusCfg.LatencyMode      = LatencyMode;

  if (HAL_OSPI_HyperbusCfg(Ctx, &sHyperbusCfg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  /* Initialize the write command */
  sCommand.AddressSpace = HAL_OSPI_REGISTER_ADDRESS_SPACE;
  sCommand.AddressSize  = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.Address      = S70KL1281_CR0_ADDRESS;
  sCommand.DQSMode      = HAL_OSPI_DQS_ENABLE;
  sCommand.NbData       = 2U;

  /* Configure the command */
  if (HAL_OSPI_HyperbusCmd(Ctx, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  /* Transmission of the data */
  if (HAL_OSPI_Transmit(Ctx, (uint8_t *)(&Value), HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  /* Reconfigure peripheral for correct write access */
  sHyperbusCfg.WriteZeroLatency = HAL_OSPI_LATENCY_ON_WRITE;

  if (HAL_OSPI_HyperbusCfg(Ctx, &sHyperbusCfg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  return S70KL1281_OK;
}

/**
  * @brief  Read configuration register 1 value
  * @param  Ctx Component object pointer
  * @param  Value Register value pointer
  * @retval error status
  */
int32_t S70KL1281_ReadCfgReg1(OSPI_HandleTypeDef *Ctx, uint16_t *Value)
{
  OSPI_HyperbusCmdTypeDef  sCommand;

  /* Initialize the read command */
  sCommand.AddressSpace = HAL_OSPI_REGISTER_ADDRESS_SPACE;
  sCommand.AddressSize  = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.Address      = S70KL1281_CR1_ADDRESS;
  sCommand.DQSMode      = HAL_OSPI_DQS_ENABLE;
  sCommand.NbData       = 2U;

  /* Configure the command */
  if (HAL_OSPI_HyperbusCmd(Ctx, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive(Ctx, (uint8_t *)Value, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  return S70KL1281_OK;
}

/**
  * @brief  Write configuration register 1
  * @param  Ctx Component object pointer
  * @param  Value Value to write to register
  * @retval error status
  */
int32_t S70KL1281_WriteCfgReg1(OSPI_HandleTypeDef *Ctx, uint16_t Value, uint32_t LatencyMode, uint32_t InitialLatency)
{
  OSPI_HyperbusCfgTypeDef sHyperbusCfg;
  OSPI_HyperbusCmdTypeDef sCommand;

  /* Reconfigure peripheral as no write latency to write in registers */
  sHyperbusCfg.RWRecoveryTime   = RW_RECOVERY_TIME;
  sHyperbusCfg.AccessTime       = InitialLatency;
  sHyperbusCfg.WriteZeroLatency = HAL_OSPI_NO_LATENCY_ON_WRITE;
  sHyperbusCfg.LatencyMode      = LatencyMode;

  if (HAL_OSPI_HyperbusCfg(Ctx, &sHyperbusCfg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  /* Initialize the write command */
  sCommand.AddressSpace = HAL_OSPI_REGISTER_ADDRESS_SPACE;
  sCommand.AddressSize  = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.Address      = S70KL1281_CR1_ADDRESS;
  sCommand.DQSMode      = HAL_OSPI_DQS_ENABLE;
  sCommand.NbData       = 2U;

  /* Configure the command */
  if (HAL_OSPI_HyperbusCmd(Ctx, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  /* Transmission of the data */
  if (HAL_OSPI_Transmit(Ctx, (uint8_t *)(&Value), HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  /* Reconfigure peripheral for correct write access */
  sHyperbusCfg.WriteZeroLatency = HAL_OSPI_LATENCY_ON_WRITE;

  if (HAL_OSPI_HyperbusCfg(Ctx, &sHyperbusCfg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  return S70KL1281_OK;
}

/* ID Commands ****************************************************************/
/**
  * @brief  Read Flash IDs.
  *         Manufacturer ID, Memory type, Memory density
  * @param  Ctx Component object pointer
  * @param  ID IDs pointer (2 * 16-bits value)
  * @retval error status
  */
int32_t S70KL1281_ReadID(OSPI_HandleTypeDef *Ctx, uint16_t *ID)
{
  OSPI_HyperbusCmdTypeDef  sCommand;
  uint16_t *data = ID;

  /* Initialize the write command */
  sCommand.AddressSpace = HAL_OSPI_REGISTER_ADDRESS_SPACE;
  sCommand.AddressSize  = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.Address      = S70KL1281_DIR0_ADDRESS;
  sCommand.DQSMode      = HAL_OSPI_DQS_ENABLE;
  sCommand.NbData       = 2U;

  /* Configure the command */
  if (HAL_OSPI_HyperbusCmd(Ctx, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive(Ctx, (uint8_t *)data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  data++;
  sCommand.Address = S70KL1281_DIR1_ADDRESS;

   /* Configure the command */
  if (HAL_OSPI_HyperbusCmd(Ctx, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive(Ctx, (uint8_t *)data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  return S70KL1281_OK;
}

/* Power down Commands ********************************************************/
/**
  * @brief  Flash enter deep power-down command
  * @param  Ctx Component object pointer
  * @retval error status
  */
int32_t S70KL1281_EnterPowerDown(OSPI_HandleTypeDef *Ctx, uint32_t LatencyMode, uint32_t InitialLatency)
{
  uint16_t reg;

  /* Read the value of the CR0 register */
  if (S70KL1281_ReadCfgReg0(Ctx, &reg) != S70KL1281_OK)
  {
    return S70KL1281_ERROR;
  }

  /* Update the deep power down value of the CR0 register */
  SET_BIT(reg, S70KL1281_CR0_DPDE);

  if (S70KL1281_WriteCfgReg0(Ctx, reg, LatencyMode, InitialLatency) != S70KL1281_OK)
  {
    return S70KL1281_ERROR;
  }

  /* ---          Memory takes 10us min to enter deep power down          --- */
  /* --- At least 30us should be respected before leaving deep power down --- */

  return S70KL1281_OK;
}

/**
  * @brief  Flash leave deep power-down command
  * @param  Ctx Component object pointer
  * @retval error status
  */
int32_t S70KL1281_LeavePowerDown(OSPI_HandleTypeDef *Ctx)
{
  /* --- A dummy command is sent to the memory, as the nCS should be low for at least 200 ns --- */
  /* ---                  Memory takes 150us max to leave deep power down                    --- */

   OSPI_HyperbusCmdTypeDef  sCommand;
   uint32_t data;

  /* Initialize the read command */
  sCommand.AddressSpace = HAL_OSPI_MEMORY_ADDRESS_SPACE;
  sCommand.AddressSize  = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.Address      = 0;
  sCommand.DQSMode      = HAL_OSPI_DQS_ENABLE;
  sCommand.NbData       = 4;

  /* Configure the command */
  if (HAL_OSPI_HyperbusCmd(Ctx, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive(Ctx, (uint8_t*)(&data), HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S70KL1281_ERROR;
  }

  return S70KL1281_OK;
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
