/**
  ******************************************************************************
  * @file    usbd_ccid_cmd.c
  * @author  MCD Application Team
  * @brief   CCID command (Bulk-OUT Messages / Bulk-IN Messages) handling
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
#include "usbd_ccid_cmd.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static uint8_t CCID_CheckCommandParams(USBD_HandleTypeDef  *pdev, uint32_t param_type);
static void CCID_UpdateCommandStatus(USBD_HandleTypeDef  *pdev, uint8_t cmd_status, uint8_t icc_status);

/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*           BULK OUT ROUTINES        */
/******************************************************************************/

/**
  * @brief  PC_to_RDR_IccPowerOn
  *         PC_TO_RDR_ICCPOWERON message execution, apply voltage and get ATR
  * @param  pdev: device instance
  * @retval status of the command execution
  */
uint8_t PC_to_RDR_IccPowerOn(USBD_HandleTypeDef *pdev)
{
  /* Apply the ICC VCC
  Fills the Response buffer with ICC ATR
  This Command is returned with RDR_to_PC_DataBlock();
  */
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint8_t voltage;
  uint8_t sc_voltage = 0U;
  uint8_t index;
  uint8_t error;

  hccid->UsbBlkInData.dwLength = 0U;  /* Reset Number of Bytes in abData */

  error = CCID_CheckCommandParams(pdev, CHK_PARAM_SLOT | CHK_PARAM_DWLENGTH |
                                  CHK_PARAM_ABRFU2 | CHK_PARAM_CARD_PRESENT |
                                  CHK_PARAM_ABORT);
  if (error != 0U)
  {
    return error;
  }

  /* Voltage that is applied to the ICC
  00h  Automatic Voltage Selection
  01h  5.0 volts
  02h  3.0 volts
  03h  1.8 volts
  */
  /* UsbBlkOutData.bSpecific_0 Contains bPowerSelect */
  voltage = hccid->UsbBlkOutData.bSpecific_0;
  if (voltage >= VOLTAGE_SELECTION_1V8)
  {
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_PRESENT_ACTIVE));
    return SLOTERROR_BAD_POWERSELECT; /* The Voltage specified is out of Spec */
  }

  /* Correct Voltage Requested by the Host */
  if ((voltage == VOLTAGE_SELECTION_AUTOMATIC) || (voltage == VOLTAGE_SELECTION_3V))
  {
    sc_voltage = SC_VOLTAGE_3V;
  }
  else
  {
    if (voltage == VOLTAGE_SELECTION_5V)
    {
      sc_voltage = SC_VOLTAGE_5V;
    }
  }
  SC_Itf_IccPowerOn(sc_voltage);

  /* Check if the Card has come to Active State*/
  error = CCID_CheckCommandParams(pdev, (uint32_t)CHK_ACTIVE_STATE);

  if (error != 0U)
  {
    /* Check if Voltage is not Automatic */
    if (voltage != 0U)
    {
      /* If Specific Voltage requested by Host i.e 3V or 5V*/
      return error;
    }
    else
    {
      /* Automatic Voltage selection requested by Host */

      if (sc_voltage != SC_VOLTAGE_5V)
      {
        /* If voltage selected was Automatic and 5V is not yet tried */
        sc_voltage = SC_VOLTAGE_5V;
        SC_Itf_IccPowerOn(sc_voltage);

        /* Check again the State */
        error = CCID_CheckCommandParams(pdev, (uint32_t)CHK_ACTIVE_STATE);
        if (error != 0U)
        {
          return error;
        }
      }
      else
      {
        /* Voltage requested from Host was 5V already*/
        CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_PRESENT_INACTIVE));
        return error;
      }
    } /* Voltage Selection was automatic */
  } /* If Active State */

  /* ATR is received, No Error Condition Found */
  hccid->UsbBlkInData.dwLength = SIZE_OF_ATR;
  CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_NO_ERROR), (BM_ICC_PRESENT_ACTIVE));

  for (index = 0U; index < SIZE_OF_ATR; index++)
  {
    /* Copy the ATR to the Response Buffer */
    hccid->UsbBlkInData.abData[index] = SC_ATR_Table[index];
  }

  return SLOT_NO_ERROR;
}

/**
  * @brief  PC_to_RDR_IccPowerOff
  *         Icc VCC is switched Off
  * @param  pdev: device instance
  * @retval error: status of the command execution
  */
uint8_t PC_to_RDR_IccPowerOff(USBD_HandleTypeDef *pdev)
{
  /*  The response to this command is the RDR_to_PC_SlotStatus*/
  uint8_t error;

  error = CCID_CheckCommandParams(pdev, CHK_PARAM_SLOT | CHK_PARAM_ABRFU3 |
                                  CHK_PARAM_DWLENGTH);
  if (error != 0U)
  {
    return error;
  }
  /* Command is ok, Check for Card Presence */
  if (SC_Detect() != 0U)
  {
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_NO_ERROR), (BM_ICC_PRESENT_INACTIVE));
  }
  else
  {
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_NO_ERROR), (BM_ICC_NO_ICC_PRESENT));
  }

  /* Power OFF the card */
  SC_Itf_IccPowerOff();

  return SLOT_NO_ERROR;
}

/**
  * @brief  PC_to_RDR_GetSlotStatus
  *         Provides the Slot status to the host
  * @param  pdev: device instance
  * @retval status of the command execution
  */
uint8_t  PC_to_RDR_GetSlotStatus(USBD_HandleTypeDef *pdev)
{
  uint8_t error;

  error = CCID_CheckCommandParams(pdev, CHK_PARAM_SLOT |  CHK_PARAM_DWLENGTH |
                                  CHK_PARAM_CARD_PRESENT | CHK_PARAM_ABRFU3);
  if (error != 0U)
  {
    return error;
  }

  CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_NO_ERROR), (BM_ICC_PRESENT_ACTIVE));
  return SLOT_NO_ERROR;
}


/**
  * @brief  PC_to_RDR_XfrBlock
  *         Handles the Block transfer from Host.
  *         Response to this command message is the RDR_to_PC_DataBlock
  * @param  pdev: device instance
  * @retval status of the command execution
  */
uint8_t  PC_to_RDR_XfrBlock(USBD_HandleTypeDef *pdev)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint16_t expectedLength;

  uint8_t error;

  error = CCID_CheckCommandParams(pdev, CHK_PARAM_SLOT | CHK_PARAM_CARD_PRESENT |
                                  CHK_PARAM_ABRFU3 | CHK_PARAM_ABORT | CHK_ACTIVE_STATE);
  if (error != 0U)
  {
    return error;
  }
  if (hccid->UsbBlkOutData.dwLength > ABDATA_SIZE)
  {
    /* Check amount of Data Sent by Host is > than memory allocated ? */

    return SLOTERROR_BAD_DWLENGTH;
  }

  /* wLevelParameter = Size of expected data to be returned by the
  bulk-IN endpoint */
  expectedLength = (uint8_t)((hccid->UsbBlkOutData.bSpecific_2 << 8) |
                             hccid->UsbBlkOutData.bSpecific_1);

  hccid->UsbBlkInData.dwLength = (uint16_t)expectedLength;

  error = SC_Itf_XferBlock(&(hccid->UsbBlkOutData.abData[0]),
                           hccid->UsbBlkOutData.dwLength,
                           expectedLength, &hccid->UsbBlkInData);

  if (error != SLOT_NO_ERROR)
  {
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_PRESENT_ACTIVE));
  }
  else
  {
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_NO_ERROR), (BM_ICC_PRESENT_ACTIVE));
    error = SLOT_NO_ERROR;
  }

  return error;
}

/**
  * @brief  PC_to_RDR_GetParameters
  *         Provides the ICC parameters to the host
  *         Response to this command message is the RDR_to_PC_Parameters
  * @param  pdev: device instance
  * @retval status of the command execution
  */
uint8_t  PC_to_RDR_GetParameters(USBD_HandleTypeDef *pdev)
{
  uint8_t error;

  error = CCID_CheckCommandParams(pdev, CHK_PARAM_SLOT | CHK_PARAM_DWLENGTH |
                                  CHK_PARAM_CARD_PRESENT | CHK_PARAM_ABRFU3);
  if (error != 0U)
  {
    return error;
  }
  CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_NO_ERROR), (BM_ICC_PRESENT_ACTIVE));

  return SLOT_NO_ERROR;
}

/**
  * @brief  PC_to_RDR_ResetParameters
  *         Set the ICC parameters to the default
  *         Response to this command message is the RDR_to_PC_Parameters
  * @param  pdev: device instance
  * @retval status of the command execution
  */
uint8_t  PC_to_RDR_ResetParameters(USBD_HandleTypeDef *pdev)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint8_t error;

  error = CCID_CheckCommandParams(pdev, CHK_PARAM_SLOT | CHK_PARAM_DWLENGTH |
                                  CHK_PARAM_CARD_PRESENT | CHK_PARAM_ABRFU3 |
                                  CHK_ACTIVE_STATE);
  if (error != 0U)
  {
    return error;
  }
  /* This command resets the slot parameters to their default values */
  hccid->UsbBlkOutData.abData[0] = DEFAULT_FIDI;
  hccid->UsbBlkOutData.abData[1] = DEFAULT_T01CONVCHECKSUM;
  hccid->UsbBlkOutData.abData[2] = DEFAULT_EXTRA_GUARDTIME;
  hccid->UsbBlkOutData.abData[3] = DEFAULT_WAITINGINTEGER;
  hccid->UsbBlkOutData.abData[4] = DEFAULT_CLOCKSTOP;
  hccid->UsbBlkOutData.abData[5] = 0x00U;
  hccid->UsbBlkOutData.abData[6] = 0x00U;

  (void)USBD_memcpy(&ProtocolData, (void const *)(&hccid->UsbBlkOutData.abData[0]),
                    sizeof(ProtocolData));

  error = SC_Itf_SetParams(&ProtocolData, ProtocolNUM_OUT);

  if (error != SLOT_NO_ERROR)
  {
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_PRESENT_ACTIVE));
  }
  else
  {
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_NO_ERROR), (BM_ICC_PRESENT_ACTIVE));
    error = SLOT_NO_ERROR;
  }

  return error;
}
/**
  * @brief  PC_to_RDR_SetParameters
  *         Set the ICC parameters to the host defined parameters
  *         Response to this command message is the RDR_to_PC_Parameters
  * @param  pdev: device instance
  * @retval status of the command execution
  */
uint8_t  PC_to_RDR_SetParameters(USBD_HandleTypeDef *pdev)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint8_t error;

  error = CCID_CheckCommandParams(pdev, CHK_PARAM_SLOT | CHK_PARAM_CARD_PRESENT |
                                  CHK_PARAM_ABRFU2 | CHK_ACTIVE_STATE);
  if (error != 0U)
  {
    return error;
  }
  error = SLOT_NO_ERROR;

  /* for Protocol T=0 (dwLength=00000005h) */
  /* for Protocol T=1 (dwLength=00000007h) */
  if (((hccid->UsbBlkOutData.dwLength == 5U) && (hccid->UsbBlkOutData.bSpecific_0 != 0U))
      || ((hccid->UsbBlkOutData.dwLength == 7U) && (hccid->UsbBlkOutData.bSpecific_0 != 1U)))
  {
    error = SLOTERROR_BAD_PROTOCOLNUM;
  }
  if (hccid->UsbBlkOutData.abData[4] != DEFAULT_CLOCKSTOP)
  {
    error = SLOTERROR_BAD_CLOCKSTOP;
  }
  if (error != SLOT_NO_ERROR)
  {
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_PRESENT_ACTIVE));
  }

  (void)USBD_memcpy(&ProtocolData, (void const *)(&hccid->UsbBlkOutData.abData[0]),
                    sizeof(ProtocolData));

  error = SC_Itf_SetParams(&ProtocolData, ProtocolNUM_OUT);

  if (error != SLOT_NO_ERROR)
  {
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_PRESENT_ACTIVE));
  }
  else
  {
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_NO_ERROR), (BM_ICC_PRESENT_ACTIVE));
    error = SLOT_NO_ERROR;
  }

  return error;
}
/**
  * @brief  PC_to_RDR_Escape
  *         Execute the Escape command. This is user specific Implementation
  *         Response to this command message is the RDR_to_PC_Escape
  * @param  pdev: device instance
  * @retval status of the command execution
  */
uint8_t  PC_to_RDR_Escape(USBD_HandleTypeDef *pdev)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint8_t error;
  uint32_t size;

  error = CCID_CheckCommandParams(pdev, CHK_PARAM_SLOT | CHK_PARAM_CARD_PRESENT |
                                  CHK_PARAM_ABRFU3 | CHK_PARAM_ABORT | CHK_ACTIVE_STATE);

  if (error != 0U)
  {
    return error;
  }
  error = SC_Itf_Escape(&hccid->UsbBlkOutData.abData[0], hccid->UsbBlkOutData.dwLength,
                        &hccid->UsbBlkInData.abData[0], &size);

  hccid->UsbBlkInData.dwLength = size;

  if (error != SLOT_NO_ERROR)
  {
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_PRESENT_ACTIVE));
  }
  else
  {
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_NO_ERROR), (BM_ICC_PRESENT_ACTIVE));
  }

  return error;
}
/**
  * @brief  PC_to_RDR_IccClock
  *         Execute the Clock specific command from host
  *         Response to this command message is the RDR_to_PC_SlotStatus
  * @param  pdev: device instance
  * @retval status of the command execution
  */
uint8_t  PC_to_RDR_IccClock(USBD_HandleTypeDef *pdev)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint8_t error;

  error = CCID_CheckCommandParams(pdev, CHK_PARAM_SLOT | CHK_PARAM_CARD_PRESENT |
                                  CHK_PARAM_ABRFU2 | CHK_PARAM_DWLENGTH | CHK_ACTIVE_STATE);
  if (error != 0U)
  {
    return error;
  }
  /* bClockCommand :
  00h restarts Clock
  01h Stops Clock in the state shown in the bClockStop field */
  if (hccid->UsbBlkOutData.bSpecific_0 > 1U)
  {
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_PRESENT_ACTIVE));
    return SLOTERROR_BAD_CLOCKCOMMAND;
  }

  error = SC_Itf_SetClock(hccid->UsbBlkOutData.bSpecific_0);

  if (error != SLOT_NO_ERROR)
  {
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_PRESENT_ACTIVE));
  }
  else
  {
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_NO_ERROR), (BM_ICC_PRESENT_ACTIVE));
  }

  return error;
}
/**
  * @brief  PC_to_RDR_Abort
  *         Execute the Abort command from host, This stops all Bulk transfers
  *         from host and ICC
  *         Response to this command message is the RDR_to_PC_SlotStatus
  * @param  pdev: device instance
  * @retval status of the command execution
  */
uint8_t  PC_to_RDR_Abort(USBD_HandleTypeDef *pdev)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint8_t error;

  error = CCID_CheckCommandParams(pdev, CHK_PARAM_SLOT | CHK_PARAM_ABRFU3 |
                                  CHK_PARAM_DWLENGTH);
  if (error != 0U)
  {
    return error;
  }
  (void)CCID_CmdAbort(pdev, hccid->UsbBlkOutData.bSlot, hccid->UsbBlkOutData.bSeq);
  CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_NO_ERROR), (BM_ICC_PRESENT_ACTIVE));
  return SLOT_NO_ERROR;
}

/**
  * @brief  CCID_CmdAbort
  *         Execute the Abort command from Bulk EP or from Control EP,
  *         This stops all Bulk transfers from host and ICC
  * @param  pdev: device instance
  * @param  slot: slot number that host wants to abort
  * @param  seq : Seq number for PC_to_RDR_Abort
  * @retval status of the command execution
  */
uint8_t CCID_CmdAbort(USBD_HandleTypeDef *pdev, uint8_t slot, uint8_t seq)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  uint8_t BSlot = hccid->USBD_CCID_Param.bSlot;
  /* This function is called for REQUEST_ABORT & PC_to_RDR_Abort */

  if (slot >= CCID_NUMBER_OF_SLOTS)
  {
    /* error from CLASS_REQUEST*/
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_NO_ICC_PRESENT));
    return SLOTERROR_BAD_SLOT;
  }

  if (hccid->USBD_CCID_Param.bAbortRequestFlag == 1U)
  {
    /* Abort Command was already received from ClassReq or PC_to_RDR */
    if ((hccid->USBD_CCID_Param.bSeq == seq) && (BSlot == slot))
    {
      /* CLASS Specific request is already Received, Reset the abort flag */
      hccid->USBD_CCID_Param.bAbortRequestFlag = 0;
    }
  }
  else
  {
    /* Abort Command was NOT received from ClassReq or PC_to_RDR,
    so save them for next ABORT command to verify */
    hccid->USBD_CCID_Param.bAbortRequestFlag = 1U;
    hccid->USBD_CCID_Param.bSeq = seq;
    hccid->USBD_CCID_Param.bSlot = slot;
  }

  return 0;
}

/**
  * @brief  PC_TO_RDR_T0Apdu
  *         Execute the PC_TO_RDR_T0APDU command from host
  *         Response to this command message is the RDR_to_PC_SlotStatus
  * @param  pdev: device instance
  * @retval status of the command execution
  */
uint8_t  PC_TO_RDR_T0Apdu(USBD_HandleTypeDef *pdev)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint8_t error;

  error = CCID_CheckCommandParams(pdev, CHK_PARAM_SLOT | CHK_PARAM_CARD_PRESENT |
                                  CHK_PARAM_DWLENGTH | CHK_PARAM_ABORT);
  if (error != 0U)
  {
    return error;
  }
  if (hccid->UsbBlkOutData.bSpecific_0 > 0x03U)
  {
    /* Bit 0 is associated with bClassGetResponse
     Bit 1 is associated with bClassEnvelope */

    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_PRESENT_ACTIVE));
    return SLOTERROR_BAD_BMCHANGES;
  }

  error = SC_Itf_T0Apdu(hccid->UsbBlkOutData.bSpecific_0,
                        hccid->UsbBlkOutData.bSpecific_1,
                        hccid->UsbBlkOutData.bSpecific_2);

  if (error != SLOT_NO_ERROR)
  {
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_PRESENT_ACTIVE));
  }
  else
  {
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_NO_ERROR), (BM_ICC_PRESENT_ACTIVE));
  }

  return error;
}

/**
  * @brief  PC_TO_RDR_Mechanical
  *         Execute the PC_TO_RDR_MECHANICAL command from host
  *         Response to this command message is the RDR_to_PC_SlotStatus
  * @param  pdev: device instance
  * @retval status of the command execution
  */
uint8_t  PC_TO_RDR_Mechanical(USBD_HandleTypeDef *pdev)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint8_t error;

  error = CCID_CheckCommandParams(pdev, CHK_PARAM_SLOT | CHK_PARAM_CARD_PRESENT |
                                  CHK_PARAM_ABRFU2 | CHK_PARAM_DWLENGTH);
  if (error != 0U)
  {
    return error;
  }
  if (hccid->UsbBlkOutData.bSpecific_0 > 0x05U)
  {
    /*
     01h Accept Card
     02h Eject Card
     03h Capture Card
     04h Lock Card
     05h Unlock Card
    */

    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_PRESENT_ACTIVE));
    return SLOTERROR_BAD_BFUNCTION_MECHANICAL;
  }

  error = SC_Itf_Mechanical(hccid->UsbBlkOutData.bSpecific_0);

  if (error != SLOT_NO_ERROR)
  {
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_PRESENT_ACTIVE));
  }
  else
  {
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_NO_ERROR), (BM_ICC_PRESENT_ACTIVE));
  }

  return error;
}

/**
  * @brief  PC_TO_RDR_SetDataRateAndClockFrequency
  *         Set the required Card Frequency and Data rate from the host.
  *         Response to this command message is the
  *         RDR_to_PC_DataRateAndClockFrequency
  * @param  pdev: device instance
  * @retval status of the command execution
  */
uint8_t  PC_TO_RDR_SetDataRateAndClockFrequency(USBD_HandleTypeDef *pdev)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint8_t error;
  uint32_t clockFrequency;
  uint32_t dataRate;
  uint32_t temp;

  error = CCID_CheckCommandParams(pdev, CHK_PARAM_SLOT | CHK_PARAM_CARD_PRESENT |
                                  CHK_PARAM_ABRFU3);
  if (error != 0U)
  {
    return error;
  }
  if (hccid->UsbBlkOutData.dwLength != 0x08U)
  {
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_PRESENT_ACTIVE));
    return SLOTERROR_BAD_LENTGH;
  }

  /* HERE we avoiding to an unaligned memory access*/
  temp = (uint32_t)(hccid->UsbBlkOutData.abData[0]) & 0x000000FFU;
  clockFrequency = temp;

  temp = (uint32_t)(hccid->UsbBlkOutData.abData[1]) & 0x000000FFU;
  clockFrequency |= temp << 8;

  temp = (uint32_t)(hccid->UsbBlkOutData.abData[2]) & 0x000000FFU;
  clockFrequency |= temp << 16;

  temp = (uint32_t)(hccid->UsbBlkOutData.abData[3]) & 0x000000FFU;
  clockFrequency |= temp << 24;

  temp = (uint32_t)(hccid->UsbBlkOutData.abData[4]) & 0x000000FFU;
  dataRate = temp;

  temp = (uint32_t)(hccid->UsbBlkOutData.abData[5]) & 0x000000FFU;
  dataRate |= temp << 8;

  temp = (uint32_t)(hccid->UsbBlkOutData.abData[6]) & 0x000000FFU;
  dataRate |= temp << 16;

  temp = (uint32_t)(hccid->UsbBlkOutData.abData[7]) & 0x000000FFU;
  dataRate |= temp << 24;

  error = SC_Itf_SetDataRateAndClockFrequency(clockFrequency, dataRate);
  hccid->UsbBlkInData.bError = error;

  if (error != SLOT_NO_ERROR)
  {
    hccid->UsbBlkInData.dwLength = 0;
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_PRESENT_ACTIVE));
  }
  else
  {
    hccid->UsbBlkInData.dwLength = 8;

    (hccid->UsbBlkInData.abData[0])  = (uint8_t)(clockFrequency & 0x000000FFU) ;

    (hccid->UsbBlkInData.abData[1])  = (uint8_t)((clockFrequency & 0x0000FF00U) >> 8);

    (hccid->UsbBlkInData.abData[2])  = (uint8_t)((clockFrequency & 0x00FF0000U) >> 16);

    (hccid->UsbBlkInData.abData[3])  = (uint8_t)((clockFrequency & 0xFF000000U) >> 24);

    (hccid->UsbBlkInData.abData[4])  = (uint8_t)(dataRate & 0x000000FFU) ;

    (hccid->UsbBlkInData.abData[5])  = (uint8_t)((dataRate & 0x0000FF00U) >> 8);

    (hccid->UsbBlkInData.abData[6])  = (uint8_t)((dataRate & 0x00FF0000U) >> 16);

    (hccid->UsbBlkInData.abData[7])  = (uint8_t)((dataRate & 0xFF000000U) >> 24);

    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_NO_ERROR), (BM_ICC_PRESENT_ACTIVE));
  }

  return error;
}

/**
  * @brief  PC_TO_RDR_Secure
  *         Execute the Secure Command from the host.
  *         Response to this command message is the RDR_to_PC_DataBlock
  * @param  pdev: device instance
  * @retval status of the command execution
  */
uint8_t  PC_TO_RDR_Secure(USBD_HandleTypeDef *pdev)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint8_t error;
  uint8_t bBWI;
  uint16_t wLevelParameter;
  uint32_t responseLen;

  hccid->UsbBlkInData.dwLength = 0;

  error = CCID_CheckCommandParams(pdev, CHK_PARAM_SLOT | CHK_PARAM_CARD_PRESENT |
                                  CHK_PARAM_ABORT);

  if (error != 0U)
  {
    return error;
  }
  bBWI = hccid->UsbBlkOutData.bSpecific_0;
  wLevelParameter = (hccid->UsbBlkOutData.bSpecific_1 + ((uint16_t)hccid->UsbBlkOutData.bSpecific_2 << 8));

  if ((EXCHANGE_LEVEL_FEATURE == TPDU_EXCHANGE) ||
      (EXCHANGE_LEVEL_FEATURE == SHORT_APDU_EXCHANGE))
  {
    /* TPDU level & short APDU level, wLevelParameter is RFU, = 0000h */
    if (wLevelParameter != 0U)
    {
      CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_PRESENT_ACTIVE));
      error = SLOTERROR_BAD_LEVELPARAMETER;
      return error;
    }
  }

  error = SC_Itf_Secure(hccid->UsbBlkOutData.dwLength, bBWI, wLevelParameter,
                        &(hccid->UsbBlkOutData.abData[0]), &responseLen);

  hccid->UsbBlkInData.dwLength = responseLen;

  if (error != SLOT_NO_ERROR)
  {
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_PRESENT_ACTIVE));
  }
  else
  {
    CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_NO_ERROR), (BM_ICC_PRESENT_ACTIVE));
  }

  return error;
}

/******************************************************************************/
/*    BULK IN ROUTINES          */
/******************************************************************************/

/**
  * @brief  RDR_to_PC_DataBlock
  *         Provide the data block response to the host
  *         Response for PC_to_RDR_IccPowerOn, PC_to_RDR_XfrBlock
  * @param  errorCode: code to be returned to the host
  * @param  pdev: device instance
  * @retval None
  */
void RDR_to_PC_DataBlock(uint8_t  errorCode, USBD_HandleTypeDef *pdev)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint32_t length = CCID_RESPONSE_HEADER_SIZE;

  hccid->UsbBlkInData.bMessageType = RDR_TO_PC_DATABLOCK;
  hccid->UsbBlkInData.bError = 0U;
  hccid->UsbBlkInData.bSpecific = 0U;  /* bChainParameter */

  if (errorCode == SLOT_NO_ERROR)
  {
    length += hccid->UsbBlkInData.dwLength;   /* Length Specified in Command */
  }

  (void)USBD_CCID_Transfer_Data_Request(pdev, (uint8_t *)&hccid->UsbBlkInData, (uint16_t)length);
}

/**
  * @brief  RDR_to_PC_SlotStatus
  *         Provide the Slot status response to the host
  *          Response for PC_to_RDR_IccPowerOff
  *                PC_to_RDR_GetSlotStatus
  *                PC_to_RDR_IccClock
  *                PC_to_RDR_T0APDU
  *                PC_to_RDR_Mechanical
  *         Also the device sends this response message when it has completed
  *         aborting a slot after receiving both the Class Specific ABORT request
  *         and PC_to_RDR_Abort command message.
  * @param  errorCode: code to be returned to the host
  * @param  pdev: device instance
  * @retval None
  */
void RDR_to_PC_SlotStatus(uint8_t  errorCode, USBD_HandleTypeDef *pdev)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint16_t length = CCID_RESPONSE_HEADER_SIZE;

  hccid->UsbBlkInData.bMessageType = RDR_TO_PC_SLOTSTATUS;
  hccid->UsbBlkInData.dwLength = 0U;
  hccid->UsbBlkInData.bError = 0U;
  hccid->UsbBlkInData.bSpecific = 0U;    /* bClockStatus = 00h Clock running
  01h Clock stopped in state L
  02h Clock stopped in state H
  03h Clock stopped in an unknown state */

  if (errorCode == SLOT_NO_ERROR)
  {
    length += (uint16_t)hccid->UsbBlkInData.dwLength;
  }

  (void)USBD_CCID_Transfer_Data_Request(pdev, (uint8_t *)(&hccid->UsbBlkInData), length);
}

/**
  * @brief  RDR_to_PC_Parameters
  *         Provide the data block response to the host
  *         Response for PC_to_RDR_GetParameters, PC_to_RDR_ResetParameters
  *                      PC_to_RDR_SetParameters
  * @param  errorCode: code to be returned to the host
  * @param  pdev: device instance
  * @retval None
  */
void RDR_to_PC_Parameters(uint8_t  errorCode, USBD_HandleTypeDef *pdev)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint16_t length = CCID_RESPONSE_HEADER_SIZE;

  hccid->UsbBlkInData.bMessageType = RDR_TO_PC_PARAMETERS;
  hccid->UsbBlkInData.bError = 0U;

  if (errorCode == SLOT_NO_ERROR)
  {
    if (ProtocolNUM_OUT == 0x00U)
    {
      hccid->UsbBlkInData.dwLength = LEN_PROTOCOL_STRUCT_T0;
      length += (uint16_t)hccid->UsbBlkInData.dwLength;
    }
    else
    {
      hccid->UsbBlkInData.dwLength = LEN_PROTOCOL_STRUCT_T1;
      length += (uint16_t)hccid->UsbBlkInData.dwLength;
    }
  }
  else
  {
    hccid->UsbBlkInData.dwLength = 0;
  }

  hccid->UsbBlkInData.abData[0] = ProtocolData.bmFindexDindex;
  hccid->UsbBlkInData.abData[1] = ProtocolData.bmTCCKST0;
  hccid->UsbBlkInData.abData[2] = ProtocolData.bGuardTimeT0;
  hccid->UsbBlkInData.abData[3] = ProtocolData.bWaitingIntegerT0;
  hccid->UsbBlkInData.abData[4] = ProtocolData.bClockStop;
  hccid->UsbBlkInData.abData[5] = ProtocolData.bIfsc;
  hccid->UsbBlkInData.abData[6] = ProtocolData.bNad;

  /* bProtocolNum */
  if (ProtocolNUM_OUT == 0x00U)
  {
    hccid->UsbBlkInData.bSpecific = BPROTOCOL_NUM_T0;
  }
  else
  {
    hccid->UsbBlkInData.bSpecific = BPROTOCOL_NUM_T1;
  }
  (void)USBD_CCID_Transfer_Data_Request(pdev, (uint8_t *)(&hccid->UsbBlkInData), length);
}

/**
  * @brief  RDR_to_PC_Escape
  *         Provide the Escaped data block response to the host
  *         Response for PC_to_RDR_Escape
  * @param  errorCode: code to be returned to the host
  * @param  pdev: device instance
  * @retval None
  */
void RDR_to_PC_Escape(uint8_t  errorCode, USBD_HandleTypeDef *pdev)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint32_t length = CCID_RESPONSE_HEADER_SIZE;

  hccid->UsbBlkInData.bMessageType = RDR_TO_PC_ESCAPE;

  hccid->UsbBlkInData.bSpecific = 0U;  /* Reserved for Future Use */
  hccid->UsbBlkInData.bError = errorCode;

  if (errorCode == SLOT_NO_ERROR)
  {
    length += hccid->UsbBlkInData.dwLength;   /* Length Specified in Command */
  }

  (void)USBD_CCID_Transfer_Data_Request(pdev, (uint8_t *)(&hccid->UsbBlkInData), (uint16_t)length);
}

/**
  * @brief  RDR_to_PC_DataRateAndClockFrequency
  *         Provide the Clock and Data Rate information to host
  *         Response for PC_TO_RDR_SetDataRateAndClockFrequency
  * @param  errorCode: code to be returned to the host
  * @param  pdev: device instance
  * @retval None
  */
void RDR_to_PC_DataRateAndClockFrequency(uint8_t  errorCode, USBD_HandleTypeDef *pdev)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint32_t length = CCID_RESPONSE_HEADER_SIZE;

  hccid->UsbBlkInData.bMessageType = RDR_TO_PC_DATARATEANDCLOCKFREQUENCY;
  hccid->UsbBlkInData.bError = errorCode;
  hccid->UsbBlkInData.bSpecific = 0U;  /* Reserved for Future Use */

  if (errorCode == SLOT_NO_ERROR)
  {
    length += hccid->UsbBlkInData.dwLength;   /* Length Specified in Command */
  }

  (void)USBD_CCID_Transfer_Data_Request(pdev, (uint8_t *)(&hccid->UsbBlkInData), (uint16_t)length);
}

/**
  * @brief  RDR_to_PC_NotifySlotChange
  *         Interrupt message to be sent to the host, Checks the card presence
  *         status and update the buffer accordingly
  * @param  pdev: device instance
  * @retval None
  */
void RDR_to_PC_NotifySlotChange(USBD_HandleTypeDef  *pdev)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  hccid->UsbIntData[OFFSET_INT_BMESSAGETYPE] = RDR_TO_PC_NOTIFYSLOTCHANGE;

  if (SC_Detect() != 0U)
  {
    /*
    SLOT_ICC_PRESENT 0x01 : LSb : (0b = no ICC present, 1b = ICC present)
    SLOT_ICC_CHANGE 0x02 : MSb : (0b = no change, 1b = change).
    */
    hccid->UsbIntData[OFFSET_INT_BMSLOTICCSTATE] = SLOT_ICC_PRESENT | SLOT_ICC_CHANGE;
  }
  else
  {
    hccid->UsbIntData[OFFSET_INT_BMSLOTICCSTATE] = SLOT_ICC_CHANGE;

    /* Power OFF the card */
    SC_Itf_IccPowerOff();
  }
}


/**
  * @brief  CCID_UpdSlotStatus
  *         Updates the variable for the slot status
  * @param  pdev: device instance
  * @param  slotStatus : slot status from the calling function
  * @retval None
  */
void CCID_UpdSlotStatus(USBD_HandleTypeDef *pdev, uint8_t slotStatus)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  hccid->SlotStatus.SlotStatus = slotStatus;
}

/**
  * @brief  CCID_UpdSlotChange
  *         Updates the variable for the slot change status
  * @param  pdev: device instance
  * @param  changeStatus : slot change status from the calling function
  * @retval None
  */
void CCID_UpdSlotChange(USBD_HandleTypeDef *pdev, uint8_t changeStatus)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  hccid->SlotStatus.SlotStatusChange  = changeStatus;
}

/**
  * @brief  CCID_IsSlotStatusChange
  *         Provides the value of the variable for the slot change status
  * @param  pdev: device instance
  * @retval slot change status
  */
uint8_t CCID_IsSlotStatusChange(USBD_HandleTypeDef *pdev)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  return hccid->SlotStatus.SlotStatusChange;
}

/**
  * @brief  CCID_UpdateCommandStatus
  *         Updates the variable for the BulkIn status
  * @param  pdev: device instance
  * @param  cmd_status : Command change status from the calling function
  * @param  icc_status : Slot change status from the calling function
  * @retval None
  */
static void CCID_UpdateCommandStatus(USBD_HandleTypeDef  *pdev, uint8_t cmd_status, uint8_t icc_status)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  hccid->UsbBlkInData.bStatus = (cmd_status | icc_status);
}
/**
  * @brief  CCID_CheckCommandParams
  *         Checks the specific parameters requested by the function and update
  *          status accordingly. This function is called from all
  *          PC_to_RDR functions
  * @param  pdev: device instance
  * @param  param_type : Parameter enum to be checked by calling function
  * @retval status
  */
static uint8_t CCID_CheckCommandParams(USBD_HandleTypeDef  *pdev, uint32_t param_type)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint32_t parameter;
  uint8_t GetState = SC_GetState();

  hccid->UsbBlkInData.bStatus = BM_ICC_PRESENT_ACTIVE | BM_COMMAND_STATUS_NO_ERROR;

  parameter = (uint32_t)param_type;

  if ((parameter & CHK_PARAM_SLOT) != 0U)
  {
    /*
    The slot number (bSlot) identifies which ICC slot is being addressed
    by the message*/

    /* SLOT Number is 0 onwards, so always < CCID_NUMBER_OF_SLOTs */
    /* Error Condition !!! */
    if (hccid->UsbBlkOutData.bSlot >= CCID_NUMBER_OF_SLOTS)
    {
      /* Slot requested is more than supported by Firmware */
      CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_NO_ICC_PRESENT));
      return SLOTERROR_BAD_SLOT;
    }
  }

  if ((parameter & CHK_PARAM_CARD_PRESENT) != 0U)
  {
    /* Commands Parameters ok, Check the Card Status */
    if (SC_Detect() == 0U)
    {
      /* Card is Not detected */
      CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_NO_ICC_PRESENT));
      return SLOTERROR_ICC_MUTE;
    }
  }

  /* Check that DwLength is 0 */
  if ((parameter & CHK_PARAM_DWLENGTH) != 0U)
  {
    if (hccid->UsbBlkOutData.dwLength != 0U)
    {
      CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_PRESENT_ACTIVE));
      return SLOTERROR_BAD_LENTGH;
    }
  }

  /* abRFU 2 : Reserved for Future Use*/
  if ((parameter & CHK_PARAM_ABRFU2) != 0U)
  {

    if ((hccid->UsbBlkOutData.bSpecific_1 != 0U) || (hccid->UsbBlkOutData.bSpecific_2 != 0U))
    {
      CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_PRESENT_ACTIVE));
      return SLOTERROR_BAD_ABRFU_2B;        /* bSpecific_1 */
    }
  }

  if ((parameter & CHK_PARAM_ABRFU3) != 0U)
  {
    /* abRFU 3 : Reserved for Future Use*/
    if ((hccid->UsbBlkOutData.bSpecific_0 != 0U) ||
        (hccid->UsbBlkOutData.bSpecific_1 != 0U) ||
        (hccid->UsbBlkOutData.bSpecific_2 != 0U))
    {
      CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_PRESENT_ACTIVE));
      return SLOTERROR_BAD_ABRFU_3B;
    }
  }

  if ((parameter & CHK_PARAM_ABORT) != 0U)
  {
    if (hccid->USBD_CCID_Param.bAbortRequestFlag != 0U)
    {
      CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_PRESENT_INACTIVE));
      return SLOTERROR_CMD_ABORTED;
    }
  }

  if ((parameter & CHK_ACTIVE_STATE) != 0U)
  {
    /* Commands Parameters ok, Check the Card Status */
    /* Card is detected */

    if ((GetState != (uint8_t)SC_ACTIVE_ON_T0) && (GetState != (uint8_t)SC_ACTIVE_ON_T1))
    {
      /* Check that from Lower Layers, the SmartCard come to known state */
      CCID_UpdateCommandStatus(pdev, (BM_COMMAND_STATUS_FAILED), (BM_ICC_PRESENT_INACTIVE));
      return SLOTERROR_HW_ERROR;
    }
  }

  return 0U;
}
