/**
  ******************************************************************************
  * @file    usbd_ccid_if_template.c
  * @author  MCD Application Team
  * @brief   This file provides all the functions for USB Interface for CCID
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

/* Includes ------------------------------------------------------------------*/
#include "usbd_ccid.h"
#include "usbd_ccid_if_template.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static REP_Command_t REP_command;

/* Private function prototypes -----------------------------------------------*/
static uint8_t CCID_Init(USBD_HandleTypeDef  *pdev);
static uint8_t CCID_DeInit(USBD_HandleTypeDef  *pdev);
static uint8_t CCID_ControlReq(uint8_t req, uint8_t *pbuf, uint16_t *length);
static uint8_t CCID_Response_SendData(USBD_HandleTypeDef  *pdev, uint8_t *buf, uint16_t len);
static uint8_t CCID_Send_Process(uint8_t *Command, uint8_t *Data);
static uint8_t CCID_Response_Process(void);
static uint8_t CCID_SetSlotStatus(USBD_HandleTypeDef *pdev);

/* Private functions ---------------------------------------------------------*/

/**
  * @}
  */

USBD_CCID_ItfTypeDef USBD_CCID_If_fops =
{
  CCID_Init,
  CCID_DeInit,
  CCID_ControlReq,
  CCID_Response_SendData,
  CCID_Send_Process,
  CCID_SetSlotStatus,
};

/**
  * @brief  CCID_Init
  *         Initialize the CCID USB Layer
  * @param  pdev: device instance
  * @retval status value
  */
uint8_t CCID_Init(USBD_HandleTypeDef  *pdev)
{
#ifdef USE_USBD_COMPOSITE
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
#else
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassData;
#endif /* USE_USBD_COMPOSITE */

  /* CCID Related Initialization */

  hccid->blkt_state = CCID_STATE_IDLE;

  return (uint8_t)USBD_OK;
}

/**
  * @brief  CCID_DeInit
  *         Uninitialize the CCID Machine
  * @param  pdev: device instance
  * @retval status value
  */
uint8_t CCID_DeInit(USBD_HandleTypeDef  *pdev)
{
#ifdef USE_USBD_COMPOSITE
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
#else
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassData;
#endif /* USE_USBD_COMPOSITE */

  hccid->blkt_state = CCID_STATE_IDLE;

  return (uint8_t)USBD_OK;
}

/**
  * @brief  CCID_ControlReq
  *         Manage the CCID class requests
  * @param  Cmd: Command code
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static uint8_t CCID_ControlReq(uint8_t req, uint8_t *pbuf, uint16_t *length)
{
#ifdef USE_USBD_COMPOSITE
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)USBD_Device.pClassDataCmsit[USBD_Device.classId];
#else
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)USBD_Device.pClassData;
#endif /* USE_USBD_COMPOSITE */

  UNUSED(length);

  switch (req)
  {
    case REQUEST_ABORT:
      /* The wValue field contains the slot number (bSlot) in the low byte
      and the sequence number (bSeq) in the high byte.*/
      hccid->slot_nb = ((uint16_t) * pbuf & 0x0fU);
      hccid->seq_nb = (((uint16_t) * pbuf & 0xf0U) >> 8);

      if (CCID_CmdAbort(&USBD_Device, (uint8_t)hccid->slot_nb, (uint8_t)hccid->seq_nb) != 0U)
      {
        /* If error is returned by lower layer :
        Generally Slot# may not have matched */
        return (int8_t)USBD_FAIL;
      }
      break;

    case REQUEST_GET_CLOCK_FREQUENCIES:

      /* User have to fill the pbuf with the GetClockFrequency data buffer */

      break;

    case REQUEST_GET_DATA_RATES:

      /* User have to fill the pbuf with the GetDataRates data buffer */

      break;

    default:
      break;
  }

  UNUSED(pbuf);

  return ((int8_t)USBD_OK);
}

/**
  * @brief  CCID_Response_SendData
  *         Send the data on bulk-in EP
  * @param  pdev: device instance
  * @param  buf: pointer to data buffer
  * @param  len: Data Length
  * @retval status value
  */
uint8_t  CCID_Response_SendData(USBD_HandleTypeDef  *pdev, uint8_t *buf, uint16_t len)
{
  (void)USBD_LL_Transmit(pdev, CCID_IN_EP, buf, len);
  return (uint8_t)USBD_OK;
}

/**
  * @brief  CCID_SEND_Process
  * @param  Command: pointer to a buffer containing command header
  * @param  Data: pointer to a buffer containing data sent from Host
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static uint8_t CCID_Send_Process(uint8_t *Command, uint8_t *Data)
{
  Command_State_t Command_State = Command_NOT_OK;

  /* Initialize ICC APP header */
  uint8_t SC_Command[5] = {0};
  UNUSED(Data);
  UNUSED(Command_State);
  UNUSED(SC_Command);

  /* Start SC Demo ---------------------------------------------------------*/
  switch (Command[1]) /* type of instruction */
  {
    case SC_ENABLE:
      /* Add your code here */
      break;

    case SC_VERIFY:
      /* Add your code here */
      break;

    case SC_READ_BINARY :
      /* Add your code here */
      break;

    case SC_CHANGE :
      /* Add your code here */
      break;

    default:
      break;
  }

  /* check if Command header is  OK */
  (void)CCID_Response_Process(); /* Get ICC response */

  return ((uint8_t)USBD_OK);
}

/**
  * @brief  CCID_Response_Process
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static uint8_t CCID_Response_Process(void)
{
  switch (REP_command)
  {
    case REP_OK:
      /* Add your code here */
      break;

    case REP_NOT_OK :
      /* Add your code here */
      break;

    case REP_NOT_SUPP :
      /* Add your code here */
      break;

    case REP_ENABLED :
      /* Add your code here */
      break;

    case REP_CHANGE :
      /* Add your code here */
      break;

    default:
      break;
  }

  return ((uint8_t)USBD_OK);
}

/**
  * @brief  CCID_SetSlotStatus
  *         Set Slot Status of the Interrupt Transfer
  * @param  pdev: device instance
  * @retval status
  */
uint8_t CCID_SetSlotStatus(USBD_HandleTypeDef *pdev)
{
  /* Get the CCID handler pointer */
#ifdef USE_USBD_COMPOSITE
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
#else
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassData;
#endif /* USE_USBD_COMPOSITE */

  if ((hccid->SlotStatus.SlotStatus) == 1U) /* Transfer Complete Status
                        of previous Interrupt transfer */
  {
    /* Add your code here */
  }
  else
  {
    /* Add your code here */
  }

  return (uint8_t)USBD_OK;
}
