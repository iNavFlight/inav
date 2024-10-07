/**
  ******************************************************************************
  * @file    ipc_basic_type.h
  * @author  MCD Application Team
  * @brief   Header for ipc_basic_type module
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
#ifndef IPC_BASIC_TYPE_H
#define IPC_BASIC_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/*cstat -MISRAC2012-* */
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "stdint.h"
/*cstat +MISRAC2012-* */

#include "mx_wifi_conf.h"

#if MX_WIFI_USE_CMSIS_OS
/*cstat -MISRAC2012-* */
#include "cmsis_os.h"
/*cstat +MISRAC2012-* */
#define MX_CALLOC net_calloc
#define MX_MALLOC pvPortMalloc
#define MX_FREE   vPortFree
#else
#define MX_MALLOC malloc
#define MX_CALLOC calloc
#define MX_FREE  free
#endif /* MX_WIFI_USE_CMSIS_OS */


enum
{
  IPC_TYPE_NONE = 0, // no more argument
  IPC_TYPE_DATA,     // uint32_t
  IPC_TYPE_POINTER,  // void *
};

#define IPC_BUFFER_SIZE  (MX_WIFI_DATA_SIZE + 600)
#define MAX_ARGUEMNT_NUM 10

/* error code */
#define IPC_RET_OK      (0)
#define IPC_RET_ERROR   (-1)
#define IPC_RET_TIMEOUT (-2)

typedef int32_t (*ipc_api_t)();

#pragma pack(1)
typedef struct
{
  uint8_t     eap_type; /* support: EAP_TYPE_PEAP, EAP_TYPE_TTLS, EAP_TYPE_TLS */
  uint16_t  rootca_len;  /* not used, set 0*/
  uint16_t  client_cert_len; /* not used, set 0*/
  uint16_t  client_key_len; /* not used, set 0*/
} lib_ipc_eap_attr_t;
#pragma pack()


#ifdef __cplusplus
}
#endif

#endif /* IPC_BASIC_TYPE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
