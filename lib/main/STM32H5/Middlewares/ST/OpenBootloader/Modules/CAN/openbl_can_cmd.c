/**
  ******************************************************************************
  * @file    openbl_can_cmd.c
  * @author  MCD Application Team
  * @brief   Contains CAN protocol commands
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "openbl_mem.h"
#include "openbl_core.h"
#include "openbl_can_cmd.h"
#include "openbootloader_conf.h"
#include "can_interface.h"
#include "interfaces_conf.h"
#include "common_interface.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define OPENBL_CAN_COMMANDS_NB_MAX        12U  /* Number of supported commands */
#define OPENBL_CAN_SPEED_MAX              4U  /* Max speed is 4 (1 Mbps) */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t tCanTxData[CAN_RAM_BUFFER_SIZE];
static uint8_t a_OPENBL_CAN_CommandsList[OPENBL_CAN_COMMANDS_NB_MAX] = {0};
static uint8_t CanCommandsNumber = 0U;

/* Private function prototypes -----------------------------------------------*/
static uint8_t OPENBL_CAN_GetAddress(uint32_t *Address);
static uint8_t OPENBL_CAN_ConstructCommandsTable(OPENBL_CommandsTypeDef *pCanCmd);

/* Exported variables --------------------------------------------------------*/
/* Exported functions---------------------------------------------------------*/

/**
  * @brief  This function is used to get a pointer to the structure that contains the available CAN commands.
  * @return Returns a pointer to the OPENBL_CommandsTypeDef struct.
  */
OPENBL_CommandsTypeDef *OPENBL_CAN_GetCommandsList(void)
{
  static OPENBL_CommandsTypeDef OPENBL_CAN_Commands =
  {
    OPENBL_CAN_GetCommand,
    OPENBL_CAN_GetVersion,
    OPENBL_CAN_GetID,
    OPENBL_CAN_ReadMemory,
    OPENBL_CAN_WriteMemory,
    OPENBL_CAN_Go,
    OPENBL_CAN_ReadoutProtect,
    OPENBL_CAN_ReadoutUnprotect,
    OPENBL_CAN_LegacyEraseMemory,
    OPENBL_CAN_WriteProtect,
    OPENBL_CAN_WriteUnprotect,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    OPENBL_CAN_Speed,
    NULL,
    NULL
  };

  OPENBL_CAN_SetCommandsList(&OPENBL_CAN_Commands);

  return (&OPENBL_CAN_Commands);
}

/**
  * @brief  This function is used to set the list of CAN supported commands.
  * @return Returns a pointer to the OPENBL_CAN_Commands struct.
  */
void OPENBL_CAN_SetCommandsList(OPENBL_CommandsTypeDef *pCanCmd)
{
  /* Get the list of commands supported & their numbers */
  CanCommandsNumber = OPENBL_CAN_ConstructCommandsTable(pCanCmd);
}

/**
  * @brief  This function is used to get the list of the available CAN commands
  * @retval None.
  */
void OPENBL_CAN_GetCommand(void)
{
  uint32_t counter;

  /* Send Acknowledge byte to notify the host that the command is recognized */
  OPENBL_CAN_SendByte(ACK_BYTE);

  /* Send the number of commands supported by CAN protocol */
  OPENBL_CAN_SendByte(CanCommandsNumber);

  /* Send CAN protocol version */
  OPENBL_CAN_SendByte(OPENBL_CAN_VERSION);

  /* Send the list of supported commands */
  for (counter = 0U; counter < CanCommandsNumber; counter++)
  {
    OPENBL_CAN_SendByte(a_OPENBL_CAN_CommandsList[counter]);
  }

  /* Send last Acknowledge synchronization byte */
  OPENBL_CAN_SendByte(ACK_BYTE);
}

/**
  * @brief  This function is used to get the CAN protocol version.
  * @retval None.
  */
void OPENBL_CAN_GetVersion(void)
{
  /* Send Acknowledge byte to notify the host that the command is recognized */
  OPENBL_CAN_SendByte(ACK_BYTE);

  /* Send CAN protocol version */
  OPENBL_CAN_SendByte(OPENBL_CAN_VERSION);

  /* Send dummy bytes */
  tCanTxData[0] = 0x00;
  tCanTxData[1] = 0x00;
  OPENBL_CAN_SendBytes(tCanTxData, CAN_DLC_BYTES_2);

  /* Send last Acknowledge synchronization byte */
  OPENBL_CAN_SendByte(ACK_BYTE);
}

/**
  * @brief  This function is used to get the device ID.
  * @retval None.
  */
void OPENBL_CAN_GetID(void)
{
  /* Send Acknowledge byte to notify the host that the command is recognized */
  OPENBL_CAN_SendByte(ACK_BYTE);

  /* Send the device ID starting by the MSB byte then the LSB byte */
  tCanTxData[0] = DEVICE_ID_MSB;
  tCanTxData[1] = DEVICE_ID_LSB;
  OPENBL_CAN_SendBytes(tCanTxData, CAN_DLC_BYTES_2);

  /* Send last Acknowledge synchronization byte */
  OPENBL_CAN_SendByte(ACK_BYTE);
}

/**
  * @brief  This function is used to set a new baud-rate.
  * @retval None.
  */
void OPENBL_CAN_Speed(void)
{
  uint8_t data;

  /*       CAN speed
    parameter   | speed        |
    ---------------------------|
    1           | 125 kbps     |
    2           | 250 kbps     |
    3           | 500 kbps     |
    4           | 1 Mbps       |
  */
  /* Read the new CAN speed */
  data = (uint8_t)tCanRxData[0];

  if (data <= OPENBL_CAN_SPEED_MAX)
  {
    /* Send Acknowledge byte to notify the host that the command is recognized and the data is valid */
    OPENBL_CAN_SendByte(ACK_BYTE);

    /* De-initialize the used CAN instance */
    OPENBL_CAN_DeInit();

    /* Change the pre-scaler value according to new CAN speed */
    OPENBL_CAN_ChangePrescaler(data);

    /* Initialize the used CAN instance */
    OPENBL_CAN_Configuration();
    HAL_Delay(303);

    /* Send the last Acknowledge byte */
    OPENBL_CAN_SendByte(ACK_BYTE);
  }
  else
  {
    /* wrong baud-rate */
    OPENBL_CAN_SendByte(NACK_BYTE);
  }
}

/**
  * @brief  This function is used to read memory from the device.
  * @retval None.
  */
void OPENBL_CAN_ReadMemory(void)
{
  uint32_t address;
  uint32_t memory_index;
  uint16_t number_of_bytes;
  uint16_t count;
  uint16_t single;
  uint8_t counter;
  uint8_t data_length;
  uint8_t data_byte;

  /* Check memory protection then send adequate response */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_CAN_SendByte(NACK_BYTE);
  }
  else
  {
    if (OPENBL_CAN_GetAddress(&address) == NACK_BYTE)
    {
      OPENBL_CAN_SendByte(NACK_BYTE);
    }
    else
    {
      OPENBL_CAN_SendByte(ACK_BYTE);

      /* Get the number of bytes to be read from memory (Max: data + 1 = 256) */
      number_of_bytes = (uint16_t)((uint16_t)tCanRxData[4] + 1U);

      count  = number_of_bytes / 8U;
      single = (number_of_bytes % 8U);

      /* Get the memory index to know from which memory we will read */
      memory_index = OPENBL_MEM_GetMemoryIndex(address);

      while (count != 0U)
      {
        data_length = 0;

        for (counter = 8U ; counter > 0U; counter--)
        {
          tCanTxData[data_length] = OPENBL_MEM_Read(address, memory_index);

          data_length++;
          address++;
        }

        OPENBL_CAN_SendBytes(tCanTxData, CAN_DLC_BYTES_8);

        count--;
      }

      if (single != 0U)
      {
        for (counter = (uint8_t)single ; counter > 0U; counter--)
        {
          data_byte = OPENBL_MEM_Read(address, memory_index);
          address++;
          OPENBL_CAN_SendByte(data_byte);
        }
      }

      /* Send last Acknowledge synchronization byte */
      OPENBL_CAN_SendByte(ACK_BYTE);
    }
  }
}

/**
  * @brief  This function is used to write in to device memory.
  * @retval None.
  */
void OPENBL_CAN_WriteMemory(void)
{
  uint32_t address;
  uint32_t code_size;
  uint32_t count;
  uint32_t single;
  uint8_t data_length;

  /* Check memory protection then send adequate response */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_CAN_SendByte(NACK_BYTE);
  }
  else
  {
    if (OPENBL_CAN_GetAddress(&address) == NACK_BYTE)
    {
      OPENBL_CAN_SendByte(NACK_BYTE);
    }
    else
    {
      OPENBL_CAN_SendByte(ACK_BYTE);

      /* Get the number of bytes to be written to memory (Max: data + 1 = 256) */
      code_size = (uint32_t)tCanRxData[4] + 1U;

      count  = code_size / 8U;
      single = code_size % 8U;

      data_length = 0;

      if (count != 0U)
      {
        while (data_length != count)
        {
          OPENBL_CAN_ReadBytes(&tCanRxData[data_length * 8U], CAN_DLC_BYTES_8);
          OPENBL_CAN_SendByte(ACK_BYTE);

          data_length++;
        }
      }

      if (single != 0U)
      {
        OPENBL_CAN_ReadBytes(&tCanRxData[(code_size - single)], CAN_DLC_BYTES_8);
        OPENBL_CAN_SendByte(ACK_BYTE);
      }

      /* Write data to memory */
      OPENBL_MEM_Write(address, (uint8_t *)tCanRxData, code_size);

      /* Send last Acknowledge synchronization byte */
      OPENBL_CAN_SendByte(ACK_BYTE);

      /* Start post processing task if needed */
      Common_StartPostProcessing();
    }
  }
}

/**
  * @brief  This function is used to jump to the user application.
  * @retval None.
  */
void OPENBL_CAN_Go(void)
{
  uint32_t address;
  uint32_t mem_area;

  /* Check memory protection then send adequate response */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_CAN_SendByte(NACK_BYTE);
  }
  else
  {
    if (OPENBL_CAN_GetAddress(&address) == NACK_BYTE)
    {
      OPENBL_CAN_SendByte(NACK_BYTE);
    }
    else
    {
      /* Check if received address is valid or not */
      mem_area = OPENBL_MEM_GetAddressArea(address);

      if ((mem_area != FLASH_AREA) && (mem_area != RAM_AREA))
      {
        OPENBL_CAN_SendByte(NACK_BYTE);
      }
      else
      {
        /* If the jump address is valid then send ACK */
        OPENBL_CAN_SendByte(ACK_BYTE);

        OPENBL_MEM_JumpToAddress(address);
      }
    }
  }
}

/**
  * @brief  This function is used to enable readout protection.
  * @retval None.
  */
void OPENBL_CAN_ReadoutProtect(void)
{
  /* Check memory protection then send adequate response */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_CAN_SendByte(NACK_BYTE);
  }
  else
  {
    OPENBL_CAN_SendByte(ACK_BYTE);

    /* Enable the read protection */
    OPENBL_MEM_SetReadOutProtection(OPENBL_DEFAULT_MEM, ENABLE);

    /* Send last Acknowledge synchronization byte */
    OPENBL_CAN_SendByte(ACK_BYTE);

    /* Start post processing task if needed */
    Common_StartPostProcessing();
  }
}

/**
  * @brief  This function is used to disable readout protection.
  * @retval None.
  */
void OPENBL_CAN_ReadoutUnprotect(void)
{
  OPENBL_CAN_SendByte(ACK_BYTE);

  OPENBL_CAN_SendByte(ACK_BYTE);

  /* Disable the read protection */
  OPENBL_MEM_SetReadOutProtection(OPENBL_DEFAULT_MEM, DISABLE);

  /* Start post processing task if needed */
  Common_StartPostProcessing();
}

/**
  * @brief  This function is used to erase a memory.
  * @retval None.
  */
void OPENBL_CAN_LegacyEraseMemory(void)
{
  ErrorStatus error_value;
  uint8_t a_erase_buffer[CAN_RAM_BUFFER_SIZE * 2];
  uint8_t nsectors;
  uint8_t count;
  uint8_t single;
  uint8_t status = ACK_BYTE;
  uint8_t data_length;
  uint8_t index;

  /* Check if the memory is protected or not */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_CAN_SendByte(NACK_BYTE);
  }
  else
  {
    OPENBL_CAN_SendByte(ACK_BYTE);

    /* Read number of pages to be erased */
    nsectors = (uint8_t)tCanRxData[0] + 1U;

    a_erase_buffer[0] = nsectors;
    a_erase_buffer[1] = 0x00;

    /* All commands in range 0xFF are reserved for special erase features */
    if (((nsectors - 1U) == 0xFFU))
    {
      error_value = OPENBL_MEM_MassErase(OPENBL_DEFAULT_MEM, tCanRxData, CAN_RAM_BUFFER_SIZE);

      if (error_value == SUCCESS)
      {
        status = ACK_BYTE;
        OPENBL_CAN_SendByte(ACK_BYTE);
      }
      else
      {
        status = NACK_BYTE;
      }
    }
    else
    {
      OPENBL_CAN_SendByte(ACK_BYTE);

      count       = nsectors / 8U;
      single      = nsectors % 8U;
      data_length = 0;

      if (count != 0U)
      {
        while (data_length != count)
        {
          OPENBL_CAN_ReadBytes(&tCanRxData[(data_length * 8U) + 1U], CAN_DLC_BYTES_8);
          OPENBL_CAN_SendByte(ACK_BYTE);

          data_length++;
        }
      }

      if (single != 0U)
      {
        OPENBL_CAN_ReadBytes(&tCanRxData[(nsectors - single) + 1U], CAN_DLC_BYTES_8);
      }

      index = 2;

      while (index <= ((nsectors * 2U) + 1U))
      {
        a_erase_buffer[index]      = tCanRxData[index / 2U];
        a_erase_buffer[index + 1U] = 0x00;
        index                      += 2U;
      }

      /* Receive the list of pages to be erased (each page number is on one byte) */
      error_value = OPENBL_MEM_Erase(OPENBL_DEFAULT_MEM, a_erase_buffer, CAN_RAM_BUFFER_SIZE * 2U);

      /* Errors from memory erase are not managed, always return ACK */
      if (error_value == SUCCESS)
      {
        status = ACK_BYTE;
      }
    }

    /* Send status byte (ACK/NACK) */
    OPENBL_CAN_SendByte(status);
  }
}

/**
  * @brief  This function is used to enable write protect.
  * @retval None.
  */
void OPENBL_CAN_WriteProtect(void)
{
  ErrorStatus error_value;
  uint8_t length;
  uint8_t count;
  uint8_t single;
  uint8_t data_length;

  /* Check if the memory is protected or not */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_CAN_SendByte(NACK_BYTE);
  }
  else
  {
    OPENBL_CAN_SendByte(ACK_BYTE);

    length = tCanRxData[0] + 1U;
    count  = length / 8U;
    single = length % 8U;

    data_length = 0;

    if (count != 0U)
    {
      while (data_length != count)
      {
        OPENBL_CAN_ReadBytes(&tCanRxData[(data_length * 8U) + 1U], CAN_DLC_BYTES_8);
        OPENBL_CAN_SendByte(ACK_BYTE);

        data_length++;
      }
    }

    if (single != 0U)
    {
      OPENBL_CAN_ReadBytes(&tCanRxData[1U + (length - single)], CAN_DLC_BYTES_8);
      OPENBL_CAN_SendByte(ACK_BYTE);
    }

    /* Enable the write protection */
    error_value = OPENBL_MEM_SetWriteProtection(ENABLE, OPENBL_DEFAULT_MEM, (uint8_t *) &tCanRxData[1], length);

    OPENBL_CAN_SendByte(ACK_BYTE);

    if (error_value == SUCCESS)
    {
      Common_StartPostProcessing();
    }
  }
}

/**
  * @brief  This function is used to disable write protect.
  * @retval None.
  */
void OPENBL_CAN_WriteUnprotect(void)
{
  ErrorStatus error_value;

  /* Check if the memory is not protected */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_CAN_SendByte(NACK_BYTE);
  }
  else
  {
    OPENBL_CAN_SendByte(ACK_BYTE);

    /* Disable write protection */
    error_value = OPENBL_MEM_SetWriteProtection(DISABLE, OPENBL_DEFAULT_MEM, NULL, 0);

    OPENBL_CAN_SendByte(ACK_BYTE);

    if (error_value == SUCCESS)
    {
      Common_StartPostProcessing();
    }
  }
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  This function is used to construct the command List table.
  * @return Returns the number of supported commands.
  */
static uint8_t OPENBL_CAN_ConstructCommandsTable(OPENBL_CommandsTypeDef *pCanCmd)
{
  uint8_t i = 0;

  if (pCanCmd->GetCommand != NULL)
  {
    a_OPENBL_CAN_CommandsList[i] = CMD_GET_COMMAND;
    i++;
  }

  if (pCanCmd->GetVersion != NULL)
  {
    a_OPENBL_CAN_CommandsList[i] = CMD_GET_VERSION;
    i++;
  }

  if (pCanCmd->GetID != NULL)
  {
    a_OPENBL_CAN_CommandsList[i] = CMD_GET_ID;
    i++;
  }

  if (pCanCmd->Speed != NULL)
  {
    a_OPENBL_CAN_CommandsList[i] = CMD_SPEED;
    i++;
  }

  if (pCanCmd->ReadMemory != NULL)
  {
    a_OPENBL_CAN_CommandsList[i] = CMD_READ_MEMORY;
    i++;
  }

  if (pCanCmd->Go != NULL)
  {
    a_OPENBL_CAN_CommandsList[i] = CMD_GO;
    i++;
  }

  if (pCanCmd->WriteMemory != NULL)
  {
    a_OPENBL_CAN_CommandsList[i] = CMD_WRITE_MEMORY;
    i++;
  }

  if (pCanCmd->EraseMemory != NULL)
  {
    a_OPENBL_CAN_CommandsList[i] = CMD_LEG_ERASE_MEMORY;
    i++;
  }

  if (pCanCmd->WriteProtect != NULL)
  {
    a_OPENBL_CAN_CommandsList[i] = CMD_WRITE_PROTECT;
    i++;
  }

  if (pCanCmd->WriteUnprotect != NULL)
  {
    a_OPENBL_CAN_CommandsList[i] = CMD_WRITE_UNPROTECT;
    i++;
  }

  if (pCanCmd->ReadoutProtect != NULL)
  {
    a_OPENBL_CAN_CommandsList[i] = CMD_READ_PROTECT;
    i++;
  }

  if (pCanCmd->ReadoutUnprotect != NULL)
  {
    a_OPENBL_CAN_CommandsList[i] = CMD_READ_UNPROTECT;
    i++;
  }

  return (i);
}

/**
  * @brief  This function is used to get a valid address.
  * @retval Returns NACK status in case of error else returns ACK status.
  */
static uint8_t OPENBL_CAN_GetAddress(uint32_t *Address)
{
  uint8_t status;

  *Address = (((((uint32_t)tCanRxData[0]) << 24) |
               (((uint32_t)tCanRxData[1]) << 16) |
               (((uint32_t)tCanRxData[2]) << 8)  |
               (((uint32_t)tCanRxData[3]))));

  /* Check if received address is valid or not */
  if (OPENBL_MEM_GetAddressArea(*Address) == AREA_ERROR)
  {
    status = NACK_BYTE;
  }
  else
  {
    status = ACK_BYTE;
  }

  return status;
}
