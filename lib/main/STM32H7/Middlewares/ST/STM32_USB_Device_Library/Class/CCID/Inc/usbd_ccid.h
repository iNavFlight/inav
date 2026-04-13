/**
  ******************************************************************************
  * @file    usbd_ccid.h
  * @author  MCD Application Team
  * @brief   header file for the usbd_ccid.c file.
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
#ifndef __USBD_CCID_H
#define __USBD_CCID_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */

/** @defgroup usbd_cdc
  * @brief This file is the Header file for usbd_ccid.c
  * @{
  */


/** @defgroup usbd_cdc_Exported_Defines
  * @{
  */
#ifndef CCID_IN_EP
#define CCID_IN_EP                                            0x81U  /* EP1 for data IN */
#endif /* CCID_IN_EP */

#ifndef CCID_OUT_EP
#define CCID_OUT_EP                                           0x01U  /* EP1 for data OUT */
#endif /* CCID_OUT_EP */

#ifndef CCID_CMD_EP
#define CCID_CMD_EP                                           0x82U  /* EP2 for CCID commands */
#endif /* CCID_CMD_EP */

#ifndef CCID_CMD_HS_BINTERVAL
#define CCID_CMD_HS_BINTERVAL                                 0x10U
#endif /* CCID_CMD_HS_BINTERVAL */

#ifndef CCID_CMD_FS_BINTERVAL
#define CCID_CMD_FS_BINTERVAL                                 0x10U
#endif /* CCID_CMD_FS_BINTERVAL */


#define CCID_DATA_HS_MAX_PACKET_SIZE                          512U  /* Endpoint IN & OUT Packet size */
#define CCID_DATA_FS_MAX_PACKET_SIZE                          64U  /* Endpoint IN & OUT Packet size */
#define CCID_CMD_PACKET_SIZE                                  8U  /* Control Endpoint Packet size */

#define USB_CCID_CONFIG_DESC_SIZ                              93U
#define CCID_DATA_HS_IN_PACKET_SIZE                           CCID_DATA_HS_MAX_PACKET_SIZE
#define CCID_DATA_HS_OUT_PACKET_SIZE                          CCID_DATA_HS_MAX_PACKET_SIZE

#define CCID_DATA_FS_IN_PACKET_SIZE                           CCID_DATA_FS_MAX_PACKET_SIZE
#define CCID_DATA_FS_OUT_PACKET_SIZE                          CCID_DATA_FS_MAX_PACKET_SIZE

/*---------------------------------------------------------------------*/
/*                 CCID definitions                                    */
/*---------------------------------------------------------------------*/
#define CCID_SEND_ENCAPSULATED_COMMAND                        0x00U
#define CCID_GET_ENCAPSULATED_RESPONSE                        0x01U
#define CCID_SET_COMM_FEATURE                                 0x02U
#define CCID_GET_COMM_FEATURE                                 0x03U
#define CCID_CLEAR_COMM_FEATURE                               0x04U
#define CCID_SET_LINE_CODING                                  0x20U
#define CCID_GET_LINE_CODING                                  0x21U
#define CCID_SET_CONTROL_LINE_STATE                           0x22U
#define CCID_SEND_BREAK                                       0x23U

/*---------------------------------------------------------------------*/
#define REQUEST_ABORT                                         0x01U
#define REQUEST_GET_CLOCK_FREQUENCIES                         0x02U
#define REQUEST_GET_DATA_RATES                                0x03U

/*---------------------------------------------------------------------*/
/*     The Smart Card Device Class Descriptor definitions              */
/*---------------------------------------------------------------------*/

#define CCID_INTERFACE_DESC_SIZE                              0x09U
#define USB_DEVICE_CLASS_CCID                                 0x0BU
#define CCID_CLASS_DESC_SIZE                                  0x36U
#define CCID_DESC_TYPE                                        0x21U

#ifndef CCID_VOLTAGE_SUPP
#define CCID_VOLTAGE_SUPP                                     0x07U
#endif /* CCID_VOLTAGE_SUPP */
#ifndef USBD_CCID_PROTOCOL
#define USBD_CCID_PROTOCOL                                    0x03U
#endif /* USBD_CCID_PROTOCOL */
#ifndef USBD_CCID_DEFAULT_CLOCK_FREQ
#define USBD_CCID_DEFAULT_CLOCK_FREQ                          3600U
#endif /* USBD_CCID_DEFAULT_CLOCK_FREQ */
#ifndef USBD_CCID_MAX_CLOCK_FREQ
#define USBD_CCID_MAX_CLOCK_FREQ                              USBD_CCID_DEFAULT_CLOCK_FREQ
#endif /* USBD_CCID_MAX_CLOCK_FREQ */
#ifndef USBD_CCID_DEFAULT_DATA_RATE
#define USBD_CCID_DEFAULT_DATA_RATE                           9677U
#endif /* USBD_CCID_DEFAULT_DATA_RATE */
#ifndef USBD_CCID_MAX_DATA_RATE
#define USBD_CCID_MAX_DATA_RATE                               USBD_CCID_DEFAULT_DATA_RATE
#endif /* USBD_CCID_MAX_DATA_RATE */
#ifndef USBD_CCID_MAX_INF_FIELD_SIZE
#define USBD_CCID_MAX_INF_FIELD_SIZE                          254U
#endif /* USBD_CCID_MAX_INF_FIELD_SIZE */
#ifndef CCID_MAX_BLOCK_SIZE_HEADER
#define CCID_MAX_BLOCK_SIZE_HEADER                            271U
#endif /* CCID_MAX_BLOCK_SIZE_HEADER */

#define TPDU_EXCHANGE                                         0x01U
#define SHORT_APDU_EXCHANGE                                   0x02U
#define EXTENDED_APDU_EXCHANGE                                0x04U
#define CHARACTER_EXCHANGE                                    0x00U

#ifndef EXCHANGE_LEVEL_FEATURE
#define EXCHANGE_LEVEL_FEATURE                               TPDU_EXCHANGE
#endif /* EXCHANGE_LEVEL_FEATURE */

#define CCID_ENDPOINT_DESC_SIZE                               0x07U

#ifndef CCID_EP0_BUFF_SIZ
#define CCID_EP0_BUFF_SIZ                                     64U
#endif /* CCID_EP0_BUFF_SIZ */

#ifndef CCID_BULK_EPIN_SIZE
#define CCID_BULK_EPIN_SIZE                                   64U
#endif /* CCID_BULK_EPIN_SIZE */

#define CCID_INT_BUFF_SIZ                                     2U
/*---------------------------------------------------------------------*/
/*
 * CCID Class specification revision 1.1
 * Command Pipe. Bulk Messages
 */

/* CCID Bulk Out Command definitions */
#define PC_TO_RDR_ICCPOWERON                                  0x62U
#define PC_TO_RDR_ICCPOWEROFF                                 0x63U
#define PC_TO_RDR_GETSLOTSTATUS                               0x65U
#define PC_TO_RDR_XFRBLOCK                                    0x6FU
#define PC_TO_RDR_GETPARAMETERS                               0x6CU
#define PC_TO_RDR_RESETPARAMETERS                             0x6DU
#define PC_TO_RDR_SETPARAMETERS                               0x61U
#define PC_TO_RDR_ESCAPE                                      0x6BU
#define PC_TO_RDR_ICCCLOCK                                    0x6EU
#define PC_TO_RDR_T0APDU                                      0x6AU
#define PC_TO_RDR_SECURE                                      0x69U
#define PC_TO_RDR_MECHANICAL                                  0x71U
#define PC_TO_RDR_ABORT                                       0x72U
#define PC_TO_RDR_SETDATARATEANDCLOCKFREQUENCY                0x73U

/* CCID Bulk In Command definitions */
#define RDR_TO_PC_DATABLOCK                                   0x80U
#define RDR_TO_PC_SLOTSTATUS                                  0x81U
#define RDR_TO_PC_PARAMETERS                                  0x82U
#define RDR_TO_PC_ESCAPE                                      0x83U
#define RDR_TO_PC_DATARATEANDCLOCKFREQUENCY                   0x84U

/* CCID Interrupt In Command definitions */
#define RDR_TO_PC_NOTIFYSLOTCHANGE                            0x50U
#define RDR_TO_PC_HARDWAREERROR                               0x51U

/* Bulk-only Command Block Wrapper */
#define ABDATA_SIZE                                           261U
#define CCID_CMD_HEADER_SIZE                                  10U
#define CCID_RESPONSE_HEADER_SIZE                             10U

/* Number of SLOTS. For single card, this value is 1 */
#define CCID_NUMBER_OF_SLOTS                                  1U

#define CARD_SLOT_FITTED                                      1U
#define CARD_SLOT_REMOVED                                     0U

#define OFFSET_INT_BMESSAGETYPE                               0x00U
#define OFFSET_INT_BMSLOTICCSTATE                             0x01U
#define SLOT_ICC_PRESENT                                      0x01U
/* LSb : (0b = no ICC present, 1b = ICC present) */
#define SLOT_ICC_CHANGE                                       0x02U
/* MSb : (0b = no change, 1b = change) */


/**
  * @}
  */


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */

/**
  * @}
  */

typedef struct
{
  uint32_t bitrate;
  uint8_t  format;
  uint8_t  paritytype;
  uint8_t  datatype;
} USBD_CCID_LineCodingTypeDef;

typedef struct
{
  uint8_t bMessageType; /* Offset = 0*/
  uint32_t dwLength;    /* Offset = 1, The length field (dwLength) is the length
                          of the message not including the 10-byte header.*/
  uint8_t bSlot;        /* Offset = 5*/
  uint8_t bSeq;         /* Offset = 6*/
  uint8_t bSpecific_0;  /* Offset = 7*/
  uint8_t bSpecific_1;  /* Offset = 8*/
  uint8_t bSpecific_2;  /* Offset = 9*/
  uint8_t abData [ABDATA_SIZE]; /* Offset = 10, For reference, the absolute
                           maximum block size for a TPDU T=0 block is 260 bytes
                           (5 bytes command; 255 bytes data),
                           or for a TPDU T=1 block is 259 bytes,
                           or for a short APDU T=1 block is 261 bytes,
                           or for an extended APDU T=1 block is 65544 bytes.*/
} __PACKED USBD_CCID_BulkOut_DataTypeDef;

typedef struct
{
  uint8_t bMessageType;   /* Offset = 0 */
  uint32_t dwLength;      /* Offset = 1 */
  uint8_t bSlot;          /* Offset = 5, Same as Bulk-OUT message */
  uint8_t bSeq;           /* Offset = 6, Same as Bulk-OUT message */
  uint8_t bStatus;        /* Offset = 7, Slot status as defined in section 6.2.6 */
  uint8_t bError;         /* Offset = 8, Slot error  as defined in section 6.2.6 */
  uint8_t bSpecific;      /* Offset = 9 */
  uint8_t abData[ABDATA_SIZE]; /* Offset = 10 */
  uint16_t u16SizeToSend;
} __PACKED USBD_CCID_BulkIn_DataTypeDef;

typedef struct
{
  __IO uint8_t SlotStatus;
  __IO uint8_t SlotStatusChange;
} USBD_CCID_SlotStatusTypeDef;


typedef struct
{
  __IO uint8_t bAbortRequestFlag;
  __IO uint8_t bSeq;
  __IO uint8_t bSlot;
} USBD_CCID_ParamTypeDef;

/*
 * CCID Class specification revision 1.1
 * Smart Card Device Class Descriptor Table
 */

typedef struct
{
  uint8_t           bLength;
  uint8_t           bDescriptorType;
  uint16_t          bcdCCID;
  uint8_t           bMaxSlotIndex;
  uint8_t           bVoltageSupport;
  uint32_t          dwProtocols;
  uint32_t          dwDefaultClock;
  uint32_t          dwMaximumClock;
  uint8_t           bNumClockSupported;
  uint32_t          dwDataRate;
  uint32_t          dwMaxDataRate;
  uint8_t           bNumDataRatesSupported;
  uint32_t          dwMaxIFSD;
  uint32_t          dwSynchProtocols;
  uint32_t          dwMechanical;
  uint32_t          dwFeatures;
  uint32_t          dwMaxCCIDMessageLength;
  uint8_t           bClassGetResponse;
  uint8_t           bClassEnvelope;
  uint16_t          wLcdLayout;
  uint8_t           bPINSupport;
  uint8_t           bMaxCCIDBusySlots;
} __PACKED USBD_CCID_DescTypeDef;

typedef struct
{
  uint8_t data[CCID_DATA_HS_MAX_PACKET_SIZE / 4U];   /* Force 32-bit alignment */
  uint32_t UsbMessageLength;
  uint8_t UsbIntData[CCID_CMD_PACKET_SIZE];          /* Buffer for the Interrupt In Data */
  uint32_t alt_setting;

  USBD_CCID_BulkIn_DataTypeDef UsbBlkInData;         /* Buffer for the Out Data */
  USBD_CCID_BulkOut_DataTypeDef UsbBlkOutData;       /* Buffer for the In Data */
  USBD_CCID_SlotStatusTypeDef SlotStatus;
  USBD_CCID_ParamTypeDef USBD_CCID_Param;

  __IO uint32_t MaxPcktLen;
  __IO uint8_t blkt_state;                           /* Bulk transfer state */

  uint16_t slot_nb;
  uint16_t seq_nb;
} USBD_CCID_HandleTypeDef;

/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */

extern USBD_ClassTypeDef  USBD_CCID;
#define USBD_CCID_CLASS   &USBD_CCID
/**
  * @}
  */

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */
typedef struct _USBD_CCID_Itf
{
  uint8_t (* Init)(USBD_HandleTypeDef  *pdev);
  uint8_t (* DeInit)(USBD_HandleTypeDef  *pdev);
  uint8_t (* Control)(uint8_t req, uint8_t *pbuf, uint16_t *length);
  uint8_t (* Response_SendData)(USBD_HandleTypeDef  *pdev, uint8_t *buf, uint16_t len);
  uint8_t (* Send_Process)(uint8_t *Command, uint8_t *Data);
  uint8_t (* SetSlotStatus)(USBD_HandleTypeDef *pdev);
} USBD_CCID_ItfTypeDef;

/**
  * @}
  */
/** @defgroup USB_CORE_Exported_Functions
  * @{
  */
uint8_t USBD_CCID_RegisterInterface(USBD_HandleTypeDef *pdev,
                                    USBD_CCID_ItfTypeDef *fops);

uint8_t USBD_CCID_IntMessage(USBD_HandleTypeDef  *pdev);


/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif  /* __USBD_CCID_H */
/**
  * @}
  */

/**
  * @}
  */
