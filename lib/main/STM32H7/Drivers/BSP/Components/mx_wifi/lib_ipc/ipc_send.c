/**
  ******************************************************************************
  * @file    ipc_send.c
  * @author  MCD Application Team
  * @brief   Host driver IPC send API of MXCHIP Wi-Fi component.
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
#include "stdlib.h"
/*cstat +MISRAC2012-* */

#include "mx_wifi_conf.h"

#ifndef IPC_MASTER
#include "mdebug.h"
#endif
#include "lib_ipc.h"

#ifdef IPC_MASTER

#ifdef IPC_DEBUG
#define app_log(format, ...)    printf(format, ##__VA_ARGS__);
#define custom_print(format, ...) printf(format, ##__VA_ARGS__);
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

int32_t ipc_api_send(uint16_t cmd, uint8_t arg_type[], ...)
{
  int32_t i = 0, argnum, offset = 0, ret = 0;
  va_list ap;
  uint8_t *buf;
  void *ptmp;
  int32_t val;
  void *p;
  int32_t len;
  int16_t point_len;

  ptmp = MX_MALLOC(IPC_BUFFER_SIZE);
  if (ptmp == NULL)
  {
    ret = -1;
  }
  else
  {
    (void)memcpy(&buf, &ptmp, sizeof(buf));
    while (arg_type[i] != (uint8_t)IPC_TYPE_NONE)
    {
      i++;
    }

    (void)memcpy(&buf[offset], &cmd, 2);
    offset += 2;

    argnum = i;

    va_start(ap, arg_type);
    for (i = 0; i < argnum; i++)
    {
      buf[offset] = arg_type[i];
      offset++;
      switch (arg_type[i])
      {
        case IPC_TYPE_DATA:
          val = va_arg(ap, int32_t);
          if ((offset + 4) >= (IPC_BUFFER_SIZE))
          {
            app_log("!!!! ERROR !! message(data) too long");
            va_end(ap);
            MX_FREE(buf);
            ret = -1;
            /* goto EXIT */
            i = argnum;  /* just for break for( i<argnum)*/
          }
          else
          {
            (void)memcpy(&buf[offset], &val, 4);
            offset += 4;
          }
          break;

        case IPC_TYPE_POINTER:
          p = va_arg(ap, void *);
          i++;
          if (arg_type[i] != (uint8_t)IPC_TYPE_DATA)
          {
            app_log("!!!! ERROR !! IPC pointer must follow int32");
            va_end(ap);
            MX_FREE(buf);
            ret = -1;
            /* goto EXIT */
            i = argnum;  /* just for break for( i<argnum)*/
          }
          else
          {
            len = va_arg(ap, int32_t);
            point_len = (int16_t)len;
            if ((offset + 2 + len) >= (IPC_BUFFER_SIZE))
            {
              app_log("!!!! ERROR !! message(pointer) too long");
              va_end(ap);
              MX_FREE(buf);
              ret = -1;
              /* goto EXIT */
              i = argnum;  /* just for break for( i<argnum)*/
            }
            else
            {
              (void)memcpy(&buf[offset], &point_len, 2);
              offset += 2;
              if (len > 0)
              {
                (void)memcpy(&buf[offset], p, (size_t)len);
                offset += len;
              }
            }
          }
          break;

        default:
          break;
      }
    }

    if (-1 != ret)
    {
      if (offset > IPC_BUFFER_SIZE)
      {
        app_log("!!!! ERROR !! message too long");
        va_end(ap);
        MX_FREE(buf);
        ret = -1;
        /* goto EXIT */
      }
      else
      {
        va_end(ap);

#ifdef IPC_DEBUG
        custom_print("IPC TX %d bytes: ", offset);
        for (i = 0; i < offset; i++)
        {
          custom_print("%02x ", buf[i]);
        }
        custom_print("\r\n");
#endif

        ret = ipc_output(buf, offset);
        MX_FREE(buf);
      }
    }
  }

  /* EXIT: */
  return ret;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
