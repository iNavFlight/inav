/**
  ******************************************************************************
  * @file    openbl_i2c_cmd.c
  * @author  MCD Application Team
  * @brief   Contains I2C protocol commands
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
#include "openbl_i2c_cmd.h"

#include "openbootloader_conf.h"
#include "app_openbootloader.h"
#include "i2c_interface.h"
#include "common_interface.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define OPENBL_I2C_COMMANDS_NB_MAX        19U       /* Number of supported commands */

#define I2C_RAM_BUFFER_SIZE               1164U     /* Size of I2C buffer used to store received data from the host */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Buffer used to store received data from the host */
static uint8_t I2C_RAM_Buf[I2C_RAM_BUFFER_SIZE];
static uint8_t a_OPENBL_I2C_CommandsList[OPENBL_I2C_COMMANDS_NB_MAX] = {0U};
static uint8_t I2cCommandsNumber = 0U;

/* Private function prototypes -----------------------------------------------*/
static uint8_t OPENBL_I2C_GetAddress(uint32_t *pAddress);
static uint8_t OPENBL_I2C_GetSpecialCmdOpCode(uint16_t *OpCode, OPENBL_SpecialCmdTypeTypeDef CmdType);
static uint8_t OPENBL_I2C_ConstructCommandsTable(OPENBL_CommandsTypeDef *pI2cCmd);

/* Exported variables --------------------------------------------------------*/
/* Exported functions---------------------------------------------------------*/

/**
  * @brief  This function is used to get a pointer to the structure that contains the available I2C commands.
  * @return Returns a pointer to the OPENBL_I2C_Commands struct.
  */
OPENBL_CommandsTypeDef *OPENBL_I2C_GetCommandsList(void)
{
  static OPENBL_CommandsTypeDef OPENBL_I2C_Commands =
  {
    OPENBL_I2C_GetCommand,
    OPENBL_I2C_GetVersion,
    OPENBL_I2C_GetID,
    OPENBL_I2C_ReadMemory,
    OPENBL_I2C_WriteMemory,
    OPENBL_I2C_Go,
    OPENBL_I2C_ReadoutProtect,
    OPENBL_I2C_ReadoutUnprotect,
    OPENBL_I2C_EraseMemory,
    OPENBL_I2C_WriteProtect,
    OPENBL_I2C_WriteUnprotect,
    OPENBL_I2C_NonStretchWriteMemory,
    OPENBL_I2C_NonStretchEraseMemory,
    OPENBL_I2C_NonStretchWriteProtect,
    OPENBL_I2C_NonStretchWriteUnprotect,
    OPENBL_I2C_NonStretchReadoutProtect,
    OPENBL_I2C_NonStretchReadoutUnprotect,
    NULL,
    OPENBL_I2C_SpecialCommand,
    OPENBL_I2C_ExtendedSpecialCommand
  };

  OPENBL_I2C_SetCommandsList(&OPENBL_I2C_Commands);

  return (&OPENBL_I2C_Commands);
}

/**
  * @brief  This function is used to set a pointer to the structure that contains the available I2C commands.
  * @retval Returns a pointer to the OPENBL_I2C_Commands struct.
  */
void OPENBL_I2C_SetCommandsList(OPENBL_CommandsTypeDef *pI2cCmd)
{
  /* Get the list of commands supported & their numbers */
  I2cCommandsNumber = OPENBL_I2C_ConstructCommandsTable(pI2cCmd);
}

/**
  * @brief  This function is used to get the list of the available I2C commands
  * @retval None.
  */
void OPENBL_I2C_GetCommand(void)
{
  uint32_t counter;

  OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

  /* Wait for address to match */
  OPENBL_I2C_WaitAddress();

  /* Send the number of commands supported by the I2C protocol */
  OPENBL_I2C_SendByte(I2cCommandsNumber);

  /* Send I2C protocol version */
  OPENBL_I2C_SendByte(OPENBL_I2C_VERSION);

  /* Send the list of supported commands */
  for (counter = 0U; counter < I2cCommandsNumber; counter++)
  {
    OPENBL_I2C_SendByte(a_OPENBL_I2C_CommandsList[counter]);
  }

  /* Wait until NACK is detected */
  OPENBL_I2C_WaitNack();

  /* Wait until STOP is detected */
  OPENBL_I2C_WaitStop();

  /* Send last Acknowledge synchronization byte */
  OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);
}

/**
  * @brief  This function is used to get the I2C protocol version.
  * @retval None.
  */
void OPENBL_I2C_GetVersion(void)
{
  /* Send Acknowledge byte to notify the host that the command is recognized */
  OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

  /* Wait for address to match */
  OPENBL_I2C_WaitAddress();

  /* Send I2C protocol version */
  OPENBL_I2C_SendByte(OPENBL_I2C_VERSION);

  /* Wait until NACK is detected */
  OPENBL_I2C_WaitNack();

  /* Wait until STOP is detected */
  OPENBL_I2C_WaitStop();

  /* Send last Acknowledge synchronization byte */
  OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);
}

/**
  * @brief  This function is used to get the device ID.
  * @retval None.
  */
void OPENBL_I2C_GetID(void)
{
  /* Send Acknowledge byte to notify the host that the command is recognized */
  OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

  /* Wait for address to match */
  OPENBL_I2C_WaitAddress();

  OPENBL_I2C_SendByte(0x01U);

  /* Send the device ID starting by the MSB byte then the LSB byte */
  OPENBL_I2C_SendByte(DEVICE_ID_MSB);
  OPENBL_I2C_SendByte(DEVICE_ID_LSB);

  /* Wait until NACK is detected */
  OPENBL_I2C_WaitNack();

  /* Wait until STOP is detected */
  OPENBL_I2C_WaitStop();

  /* Send last Acknowledge synchronization byte */
  OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);
}

/**
  * @brief  This function is used to read memory from the device.
  * @retval None.
  */
void OPENBL_I2C_ReadMemory(void)
{
  uint32_t address;
  uint32_t counter;
  uint32_t memory_index;
  uint8_t data;
  uint8_t xor;

  /* Check memory protection then send adequate response */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
  }
  else
  {
    OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

    /* Get the memory address */
    if (OPENBL_I2C_GetAddress(&address) == NACK_BYTE)
    {
      OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
    }
    else
    {
      OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

      /* Wait for address to match */
      OPENBL_I2C_WaitAddress();

      /* Get the number of bytes to be received */
      data = OPENBL_I2C_ReadByte();
      xor  = ~data;

      /* Check data integrity */
      if (OPENBL_I2C_ReadByte() != xor)
      {
        OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
      }
      else
      {
        OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

        /* Get the memory index to know from which memory we will read */
        memory_index = OPENBL_MEM_GetMemoryIndex(address);

        /* Wait for address to match */
        OPENBL_I2C_WaitAddress();

        /* Read the data (data + 1) from the memory and send them to the host */
        for (counter = ((uint32_t)data + 1U); counter != 0U; counter--)
        {
          OPENBL_I2C_SendByte(OPENBL_MEM_Read(address, memory_index));
          address++;
        }

        /* Wait until NACK is detected */
        OPENBL_I2C_WaitNack();

        /* Wait until STOP is detected */
        OPENBL_I2C_WaitStop();
      }
    }
  }
}

/**
  * @brief  This function is used to write in to device memory.
  * @retval None.
  */
void OPENBL_I2C_WriteMemory(void)
{
  uint32_t address;
  uint32_t xor;
  uint32_t counter;
  uint32_t codesize;
  uint8_t *p_ramaddress;
  uint8_t data;

  /* Check memory protection then send adequate response */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
  }
  else
  {
    OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

    /* Get the memory address */
    if (OPENBL_I2C_GetAddress(&address) == NACK_BYTE)
    {
      OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
    }
    else
    {
      OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

      /* Read number of bytes to be written and data */
      p_ramaddress = (uint8_t *)I2C_RAM_Buf;

      OPENBL_I2C_WaitAddress();

      /* Read the number of bytes to be written: Max number of data = data + 1 = 256 */
      data = OPENBL_I2C_ReadByte();

      /* Number of data to be written = data + 1 */
      codesize = (uint32_t)data + 1U;

      /* Checksum Initialization */
      xor = data;

      /* I2C receive data and send to RAM Buffer */
      for (counter = codesize; counter != 0U ; counter--)
      {
        data    = OPENBL_I2C_ReadByte();
        xor ^= data;

        *(__IO uint8_t *)(p_ramaddress) = data;

        p_ramaddress++;
      }

      /* Send NACk if Checksum is incorrect */
      if (OPENBL_I2C_ReadByte() != xor)
      {
        OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
      }
      else
      {
        /* Write data to memory */
        OPENBL_MEM_Write(address, (uint8_t *)I2C_RAM_Buf, codesize);

        /* Send last Acknowledge synchronization byte */
        OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

        /* Start post processing task if needed */
        Common_StartPostProcessing();
      }
    }
  }
}

/**
  * @brief  This function is used to jump to the user application.
  * @retval None.
  */
void OPENBL_I2C_Go(void)
{
  uint32_t address;
  uint8_t status;

  /* Check memory protection then send adequate response */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
  }
  else
  {
    OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

    /* Get memory address */
    if (OPENBL_I2C_GetAddress(&address) == NACK_BYTE)
    {
      OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
    }
    else
    {
      /* Check if received address is valid or not */
      status = OPENBL_MEM_CheckJumpAddress(address);

      if (status == 0U)
      {
        OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
      }
      else
      {
        /* If the jump address is valid then send ACK */
        OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

        OPENBL_MEM_JumpToAddress(address);
      }
    }
  }
}

/**
  * @brief  This function is used to enable readout protection.
  * @retval None.
  */
void OPENBL_I2C_ReadoutProtect(void)
{
  /* Check memory protection then send adequate response */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
  }
  else
  {
    OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

    /* Enable the read protection */
    OPENBL_MEM_SetReadOutProtection(OPENBL_DEFAULT_MEM, ENABLE);

    OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

    /* Start post processing task if needed */
    Common_StartPostProcessing();
  }
}

/**
  * @brief  This function is used to disable readout protection.
  * @note   Going from RDP level 1 to RDP level 0 erase all the flash,
  *         so the send of second acknowledge after Disabling the read protection
  *         is not possible what make the communication with the host get lost
  * @retval None.
  */
void OPENBL_I2C_ReadoutUnprotect(void)
{
  OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

  OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

  /* Disable the read protection */
  OPENBL_MEM_SetReadOutProtection(OPENBL_DEFAULT_MEM, DISABLE);

  /* Start post processing task if needed */
  Common_StartPostProcessing();
}

/**
  * @brief  This function is used to erase a memory.
  * @retval None.
  */
void OPENBL_I2C_EraseMemory(void)
{
  uint32_t xor;
  uint32_t counter;
  uint32_t numpage;
  uint16_t data;
  ErrorStatus error_value;
  uint8_t status = ACK_BYTE;
  uint8_t *p_ramaddress;

  p_ramaddress = (uint8_t *) I2C_RAM_Buf;

  /* Check if the memory is not protected */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
  }
  else
  {
    OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

    /* Wait for address to match */
    OPENBL_I2C_WaitAddress();

    /* Read number of pages to be erased */
    data = OPENBL_I2C_ReadByte();
    data = (uint16_t)(data << 8) | OPENBL_I2C_ReadByte();

    /* Checksum initialization */
    xor  = ((uint32_t)data & 0xFF00U) >> 8;
    xor ^= (uint32_t)data & 0x00FFU;

    /* All commands in range 0xFFFZ are reserved for special erase features */
    if ((data & 0xFFF0U) == 0xFFF0U)
    {
      /* Check data integrity */
      if (OPENBL_I2C_ReadByte() != (uint8_t) xor)
      {
        OPENBL_I2C_WaitStop();

        status = NACK_BYTE;
      }
      else
      {
        OPENBL_I2C_WaitStop();

        if ((data == 0xFFFFU) || (data == 0xFFFEU) || (data == 0xFFFDU))
        {
          p_ramaddress[0] = (uint8_t)(data & 0x00FFU);
          p_ramaddress[1] = (uint8_t)((data & 0xFF00U) >> 8);

          error_value = OPENBL_MEM_MassErase(OPENBL_DEFAULT_MEM, (uint8_t *) I2C_RAM_Buf, I2C_RAM_BUFFER_SIZE);

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
    }
    else
    {
      /* Check data integrity */
      if (OPENBL_I2C_ReadByte() != (uint8_t) xor)
      {
        OPENBL_I2C_WaitStop();

        status = NACK_BYTE;
      }
      else
      {
        OPENBL_I2C_WaitStop();

        OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

        p_ramaddress = (uint8_t *) I2C_RAM_Buf;

        /* Number of pages to be erased (data + 1) */
        numpage = (uint32_t)data + 1U;

        *p_ramaddress = (uint8_t)(numpage & 0x00FFU);
        p_ramaddress++;

        *p_ramaddress = (uint8_t)((numpage & 0xFF00U) >> 8);
        p_ramaddress++;

        OPENBL_I2C_WaitAddress();

        xor = 0;

        /* Get the pages to be erased */
        for (counter = numpage; counter != 0U ; counter--)
        {
          /* Receive the MSB byte */
          data  = OPENBL_I2C_ReadByte();
          xor  ^= data;
          data  = (uint16_t)((data & 0x00FFU) << 8);

          /* Receive the LSB byte */
          data |= (uint8_t)(OPENBL_I2C_ReadByte() & 0x00FFU);
          xor  ^= ((uint32_t)data & 0x00FFU);

          /* Only store data that fit in the buffer length */
          if (counter < (I2C_RAM_BUFFER_SIZE / 2U))
          {
            *p_ramaddress = (uint8_t)(data & 0x00FFU);
            p_ramaddress++;

            *p_ramaddress = (uint8_t)((data & 0xFF00U) >> 8);
            p_ramaddress++;
          }
        }

        /* Check data integrity */
        if (OPENBL_I2C_ReadByte() != (uint8_t) xor)
        {
          OPENBL_I2C_WaitStop();

          status = NACK_BYTE;
        }
        else
        {
          OPENBL_I2C_WaitStop();

          error_value = OPENBL_MEM_Erase(OPENBL_DEFAULT_MEM, (uint8_t *) I2C_RAM_Buf, I2C_RAM_BUFFER_SIZE);

          /* Errors from memory erase are not managed, always return ACK */
          if (error_value == SUCCESS)
          {
            status = ACK_BYTE;
          }
        }
      }
    }

    OPENBL_I2C_SendAcknowledgeByte(status);
  }
}

/**
  * @brief  This function is used to enable write protect.
  * @retval None.
  */
void OPENBL_I2C_WriteProtect(void)
{
  uint16_t counter;
  uint16_t length;
  uint8_t data;
  uint8_t xor;
  ErrorStatus error_value;
  uint8_t *p_ramaddress;

  /* Check if the memory is not protected */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
  }
  else
  {
    OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

    /* Wait for address to match */
    OPENBL_I2C_WaitAddress();

    /* Get the data length */
    data = OPENBL_I2C_ReadByte();
    xor  = ~data;

    /* Send NACk if Checksum is incorrect */
    if (OPENBL_I2C_ReadByte() != xor)
    {
      OPENBL_I2C_WaitStop();
      OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
    }
    else
    {
      OPENBL_I2C_WaitStop();
      OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

      p_ramaddress = (uint8_t *) I2C_RAM_Buf;
      length       = (uint16_t) data + 1U;

      /* Checksum Initialization */
      xor = 0U;

      /* Wait for address to match */
      OPENBL_I2C_WaitAddress();

      /* Receive data and write to RAM Buffer */
      for (counter = (length); counter != 0U ; counter--)
      {
        data  = OPENBL_I2C_ReadByte();
        xor  ^= data;

        *(__IO uint8_t *)(p_ramaddress) = (uint8_t) data;

        p_ramaddress++;
      }

      /* Check data integrity and send NACK if Checksum is incorrect */
      if (OPENBL_I2C_ReadByte() != (uint8_t) xor)
      {
        OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
      }
      else
      {
        p_ramaddress = (uint8_t *) I2C_RAM_Buf;

        /* Enable the write protection */
        error_value = OPENBL_MEM_SetWriteProtection(ENABLE, OPENBL_DEFAULT_MEM, p_ramaddress, length);

        OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

        if (error_value == SUCCESS)
        {
          /* Start post processing task if needed */
          Common_StartPostProcessing();
        }
      }
    }
  }
}

/**
  * @brief  This function is used to disable write protect.
  * @retval None.
  */
void OPENBL_I2C_WriteUnprotect(void)
{
  ErrorStatus error_value;

  /* Check if the memory is not protected */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
  }
  else
  {
    OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

    /* Disable write protection */
    error_value = OPENBL_MEM_SetWriteProtection(DISABLE, OPENBL_DEFAULT_MEM, NULL, 0U);

    OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

    if (error_value == SUCCESS)
    {
      /* Start post processing task if needed */
      Common_StartPostProcessing();
    }
  }
}

/**
  * @brief  This function is used to write in to device memory in non stretch mode.
  * @note   In this mode, when the write memory operation is executed the device
  *         send busy bytes to the host
  * @retval None.
  */
void OPENBL_I2C_NonStretchWriteMemory(void)
{
  uint32_t address;
  uint32_t xor;
  uint32_t counter;
  uint32_t codesize;
  uint8_t *p_ramaddress;
  uint8_t data;

  /* Check memory protection then send adequate response */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
  }
  else
  {
    OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

    /* Get the memory address */
    if (OPENBL_I2C_GetAddress(&address) == NACK_BYTE)
    {
      OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
    }
    else
    {
      OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

      /* Read number of bytes to be written and data */
      p_ramaddress = (uint8_t *)I2C_RAM_Buf;

      OPENBL_I2C_WaitAddress();

      /* Read the number of bytes to be written: Max number of data = data + 1 = 256 */
      data = OPENBL_I2C_ReadByte();

      /* Number of data to be written = data + 1 */
      codesize = (uint32_t)data + 1U;

      /* Checksum Initialization */
      xor = data;

      /* I2C receive data and send to RAM Buffer */
      for (counter = codesize; counter != 0U ; counter--)
      {
        data  = OPENBL_I2C_ReadByte();
        xor  ^= data;

        *(__IO uint8_t *)(p_ramaddress) = data;

        p_ramaddress++;
      }

      /* Send NACk if Checksum is incorrect */
      if (OPENBL_I2C_ReadByte() != xor)
      {
        OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
      }
      else
      {
        /* Send Busy Byte */
        OPENBL_Enable_BusyState_Sending();

        /* Write data to memory */
        OPENBL_MEM_Write(address, (uint8_t *)I2C_RAM_Buf, codesize);

        /* Send Busy Byte */
        OPENBL_Disable_BusyState_Sending();

        /* Send last Acknowledge synchronization byte */
        OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

        /* Start post processing task if needed */
        Common_StartPostProcessing();
      }
    }
  }
}

/**
  * @brief  This function is used to erase a memory in non stretch mode.
  * @note   In this mode, when the erase memory operation is executed the device
  *         send busy bytes to the host
  * @retval None.
  */
void OPENBL_I2C_NonStretchEraseMemory(void)
{
  uint32_t xor;
  uint32_t counter;
  uint32_t numpage;
  uint16_t data;
  ErrorStatus error_value;
  uint8_t status = ACK_BYTE;
  uint8_t *p_ramaddress;

  p_ramaddress = (uint8_t *) I2C_RAM_Buf;

  /* Check if the memory is not protected */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
  }
  else
  {
    OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

    /* Wait for address to match */
    OPENBL_I2C_WaitAddress();

    /* Read number of pages to be erased */
    data = OPENBL_I2C_ReadByte();
    data = (uint16_t)(data << 8) | OPENBL_I2C_ReadByte();

    /* Checksum initialization */
    xor  = ((uint32_t)data & 0xFF00U) >> 8;
    xor ^= (uint32_t)data & 0x00FFU;

    /* All commands in range 0xFFFZ are reserved for special erase features */
    if ((data & 0xFFF0U) == 0xFFF0U)
    {
      /* Check data integrity */
      if (OPENBL_I2C_ReadByte() != (uint8_t) xor)
      {
        OPENBL_I2C_WaitStop();
        status = NACK_BYTE;
      }
      else
      {
        OPENBL_I2C_WaitStop();

        /* Send Busy Byte */
        OPENBL_Enable_BusyState_Sending();

        if ((data == 0xFFFFU) || (data == 0xFFFEU) || (data == 0xFFFDU))
        {
          p_ramaddress[0] = (uint8_t)(data & 0x00FFU);
          p_ramaddress[1] = (uint8_t)((data & 0xFF00U) >> 8);

          error_value = OPENBL_MEM_MassErase(OPENBL_DEFAULT_MEM, (uint8_t *) I2C_RAM_Buf, I2C_RAM_BUFFER_SIZE);

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
    }
    else
    {
      /* Check data integrity */
      if (OPENBL_I2C_ReadByte() != (uint8_t) xor)
      {
        OPENBL_I2C_WaitStop();
        status = NACK_BYTE;
      }
      else
      {
        OPENBL_I2C_WaitStop();
        OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

        p_ramaddress = (uint8_t *) I2C_RAM_Buf;

        /* Number of pages to be erased (data + 1) */
        numpage = (uint32_t)data + 1U;

        *p_ramaddress = (uint8_t)(numpage & 0x00FFU);
        p_ramaddress++;

        *p_ramaddress = (uint8_t)((numpage & 0xFF00U) >> 8);
        p_ramaddress++;

        /* Wait for address to match */
        OPENBL_I2C_WaitAddress();

        /* Checksum Initialization */
        xor = 0U;

        /* Get the pages to be erased */
        for (counter = numpage; counter != 0U ; counter--)
        {
          /* Receive the MSB byte */
          data  = OPENBL_I2C_ReadByte();
          xor  ^= data;
          data  = (uint16_t)((data & 0x00FFU) << 8);

          /* Receive the LSB byte */
          data |= (uint8_t)(OPENBL_I2C_ReadByte() & 0x00FFU);
          xor  ^= ((uint32_t)data & 0x00FFU);

          /* Only store data that fit in the buffer length */
          if (counter < (I2C_RAM_BUFFER_SIZE / 2U))
          {
            *p_ramaddress = (uint8_t)(data & 0x00FFU);
            p_ramaddress++;

            *p_ramaddress = (uint8_t)((data & 0xFF00U) >> 8);
            p_ramaddress++;
          }
        }

        /* Check data integrity */
        if (OPENBL_I2C_ReadByte() != (uint8_t) xor)
        {
          OPENBL_I2C_WaitStop();
          status = NACK_BYTE;
        }
        else
        {
          OPENBL_I2C_WaitStop();

          /* Send Busy Byte */
          OPENBL_Enable_BusyState_Sending();

          error_value = OPENBL_MEM_Erase(OPENBL_DEFAULT_MEM, (uint8_t *) I2C_RAM_Buf, I2C_RAM_BUFFER_SIZE);

          /* Disable Busy Byte */
          OPENBL_Disable_BusyState_Sending();

          /* Errors from memory erase are not managed, always return ACK */
          if (error_value == SUCCESS)
          {
            status = ACK_BYTE;
          }
        }
      }
    }

    OPENBL_I2C_SendAcknowledgeByte(status);
  }
}

/**
  * @brief  This function is used to enable write protect in non stretch mode.
  * @note   In this mode, when enabling the write protection the device
  *         send busy bytes to the host
  * @retval None.
  */
void OPENBL_I2C_NonStretchWriteProtect(void)
{
  uint16_t counter;
  uint16_t length;
  uint8_t data;
  uint8_t xor;
  ErrorStatus error_value;
  uint8_t *p_ramaddress;

  /* Check if the memory is not protected */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
  }
  else
  {
    OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

    /* Wait for address to match */
    OPENBL_I2C_WaitAddress();

    /* Get the data length */
    data = OPENBL_I2C_ReadByte();
    xor  = ~data;

    /* Send NACk if Checksum is incorrect */
    if (OPENBL_I2C_ReadByte() != xor)
    {
      OPENBL_I2C_WaitStop();
      OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
    }
    else
    {
      OPENBL_I2C_WaitStop();
      OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

      p_ramaddress = (uint8_t *) I2C_RAM_Buf;
      length       = (uint16_t) data + 1U;

      /* Checksum Initialization */
      xor = 0U;

      /* Wait for address to match */
      OPENBL_I2C_WaitAddress();

      /* Receive data and write to RAM Buffer */
      for (counter = (length); counter != 0U ; counter--)
      {
        data  = OPENBL_I2C_ReadByte();
        xor  ^= data;

        *(__IO uint8_t *)(p_ramaddress) = (uint8_t) data;

        p_ramaddress++;
      }

      /* Check data integrity and send NACK if Checksum is incorrect */
      if (OPENBL_I2C_ReadByte() != (uint8_t) xor)
      {
        OPENBL_I2C_WaitStop();
        OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
      }
      else
      {
        OPENBL_I2C_WaitStop();
        p_ramaddress = (uint8_t *) I2C_RAM_Buf;

        /* Send Busy Byte */
        OPENBL_Enable_BusyState_Sending();

        /* Enable the write protection */
        error_value = OPENBL_MEM_SetWriteProtection(ENABLE, OPENBL_DEFAULT_MEM, p_ramaddress, length);

        /* Disable Busy Byte */
        OPENBL_Disable_BusyState_Sending();

        if (error_value == SUCCESS)
        {
          OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

          /* Start post processing task if needed */
          Common_StartPostProcessing();
        }
      }
    }
  }
}

/**
  * @brief  This function is used to disable write protect in non stretch mode.
  * @note   In this mode, when disabling the write protection the device
  *         send busy bytes to the host
  * @retval None.
  */
void OPENBL_I2C_NonStretchWriteUnprotect(void)
{
  ErrorStatus error_value;

  /* Check if the memory is not protected */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
  }
  else
  {
    OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

    /* Send Busy Byte */
    OPENBL_Enable_BusyState_Sending();

    /* Disable write protection */
    error_value = OPENBL_MEM_SetWriteProtection(DISABLE, OPENBL_DEFAULT_MEM, NULL, 0U);

    /* Disable Busy Byte */
    OPENBL_Disable_BusyState_Sending();

    OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

    if (error_value == SUCCESS)
    {
      /* Start post processing task if needed */
      Common_StartPostProcessing();
    }
  }
}

/**
  * @brief  This function is used to enable readout protection in non stretch mode.
  * @note   In this mode, when enabling the readout protection the device
  *         send busy bytes to the host
  * @retval None.
  */
void OPENBL_I2C_NonStretchReadoutProtect(void)
{
  /* Check memory protection then send adequate response */
  if (Common_GetProtectionStatus() != RESET)
  {
    OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
  }
  else
  {
    OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

    /* Send Busy Byte */
    OPENBL_Enable_BusyState_Sending();

    /* Enable the read protection */
    OPENBL_MEM_SetReadOutProtection(OPENBL_DEFAULT_MEM, ENABLE);

    /* Disable Busy Byte */
    OPENBL_Disable_BusyState_Sending();

    OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

    /* Start post processing task if needed */
    Common_StartPostProcessing();
  }
}

/**
  * @brief  This function is used to disable readout protection in non stretch mode.
  * @note   In this mode, when disabling the readout protection the device
  *         send busy bytes to the host.
  *         going from RDP level 1 to RDP level 0 erase all the flash,
  *         so the send of second acknowledge after Disabling the read protection
  *         is not possible what make the communication with the host get lost
  * @retval None.
  */
void OPENBL_I2C_NonStretchReadoutUnprotect(void)
{
  OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

  OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

  /* Send Busy Byte */
  OPENBL_Enable_BusyState_Sending();

  /* Disable the read protection */
  OPENBL_MEM_SetReadOutProtection(OPENBL_DEFAULT_MEM, DISABLE);

  /* Disable Busy Byte */
  OPENBL_Disable_BusyState_Sending();

  /* Start post processing task if needed */
  Common_StartPostProcessing();
}

/**
  * @brief  This function is used to get a valid address.
  * @retval Returns NACK status in case of error else returns ACK status.
  */
uint8_t OPENBL_I2C_GetAddress(uint32_t *pAddress)
{
  uint8_t data[4] = {0U, 0U, 0U, 0U};
  uint8_t status;
  uint8_t xor;

  /* Wait for address to match */
  OPENBL_I2C_WaitAddress();

  data[3] = OPENBL_I2C_ReadByte();
  data[2] = OPENBL_I2C_ReadByte();
  data[1] = OPENBL_I2C_ReadByte();
  data[0] = OPENBL_I2C_ReadByte();

  xor = data[3] ^ data[2] ^ data[1] ^ data[0];

  /* Check the integrity of received data */
  if (OPENBL_I2C_ReadByte() != xor)
  {
    status = NACK_BYTE;
  }
  else
  {
    /* Wait until STOP is detected */
    OPENBL_I2C_WaitStop();

    *pAddress = ((uint32_t)data[3] << 24) | ((uint32_t)data[2] << 16) | ((uint32_t)data[1] << 8) | (uint32_t)data[0];

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
  * @brief  This function is used to execute special command commands.
  * @retval None.
  */
void OPENBL_I2C_SpecialCommand(void)
{
  OPENBL_SpecialCmdTypeDef *special_cmd;
  uint16_t op_code;
  uint8_t xor;
  uint8_t index;
  uint8_t data;

  /* Point to the RAM I2C buffer to gain size and reliability */
  special_cmd = (OPENBL_SpecialCmdTypeDef *)(uint32_t) I2C_RAM_Buf;

  /* Send Operation code acknowledgment */
  OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

  /* Wait for address to match */
  OPENBL_I2C_WaitAddress();

  /* Get the command operation code */
  if (OPENBL_I2C_GetSpecialCmdOpCode(&op_code, OPENBL_SPECIAL_CMD) == NACK_BYTE)
  {
    OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
  }
  else
  {
    /* Send Operation code acknowledgment */
    OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

    /* Wait for address to match */
    OPENBL_I2C_WaitAddress();

    /* Initialize the special command frame */
    special_cmd->CmdType = OPENBL_SPECIAL_CMD;
    special_cmd->OpCode  = op_code;

    /* Initialize the xor variable */
    xor = 0U;

    /* Get the number of bytes to be received */
    /* Read the MSB of the size byte */
    data                     = OPENBL_I2C_ReadByte();
    special_cmd->SizeBuffer1 = ((uint16_t)data) << 8;
    xor                     ^= data;

    /* Read the LSB of the size byte */
    data                      = OPENBL_I2C_ReadByte();
    special_cmd->SizeBuffer1 |= (uint16_t)data;
    xor                      ^= data;

    if (special_cmd->SizeBuffer1 > SPECIAL_CMD_SIZE_BUFFER1)
    {
      OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
    }
    else
    {
      if (special_cmd->SizeBuffer1 != 0U)
      {
        /* Read received bytes */
        for (index = 0U; index < special_cmd->SizeBuffer1; index++)
        {
          data                        = OPENBL_I2C_ReadByte();
          special_cmd->Buffer1[index] = data;
          xor                        ^= data;
        }
      }

      /* Check data integrity */
      if (OPENBL_I2C_ReadByte() != xor)
      {
        OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
      }
      else
      {
        /* Send received size acknowledgment */
        OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

        /* Wait for address to match */
        OPENBL_I2C_WaitAddress();

        /* Process the special command */
        OPENBL_I2C_SpecialCommandProcess(special_cmd);

        /* NOTE: In case of any operation inside "SpecialCommandProcess" function that prevents the code
         * from returning to here (reset operation...), to be compatible with the OpenBL protocol,
         * the user must ensure sending the last ACK in the application side.
         */

        /* Send acknowledgment */
        OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

        /* Wait until NACK is detected */
        OPENBL_I2C_WaitNack();

        /* Wait until STOP is detected */
        OPENBL_I2C_WaitStop();
      }
    }
  }
}

/**
  * @brief  This function is used to execute extended special command commands.
  * @retval None.
  */
void OPENBL_I2C_ExtendedSpecialCommand(void)
{
  OPENBL_SpecialCmdTypeDef *special_cmd;
  uint16_t op_code;
  uint16_t index;
  uint8_t xor;
  uint8_t data;

  /* Point to the RAM I2C buffer to gain size and reliability */
  special_cmd = (OPENBL_SpecialCmdTypeDef *)(uint32_t) I2C_RAM_Buf;

  /* Send Operation code acknowledgment */
  OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

  /* Wait for address to match */
  OPENBL_I2C_WaitAddress();

  /* Get the command operation code */
  if (OPENBL_I2C_GetSpecialCmdOpCode(&op_code, OPENBL_EXTENDED_SPECIAL_CMD) == NACK_BYTE)
  {
    OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
  }
  else
  {
    /* Send Operation code acknowledgment */
    OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

    /* Wait for address to match */
    OPENBL_I2C_WaitAddress();

    /* Initialize the special command frame */
    special_cmd->CmdType = OPENBL_EXTENDED_SPECIAL_CMD;
    special_cmd->OpCode  = op_code;

    /* Initialize the xor variable */
    xor = 0U;

    /* Get the number of bytes to be received */
    /* Read the MSB of the size byte */
    data                     = OPENBL_I2C_ReadByte();
    special_cmd->SizeBuffer1 = ((uint16_t)data) << 8;
    xor                     ^= data;

    /* Read the LSB of the size byte */
    data                      = OPENBL_I2C_ReadByte();
    special_cmd->SizeBuffer1 |= (uint16_t)data;
    xor                      ^= data;

    if (special_cmd->SizeBuffer1 > SPECIAL_CMD_SIZE_BUFFER1)
    {
      OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
    }
    else
    {
      if (special_cmd->SizeBuffer1 != 0U)
      {
        /* Read received bytes */
        for (index = 0U; index < special_cmd->SizeBuffer1; index++)
        {
          data                        = OPENBL_I2C_ReadByte();
          special_cmd->Buffer1[index] = data;
          xor                        ^= data;
        }
      }

      /* Check data integrity */
      if (OPENBL_I2C_ReadByte() != xor)
      {
        OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
      }
      else
      {
        /* Send receive size acknowledgment */
        OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

        /* Wait for address to match */
        OPENBL_I2C_WaitAddress();

        /* Get the number of bytes to be written */
        /* Read the MSB of the size byte */
        xor                      = 0U;
        data                     = OPENBL_I2C_ReadByte();
        special_cmd->SizeBuffer2 = ((uint16_t)data) << 8;
        xor                     ^= data;

        /* Read the LSB of the size byte */
        data                      = OPENBL_I2C_ReadByte();
        special_cmd->SizeBuffer2 |= (uint16_t)data;
        xor                      ^= data;

        if (special_cmd->SizeBuffer2 > SPECIAL_CMD_SIZE_BUFFER2)
        {
          OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
        }
        else
        {
          if (special_cmd->SizeBuffer2 != 0U)
          {
            /* Read received bytes */
            for (index = 0U; index < special_cmd->SizeBuffer2; index++)
            {
              data                        = OPENBL_I2C_ReadByte();
              special_cmd->Buffer2[index] = data;
              xor                        ^= data;
            }
          }

          /* Check data integrity */
          if (OPENBL_I2C_ReadByte() != xor)
          {
            OPENBL_I2C_SendAcknowledgeByte(NACK_BYTE);
          }
          else
          {
            /* Send receive write size acknowledgment */
            OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

            /* Wait for address to match */
            OPENBL_I2C_WaitAddress();

            /* Process the special command */
            OPENBL_I2C_SpecialCommandProcess(special_cmd);

            /* NOTE: In case of any operation inside "SpecialCommandProcess" function that prevents the code
             * from returning to here (reset operation...), to be compatible with the OpenBL protocol,
             * the user must ensure sending the last ACK in the application side.
             */

            /* Send acknowledgment */
            OPENBL_I2C_SendAcknowledgeByte(ACK_BYTE);

            /* Wait until NACK is detected */
            OPENBL_I2C_WaitNack();

            /* Wait until STOP is detected */
            OPENBL_I2C_WaitStop();
          }
        }
      }
    }
  }
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  This function is used to construct the command List table.
  * @return Returns a table with all opcodes supported.
  */
static uint8_t OPENBL_I2C_ConstructCommandsTable(OPENBL_CommandsTypeDef *pI2cCmd)
{
  uint8_t i = 0U;

  if (pI2cCmd->GetCommand != NULL)
  {
    a_OPENBL_I2C_CommandsList[i] = CMD_GET_COMMAND;
    i++;
  }

  if (pI2cCmd->GetVersion != NULL)
  {
    a_OPENBL_I2C_CommandsList[i] = CMD_GET_VERSION;
    i++;
  }

  if (pI2cCmd->GetID != NULL)
  {
    a_OPENBL_I2C_CommandsList[i] = CMD_GET_ID;
    i++;
  }

  if (pI2cCmd->ReadMemory != NULL)
  {
    a_OPENBL_I2C_CommandsList[i] = CMD_READ_MEMORY;
    i++;
  }

  if (pI2cCmd->Go != NULL)
  {
    a_OPENBL_I2C_CommandsList[i] = CMD_GO;
    i++;
  }

  if (pI2cCmd->WriteMemory != NULL)
  {
    a_OPENBL_I2C_CommandsList[i] = CMD_WRITE_MEMORY;
    i++;
  }

  if (pI2cCmd->EraseMemory != NULL)
  {
    a_OPENBL_I2C_CommandsList[i] = CMD_EXT_ERASE_MEMORY;
    i++;
  }

  if (pI2cCmd->WriteProtect != NULL)
  {
    a_OPENBL_I2C_CommandsList[i] = CMD_WRITE_PROTECT;
    i++;
  }

  if (pI2cCmd->WriteUnprotect != NULL)
  {
    a_OPENBL_I2C_CommandsList[i] = CMD_WRITE_UNPROTECT;
    i++;
  }

  if (pI2cCmd->ReadoutProtect != NULL)
  {
    a_OPENBL_I2C_CommandsList[i] = CMD_READ_PROTECT;
    i++;
  }

  if (pI2cCmd->ReadoutUnprotect != NULL)
  {
    a_OPENBL_I2C_CommandsList[i] = CMD_READ_UNPROTECT;
    i++;
  }

  if (pI2cCmd->NsWriteMemory != NULL)
  {
    a_OPENBL_I2C_CommandsList[i] = CMD_NS_WRITE_MEMORY;
    i++;
  }

  if (pI2cCmd->NsEraseMemory != NULL)
  {
    a_OPENBL_I2C_CommandsList[i] = CMD_NS_ERASE_MEMORY;
    i++;
  }

  if (pI2cCmd->NsWriteProtect != NULL)
  {
    a_OPENBL_I2C_CommandsList[i] = CMD_NS_WRITE_PROTECT;
    i++;
  }

  if (pI2cCmd->NsWriteUnprotect != NULL)
  {
    a_OPENBL_I2C_CommandsList[i] = CMD_NS_WRITE_UNPROTECT;
    i++;
  }

  if (pI2cCmd->NsReadoutProtect != NULL)
  {
    a_OPENBL_I2C_CommandsList[i] = CMD_NS_READ_PROTECT;
    i++;
  }

  if (pI2cCmd->NsReadoutUnprotect != NULL)
  {
    a_OPENBL_I2C_CommandsList[i] = CMD_NS_READ_UNPROTECT;
    i++;
  }

  if (pI2cCmd->SpecialCommand != NULL)
  {
    a_OPENBL_I2C_CommandsList[i] = CMD_SPECIAL_COMMAND;
    i++;
  }

  if (pI2cCmd->ExtendedSpecialCommand != NULL)
  {
    a_OPENBL_I2C_CommandsList[i] = CMD_EXTENDED_SPECIAL_COMMAND;
    i++;
  }

  return (i);
}

/**
  * @brief  This function is used to get the operation code.
  * @param  OpCode Pointer to the operation code to be returned.
  * @param  CmdType Type of the command, Special write or extended special command.
  * @retval Returns NACK status in case of error else returns ACK status.
  */
uint8_t OPENBL_I2C_GetSpecialCmdOpCode(uint16_t *OpCode, OPENBL_SpecialCmdTypeTypeDef CmdType)
{
  uint8_t op_code[2];
  uint8_t xor;
  uint8_t status;
  uint8_t index;

  /* Initialize the status variable */
  status = NACK_BYTE;

  /* Get the command OpCode (2 bytes) */
  op_code[0] = OPENBL_I2C_ReadByte(); /* Read the MSB byte */
  op_code[1] = OPENBL_I2C_ReadByte(); /* Read the LSB byte */

  /* Get the checksum */
  xor  = op_code[0];
  xor ^= op_code[1];

  if (OPENBL_I2C_ReadByte() != xor)
  {
    status = NACK_BYTE;
  }
  else
  {
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
    else if (CmdType == OPENBL_EXTENDED_SPECIAL_CMD)
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
    else
    {
      status = NACK_BYTE;
    }
  }

  return status;
}
