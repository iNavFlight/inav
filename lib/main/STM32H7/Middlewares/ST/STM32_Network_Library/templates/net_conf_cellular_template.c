/**
  ******************************************************************************
  * @file    net_conf.c
  * @author  MCD Application Team
  * @brief   Implement cellular_probe() called to initialize the Cellular low level driver
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
#include "net_connect.h"



/* Private functions ---------------------------------------------------------*/
int32_t cellular_probe(void **ll_drv_context);


int32_t cellular_probe(void **ll_drv_context)
{

  return 0;
}

#ifdef GENERATOR_AWS_CLOUD
#include "mbedtls/x509_crt.h"

/*
 * Amazon Profile
 */
const   mbedtls_x509_crt_profile mbedtls_x509_crt_amazon_suite =
{
  /* Only SHA-256 and 384 */
  MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA256) |
  MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA384),
  /* Only ECDSA */
  MBEDTLS_X509_ID_FLAG(MBEDTLS_PK_RSA) | /* */
  MBEDTLS_X509_ID_FLAG(MBEDTLS_PK_ECKEY) | /* */
  MBEDTLS_X509_ID_FLAG(MBEDTLS_PK_ECDSA),
#if defined(MBEDTLS_ECP_C)
  /* Only NIST P-256 and P-384 */
  MBEDTLS_X509_ID_FLAG(MBEDTLS_ECP_DP_SECP256R1) |
  MBEDTLS_X509_ID_FLAG(MBEDTLS_ECP_DP_SECP384R1),
#else
  0,
#endif /* MBEDTLS_ECP_C */
  2048
};
const int32_t net_tls_sizeof_suite_structure = sizeof(mbedtls_x509_crt_profile);
const void    *net_tls_user_suite0 = (void *) &mbedtls_x509_crt_amazon_suite;

#endif /* GENERATOR_AWS_CLOUD */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
