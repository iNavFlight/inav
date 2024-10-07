/**
  ******************************************************************************
  * @file    mx_wifi_hci.h
  * @author  MCD Application Team
  * @brief   Header for mx_wifi_hci.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MX_WIFI_HCI_H
#define MX_WIFI_HCI_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "mx_wifi.h"
#include "mx_wifi_ipc.h"

/**
  * @brief CONFIGURATIONS
  */
/* #define MX_WIFI_HCI_DEBUG */

/**
  * @brief HCI PACKET
  */
#define MX_WIFI_HCI_DATA_SIZE   (MIPC_PKT_MAX_SIZE)

/**
  * @brief PROTOTYPES
  */
typedef struct _hci_pkt_s
{
  uint16_t len;
  uint8_t *data;
} hci_pkt_t;



/**
  * @brief API
  */

/**
  * @brief prototype of the low level send function for the HCI layer
  * @param data: data to send
  * @param size: size of the data to send
  * @retval size of the data sent
  */
typedef uint16_t (*hci_send_func_t)(uint8_t *data, uint16_t size);

/**
  * @brief Init for the HCI layer
  * @param low_level_send: send function for the HCI low level msg
  * @retval 0 success, otherwise failed
  */
int32_t mx_wifi_hci_init(hci_send_func_t low_level_send);

/**
  * @brief Send msg for the HCI layer
  * @param payload: data to send
  * @param len: size of the data to send
  * @retval 0 success, otherwise failed
  */
int32_t mx_wifi_hci_send(uint8_t *payload, uint16_t len);

/**
  * @brief Recv msg for the HCI layer
  * @param timeout: recv timeout in milliseconds
  * @retval msg buffer got, NULL if failed
  */
mx_buf_t *mx_wifi_hci_recv(uint32_t timeout);

/**
  * @brief Free msg buffer got from the HCI layer
  * @param nbuf: msg buffer to be free
  */
void mx_wifi_hci_free(mx_buf_t *nbuf);

/**
  * @brief DeInit for the HCI layer
  * @retval 0 success, otherwise failed
  */
int32_t mx_wifi_hci_deinit(void);

/**
  * @brief LOW LEVEL INTERFACE
  */

/**
  * @brief Feed msg for the HCI layer to handle
  * @param netbuf: low level msg to be handled by the HCI layer
  */
void mx_wifi_hci_input(mx_buf_t *netbuf);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MX_WIFI_HCI_H */
