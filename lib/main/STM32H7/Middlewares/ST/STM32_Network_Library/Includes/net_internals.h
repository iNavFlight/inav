/**
  ******************************************************************************
  * @file    net_internals.h
  * @author  MCD Application Team
  * @brief   Header for the network interface with mbedTLS (if used)
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef NET_INTERNALS_H
#define NET_INTERNALS_H

#include "net_core.h"
#ifdef  NET_MBEDTLS_HOST_SUPPORT
/*cstat -MISRAC* -DEFINE-* -CERT-EXP19*  */
#include "net_mbedtls.h"
/*cstat +MISRAC* +DEFINE-* +CERT-EXP19*  */
#endif /* NET_MBEDTLS_HOST_SUPPORT */
#endif /* NET_INTERNALS_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
