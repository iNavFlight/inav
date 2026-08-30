/**
  ******************************************************************************
  * @file    usbd_ccid_cmd.h
  * @author  MCD Application Team
  * @brief   header file for the usbd_ccid_cmd.c file.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CCID_CMD_H
#define __USBD_CCID_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#ifndef __USBD_CCID_IF_H
#include "usbd_ccid_if_template.h"
#endif /* __USBD_CCID_IF_H */

#ifndef __USBD_CCID_SC_IF_H
#include "usbd_ccid_sc_if_template.h"
#endif /* __USBD_CCID_SC_IF_H */


/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

/******************************************************************************/
/*  ERROR CODES for USB Bulk In Messages : bError                             */
/******************************************************************************/

#define   SLOT_NO_ERROR                           0x81U
#define   SLOTERROR_UNKNOWN                       0x82U
/*----------------------------------------------------------------------------*/

/* Index of not supported / incorrect message parameter : 7Fh to 01h */
/* These Values are used for Return Types between Firmware Layers    */
/*
  Failure of a command
  The CCID cannot parse one parameter or the ICC is not supporting one parameter.
  Then the Slot Error register contains the index of the first bad parameter as a
  positive number (1-127). For instance, if the CCID receives an ICC command to
  an unimplemented slot, then the Slot Error register shall be set to 5 (index of bSlot field) */

/*
 * CCID Class specification revision 1.1
 */

/* Following Parameters used in PC_to_RDR_XfrBlock */
#define   SLOTERROR_BAD_LENTGH                     0x01U
#define   SLOTERROR_BAD_SLOT                       0x05U
#define   SLOTERROR_BAD_POWERSELECT                0x07U
#define   SLOTERROR_BAD_PROTOCOLNUM                0x07U
#define   SLOTERROR_BAD_CLOCKCOMMAND               0x07U
#define   SLOTERROR_BAD_ABRFU_3B                   0x07U
#define   SLOTERROR_BAD_BMCHANGES                  0x07U
#define   SLOTERROR_BAD_BFUNCTION_MECHANICAL       0x07U
#define   SLOTERROR_BAD_ABRFU_2B                   0x08U
#define   SLOTERROR_BAD_LEVELPARAMETER             0x08U
#define   SLOTERROR_BAD_FIDI                       0x0AU
#define   SLOTERROR_BAD_T01CONVCHECKSUM            0x0BU
#define   SLOTERROR_BAD_GUARDTIME                  0x0CU
#define   SLOTERROR_BAD_WAITINGINTEGER             0x0DU
#define   SLOTERROR_BAD_CLOCKSTOP                  0x0EU
#define   SLOTERROR_BAD_IFSC                       0x0FU
#define   SLOTERROR_BAD_NAD                        0x10U
#define   SLOTERROR_BAD_DWLENGTH                   0x08U


/*----------  Table 6.2-2 Slot error register when bmCommandStatus = 1        */
#define   SLOTERROR_CMD_ABORTED                    0xFFU
#define   SLOTERROR_ICC_MUTE                       0xFEU
#define   SLOTERROR_XFR_PARITY_ERROR               0xFDU
#define   SLOTERROR_XFR_OVERRUN                    0xFCU
#define   SLOTERROR_HW_ERROR                       0xFBU
#define   SLOTERROR_BAD_ATR_TS                     0xF8U
#define   SLOTERROR_BAD_ATR_TCK                    0xF7U
#define   SLOTERROR_ICC_PROTOCOL_NOT_SUPPORTED     0xF6U
#define   SLOTERROR_ICC_CLASS_NOT_SUPPORTED        0xF5U
#define   SLOTERROR_PROCEDURE_BYTE_CONFLICT        0xF4U
#define   SLOTERROR_DEACTIVATED_PROTOCOL           0xF3U
#define   SLOTERROR_BUSY_WITH_AUTO_SEQUENCE        0xF2U
#define   SLOTERROR_PIN_TIMEOUT                    0xF0U
#define   SLOTERROR_PIN_CANCELLED                  0xEFU
#define   SLOTERROR_CMD_SLOT_BUSY                  0xE0U
#define   SLOTERROR_CMD_NOT_SUPPORTED              0x00U

/* Following Parameters used in PC_to_RDR_ResetParameters */
/* DEFAULT_FIDI_VALUE */
#ifndef   DEFAULT_FIDI
#define   DEFAULT_FIDI                             0x11U
#endif /* DEFAULT_FIDI */
#ifndef   DEFAULT_T01CONVCHECKSUM
#define   DEFAULT_T01CONVCHECKSUM                  0x00U
#endif /* DEFAULT_T01CONVCHECKSUM */
#ifndef   DEFAULT_EXTRA_GUARDTIME
#define   DEFAULT_EXTRA_GUARDTIME                  0x00U
#endif /* DEFAULT_EXTRA_GUARDTIME */
#ifndef   DEFAULT_WAITINGINTEGER
#define   DEFAULT_WAITINGINTEGER                   0x0AU
#endif /* DEFAULT_WAITINGINTEGER */
#ifndef   DEFAULT_CLOCKSTOP
#define   DEFAULT_CLOCKSTOP                        0x00U
#endif /* DEFAULT_CLOCKSTOP */
#ifndef   DEFAULT_IFSC
#define   DEFAULT_IFSC                             0x20U
#endif /* DEFAULT_IFSC */
#ifndef   DEFAULT_NAD
#define   DEFAULT_NAD                              0x00U
#endif /* DEFAULT_NAD */

/* Following Parameters used in PC_to_RDR_IccPowerOn */
#define VOLTAGE_SELECTION_AUTOMATIC               0xFFU
#define VOLTAGE_SELECTION_3V                      0x02U
#define VOLTAGE_SELECTION_5V                      0x01U
#define VOLTAGE_SELECTION_1V8                     0x03U

/*
Offset=0 bmICCStatus 2 bit  0, 1, 2
    0 - An ICC is present and active (power is on and stable, RST is inactive)
    1 - An ICC is present and inactive (not activated or shut down by hardware error)
    2 - No ICC is present
    3 - RFU
Offset=0 bmRFU 4 bits 0 RFU
Offset=6 bmCommandStatus 2 bits 0, 1, 2
    0 - Processed without error
    1 - Failed (error code provided by the error register)
    2 - Time Extension is requested
    3 - RFU
  */

#define BM_ICC_PRESENT_ACTIVE                    0x00U
#define BM_ICC_PRESENT_INACTIVE                  0x01U
#define BM_ICC_NO_ICC_PRESENT                    0x02U

#define BM_COMMAND_STATUS_OFFSET                 0x06U
#define BM_COMMAND_STATUS_NO_ERROR               0x00U
#define BM_COMMAND_STATUS_FAILED            (0x01U << BM_COMMAND_STATUS_OFFSET)
#define BM_COMMAND_STATUS_TIME_EXTN         (0x02 << BM_COMMAND_STATUS_OFFSET)


#if (ATR_T01 == 0)
#define SIZE_OF_ATR                                19U
#else
#define SIZE_OF_ATR                                15U
#endif /* (ATR_T01 == 0) */

/* defines for the CCID_CMD Layers */
#define LEN_PROTOCOL_STRUCT_T0                      5U
#define LEN_PROTOCOL_STRUCT_T1                      7U

#define BPROTOCOL_NUM_T0                            0U
#define BPROTOCOL_NUM_T1                            1U

/************************************************************************************/
/*   ERROR CODES for RDR_TO_PC_HARDWAREERROR Message : bHardwareErrorCode           */
/************************************************************************************/

#define   HARDWAREERRORCODE_OVERCURRENT           0x01U
#define   HARDWAREERRORCODE_VOLTAGEERROR          0x02U
#define   HARDWAREERRORCODE_OVERCURRENT_IT        0x04U
#define   HARDWAREERRORCODE_VOLTAGEERROR_IT       0x08U



#define  CHK_PARAM_SLOT                           0x01U
#define  CHK_PARAM_DWLENGTH                       0x02U
#define  CHK_PARAM_ABRFU2                         0x04U
#define  CHK_PARAM_ABRFU3                         0x08U
#define  CHK_PARAM_CARD_PRESENT                   0x10U
#define  CHK_PARAM_ABORT                          0x20U
#define  CHK_ACTIVE_STATE                         0x40U


/* Exported functions ------------------------------------------------------- */
uint8_t  PC_to_RDR_IccPowerOn(USBD_HandleTypeDef *pdev);
uint8_t  PC_to_RDR_IccPowerOff(USBD_HandleTypeDef *pdev);
uint8_t  PC_to_RDR_GetSlotStatus(USBD_HandleTypeDef *pdev);
uint8_t  PC_to_RDR_XfrBlock(USBD_HandleTypeDef *pdev);
uint8_t  PC_to_RDR_GetParameters(USBD_HandleTypeDef *pdev);
uint8_t  PC_to_RDR_ResetParameters(USBD_HandleTypeDef *pdev);
uint8_t  PC_to_RDR_SetParameters(USBD_HandleTypeDef *pdev);
uint8_t  PC_to_RDR_Escape(USBD_HandleTypeDef *pdev);
uint8_t  PC_to_RDR_IccClock(USBD_HandleTypeDef *pdev);
uint8_t  PC_to_RDR_Abort(USBD_HandleTypeDef *pdev);
uint8_t  PC_TO_RDR_T0Apdu(USBD_HandleTypeDef *pdev);
uint8_t  PC_TO_RDR_Mechanical(USBD_HandleTypeDef *pdev);
uint8_t  PC_TO_RDR_SetDataRateAndClockFrequency(USBD_HandleTypeDef *pdev);
uint8_t  PC_TO_RDR_Secure(USBD_HandleTypeDef *pdev);

void RDR_to_PC_DataBlock(uint8_t errorCode, USBD_HandleTypeDef *pdev);
void RDR_to_PC_NotifySlotChange(USBD_HandleTypeDef *pdev);
void RDR_to_PC_SlotStatus(uint8_t errorCode, USBD_HandleTypeDef *pdev);
void RDR_to_PC_Parameters(uint8_t errorCode, USBD_HandleTypeDef *pdev);
void RDR_to_PC_Escape(uint8_t errorCode, USBD_HandleTypeDef *pdev);
void RDR_to_PC_DataRateAndClockFrequency(uint8_t errorCode, USBD_HandleTypeDef *pdev);

void CCID_UpdSlotStatus(USBD_HandleTypeDef *pdev, uint8_t slotStatus);
void CCID_UpdSlotChange(USBD_HandleTypeDef *pdev, uint8_t changeStatus);
uint8_t CCID_IsSlotStatusChange(USBD_HandleTypeDef *pdev);
uint8_t CCID_CmdAbort(USBD_HandleTypeDef *pdev, uint8_t slot, uint8_t seq);
uint8_t USBD_CCID_Transfer_Data_Request(USBD_HandleTypeDef *pdev,
                                        uint8_t *dataPointer, uint16_t dataLen);


#ifdef __cplusplus
}
#endif

#endif /* __USBD_CCID_CMD_H */
