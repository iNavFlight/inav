/**
  ******************************************************************************
  * @file    MT25TL01G.c
  * @author  MCD Application Team
  * @brief   This file provides the MT25TL01G QSPI driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
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
#include "mt25tl01g.h"
/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @addtogroup MT25TL01G
  * @brief     This file provides a set of functions needed to drive the
  *             MT25TL01G QSPI memory.
  * @{
  */
/** @defgroup MT25TL01G_Exported_Functions MT25TL01G Exported Functions
  * @{
  */

/**
  * @brief  Return the configuration of the QSPI memory.
  * @param  pInfo pointer on the configuration structure
  * @retval QSPI memory status
  */
int32_t MT25TL01G_GetFlashInfo(MT25TL01G_Info_t *pInfo)
{
  pInfo->FlashSize          = MT25TL01G_FLASH_SIZE;
  pInfo->EraseSectorSize    = (2 * MT25TL01G_SUBSECTOR_SIZE);
  pInfo->ProgPageSize       = MT25TL01G_PAGE_SIZE;
  pInfo->EraseSectorsNumber = (MT25TL01G_FLASH_SIZE/pInfo->EraseSectorSize);
  pInfo->ProgPagesNumber    = (MT25TL01G_FLASH_SIZE/pInfo->ProgPageSize);
  return MT25TL01G_OK;
}

/**
  * @brief  This function set the QSPI memory in 4-byte address mode
  *          SPI/QPI; 1-0-1/4-0-4
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */
int32_t MT25TL01G_Enter4BytesAddressMode(QSPI_HandleTypeDef *Ctx, MT25TL01G_Interface_t Mode)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the command */
  s_command.InstructionMode   = (Mode == MT25TL01G_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = MT25TL01G_ENTER_4_BYTE_ADDR_MODE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /*write enable */
  if( MT25TL01G_WriteEnable(Ctx,Mode)!=MT25TL01G_OK)
  {
    return MT25TL01G_ERROR_COMMAND;
  }
  /* Send the command */
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_COMMAND;
  }

  /* Configure automatic polling mode to wait the memory is ready */
  else if(MT25TL01G_AutoPollingMemReady(Ctx,Mode)!=MT25TL01G_OK)
  {
    return MT25TL01G_ERROR_COMMAND;
  }

  return MT25TL01G_OK;
}

/**
  * @brief  Flash exit 4 Byte address mode. Effect 3/4 address byte commands only.
  *         SPI/QPI; 1-0-0/4-0-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */
int32_t MT25TL01G_Exit4BytesAddressMode(QSPI_HandleTypeDef *Ctx, MT25TL01G_Interface_t Mode)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the command */
  s_command.InstructionMode   = (Mode == MT25TL01G_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = MT25TL01G_EXIT_4_BYTE_ADDR_MODE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;


  /* Send the command */
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_COMMAND;
  }

  return MT25TL01G_OK;
}
/**
  * @brief  Polling WIP(Write In Progress) bit become to 0
  *         SPI/QPI;4-0-4
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */
int32_t MT25TL01G_AutoPollingMemReady(QSPI_HandleTypeDef *Ctx, MT25TL01G_Interface_t Mode)
{

  QSPI_CommandTypeDef     s_command;
  QSPI_AutoPollingTypeDef s_config;

  /* Configure automatic polling mode to wait for memory ready */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = MT25TL01G_READ_STATUS_REG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.DummyCycles       = 2;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  s_config.Match           = 0;
  s_config.MatchMode       = QSPI_MATCH_MODE_AND;
  s_config.Interval        = 0x10;
  s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;
  s_config.Mask            = MT25TL01G_SR_WIP | (MT25TL01G_SR_WIP <<8);
  s_config.StatusBytesSize = 2;

  if (HAL_QSPI_AutoPolling(Ctx, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_AUTOPOLLING;
  }

  return MT25TL01G_OK;

}
/**
  * @brief  This function send a Write Enable and wait it is effective.
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */

int32_t MT25TL01G_WriteEnable(QSPI_HandleTypeDef *Ctx, MT25TL01G_Interface_t Mode)
{
  QSPI_CommandTypeDef     s_command;
  QSPI_AutoPollingTypeDef s_config;

  /* Enable write operations */
  s_command.InstructionMode   = (Mode == MT25TL01G_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;

  s_command.Instruction       = MT25TL01G_WRITE_ENABLE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_COMMAND;
  }

  /* Configure automatic polling mode to wait for write enabling */
  s_config.Match           = MT25TL01G_SR_WREN | (MT25TL01G_SR_WREN << 8);
  s_config.Mask            = MT25TL01G_SR_WREN | (MT25TL01G_SR_WREN << 8);
  s_config.MatchMode       = QSPI_MATCH_MODE_AND;
  s_config.StatusBytesSize = 2;
  s_config.Interval        = 0x10;
  s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  s_command.Instruction    = MT25TL01G_READ_STATUS_REG_CMD;
  s_command.DataMode       = (Mode == MT25TL01G_QPI_MODE) ? QSPI_DATA_4_LINES : QSPI_DATA_1_LINE;


  if (HAL_QSPI_AutoPolling(Ctx, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_AUTOPOLLING;
  }

  return MT25TL01G_OK;
}

/**
  * @brief  This function reset the (WEL) Write Enable Latch bit.
  *         SPI/QPI; 1-0-0/4-0-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */
int32_t MT25TL01G_WriteDisable(QSPI_HandleTypeDef *Ctx, MT25TL01G_Interface_t Mode)
{
  QSPI_CommandTypeDef     s_command;
  /* Enable write operations */
  s_command.InstructionMode   = (Mode == MT25TL01G_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = MT25TL01G_WRITE_DISABLE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_COMMAND;
  }
  return MT25TL01G_OK;
}


/**
  * @brief  Writes an amount of data to the QSPI memory.
  *         SPI/QPI; 1-1-1/1-2-2/1-4-4/4-4-4
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  pData Pointer to data to be written
  * @param  WriteAddr Write start address
  * @param  Size Size of data to write. Range 1 ~ 256
  * @retval QSPI memory status
  */

int32_t MT25TL01G_PageProgram(QSPI_HandleTypeDef *Ctx, MT25TL01G_Interface_t Mode, uint8_t *pData, uint32_t WriteAddr, uint32_t Size)
{
  QSPI_CommandTypeDef s_command;
  switch(Mode)
  {

  case MT25TL01G_SPI_MODE :                   /* 1-1-1 commands, Power on H/W default setting */
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction     = MT25TL01G_PAGE_PROG_CMD;
    s_command.AddressMode     = QSPI_ADDRESS_1_LINE;
    s_command.DataMode        = QSPI_DATA_1_LINE;
    break;

  case MT25TL01G_SPI_2IO_MODE :               /*  1-2-2 commands */
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction     = MT25TL01G_EXT_DUAL_IN_FAST_PROG_CMD;
    s_command.AddressMode     = QSPI_ADDRESS_2_LINES;
    s_command.DataMode        = QSPI_DATA_2_LINES;
    break;

  case MT25TL01G_SPI_4IO_MODE :               /* 1-4-4 program commands */
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction     = MT25TL01G_EXT_QUAD_IN_FAST_PROG_CMD;
    s_command.AddressMode     = QSPI_ADDRESS_4_LINES;
    s_command.DataMode        = QSPI_DATA_4_LINES;
    break;

  case MT25TL01G_QPI_MODE :                   /* 4-4-4 commands */
    s_command.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    s_command.Instruction     = MT25TL01G_QUAD_IN_FAST_PROG_4_BYTE_ADDR_CMD;
    s_command.AddressMode     = QSPI_ADDRESS_4_LINES;
    s_command.DataMode        = QSPI_DATA_4_LINES;
    break;

  }

  s_command.AddressSize       = QSPI_ADDRESS_32_BITS;
  s_command.Address           = WriteAddr;
  s_command.NbData            = Size;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_COMMAND;
  }
  if (HAL_QSPI_Transmit(Ctx, pData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_TRANSMIT;
  }
  return MT25TL01G_OK;
}
/**
  * @brief  Reads an amount of data from the QSPI memory on DTR mode.
  *         SPI/QPI; 1-1-1/1-1-2/1-4-4/4-4-4
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  pData Pointer to data to be read
  * @param  ReadAddr Read start address
  * @param  Size Size of data to read
  * @retval QSPI memory status
  */
int32_t MT25TL01G_ReadDTR(QSPI_HandleTypeDef *Ctx, MT25TL01G_Interface_t Mode, uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
  QSPI_CommandTypeDef s_command;
  switch(Mode)
  {
  case MT25TL01G_SPI_MODE:                /* 1-1-1 commands, Power on H/W default setting */
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = MT25TL01G_FAST_READ_4_BYTE_DTR_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
    s_command.DataMode          = QSPI_DATA_1_LINE;

    break;
  case MT25TL01G_SPI_2IO_MODE:           /* 1-1-2 read commands */

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = MT25TL01G_DUAL_OUT_FAST_READ_DTR_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
    s_command.DataMode          = QSPI_DATA_2_LINES;

    break;
  case MT25TL01G_SPI_4IO_MODE:             /* 1-4-4 read commands */

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = MT25TL01G_QUAD_INOUT_FAST_READ_4_BYTE_DTR_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
    s_command.DataMode          = QSPI_DATA_4_LINES;

    break;
  case MT25TL01G_QPI_MODE:                 /* 4-4-4 commands */
    s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
    s_command.Instruction       = MT25TL01G_QUAD_INOUT_FAST_READ_DTR_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
    s_command.DataMode          = QSPI_DATA_4_LINES;

    break;
  }
  /* Initialize the read command */
  s_command.DummyCycles       = MT25TL01G_DUMMY_CYCLES_READ_QUAD_DTR;
  s_command.AddressSize       = QSPI_ADDRESS_32_BITS;
  s_command.Address           = ReadAddr;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.NbData            = Size;
  s_command.DdrMode           = QSPI_DDR_MODE_ENABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_HALF_CLK_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_COMMAND;
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive(Ctx, pData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_RECEIVE;
  }

  return MT25TL01G_OK;

}

/**
  * @brief  Reads an amount of data from the QSPI memory on STR mode.
  *         SPI/QPI; 1-1-1/1-2-2/1-4-4/4-4-4
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  pData Pointer to data to be read
  * @param  ReadAddr Read start address
  * @param  Size Size of data to read
  * @retval QSPI memory status
  */

int32_t MT25TL01G_ReadSTR(QSPI_HandleTypeDef *Ctx, MT25TL01G_Interface_t Mode, uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
  QSPI_CommandTypeDef s_command;
  switch(Mode)
  {
  case MT25TL01G_SPI_MODE:           /* 1-1-1 read commands */
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = MT25TL01G_FAST_READ_4_BYTE_ADDR_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
    s_command.DataMode          = QSPI_DATA_1_LINE;


    break;
  case MT25TL01G_SPI_2IO_MODE:           /* 1-2-2 read commands */

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = MT25TL01G_DUAL_INOUT_FAST_READ_4_BYTE_ADDR_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_2_LINES;
    s_command.DataMode          = QSPI_DATA_2_LINES;

    break;
  case MT25TL01G_SPI_4IO_MODE:             /* 1-4-4 read commands */

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = MT25TL01G_QUAD_INOUT_FAST_READ_4_BYTE_ADDR_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
    s_command.DataMode          = QSPI_DATA_4_LINES;

    break;
  case MT25TL01G_QPI_MODE:                 /* 4-4-4 commands */
    s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
    s_command.Instruction       = MT25TL01G_QUAD_INOUT_FAST_READ_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
    s_command.DataMode          = QSPI_DATA_4_LINES;

    break;
  }
  /* Initialize the read command */
  s_command.DummyCycles       = MT25TL01G_DUMMY_CYCLES_READ;
  s_command.AddressSize       = QSPI_ADDRESS_32_BITS;
  s_command.Address           = ReadAddr;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.NbData            = Size;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_COMMAND;
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive(Ctx, pData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_RECEIVE;
  }


  return MT25TL01G_OK;

}

/**
  * @brief  Erases the specified block of the QSPI memory.
  *         MT25TL01G support 4K, 32K, 64K size block erase commands.
  *         SPI/QPI; 1-1-0/4-4-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  BlockAddress Block address to erase
  * @retval QSPI memory status
  */

int32_t MT25TL01G_BlockErase(QSPI_HandleTypeDef *Ctx, MT25TL01G_Interface_t Mode ,uint32_t BlockAddress, MT25TL01G_Erase_t BlockSize)
{
  QSPI_CommandTypeDef s_command;
  switch(BlockSize)
  {
  default :
  case MT25TL01G_ERASE_4K :
    s_command.Instruction     = MT25TL01G_SUBSECTOR_ERASE_4_BYTE_ADDR_CMD_4K;
    break;

  case MT25TL01G_ERASE_32K :
    s_command.Instruction     = MT25TL01G_SUBSECTOR_ERASE_CMD_32K;
    break;

  case MT25TL01G_ERASE_64K :
    s_command.Instruction     = MT25TL01G_SECTOR_ERASE_4_BYTE_ADDR_CMD;
    break;

  case MT25TL01G_ERASE_CHIP :
    return MT25TL01G_ChipErase(Ctx, Mode);
  }
  /* Initialize the erase command */

  s_command.InstructionMode   = (Mode == MT25TL01G_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
  s_command.AddressSize       = QSPI_ADDRESS_32_BITS;
  s_command.Address           = BlockAddress;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  MT25TL01G_WriteEnable(Ctx,Mode);
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_COMMAND;
  }
  MT25TL01G_AutoPollingMemReady(Ctx,Mode);
  return MT25TL01G_OK;
}

/**
  * @brief  Whole chip erase.
  *         SPI/QPI; 1-0-0/4-0-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */

int32_t MT25TL01G_ChipErase(QSPI_HandleTypeDef *Ctx, MT25TL01G_Interface_t Mode)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the erase command */
  s_command.InstructionMode   = (Mode == MT25TL01G_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = MT25TL01G_DIE_ERASE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_COMMAND;
  }

  return MT25TL01G_OK;

}
/**
  * @brief  Read Flash Status register value
  *         SPI/QPI; 1-0-1/4-0-4
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  Value pointer to status register value
  * @retval QSPI memory status
  */
int32_t MT25TL01G_ReadStatusRegister(QSPI_HandleTypeDef *Ctx, MT25TL01G_Interface_t Mode, uint8_t *Value)
{
  QSPI_CommandTypeDef s_command;
  /* Initialize the read flag status register command */
  s_command.InstructionMode   = (Mode == MT25TL01G_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = MT25TL01G_READ_STATUS_REG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = (Mode == MT25TL01G_QPI_MODE) ? QSPI_DATA_4_LINES : QSPI_DATA_1_LINE;
  s_command.DummyCycles       = 0;
  s_command.NbData            = 1;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_COMMAND;
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive(Ctx,Value, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_RECEIVE;
  }

  return MT25TL01G_OK;
}

/**
  * @brief  This function put QSPI memory in QPI mode (Quad I/O) from SPI mode.
  *         SPI -> QPI; 1-x-x -> 4-4-4
  *         SPI; 1-0-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */
int32_t MT25TL01G_EnterQPIMode(QSPI_HandleTypeDef *Ctx)
{
  QSPI_CommandTypeDef s_command;

  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = MT25TL01G_ENTER_QUAD_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_COMMAND;
  }

  return MT25TL01G_OK;
}
/**
  * @brief  This function put QSPI memory in SPI mode (Single I/O) from QPI mode.
  *         QPI -> SPI; 4-4-4 -> 1-x-x
  *         QPI; 4-0-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */
int32_t MT25TL01G_ExitQPIMode(QSPI_HandleTypeDef *Ctx)
{
  QSPI_CommandTypeDef s_command;

  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = MT25TL01G_EXIT_QUAD_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_COMMAND;
  }

  return MT25TL01G_OK;
}

/**
  * @brief  Reads an amount of data from the QSPI memory on DTR mode.
  *         SPI/QPI; 1-1-1/1-1-2/1-4-4/4-4-4
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */
int32_t MT25TL01G_EnableMemoryMappedModeDTR(QSPI_HandleTypeDef *Ctx, MT25TL01G_Interface_t Mode)
{
  QSPI_CommandTypeDef      s_command;
  QSPI_MemoryMappedTypeDef s_mem_mapped_cfg;
  switch(Mode)
  {
  case MT25TL01G_SPI_MODE:                /* 1-1-1 commands, Power on H/W default setting */
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = MT25TL01G_FAST_READ_4_BYTE_DTR_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
    s_command.DataMode          = QSPI_DATA_1_LINE;

    break;
  case MT25TL01G_SPI_2IO_MODE:           /* 1-1-2 read commands */

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = MT25TL01G_DUAL_OUT_FAST_READ_DTR_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
    s_command.DataMode          = QSPI_DATA_2_LINES;

    break;
  case MT25TL01G_SPI_4IO_MODE:             /* 1-4-4 read commands */

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = MT25TL01G_QUAD_INOUT_FAST_READ_4_BYTE_DTR_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
    s_command.DataMode          = QSPI_DATA_4_LINES;

    break;
  case MT25TL01G_QPI_MODE:                 /* 4-4-4 commands */
    s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
    s_command.Instruction       = MT25TL01G_QUAD_INOUT_FAST_READ_DTR_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
    s_command.DataMode          = QSPI_DATA_4_LINES;

    break;
  }
  /* Configure the command for the read instruction */
  s_command.AddressSize       = QSPI_ADDRESS_32_BITS;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DummyCycles       = MT25TL01G_DUMMY_CYCLES_READ_QUAD_DTR;
  s_command.DdrMode           = QSPI_DDR_MODE_ENABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_HALF_CLK_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the memory mapped mode */
  s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
  s_mem_mapped_cfg.TimeOutPeriod     = 0;

  if (HAL_QSPI_MemoryMapped(Ctx, &s_command, &s_mem_mapped_cfg) != HAL_OK)
  {
    return MT25TL01G_ERROR_MEMORYMAPPED;
  }

  return MT25TL01G_OK;
}

/**
  * @brief  Reads an amount of data from the QSPI memory on STR mode.
  *         SPI/QPI; 1-1-1/1-2-2/1-4-4/4-4-4
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */

int32_t MT25TL01G_EnableMemoryMappedModeSTR(QSPI_HandleTypeDef *Ctx, MT25TL01G_Interface_t Mode)
{
  QSPI_CommandTypeDef      s_command;
  QSPI_MemoryMappedTypeDef s_mem_mapped_cfg;
  switch(Mode)
  {
  case MT25TL01G_SPI_MODE:           /* 1-1-1 read commands */
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = MT25TL01G_FAST_READ_4_BYTE_ADDR_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
    s_command.DataMode          = QSPI_DATA_1_LINE;


    break;
  case MT25TL01G_SPI_2IO_MODE:           /* 1-2-2 read commands */

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = MT25TL01G_DUAL_INOUT_FAST_READ_4_BYTE_ADDR_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_2_LINES;
    s_command.DataMode          = QSPI_DATA_2_LINES;

    break;

  case MT25TL01G_SPI_4IO_MODE:             /* 1-4-4 read commands */

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = MT25TL01G_QUAD_INOUT_FAST_READ_4_BYTE_ADDR_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
    s_command.DataMode          = QSPI_DATA_4_LINES;

    break;

  case MT25TL01G_QPI_MODE:                 /* 4-4-4 commands */
    s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
    s_command.Instruction       = MT25TL01G_QUAD_INOUT_FAST_READ_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
    s_command.DataMode          = QSPI_DATA_4_LINES;

    break;

  }
  /* Configure the command for the read instruction */
  s_command.DummyCycles       = MT25TL01G_DUMMY_CYCLES_READ;
  s_command.AddressSize       = QSPI_ADDRESS_32_BITS;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the memory mapped mode */
  s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
  s_mem_mapped_cfg.TimeOutPeriod     = 0;

  if (HAL_QSPI_MemoryMapped(Ctx, &s_command, &s_mem_mapped_cfg) != HAL_OK)
  {
    return MT25TL01G_ERROR_MEMORYMAPPED;
  }

  return MT25TL01G_OK;
}



/**
  * @brief  Flash reset enable command
  *         SPI/QPI; 1-0-0, 4-0-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */
int32_t MT25TL01G_ResetEnable(QSPI_HandleTypeDef *Ctx, MT25TL01G_Interface_t Mode)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the reset enable command */
  s_command.InstructionMode   = (Mode == MT25TL01G_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = MT25TL01G_RESET_ENABLE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_COMMAND;
  }

  return MT25TL01G_OK;
}


/**
  * @brief  Flash reset memory command
  *         SPI/QPI; 1-0-0, 4-0-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */
int32_t MT25TL01G_ResetMemory(QSPI_HandleTypeDef *Ctx, MT25TL01G_Interface_t Mode)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the reset enable command */
  s_command.InstructionMode   = (Mode == MT25TL01G_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = MT25TL01G_RESET_MEMORY_CMD ;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_COMMAND;
  }

  return MT25TL01G_OK;
}


/**
  * @brief  Read Flash 3 Byte IDs.
  *         Manufacturer ID, Memory type, Memory density
  *         SPI/QPI; 1-0-1/4-0-4
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  ID pointer to flash id value
  * @retval QSPI memory status
  */
int32_t MT25TL01G_ReadID(QSPI_HandleTypeDef *Ctx, MT25TL01G_Interface_t Mode, uint8_t *ID)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the read ID command */
  s_command.InstructionMode   = (Mode == MT25TL01G_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = (Mode == MT25TL01G_QPI_MODE) ? MT25TL01G_MULTIPLE_IO_READ_ID_CMD  : MT25TL01G_READ_ID_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DummyCycles       = 0;
  s_command.DataMode          = (Mode == MT25TL01G_QPI_MODE) ? QSPI_DATA_4_LINES : QSPI_DATA_1_LINE;
  s_command.NbData            = 3;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_COMMAND;
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive(Ctx, ID, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_RECEIVE;
  }

  return MT25TL01G_OK;
}

/**
  * @brief  Program/Erases suspend. Interruption Program/Erase operations.
  *         After the device has entered Erase-Suspended mode,
  *         system can read any address except the block/sector being Program/Erased.
  *         SPI/QPI; 1-0-0/4-0-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */

int32_t MT25TL01G_ProgEraseSuspend(QSPI_HandleTypeDef *Ctx, MT25TL01G_Interface_t Mode)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the read ID command */
  s_command.InstructionMode   = (Mode == MT25TL01G_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = MT25TL01G_PROG_ERASE_SUSPEND_CMD ;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DummyCycles       = 0;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_COMMAND;
  }

  return MT25TL01G_OK;
}

/**
  * @brief  Program/Erases resume.
  *         SPI/QPI; 1-0-0/4-0-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */
int32_t MT25TL01G_ProgEraseResume(QSPI_HandleTypeDef *Ctx, MT25TL01G_Interface_t Mode)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the read ID command */
  s_command.InstructionMode   = (Mode == MT25TL01G_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = MT25TL01G_PROG_ERASE_RESUME_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DummyCycles       = 0;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_COMMAND;
  }

  return MT25TL01G_OK;
}

/**
  * @brief  Deep power down.
  *         The device is not active and all Write/Program/Erase instruction are ignored.
  *         SPI/QPI; 1-0-0/4-0-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */
int32_t MT25TL01G_EnterDeepPowerDown(QSPI_HandleTypeDef *Ctx, MT25TL01G_Interface_t Mode)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the read ID command */
  s_command.InstructionMode   = (Mode == MT25TL01G_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = MT25TL01G_ENTER_DEEP_POWER_DOWN;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DummyCycles       = 0;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_COMMAND;
  }

  return MT25TL01G_OK;
}

/**
  * @brief  Release from deep power down.
  *         After CS# go high, system need wait tRES1 time for device ready.
  *         SPI/QPI; 1-0-0/4-0-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */
int32_t MT25TL01G_ReleaseFromDeepPowerDown(QSPI_HandleTypeDef *Ctx, MT25TL01G_Interface_t Mode)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the read ID command */
  s_command.InstructionMode   = (Mode == MT25TL01G_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = MT25TL01G_RELEASE_FROM_DEEP_POWER_DOWN ;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DummyCycles       = 0;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_COMMAND;
  }

  return MT25TL01G_OK;
}

/**
  * @brief  Read SECTOR PROTECTION Block register value.
  *         SPI; 1-0-1
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  SPBRegister pointer to SPBRegister value
  * @retval QSPI memory status
  */
int32_t MT25TL01G_ReadSPBLockRegister(QSPI_HandleTypeDef *Ctx, MT25TL01G_Interface_t Mode, uint8_t *SPBRegister)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the reading of SPB lock register command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = MT25TL01G_READ_SECTOR_PROTECTION_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DummyCycles       = 0;
  s_command.DataMode          = QSPI_DATA_1_LINE;
  s_command.NbData            = 1;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_COMMAND;
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive(Ctx, SPBRegister, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return MT25TL01G_ERROR_RECEIVE;
  }

  return MT25TL01G_OK;
}



