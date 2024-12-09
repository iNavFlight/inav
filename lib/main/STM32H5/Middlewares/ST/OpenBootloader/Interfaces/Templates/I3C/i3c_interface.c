/**
  ******************************************************************************
  * @file    i3c_interface.c
  * @author  MCD Application Team
  * @brief   Contains I3C HW configuration
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
#include "platform.h"
#include "interfaces_conf.h"

#include "openbl_core.h"
#include "openbl_i3c_cmd.h"

#include "i3c_interface.h"
#include "iwdg_interface.h"
#include "interfaces_conf.h"
#include "flash_interface.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define AVAL_TIMING                     0xFFU
#define FREE_TIMING                     0x3FU
#define OPENBL_I3C_SYNC_BYTE            0x5AU

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t I3cDetected = 0U;

/* Exported variables --------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void OPENBL_I3C_Init(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  This function is used to initialize the used I3C instance.
  * @retval None.
  */
static void OPENBL_I3C_Init(void)
{
}

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  This function is used to configure I3C PINs and then initialize the used I3C instance.
  * @retval None.
  */
void OPENBL_I3C_Configuration(void)
{
}

/**
  * @brief  This function is used to De-initialize the I3C PINs and instance.
  * @retval None.
  */
void OPENBL_I3C_DeInit(void)
{
}

/**
  * @brief  This function is used to detect if there is any activity on I3C protocol.
  * @retval Returns 1 if the interface is detected else 0.
  */
uint8_t OPENBL_I3C_ProtocolDetection(void)
{
  return I3cDetected;
}

/**
  * @brief  This function is used to get the command opcode from the host.
  * @retval Returns the command opcode value.
  */
uint8_t OPENBL_I3C_GetCommandOpcode(void)
{
}

/**
  * @brief  This function is used to read one byte from I3C pipe.
  * @retval Returns the read byte.
  */
uint8_t OPENBL_I3C_ReadByte(void)
{
  return LL_I3C_ReceiveData8(I3Cx);
}

/**
  * @brief  This function is used to send one byte through I3C pipe.
  * @param  Byte The byte to be sent.
  * @retval None.
  */
void OPENBL_I3C_SendByte(uint8_t Byte)
{
}

/**
  * @brief  This function is used to send Acknowledgment.
  * @param  Acknowledge The acknowledge byte to be sent.
  * @retval None.
  */
void OPENBL_I3C_SendAcknowledgeByte(uint8_t Acknowledge)
{
}

/**
  * @brief  This function is used to send a buffer using I3C.
  * @param  pBuffer The buffer that contains the data to be sent.
  * @param  BufferSize The size of the data to be sent.
  * @retval None.
  */
void OPENBL_I3C_SendBytes(uint8_t *pBuffer, uint32_t BufferSize)
{
}

/**
  * @brief  This function is used to read bytes from I3C pipe.
  * @param  pBuffer The buffer that stores the received data.
  * @param  BufferSize The number of bytes to be read and stored in the receive buffer.
  * @retval None.
  */
void OPENBL_I3C_ReadBytes(uint8_t *pBuffer, uint32_t BufferSize)
{
}

/**
  * @brief  This function is used to process and execute the special commands.
  *         The user must define the special commands routine here.
  * @param  SpecialCmd Pointer to the OPENBL_SpecialCmdTypeDef structure.
  * @retval Returns NACK status in case of error else returns ACK status.
  */
void OPENBL_I3C_SpecialCommandProcess(OPENBL_SpecialCmdTypeDef *SpecialCmd)
{
}

/**
  * @brief  Handle I3C interrupt request. This handler is used to detect host communication.
  *         It is only used during connection phase.
  * @retval None.
  */
void OPENBL_I3C_IRQHandler(void)
{
}
