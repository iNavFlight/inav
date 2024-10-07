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
  LL_I2C_InitTypeDef I2C_InitStruct;

  I2C_InitStruct.PeripheralMode      = LL_I2C_MODE_I2C;
  I2C_InitStruct.Timing              = I2C_TIMING;
  I2C_InitStruct.AnalogFilter        = LL_I2C_ANALOGFILTER_ENABLE;
  I2C_InitStruct.DigitalFilter       = 0U;
  I2C_InitStruct.OwnAddress1         = I2C_ADDRESS;
  I2C_InitStruct.TypeAcknowledge     = LL_I2C_NACK;
  I2C_InitStruct.OwnAddrSize         = LL_I2C_OWNADDRESS1_7BIT;

  LL_I2C_Init(I2Cx, &I2C_InitStruct);
  LL_I2C_Enable(I2Cx);
}

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  This function is used to configure I2C pins and then initialize the used I2C instance.
  * @retval None.
  */
void OPENBL_I2C_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Enable all resources clocks --------------------------------------------*/
  /* Enable used GPIOx clocks */
  I2Cx_GPIO_CLK_ENABLE();

  /* Enable I2C clock */
  I2Cx_CLK_ENABLE();

  GPIO_InitStruct.Pin       = I2Cx_SCL_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = I2Cx_ALTERNATE;
  HAL_GPIO_Init(I2Cx_SCL_PIN_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin       = I2Cx_SDA_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = I2Cx_ALTERNATE;
  HAL_GPIO_Init(I2Cx_SDA_PIN_PORT, &GPIO_InitStruct);

  OPENBL_I2C_Init();
}

/**
  * @brief  This function is used to De-initialize the I2C pins and instance.
  * @retval None.
  */
void OPENBL_I2C_DeInit(void)
{
  /* Only de-initialize the I2C if it is not the current detected interface */
  if (I2cDetected == 0U)
  {
    LL_I2C_Disable(I2Cx);

    I2Cx_CLK_DISABLE();
  }
}

/**
  * @brief  This function is used to detect if there is any activity on I2C protocol.
  * @retval None.
  */
uint8_t OPENBL_I2C_ProtocolDetection(void)
{
  /* Check if the I2Cx is addressed */
  if ((I2Cx->ISR & I2C_ISR_ADDR) != 0)
  {
    I2cDetected = 1U;
  }
  else
  {
    I2cDetected = 0U;
  }

  return I2cDetected;
}

/**
  * @brief  This function is used to get the command opcode from the host.
  * @retval Returns the command.
  */
uint8_t OPENBL_I2C_GetCommandOpcode(void)
{
  uint8_t command_opc = 0x0U;

  while (LL_I2C_IsActiveFlag_ADDR(I2Cx) == 0)
  {
    OPENBL_IWDG_Refresh();
  }

  LL_I2C_ClearFlag_ADDR(I2Cx);

  /* Get the command opcode */
  command_opc = OPENBL_I2C_ReadByte();

  /* Check the data integrity */
  if ((command_opc ^ OPENBL_I2C_ReadByte()) != 0xFFU)
  {
    command_opc = ERROR_COMMAND;
  }

  OPENBL_I2C_WaitStop();

  return command_opc;
}

/**
  * @brief  This function is used to read one byte from I2C pipe.
  * @retval Returns the read byte.
  */
uint8_t OPENBL_I2C_ReadByte(void)
{
  uint32_t timeout = 0U;

  while (LL_I2C_IsActiveFlag_RXNE(I2Cx) == 0)
  {
    OPENBL_IWDG_Refresh();

    if ((timeout++) >= OPENBL_I2C_TIMEOUT)
    {
      /* System Reset */
      NVIC_SystemReset();
    }
  }

  return LL_I2C_ReceiveData8(I2Cx);
}

/**
  * @brief  This function is used to send one byte through I2C pipe.
  * @param  Byte The byte to be sent.
  * @retval None.
  */
void OPENBL_I2C_SendByte(uint8_t Byte)
{
  uint32_t timeout = 0U;

  if (LL_I2C_IsActiveFlag_TXIS(I2Cx) == 0)
  {
    while (LL_I2C_IsActiveFlag_TXIS(I2Cx) == 0)
    {
      OPENBL_IWDG_Refresh();

      if ((timeout++) >= OPENBL_I2C_TIMEOUT)
      {
        /* System Reset */
        NVIC_SystemReset();
      }
    }
  }

  LL_I2C_TransmitData8(I2Cx, Byte);
}

/**
  * @brief  This function is used to wait until the address is matched.
  * @retval None.
  */
void OPENBL_I2C_WaitAddress(void)
{
  uint32_t timeout = 0U;

  while (LL_I2C_IsActiveFlag_ADDR(I2Cx) == 0)
  {
    OPENBL_IWDG_Refresh();

    if ((timeout++) >= OPENBL_I2C_TIMEOUT)
    {
      /* System Reset */
      NVIC_SystemReset();
    }
  }

  LL_I2C_ClearFlag_ADDR(I2Cx);
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
  uint32_t timeout = 0U;

  /* While the i2C NACK is not detected, the IWDG is refreshed,
  if the timeout is reached a system reset occurs */
  while ((I2Cx->ISR & I2C_ISR_NACKF) == 0)
  {
    /* Refresh IWDG: reload counter */
    IWDG->KR = IWDG_KEY_RELOAD;

    if ((timeout++) >= OPENBL_I2C_TIMEOUT)
    {
      /* System Reset */
      SCB->AIRCR  = ((0x5FAUL << SCB_AIRCR_VECTKEY_Pos)    |
                     (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |
                     SCB_AIRCR_SYSRESETREQ_Msk);
    }
  }

  /* Clear Not Acknowledge detection flag. */
  I2Cx->ICR |= I2C_ICR_NACKCF;
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
  uint32_t timeout = 0U;

  /* While the i2C stop is not detected, refresh the IWDG,
  if the timeout is reached a system reset occurs */
  while ((I2Cx->ISR & I2C_ISR_STOPF) == 0)
  {
    /* Refresh IWDG: reload counter */
    IWDG->KR = IWDG_KEY_RELOAD;

    if ((timeout++) >= OPENBL_I2C_TIMEOUT)
    {
      /* System Reset */
      SCB->AIRCR  = ((0x5FAUL << SCB_AIRCR_VECTKEY_Pos)    |
                     (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |
                     SCB_AIRCR_SYSRESETREQ_Msk);
    }
  }

  /* Clear Stop detection flag. */
  I2Cx->ICR |= I2C_ICR_STOPCF;
}

/**
  * @brief  This function is used to send Acknowledgment.
  * @retval None.
  */
void OPENBL_I2C_SendAcknowledgeByte(uint8_t Byte)
{
  /* Wait until address is matched */
  OPENBL_I2C_WaitAddress();

  /* Send ACK or NACK byte */
  OPENBL_I2C_SendByte(Byte);

  /* Wait until NACK is detected */
  OPENBL_I2C_WaitNack();

  /* Wait until STOP byte is detected*/
  OPENBL_I2C_WaitStop();
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
  uint32_t timeout = 0;

  /* Wait for the received address to match with the device address */
  if (((I2Cx->ISR & I2C_ISR_ADDR) != 0))
  {
    /* Clear the flag of address match*/
    I2Cx->ICR |= I2C_ICR_ADDRCF;

    /* While the transmit data is not empty, refresh the IWDG,
    if the timeout is reached a system reset occurs */
    while ((I2Cx->ISR & I2C_ISR_TXIS) == 0)
    {
      IWDG->KR = IWDG_KEY_RELOAD;

      if ((timeout++) >= OPENBL_I2C_TIMEOUT)
      {
        /* System Reset */
        SCB->AIRCR  = ((0x5FAUL << SCB_AIRCR_VECTKEY_Pos)    |
                       (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |
                       SCB_AIRCR_SYSRESETREQ_Msk);
      }
    }

    /* Send busy byte */
    I2Cx->TXDR = BUSY_BYTE;

    /* Wait until NACK is detected */
    OPENBL_I2C_WaitNack();

    /* Wait until STOP byte is detected */
    OPENBL_I2C_WaitStop();
  }
}

/**
  * @brief  This function is used to process and execute the special commands.
  *         The user must define the special commands routine here.
  * @param  SpecialCmd Pointer to the OPENBL_SpecialCmdTypeDef structure.
  * @retval Returns NACK status in case of error else returns ACK status.
  */
void OPENBL_I2C_SpecialCommandProcess(OPENBL_SpecialCmdTypeDef *SpecialCmd)
{
  switch (SpecialCmd->OpCode)
  {
    /* Unknown command opcode */
    default:
      if (SpecialCmd->CmdType == OPENBL_SPECIAL_CMD)
      {
        /* Send NULL data size */
        OPENBL_I2C_SendByte(0x00U);
        OPENBL_I2C_SendByte(0x00U);

        /* Wait for address to match */
        OPENBL_I2C_WaitAddress();

        /* Send NULL status size */
        OPENBL_I2C_SendByte(0x00U);
        OPENBL_I2C_SendByte(0x00U);
      }
      else if (SpecialCmd->CmdType == OPENBL_EXTENDED_SPECIAL_CMD)
      {
        /* Send NULL status size */
        OPENBL_I2C_SendByte(0x00U);
        OPENBL_I2C_SendByte(0x00U);
      }
      break;
  }
}

/**
  * @brief  This function is used to Set Flash busy state variable to activate busy state sending
  *         during flash operations
  * @retval None.
  */
void OPENBL_Enable_BusyState_Sending(void)
{
  /* Enable Flash busy state sending */
  OPENBL_Enable_BusyState_Flag();
}

/**
  * @brief  This function is used to disable the send of busy state in I2C non stretch mode.
  * @retval None.
  */
void OPENBL_Disable_BusyState_Sending(void)
{
  /* Disable Flash busy state sending */
  OPENBL_Disable_BusyState_Flag();
}
