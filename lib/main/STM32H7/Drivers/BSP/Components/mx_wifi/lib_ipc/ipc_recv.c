/**
  ******************************************************************************
  * @file    ipc_recv.c
  * @author  MCD Application Team
  * @brief   Host driver IPC recv API of MXCHIP Wi-Fi component.
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

/* Includes ------------------------------------------------------------------*/
/*cstat -MISRAC2012-* */
#include "stdio.h"
#include "stdarg.h"
/*cstat +MISRAC2012-* */

#include "mx_wifi_conf.h"

#ifndef IPC_MASTER
#include "mdebug.h"
#endif
#include "lib_ipc.h"

#ifdef IPC_MASTER

#ifdef IPC_DEBUG
#define app_log(format, ...)      printf(format, ##__VA_ARGS__);
#define custom_print(format, ...)   printf(format, ##__VA_ARGS__);
#else
#define app_log(format, ...)
#define custom_print(format, ...)
#endif

#else

#ifdef IPC_DEBUG
#define app_log(format, ...)      custom_log("ipc", format, ##__VA_ARGS__)
#else
#define app_log(format, ...)
#endif

#endif

int32_t ipc_api_recv(uint8_t *data, int32_t len)
{
  int32_t i, argnum = 0, ret = -1;
  uint16_t cmd, point_len, point_num = 0;
  uint8_t type;
  uint32_t arg[MAX_ARGUEMNT_NUM];
  uint32_t point[MAX_ARGUEMNT_NUM];
  void *p;
  void *p_arg;

#ifdef IPC_DEBUG
  custom_print("IPC RX %d bytes: ", len);
  for (i = 0; i < len; i++)
  {
    custom_print("%02x ", data[i]);
  }
  custom_print("\r\n");
#endif

  (void)memcpy(&cmd, data, 2);
  i = 2;

#ifdef IPC_MASTER
  if ((cmd >= (uint16_t)IPC_EVENT_END) || (cmd <= (uint16_t)IPC_EVENT_START))
  {
    app_log("rx bad event %x !\r\n", cmd);
    ret = -1;
  }
  else
  {
    cmd -= ((uint16_t)IPC_EVENT_START + (uint16_t)1);
#else
  if ((cmd >= IPC_CMD_END) || (cmd <= IPC_CMD_START))
  {
    app_log("rx bad cmd %x !\r\n", cmd);
    ret = -1;
  }
  else
  {
    cmd -= (IPC_CMD_START + 1);
#endif

    while (i < len)
    {
      type = data[i];
      i++;
      switch (type)
      {
        case IPC_TYPE_DATA:
          (void)memcpy(&arg[argnum], &data[i], 4);
          argnum++;
          i += 4;
          break;
        case IPC_TYPE_POINTER:
          (void)memcpy(&point_len, &data[i], 2);
          i += 2;
          if (point_len == (uint16_t)0)
          {
            arg[argnum] = (uint32_t)NULL;
            argnum++;
            arg[argnum] = (uint32_t)0;
            argnum++;
          }
          else
          {
            p_arg = MX_MALLOC(point_len);
            if (NULL == p_arg)
            {
              app_log("*** point_len error! malloc failed!!!");
              /* goto ERR_EXIT */
              i = len;      /* just for break while(i < len) */
              argnum = -1;  /* just for skip next switch(argnum) */
            }
            else
            {
              (void)memcpy(&(arg[argnum]), &p_arg, sizeof(uint32_t));
              point[point_num] = arg[argnum];
              point_num++;
              (void)memcpy(p_arg, &data[i], point_len);
              argnum++;
              arg[argnum] = (uint32_t)point_len;
              argnum++;
            }
          }
          i += (int32_t)point_len;
          break;

        default:
          break;
      }
    }

    switch (argnum)
    {
      case 0:
        ret = ipc_event_tbl[cmd]();
        break;
      case 1:
        ret = ipc_event_tbl[cmd](arg[0]);
        break;
      case 2:
        ret = ipc_event_tbl[cmd](arg[0], arg[1]);
        break;
      case 3:
        ret = ipc_event_tbl[cmd](arg[0], arg[1], arg[2]);
        break;
      case 4:
        ret = ipc_event_tbl[cmd](arg[0], arg[1], arg[2], arg[3]);
        break;
      case 5:
        ret = ipc_event_tbl[cmd](arg[0], arg[1], arg[2], arg[3], arg[4]);
        break;
      case 6:
        ret = ipc_event_tbl[cmd](arg[0], arg[1], arg[2], arg[3], arg[4], arg[5]);
        break;
      case 7:
        ret = ipc_event_tbl[cmd](arg[0], arg[1], arg[2], arg[3], arg[4], arg[5],
                                 arg[6]);
        break;
      case 8:
        ret = ipc_event_tbl[cmd](arg[0], arg[1], arg[2], arg[3], arg[4], arg[5],
                                 arg[6], arg[7]);
        break;
      case 9:
        ret = ipc_event_tbl[cmd](arg[0], arg[1], arg[2], arg[3], arg[4], arg[5],
                                 arg[6], arg[7], arg[8]);
        break;
      case 10:
        ret = ipc_event_tbl[cmd](arg[0], arg[1], arg[2], arg[3], arg[4], arg[5],
                                 arg[6], arg[7], arg[8], arg[9]);
        break;

      default:
        break;
    }

    /* ERR_EXIT: */
    for (i = 0; i < (int32_t)point_num; i++)
    {
      (void)memcpy(&p, &point[i], sizeof(p));
      if (NULL != p)
      {
        MX_FREE(p);
      }
    }
  }

  return ret;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
