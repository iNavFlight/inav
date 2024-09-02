/**
  ******************************************************************************
  * @file    openbl_core.c
  * @author  MCD Application Team
  * @brief   Open Bootloader core file
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
#include "openbl_core.h"
#include "app_openbootloader.h"
#include <stdbool.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint32_t NumberOfInterfaces = 0U;
static OPENBL_HandleTypeDef a_InterfacesTable[INTERFACES_SUPPORTED];
static OPENBL_HandleTypeDef *p_Interface;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
  * @brief  This function is used to initialize the registered interfaces in the Open Bootloader MW.
  * @retval None.
  */
void OPENBL_Init(void)
{
  uint32_t counter;

  for (counter = 0U; counter < NumberOfInterfaces; counter++)
  {
    if (a_InterfacesTable[counter].p_Ops->Init != NULL)
    {
      a_InterfacesTable[counter].p_Ops->Init();
    }
  }
}

/**
  * @brief  This function is used to de-initialize the Open Bootloader MW.
  * @retval None.
  */
void OPENBL_DeInit(void)
{
  OpenBootloader_DeInit();
}

/**
  * @brief  This function is used to de-initialize the registered interfaces in the Open Bootloader MW.
  * @retval None.
  */
void OPENBL_InterfacesDeInit(void)
{
  uint32_t counter;

  for (counter = 0U; counter < NumberOfInterfaces; counter++)
  {
    if (a_InterfacesTable[counter].p_Ops->DeInit != NULL)
    {
      a_InterfacesTable[counter].p_Ops->DeInit();
    }
  }
}

/**
  * @brief  This function is used to register a given interface in the Open Bootloader MW.
  * @retval None.
  */
ErrorStatus OPENBL_RegisterInterface(OPENBL_HandleTypeDef *Interface)
{
  ErrorStatus status = SUCCESS;

  if (NumberOfInterfaces < INTERFACES_SUPPORTED)
  {
    a_InterfacesTable[NumberOfInterfaces].p_Ops = Interface->p_Ops;
    a_InterfacesTable[NumberOfInterfaces].p_Cmd = Interface->p_Cmd;

    NumberOfInterfaces++;
  }
  else
  {
    status = ERROR;
  }

  return status;
}

/**
  * @brief  This function is used to detect if there is any activity on a given interface.
  * @retval None.
  */
uint32_t OPENBL_InterfaceDetection(void)
{
  uint32_t counter;
  uint8_t detected = 0U;

  for (counter = 0U; counter < NumberOfInterfaces; counter++)
  {
    if (a_InterfacesTable[counter].p_Ops->Detection != NULL)
    {
      detected = a_InterfacesTable[counter].p_Ops->Detection();

      if (detected == 1U)
      {
        p_Interface = &(a_InterfacesTable[counter]);
        break;
      }
    }
  }

  return detected;
}

/**
  * @brief  This function is used to get the command opcode from the given interface and execute the right command.
  * @retval None.
  */
void OPENBL_CommandProcess(void)
{
  uint8_t command_opcode;

  /* Get the user command opcode */
  if (p_Interface->p_Ops->GetCommandOpcode != NULL)
  {
    command_opcode = p_Interface->p_Ops->GetCommandOpcode();

    switch (command_opcode)
    {
      case CMD_GET_COMMAND:
        if (p_Interface->p_Cmd->GetCommand != NULL)
        {
          p_Interface->p_Cmd->GetCommand();
        }
        else
        {
          if (p_Interface->p_Ops->SendByte != NULL)
          {
            p_Interface->p_Ops->SendByte(NACK_BYTE);
          }
        }
        break;

      case CMD_GET_VERSION:
        if (p_Interface->p_Cmd->GetVersion != NULL)
        {
          p_Interface->p_Cmd->GetVersion();
        }
        else
        {
          if (p_Interface->p_Ops->SendByte != NULL)
          {
            p_Interface->p_Ops->SendByte(NACK_BYTE);
          }
        }
        break;

      case CMD_GET_ID:
        if (p_Interface->p_Cmd->GetID != NULL)
        {
          p_Interface->p_Cmd->GetID();
        }
        else
        {
          if (p_Interface->p_Ops->SendByte != NULL)
          {
            p_Interface->p_Ops->SendByte(NACK_BYTE);
          }
        }
        break;

      case CMD_READ_MEMORY:
        if (p_Interface->p_Cmd->ReadMemory != NULL)
        {
          p_Interface->p_Cmd->ReadMemory();
        }
        else
        {
          if (p_Interface->p_Ops->SendByte != NULL)
          {
            p_Interface->p_Ops->SendByte(NACK_BYTE);
          }
        }
        break;

      case CMD_WRITE_MEMORY:
        if (p_Interface->p_Cmd->WriteMemory != NULL)
        {
          p_Interface->p_Cmd->WriteMemory();
        }
        else
        {
          if (p_Interface->p_Ops->SendByte != NULL)
          {
            p_Interface->p_Ops->SendByte(NACK_BYTE);
          }
        }
        break;

      case CMD_GO:
        if (p_Interface->p_Cmd->Go != NULL)
        {
          p_Interface->p_Cmd->Go();
        }
        else
        {
          if (p_Interface->p_Ops->SendByte != NULL)
          {
            p_Interface->p_Ops->SendByte(NACK_BYTE);
          }
        }
        break;

      case CMD_READ_PROTECT:
        if (p_Interface->p_Cmd->ReadoutProtect != NULL)
        {
          p_Interface->p_Cmd->ReadoutProtect();
        }
        else
        {
          if (p_Interface->p_Ops->SendByte != NULL)
          {
            p_Interface->p_Ops->SendByte(NACK_BYTE);
          }
        }
        break;

      case CMD_READ_UNPROTECT:
        if (p_Interface->p_Cmd->ReadoutUnprotect != NULL)
        {
          p_Interface->p_Cmd->ReadoutUnprotect();
        }
        else
        {
          if (p_Interface->p_Ops->SendByte != NULL)
          {
            p_Interface->p_Ops->SendByte(NACK_BYTE);
          }
        }
        break;

      case CMD_EXT_ERASE_MEMORY:
        if (p_Interface->p_Cmd->EraseMemory != NULL)
        {
          p_Interface->p_Cmd->EraseMemory();
        }
        else
        {
          if (p_Interface->p_Ops->SendByte != NULL)
          {
            p_Interface->p_Ops->SendByte(NACK_BYTE);
          }
        }
        break;

      case CMD_LEG_ERASE_MEMORY:
        if (p_Interface->p_Cmd->EraseMemory != NULL)
        {
          p_Interface->p_Cmd->EraseMemory();
        }
        else
        {
          if (p_Interface->p_Ops->SendByte != NULL)
          {
            p_Interface->p_Ops->SendByte(NACK_BYTE);
          }
        }
        break;

      case CMD_WRITE_PROTECT:
        if (p_Interface->p_Cmd->WriteProtect != NULL)
        {
          p_Interface->p_Cmd->WriteProtect();
        }
        else
        {
          if (p_Interface->p_Ops->SendByte != NULL)
          {
            p_Interface->p_Ops->SendByte(NACK_BYTE);
          }
        }
        break;

      case CMD_WRITE_UNPROTECT:
        if (p_Interface->p_Cmd->WriteUnprotect != NULL)
        {
          p_Interface->p_Cmd->WriteUnprotect();
        }
        else
        {
          if (p_Interface->p_Ops->SendByte != NULL)
          {
            p_Interface->p_Ops->SendByte(NACK_BYTE);
          }
        }
        break;

      case CMD_NS_WRITE_MEMORY:
        if (p_Interface->p_Cmd->NsWriteMemory != NULL)
        {
          p_Interface->p_Cmd->NsWriteMemory();
        }
        else
        {
          if (p_Interface->p_Ops->SendByte != NULL)
          {
            p_Interface->p_Ops->SendByte(NACK_BYTE);
          }
        }
        break;

      case CMD_NS_ERASE_MEMORY:
        if (p_Interface->p_Cmd->NsEraseMemory != NULL)
        {
          p_Interface->p_Cmd->NsEraseMemory();
        }
        else
        {
          if (p_Interface->p_Ops->SendByte != NULL)
          {
            p_Interface->p_Ops->SendByte(NACK_BYTE);
          }
        }
        break;

      case CMD_NS_WRITE_PROTECT:
        if (p_Interface->p_Cmd->NsWriteProtect != NULL)
        {
          p_Interface->p_Cmd->NsWriteProtect();
        }
        else
        {
          if (p_Interface->p_Ops->SendByte != NULL)
          {
            p_Interface->p_Ops->SendByte(NACK_BYTE);
          }
        }
        break;

      case CMD_NS_WRITE_UNPROTECT:
        if (p_Interface->p_Cmd->NsWriteUnprotect != NULL)
        {
          p_Interface->p_Cmd->NsWriteUnprotect();
        }
        else
        {
          if (p_Interface->p_Ops->SendByte != NULL)
          {
            p_Interface->p_Ops->SendByte(NACK_BYTE);
          }
        }
        break;

      case CMD_NS_READ_PROTECT:
        if (p_Interface->p_Cmd->NsReadoutProtect != NULL)
        {
          p_Interface->p_Cmd->NsReadoutProtect();
        }
        else
        {
          if (p_Interface->p_Ops->SendByte != NULL)
          {
            p_Interface->p_Ops->SendByte(NACK_BYTE);
          }
        }
        break;

      case CMD_NS_READ_UNPROTECT:
        if (p_Interface->p_Cmd->NsReadoutUnprotect != NULL)
        {
          p_Interface->p_Cmd->NsReadoutUnprotect();
        }
        else
        {
          if (p_Interface->p_Ops->SendByte != NULL)
          {
            p_Interface->p_Ops->SendByte(NACK_BYTE);
          }
        }
        break;

      case CMD_SPEED:
        if (p_Interface->p_Cmd->Speed != NULL)
        {
          p_Interface->p_Cmd->Speed();
        }
        else
        {
          if (p_Interface->p_Ops->SendByte != NULL)
          {
            p_Interface->p_Ops->SendByte(NACK_BYTE);
          }
        }
        break;

      case CMD_SPECIAL_COMMAND:
        if (p_Interface->p_Cmd->SpecialCommand != NULL)
        {
          p_Interface->p_Cmd->SpecialCommand();
        }
        else
        {
          if (p_Interface->p_Ops->SendByte != NULL)
          {
            p_Interface->p_Ops->SendByte(NACK_BYTE);
          }
        }
        break;

      case CMD_EXTENDED_SPECIAL_COMMAND:
        if (p_Interface->p_Cmd->ExtendedSpecialCommand != NULL)
        {
          p_Interface->p_Cmd->ExtendedSpecialCommand();
        }
        else
        {
          if (p_Interface->p_Ops->SendByte != NULL)
          {
            p_Interface->p_Ops->SendByte(NACK_BYTE);
          }
        }
        break;

      /* Unknown command opcode */
      default:
        if (p_Interface->p_Ops->SendByte != NULL)
        {
          p_Interface->p_Ops->SendByte(NACK_BYTE);
        }
        break;
    }
  }
}
