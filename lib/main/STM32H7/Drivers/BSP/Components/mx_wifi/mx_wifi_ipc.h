/**
  ******************************************************************************
  * @file    mx_wifi_ipc.h
  * @author  MCD Application Team
  * @brief   Header for mx_wifi_ipc.c module
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
#ifndef MX_WIFI_IPC_H
#define MX_WIFI_IPC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/*cstat -MISRAC2012-* */
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
/*cstat +MISRAC2012-* */

#include "mx_wifi_hci.h"

/* Exported Constants --------------------------------------------------------*/
#define MIPC_RET_FLAG_SET   (1)
#define MIPC_RET_FLAG_CLR   (0)

extern int32_t recv_cnt;
/* Exported macro-------------------------------------------------------------*/

/* Exported typedef ----------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
/*
  * @param   timeout: in ms
  * @retval  0: success, <0: errcode,  >0: return data
  *
 */
int32_t mipc_init(phy_tx_func_t io_send, phy_rx_func_t io_recv, uint32_t timeout_ms);

int32_t mipc_deinit(void);

// new API
int32_t mipc_poll(int32_t *p_ret_flag, int32_t *p_ret, uint32_t timeout_ms);


#ifdef __cplusplus
}
#endif

#endif /* MX_WIFI_IPC_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
