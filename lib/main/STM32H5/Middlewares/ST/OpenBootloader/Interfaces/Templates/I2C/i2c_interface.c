/**
  ******************************************************************************
  * @file    i2c_interface.c
  * @author  MCD Application Team
  * @brief   Contains I2C HW configuration
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
#include "openbl_i2c_cmd.h"
#include "i2c_interface.h"
#include "iwdg_interface.h"
#include "flash_interface.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t I2cDetected = 0;

/* Exported variables --------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void OPENBL_I2C_Init(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  This function is used to initialize the used I2C instance.
  * @retval None.
  */
static void OPENBL_I2C_Init(void)
{
}

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  This function is used to configure I2C pins and then initialize the used I2C instance.
  * @retval None.
  */
void OPENBL_I2C_Configuration(void)
{
}

/**
  * @brief  This function is used to De-initialize the I2C pins and instance.
  * @retval None.
  */
void OPENBL_I2C_DeInit(void)
{
}

/**
  * @brief  This function is used to detect if there is any activity on I2C protocol.
  * @retval None.
  */
uint8_t OPENBL_I2C_ProtocolDetection(void)
{
  return I2cDetected;
}

/**
  * @brief  This function is used to get the command opcode from the host.
  * @retval Returns the command.
  */
uint8_t OPENBL_I2C_GetCommandOpcode(void)
{
  uint8_t command_opc = 0x0U;

  return command_opc;
}

/**
  * @brief  This function is used to read one byte from I2C pipe.
  * @retval Returns the read byte.
  */
uint8_t OPENBL_I2C_ReadByte(void)
{
  uint32_t timeout = 0U;

  return LL_I2C_ReceiveData8(I2Cx);
}

/**
  * @brief  This function is used to send one byte through I2C pipe.
  * @param  Byte The byte to be sent.
  * @retval None.
  */
void OPENBL_I2C_SendByte(uint8_t Byte)
{
}

/**
  * @brief  This function is used to wait until the address is matched.
  * @retval None.
  */
void OPENBL_I2C_WaitAddress(void)
{
}

/**
  * @brief  This function is used to wait until NACK is detected.
  * @retval None.
  */
#if defined (__ICCARM__)
__ramfunc void OPENBL_I2C_WaitNack(void)
#else
__attribute__((section(".ramfunc"))) void OPENBL_I2C_WaitNack(void)
#endif /* (__ICCARM__) */
{
}

/**
  * @brief  This function is used to wait until STOP is detected.
  * @retval None.
  */
#if defined (__ICCARM__)
__ramfunc void OPENBL_I2C_WaitStop(void)
#else
__attribute__((section(".ramfunc"))) void OPENBL_I2C_WaitStop(void)
#endif /* (__ICCARM__) */
{
}

/**
  * @brief  This function is used to send Acknowledgment.
  * @retval None.
  */
void OPENBL_I2C_SendAcknowledgeByte(uint8_t Byte)
{
}

/**
  * @brief  This function is used to send busy byte through I2C pipe.
  * @param
  * @retval None.
  */
#if defined (__ICCARM__)
__ramfunc void OPENBL_I2C_SendBusyByte(void)
#else
__attribute__((section(".ramfunc"))) void OPENBL_I2C_SendBusyByte(void)
#endif /* (__ICCARM__) */
{
}

/**
  * @brief  This function is used to process and execute the special commands.
  *         The user must define the special commands routine here.
  * @param  SpecialCmd Pointer to the OPENBL_SpecialCmdTypeDef structure.
  * @retval Returns NACK status in case of error else returns ACK status.
  */
void OPENBL_I2C_SpecialCommandProcess(OPENBL_SpecialCmdTypeDef *SpecialCmd)
{
}

/**
  * @brief  This function is used to Set Flash busy state variable to activate busy state sending
  *         during flash operations
  * @retval None.
  */
void OPENBL_Enable_BusyState_Sending(void)
{
}

/**
  * @brief  This function is used to disable the send of busy state in I2C non stretch mode.
  * @retval None.
  */
void OPENBL_Disable_BusyState_Sending(void)
{
}
