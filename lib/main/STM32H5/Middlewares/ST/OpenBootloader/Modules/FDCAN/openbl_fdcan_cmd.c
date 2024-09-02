/**
  ******************************************************************************
  * @file    openbl_fdcan_cmd.c
  * @author  MCD Application Team
  * @brief   Contains FDCAN protocol commands
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019-2021 STMicroelectronics.
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
#include "openbl_fdcan_cmd.h"

#include "openbootloader_conf.h"
#include "app_openbootloader.h"
#include "fdcan_interface.h"
#include "common_interface.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define OPENBL_FDCAN_COMMANDS_NB_MAX      13U       /* The maximum number of supported commands */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t a_OPENBL_FDCAN_CommandsList[OPENBL_FDCAN_COMMANDS_NB_MAX] = {0U};
static uint8_t FdcanCommandsNumber = 0U;

/* Private function prototypes -----------------------------------------------*/
static uint8_t OPENBL_FDCAN_GetAddress(uint32_t *Address);
static uint8_t OPENBL_FDCAN_GetSpecialCmdOpCode(uint16_t *OpCode, OPENBL_SpecialCmdTypeTypeDef CmdType);
static uint8_t OPENBL_FDCAN_ConstructCommandsTable(OPENBL_CommandsTypeDef *pFdcanCmd);

/* Exported variables --------------------------------------------------------*/
/* Exported functions---------------------------------------------------------*/
/**
  * @brief  This function is used to get a pointer to the structure that contains the available FDCAN commands.
  * @retval Returns a pointer to the OPENBL_CommandsTypeDef struct.
  */
OPENBL_CommandsTypeDef *OPENBL_FDCAN_GetCommandsList(void)
{
  static OPENBL_CommandsTypeDef OPENBL_FDCAN_Commands =
  {
    OPENBL_FDCAN_GetCommand,
    OPENBL_FDCAN_GetVersion,
    OPENBL_FDCAN_GetID,
    OPENBL_FDCAN_ReadMemory,
    OPENBL_FDCAN_WriteMemory,
    OPENBL_FDCAN_Go,
    OPENBL_FDCAN_ReadoutProtect,
    OPENBL_FDCAN_ReadoutUnprotect,
    OPENBL_FDCAN_EraseMemory,
    OPENBL_FDCAN_WriteProtect,
    OPENBL_FDCAN_WriteUnprotect,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    OPENBL_FDCAN_SpecialCommand,
    OPENBL_FDCAN_ExtendedSpecialCommand
  };

  OPENBL_FDCAN_SetCommandsList(&OPENBL_FDCAN_Commands);

  return (&OPENBL_FDCAN_Commands);
}

/**
  * @brief  This function is used to set the list of FDCAN supported commands.
  * @return Returns a pointer to the OPENBL_FDCAN_Commands struct.
  */
void OPENBL_FDCAN_SetCommandsList(OPENBL_CommandsTypeDef *pFdcanCmd)
{
  /* Get the list of commands supported & their numbers */
  FdcanCommandsNumber = OPENBL_FDCAN_ConstructCommandsTable(pFdcanCmd);
}

/**
  * @brief  This function is used to get the list of the available FDCAN commands
  * @retval None.
  */
void OPENBL_FDCAN_GetCommand(void)
{
  uint32_t counter;

  /* Send Acknowledge byte to notify the host that the command is recognized */
  OPENBL_FDCAN_SendByte(ACK_BYTE);

  /* Send the number of commands supported by FDCAN protocol */
  OPENBL_FDCAN_SendByte(FdcanCommandsNumber);

  /* Send FDCAN protocol version */
  OPENBL_FDCAN_SendByte(OPENBL_FDCAN_VERSION);

  /* Send the list of supported commands */
  for (counter = 0U; counter < FdcanCommandsNumber; counter++)
  {
    OPENBL_FDCAN_SendByte(a_OPENBL_FDCAN_CommandsList[counter]);
  }

  /* Send last Acknowledge synchronization byte */
  OPENBL_FDCAN_SendByte(ACK_BYTE);
}

/**
  * @brief  This function is used to get the FDCAN protocol version.
  * @retval None.
  */
void OPENBL_FDCAN_GetVersion(void)
{
  /* Send Acknowledge byte to notify the host that the command is recognized */
  OPENBL_FDCAN_SendByte(ACK_BYTE);

  /* Send FDCAN protocol version */
  OPENBL_FDCAN_SendByte(OPENBL_FDCAN_VERSION);

  /* Send dummy bytes */
  TxData[0] = 0x0U;
  TxData[1] = 0x0U;
  OPENBL_FDCAN_SendBytes(TxData, FDCAN_DLC_BYTES_2);

  /* Send last Acknowledge synchronization byte */
  OPENBL_FDCAN_SendByte(ACK_BYTE);
}

/**
  * @brief  This function is used to get the device ID.
  * @retval None.
  */
void OPENBL_FDCAN_GetID(void)
{
  /* Send Acknowledge byte to notify the host that the command is recognized */
  OPENBL_FDCAN_SendByte(ACK_BYTE);

  /* Send the device ID starting by the MSB byte then the LSB byte */
  TxData[0] = DEVICE_ID_MSB;
  TxData[1] = DEVICE_ID_LSB;
  OPENBL_FDCAN_SendBytes(TxData, FDCAN_DLC_BYTES_2);

  /* Send last Acknowledge synchronization byte */
  OPENBL_FDCAN_SendByte(ACK_BYTE);
}

/**
  * @brief  This function is used to read memory from the device.
  * @retval None.
  */
void OPENBL_FDCAN_ReadMemory(void)
{
  uint32_t address;
  uint32_t counter;
  uint32_t number_of_bytes;
  uint32_t count;
  uint32_t single;
  uint32_t memory_index;
  uint8_t  data_length;

  /* Check memory protection then send adequate response */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_FDCAN_SendByte(NACK_BYTE);
  }
  else
  {
    if (OPENBL_FDCAN_GetAddress(&address) == NACK_BYTE)
    {
      OPENBL_FDCAN_SendByte(NACK_BYTE);
    }
    else
    {
      OPENBL_FDCAN_SendByte(ACK_BYTE);

      /* Get the number of bytes to be read from memory (Max: data + 1 = 256) */
      number_of_bytes = (uint32_t)RxData[4] + 1U;

      count  = number_of_bytes / 64U;
      single = (uint32_t)(number_of_bytes % 64U);

      /* Get the memory index to know from which memory we will read */
      memory_index = OPENBL_MEM_GetMemoryIndex(address);

      while (count != 0U)
      {
        data_length = 0U;

        for (counter = 64U ; counter > 0U; counter--)
        {
          TxData[data_length] = OPENBL_MEM_Read(address, memory_index);

          data_length++;
          address++;
        }

        OPENBL_FDCAN_SendBytes(TxData, FDCAN_DLC_BYTES_64);

        count--;
      }

      if (single != 0U)
      {
        data_length = 0U;

        for (counter = single ; counter > 0U; counter--)
        {
          TxData[data_length] = OPENBL_MEM_Read(address, memory_index);

          data_length++;
          address++;
        }

        /* Fill the rest of the buffer with 0xFF */
        for (counter = (64U - single) ; counter > 0U; counter--)
        {
          TxData[data_length] = 0xFFU;

          data_length++;
        }

        OPENBL_FDCAN_SendBytes(TxData, FDCAN_DLC_BYTES_64);
      }

      OPENBL_FDCAN_SendByte(ACK_BYTE);
    }
  }
}

/**
  * @brief  This function is used to write in to device memory.
  * @retval None.
  */
void OPENBL_FDCAN_WriteMemory(void)
{
  uint32_t address;
  uint32_t CodeSize;
  uint32_t count;
  uint32_t single;
  uint8_t data_length;

  /* Check memory protection then send adequate response */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_FDCAN_SendByte(NACK_BYTE);
  }
  else
  {
    if (OPENBL_FDCAN_GetAddress(&address) == NACK_BYTE)
    {
      OPENBL_FDCAN_SendByte(NACK_BYTE);
    }
    else
    {
      OPENBL_FDCAN_SendByte(ACK_BYTE);

      /* Get the number of bytes to be written to memory (Max: data + 1 = 256) */
      CodeSize = (uint32_t)RxData[4] + 1U;

      count = CodeSize / 64U;
      single = (uint32_t)(CodeSize % 64U);

      data_length = 0U;

      if (count != 0U)
      {
        while (data_length != count)
        {
          OPENBL_FDCAN_ReadBytes(&RxData[data_length * 64U], 64U);

          data_length++;
        }
      }

      if (single != 0U)
      {
        OPENBL_FDCAN_ReadBytes(&RxData[(CodeSize - single)], 64U);
      }

      /* Write data to memory */
      OPENBL_MEM_Write(address, (uint8_t *)RxData, CodeSize);

      /* Send last Acknowledge synchronization byte */
      OPENBL_FDCAN_SendByte(ACK_BYTE);

      /* Start post processing task if needed */
      Common_StartPostProcessing();
    }
  }
}

/**
  * @brief  This function is used to jump to the user application.
  * @retval None.
  */
void OPENBL_FDCAN_Go(void)
{
  uint32_t address;
  uint8_t status;

  /* Check memory protection then send adequate response */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_FDCAN_SendByte(NACK_BYTE);
  }
  else
  {
    OPENBL_FDCAN_SendByte(ACK_BYTE);

    /* Get memory address */
    if (OPENBL_FDCAN_GetAddress(&address) == NACK_BYTE)
    {
      OPENBL_FDCAN_SendByte(NACK_BYTE);
    }
    else
    {
      /* Check if received address is valid or not */
      status = OPENBL_MEM_CheckJumpAddress(address);

      if (status == 0U)
      {
        OPENBL_FDCAN_SendByte(NACK_BYTE);
      }
      else
      {
        /* If the jump address is valid then send ACK */
        OPENBL_FDCAN_SendByte(ACK_BYTE);

        OPENBL_MEM_JumpToAddress(address);
      }
    }
  }
}

/**
  * @brief  This function is used to enable readout protection.
  * @retval None.
  */
void OPENBL_FDCAN_ReadoutProtect(void)
{
  /* Check memory protection then send adequate response */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_FDCAN_SendByte(NACK_BYTE);
  }
  else
  {
    OPENBL_FDCAN_SendByte(ACK_BYTE);

    /* Enable the read protection */
    OPENBL_MEM_SetReadOutProtection(OPENBL_DEFAULT_MEM, ENABLE);

    OPENBL_FDCAN_SendByte(ACK_BYTE);

    /* Start post processing task if needed */
    Common_StartPostProcessing();
  }
}

/**
  * @brief  This function is used to disable readout protection.
  * @retval None.
  */
void OPENBL_FDCAN_ReadoutUnprotect(void)
{
  OPENBL_FDCAN_SendByte(ACK_BYTE);

  /* Once the option bytes modification start bit is set in FLASH CR register,
     all the RAM is erased, this causes the erase of the Open Bootloader RAM.
     This is why the last ACK is sent before the call of OPENBL_MEM_SetReadOutProtection */
  OPENBL_FDCAN_SendByte(ACK_BYTE);

  /* Disable the read protection */
  OPENBL_MEM_SetReadOutProtection(OPENBL_DEFAULT_MEM, DISABLE);

  /* Start post processing task if needed */
  Common_StartPostProcessing();
}

/**
  * @brief  This function is used to erase a memory.
  * @retval None.
  */
void OPENBL_FDCAN_EraseMemory(void)
{
  uint16_t data;
  uint16_t counter;
  uint16_t i;
  uint8_t tempdata;
  uint8_t status = ACK_BYTE;
  ErrorStatus error_value;

  /* Check if the memory is protected or not */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_FDCAN_SendByte(NACK_BYTE);
  }
  else
  {
    OPENBL_FDCAN_SendByte(ACK_BYTE);

    /* Read number of pages to be erased:
     * RxData[0] contains the MSB byte
     * RxData[1] contains the LSB byte
     */
    data = (uint16_t)RxData[0] << 8;
    data = data | RxData[1];

    /* Rewrite the data in right order in the RxData buffer
     * RxData[0] = LSB
     * RxData[1] = MSB
     */
    RxData[0] = (uint8_t)(data & 0x00FFU);
    RxData[1] = (uint8_t)((data & 0xFF00U) >> 8);

    /* All commands in range 0xFFFZ are reserved for special erase features */
    if ((data & 0xFFF0U) == 0xFFF0U)
    {
      if ((data == 0xFFFFU) || (data == 0xFFFEU) || (data == 0xFFFDU))
      {
        error_value = OPENBL_MEM_MassErase(OPENBL_DEFAULT_MEM, RxData, FDCAN_RAM_BUFFER_SIZE);

        if (error_value == SUCCESS)
        {
          status = ACK_BYTE;
        }
        else
        {
          status = NACK_BYTE;
        }
      }
      else
      {
        /* This sub-command is not supported */
        status = NACK_BYTE;
      }
    }
    else
    {
      OPENBL_FDCAN_SendByte(ACK_BYTE);

      /* Receive the list of pages to be erased (each page num is on two bytes)
       * The order of data received is LSB first
       */
      OPENBL_FDCAN_ReadBytes(&RxData[2], 64U);

      i = 2;

      for (counter = data; counter != (uint16_t)0; counter--)
      {
        /* Receive the MSB byte */
        tempdata = RxData[i];
        i++;
        RxData[i - (uint16_t)1] = RxData[i];
        RxData[i] = tempdata;
        i++;
      }

      error_value = OPENBL_MEM_Erase(OPENBL_DEFAULT_MEM, RxData, FDCAN_RAM_BUFFER_SIZE);

      /* Errors from memory erase are not managed, always return ACK */
      if (error_value == SUCCESS)
      {
        status = ACK_BYTE;
      }
    }

    OPENBL_FDCAN_SendByte(status);
  }
}

/**
  * @brief  This function is used to enable write protect.
  * @retval None.
  */
void OPENBL_FDCAN_WriteProtect(void)
{
  uint32_t length;
  ErrorStatus error_value;

  /* Check if the memory is protected or not */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_FDCAN_SendByte(NACK_BYTE);
  }
  else
  {
    OPENBL_FDCAN_SendByte(ACK_BYTE);

    length = RxData[0];

    /* Enable the write protection */
    error_value = OPENBL_MEM_SetWriteProtection(ENABLE, OPENBL_DEFAULT_MEM, (uint8_t *) &RxData[1], length);

    OPENBL_FDCAN_SendByte(ACK_BYTE);

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
void OPENBL_FDCAN_WriteUnprotect(void)
{
  ErrorStatus error_value;

  /* Check if the memory is not protected */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_FDCAN_SendByte(NACK_BYTE);
  }
  else
  {
    OPENBL_FDCAN_SendByte(ACK_BYTE);

    /* Disable write protection */
    error_value = OPENBL_MEM_SetWriteProtection(DISABLE, OPENBL_DEFAULT_MEM, NULL, 0U);

    OPENBL_FDCAN_SendByte(ACK_BYTE);

    if (error_value == SUCCESS)
    {
      Common_StartPostProcessing();
    }
  }
}

/**
  * @brief  This function is used to get a valid address.
  * @retval Returns NACK status in case of error else returns ACK status.
  */
uint8_t OPENBL_FDCAN_GetAddress(uint32_t *Address)
{
  uint8_t status;

  *Address = (((((uint32_t) RxData[0]) << 24)  |
               (((uint32_t) RxData[1]) << 16)  |
               (((uint32_t) RxData[2]) << 8)   |
               (((uint32_t) RxData[3]))));

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

/**
  * @brief  This function is used to execute special command commands.
  * @retval None.
  */
void OPENBL_FDCAN_SpecialCommand(void)
{
  OPENBL_SpecialCmdTypeDef *special_cmd;
  uint16_t op_code;
  uint8_t index;
  uint8_t data;

  /* Point to the RAM USART buffer to gain size and reliability */
  special_cmd = (OPENBL_SpecialCmdTypeDef *)(uint32_t) TxData;

  /* Send special command code acknowledgment */
  OPENBL_FDCAN_SendByte(ACK_BYTE);

  /* Get the command operation code */
  if (OPENBL_FDCAN_GetSpecialCmdOpCode(&op_code, OPENBL_SPECIAL_CMD) == NACK_BYTE)
  {
    OPENBL_FDCAN_SendByte(NACK_BYTE);
  }
  else
  {
    /* Send Operation code acknowledgment */
    OPENBL_FDCAN_SendByte(ACK_BYTE);

    /* Initialize the special command frame */
    special_cmd->CmdType = OPENBL_SPECIAL_CMD;
    special_cmd->OpCode  = op_code;

    /* Get the number of bytes to be received */
    OPENBL_FDCAN_ReadBytes(RxData, 2U);

    /* Read the MSB of the size byte */
    data                     = (*(uint8_t *)(RxData));
    special_cmd->SizeBuffer1 = ((uint16_t)data) << 8;

    /* Read the LSB of the size byte */
    data                      = (*(uint8_t *)(RxData + 1));
    special_cmd->SizeBuffer1 |= (uint16_t)data;

    if (special_cmd->SizeBuffer1 > SPECIAL_CMD_SIZE_BUFFER1)
    {
      OPENBL_FDCAN_SendByte(NACK_BYTE);
    }
    else
    {
      if (special_cmd->SizeBuffer1 != 0U)
      {
        if (special_cmd->SizeBuffer1 > 64U)
        {
          OPENBL_FDCAN_ReadBytes(RxData, 64U);
          OPENBL_FDCAN_ReadBytes(&RxData[64U], ((uint32_t)special_cmd->SizeBuffer1) - 64U);
        }
        else
        {
          OPENBL_FDCAN_ReadBytes(RxData, ((uint32_t) special_cmd->SizeBuffer1));
        }

        /* Read received bytes */
        for (index = 0U; index < special_cmd->SizeBuffer1; index++)
        {
          special_cmd->Buffer1[index] = RxData[index];
        }
      }

      /* Send received size acknowledgment */
      OPENBL_FDCAN_SendByte(ACK_BYTE);

      /* Process the special command */
      OPENBL_FDCAN_SpecialCommandProcess(special_cmd);

      /* NOTE: In case of any operation inside "SpecialCommandProcess" function that prevents the code
       * from returning to here (reset operation...), to be compatible with the OpenBL protocol,
       * the user must ensure sending the last ACK in the application side.
       */

      /* Send last acknowledgment */
      OPENBL_FDCAN_SendByte(ACK_BYTE);
    }
  }
}

/**
  * @brief  This function is used to execute extended special command commands.
  * @retval None.
  */
void OPENBL_FDCAN_ExtendedSpecialCommand(void)
{
  OPENBL_SpecialCmdTypeDef *special_cmd;
  uint16_t op_code;
  uint16_t index;
  uint16_t count;
  uint16_t single;
  uint8_t data;
  uint8_t data_length;


  /* Point to the RAM USART buffer to gain size and reliability */
  special_cmd = (OPENBL_SpecialCmdTypeDef *)(uint32_t) TxData;

  /* Send extended special command code acknowledgment */
  OPENBL_FDCAN_SendByte(ACK_BYTE);

  /* Get the command operation code */
  if (OPENBL_FDCAN_GetSpecialCmdOpCode(&op_code, OPENBL_EXTENDED_SPECIAL_CMD) == NACK_BYTE)
  {
    OPENBL_FDCAN_SendByte(NACK_BYTE);
  }
  else
  {
    /* Send Operation code acknowledgment */
    OPENBL_FDCAN_SendByte(ACK_BYTE);

    /* Initialize the special command frame */
    special_cmd->CmdType = OPENBL_EXTENDED_SPECIAL_CMD;
    special_cmd->OpCode  = op_code;

    /* Initialize the data length variable */
    data_length = 0U;

    /* Get the number of bytes to be received */
    OPENBL_FDCAN_ReadBytes(RxData, 2U);

    /* Read the MSB of the size byte */
    data                    = (*(uint8_t *)(RxData));
    special_cmd->SizeBuffer1 = ((uint16_t)data) << 8;

    /* Read the LSB of the size byte */
    data                     = (*(uint8_t *)(RxData + 1));
    special_cmd->SizeBuffer1 |= (uint16_t)data;

    if (special_cmd->SizeBuffer1 > SPECIAL_CMD_SIZE_BUFFER1)
    {
      OPENBL_FDCAN_SendByte(NACK_BYTE);
    }
    else
    {
      if (special_cmd->SizeBuffer1 != 0U)
      {
        if (special_cmd->SizeBuffer1 > 64U)
        {
          OPENBL_FDCAN_ReadBytes(RxData, 64U);
          OPENBL_FDCAN_ReadBytes(&RxData[64U], ((uint32_t)special_cmd->SizeBuffer1) - 64U);
        }
        else
        {
          OPENBL_FDCAN_ReadBytes(RxData, ((uint32_t) special_cmd->SizeBuffer1));
        }

        /* Read received bytes */
        for (index = 0U; index < special_cmd->SizeBuffer1; index++)
        {
          special_cmd->Buffer1[index] = RxData[index];
        }
      }

      /* Send receive size acknowledgment */
      OPENBL_FDCAN_SendByte(ACK_BYTE);

      /* Get the number of bytes to be received */
      OPENBL_FDCAN_ReadBytes(RxData, 2U);

      /* Read the MSB of the size byte */
      data                    = (*(uint8_t *)(RxData));
      special_cmd->SizeBuffer2 = ((uint16_t)data) << 8;

      /* Read the LSB of the size byte */
      data                     = (*(uint8_t *)(RxData + 1));
      special_cmd->SizeBuffer2 |= (uint16_t)data;

      if (special_cmd->SizeBuffer2 > SPECIAL_CMD_SIZE_BUFFER2)
      {
        OPENBL_FDCAN_SendByte(NACK_BYTE);
      }
      else
      {
        if (special_cmd->SizeBuffer2 != 0U)
        {
          count  = special_cmd->SizeBuffer2 / 64U;
          single = special_cmd->SizeBuffer2 % 64U;

          while (count != 0U)
          {
            OPENBL_FDCAN_ReadBytes(&RxData[data_length * 64U], 64U);
            count--;
            data_length++;
          }

          if (single != 0U)
          {
            OPENBL_FDCAN_ReadBytes(&RxData[data_length * 64U], single);
          }

          /* Read received bytes */
          for (index = 0U; index < special_cmd->SizeBuffer2; index++)
          {
            special_cmd->Buffer2[index] = RxData[index];
          }
        }

        /* Send receive write size acknowledgment */
        OPENBL_FDCAN_SendByte(ACK_BYTE);

        /* Process the special command */
        OPENBL_FDCAN_SpecialCommandProcess(special_cmd);

        /* NOTE: In case of any operation inside "SpecialCommandProcess" function that prevents the code
         * from returning to here (reset operation...), to be compatible with the OpenBL protocol,
         * the user must ensure sending the last ACK in the application side.
         */

        /* Send last acknowledgment */
        OPENBL_FDCAN_SendByte(ACK_BYTE);
      }
    }
  }
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  This function is used to construct the command List table.
  * @return Returns the number of supported commands.
  */
static uint8_t OPENBL_FDCAN_ConstructCommandsTable(OPENBL_CommandsTypeDef *pFdcanCmd)
{
  uint8_t i = 0U;

  if (pFdcanCmd->GetCommand != NULL)
  {
    a_OPENBL_FDCAN_CommandsList[i] = CMD_GET_COMMAND;
    i++;
  }

  if (pFdcanCmd->GetVersion != NULL)
  {
    a_OPENBL_FDCAN_CommandsList[i] = CMD_GET_VERSION;
    i++;
  }

  if (pFdcanCmd->GetID != NULL)
  {
    a_OPENBL_FDCAN_CommandsList[i] = CMD_GET_ID;
    i++;
  }

  if (pFdcanCmd->ReadMemory != NULL)
  {
    a_OPENBL_FDCAN_CommandsList[i] = CMD_READ_MEMORY;
    i++;
  }

  if (pFdcanCmd->Go != NULL)
  {
    a_OPENBL_FDCAN_CommandsList[i] = CMD_GO;
    i++;
  }

  if (pFdcanCmd->WriteMemory != NULL)
  {
    a_OPENBL_FDCAN_CommandsList[i] = CMD_WRITE_MEMORY;
    i++;
  }

  if (pFdcanCmd->EraseMemory != NULL)
  {
    a_OPENBL_FDCAN_CommandsList[i] = CMD_EXT_ERASE_MEMORY;
    i++;
  }

  if (pFdcanCmd->WriteProtect != NULL)
  {
    a_OPENBL_FDCAN_CommandsList[i] = CMD_WRITE_PROTECT;
    i++;
  }

  if (pFdcanCmd->WriteUnprotect != NULL)
  {
    a_OPENBL_FDCAN_CommandsList[i] = CMD_WRITE_UNPROTECT;
    i++;
  }

  if (pFdcanCmd->ReadoutProtect != NULL)
  {
    a_OPENBL_FDCAN_CommandsList[i] = CMD_READ_PROTECT;
    i++;
  }

  if (pFdcanCmd->ReadoutUnprotect != NULL)
  {
    a_OPENBL_FDCAN_CommandsList[i] = CMD_READ_UNPROTECT;
    i++;
  }

  if (pFdcanCmd->SpecialCommand != NULL)
  {
    a_OPENBL_FDCAN_CommandsList[i] = CMD_SPECIAL_COMMAND;
    i++;
  }

  if (pFdcanCmd->ExtendedSpecialCommand != NULL)
  {
    a_OPENBL_FDCAN_CommandsList[i] = CMD_EXTENDED_SPECIAL_COMMAND;
    i++;
  }

  return (i);
}

/**
  * @brief  This function is used to get the operation code.
  * @param  OpCode Pointer to the operation code to be returned.
  * @param  CmdType Type of the command, Special command or extended special command.
  * @retval Returns NACK status in case of error else returns ACK status.
  */
static uint8_t OPENBL_FDCAN_GetSpecialCmdOpCode(uint16_t *OpCode, OPENBL_SpecialCmdTypeTypeDef CmdType)
{
  uint8_t op_code[2];
  uint8_t status;
  uint8_t index;

  /* Initialize the status variable */
  status = NACK_BYTE;

  /* Get the command OpCode (2 bytes) */
  op_code[0] = RxData[0]; /* Read the MSB byte */
  op_code[1] = RxData[1]; /* Read the LSB byte */

  /* Get the operation code */
  *OpCode = ((uint16_t)op_code[0] << 8) | (uint16_t)op_code[1];

  if (CmdType == OPENBL_SPECIAL_CMD)
  {
    for (index = 0U; index < SPECIAL_CMD_MAX_NUMBER; index++)
    {
      if (SpecialCmdList[index] == *OpCode)
      {
        status = ACK_BYTE;
        break;
      }
    }
  }
  else
  {
    for (index = 0U; index < EXTENDED_SPECIAL_CMD_MAX_NUMBER; index++)
    {
      if (ExtendedSpecialCmdList[index] == *OpCode)
      {
        status = ACK_BYTE;
        break;
      }
    }
  }

  return status;
}
