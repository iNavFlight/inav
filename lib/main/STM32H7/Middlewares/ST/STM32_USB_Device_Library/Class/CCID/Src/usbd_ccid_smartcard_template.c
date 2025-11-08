/**
  ******************************************************************************
  * @file    usbd_ccid_smartcard_template.c
  * @author  MCD Application Team
  * @brief   This file provides all the Smartcard firmware functions.
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

/** @addtogroup usbd_ccid_Smartcard
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "usbd_ccid_smartcard_template.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Directories & Files ID */
/*The following Directories & Files ID  can take any of following Values and can
  be used in the smartcard application */
/*
const uint8_t MasterRoot[2] = {0x3F, 0x00};
const uint8_t GSMDir[2] = {0x7F, 0x20};
const uint8_t ICCID[2] = {0x2F, 0xE2};
const uint8_t IMSI[2] = {0x6F, 0x07};

__IO uint8_t ICCID_Content[10] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

uint32_t CHV1Status = 0U;

uint8_t CHV1[8] = {'0', '0', '0', '0', '0', '0', '0', '0'};
__IO uint8_t IMSI_Content[9] = {0x01, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
*/

/* F Table: Clock Rate Conversion Table from ISO/IEC 7816-3 */
/* static uint32_t F_Table[16] = {372, 372, 558, 744, 1116, 1488, 1860, 0, 0, 512, 768,
                                  1024, 1536, 2048, 0, 0
                                 }; */


/* D Table: Baud Rate Adjustment Factor Table from ISO/IEC 7816-3 */
static uint32_t D_Table[16] = {0, 1, 2, 4, 8, 16, 32, 64, 12, 20, 0, 0, 0, 0, 0, 0};

/* Global variables definition and initialization ----------------------------*/
SC_ATRTypeDef SC_A2R;
uint8_t SC_ATR_Table[40];
uint8_t ProtocolNUM_OUT;

/* Private function prototypes -----------------------------------------------*/
static void SC_Init(void);
static void SC_DeInit(void);
static void SC_AnswerReq(SC_State *SC_state, uint8_t *card, uint8_t length);  /* Ask ATR */
static uint8_t SC_decode_Answer2reset(uint8_t *card);  /* Decode ATR */
static void SC_SendData(SC_ADPU_CommandsTypeDef *SCADPU, SC_ADPU_ResponseTypeDef *SC_ResponseStatus);
/* static void SC_Reset(GPIO_PinState ResetState); */

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Handles all Smartcard states and serves to send and receive all
  *         communication data between Smartcard and reader.
  * @param  SCState: pointer to an SC_State enumeration that will contain the
  *         Smartcard state.
  * @param  SC_ADPU: pointer to an SC_ADPU_Commands structure that will be initialized.
  * @param  SC_Response: pointer to a SC_ADPU_Response structure which will be initialized.
  * @retval None
  */
void SC_Handler(SC_State *SCState, SC_ADPU_CommandsTypeDef *SC_ADPU, SC_ADPU_ResponseTypeDef *SC_Response)
{
  uint32_t i;
  uint32_t j;

  switch (*SCState)
  {
    case SC_POWER_ON:
      if (SC_ADPU->Header.INS == SC_GET_A2R)
      {
        /* Smartcard initialization */
        SC_Init();

        /* Reset Data from SC buffer */
        for (i = 0U; i < 40U; i++)
        {
          SC_ATR_Table[i] = 0;
        }

        /* Reset SC_A2R Structure */
        SC_A2R.TS = 0U;
        SC_A2R.T0 = 0U;

        for (i = 0U; i < MAX_PROTOCOLLEVEL; i++)
        {
          for (j = 0U; j < MAX_INTERFACEBYTE; j++)
          {
            SC_A2R.T[i].InterfaceByte[j].Status = 0U;
            SC_A2R.T[i].InterfaceByte[j].Value = 0U;
          }
        }

        for (i = 0U; i < HIST_LENGTH; i++)
        {
          SC_A2R.Historical[i] = 0U;
        }

        SC_A2R.Tlength = 0U;
        SC_A2R.Hlength = 0U;

        /* Next State */
        *SCState = SC_RESET_LOW;
      }
      break;

    case SC_RESET_LOW:
      if (SC_ADPU->Header.INS == SC_GET_A2R)
      {
        /* If card is detected then Power ON, Card Reset and wait for an answer) */
        if (SC_Detect() != 0U)
        {
          while (((*SCState) != SC_POWER_OFF) && ((*SCState) != SC_ACTIVE))
          {
            SC_AnswerReq(SCState, &SC_ATR_Table[0], 40U); /* Check for answer to reset */
          }
        }
        else
        {
          (*SCState) = SC_POWER_OFF;
        }
      }
      break;

    case SC_ACTIVE:
      if (SC_ADPU->Header.INS == SC_GET_A2R)
      {
        uint8_t protocol = SC_decode_Answer2reset(&SC_ATR_Table[0]);
        if (protocol == T0_PROTOCOL)
        {
          (*SCState) = SC_ACTIVE_ON_T0;
          ProtocolNUM_OUT = T0_PROTOCOL;
        }
        else if (protocol == T1_PROTOCOL)
        {
          (*SCState) = SC_ACTIVE_ON_T1;
          ProtocolNUM_OUT = T1_PROTOCOL;
        }
        else
        {
          (*SCState) = SC_POWER_OFF;
        }
      }
      break;

    case SC_ACTIVE_ON_T0:
      /* process commands other than ATR */
      SC_SendData(SC_ADPU, SC_Response);
      break;

    case SC_ACTIVE_ON_T1:
      /* process commands other than ATR */
      SC_SendData(SC_ADPU, SC_Response);
      break;

    case SC_POWER_OFF:
      SC_DeInit(); /* Disable Smartcard interface */
      break;

    default:
      (*SCState) = SC_POWER_OFF;
      break;
  }
}

/**
  * @brief  Enables or disables the power to the Smartcard.
  * @param  NewState: new state of the Smartcard power supply.
  *         This parameter can be: SC_ENABLED or SC_DISABLED.
  * @retval None
  */
void SC_PowerCmd(SCPowerState NewState)
{
  UNUSED(NewState);
  /* enable or disable smartcard pin */

  return;
}

/**
  * @brief  Sets or clears the Smartcard reset pin.
  * @param  ResetState: this parameter specifies the state of the Smartcard
  *         reset pin. BitVal must be one of the BitAction enum values:
  *                 @arg Bit_RESET: to clear the port pin.
  *                 @arg Bit_SET: to set the port pin.
  * @retval None
  */
/* static void SC_Reset(GPIO_PinState ResetState)
{
  UNUSED(ResetState);

  return;
}
*/


/**
  * @brief  Resends the byte that failed to be received (by the Smartcard) correctly.
  * @param  None
  * @retval None
  */

void SC_ParityErrorHandler(void)
{
  /* Add your code here */

  return;
}

/**
  * @brief  Configures the IO speed (BaudRate) communication.
  * @param  None
  * @retval None
  */

void SC_PTSConfig(void)
{
  /* Add your code here */

  return;
}


/**
  * @brief  Manages the Smartcard transport layer: send APDU commands and receives
  *         the APDU response.
  * @param  SC_ADPU: pointer to a SC_ADPU_Commands structure which will be initialized.
  * @param  SC_Response: pointer to a SC_ADPU_Response structure which will be initialized.
  * @retval None
  */
static void SC_SendData(SC_ADPU_CommandsTypeDef *SCADPU, SC_ADPU_ResponseTypeDef *SC_ResponseStatus)
{
  uint8_t i;
  uint8_t SC_Command[5];
  uint8_t SC_DATA[LC_MAX];

  UNUSED(SCADPU);

  /* Reset response buffer */
  for (i = 0U; i < LC_MAX; i++)
  {
    SC_ResponseStatus->Data[i] = 0U;
    SC_DATA[i] = 0U;
  }

  /* User to add code here */

  /* send command to ICC and get response status */
  USBD_CCID_If_fops.Send_Process((uint8_t *)&SC_Command, (uint8_t *)&SC_DATA);

}

/**
  * @brief  SC_AnswerReq
            Requests the reset answer from card.
  * @param  SC_state: pointer to an SC_State enumeration that will contain the Smartcard state.
  * @param  atr_buffer: pointer to a buffer which will contain the card ATR.
  * @param  length: maximum ATR length
  * @retval None
  */
static void SC_AnswerReq(SC_State *SC_state, uint8_t *atr_buffer, uint8_t length)
{
  UNUSED(length);
  UNUSED(atr_buffer);

  /* to be implemented by USER */
  switch (*SC_state)
  {
    case SC_RESET_LOW:
      /* Check response with reset low */
      (*SC_state) = SC_ACTIVE;
      break;

    case SC_ACTIVE:
      break;
    case SC_RESET_HIGH:
      /* Check response with reset high */

      break;

    case SC_POWER_OFF:
      /* Close Connection if no answer received */

      break;

    default:
      (*SC_state) = SC_RESET_LOW;
      break;
  }

  return;
}

/**
  * @brief  SC_decode_Answer2reset
            Decodes the Answer to reset received from card.
  * @param  card: pointer to the buffer containing the card ATR.
  * @retval None
  */
static uint8_t SC_decode_Answer2reset(uint8_t *card)
{
  uint32_t i = 0U;
  uint32_t flag = 0U;
  uint32_t protocol;
  uint8_t index = 0U;
  uint8_t level = 0U;

  /******************************TS/T0 Decode************************************/
  index++;
  SC_A2R.TS = card[index];  /* Initial character */

  index++;
  SC_A2R.T0 = card[index];  /* Format character */

  /*************************Historical Table Length Decode***********************/
  SC_A2R.Hlength = SC_A2R.T0 & 0x0FU;

  /******************************Protocol Level(1) Decode************************/
  /* Check TD(1) if present */
  if ((SC_A2R.T0 & 0x80U) == 0x80U)
  {
    flag = 1U;
  }

  /* Each bits in the T0 high nibble(b8 to b5) equal to 1 indicates the presence
  of a further interface byte */
  for (i = 0U; i < 4U; i++)
  {
    if ((((SC_A2R.T0 & 0xF0U) >> (4U + i)) & 0x1U) != 0U)
    {
      SC_A2R.T[level].InterfaceByte[i].Status = 1U;
      index++;
      SC_A2R.T[level].InterfaceByte[i].Value = card[index];
      SC_A2R.Tlength++;
    }
  }

  /*****************************T Decode*****************************************/
  if (SC_A2R.T[level].InterfaceByte[3].Status == 1U)
  {
    /* Only the protocol(parameter T) present in TD(1) is detected
    if two or more values of parameter T are present in TD(1), TD(2)..., so the
    firmware should be updated to support them */
    protocol = (uint8_t)(SC_A2R.T[level].InterfaceByte[SC_INTERFACEBYTE_TD].Value  & 0x0FU);
  }
  else
  {
    protocol = 0U;
  }

  /* Protocol Level Increment */
  /******************************Protocol Level(n>1) Decode**********************/
  while (flag != 0U)
  {
    if ((SC_A2R.T[level].InterfaceByte[SC_INTERFACEBYTE_TD].Value & 0x80U) == 0x80U)
    {
      flag = 1U;
    }
    else
    {
      flag = 0U;
    }
    /* Each bits in the high nibble(b8 to b5) for the TD(i) equal to 1 indicates
    the presence of a further interface byte */
    for (i = 0U; i < 4U; i++)
    {
      if ((((SC_A2R.T[level].InterfaceByte[SC_INTERFACEBYTE_TD].Value & 0xF0U) >> (4U + i)) & 0x1U) != 0U)
      {
        SC_A2R.T[level + 1U].InterfaceByte[i].Status = 1U;
        index++;
        SC_A2R.T[level + 1U].InterfaceByte[i].Value = card[index];
        SC_A2R.Tlength++;
      }
    }
    level++;
  }

  for (i = 0U; i < SC_A2R.Hlength; i++)
  {
    SC_A2R.Historical[i] = card[i + 2U + SC_A2R.Tlength];
  }
  /*************************************TCK Decode*******************************/
  SC_A2R.TCK = card[SC_A2R.Hlength + 2U + SC_A2R.Tlength];

  return (uint8_t)protocol;
}

/**
  * @brief  Initializes all peripheral used for Smartcard interface.
  * @param  None
  * @retval None
  */
static void SC_Init(void)
{
  /*
     Add your initialization code here
  */

  return;
}


/**
  * @brief  Deinitializes all resources used by the Smartcard interface.
  * @param  None
  * @retval None
  */
static void SC_DeInit(void)
{
  /*
     Add your deinitialization code here
  */

  return;
}

/**
  * @brief  Configures the card power voltage.
  * @param  SC_Voltage: specifies the card power voltage.
  *         This parameter can be one of the following values:
  *              @arg SC_VOLTAGE_5V: 5V cards.
  *              @arg SC_VOLTAGE_3V: 3V cards.
  * @retval None
  */
void SC_VoltageConfig(uint32_t SC_Voltage)
{
  UNUSED(SC_Voltage);
  /* Add your code here */

  return;
}

/**
  * @brief  Configures GPIO hardware resources used for Samrtcard.
  * @param  None
  * @retval None
  */
void SC_IOConfig(void)
{
  /* Add your code here */

  return;
}

/**
  * @brief  Detects whether the Smartcard is present or not.
  * @param  None.
  * @retval 1 - Smartcard inserted
  *         0 - Smartcard not inserted
  */
uint8_t SC_Detect(void)
{
  uint8_t PIN_State = 0U;

  /* Add your code here */

  return PIN_State;
}

/**
  * @brief  Get the Right Value from the D_Table Index
  * @param  idx : Index to Read from the Table
  * @retval Value read from the Table
  */
uint32_t SC_GetDTableValue(uint32_t idx)
{
  return D_Table[idx];
}
