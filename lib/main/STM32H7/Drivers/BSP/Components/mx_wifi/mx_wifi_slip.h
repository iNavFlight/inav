/**
  ******************************************************************************
  * @file    mx_wifi_slip.h
  * @author  MCD Application Team
  * @brief   Header for mx_wifi_slip.c module
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
#ifndef MX_WIFI_SLIP_H
#define MX_WIFI_SLIP_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mx_wifi_conf.h"

#if MX_WIFI_USE_CMSIS_OS
/*cstat -MISRAC2012-* */
#include "cmsis_os.h"
/*cstat +MISRAC2012-* */
#define HCI_MALLOC pvPortMalloc
#define HCI_FREE   vPortFree
#else
#define HCI_MALLOC malloc
#define HCI_FREE  free
#endif /* MX_WIFI_USE_CMSIS_OS */

#define SLIP_DATA 0
#define SLIP_ACK  1
enum
{
  SLIP_START = 0xC0,
  SLIP_END   = 0xD0,
  SLIP_ESCAPE = 0xDB,
  SLIP_ESCAPE_START = 0xDC,
  SLIP_ESCAPE_ES    = 0xDD,
  SLIP_ESCAPE_END   = 0xDE,
};

#define SLIP_BUFFER_SIZE (MX_WIFI_DATA_SIZE + 600)

/* transfer hci data to slip data.
  * return the slip buffer
  */
uint8_t *slip_transfer(uint8_t *data, int32_t len, int32_t *outlen);

int16_t hci_phy_send(uint8_t *data, uint16_t len, uint32_t timeout);
int8_t hci_data_process(uint8_t *data, uint16_t len);
int8_t hci_ack_process(uint8_t seq);
void hci_event_notify(uint8_t event);

/* PHY receive one serial byte to HCI_SLIP*/
int32_t slip_input_byte(uint8_t data);
int32_t slip_init(void);
int32_t slip_deinit(void);


#ifdef __cplusplus
}
#endif

#endif /* MX_WIFI_SLIP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
