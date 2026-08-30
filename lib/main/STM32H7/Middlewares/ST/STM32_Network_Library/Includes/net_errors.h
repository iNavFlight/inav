/**
  ******************************************************************************
  * @file    net_errors.h
  * @author  MCD Application Team
  * @brief   Defines the network interface error codes
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
#ifndef NET_ERRORS_H
#define NET_ERRORS_H


#ifdef __cplusplus
extern "C" {
#endif

#define NET_OK                               0   /*!< no error */
#define NET_TIMEOUT                         -1   /*!< Timeout rearched during a blocking operation. */
#define NET_ERROR_WOULD_BLOCK               -2   /*!< no data is available but call is non-blocking */
#define NET_ERROR_UNSUPPORTED               -3   /*!< unsupported functionality */
#define NET_ERROR_PARAMETER                 -4   /*!< invalid parameter/configuration */
#define NET_ERROR_NO_CONNECTION             -5   /*!< not connected to a network */
#define NET_ERROR_INVALID_SOCKET            -6   /*!< socket invalid */
#define NET_ERROR_NO_ADDRESS                -7   /*!< IP address is not known */
#define NET_ERROR_NO_MEMORY                 -8   /*!< memory resource not available */
#define NET_ERROR_NO_SSID                   -9   /*!< ssid not found */
#define NET_ERROR_DNS_FAILURE               -10  /*!< DNS failed to complete successfully */
#define NET_ERROR_DHCP_FAILURE              -11  /*!< DHCP failed to complete successfully */
#define NET_ERROR_AUTH_FAILURE              -12  /*!< connection to access point failed */
#define NET_ERROR_DEVICE_ERROR              -13  /*!< failure interfacing with the network processor */
#define NET_ERROR_IN_PROGRESS               -14  /*!< operation (eg connect) in progress */
#define NET_ERROR_ALREADY                   -15  /*!< operation (eg connect) already in progress */
#define NET_ERROR_IS_CONNECTED              -16  /*!< socket is already connected */
#define NET_ERROR_INTERFACE_FAILURE         -17  /*!< an error in interface level */
#define NET_ERROR_DATA                      -18  /*!< an error in interface level */
#define NET_ERROR_SOCKET_FAILURE            -19  /*!< an error in interface level */
#define NET_ERROR_OUT_OF_SOCKET             -20  /*!< no more available socket , open failed */
#define NET_ERROR_CLOSE_SOCKET              -21  /*!< error while closing socket */
#define NET_ERROR_DISCONNECTED              -22  /*!< Connection dropped during the operation. */
#define NET_ERROR_CREATE_SECURE_SOCKET      -23  /*!< failed to create the secure socket */
#define NET_ERROR_IS_NOT_SECURE             -24  /*!< try to set secure option on a non secure socket */
#define NET_ERROR_FRAMEWORK                 -25  /*!< should never happen */
#define NET_ERROR_STATE_TRANSITION          -26  /*!< should never happen */
#define NET_ERROR_INVALID_STATE_TRANSITION  -27  /*!< should never happen */
#define NET_ERROR_INVALID_STATE             -28  /*!< should never happen */
#define NET_ERROR_GENERIC                   -29  /*!< generic error */
#define NET_ERROR_MODULE_INITIALIZATION     -30  /*!< module is not able to initialized  */
#define NET_ERROR_WIFI_CANT_JOIN            -31  /*!< wifi module is not able to join  */



#define NET_ERROR_MBEDTLS_ENTROPY       -100/*!<mbedtls enthropy setup failed */
#define NET_ERROR_MBEDTLS_CRT_PARSE     -101/*!<mbedtls parsing certificat failed */
#define NET_ERROR_MBEDTLS_KEY_PARSE     -102/*!<mbedtls parsing key failed */
#define NET_ERROR_MBEDTLS_SET_HOSTNAME  -103/*!<mbedtls cannot setup hostname*/
#define NET_ERROR_MBEDTLS_SEED          -104/*!<mbedtls seed setup failed */

#define NET_ERROR_MBEDTLS_REMOTE_AUTH   -105/*!<mbedtls remote host could not be authentified */
#define NET_ERROR_MBEDTLS_CONFIG        -106/*!<mbedtls error in config */
#define NET_ERROR_MBEDTLS_SSL_SETUP     -107/*!<mbedtls error setting setup  */
#define NET_ERROR_MBEDTLS_CONNECT       -108/*!<mbedtls error while connecting */
#define NET_ERROR_MBEDTLS               -109/*!<mbedtls error while reading writing data */

#ifdef __cplusplus
}
#endif


#endif /* NET_ERRORS_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
