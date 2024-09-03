/**
  ******************************************************************************
  * @file    usbd_printer.h
  * @author  MCD Application Team
  * @brief   header file for the usbd_printer.c file.
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
#ifndef __USB_PRNT_H
#define __USB_PRNT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */

/** @defgroup usbd_PRNT
  * @brief This file is the Header file for usbd_PRNT.c
  * @{
  */


/** @defgroup usbd_PRNT_Exported_Defines
  * @{
  */
#ifndef PRNT_IN_EP
#define PRNT_IN_EP                                   0x81U  /* Default: EP1 for data IN */
#endif /* PRNT_IN_EP */

#ifndef PRNT_OUT_EP
#define PRNT_OUT_EP                                  0x01U  /* Default: EP1 for data OUT */
#endif /* PRNT_OUT_EP */

#ifndef PRNT_DATA_HS_MAX_PACKET_SIZE
#define PRNT_DATA_HS_MAX_PACKET_SIZE                 512U  /* Endpoint IN & OUT Packet size */
#endif /* PRNT_DATA_HS_MAX_PACKET_SIZE */

#ifndef PRNT_DATA_FS_MAX_PACKET_SIZE
#define PRNT_DATA_FS_MAX_PACKET_SIZE                 64U   /* Endpoint IN & OUT Packet size */
#endif /* PRNT_DATA_FS_MAX_PACKET_SIZE */

#define USB_PRNT_CONFIG_DESC_SIZ                     32U
#define PRNT_DATA_HS_IN_PACKET_SIZE                  PRNT_DATA_HS_MAX_PACKET_SIZE
#define PRNT_DATA_HS_OUT_PACKET_SIZE                 PRNT_DATA_HS_MAX_PACKET_SIZE

#define PRNT_DATA_FS_IN_PACKET_SIZE                  PRNT_DATA_FS_MAX_PACKET_SIZE
#define PRNT_DATA_FS_OUT_PACKET_SIZE                 PRNT_DATA_FS_MAX_PACKET_SIZE

/*---------------------------------------------------------------------*/
/*  PRNT definitions                                                    */
/*---------------------------------------------------------------------*/
#define PRNT_STATUS_PAPER_EMPTY                      0x10U
#define PRNT_STATUS_SELECTED                         0x08U
#define PRNT_STATUS_NO_ERROR                         0x00U

#define USB_PRNT_SUBCLASS                            0x01U

#define USB_PRNT_UNIDIRECTIONAL                      0x01U
#define USB_PRNT_BIDIRECTIONAL                       0x02U

/* USB PRNT Request types */
#define PRNT_GET_DEVICE_ID                           0x00U
#define PRNT_GET_PORT_STATUS                         0x01U
#define PRNT_SOFT_RESET                              0x02U
/**
  * @}
  */


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */

/**
  * @}
  */

typedef struct _USBD_PRNT_Itf
{
  int8_t (* Init)(void);
  int8_t (* DeInit)(void);
  int8_t (* Control_req)(uint8_t req, uint8_t *pbuf, uint16_t *length);
  int8_t (* Receive)(uint8_t *Buf, uint32_t *Len);

} USBD_PRNT_ItfTypeDef;

typedef struct
{
  uint32_t data[PRNT_DATA_HS_MAX_PACKET_SIZE / 4U];  /* Force 32-bit alignment */
  uint8_t CmdOpCode;
  uint8_t CmdLength;
  uint8_t *RxBuffer;
  uint8_t *TxBuffer;
  uint32_t RxLength;
  uint32_t TxLength;

  __IO uint32_t TxState;
  __IO uint32_t RxState;
} USBD_PRNT_HandleTypeDef;



/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */

extern USBD_ClassTypeDef   USBD_PRNT;
#define USBD_PRNT_CLASS    &USBD_PRNT
/**
  * @}
  */

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */
uint8_t  USBD_PRNT_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_PRNT_ItfTypeDef *fops);
uint8_t  USBD_PRNT_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff);
uint8_t  USBD_PRNT_ReceivePacket(USBD_HandleTypeDef *pdev);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif  /* __USB_PRNT_H */
/**
  * @}
  */

/**
  * @}
  */

