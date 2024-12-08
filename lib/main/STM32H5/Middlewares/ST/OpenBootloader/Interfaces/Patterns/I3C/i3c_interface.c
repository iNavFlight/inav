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
  /* Enable Mode Peripheral I3C Controller */
  LL_I3C_SetMode(I3Cx, LL_I3C_MODE_TARGET);

  /* Configure the Bus and Free timing */
  LL_I3C_SetAvalTiming(I3Cx, AVAL_TIMING);
  LL_I3C_SetFreeTiming(I3Cx, FREE_TIMING);

  /* Enable IBI procedure */
  LL_I3C_EnableIBI(I3Cx);

  /* Configure one additional data byte after IBI acknowledge */
  LL_I3C_ConfigNbIBIAddData(I3Cx, LL_I3C_PAYLOAD_1_BYTE);

  /* Configure the Device IBI Payload */
  LL_I3C_SetDeviceIBIPayload(I3Cx, LL_I3C_IBI_ADDITIONAL_DATA);

  /* Enable I3C peripheral */
  LL_I3C_Enable(I3Cx);

  /* Enable Receive Fifo Interrupt */
  LL_I3C_EnableIT_RXFNE(I3Cx);

  NVIC_SetPriority(I3Cx_EV_IRQ, 5U); /* Set priority for I3Cx_EV_IRQ */
  NVIC_EnableIRQ(I3Cx_EV_IRQ);       /* Enable I3Cx_EV_IRQ */
}

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  This function is used to configure I3C PINs and then initialize the used I3C instance.
  * @retval None.
  */
void OPENBL_I3C_Configuration(void)
{
  /*      +--------------+
          |     I3C1     |
  +-------+--------------+
  |  SCL  |     PH11     |
  +-------+--------------+
  |  SDA  |     PH12     |
  +-------+--------------+ */

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0U};

  /* Enable used GPIO clocks for SDA and SCL PINs */
  I3Cx_GPIO_CLK_SCL_ENABLE();
  I3Cx_GPIO_CLK_SDA_ENABLE();

  /* Enable I3Cx clock */
  I3Cx_CLK_ENABLE();

  /* Configure I3Cx SCL as alternate function */
  GPIO_InitStruct.Pin        = I3Cx_SCL_PIN;
  GPIO_InitStruct.Mode       = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Pull       = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Alternate  = I3Cx_ALTERNATE;

  (void)LL_GPIO_Init(I3Cx_SCL_PORT, &GPIO_InitStruct);

  /* Configure I3Cx SDA as alternate function */
  GPIO_InitStruct.Pin = I3Cx_SDA_PIN;

  (void) LL_GPIO_Init(I3Cx_SDA_PORT, &GPIO_InitStruct);

  OPENBL_I3C_Init();
}

/**
  * @brief  This function is used to De-initialize the I3C PINs and instance.
  * @retval None.
  */
void OPENBL_I3C_DeInit(void)
{
  /* Only de-initialize the I3C if it is not the current detected interface */
  if (I3cDetected == 0U)
  {
    LL_I3C_Disable(I3Cx);

    I3Cx_DEINIT();
  }
}

/**
  * @brief  This function is used to detect if there is any activity on I3C protocol.
  * @retval Returns 1 if the interface is detected else 0.
  */
uint8_t OPENBL_I3C_ProtocolDetection(void)
{
  /* Check the status of Error flag */
  if (LL_I3C_IsActiveFlag_ERR(I3Cx) == 1U)
  {
    /* Clear Error flag */
    LL_I3C_ClearFlag_ERR(I3Cx);
  }

  /* Check the status of Dynamic Address Update flag */
  if (LL_I3C_IsActiveFlag_DAUPD(I3Cx) == 1U)
  {
    /* Clear Pending flag */
    LL_I3C_ClearFlag_DAUPD(I3Cx);
  }

  if (I3cDetected == 1U)
  {
    /* Acknowledge the host that the connection is done */
    OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

    LL_I3C_DisableIT_RXFNE(I3Cx);
  }

  return I3cDetected;
}

/**
  * @brief  This function is used to get the command opcode from the host.
  * @retval Returns the command opcode value.
  */
uint8_t OPENBL_I3C_GetCommandOpcode(void)
{
  uint32_t timeout  = OPENBL_I3C_TIMEOUT;
  uint32_t count    = 0U;
  uint8_t buffer[2] = {0U};

  while (count != 2U)
  {
    if (LL_I3C_IsActiveFlag_RXFNE(I3Cx) == 1U)
    {
      buffer[count] = LL_I3C_ReceiveData8(I3Cx);
      count++;
    }
  }

  /* Wait the end of frame */
  while (LL_I3C_IsActiveFlag_FC(I3Cx) == 0U)
  {
    OPENBL_IWDG_Refresh();

    if (timeout == 0U)
    {
      NVIC_SystemReset();
    }

    timeout--;
  }

  /* Clear Frame Complete flag */
  LL_I3C_ClearFlag_FC(I3Cx);

  /* Check data integrity using XOR value */
  if ((buffer[0] ^ buffer[1]) != 0xFFU)
  {
    /* Communication error */
    return ERROR_COMMAND;
  }
  else
  {
    /* Return the command opcode */
    return buffer[0];
  }
}

/**
  * @brief  This function is used to read one byte from I3C pipe.
  * @retval Returns the read byte.
  */
uint8_t OPENBL_I3C_ReadByte(void)
{
  uint32_t timeout = 0U;

  while (LL_I3C_IsActiveFlag_RXFNE(I3Cx) == 0U)
  {
    OPENBL_IWDG_Refresh();

    if ((timeout++) >= OPENBL_I3C_TIMEOUT)
    {
      NVIC_SystemReset();
    }
  }

  /* Clear Frame Complete flag */
  LL_I3C_ClearFlag_FC(I3Cx);

  return LL_I3C_ReceiveData8(I3Cx);
}

/**
  * @brief  This function is used to send one byte through I3C pipe.
  * @param  Byte The byte to be sent.
  * @retval None.
  */
void OPENBL_I3C_SendByte(uint8_t Byte)
{
  uint32_t timeout = OPENBL_I3C_TIMEOUT;

  /* Send the byte */
  LL_I3C_TransmitData8(I3Cx, Byte);

  /* Wait till the end of frame or timeout */
  while (LL_I3C_IsActiveFlag_FC(I3Cx) == 0U)
  {
    OPENBL_IWDG_Refresh();

    if (timeout == 0U)
    {
      NVIC_SystemReset();
    }

    timeout--;
  }

  /* Clear the flag */
  LL_I3C_ClearFlag_FC(I3Cx);
}

/**
  * @brief  This function is used to send Acknowledgment.
  * @param  Acknowledge The acknowledge byte to be sent.
  * @retval None.
  */
void OPENBL_I3C_SendAcknowledgeByte(uint8_t Acknowledge)
{
  uint32_t timeout = OPENBL_I3C_TIMEOUT;

  /* Set the acknowledge byte in the IBI payload */
  LL_I3C_SetIBIPayload(I3Cx, Acknowledge);

  /* Configure the device to send IBI message */
  LL_I3C_TargetHandleMessage(I3Cx, LL_I3C_TARGET_MTYPE_IBI, 1U);

  /* Wait IBI completion confirmation flag */
  while (LL_I3C_IsActiveFlag_IBIEND(I3Cx) != 1U)
  {
    OPENBL_IWDG_Refresh();

    if (timeout == 0U)
    {
      /* System Reset */
      NVIC_SystemReset();
    }

    timeout--;
  }

  /* Clear IBI completion confirmation flag */
  LL_I3C_ClearFlag_IBIEND(I3Cx);

  /* Clear the Frame Complete flag */
  LL_I3C_ClearFlag_FC(I3Cx);
}

/**
  * @brief  This function is used to send a buffer using I3C.
  * @param  pBuffer The buffer that contains the data to be sent.
  * @param  BufferSize The size of the data to be sent.
  * @retval None.
  */
void OPENBL_I3C_SendBytes(uint8_t *pBuffer, uint32_t BufferSize)
{
  uint32_t count;
  uint32_t timeout = OPENBL_I3C_TIMEOUT;

  /* Configure the preload data to emit into TX FIFO in target mode */
  LL_I3C_ConfigTxPreload(I3Cx, (uint16_t)BufferSize);

  for (count = 0U; count < BufferSize; ++count)
  {
    while (LL_I3C_IsActiveFlag_TXFNF(I3Cx) != 1U)
    {
      OPENBL_IWDG_Refresh();

      if (timeout == 0U)
      {
        NVIC_SystemReset();
      }
      else
      {
        --timeout;
      }
    }

    /* Send one Byte of data */
    LL_I3C_TransmitData8(I3Cx, pBuffer[count]);

    /* Reset timeout value */
    timeout = OPENBL_I3C_TIMEOUT;
  }

  /* Wait the end of frame */
  while (LL_I3C_IsActiveFlag_FC(I3Cx) == 0U)
  {
    OPENBL_IWDG_Refresh();

    if (timeout == 0U)
    {
      NVIC_SystemReset();
    }

    timeout--;
  }

  /* Clear the Frame Complete flag */
  LL_I3C_ClearFlag_FC(I3Cx);
}

/**
  * @brief  This function is used to read bytes from I3C pipe.
  * @param  pBuffer The buffer that stores the received data.
  * @param  BufferSize The number of bytes to be read and stored in the receive buffer.
  * @retval None.
  */
void OPENBL_I3C_ReadBytes(uint8_t *pBuffer, uint32_t BufferSize)
{
  uint32_t count;
  uint32_t timeout;

  /* Loop until we receive the intended number of bytes */
  for (count = 0U; count < BufferSize; count++)
  {
    timeout = OPENBL_I3C_TIMEOUT;

    /* Wait till Rx flag detection or timeout */
    while (LL_I3C_IsActiveFlag_RXFNE(I3Cx) != 1U)
    {
      OPENBL_IWDG_Refresh();

      if (timeout == 0U)
      {
        NVIC_SystemReset();
      }

      timeout--;
    }

    /* Reset the timeout value */
    timeout = OPENBL_I3C_TIMEOUT;

    /* Rx flag detected */
    pBuffer[count] = LL_I3C_ReceiveData8(I3Cx);
  }

  /* Wait the end of frame */
  while (LL_I3C_IsActiveFlag_FC(I3Cx) == 0U)
  {
    OPENBL_IWDG_Refresh();

    if (timeout == 0U)
    {
      NVIC_SystemReset();
    }
    else
    {
      timeout--;
    }
  }

  /* Clear the Frame Complete flag */
  LL_I3C_ClearFlag_FC(I3Cx);
}

/**
  * @brief  This function is used to process and execute the special commands.
  *         The user must define the special commands routine here.
  * @param  SpecialCmd Pointer to the OPENBL_SpecialCmdTypeDef structure.
  * @retval Returns NACK status in case of error else returns ACK status.
  */
void OPENBL_I3C_SpecialCommandProcess(OPENBL_SpecialCmdTypeDef *SpecialCmd)
{
  uint8_t data[128] = {0x00U};

  switch (SpecialCmd->OpCode)
  {
    /* Unknown command opcode */
    default:
      if (SpecialCmd->CmdType == OPENBL_SPECIAL_CMD)
      {
        /* Send Ack to inform the host that we are ready to send data */
        OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

        /* Send NULL data size */
        data[0] = 0x00U;
        data[1] = 0x00U;

        OPENBL_I3C_SendBytes(data, 2U);

        /* Send NULL status size */
        OPENBL_I3C_SendBytes(data, 2U);

      }
      else if (SpecialCmd->CmdType == OPENBL_EXTENDED_SPECIAL_CMD)
      {
        /* Send Ack to inform the host that we are ready to send data */
        OPENBL_I3C_SendAcknowledgeByte(ACK_BYTE);

        /* Send NULL status size */
        data[0] = 0x00U;
        data[1] = 0x00U;

        OPENBL_I3C_SendBytes(data, 2U);
      }

      break;
  }
}

/**
  * @brief  Handle I3C interrupt request. This handler is used to detect host communication.
  *         It is only used during connection phase.
  * @retval None.
  */
void OPENBL_I3C_IRQHandler(void)
{
  uint8_t data;

  data = OPENBL_I3C_ReadByte();

  /* Check if the host has sent a synchronization bye */
  if (data == OPENBL_I3C_SYNC_BYTE)
  {
    I3cDetected = 1U;
  }
}
