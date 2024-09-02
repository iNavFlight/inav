/**
  ******************************************************************************
  * @file    openbl_i3c_cmd.c
  * @author  MCD Application Team
  * @brief   Contains I3C protocol commands
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "openbl_i3c_cmd.h"

#include "openbootloader_conf.h"
#include "app_openbootloader.h"
#include "i3c_interface.h"
#include "common_interface.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define OPENBL_I3C_COMMANDS_NB_MAX        13U       /* The maximum number of supported commands */

#define I3C_RAM_BUFFER_SIZE               2049U     /* Size of I3C buffer used to store received data from the host */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Buffer used to store received data from the host */
static uint8_t I3C_RAM_Buffer[I3C_RAM_BUFFER_SIZE];
static uint8_t I3cCommandsNumber                                     = 0U;
static uint8_t a_OPENBL_I3C_CommandsList[OPENBL_I3C_COMMANDS_NB_MAX] = {0U};

/* Private function prototypes -----------------------------------------------*/
static uint8_t OPENBL_I3C_ConstructCommandsTable(OPENBL_CommandsTypeDef *pI3cCmd);
static uint8_t OPENBL_I3C_GetAddress(uint32_t *pAddress);
static uint8_t OPENBL_I3C_GetSpecialCmdOpCode(uint16_t *pOpCode, OPENBL_SpecialCmdTypeTypeDef CmdType);

/* Exported variables --------------------------------------------------------*/
/* Exported functions---------------------------------------------------------*/

/**
  * @brief  This function is used to get a pointer to the structure that contains the available I3C commands.
  * @return Returns a pointer to the OPENBL_I3C_Commands structure.
  */
OPENBL_CommandsTypeDef *OPENBL_I3C_GetCommandsList(void)
{
  static OPENBL_CommandsTypeDef OPENBL_I3C_Commands =
  {
    OPENBL_I3C_GetCommand,
    OPENBL_I3C_GetVersion,
    OPENBL_I3C_GetID,
    OPENBL_I3C_ReadMemory,
    OPENBL_I3C_WriteMemory,
    OPENBL_I3C_Go,
    NULL,
    NULL,
    OPENBL_I3C_EraseMemory,
    OPENBL_I3C_WriteProtect,
    OPENBL_I3C_WriteUnprotect,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    OPENBL_I3C_SpecialCommand,
    OPENBL_I3C_ExtendedSpecialCommand
  };

  OPENBL_I3C_SetCommandsList(&OPENBL_I3C_Commands);

  return (&OPENBL_I3C_Commands);
}

/**
  * @brief  This function is used to set a pointer to the structure that contains the available I3C commands.
  * @param  pI3cCmd Pointer to the list of I3C commands structure.
  * @retval None.
  */
void OPENBL_I3C_SetCommandsList(OPENBL_CommandsTypeDef *pI3cCmd)
{
  /* Get the list of commands supported & their numbers */
  I3cCommandsNumber = OPENBL_I3C_ConstructCommandsTable(pI3cCmd);
}

/**
  * @brief  This function is used to get the list of the available I3C commands.
  * @retval None.
  */
void OPENBL_I3C_GetCommand(void)
{
  /* Send Acknowledge byte to notify the host that the command is recognized */
  OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

  /* Send the number of commands supported by the I3C protocol */
  OPENBL_I3C_SendByte(I3cCommandsNumber);

  OPENBL_I3C_SendByte(OPENBL_I3C_VERSION);

  /* Send the list of supported commands */
  OPENBL_I3C_SendBytes(a_OPENBL_I3C_CommandsList, I3cCommandsNumber);

  /* Send last Acknowledge synchronization byte */
  OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);
}

/**
  * @brief  This function is used to get the I3C protocol version.
  * @retval None.
  */
void OPENBL_I3C_GetVersion(void)
{
  /* Send Acknowledge byte to notify the host that the command is recognized */
  OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

  OPENBL_I3C_SendByte(OPENBL_I3C_VERSION);

  /* Send last Acknowledge synchronization byte */
  OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);
}

/**
  * @brief  This function is used to get the device ID.
  * @retval None.
  */
void OPENBL_I3C_GetID(void)
{
  uint8_t device_id[2] = {DEVICE_ID_MSB, DEVICE_ID_LSB};

  /* Send Acknowledge byte to notify the host that the command is recognized */
  OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

  /* Send the number of bytes to be transmitted to the host */
  OPENBL_I3C_SendByte(0x02U);

  /* Send the device ID bytes */
  OPENBL_I3C_SendBytes(device_id, 2U);

  /* Send last Acknowledge synchronization byte */
  OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);
}

/**
  * @brief  This function is used to read memory from the device.
  * @retval None.
  */
void OPENBL_I3C_ReadMemory(void)
{
  uint32_t size;
  uint32_t address;
  uint32_t index;
  uint32_t memory_index;
  uint8_t data[3] = {0U};
  uint8_t loop    = 1U;
  uint8_t xor;

  /* Check memory protection then send adequate response */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
  }
  else
  {
    OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

    /* Get the memory address */
    if (OPENBL_I3C_GetAddress(&address) == NACK_BYTE)
    {
      OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
    }
    else
    {
      OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

      while (loop != 0U)
      {
        /* Get the loop value, the number of bytes to be read and the XOR byte */
        OPENBL_I3C_ReadBytes(data, 3U);

        /* Extract the size from the received data */
        size  = (uint32_t) data[0] << 8U;
        size |= (uint32_t)(data[1]);

        /* Extract the loop value from LSB bit.
           The read loop will continue until this bit is set to 0 */
        loop = (uint8_t)(size & 0x1U);

        /* Extract the actual size */
        size = size >> 1U;

        /* Compute XOR value from received bytes */
        xor = data[0] ^ data[1];

        /* Check whether the parameters are valid or not */
        if ((OPENBL_MEM_GetAddressArea(address + size - 1U) == AREA_ERROR)   /* Check the validity of the address */
            || (xor != data[2])                                              /* Check data integrity */
            || (size > I3C_RAM_BUFFER_SIZE)                                  /* Size must not exceeds buffer size */
            || (size == 0U))                                                 /* Size must be different from 0 */
        {
          OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);

          /* End the loop */
          loop = 0U;
        }
        else
        {
          /* Get the memory index to know from which memory we will read */
          memory_index = OPENBL_MEM_GetMemoryIndex(address);

          /* Read the data from the memory and send them to the host */
          for (index = 0U; index < size; index++)
          {
            I3C_RAM_Buffer[index] = OPENBL_MEM_Read(address, memory_index);
            address++;
          }

          OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

          OPENBL_I3C_SendBytes(I3C_RAM_Buffer, size);
        }
      }
    }
  }
}

/**
  * @brief  This function is used to write in to device memory.
  * @retval None.
  */
void OPENBL_I3C_WriteMemory(void)
{
  uint32_t address;
  uint32_t size;
  uint32_t index;
  uint8_t xor;
  uint8_t loop    = 1U;
  uint8_t data[3] = {0U};

  /* Check memory protection then send adequate response */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
  }
  else
  {
    OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

    /* Get the memory address */
    if (OPENBL_I3C_GetAddress(&address) == NACK_BYTE)
    {
      OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
    }
    else
    {
      OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

      while (loop != 0U)
      {
        /* Get the loop value, the number of bytes to be read and the XOR byte */
        OPENBL_I3C_ReadBytes(data, 3U);

        /*  Extract the size from the received data */
        size  = (uint32_t) data[0] << 8U;
        size |= (uint32_t)(data[1]);

        /* Extract the loop value from LSB bit.
           The write loop will continue until this bit is set to 0 */
        loop = (uint8_t)size & 0x1U;

        /* Extract the actual size */
        size = size >> 1U;

        /* Compute XOR value from received bytes */
        xor = data[0] ^ data[1];

        /* Check whether the region is valid or not */
        if ((OPENBL_MEM_GetAddressArea(address + size - 1U) == AREA_ERROR)   /* Check the validity of the address */
            || (xor != data[2])                                              /* Check data integrity */
            || (size > (I3C_RAM_BUFFER_SIZE - 1U))                           /* Size must not exceeds buffer size */
            || (size == 0U))                                                 /* Size must be different from 0 */
        {
          OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);

          /* End the loop */
          loop = 0U;
        }
        else
        {
          OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

          /* Get the data and the xor byte (they are sent in the same I3C frame) */
          OPENBL_I3C_ReadBytes(I3C_RAM_Buffer, size + 1U);

          /* Initialize the XOR value */
          xor = 0U;

          /* Compute the XOR for the received data */
          for (index = 0U; index < size; index ++)
          {
            xor ^= I3C_RAM_Buffer[index];
          }

          /* Check the data integrity.
             The last byte in the buffer is the received XOR value */
          if (xor != I3C_RAM_Buffer[size])
          {
            OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
          }
          else
          {
            /* Write data to memory */
            OPENBL_MEM_Write(address, I3C_RAM_Buffer, size);

            /* Compute the new address value */
            address = address + size;

            /* Send last Acknowledge synchronization byte */
            OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

            /* Start post processing task if needed */
            Common_StartPostProcessing();
          }
        }
      }
    }
  }
}

/**
  * @brief  This function is used to jump to the user application.
  * @retval None.
  */
void OPENBL_I3C_Go(void)
{
  uint32_t address;
  uint8_t status;

  /* Check memory protection then send adequate response */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
  }
  else
  {
    OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

    /* Get memory address */
    if (OPENBL_I3C_GetAddress(&address) == NACK_BYTE)
    {
      OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
    }
    else
    {
      /* Check if received address is valid or not */
      status = OPENBL_MEM_CheckJumpAddress(address);

      if (status == 0U)
      {
        OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
      }
      else
      {
        /* If the jump address is valid then send ACK */
        OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

        OPENBL_MEM_JumpToAddress(address);
      }
    }
  }
}

/**
  * @brief  This function is used to erase a memory.
  * @retval None.
  */
void OPENBL_I3C_EraseMemory(void)
{
  uint32_t xor;
  uint32_t index;
  uint32_t numpage;
  uint32_t number_of_bytes;
  uint16_t data;
  ErrorStatus status;
  uint8_t temp_data;

  /* Check if the memory is not protected */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
  }
  else
  {
    OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

    /* Read number of pages to be erased (2 bytes) */
    data = OPENBL_I3C_ReadByte();
    data = (data << 8U) | OPENBL_I3C_ReadByte();

    /* Compute the XOR value */
    xor  = ((uint32_t)data & 0xFF00U) >> 8U;
    xor ^= (uint32_t)data & 0x00FFU;

    /* All commands in range [0xFFF0 - 0xFFFF] are reserved for special erase features */
    if ((data & 0xFFF0U) == 0xFFF0U)
    {
      /* Check data integrity */
      if ((uint8_t) xor != OPENBL_I3C_ReadByte())
      {
        OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
      }
      else
      {
        /* 0xFFFF: mass erase
         * 0xFFFE: bank 1 erase
         * 0xFFFD: bank 2 erase
         */
        if ((data == 0xFFFFU) || (data == 0xFFFEU) || (data == 0xFFFDU))
        {
          I3C_RAM_Buffer[0] = (uint8_t)(data & 0x00FFU);
          I3C_RAM_Buffer[1] = (uint8_t)((data & 0xFF00U) >> 8U);

          status = OPENBL_MEM_MassErase(OPENBL_DEFAULT_MEM, I3C_RAM_Buffer, I3C_RAM_BUFFER_SIZE);

          if (status == SUCCESS)
          {
            OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);
          }
          else
          {
            OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
          }
        }
        else
        {
          /* This sub-command is not supported */
          OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
        }
      }
    }
    else
    {
      /* Check data integrity */
      if (OPENBL_I3C_ReadByte() != (uint8_t) xor)
      {
        OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
      }
      else
      {
        /* Number of pages to be erased */
        numpage = (uint32_t)data;

        /* Check if the number of pages to be erased is valid or not.
           It must not exceeds the half buffer size as each page is coded on two bytes */
        if ((numpage != 0U) && (numpage < (I3C_RAM_BUFFER_SIZE / 2U)))
        {
          OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

          /* Compute the number of bytes: number of pages * 2 (each page is coded in two bytes) + 1 byte for XOR */
          number_of_bytes = (2U * numpage) + 1U;

          I3C_RAM_Buffer[0] = (uint8_t)(numpage & 0x00FFU);
          I3C_RAM_Buffer[1] = (uint8_t)((numpage & 0xFF00U) >> 8U);

          /* Get the pages to be erased and their XOR */
          OPENBL_I3C_ReadBytes(&I3C_RAM_Buffer[2], number_of_bytes);

          /* Initialize the XOR value */
          xor = 0U;

          /* Compute the xor for the received data */
          for (index = 2U; index <= number_of_bytes ; index++)
          {
            xor ^= I3C_RAM_Buffer[index];
          }

          /* Check data integrity:
             The pages bytes are stored starting from the 2nd index of the I3C_RAM_Buffer.
             The xor byte index is at "number_of_bytes + 1" */
          if ((uint8_t) xor != I3C_RAM_Buffer[number_of_bytes + 1U])
          {
            OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
          }
          else
          {
            /* Invert bytes as they were received MSB first */
            for (index = 2U; index <= number_of_bytes; index += 2U)
            {
              temp_data                  = I3C_RAM_Buffer[index];
              I3C_RAM_Buffer[index]      = I3C_RAM_Buffer[index + 1U];
              I3C_RAM_Buffer[index + 1U] = temp_data;
            }

            status = OPENBL_MEM_Erase(OPENBL_DEFAULT_MEM, I3C_RAM_Buffer, I3C_RAM_BUFFER_SIZE);

            /* Errors from memory erase are not managed, always return ACK */
            if (status == SUCCESS)
            {
              OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);
            }
            else
            {
              OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
            }
          }
        }
        else
        {
          /* The number of pages to be erased is not valid */
          OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
        }
      }
    }
  }
}

/**
  * @brief  This function is used to enable write protection.
  * @retval None.
  */
void OPENBL_I3C_WriteProtect(void)
{
  uint32_t number_of_bytes;
  uint16_t index;
  uint16_t numpage;
  uint8_t data[3] = {0U};
  uint8_t xor;
  ErrorStatus status;

  /* Check memory protection then send adequate response */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
  }
  else
  {
    OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

    /* Get the number of pages/sectors and the XOR value  */
    OPENBL_I3C_ReadBytes(data, 3U);

    /* Compute the XOR for the received data */
    xor = data[0] ^ data[1];

    /* Send NACK if the XOR is incorrect */
    if (xor != data[2])
    {
      OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
    }
    else
    {
      OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

      /* Number of pages/sectors to be protected */
      numpage = (uint16_t)data[0] << 8U;
      numpage = numpage | (uint16_t)data[1];

      /* Number of bytes to be read (each page/sector number is coded on two bytes) */
      number_of_bytes = (uint32_t)2U * (uint32_t)numpage;

      /* The number of bytes to be read must not exceeds the buffer size */
      if (number_of_bytes > (I3C_RAM_BUFFER_SIZE - 1U))
      {
        OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
      }
      else
      {
        /* Receive data and the XOR byte */
        OPENBL_I3C_ReadBytes(I3C_RAM_Buffer, number_of_bytes + 1U);

        /* Initialize the XOR value */
        xor = 0U;

        /* Compute the XOR value for the received data */
        for (index = 0U; index < number_of_bytes ; index++)
        {
          xor ^= I3C_RAM_Buffer[index];
        }

        /* Check data integrity and send NACK if the XOR is incorrect */
        if (I3C_RAM_Buffer[number_of_bytes] != (uint8_t) xor)
        {
          OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
        }
        else
        {
          /* Enable write protection */
          status = OPENBL_MEM_SetWriteProtection(ENABLE, OPENBL_DEFAULT_MEM, I3C_RAM_Buffer, numpage);

          OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

          if (status == SUCCESS)
          {
            /* Start post processing task if needed */
            Common_StartPostProcessing();
          }
        }
      }
    }
  }
}

/**
  * @brief  This function is used to disable write protection.
  * @retval None.
  */
void OPENBL_I3C_WriteUnprotect(void)
{
  ErrorStatus status;

  /* Check memory protection then send adequate response */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
  }
  else
  {
    OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

    /* Disable write protection */
    status = OPENBL_MEM_SetWriteProtection(DISABLE, OPENBL_DEFAULT_MEM, NULL, 0U);

    OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

    if (status == SUCCESS)
    {
      /* Start post processing task if needed */
      Common_StartPostProcessing();
    }
  }
}

/**
  * @brief  This function is used to execute special command.
  * @retval None.
  */
void OPENBL_I3C_SpecialCommand(void)
{
  uint32_t number_of_bytes;
  OPENBL_SpecialCmdTypeDef *p_special_cmd;
  uint16_t op_code;
  uint8_t xor;
  uint8_t index;
  uint8_t temp_buffer[2];

  /* Send acknowledgment */
  OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

  /* Get the special command operation code */
  if (OPENBL_I3C_GetSpecialCmdOpCode(&op_code, OPENBL_SPECIAL_CMD) == NACK_BYTE)
  {
    OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
  }
  else
  {
    /* Send Operation code acknowledgment */
    OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

    /* Get the number of bytes to be received */
    OPENBL_I3C_ReadBytes(temp_buffer, 2U);

    /* Initialize the special command pointer */
    p_special_cmd = (OPENBL_SpecialCmdTypeDef *)(uint32_t) I3C_RAM_Buffer;

    /* Initialize the special command structure */
    p_special_cmd->CmdType = OPENBL_SPECIAL_CMD;
    p_special_cmd->OpCode  = op_code;

    /* Initialize the XOR value */
    xor = 0U;

    p_special_cmd->SizeBuffer1  = ((uint16_t)temp_buffer[0]) << 8U;
    xor                        ^= temp_buffer[0];

    p_special_cmd->SizeBuffer1 |= (uint16_t)temp_buffer[1];
    xor                        ^= temp_buffer[1];

    /* Check that the received buffer size doesn't exceeds the max buffer size */
    if ((p_special_cmd->SizeBuffer1) > SPECIAL_CMD_SIZE_BUFFER1)
    {
      OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
    }
    else
    {
      number_of_bytes = (uint32_t)p_special_cmd->SizeBuffer1;

      /* The buffer can be empty, so only receive data if number of bytes is different of 0 */
      if (number_of_bytes != 0U)
      {
        /* Get received bytes and their XOR byte */
        OPENBL_I3C_ReadBytes(p_special_cmd->Buffer1, number_of_bytes + 1U);

        /* Compute the XOR for the received bytes */
        for (index = 0U; index < number_of_bytes; index++)
        {
          xor ^= p_special_cmd->Buffer1[index];
        }
      }

      /* Check data integrity */
      if (xor != p_special_cmd->Buffer1[number_of_bytes])
      {
        OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
      }
      else
      {
        /* Send received size acknowledgment */
        OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

        /* Process the special command */
        OPENBL_I3C_SpecialCommandProcess(p_special_cmd);

        /* NOTE: In case of any operation inside "SpecialCommandProcess" function that prevents the code
         * from returning to here (reset operation...), to be compatible with the OpenBL protocol,
         * the user must ensure sending the last ACK in the application side.
         */

        /* Send last acknowledgment */
        OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);
      }
    }
  }
}

/**
  * @brief  This function is used to execute extended special command.
  * @retval None.
  */
void OPENBL_I3C_ExtendedSpecialCommand(void)
{
  uint32_t number_of_bytes;
  OPENBL_SpecialCmdTypeDef *p_special_cmd;
  uint16_t op_code;
  uint16_t index;
  uint8_t temp_buffer[2];
  uint8_t xor;

  /* Send Operation code acknowledgment */
  OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

  /* Get the command operation code */
  if (OPENBL_I3C_GetSpecialCmdOpCode(&op_code, OPENBL_EXTENDED_SPECIAL_CMD) == NACK_BYTE)
  {
    OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
  }
  else
  {
    /* Send Operation code acknowledgment */
    OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

    /* Get the number of bytes to be received */
    OPENBL_I3C_ReadBytes(temp_buffer, 2U);

    /* Initialize the special command pointer */
    p_special_cmd = (OPENBL_SpecialCmdTypeDef *)(uint32_t) I3C_RAM_Buffer;

    /* Initialize the special command structure */
    p_special_cmd->CmdType = OPENBL_EXTENDED_SPECIAL_CMD;
    p_special_cmd->OpCode  = op_code;

    /* Initialize the XOR value */
    xor = 0U;

    p_special_cmd->SizeBuffer1  = ((uint16_t)temp_buffer[0]) << 8U;
    xor                        ^= temp_buffer[0];

    p_special_cmd->SizeBuffer1 |= (uint16_t)temp_buffer[1];
    xor                        ^= temp_buffer[1];

    /* Check that the received buffer size doesn't exceeds the max buffer size */
    if ((p_special_cmd->SizeBuffer1) > SPECIAL_CMD_SIZE_BUFFER1)
    {
      OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
    }
    else
    {
      number_of_bytes = (uint32_t)p_special_cmd->SizeBuffer1;

      /* The buffer can be empty, so only receive data if the buffer size is different of 0 */
      if (number_of_bytes != 0U)
      {
        /* Get received bytes and their XOR */
        OPENBL_I3C_ReadBytes(p_special_cmd->Buffer1, number_of_bytes + 1U);

        /* Compute the XOR for the received bytes */
        for (index = 0U; index < number_of_bytes; index++)
        {
          xor ^= p_special_cmd->Buffer1[index];
        }
      }

      /* Check data integrity */
      if (xor != p_special_cmd->Buffer1[number_of_bytes])
      {
        OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
      }
      else
      {
        /* Send receive size acknowledgment */
        OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

        /* Get size of buffer 2 */
        OPENBL_I3C_ReadBytes(temp_buffer, 2U);

        /* Initialize the XOR value */
        xor = 0U;

        /* Read the MSB of the size byte */
        p_special_cmd->SizeBuffer2 = ((uint16_t)temp_buffer[0]) << 8U;
        xor                       ^= temp_buffer[0];

        /* Read the LSB of the size byte */
        p_special_cmd->SizeBuffer2 |= (uint16_t)temp_buffer[1];
        xor                        ^= temp_buffer[1];

        /* Check that the received buffer size doesn't exceeds the max buffer size */
        if ((p_special_cmd->SizeBuffer2) > SPECIAL_CMD_SIZE_BUFFER2)
        {
          OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
        }
        else
        {
          number_of_bytes = (uint32_t)(p_special_cmd->SizeBuffer2);

          /* The buffer can be empty, so only receive data if the buffer size is different of 0 */
          if (number_of_bytes != 0U)
          {
            /* Get received bytes and their XOR */
            OPENBL_I3C_ReadBytes(p_special_cmd->Buffer2, number_of_bytes + 1U);

            /* Compute the XOR for the received bytes */
            for (index = 0U; index < number_of_bytes; index++)
            {
              xor ^= p_special_cmd->Buffer2[index];
            }
          }

          /* Check data integrity */
          if (xor != p_special_cmd->Buffer2[number_of_bytes])
          {
            OPENBL_I3C_SendAcknowledgeByte(NACK_BYTE);
          }
          else
          {
            /* Send receive write size acknowledgment */
            OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

            /* Process the special command */
            OPENBL_I3C_SpecialCommandProcess(p_special_cmd);

            /* NOTE: In case of any operation inside "SpecialCommandProcess" function that prevents the code
             * from returning to here (reset operation...), to be compatible with the OpenBL protocol,
             * the user must ensure sending the last ACK in the application side.
             */

            /* Send last acknowledgment */
            OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);
          }
        }
      }
    }
  }
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  This function is used to construct the command List table.
  * @param  pI3cCmd pointer to the structure that contains the available I3C commands.
  * @return Returns a table with all opcodes supported.
  */
static uint8_t OPENBL_I3C_ConstructCommandsTable(OPENBL_CommandsTypeDef *pI3cCmd)
{
  uint8_t index = 0U;

  if (pI3cCmd->GetCommand != NULL)
  {
    a_OPENBL_I3C_CommandsList[index] = CMD_GET_COMMAND;
    index++;
  }

  if (pI3cCmd->GetVersion != NULL)
  {
    a_OPENBL_I3C_CommandsList[index] = CMD_GET_VERSION;
    index++;
  }

  if (pI3cCmd->GetID != NULL)
  {
    a_OPENBL_I3C_CommandsList[index] = CMD_GET_ID;
    index++;
  }

  if (pI3cCmd->ReadMemory != NULL)
  {
    a_OPENBL_I3C_CommandsList[index] = CMD_READ_MEMORY;
    index++;
  }

  if (pI3cCmd->Go != NULL)
  {
    a_OPENBL_I3C_CommandsList[index] = CMD_GO;
    index++;
  }

  if (pI3cCmd->WriteMemory != NULL)
  {
    a_OPENBL_I3C_CommandsList[index] = CMD_WRITE_MEMORY;
    index++;
  }

  if (pI3cCmd->EraseMemory != NULL)
  {
    a_OPENBL_I3C_CommandsList[index] = CMD_EXT_ERASE_MEMORY;
    index++;
  }

  if (pI3cCmd->WriteProtect != NULL)
  {
    a_OPENBL_I3C_CommandsList[index] = CMD_WRITE_PROTECT;
    index++;
  }

  if (pI3cCmd->WriteUnprotect != NULL)
  {
    a_OPENBL_I3C_CommandsList[index] = CMD_WRITE_UNPROTECT;
    index++;
  }

  if (pI3cCmd->ReadoutProtect != NULL)
  {
    a_OPENBL_I3C_CommandsList[index] = CMD_READ_PROTECT;
    index++;
  }

  if (pI3cCmd->ReadoutUnprotect != NULL)
  {
    a_OPENBL_I3C_CommandsList[index] = CMD_READ_UNPROTECT;
    index++;
  }

  if (pI3cCmd->SpecialCommand != NULL)
  {
    a_OPENBL_I3C_CommandsList[index] = CMD_SPECIAL_COMMAND;
    index++;
  }

  if (pI3cCmd->ExtendedSpecialCommand != NULL)
  {
    a_OPENBL_I3C_CommandsList[index] = CMD_EXTENDED_SPECIAL_COMMAND;
    index++;
  }

  return (index);
}

/**
  * @brief  This function is used to get a valid address.
  * @param  pAddress pointer to the address.
  * @retval Returns NACK status in case of error else returns ACK status.
  */
static uint8_t OPENBL_I3C_GetAddress(uint32_t *pAddress)
{
  uint8_t data[5] = {0U};
  uint8_t status;
  uint8_t xor;

  /* Get the address (4 bytes) and its XOR value (1 byte) */
  OPENBL_I3C_ReadBytes(data, 5U);

  xor = data[0] ^ data[1] ^ data[2] ^ data[3];

  /* Check the integrity of received data */
  if (xor != data[4])
  {
    status = NACK_BYTE;
  }
  else
  {
    *pAddress = ((uint32_t)data[0] << 24U)
                | ((uint32_t)data[1] << 16U)
                | ((uint32_t)data[2] << 8U)
                | (uint32_t)data[3];

    /* Check if received address is valid or not */
    if (OPENBL_MEM_GetAddressArea(*pAddress) == AREA_ERROR)
    {
      status = NACK_BYTE;
    }
    else
    {
      status = ACK_BYTE;
    }
  }

  return status;
}

/**
  * @brief  This function is used to get the operation code.
  * @param  pOpCode Pointer to the operation code to be returned.
  * @param  CmdType Type of the command, special command or extended special command.
  * @retval Returns NACK status in case of error else returns ACK status.
  */
static uint8_t OPENBL_I3C_GetSpecialCmdOpCode(uint16_t *pOpCode, OPENBL_SpecialCmdTypeTypeDef CmdType)
{
  uint8_t data[3];
  uint8_t xor;
  uint8_t status;
  uint8_t index;

  /* Initialize the status variable */
  status = NACK_BYTE;

  /* Get the command OpCode and its XOR */
  OPENBL_I3C_ReadBytes(data, 3U);

  /* Get the XOR byte */
  xor  = data[0];
  xor ^= data[1];

  /* Check the integrity of received data */
  if (xor != data[2])
  {
    status = NACK_BYTE;
  }
  else
  {
    /* Get the operation code */
    *pOpCode = ((uint16_t)data[0] << 8U) | (uint16_t)data[1];

    if (CmdType == OPENBL_SPECIAL_CMD)
    {
      /* Check if the operation code exists in special command list */
      for (index = 0U; index < SPECIAL_CMD_MAX_NUMBER; index++)
      {
        if (SpecialCmdList[index] == *pOpCode)
        {
          status = ACK_BYTE;
          break;
        }
      }
    }
    else if (CmdType == OPENBL_EXTENDED_SPECIAL_CMD)
    {
      /* Check if the operation code exists in extended special command list */
      for (index = 0U; index < EXTENDED_SPECIAL_CMD_MAX_NUMBER; index++)
      {
        if (ExtendedSpecialCmdList[index] == *pOpCode)
        {
          status = ACK_BYTE;
          break;
        }
      }
    }
    else
    {
      status = NACK_BYTE;
    }
  }

  return status;
}
