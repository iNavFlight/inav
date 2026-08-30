/**
  ******************************************************************************
  * @file    mx_wifi_hci.h
  * @author  MCD Application Team
  * @brief   Header for mx_wifi_hci.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MX_WIFI_HCI_H
#define MX_WIFI_HCI_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/*cstat -MISRAC2012-* */
#include "stdint.h"
#include "string.h"
#include "stdbool.h"
#include "stdio.h"
/*cstat +MISRAC2012-* */

uint32_t HAL_GetTick(void);
/* Exported Constants --------------------------------------------------------*/
//#define MX_WIFI_HCI_ACK_ENABLE  // comment this out if not need low level hci ack

/* Exported macro-------------------------------------------------------------*/
enum
{
  HCI_EVENT_ERROR_REPEAT_FRAME, /* received a duplicated sequence HCI frame */
  HCI_EVENT_ERROR_CRC_ERROR,    /* received a CRC error HCI frame*/
  HCI_EVENT_ERROR_OUT_OF_SEQ,   /* received an out of sequence HCI frame */

};

/* Exported typedef ----------------------------------------------------------*/
// PHY tx data
typedef int16_t (*phy_tx_func_t)(uint8_t *pdata, uint16_t len, uint32_t timeout);
// PHY rx data
typedef int16_t (*phy_rx_func_t)(uint8_t *pdata, uint16_t len, uint32_t timeout);
// hci event notify
typedef void (*event_notify_func_t)(uint8_t event);

/* Exported functions --------------------------------------------------------*/
/*
  * @param   timeout: in ms
  * @retval  0: success, <0: errcode,  >0: data send/recv len
  *
 */
int8_t mx_wifi_hci_init(phy_tx_func_t phy_send_func, phy_rx_func_t phy_recv_func, event_notify_func_t event_cb);

int8_t mx_wifi_hci_deinit(void);

int16_t mx_wifi_hci_send(uint8_t *data, uint16_t len, uint32_t timeout);

int16_t mx_wifi_hci_recv(uint8_t **p_data, uint32_t timeout);


#ifdef __cplusplus
}
#endif

#endif /* MX_WIFI_HCI_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
