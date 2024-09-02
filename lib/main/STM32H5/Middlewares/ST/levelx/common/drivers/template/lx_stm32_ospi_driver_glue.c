/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

#include "lx_stm32_ospi_driver.h"

/* USER CODE BEGIN OSPI_CONFIG */
/* Sample OctoSPI glue implementation using the the HAL/OSPI polling API.
 * This has been tested with the MX25LM51245G OctoSPI memory.

   The OSPI IP was configured as below:

   Instance              = OCTOSPI1
   FifoThreshold         = 4
   DualQuad              = disabled
   MemoryType            = Macronix
   DeviceSize            = 26
   ChipSelectHighTime    = 2
   FreeRunningClock      = disabled
   ClockMode             = low
   ClockPrescaler        = 2
   SampleShifting        = none
   DelayHoldQuarterCycle = enabled
   ChipSelectBoundary    = 0
   DelayBlockBypass      = used

 * Different configuration can be used but need to be reflected in
 * the implementation guarded with OSPI_HAL_CFG_xxx user tags.
 */
 /* USER CODE END OSPI_CONFIG */

extern OSPI_HandleTypeDef hospi1;

extern void MX_OCTOSPI1_Init(void);

static uint8_t ospi_memory_reset            (OSPI_HandleTypeDef *hospi);
static uint8_t ospi_set_write_enable        (OSPI_HandleTypeDef *hospi);
static uint8_t ospi_auto_polling_ready      (OSPI_HandleTypeDef *hospi, uint32_t timeout);
static uint8_t ospi_set_octalmode           (OSPI_HandleTypeDef *hospi, uint8_t mode);

/* USER CODE BEGIN SECTOR_BUFFER */
ULONG ospi_sector_buffer[LX_STM32_OSPI_SECTOR_SIZE / sizeof(ULONG)];
/* USER CODE END SECTOR_BUFFER */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
* @brief Initializes the OSPI IP instance
* @param UINT instance OSPI instance to initialize
* @retval 0 on success error value otherwise
*/
INT lx_stm32_ospi_lowlevel_init(UINT instance)
{
  INT status = 0;

  /* USER CODE BEGIN PRE_OSPI_Init */

  LX_PARAMETER_NOT_USED(instance);

  /* USER CODE END PRE_OSPI_Init */

  /* Call the DeInit function to reset the driver */
  hospi1.Instance = OCTOSPI1;
  if (HAL_OSPI_DeInit(&hospi1) != HAL_OK)
  {
    return 1;
  }

  /* Init the OSPI */
  MX_OCTOSPI1_Init();

  /* OSPI memory reset */
  if (ospi_memory_reset(&hospi1) != 0)
  {
    return 1;
  }

  /* Insert 10 config intervals delay */
  HAL_Delay(1);

  /* Enable Octal Mode */
  if (ospi_set_octalmode(&hospi1, 1) != 0)
  {
    return 1;
  }

  /* USER CODE BEGIN POST_OSPI_Init */

  /* USER CODE END POST_OSPI_Init */

  return status;
}

/**
* @brief Get the status of the OSPI instance
* @param UINT instance OSPI instance
* @retval 0 if the OSPI is ready 1 otherwise
*/
INT lx_stm32_ospi_get_status(UINT instance)
{
  INT status = 0;

  /* USER CODE BEGIN PRE_OSPI_Get_Status */

  LX_PARAMETER_NOT_USED(instance);

  /* USER CODE END PRE_OSPI_Get_Status */

  OSPI_RegularCmdTypeDef s_command;
  uint8_t reg[2];

  /* Initialize the read status register command */
  /* USER CODE BEGIN OSPI_HAL_CFG_Get_Status */
  s_command.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId               = HAL_OSPI_FLASH_ID_1;
  s_command.Instruction           = OCTAL_READ_STATUS_REG_CMD;
  s_command.InstructionMode       = HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionSize       = HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Address               = 0;
  s_command.AddressMode           = HAL_OSPI_ADDRESS_8_LINES;
  s_command.AddressSize           = HAL_OSPI_ADDRESS_32_BITS;
  s_command.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode              = HAL_OSPI_DATA_8_LINES;
  s_command.NbData                = 2;
  s_command.DummyCycles           = LX_STM32_OSPI_DUMMY_CYCLES_READ_OCTAL;
  s_command.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* DTR mode is enabled */
  s_command.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_ENABLE;
  s_command.AddressDtrMode        = HAL_OSPI_ADDRESS_DTR_ENABLE;
  s_command.DataDtrMode           = HAL_OSPI_DATA_DTR_ENABLE;
  s_command.DQSMode               = HAL_OSPI_DQS_ENABLE;
  /* USER CODE END OSPI_HAL_CFG_Get_Status */

  /* Configure the command */
  if (HAL_OSPI_Command(&hospi1, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive(&hospi1, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* Check the value of the register */
  if ((reg[0] & LX_STM32_OSPI_SR_WIP) != 0)
  {
    return 1;
  }

  /* USER CODE BEGIN POST_OSPI_Get_Status */

  /* USER CODE END POST_OSPI_Get_Status */

  return status;
}

/**
* @brief Get size info of the flash meomory
* @param UINT instance OSPI instance
* @param ULONG * block_size pointer to be filled with Flash block size
* @param ULONG * total_blocks pointer to be filled with Flash total number of blocks
* @retval 0 on Success and block_size and total_blocks are correctly filled
          1 on Failure, block_size = 0, total_blocks = 0
*/
INT lx_stm32_ospi_get_info(UINT instance, ULONG *block_size, ULONG *total_blocks)
{
  INT status = 0;

  /* USER CODE BEGIN PRE_OSPI_Get_Info */

  LX_PARAMETER_NOT_USED(instance);

  /* USER CODE END PRE_OSPI_Get_Info */

  status = 0;
  *block_size = LX_STM32_OSPI_SECTOR_SIZE;
  *total_blocks = (LX_STM32_OSPI_FLASH_SIZE / LX_STM32_OSPI_SECTOR_SIZE);

  /* USER CODE BEGIN POST_OSPI_Get_Info */

  /* USER CODE END POST_OSPI_Get_Info */

  return status;
}

/**
* @brief Read data from the OSPI memory into a buffer
* @param UINT instance OSPI instance
* @param ULONG * address the start address to read from
* @param ULONG * buffer the destination buffer
* @param ULONG words the total number of words to be read
* @retval 0 on Success 1 on Failure
*/
INT lx_stm32_ospi_read(UINT instance, ULONG *address, ULONG *buffer, ULONG words)
{
  INT status = 0;

  /* USER CODE BEGIN PRE_OSPI_Read */

  LX_PARAMETER_NOT_USED(instance);

  /* USER CODE END PRE_OSPI_Read */

  OSPI_RegularCmdTypeDef s_command;

  /* Initialize the read command */
  /* USER CODE BEGIN OSPI_HAL_CFG_read */
  s_command.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId               = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode       = HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionSize       = HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Address               = (uint32_t)address;
  s_command.AddressMode           = HAL_OSPI_ADDRESS_8_LINES;
  s_command.AddressSize           = HAL_OSPI_ADDRESS_32_BITS;
  s_command.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode              = HAL_OSPI_DATA_8_LINES;
  s_command.NbData                = words * sizeof(ULONG);
  s_command.DummyCycles           = LX_STM32_OSPI_DUMMY_CYCLES_READ_OCTAL;
  s_command.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* DTR mode is enabled */
  s_command.Instruction           = OCTAL_IO_DTR_READ_CMD;
  s_command.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_ENABLE;
  s_command.AddressDtrMode        = HAL_OSPI_ADDRESS_DTR_ENABLE;
  s_command.DataDtrMode           = HAL_OSPI_DATA_DTR_ENABLE;
  s_command.DQSMode               = HAL_OSPI_DQS_ENABLE;
  /* USER CODE END OSPI_HAL_CFG_read */

  /* Configure the command */
  if (HAL_OSPI_Command(&hospi1, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive(&hospi1, (uint8_t*)buffer, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* USER CODE BEGIN POST_OSPI_Read */

  /* USER CODE END POST_OSPI_Read */

  return status;
}

/**
* @brief write a data buffer into the OSPI memory
* @param UINT instance OSPI instance
* @param ULONG * address the start address to write into
* @param ULONG * buffer the data source buffer
* @param ULONG words the total number of words to be written
* @retval 0 on Success 1 on Failure
*/
INT lx_stm32_ospi_write(UINT instance, ULONG *address, ULONG *buffer, ULONG words)
{
  INT status = 0;

  /* USER CODE BEGIN PRE_OSPI_Write */

  LX_PARAMETER_NOT_USED(instance);

  /* USER CODE END PRE_OSPI_Write */

  OSPI_RegularCmdTypeDef s_command;
  uint32_t end_addr, current_size, current_addr;

  /* Calculation of the size between the write address and the end of the page */
  current_size = LX_STM32_OSPI_PAGE_SIZE - ((uint32_t)address % LX_STM32_OSPI_PAGE_SIZE);

  /* Check if the size of the data is less than the remaining place in the page */
  if (current_size > (uint32_t) words * sizeof(ULONG))
  {
    current_size = (uint32_t) words * sizeof(ULONG);
  }

  /* Initialize the address variables */
  current_addr = (uint32_t)address;
  end_addr = (uint32_t)address + (uint32_t) words * sizeof(ULONG);

  /* Initialize the program command */
  /* USER CODE BEGIN OSPI_HAL_CFG_write */
  s_command.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId               = HAL_OSPI_FLASH_ID_1;
  s_command.Instruction           = OCTAL_PAGE_PROG_CMD;
  s_command.InstructionMode       = HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionSize       = HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.AddressMode           = HAL_OSPI_ADDRESS_8_LINES;
  s_command.AddressSize           = HAL_OSPI_ADDRESS_32_BITS;
  s_command.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode              = HAL_OSPI_DATA_8_LINES;
  s_command.DummyCycles           = 0;
  s_command.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* DTR mode is enabled */
  s_command.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_ENABLE;
  s_command.AddressDtrMode        = HAL_OSPI_ADDRESS_DTR_ENABLE;
  s_command.DataDtrMode           = HAL_OSPI_DATA_DTR_ENABLE;
  s_command.DQSMode               = HAL_OSPI_DQS_ENABLE;
  /* USER CODE END OSPI_HAL_CFG_write */

  /* Perform the write page by page */
  do
  {
    s_command.Address = current_addr;
    s_command.NbData  = current_size;

    /* Enable write operations */
    if (ospi_set_write_enable(&hospi1) != 0)
    {
      return 1;
    }

    /* Configure the command */
    if (HAL_OSPI_Command(&hospi1, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }

    /* Transmission of the data */
    if (HAL_OSPI_Transmit(&hospi1, (uint8_t*)buffer, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }

    /* Configure automatic polling mode to wait for end of program */
    if (ospi_auto_polling_ready(&hospi1, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != 0)
    {
      return 1;
    }

    /* Update the address and size variables for next page programming */
    current_addr += current_size;
    address += current_size;
    current_size = ((current_addr + LX_STM32_OSPI_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : LX_STM32_OSPI_PAGE_SIZE;
  } while (current_addr < end_addr);

  /* USER CODE BEGIN POST_OSPI_Write */

  /* USER CODE END POST_OSPI_Write */

  return status;
}

/**
* @brief Erase the whole flash or a single block
* @param UINT instance OSPI instance
* @param ULONG  block the block to be erased
* @param ULONG  erase_count the number of times the block was erased
* @param UINT full_chip_erase if set to 0 a single block is erased otherwise the whole flash is erased
* @retval 0 on Success 1 on Failure
*/
INT lx_stm32_ospi_erase(UINT instance, ULONG block, ULONG erase_count, UINT full_chip_erase)
{
  INT status = 0;

  /* USER CODE BEGIN PRE_OSPI_Erase */

  LX_PARAMETER_NOT_USED(instance);
  LX_PARAMETER_NOT_USED(erase_count);

  /* USER CODE END PRE_OSPI_Erase */

  OSPI_RegularCmdTypeDef s_command;

  /* Initialize the erase command */
  /* USER CODE BEGIN OSPI_HAL_CFG_erase */
  s_command.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId               = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode       = HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionSize       = HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode              = HAL_OSPI_DATA_NONE;
  s_command.DummyCycles           = 0;
  s_command.DQSMode               = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* DTR mode is enabled */
  s_command.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_ENABLE;

  if(full_chip_erase)
  {
    s_command.Instruction         = OCTAL_CHIP_ERASE_CMD;
    s_command.AddressMode         = HAL_OSPI_ADDRESS_NONE;
  }
  else
  {
    s_command.Instruction         = OCTAL_BLOCK_ERASE_CMD;
    s_command.Address             = (block * LX_STM32_OSPI_SECTOR_SIZE);
    s_command.AddressMode         = HAL_OSPI_ADDRESS_8_LINES;
    s_command.AddressSize         = HAL_OSPI_ADDRESS_32_BITS;
    s_command.AddressDtrMode      = HAL_OSPI_ADDRESS_DTR_ENABLE; /* DTR mode is enabled */
  }
  /* USER CODE END OSPI_HAL_CFG_erase */

  /* Enable write operations */
  if (ospi_set_write_enable(&hospi1) != 0)
  {
    return 1;
  }

  /* Send the command */
  if (HAL_OSPI_Command(&hospi1, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* Configure automatic polling mode to wait for end of erase */
  if (ospi_auto_polling_ready(&hospi1, LX_STM32_OSPI_BULK_ERASE_MAX_TIME) != 0)
  {
    return 1;
  }

  /* USER CODE BEGIN POST_OSPI_Erase */

  /* USER CODE END POST_OSPI_Erase */

  return status;
}

/**
* @brief Check that a block was actually erased
* @param UINT instance OSPI instance
* @param ULONG  block the block to be checked
* @retval 0 on Success 1 on Failure
*/
INT lx_stm32_ospi_is_block_erased(UINT instance, ULONG block)
{
  INT status = 0;

  /* USER CODE BEGIN OSPI_Block_Erased */

  /* USER CODE END OSPI_Block_Erased */

  return status;
}

UINT  lx_ospi_driver_system_error(UINT error_code)
{
  UINT status = LX_ERROR;

  /* USER CODE BEGIN OSPI_System_Error */

  /* USER CODE END OSPI_System_Error */

  return status;
}

/**
  * @brief  Reset the OSPI memory.
  * @param  hospi: OSPI handle pointer
  */
static uint8_t ospi_memory_reset(OSPI_HandleTypeDef *hospi)
{
  /* USER CODE BEGIN OSPI_HAL_CFG_MEMORY_RESET */

  OSPI_RegularCmdTypeDef s_command;
  OSPI_AutoPollingTypeDef s_config;

  /* Initialize the reset enable command */
  s_command.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId               = HAL_OSPI_FLASH_ID_1;
  s_command.Instruction           = RESET_ENABLE_CMD;
  s_command.InstructionMode       = HAL_OSPI_INSTRUCTION_1_LINE;
  s_command.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.AddressMode           = HAL_OSPI_ADDRESS_NONE;
  s_command.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode              = HAL_OSPI_DATA_NONE;
  s_command.DummyCycles           = 0;
  s_command.DQSMode               = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_OSPI_Command(&hospi1, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* Send the reset memory command */
  s_command.Instruction = RESET_MEMORY_CMD;
  if (HAL_OSPI_Command(&hospi1, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* Configure automatic polling mode to wait the memory is ready */
  s_command.Instruction  = READ_STATUS_REG_CMD;
  s_command.DataMode     = HAL_OSPI_DATA_1_LINE;
  s_command.NbData       = 1;
  s_command.DataDtrMode  = HAL_OSPI_DATA_DTR_DISABLE;

  s_config.Match         = 0;
  s_config.Mask          = LX_STM32_OSPI_SR_WIP;
  s_config.MatchMode     = HAL_OSPI_MATCH_MODE_AND;
  s_config.Interval      = 0x10;
  s_config.AutomaticStop = HAL_OSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_OSPI_Command(&hospi1, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  if (HAL_OSPI_AutoPolling(&hospi1, &s_config, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* USER CODE END OSPI_HAL_CFG_MEMORY_RESET */

  return 0;
}

/**
  * @brief  Send a Write Enable command and wait its effective.
  * @param  hospi: OSPI handle pointer
  */
static uint8_t ospi_set_write_enable(OSPI_HandleTypeDef *hospi)
{
  /* USER CODE BEGIN OSPI_HAL_CFG_WRITE_ENABLE */

  OSPI_RegularCmdTypeDef  s_command;
  OSPI_AutoPollingTypeDef s_config;
  uint8_t reg[2];

  /* Enable write operations */
  s_command.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId               = HAL_OSPI_FLASH_ID_1;
  s_command.Instruction           = OCTAL_WRITE_ENABLE_CMD;
  s_command.InstructionMode       = HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionSize       = HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.AddressMode           = HAL_OSPI_ADDRESS_NONE;
  s_command.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode              = HAL_OSPI_DATA_NONE;
  s_command.DummyCycles           = 0;
  s_command.DQSMode               = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* DTR mode is enabled */
  s_command.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_ENABLE;

  if (HAL_OSPI_Command(&hospi1, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* Configure automatic polling mode to wait for write enabling */
  s_config.Match           = LX_STM32_OSPI_SR_WEL;
  s_config.Mask            = LX_STM32_OSPI_SR_WEL;

  s_command.Instruction    = OCTAL_READ_STATUS_REG_CMD;
  s_command.Address        = 0x0;
  s_command.AddressMode    = HAL_OSPI_ADDRESS_8_LINES;
  s_command.AddressSize    = HAL_OSPI_ADDRESS_32_BITS;
  s_command.DataMode       = HAL_OSPI_DATA_8_LINES;
  s_command.NbData         = 2;

  /* DTR mode is enabled */
  s_command.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_ENABLE;
  s_command.DataDtrMode    = HAL_OSPI_DATA_DTR_ENABLE;
  s_command.DQSMode        = HAL_OSPI_DQS_ENABLE;
  s_command.DummyCycles    = 4;

  do
  {
    /* Wait for 10 config interval between each request */
    HAL_Delay(1); /* s_config.Interval(0x10) / Clock(55 MHz) = 0.29 ms */

    if (HAL_OSPI_Command(&hospi1, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }

    if (HAL_OSPI_Receive(&hospi1, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }
  }while((reg[0] & s_config.Mask ) != s_config.Match);

  /* USER CODE END OSPI_HAL_CFG_WRITE_ENABLE */

  return 0;
}

/**
  * @brief  Read the SR of the memory and wait the EOP.
  * @param  hospi: OSPI handle pointer
  * @param  timeout: timeout value before returning an error
  */
static uint8_t ospi_auto_polling_ready(OSPI_HandleTypeDef *hospi, uint32_t timeout)
{
  /* USER CODE BEGIN OSPI_HAL_CFG_AUTO_POLLING_READY */

  OSPI_RegularCmdTypeDef  s_command;
  OSPI_AutoPollingTypeDef s_config;
  uint8_t reg[2];

  /* Configure automatic polling mode to wait for memory ready */
  s_command.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId               = HAL_OSPI_FLASH_ID_1;
  s_command.Instruction           = OCTAL_READ_STATUS_REG_CMD;
  s_command.InstructionMode       = HAL_OSPI_INSTRUCTION_8_LINES;
  s_command.InstructionSize       = HAL_OSPI_INSTRUCTION_16_BITS;
  s_command.Address               = 0;
  s_command.AddressMode           = HAL_OSPI_ADDRESS_8_LINES;
  s_command.AddressSize           = HAL_OSPI_ADDRESS_32_BITS;
  s_command.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode              = HAL_OSPI_DATA_8_LINES;
  s_command.NbData                = 2;
  s_command.DummyCycles           = LX_STM32_OSPI_DUMMY_CYCLES_READ_OCTAL;
  s_command.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* DTR mode is enabled */
  s_command.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_ENABLE;
  s_command.AddressDtrMode        = HAL_OSPI_ADDRESS_DTR_ENABLE;
  s_command.DataDtrMode           = HAL_OSPI_DATA_DTR_ENABLE;
  s_command.DQSMode               = HAL_OSPI_DQS_ENABLE;

  s_config.Match           = 0;
  s_config.Mask            = LX_STM32_OSPI_SR_WIP;

  do
  {
    /* Wait for 10 config interval between each request */
    HAL_Delay(1); /* s_config.Interval(0x10) / Clock(55 MHz) = 0.29 ms */

    if (HAL_OSPI_Command(&hospi1, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }

    if (HAL_OSPI_Receive(&hospi1, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }
  }while((reg[0] & s_config.Mask ) != s_config.Match);

  /* USER CODE END OSPI_HAL_CFG_AUTO_POLLING_READY */

  return 0;
}

/**
  * @brief  This function enables/disables the octal mode of the memory.
  * @param  hospi: OSPI handle
  * @param  mode: Octal operation mode enable/disable
  * @retval None
  */
static uint8_t ospi_set_octalmode(OSPI_HandleTypeDef *hospi, uint8_t mode)
{
  /* USER CODE BEGIN OSPI_HAL_CFG_OCTAL_MODE */

  OSPI_RegularCmdTypeDef  s_command;
  OSPI_AutoPollingTypeDef s_config;
  uint8_t reg[2];

  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  s_command.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  s_config.MatchMode     = HAL_OSPI_MATCH_MODE_AND;
  s_config.Interval      = 0x10;
  s_config.AutomaticStop = HAL_OSPI_AUTOMATIC_STOP_ENABLE;

  /* Activate/deactivate the Octal mode */
  if (mode)
  {
    /* Enable write operations */
    s_command.Instruction     = WRITE_ENABLE_CMD;
    s_command.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
    s_command.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
    s_command.AddressMode     = HAL_OSPI_ADDRESS_NONE;
    s_command.DataMode        = HAL_OSPI_DATA_NONE;
    s_command.DummyCycles     = 0;

    if (HAL_OSPI_Command(&hospi1, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }

    /* Configure automatic polling mode to wait for write enabling */
    s_config.Match = LX_STM32_OSPI_SR_WEL;
    s_config.Mask  = LX_STM32_OSPI_SR_WEL;

    s_command.Instruction = READ_STATUS_REG_CMD;
    s_command.DataMode    = HAL_OSPI_DATA_1_LINE;
    s_command.NbData      = 1;

    if (HAL_OSPI_Command(&hospi1, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }

    if (HAL_OSPI_AutoPolling(&hospi1, &s_config, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }

    /* Write Configuration register 2 (with new dummy cycles) */
    s_command.Instruction = WRITE_CFG_REG_2_CMD;
    s_command.Address     = LX_STM32_OSPI_CR2_REG3_ADDR;
    s_command.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
    reg[0] = LX_STM32_OSPI_DUMMY_CYCLES_CR_CFG;

    if (HAL_OSPI_Command(&hospi1, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }

    if (HAL_OSPI_Transmit(&hospi1, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }

    /* Enable write operations */
    s_command.Instruction = WRITE_ENABLE_CMD;
    s_command.AddressMode = HAL_OSPI_ADDRESS_NONE;
    s_command.DataMode    = HAL_OSPI_DATA_NONE;

    if (HAL_OSPI_Command(&hospi1, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }

    /* Configure automatic polling mode to wait for write enabling */
    s_command.Instruction = READ_STATUS_REG_CMD;
    s_command.DataMode    = HAL_OSPI_DATA_1_LINE;

    if (HAL_OSPI_Command(&hospi1, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }

    if (HAL_OSPI_AutoPolling(&hospi1, &s_config, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }

    /* Write Configuration register 2 (with Octal I/O SPI protocol) */
    s_command.Instruction = WRITE_CFG_REG_2_CMD;
    s_command.Address     = LX_STM32_QSPI_CR2_REG1_ADDR;
    s_command.AddressMode = HAL_OSPI_ADDRESS_1_LINE;

    /* DTR mode is enabled */
    reg[0] = LX_STM32_OSPI_CR2_DOPI;

    if (HAL_OSPI_Command(&hospi1, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }

    if (HAL_OSPI_Transmit(&hospi1, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }

    /* Wait that the configuration is effective and check that memory is ready */
    HAL_Delay(LX_STM32_OSPI_WRITE_REG_MAX_TIME);

    if (ospi_auto_polling_ready(&hospi1, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != 0)
    {
      return 1;
    }

    /* Check the configuration has been correctly done */
    s_command.Instruction     = OCTAL_READ_CFG_REG_2_CMD;
    s_command.InstructionMode = HAL_OSPI_INSTRUCTION_8_LINES;
    s_command.InstructionSize = HAL_OSPI_INSTRUCTION_16_BITS;
    s_command.AddressMode     = HAL_OSPI_ADDRESS_8_LINES;
    s_command.DataMode        = HAL_OSPI_DATA_8_LINES;
    s_command.DummyCycles     = LX_STM32_OSPI_DUMMY_CYCLES_READ_OCTAL;
    s_command.NbData          = 2;
    reg[0] = 0;

    s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_ENABLE;
    s_command.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_ENABLE;
    s_command.DataDtrMode        = HAL_OSPI_DATA_DTR_ENABLE;
    s_command.DQSMode            = HAL_OSPI_DQS_ENABLE;

    if (HAL_OSPI_Command(&hospi1, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }

    if (HAL_OSPI_Receive(&hospi1, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }

    /* DTR mode is enabled */
    if (reg[0] != LX_STM32_OSPI_CR2_DOPI)
    {
      return 1;
    }
  }
  else
  {
    /* Enable write operations */
    if (ospi_set_write_enable(&hospi1) != 0)
    {
      return 1;
    }

    /* Write Configuration register 2 (with Octal I/O SPI protocol) */
    s_command.Instruction     = OCTAL_WRITE_CFG_REG_2_CMD;
    s_command.InstructionMode = HAL_OSPI_INSTRUCTION_8_LINES;
    s_command.InstructionSize = HAL_OSPI_INSTRUCTION_16_BITS;
    s_command.Address         = LX_STM32_QSPI_CR2_REG1_ADDR;
    s_command.AddressMode     = HAL_OSPI_ADDRESS_8_LINES;
    s_command.DataMode        = HAL_OSPI_DATA_8_LINES;
    s_command.NbData          = 2;
    s_command.DummyCycles     = 0;
    reg[0] = 0;
    reg[1] = 0;

    /* DTR mode is enabled */
    s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_ENABLE;
    s_command.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_ENABLE;
    s_command.DataDtrMode        = HAL_OSPI_DATA_DTR_ENABLE;

    if (HAL_OSPI_Command(&hospi1, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }

    if (HAL_OSPI_Transmit(&hospi1, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }

    /* Wait that the configuration is effective and check that memory is ready */
    HAL_Delay(LX_STM32_OSPI_WRITE_REG_MAX_TIME);

    s_command.Instruction     = READ_STATUS_REG_CMD;
    s_command.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
    s_command.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
    s_command.AddressMode     = HAL_OSPI_ADDRESS_NONE;
    s_command.DataMode        = HAL_OSPI_DATA_1_LINE;
    s_command.NbData          = 1;
    s_command.DummyCycles     = 0;

    /* DTR mode is enabled */
    s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    s_command.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
    s_command.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;

    s_config.Match = 0;
    s_config.Mask  = LX_STM32_OSPI_SR_WIP;

    if (HAL_OSPI_Command(&hospi1, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }

    if (HAL_OSPI_AutoPolling(&hospi1, &s_config, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }

    /* Check the configuration has been correctly done */
    s_command.Instruction = READ_CFG_REG_2_CMD;
    s_command.Address     = LX_STM32_QSPI_CR2_REG1_ADDR;
    s_command.AddressMode = HAL_OSPI_ADDRESS_1_LINE;

    if (HAL_OSPI_Command(&hospi1, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }

    if (HAL_OSPI_Receive(&hospi1, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }

    if (reg[0] != 0)
    {
      return 1;
    }
  }

  /* USER CODE END OSPI_HAL_CFG_OCTAL_MODE */

  return 0;
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
