/**
  ******************************************************************************
  * @file    fdcan_interface.c
  * @author  MCD Application Team
  * @brief   Contains FDCAN HW configuration
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
#include "platform.h"
#include "interfaces_conf.h"
#include "openbl_core.h"
#include "openbl_fdcan_cmd.h"
#include "fdcan_interface.h"
#include "iwdg_interface.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static FDCAN_HandleTypeDef hfdcan;
static FDCAN_FilterTypeDef sFilterConfig;
static FDCAN_TxHeaderTypeDef TxHeader;
static FDCAN_RxHeaderTypeDef RxHeader;
static uint8_t FdcanDetected = 0U;

/* Exported variables --------------------------------------------------------*/
uint8_t TxData[FDCAN_RAM_BUFFER_SIZE];
uint8_t RxData[FDCAN_RAM_BUFFER_SIZE];

/* Private function prototypes -----------------------------------------------*/
static void OPENBL_FDCAN_Init(void);

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  This function is used to initialize the used FDCAN instance.
  * @retval None.
  */
static void OPENBL_FDCAN_Init(void)
{
}

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  This function is used to configure FDCAN pins and then initialize the used FDCAN instance.
  * @retval None.
  */
void OPENBL_FDCAN_Configuration(void)
{
}

/**
  * @brief  This function is used to De-initialize the FDCAN pins and instance.
  * @retval None.
  */
void OPENBL_FDCAN_DeInit(void)
{
}

/**
  * @brief  This function is used to detect if there is any activity on FDCAN protocol.
  * @retval Returns 1 if interface is detected else 0..
  */
uint8_t OPENBL_FDCAN_ProtocolDetection(void)
{
  return FdcanDetected;
}

/**
  * @brief  This function is used to get the command opcode from the host.
  * @retval Returns the command.
  */
uint8_t OPENBL_FDCAN_GetCommandOpcode(void)
{
  uint8_t command_opc = 0x0;

  return command_opc;
}

/**
  * @brief  This function is used to read one byte from FDCAN pipe.
  * @retval Returns the read byte.
  */
uint8_t OPENBL_FDCAN_ReadByte(void)
{
  uint8_t byte;

  return byte;
}

/**
  * @brief  This function is used to read bytes from FDCAN pipe.
  * @retval None.
  */
void OPENBL_FDCAN_ReadBytes(uint8_t *Buffer, uint32_t BufferSize)
{
}

/**
  * @brief  This function is used to send one byte through FDCAN pipe.
  * @param  Byte The byte to be sent.
  * @retval None.
  */
void OPENBL_FDCAN_SendByte(uint8_t Byte)
{
}

/**
  * @brief  This function is used to send a buffer using FDCAN.
  * @param  Buffer The data buffer to be sent.
  * @param  BufferSize The size of the data buffer to be sent.
  * @retval None.
  */
void OPENBL_FDCAN_SendBytes(uint8_t *Buffer, uint32_t BufferSize)
{
}

/**
  * @brief  This function is used to process and execute the special commands.
  *         The user must define the special commands routine here.
  * @retval Returns NACK status in case of error else returns ACK status.
  */
void OPENBL_FDCAN_SpecialCommandProcess(OPENBL_SpecialCmdTypeDef *Frame)
{
}
