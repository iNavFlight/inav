/**
  ******************************************************************************
  * @file    usbd_ccid_sc_if_template.c
  * @author  MCD Application Team
  * @brief   SmartCard Interface file
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
#include "usbd_ccid_sc_if_template.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* State Machine for the SmartCard Interface */
static SC_State SCState = SC_POWER_OFF;

/* APDU Transport Structures */
SC_ADPU_CommandsTypeDef SC_ADPU;
SC_ADPU_ResponseTypeDef SC_Response;
SC_Param_t  SC_Param;
Protocol_01_DataTypeDef ProtocolData;

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void SC_SaveVoltage(uint8_t voltage);
static void SC_Itf_UpdateParams(void);

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  SC_Itf_IccPowerOn Manages the Warm and Cold Reset
            and get the Answer to Reset from ICC
  * @param  voltage: required by host
  * @retval None
  */
void SC_Itf_IccPowerOn(uint8_t voltage)
{
  SCState = SC_POWER_ON;
  SC_ADPU.Header.CLA = 0x00U;
  SC_ADPU.Header.INS = SC_GET_A2R;
  SC_ADPU.Header.P1 = 0x00U;
  SC_ADPU.Header.P2 = 0x00U;
  SC_ADPU.Body.LC = 0x00U;

  /* Power ON the card */
  SC_PowerCmd(SC_ENABLED);

  /* Configure the Voltage, Even if IO is still not configured */
  SC_VoltageConfig(voltage);

  while ((SCState != SC_ACTIVE_ON_T0) && (SCState != SC_ACTIVE_ON_T1)
         && (SCState != SC_NO_INIT))
  {
    /* If Either The Card has become Active or Become De-Active */
    SC_Handler(&SCState, &SC_ADPU, &SC_Response);
  }

  if ((SCState == SC_ACTIVE_ON_T0) || (SCState == SC_ACTIVE_ON_T1))
  {
    SC_Itf_UpdateParams();
    /* Apply the Procedure Type Selection (PTS) */
    SC_PTSConfig();

    /* Save Voltage for Future use */
    SC_SaveVoltage(voltage);
  }

  return;
}

/**
  * @brief  SC_Itf_IccPowerOff Power OFF the card
  * @param  None
  * @retval None
  */
void SC_Itf_IccPowerOff(void)
{
  SC_PowerCmd(SC_DISABLED);
  SC_SetState(SC_POWER_OFF);

  return;
}

/**
  * @brief  Initialize the parameters structures to the default value
  * @param  None
  * @retval None
  */
void SC_Itf_InitParams(void)
{
  /*
  FI, the reference to a clock rate conversion factor
  over the bits b8 to b5
  - DI, the reference to a baud rate adjustment factor
  over the bits b4 to bl
  */
  SC_Param.SC_A2R_FiDi = DEFAULT_FIDI;
  SC_Param.SC_hostFiDi = DEFAULT_FIDI;

  ProtocolData.bmFindexDindex = DEFAULT_FIDI;

  /* Placeholder, Ignored */
  /* 0 = Direct, first byte of the ICC ATR data. */
  ProtocolData.bmTCCKST0 = DEFAULT_T01CONVCHECKSUM;

  /* Extra GuardTime = 0 etu */
  ProtocolData.bGuardTimeT0 = DEFAULT_EXTRA_GUARDTIME;
  ProtocolData.bWaitingIntegerT0 = DEFAULT_WAITINGINTEGER;
  ProtocolData.bClockStop = 0U; /* Stopping the Clock is not allowed */

  /*T=1 protocol */
  ProtocolData.bIfsc = DEFAULT_IFSC;
  ProtocolData.bNad = DEFAULT_NAD;

  return;
}

/**
  * @brief  Save the A2R Parameters for further usage
  * @param  None
  * @retval None
  */
static void SC_Itf_UpdateParams(void)
{
  /*
  FI, the reference to a clock rate conversion factor
  over the bits b8 to b5
  DI, the reference to a baud rate adjustment factor
  over the bits b4 to bl
  */
  SC_Param.SC_A2R_FiDi = SC_A2R.T[0].InterfaceByte[0].Value;
  SC_Param.SC_hostFiDi = SC_A2R.T[0].InterfaceByte[0].Value;

  ProtocolData.bmFindexDindex = SC_A2R.T[0].InterfaceByte[0].Value;

  return;
}

/**
  * @brief  SC_Itf_SetParams
  *         Set the parameters for CCID/USART interface
  * @param  pPtr: pointer to buffer containing the
  *          parameters to be set in USART
  * @param  T_01: type of protocol, T=1 or T=0
  * @retval status value
  */
uint8_t SC_Itf_SetParams(Protocol_01_DataTypeDef *pPtr, uint8_t T_01)
{
  /* uint16_t guardTime; */   /* Keep it 16b for handling 8b additions */
  uint32_t fi_new;
  uint32_t di_new;
  Protocol_01_DataTypeDef New_DataStructure;
  fi_new = pPtr->bmFindexDindex;
  di_new = pPtr->bmFindexDindex;

  New_DataStructure.bmTCCKST0 = pPtr->bmTCCKST0;

  New_DataStructure.bGuardTimeT0 = pPtr->bGuardTimeT0;
  New_DataStructure.bWaitingIntegerT0 = pPtr->bWaitingIntegerT0;
  New_DataStructure.bClockStop = pPtr->bClockStop;
  if (T_01 == 0x01U)
  {
    New_DataStructure.bIfsc = pPtr->bIfsc;
    New_DataStructure.bNad = pPtr->bNad;
  }
  else
  {
    New_DataStructure.bIfsc = 0x00U;
    New_DataStructure.bNad = 0x00U;
  }

  /* Check for the FIDI Value set by Host */
  di_new &= (uint8_t)0x0F;
  if (SC_GetDTableValue(di_new) == 0U)
  {
    return SLOTERROR_BAD_FIDI;
  }

  fi_new >>= 4U;
  fi_new &= 0x0FU;

  if (SC_GetDTableValue(fi_new) == 0U)
  {
    return SLOTERROR_BAD_FIDI;
  }

  if ((T_01 == 0x00U)
      && (New_DataStructure.bmTCCKST0 != 0x00U)
      && (New_DataStructure.bmTCCKST0 != 0x02U))
  {
    return SLOTERROR_BAD_T01CONVCHECKSUM;
  }

  if ((T_01 == 0x01U)
      && (New_DataStructure.bmTCCKST0 != 0x10U)
      && (New_DataStructure.bmTCCKST0 != 0x11U)
      && (New_DataStructure.bmTCCKST0 != 0x12U)
      && (New_DataStructure.bmTCCKST0 != 0x13U))
  {
    return SLOTERROR_BAD_T01CONVCHECKSUM;
  }

  if ((New_DataStructure.bWaitingIntegerT0 >= 0xA0U)
      && ((New_DataStructure.bmTCCKST0 & 0x10U) == 0x10U))
  {
    return SLOTERROR_BAD_WAITINGINTEGER;
  }
  if ((New_DataStructure.bClockStop != 0x00U)
      && (New_DataStructure.bClockStop != 0x03U))
  {
    return SLOTERROR_BAD_CLOCKSTOP;
  }
  if (New_DataStructure.bNad != 0x00U)
  {
    return SLOTERROR_BAD_NAD;
  }
  /* Put Total GuardTime in USART Settings */
  /* USART_SetGuardTime(SC_USART, (uint8_t)(guardTime + DEFAULT_EXTRA_GUARDTIME)); */

  /* Save Extra GuardTime Value */
  ProtocolData.bGuardTimeT0 = New_DataStructure.bGuardTimeT0;
  ProtocolData.bmTCCKST0 = New_DataStructure.bmTCCKST0;
  ProtocolData.bWaitingIntegerT0 = New_DataStructure.bWaitingIntegerT0;
  ProtocolData.bClockStop = New_DataStructure.bClockStop;
  ProtocolData.bIfsc = New_DataStructure.bIfsc;
  ProtocolData.bNad = New_DataStructure.bNad;

  /* Save New bmFindexDindex */
  SC_Param.SC_hostFiDi = pPtr->bmFindexDindex;
  SC_PTSConfig();

  ProtocolData.bmFindexDindex = pPtr->bmFindexDindex;

  return SLOT_NO_ERROR;
}

/**
  * @brief  SC_Itf_Escape function from the host
  *         This is user implementable
  * @param  ptrEscape: pointer to buffer containing the Escape data
  * @param  escapeLen: length of escaped data
  * @param  responseBuff: pointer containing escape buffer response
  * @param  responseLen: length of escape response buffer
  * @retval status value
  */
uint8_t SC_Itf_Escape(uint8_t *ptrEscape, uint32_t escapeLen,
                      uint8_t *responseBuff, uint32_t *responseLen)
{
  UNUSED(ptrEscape);
  UNUSED(escapeLen);
  UNUSED(responseBuff);
  UNUSED(responseLen);

  /* Manufacturer specific implementation ... */
  /*
   uint32_t idx;
   uint8_t *pResBuff = responseBuff;
   uint8_t *pEscape = ptrEscape;

   for(idx = 0; idx < escapeLen; idx++)
   {
     *pResBuff = *pEscape;
     pResBuff++;
     pEscape++;
   }

   *responseLen = escapeLen;
  */
  return SLOT_NO_ERROR;
}

/**
  * @brief  SC_Itf_SetClock function to define Clock Status request from the host.
  *         This is user implementable
  * @param  bClockCommand: Clock status from the host
  * @retval status value
  */
uint8_t SC_Itf_SetClock(uint8_t bClockCommand)
{
  /* bClockCommand
  00h restarts Clock
  01h Stops Clock in the state shown in the bClockStop
  field of the PC_to_RDR_SetParameters command
  and RDR_to_PC_Parameters message.*/

  if (bClockCommand == 0U)
  {
    /* 00h restarts Clock : Since Clock is always running, PASS this command */
    return SLOT_NO_ERROR;
  }
  else
  {
    if (bClockCommand == 1U)
    {
      return SLOTERROR_BAD_CLOCKCOMMAND;
    }
  }

  return SLOTERROR_CMD_NOT_SUPPORTED;
}

/**
  * @brief  SC_Itf_XferBlock function from the host.
  *         This is user implementable
  * @param  ptrBlock : Pointer containing the data from host
  * @param  blockLen : length of block data for the data transfer
  * @param  expectedLen: expected length of data transfer
  * @param  CCID_BulkIn_Data: Pointer containing the CCID Bulk In Data Structure
  * @retval status value
  */
uint8_t SC_Itf_XferBlock(uint8_t *ptrBlock, uint32_t blockLen, uint16_t expectedLen,
                         USBD_CCID_BulkIn_DataTypeDef *CCID_BulkIn_Data)
{
  uint8_t ErrorCode = SLOT_NO_ERROR;
  UNUSED(CCID_BulkIn_Data);
  UNUSED(expectedLen);
  UNUSED(blockLen);
  UNUSED(ptrBlock);

  if (ProtocolNUM_OUT == 0x00U)
  {
    /* Add your code here */
  }

  if (ProtocolNUM_OUT == 0x01U)
  {
    /* Add your code here */
  }

  if (ErrorCode != SLOT_NO_ERROR)
  {
    return ErrorCode;
  }

  return ErrorCode;
}


/**
  * @brief  SC_Itf_T0Apdu
            Class Specific Request from the host to provide supported data rates
  *         This is Optional function & user implementable
  * @param  bmChanges : value specifying which parameter is valid in
  *                    command among next bClassGetResponse, bClassEnvelope
  * @param  bClassGetResponse : Value to force the class byte of the
  *                     header in a Get Response command.
  * @param  bClassEnvelope : Value to force the class byte of the header
  *                     in a Envelope command.
  * @retval status value
  */
uint8_t SC_Itf_T0Apdu(uint8_t bmChanges, uint8_t bClassGetResponse,
                      uint8_t bClassEnvelope)
{
  UNUSED(bClassEnvelope);
  UNUSED(bClassGetResponse);

  /* User have to fill the pbuf with the GetDataRates data buffer */

  if (bmChanges == 0U)
  {
    /* Bit cleared indicates that the associated field is not significant and
    that default behaviour defined in CCID class descriptor is selected */
    return SLOT_NO_ERROR;
  }

  return SLOTERROR_CMD_NOT_SUPPORTED;
}

/**
  * @brief  SC_Itf_Mechanical
            Mechanical Function being requested by Host
  *         This is Optional function & user implementable
  * @param  bFunction : value corresponds to the mechanical function
  *                       being requested by host
  * @retval status value
  */
uint8_t SC_Itf_Mechanical(uint8_t bFunction)
{
  UNUSED(bFunction);

  return SLOTERROR_CMD_NOT_SUPPORTED;
}

/**
  * @brief  SC_Itf_SetDataRateAndClockFrequency
  *         Set the Clock and data Rate of the Interface
  *         This is Optional function & user implementable
  * @param  dwClockFrequency : value of clock in kHz requested by host
  * @param  dwDataRate : value of data rate requested by host
  * @retval status value
  */
uint8_t SC_Itf_SetDataRateAndClockFrequency(uint32_t dwClockFrequency,
                                            uint32_t dwDataRate)
{
  /* User have to fill the pbuf with the GetDataRates data buffer */

  if ((dwDataRate == USBD_CCID_DEFAULT_DATA_RATE) &&
      (dwClockFrequency == USBD_CCID_DEFAULT_CLOCK_FREQ))
  {
    return SLOT_NO_ERROR;
  }

  return SLOTERROR_CMD_NOT_SUPPORTED;
}

/**
  * @brief  SC_Itf_Secure
  *         Process the Secure command
  *          This is Optional function & user implementable
  * @param  dwLength : length of data from the host
  * @param  bBWI : Block Waiting Timeout sent by host
  * @param  wLevelParameter : Parameters sent by host
  * @param  pbuf : buffer containing the data
  * @param  returnLen : Length of data expected to return
  * @retval status value
  */
uint8_t SC_Itf_Secure(uint32_t dwLength, uint8_t bBWI, uint16_t wLevelParameter,
                      uint8_t *pbuf, uint32_t *returnLen)
{
  UNUSED(pbuf);
  UNUSED(wLevelParameter);
  UNUSED(bBWI);
  UNUSED(dwLength);
  *returnLen = 0U;

  return SLOTERROR_CMD_NOT_SUPPORTED;
}

/**
  * @brief  SC_SaveVoltage
            Saves the voltage value to be saved for further usage
  * @param  voltage: voltage value to be saved for further usage
  * @retval None
  */
static void SC_SaveVoltage(uint8_t voltage)
{
  SC_Param.voltage = voltage;

  return;
}

/**
  * @brief  Provides the value of SCState variable
  * @param  None
  * @retval uint8_t SCState
  */
uint8_t SC_GetState(void)
{
  return (uint8_t)SCState;
}

/**
  * @brief  Set the value of SCState variable to Off
  * @param  scState: value of SCState to be updated
  * @retval None
  */
void SC_SetState(SC_State scState)
{
  SCState = scState;

  return;
}
