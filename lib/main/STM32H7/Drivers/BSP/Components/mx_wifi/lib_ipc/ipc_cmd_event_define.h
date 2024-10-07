/**
  ******************************************************************************
  * @file    lib_ipc_cmd_event_define.h
  * @author  MCD Application Team
  * @brief   Header for lib_ipc_cmd_event_define module
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
#ifndef IPC_CMD_EVENT_DEFINE_H
#define IPC_CMD_EVENT_DEFINE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ipc_basic_type.h"

/* define all command and event index */

enum
{
  IPC_CMD_START = 0x1000,
#define IPC_CMD_API(name) IPC_CMD_ ##name,
#include "ipc_cmd_list.h"
  IPC_CMD_END,
};

enum
{
  IPC_EVENT_START = 0x2000,
#define IPC_CMD_API(name) IPC_EVENT_ ##name,
#include "ipc_event_list.h"
  IPC_EVENT_END,
};


#ifdef __cplusplus
}
#endif

#endif /* IPC_CMD_EVENT_DEFINE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
