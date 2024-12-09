/**
  ******************************************************************************
  * @file    usbd_ccid_if_template.h
  * @author  MCD Application Team
  * @brief   header file for the usbd_ccid_if_template.c file.
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
#ifndef __USBD_CCID_IF_TEMPLATE_H
#define __USBD_CCID_IF_TEMPLATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_ccid.h"
#include "usbd_ccid_cmd.h"

#ifndef __USBD_CCID_SMARTCARD_H
#include "usbd_ccid_smartcard_template.h"
#endif /* __USBD_CCID_SMARTCARD_H */

/* Exported defines ----------------------------------------------------------*/

/*****************************************************************************/
/*********************** CCID Bulk Transfer State machine ********************/
/*****************************************************************************/
#define CCID_STATE_IDLE                                   0U
#define CCID_STATE_DATA_OUT                               1U
#define CCID_STATE_RECEIVE_DATA                           2U
#define CCID_STATE_SEND_RESP                              3U
#define CCID_STATE_DATAIN                                 4U
#define CCID_STATE_UNCORRECT_LENGTH                       5U

#define DIR_IN                                            0U
#define DIR_OUT                                           1U
#define BOTH_DIR                                          2U

/************ Value of the Interrupt transfer status to set ******************/
#define INTRSTATUS_COMPLETE                               1U
#define INTRSTATUS_RESET                                  0U
/************** slot change status *******************************************/
#define SLOTSTATUS_CHANGED                                1U
#define SLOTSTATUS_RESET                                  0U

/* Exported types ------------------------------------------------------------*/
extern USBD_HandleTypeDef USBD_Device;

/* CCID Interface callback */
extern USBD_CCID_ItfTypeDef USBD_CCID_If_fops;

/* Exported macros -----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USBD_CCID_IF_TEMPLATE_H */
