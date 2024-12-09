/**
  ******************************************************************************
  * @file    usbd_ccid_sc_if_template.h
  * @author  MCD Application Team
  * @brief   header file for the usbd_ccid_sc_if_template.c file.
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
#ifndef __USBD_CCID_SC_IF_TEMPLATE_H
#define __USBD_CCID_SC_IF_TEMPLATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_ccid.h"
#include "usbd_ccid_cmd.h"

#ifndef __USBD_CCID_SMARTCARD_H
#include "usbd_ccid_smartcard_template.h"
#endif /* __USBD_CCID_SMARTCARD_H */

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef struct
{
  uint8_t voltage;  /* Voltage for the Card Already Selected */
  uint8_t USART_GuardTime;
  uint8_t SC_A2R_FiDi;
  uint8_t SC_hostFiDi;
  uint8_t USART_DefaultGuardTime;
  uint32_t USART_BaudRate;
} SC_Param_t;

#pragma pack(1)
typedef struct
{
  uint8_t bmFindexDindex;
  uint8_t bmTCCKST0;
  uint8_t bGuardTimeT0;
  uint8_t bWaitingIntegerT0;
  uint8_t bClockStop;
  uint8_t bIfsc;
  uint8_t bNad;
} Protocol_01_DataTypeDef;
#pragma pack()

extern Protocol_01_DataTypeDef ProtocolData;
extern SC_Param_t SC_Param;

/* Exported macro ------------------------------------------------------------*/
#define MAX_EXTRA_GUARD_TIME (0xFF - DEFAULT_EXTRA_GUARDTIME)

/* Following macros are used for SC_XferBlock command */
#define XFER_BLK_SEND_DATA     1U     /* Command is for issuing the data  */
#define XFER_BLK_RECEIVE_DATA  2U     /* Command is for receiving the data */
#define XFER_BLK_NO_DATA       3U     /* Command type is No data exchange  */

/* Exported functions ------------------------------------------------------- */
/* APPLICATION LAYER ---------------------------------------------------------*/
void SC_Itf_InitParams(void);
void SC_Itf_IccPowerOn(uint8_t voltage);
void SC_Itf_IccPowerOff(void);
uint8_t SC_GetState(void);
uint8_t SC_Itf_XferBlock(uint8_t *ptrBlock, uint32_t blockLen,
                         uint16_t expectedLen,
                         USBD_CCID_BulkIn_DataTypeDef *CCID_BulkIn_Data);

uint8_t SC_Itf_SetParams(Protocol_01_DataTypeDef *pPtr, uint8_t T_01);
uint8_t SC_Itf_Escape(uint8_t *escapePtr, uint32_t escapeLen,
                      uint8_t *responseBuff, uint32_t *responseLen);

uint8_t SC_Itf_SetClock(uint8_t bClockCommand);
uint8_t SC_Itf_T0Apdu(uint8_t bmChanges, uint8_t bClassGetResponse,
                      uint8_t bClassEnvelope);

uint8_t SC_Itf_Mechanical(uint8_t bFunction);
uint8_t SC_Itf_SetDataRateAndClockFrequency(uint32_t dwClockFrequency,
                                            uint32_t dwDataRate);

uint8_t SC_Itf_Secure(uint32_t dwLength, uint8_t bBWI, uint16_t wLevelParameter,
                      uint8_t *pbuf, uint32_t *returnLen);

#ifdef __cplusplus
}
#endif

#endif /* __USBD_CCID_SC_IF_TEMPLATE_H */
