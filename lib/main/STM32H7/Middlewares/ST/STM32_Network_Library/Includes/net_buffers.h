/**
  ******************************************************************************
  * @file    net_buffers.h
  * @author  MCD Application Team
  * @brief   Defines buffer allocation functions
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
#ifndef NET_BUFFERS_H
#define NET_BUFFERS_H

#include "lwip/pbuf.h"

#define NET_BUF_ALLOC(n)        (net_buf_t*)pbuf_alloc(PBUF_RAW, n, PBUF_POOL)
#define NET_BUF_REF_ALLOC()     (net_buf_t*)pbuf_alloc(PBUF_RAW, 0, PBUF_REF);
#define NET_BUF_FREE(p)         pbuf_free((struct pbuf*)p);
#define  NET_BUF_REF(p)         pbuf_ref((struct pbuf*)p);

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
#endif /* NET_BUFFERS_H */

