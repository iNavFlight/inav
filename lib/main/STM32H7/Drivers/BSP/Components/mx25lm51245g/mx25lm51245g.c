/**
 ******************************************************************************
 * @file    mx25lm51245g.c
 * @modify  MCD Application Team
 * @brief   This file provides the MX25LM51245G OSPI drivers.
 ******************************************************************************
 * MX25LM51245G action :
 *   STR Octal IO protocol (SOPI) and DTR Octal IO protocol (DOPI) bits of
 *   Configuration Register 2 :
 *     DOPI = 1 and SOPI = 0: Operates in DTR Octal IO protocol (accepts 8-8-8 commands)
 *     DOPI = 0 and SOPI = 1: Operates in STR Octal IO protocol (accepts 8-8-8 commands)
 *     DOPI = 0 and SOPI = 0: Operates in Single IO protocol (accepts 1-1-1 commands)
 *   Enter SOPI mode by configuring DOPI = 0 and SOPI = 1 in CR2-Addr0
 *   Exit SOPI mode by configuring DOPI = 0 and SOPI = 0 in CR2-Addr0
 *   Enter DOPI mode by configuring DOPI = 1 and SOPI = 0 in CR2-Addr0
 *   Exit DOPI mode by configuring DOPI = 0 and SOPI = 0 in CR2-Addr0
 *
 *   Memory commands support STR(Single Transfer Rate) &
 *   DTR(Double Transfer Rate) modes in OPI
 *
 *   Memory commands support STR(Single Transfer Rate) &
 *   DTR(Double Transfer Rate) modes in SPI
 *
 ******************************************************************************
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
#include "mx25lm51245g.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @defgroup MX25LM51245G MX25LM51245G
  * @{
  */

/** @defgroup MX25LM51245G_Exported_Functions MX25LM51245G Exported Functions
  * @{
  */

/**
  * @brief  Get Flash information
  * @param  pInfo pointer to information structure
  * @retval error status
  */
int32_t MX25LM51245G_GetFlashInfo(MX25LM51245G_Info_t *pInfo)
{
  /* Configure the structure with the memory configuration */
  pInfo->FlashSize              = MX25LM51245G_FLASH_SIZE;
  pInfo->EraseSectorSize        = MX25LM51245G_SECTOR_64K;
  pInfo->EraseSectorsNumber     = (MX25LM51245G_FLASH_SIZE/MX25LM51245G_SECTOR_64K);
  pInfo->EraseSubSectorSize     = MX25LM51245G_SUBSECTOR_4K;
  pInfo->EraseSubSectorNumber   = (MX25LM51245G_FLASH_SIZE/MX25LM51245G_SUBSECTOR_4K);
  pInfo->EraseSubSector1Size    = MX25LM51245G_SUBSECTOR_4K;
  pInfo->EraseSubSector1Number  = (MX25LM51245G_FLASH_SIZE/MX25LM51245G_SUBSECTOR_4K);
  pInfo->ProgPageSize           = MX25LM51245G_PAGE_SIZE;
  pInfo->ProgPagesNumber        = (MX25LM51245G_FLASH_SIZE/MX25LM51245G_PAGE_SIZE);

  return MX25LM51245G_OK;
};

/**
  * @brief  Polling WIP(Write In Progress) bit become to 0
  *         SPI/OPI;
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  Rate Transfer rate
  * @retval error status
  */
int32_t MX25LM51245G_AutoPollingMemReady(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode, MX25LM51245G_Transfer_t Rate)
{
  OSPI_RegularCmdTypeDef  s_command = {0};
  OSPI_AutoPollingTypeDef s_config = {0};

  /* SPI mode and DTR transfer not supported by memory */
  if ((Mode == MX25LM51245G_SPI_MODE) && (Rate == MX25LM51245G_DTR_TRANSFER))
  {
    return MX25LM51245G_ERROR;
  }

  /* Configure automatic polling mode to wait for memory ready */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_1_LINE : HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_INSTRUCTION_DTR_ENABLE : HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_8_BITS : HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = (Mode == MX25LM51245G_SPI_MODE) ? MX25LM51245G_READ_STATUS_REG_CMD : MX25LM51245G_OCTA_READ_STATUS_REG_CMD;
  s_command.AddressMode        = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_ADDRESS_NONE : HAL_OSPI_ADDRESS_8_LINES;
  s_command.AddressDtrMode     = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_ADDRESS_DTR_ENABLE : HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  s_command.Address            = 0U;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_DATA_1_LINE : HAL_OSPI_DATA_8_LINES;
  s_command.DataDtrMode        = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_DATA_DTR_ENABLE : HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = (Mode == MX25LM51245G_SPI_MODE) ? 0U : ((Rate == MX25LM51245G_DTR_TRANSFER) ? DUMMY_CYCLES_REG_OCTAL_DTR : DUMMY_CYCLES_REG_OCTAL);
  s_command.NbData             = (Rate == MX25LM51245G_DTR_TRANSFER) ? 2U : 1U;
  s_command.DQSMode            = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_DQS_ENABLE : HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  s_config.Match         = 0U;
  s_config.Mask          = MX25LM51245G_SR_WIP;
  s_config.MatchMode     = HAL_OSPI_MATCH_MODE_AND;
  s_config.Interval      = MX25LM51245G_AUTOPOLLING_INTERVAL_TIME;
  s_config.AutomaticStop = HAL_OSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  if (HAL_OSPI_AutoPolling(Ctx, &s_config, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/* Read/Write Array Commands (3/4 Byte Address Command Set) *********************/
/**
  * @brief  Reads an amount of data from the OSPI memory on STR mode.
  *         SPI/OPI; 1-1-1/8-8-8
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  AddressSize Address size
  * @param  pData Pointer to data to be read
  * @param  ReadAddr Read start address
  * @param  Size Size of data to read
  * @retval OSPI memory status
  */
int32_t MX25LM51245G_ReadSTR(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode, MX25LM51245G_AddressSize_t AddressSize, uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* OPI mode and 3-bytes address size not supported by memory */
  if ((Mode == MX25LM51245G_OPI_MODE) && (AddressSize == MX25LM51245G_3BYTES_SIZE))
  {
    return MX25LM51245G_ERROR;
  }

  /* Initialize the read command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_1_LINE : HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_8_BITS : HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = (Mode == MX25LM51245G_SPI_MODE) ? ((AddressSize == MX25LM51245G_3BYTES_SIZE) ? MX25LM51245G_FAST_READ_CMD : MX25LM51245G_4_BYTE_ADDR_FAST_READ_CMD) : MX25LM51245G_OCTA_READ_CMD;
  s_command.AddressMode        = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_ADDRESS_1_LINE : HAL_OSPI_ADDRESS_8_LINES;
  s_command.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = (AddressSize == MX25LM51245G_3BYTES_SIZE) ? HAL_OSPI_ADDRESS_24_BITS : HAL_OSPI_ADDRESS_32_BITS;
  s_command.Address            = ReadAddr;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_DATA_1_LINE : HAL_OSPI_DATA_8_LINES;
  s_command.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = (Mode == MX25LM51245G_SPI_MODE) ? DUMMY_CYCLES_READ : DUMMY_CYCLES_READ_OCTAL;
  s_command.NbData             = Size;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive(Ctx, pData, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/**
  * @brief  Reads an amount of data from the OSPI memory on DTR mode.
  *         OPI
  * @param  Ctx Component object pointer
  * @param  AddressSize Address size
  * @param  pData Pointer to data to be read
  * @param  ReadAddr Read start addressS
  * @param  Size Size of data to read
  * @note   Only OPI mode support DTR transfer rate
  * @retval OSPI memory status
  */
int32_t MX25LM51245G_ReadDTR(OSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* Initialize the read command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_ENABLE;
  s_command.InstructionSize    = HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = MX25LM51245G_OCTA_READ_DTR_CMD;
  s_command.AddressMode        = HAL_OSPI_ADDRESS_8_LINES;
  s_command.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_ENABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  s_command.Address            = ReadAddr;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = HAL_OSPI_DATA_8_LINES;
  s_command.DataDtrMode        = HAL_OSPI_DATA_DTR_ENABLE;
  s_command.DummyCycles        = DUMMY_CYCLES_READ_OCTAL_DTR;
  s_command.NbData             = Size;
  s_command.DQSMode            = HAL_OSPI_DQS_ENABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive(Ctx, pData, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/**
  * @brief  Writes an amount of data to the OSPI memory.
  *         SPI/OPI
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  AddressSize Address size
  * @param  pData Pointer to data to be written
  * @param  WriteAddr Write start address
  * @param  Size Size of data to write. Range 1 ~ MX25LM51245G_PAGE_SIZE
  * @note   Address size is forced to 3 Bytes when the 4 Bytes address size
  *         command is not available for the specified interface mode
  * @retval OSPI memory status
  */
int32_t MX25LM51245G_PageProgram(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode, MX25LM51245G_AddressSize_t AddressSize, uint8_t *pData, uint32_t WriteAddr, uint32_t Size)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* OPI mode and 3-bytes address size not supported by memory */
  if ((Mode == MX25LM51245G_OPI_MODE) && (AddressSize == MX25LM51245G_3BYTES_SIZE))
  {
    return MX25LM51245G_ERROR;
  }

  /* Initialize the program command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_1_LINE : HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_8_BITS : HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = (Mode == MX25LM51245G_SPI_MODE) ? ((AddressSize == MX25LM51245G_3BYTES_SIZE) ? MX25LM51245G_PAGE_PROG_CMD : MX25LM51245G_4_BYTE_PAGE_PROG_CMD) : MX25LM51245G_OCTA_PAGE_PROG_CMD;
  s_command.AddressMode        = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_ADDRESS_1_LINE : HAL_OSPI_ADDRESS_8_LINES;
  s_command.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = (AddressSize == MX25LM51245G_3BYTES_SIZE) ? HAL_OSPI_ADDRESS_24_BITS : HAL_OSPI_ADDRESS_32_BITS;
  s_command.Address            = WriteAddr;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_DATA_1_LINE : HAL_OSPI_DATA_8_LINES;
  s_command.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = 0U;
  s_command.NbData             = Size;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  /* Transmission of the data */
  if (HAL_OSPI_Transmit(Ctx, pData, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/**
  * @brief  Writes an amount of data to the OSPI memory on DTR mode.
  *         SPI/OPI
  * @param  Ctx Component object pointer
  * @param  pData Pointer to data to be written
  * @param  WriteAddr Write start address
  * @param  Size Size of data to write. Range 1 ~ MX25LM51245G_PAGE_SIZE
  * @note   Only OPI mode support DTR transfer rate
  * @retval OSPI memory status
  */
int32_t MX25LM51245G_PageProgramDTR(OSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t WriteAddr, uint32_t Size)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* Initialize the program command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_ENABLE;
  s_command.InstructionSize    = HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = MX25LM51245G_OCTA_PAGE_PROG_CMD;
  s_command.AddressMode        = HAL_OSPI_ADDRESS_8_LINES;
  s_command.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_ENABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  s_command.Address            = WriteAddr;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = HAL_OSPI_DATA_8_LINES;
  s_command.DataDtrMode        = HAL_OSPI_DATA_DTR_ENABLE;
  s_command.DummyCycles        = 0U;
  s_command.NbData             = Size;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  /* Transmission of the data */
  if (HAL_OSPI_Transmit(Ctx, pData, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/**
  * @brief  Erases the specified block of the OSPI memory.
  *         MX25LM51245G support 4K, 64K size block erase commands.
  *         SPI/OPI; 1-1-1/8-8-8
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  AddressSize Address size
  * @param  BlockAddress Block address to erase
  * @param  BlockSize Block size to erase
  * @retval OSPI memory status
  */
int32_t MX25LM51245G_BlockErase(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode, MX25LM51245G_Transfer_t Rate, MX25LM51245G_AddressSize_t AddressSize, uint32_t BlockAddress, MX25LM51245G_Erase_t BlockSize)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* SPI mode and DTR transfer not supported by memory */
  if ((Mode == MX25LM51245G_SPI_MODE) && (Rate == MX25LM51245G_DTR_TRANSFER))
  {
    return MX25LM51245G_ERROR;
  }

  /* OPI mode and 3-bytes address size not supported by memory */
  if ((Mode == MX25LM51245G_OPI_MODE) && (AddressSize == MX25LM51245G_3BYTES_SIZE))
  {
    return MX25LM51245G_ERROR;
  }

  /* Initialize the erase command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_1_LINE : HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_INSTRUCTION_DTR_ENABLE : HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_8_BITS : HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.AddressMode        = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_ADDRESS_1_LINE : HAL_OSPI_ADDRESS_8_LINES;
  s_command.AddressDtrMode     = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_ADDRESS_DTR_ENABLE : HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = (AddressSize == MX25LM51245G_3BYTES_SIZE) ? HAL_OSPI_ADDRESS_24_BITS : HAL_OSPI_ADDRESS_32_BITS;
  s_command.Address            = BlockAddress;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = HAL_OSPI_DATA_NONE;
  s_command.DummyCycles        = 0U;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  switch(Mode)
  {
  case MX25LM51245G_OPI_MODE :
    if(BlockSize == MX25LM51245G_ERASE_64K)
    {
      s_command.Instruction = MX25LM51245G_OCTA_SECTOR_ERASE_64K_CMD;
    }
    else
    {
      s_command.Instruction = MX25LM51245G_OCTA_SUBSECTOR_ERASE_4K_CMD;
    }
    break;

  case MX25LM51245G_SPI_MODE :
  default:
    if(BlockSize == MX25LM51245G_ERASE_64K)
    {
      s_command.Instruction = (AddressSize == MX25LM51245G_3BYTES_SIZE) ? MX25LM51245G_SECTOR_ERASE_64K_CMD : MX25LM51245G_4_BYTE_SECTOR_ERASE_64K_CMD;
    }
    else
    {
      s_command.Instruction = (AddressSize == MX25LM51245G_3BYTES_SIZE) ? MX25LM51245G_SUBSECTOR_ERASE_4K_CMD : MX25LM51245G_4_BYTE_SUBSECTOR_ERASE_4K_CMD;
    }
    break;
  }

  /* Send the command */
  if(HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/**
  * @brief  Whole chip erase.
  *         SPI/OPI; 1-0-0/8-0-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval error status
  */
int32_t MX25LM51245G_ChipErase(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode, MX25LM51245G_Transfer_t Rate)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* SPI mode and DTR transfer not supported by memory */
  if ((Mode == MX25LM51245G_SPI_MODE) && (Rate == MX25LM51245G_DTR_TRANSFER))
  {
    return MX25LM51245G_ERROR;
  }

  /* Initialize the erase command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_1_LINE : HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_INSTRUCTION_DTR_ENABLE : HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_8_BITS : HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = (Mode == MX25LM51245G_SPI_MODE) ? MX25LM51245G_BULK_ERASE_CMD : MX25LM51245G_OCTA_BULK_ERASE_CMD;
  s_command.AddressMode        = HAL_OSPI_ADDRESS_NONE;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = HAL_OSPI_DATA_NONE;
  s_command.DummyCycles        = 0U;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if(HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/**
  * @brief  Enable memory mapped mode for the OSPI memory on STR mode.
  *         SPI/OPI; 1-1-1/8-8-8
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  AddressSize Address size
  * @retval OSPI memory status
  */
int32_t MX25LM51245G_EnableSTRMemoryMappedMode(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode, MX25LM51245G_AddressSize_t AddressSize)
{
  OSPI_RegularCmdTypeDef      s_command = {0};
  OSPI_MemoryMappedTypeDef s_mem_mapped_cfg = {0};

  /* OPI mode and 3-bytes address size not supported by memory */
  if ((Mode == MX25LM51245G_OPI_MODE) && (AddressSize == MX25LM51245G_3BYTES_SIZE))
  {
    return MX25LM51245G_ERROR;
  }

  /* Initialize the read command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_READ_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_1_LINE : HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_8_BITS : HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = (Mode == MX25LM51245G_SPI_MODE) ? ((AddressSize == MX25LM51245G_3BYTES_SIZE) ? MX25LM51245G_FAST_READ_CMD : MX25LM51245G_4_BYTE_ADDR_FAST_READ_CMD) : MX25LM51245G_OCTA_READ_CMD;
  s_command.AddressMode        = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_ADDRESS_1_LINE : HAL_OSPI_ADDRESS_8_LINES;
  s_command.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = (AddressSize == MX25LM51245G_3BYTES_SIZE) ? HAL_OSPI_ADDRESS_24_BITS : HAL_OSPI_ADDRESS_32_BITS;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_DATA_1_LINE : HAL_OSPI_DATA_8_LINES;
  s_command.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = (Mode == MX25LM51245G_SPI_MODE) ? DUMMY_CYCLES_READ : DUMMY_CYCLES_READ_OCTAL;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the read command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  /* Initialize the program command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_WRITE_CFG;
  s_command.Instruction        = (Mode == MX25LM51245G_SPI_MODE) ? ((AddressSize == MX25LM51245G_3BYTES_SIZE) ? MX25LM51245G_PAGE_PROG_CMD : MX25LM51245G_4_BYTE_PAGE_PROG_CMD) : MX25LM51245G_OCTA_PAGE_PROG_CMD;
  s_command.DummyCycles        = 0U;

  /* Send the write command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  /* Configure the memory mapped mode */
  s_mem_mapped_cfg.TimeOutActivation = HAL_OSPI_TIMEOUT_COUNTER_DISABLE;

  if (HAL_OSPI_MemoryMapped(Ctx, &s_mem_mapped_cfg) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/**
  * @brief  Enable memory mapped mode for the OSPI memory on DTR mode.
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  AddressSize Address size
  * @note   Only OPI mode support DTR transfer rate
  * @retval OSPI memory status
  */
int32_t MX25LM51245G_EnableDTRMemoryMappedMode(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Mode);

  OSPI_RegularCmdTypeDef      s_command = {0};
  OSPI_MemoryMappedTypeDef s_mem_mapped_cfg = {0};

  /* Initialize the read command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_READ_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_ENABLE;
  s_command.InstructionSize    = HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = MX25LM51245G_OCTA_READ_DTR_CMD;
  s_command.AddressMode        = HAL_OSPI_ADDRESS_8_LINES;
  s_command.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_ENABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = HAL_OSPI_DATA_8_LINES;
  s_command.DataDtrMode        = HAL_OSPI_DATA_DTR_ENABLE;
  s_command.DummyCycles        = DUMMY_CYCLES_READ_OCTAL_DTR;
  s_command.DQSMode            = HAL_OSPI_DQS_ENABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  /* Initialize the program command */
  s_command.OperationType = HAL_OSPI_OPTYPE_WRITE_CFG;
  s_command.Instruction   = MX25LM51245G_OCTA_PAGE_PROG_CMD;
  s_command.DummyCycles   = 0U;
  s_command.DQSMode       = HAL_OSPI_DQS_DISABLE;

  /* Send the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }
  /* Configure the memory mapped mode */
  s_mem_mapped_cfg.TimeOutActivation = HAL_OSPI_TIMEOUT_COUNTER_DISABLE;

  if (HAL_OSPI_MemoryMapped(Ctx, &s_mem_mapped_cfg) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/**
  * @brief  Flash suspend program or erase command
  *         SPI/OPI
  * @param  Ctx Component object pointer
  * @param  Mode Interface select
  * @param  Rate Transfer rate STR or DTR
  * @retval error status
  */
int32_t MX25LM51245G_Suspend(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode, MX25LM51245G_Transfer_t Rate)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* SPI mode and DTR transfer not supported by memory */
  if ((Mode == MX25LM51245G_SPI_MODE) && (Rate == MX25LM51245G_DTR_TRANSFER))
  {
    return MX25LM51245G_ERROR;
  }

  /* Initialize the suspend command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_1_LINE : HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_INSTRUCTION_DTR_ENABLE : HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_8_BITS : HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = (Mode == MX25LM51245G_SPI_MODE) ? MX25LM51245G_PROG_ERASE_SUSPEND_CMD : MX25LM51245G_OCTA_PROG_ERASE_SUSPEND_CMD;
  s_command.AddressMode        = HAL_OSPI_ADDRESS_NONE;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = HAL_OSPI_DATA_NONE;
  s_command.DummyCycles        = 0U;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/**
  * @brief  Flash resume program or erase command
  *         SPI/OPI
  * @param  Ctx Component object pointer
  * @param  Mode Interface select
  * @param  Rate Transfer rate STR or DTR
  * @retval error status
  */
int32_t MX25LM51245G_Resume(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode, MX25LM51245G_Transfer_t Rate)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* SPI mode and DTR transfer not supported by memory */
  if ((Mode == MX25LM51245G_SPI_MODE) && (Rate == MX25LM51245G_DTR_TRANSFER))
  {
    return MX25LM51245G_ERROR;
  }

  /* Initialize the resume command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_1_LINE : HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_INSTRUCTION_DTR_ENABLE : HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_8_BITS : HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = (Mode == MX25LM51245G_SPI_MODE) ? MX25LM51245G_PROG_ERASE_RESUME_CMD : MX25LM51245G_OCTA_PROG_ERASE_RESUME_CMD;
  s_command.AddressMode        = HAL_OSPI_ADDRESS_NONE;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = HAL_OSPI_DATA_NONE;
  s_command.DummyCycles        = 0U;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/* Register/Setting Commands **************************************************/
/**
  * @brief  This function send a Write Enable and wait it is effective.
  *         SPI/OPI
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  Rate Transfer rate STR or DTR
  * @retval error status
  */
int32_t MX25LM51245G_WriteEnable(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode, MX25LM51245G_Transfer_t Rate)
{
  OSPI_RegularCmdTypeDef     s_command = {0};
  OSPI_AutoPollingTypeDef s_config = {0};

  /* SPI mode and DTR transfer not supported by memory */
  if ((Mode == MX25LM51245G_SPI_MODE) && (Rate == MX25LM51245G_DTR_TRANSFER))
  {
    return MX25LM51245G_ERROR;
  }

  /* Initialize the write enable command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_1_LINE : HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_INSTRUCTION_DTR_ENABLE : HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_8_BITS : HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = (Mode == MX25LM51245G_SPI_MODE) ? MX25LM51245G_WRITE_ENABLE_CMD : MX25LM51245G_OCTA_WRITE_ENABLE_CMD;
  s_command.AddressMode        = HAL_OSPI_ADDRESS_NONE;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = HAL_OSPI_DATA_NONE;
  s_command.DummyCycles        = 0U;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  /* Configure automatic polling mode to wait for write enabling */
  s_command.Instruction    = (Mode == MX25LM51245G_SPI_MODE) ? MX25LM51245G_READ_STATUS_REG_CMD : MX25LM51245G_OCTA_READ_STATUS_REG_CMD;
  s_command.AddressMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_ADDRESS_NONE : HAL_OSPI_ADDRESS_8_LINES;
  s_command.AddressDtrMode = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_ADDRESS_DTR_ENABLE : HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize    = HAL_OSPI_ADDRESS_32_BITS;
  s_command.Address        = 0U;
  s_command.DataMode       = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_DATA_1_LINE : HAL_OSPI_DATA_8_LINES;
  s_command.DataDtrMode    = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_DATA_DTR_ENABLE : HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles    = (Mode == MX25LM51245G_SPI_MODE) ? 0U : ((Rate == MX25LM51245G_DTR_TRANSFER) ? DUMMY_CYCLES_REG_OCTAL_DTR : DUMMY_CYCLES_REG_OCTAL);
  s_command.NbData         = (Rate == MX25LM51245G_DTR_TRANSFER) ? 2U : 1U;
  s_command.DQSMode        = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_DQS_ENABLE : HAL_OSPI_DQS_DISABLE;

  /* Send the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  s_config.Match           = 2U;
  s_config.Mask            = 2U;
  s_config.MatchMode       = HAL_OSPI_MATCH_MODE_AND;
  s_config.Interval        = MX25LM51245G_AUTOPOLLING_INTERVAL_TIME;
  s_config.AutomaticStop   = HAL_OSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_OSPI_AutoPolling(Ctx, &s_config, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/**
  * @brief  This function reset the (WEN) Write Enable Latch bit.
  *         SPI/OPI
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  Rate Transfer rate STR or DTR
  * @retval error status
  */
int32_t MX25LM51245G_WriteDisable(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode, MX25LM51245G_Transfer_t Rate)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* SPI mode and DTR transfer not supported by memory */
  if ((Mode == MX25LM51245G_SPI_MODE) && (Rate == MX25LM51245G_DTR_TRANSFER))
  {
    return MX25LM51245G_ERROR;
  }

  /* Initialize the write disable command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_1_LINE : HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_INSTRUCTION_DTR_ENABLE : HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_8_BITS : HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = (Mode == MX25LM51245G_SPI_MODE) ? MX25LM51245G_WRITE_DISABLE_CMD : MX25LM51245G_OCTA_WRITE_DISABLE_CMD;
  s_command.AddressMode        = HAL_OSPI_ADDRESS_NONE;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = HAL_OSPI_DATA_NONE;
  s_command.DummyCycles        = 0U;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/**
  * @brief  Read Flash Status register value
  *         SPI/OPI
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  Rate Transfer rate STR or DTR
  * @param  Value Status register value pointer
  * @retval error status
  */
int32_t MX25LM51245G_ReadStatusRegister(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode, MX25LM51245G_Transfer_t Rate, uint8_t *Value)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* SPI mode and DTR transfer not supported by memory */
  if ((Mode == MX25LM51245G_SPI_MODE) && (Rate == MX25LM51245G_DTR_TRANSFER))
  {
    return MX25LM51245G_ERROR;
  }

  /* Initialize the reading of status register */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_1_LINE : HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_INSTRUCTION_DTR_ENABLE : HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_8_BITS : HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = (Mode == MX25LM51245G_SPI_MODE) ? MX25LM51245G_READ_STATUS_REG_CMD : MX25LM51245G_OCTA_READ_STATUS_REG_CMD;
  s_command.AddressMode        = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_ADDRESS_NONE : HAL_OSPI_ADDRESS_8_LINES;
  s_command.AddressDtrMode     = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_ADDRESS_DTR_ENABLE : HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  s_command.Address            = 0U;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_DATA_1_LINE : HAL_OSPI_DATA_8_LINES;
  s_command.DataDtrMode        = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_DATA_DTR_ENABLE : HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = (Mode == MX25LM51245G_SPI_MODE) ? 0U : ((Rate == MX25LM51245G_DTR_TRANSFER) ? DUMMY_CYCLES_REG_OCTAL_DTR : DUMMY_CYCLES_REG_OCTAL);
  s_command.NbData             = (Rate == MX25LM51245G_DTR_TRANSFER) ? 2U : 1U;
  s_command.DQSMode            = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_DQS_ENABLE : HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive(Ctx, Value, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/**
  * @brief  Write Flash Status register
  *         SPI/OPI
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  Rate Transfer rate STR or DTR
  * @param  Value Value to write to Status register
  * @retval error status
  */
int32_t MX25LM51245G_WriteStatusRegister(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode, MX25LM51245G_Transfer_t Rate, uint8_t Value)
{
  OSPI_RegularCmdTypeDef s_command = {0};
  uint8_t reg[2];

  /* SPI mode and DTR transfer not supported by memory */
  if ((Mode == MX25LM51245G_SPI_MODE) && (Rate == MX25LM51245G_DTR_TRANSFER))
  {
    return MX25LM51245G_ERROR;
  }

  /* In SPI mode, the status register is configured with configuration register */
  if (Mode == MX25LM51245G_SPI_MODE)
  {
    if (MX25LM51245G_ReadCfgRegister(Ctx, Mode, Rate, &reg[1]) != MX25LM51245G_OK)
    {
      return MX25LM51245G_ERROR;
    }
  }
  reg[0] = Value;

  /* Initialize the writing of status register */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_1_LINE : HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_INSTRUCTION_DTR_ENABLE : HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_8_BITS : HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = (Mode == MX25LM51245G_SPI_MODE) ? MX25LM51245G_WRITE_STATUS_REG_CMD : MX25LM51245G_OCTA_WRITE_STATUS_REG_CMD;
  s_command.AddressMode        = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_ADDRESS_NONE : HAL_OSPI_ADDRESS_8_LINES;
  s_command.AddressDtrMode     = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_ADDRESS_DTR_ENABLE : HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  s_command.Address            = 0U;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_DATA_1_LINE : HAL_OSPI_DATA_8_LINES;
  s_command.DataDtrMode        = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_DATA_DTR_ENABLE : HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = 0U;
  s_command.NbData             = (Mode == MX25LM51245G_SPI_MODE) ? 2U : ((Rate == MX25LM51245G_DTR_TRANSFER) ? 2U : 1U);
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  if (HAL_OSPI_Transmit(Ctx, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/**
  * @brief  Write Flash configuration register
  *         SPI/OPI
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  Rate Transfer rate STR or DTR
  * @param  Value Value to write to configuration register
  * @retval error status
  */
int32_t MX25LM51245G_WriteCfgRegister(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode, MX25LM51245G_Transfer_t Rate, uint8_t Value)
{
  OSPI_RegularCmdTypeDef s_command = {0};
  uint8_t reg[2];

  /* SPI mode and DTR transfer not supported by memory */
  if ((Mode == MX25LM51245G_SPI_MODE) && (Rate == MX25LM51245G_DTR_TRANSFER))
  {
    return MX25LM51245G_ERROR;
  }

  /* In SPI mode, the configuration register is configured with status register */
  if (Mode == MX25LM51245G_SPI_MODE)
  {
    if (MX25LM51245G_ReadStatusRegister(Ctx, Mode, Rate, &reg[0]) != MX25LM51245G_OK)
    {
      return MX25LM51245G_ERROR;
    }
    reg[1] = Value;
  }
  else
  {
    reg[0] = Value;
  }

  /* Initialize the writing of configuration register */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_1_LINE : HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_INSTRUCTION_DTR_ENABLE : HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_8_BITS : HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = (Mode == MX25LM51245G_SPI_MODE) ? MX25LM51245G_WRITE_STATUS_REG_CMD : MX25LM51245G_OCTA_WRITE_STATUS_REG_CMD;
  s_command.AddressMode        = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_ADDRESS_NONE : HAL_OSPI_ADDRESS_8_LINES;
  s_command.AddressDtrMode     = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_ADDRESS_DTR_ENABLE : HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  s_command.Address            = 1U;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_DATA_1_LINE : HAL_OSPI_DATA_8_LINES;
  s_command.DataDtrMode        = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_DATA_DTR_ENABLE : HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = 0U;
  s_command.NbData             = (Mode == MX25LM51245G_SPI_MODE) ? 2U : ((Rate == MX25LM51245G_DTR_TRANSFER) ? 2U : 1U);
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  if (HAL_OSPI_Transmit(Ctx, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/**
  * @brief  Read Flash configuration register value
  *         SPI/OPI
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  Rate Transfer rate STR or DTR
  * @param  Value configuration register value pointer
  * @retval error status
  */
int32_t MX25LM51245G_ReadCfgRegister(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode, MX25LM51245G_Transfer_t Rate, uint8_t *Value)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* SPI mode and DTR transfer not supported by memory */
  if ((Mode == MX25LM51245G_SPI_MODE) && (Rate == MX25LM51245G_DTR_TRANSFER))
  {
    return MX25LM51245G_ERROR;
  }

  /* Initialize the reading of configuration register */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_1_LINE : HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_INSTRUCTION_DTR_ENABLE : HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_8_BITS : HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = (Mode == MX25LM51245G_SPI_MODE) ? MX25LM51245G_READ_CFG_REG_CMD : MX25LM51245G_OCTA_READ_CFG_REG_CMD;
  s_command.AddressMode        = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_ADDRESS_NONE : HAL_OSPI_ADDRESS_8_LINES;
  s_command.AddressDtrMode     = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_ADDRESS_DTR_ENABLE : HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  s_command.Address            = 1U;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_DATA_1_LINE : HAL_OSPI_DATA_8_LINES;
  s_command.DataDtrMode        = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_DATA_DTR_ENABLE : HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = (Mode == MX25LM51245G_SPI_MODE) ? 0U : ((Rate == MX25LM51245G_DTR_TRANSFER) ? DUMMY_CYCLES_REG_OCTAL_DTR : DUMMY_CYCLES_REG_OCTAL);
  s_command.NbData             = (Rate == MX25LM51245G_DTR_TRANSFER) ? 2U : 1U;
  s_command.DQSMode            = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_DQS_ENABLE : HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive(Ctx, Value, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/**
  * @brief  Write Flash configuration register 2
  *         SPI/OPI
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  Rate Transfer rate STR or DTR
  * @param  Value Value to write to configuration register
  * @retval error status
  */
int32_t MX25LM51245G_WriteCfg2Register(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode, MX25LM51245G_Transfer_t Rate, uint32_t WriteAddr, uint8_t Value)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* SPI mode and DTR transfer not supported by memory */
  if ((Mode == MX25LM51245G_SPI_MODE) && (Rate == MX25LM51245G_DTR_TRANSFER))
  {
    return MX25LM51245G_ERROR;
  }

  /* Initialize the writing of configuration register 2 */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_1_LINE : HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_INSTRUCTION_DTR_ENABLE : HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_8_BITS : HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = (Mode == MX25LM51245G_SPI_MODE) ? MX25LM51245G_WRITE_CFG_REG2_CMD : MX25LM51245G_OCTA_WRITE_CFG_REG2_CMD;
  s_command.AddressMode        = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_ADDRESS_1_LINE : HAL_OSPI_ADDRESS_8_LINES;
  s_command.AddressDtrMode     = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_ADDRESS_DTR_ENABLE : HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  s_command.Address            = WriteAddr;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_DATA_1_LINE : HAL_OSPI_DATA_8_LINES;
  s_command.DataDtrMode        = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_DATA_DTR_ENABLE : HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = 0U;
  s_command.NbData             = (Mode == MX25LM51245G_SPI_MODE) ? 1U : ((Rate == MX25LM51245G_DTR_TRANSFER) ? 2U : 1U);
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  if (HAL_OSPI_Transmit(Ctx, &Value, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/**
  * @brief  Read Flash configuration register 2 value
  *         SPI/OPI
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  Rate Transfer rate STR or DTR
  * @param  Value configuration register 2 value pointer
  * @retval error status
  */
int32_t MX25LM51245G_ReadCfg2Register(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode, MX25LM51245G_Transfer_t Rate, uint32_t ReadAddr, uint8_t *Value)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* SPI mode and DTR transfer not supported by memory */
  if ((Mode == MX25LM51245G_SPI_MODE) && (Rate == MX25LM51245G_DTR_TRANSFER))
  {
    return MX25LM51245G_ERROR;
  }

  /* Initialize the reading of status register */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_1_LINE : HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_INSTRUCTION_DTR_ENABLE : HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_8_BITS : HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = (Mode == MX25LM51245G_SPI_MODE) ? MX25LM51245G_READ_CFG_REG2_CMD : MX25LM51245G_OCTA_READ_CFG_REG2_CMD;
  s_command.AddressMode        = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_ADDRESS_1_LINE : HAL_OSPI_ADDRESS_8_LINES;
  s_command.AddressDtrMode     = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_ADDRESS_DTR_ENABLE : HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  s_command.Address            = ReadAddr;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_DATA_1_LINE : HAL_OSPI_DATA_8_LINES;
  s_command.DataDtrMode        = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_DATA_DTR_ENABLE : HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = (Mode == MX25LM51245G_SPI_MODE) ? 0U : ((Rate == MX25LM51245G_DTR_TRANSFER) ? DUMMY_CYCLES_REG_OCTAL_DTR : DUMMY_CYCLES_REG_OCTAL);
  s_command.NbData             = (Rate == MX25LM51245G_DTR_TRANSFER) ? 2U : 1U;
  s_command.DQSMode            = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_DQS_ENABLE : HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive(Ctx, Value, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/**
  * @brief  Write Flash Security register
  *         SPI/OPI
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  Rate Transfer rate STR or DTR
  * @param  Value Value to write to Security register
  * @retval error status
  */
int32_t MX25LM51245G_WriteSecurityRegister(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode, MX25LM51245G_Transfer_t Rate, uint8_t Value)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Value);

  OSPI_RegularCmdTypeDef s_command = {0};

  /* SPI mode and DTR transfer not supported by memory */
  if ((Mode == MX25LM51245G_SPI_MODE) && (Rate == MX25LM51245G_DTR_TRANSFER))
  {
    return MX25LM51245G_ERROR;
  }

  /* Initialize the write of security register */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_1_LINE : HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_INSTRUCTION_DTR_ENABLE : HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_8_BITS : HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = (Mode == MX25LM51245G_SPI_MODE) ? MX25LM51245G_WRITE_SECURITY_REG_CMD : MX25LM51245G_OCTA_WRITE_SECURITY_REG_CMD;
  s_command.AddressMode        = HAL_OSPI_ADDRESS_NONE;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = HAL_OSPI_DATA_NONE;
  s_command.DummyCycles        = 0U;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/**
  * @brief  Read Flash Security register value
  *         SPI/OPI
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  Rate Transfer rate STR or DTR
  * @param  Value Security register value pointer
  * @retval error status
  */
int32_t MX25LM51245G_ReadSecurityRegister(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode, MX25LM51245G_Transfer_t Rate, uint8_t *Value)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* SPI mode and DTR transfer not supported by memory */
  if ((Mode == MX25LM51245G_SPI_MODE) && (Rate == MX25LM51245G_DTR_TRANSFER))
  {
    return MX25LM51245G_ERROR;
  }

  /* Initialize the reading of security register */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_1_LINE : HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_INSTRUCTION_DTR_ENABLE : HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_8_BITS : HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = (Mode == MX25LM51245G_SPI_MODE) ? MX25LM51245G_READ_SECURITY_REG_CMD : MX25LM51245G_OCTA_READ_SECURITY_REG_CMD;
  s_command.AddressMode        = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_ADDRESS_NONE : HAL_OSPI_ADDRESS_8_LINES;
  s_command.AddressDtrMode     = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_ADDRESS_DTR_ENABLE : HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  s_command.Address            = 0U;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_DATA_1_LINE : HAL_OSPI_DATA_8_LINES;
  s_command.DataDtrMode        = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_DATA_DTR_ENABLE : HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = (Mode == MX25LM51245G_SPI_MODE) ? 0U : ((Rate == MX25LM51245G_DTR_TRANSFER) ? DUMMY_CYCLES_REG_OCTAL_DTR : DUMMY_CYCLES_REG_OCTAL);
  s_command.NbData             = (Rate == MX25LM51245G_DTR_TRANSFER) ? 2U : 1U;
  s_command.DQSMode            = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_DQS_ENABLE : HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive(Ctx, Value, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}


/* ID Commands ****************************************************************/
/**
  * @brief  Read Flash 3 Byte IDs.
  *         Manufacturer ID, Memory type, Memory density
  *         SPI/OPI; 1-0-1/1-0-8
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  ID 3 bytes IDs pointer
  * @param  DualFlash Dual flash mode state
  * @retval error status
  */
int32_t MX25LM51245G_ReadID(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode, MX25LM51245G_Transfer_t Rate, uint8_t *ID)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* SPI mode and DTR transfer not supported by memory */
  if ((Mode == MX25LM51245G_SPI_MODE) && (Rate == MX25LM51245G_DTR_TRANSFER))
  {
    return MX25LM51245G_ERROR;
  }

  /* Initialize the read ID command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_1_LINE : HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_INSTRUCTION_DTR_ENABLE : HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_8_BITS : HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = (Mode == MX25LM51245G_SPI_MODE) ? MX25LM51245G_READ_ID_CMD : MX25LM51245G_OCTA_READ_ID_CMD;
  s_command.AddressMode        = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_ADDRESS_NONE : HAL_OSPI_ADDRESS_8_LINES;
  s_command.AddressDtrMode     = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_ADDRESS_DTR_ENABLE : HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  s_command.Address            = 0U;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_DATA_1_LINE : HAL_OSPI_DATA_8_LINES;
  s_command.DataDtrMode        = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_DATA_DTR_ENABLE : HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = (Mode == MX25LM51245G_SPI_MODE) ? 0U : ((Rate == MX25LM51245G_DTR_TRANSFER) ? DUMMY_CYCLES_REG_OCTAL_DTR : DUMMY_CYCLES_REG_OCTAL);
  s_command.NbData             = 3U;
  s_command.DQSMode            = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_DQS_ENABLE : HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive(Ctx, ID, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/* Reset Commands *************************************************************/
/**
  * @brief  Flash reset enable command
  *         SPI/OPI
  * @param  Ctx Component object pointer
  * @param  Mode Interface select
  * @param  Rate Transfer rate STR or DTR
  * @retval error status
  */
int32_t MX25LM51245G_ResetEnable(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode, MX25LM51245G_Transfer_t Rate)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* SPI mode and DTR transfer not supported by memory */
  if ((Mode == MX25LM51245G_SPI_MODE) && (Rate == MX25LM51245G_DTR_TRANSFER))
  {
    return MX25LM51245G_ERROR;
  }

  /* Initialize the reset enable command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_1_LINE : HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_INSTRUCTION_DTR_ENABLE : HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_8_BITS : HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = (Mode == MX25LM51245G_SPI_MODE) ? MX25LM51245G_RESET_ENABLE_CMD : MX25LM51245G_OCTA_RESET_ENABLE_CMD;
  s_command.AddressMode        = HAL_OSPI_ADDRESS_NONE;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = HAL_OSPI_DATA_NONE;
  s_command.DummyCycles        = 0U;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/**
  * @brief  Flash reset memory command
  *         SPI/OPI
  * @param  Ctx Component object pointer
  * @param  Mode Interface select
  * @param  Rate Transfer rate STR or DTR
  * @retval error status
  */
int32_t MX25LM51245G_ResetMemory(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode, MX25LM51245G_Transfer_t Rate)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* SPI mode and DTR transfer not supported by memory */
  if ((Mode == MX25LM51245G_SPI_MODE) && (Rate == MX25LM51245G_DTR_TRANSFER))
  {
    return MX25LM51245G_ERROR;
  }

  /* Initialize the reset enable command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_1_LINE : HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_INSTRUCTION_DTR_ENABLE : HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_8_BITS : HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = (Mode == MX25LM51245G_SPI_MODE) ? MX25LM51245G_RESET_MEMORY_CMD : MX25LM51245G_OCTA_RESET_MEMORY_CMD;
  s_command.AddressMode        = HAL_OSPI_ADDRESS_NONE;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = HAL_OSPI_DATA_NONE;
  s_command.DummyCycles        = 0U;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/**
  * @brief  Flash no operation command
  *         SPI/OPI
  * @param  Ctx Component object pointer
  * @param  Mode Interface select
  * @param  Rate Transfer rate STR or DTR
  * @retval error status
  */
int32_t MX25LM51245G_NoOperation(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode, MX25LM51245G_Transfer_t Rate)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* SPI mode and DTR transfer not supported by memory */
  if ((Mode == MX25LM51245G_SPI_MODE) && (Rate == MX25LM51245G_DTR_TRANSFER))
  {
    return MX25LM51245G_ERROR;
  }

  /* Initialize the no operation command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_1_LINE : HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_INSTRUCTION_DTR_ENABLE : HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_8_BITS : HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = (Mode == MX25LM51245G_SPI_MODE) ? MX25LM51245G_NOP_CMD : MX25LM51245G_OCTA_NOP_CMD;
  s_command.AddressMode        = HAL_OSPI_ADDRESS_NONE;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = HAL_OSPI_DATA_NONE;
  s_command.DummyCycles        = 0U;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
}

/**
  * @brief  Flash enter deep power-down command
  *         SPI/OPI
  * @param  Ctx Component object pointer
  * @param  Mode Interface select
  * @param  Rate Transfer rate STR or DTR
  * @retval error status
  */
int32_t MX25LM51245G_EnterPowerDown(OSPI_HandleTypeDef *Ctx, MX25LM51245G_Interface_t Mode, MX25LM51245G_Transfer_t Rate)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* SPI mode and DTR transfer not supported by memory */
  if ((Mode == MX25LM51245G_SPI_MODE) && (Rate == MX25LM51245G_DTR_TRANSFER))
  {
    return MX25LM51245G_ERROR;
  }

  /* Initialize the enter power down command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_1_LINE : HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDtrMode = (Rate == MX25LM51245G_DTR_TRANSFER) ? HAL_OSPI_INSTRUCTION_DTR_ENABLE : HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = (Mode == MX25LM51245G_SPI_MODE) ? HAL_OSPI_INSTRUCTION_8_BITS : HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = (Mode == MX25LM51245G_SPI_MODE) ? MX25LM51245G_ENTER_DEEP_POWER_DOWN_CMD : MX25LM51245G_OCTA_ENTER_DEEP_POWER_DOWN_CMD;
  s_command.AddressMode        = HAL_OSPI_ADDRESS_NONE;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode           = HAL_OSPI_DATA_NONE;
  s_command.DummyCycles        = 0U;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_OSPI_Command(Ctx, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MX25LM51245G_ERROR;
  }

  return MX25LM51245G_OK;
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
