/**
  ******************************************************************************
  * @file    net_conf_template.h
  * @author  MCD Application Team
  * @brief   Configures the network socket APIs.
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
#ifndef NET_CONF_TEMPLATE_H
#define NET_CONF_TEMPLATE_H

#ifdef __cplusplus
extern "C" {
#endif
/* disable Misra rule to enable doxigen comment , A sectio of code appear to have been commented out */
/*cstat -MISRAC2012-Dir-4.4 */


/*cstat -MISRAC* -DEFINE-* -CERT-EXP19*  */
#include <stdio.h>
/*cstat +MISRAC* +DEFINE-* +CERT-EXP19*  */

/* Please uncomment if you want socket address defintion from LWIP include file rather than local one  */
/* This is recommended if network interface uses LWIP to save some code size. This is required if      */
/* project uses IPv6 */

/* #define NET_USE_LWIP_DEFINITIONS  */

/* Experimental : Please uncomment if you want to use only control part of network library              */
/* net_socket api are directly redifined to LWIP ,NET_MBEDTLS_HOST_SUPPORT is not supported with        */
/* this mode, dedicated to save memory (4K code)                                                        */
/* #define NET_BYPASS_NET_SOCKET */


/* Please uncomment if secure socket have to be supported and is implemented thanks to MBEDTLS running on MCU */
/* #define NET_MBEDTLS_HOST_SUPPORT */

/* Please uncomment if device supports internally Secure TCP connection */
/* #define NET_MBEDTLS_DEVICE_SUPPORT */

#ifdef NET_USE_RTOS
/*cstat -MISRAC* -DEFINE-* -CERT-EXP19*  */
#include "cmsis_os.h"
/*cstat +MISRAC* +DEFINE-* +CERT-EXP19*  */
#endif /* NET_USE_RTOS */





/* when using LWIP , size of hostname */
#define NET_IP_HOSTNAME_MAX_LEN        32

#ifndef NET_USE_IPV6
#define NET_USE_IPV6    0
#endif /* NET_USE_IPV6 */

#if NET_USE_IPV6 && !defined(NET_USE_LWIP_DEFINTIONS)
#error "NET IPV6 required to define NET_USE_LWIP_DEFINTIONS"
#endif /* NET_USE_IPV6 */


/* MbedTLS configuration */
#ifdef NET_MBEDTLS_HOST_SUPPORT


#if !defined NET_MBEDTLS_DEBUG_LEVEL
#define NET_MBEDTLS_DEBUG_LEVEL 1
#endif /*   NET_MBEDTLS_DEBUG_LEVEL */

#if !defined NET_MBEDTLS_CONNECT_TIMEOUT
#define NET_MBEDTLS_CONNECT_TIMEOUT     10000U
#endif /* NET_MBEDTLS_CONNECT_TIMEOUT */

#if !defined(MBEDTLS_CONFIG_FILE)
#define MBEDTLS_CONFIG_FILE "mbedtls/config.h"
#endif /* MBEDTLS_CONFIG_FILE */
#endif /* NET_MBEDTLS_HOST_SUPPORT */

#if !defined(NET_MAX_SOCKETS_NBR)
#define NET_MAX_SOCKETS_NBR            5
#endif /* NET_MAX_SOCKETS_NBR */

#define NET_IF_NAME_LEN                128
#define NET_DEVICE_NAME_LEN            64
#define NET_DEVICE_ID_LEN              64
#define NET_DEVICE_VER_LEN             64


#define NET_SOCK_DEFAULT_RECEIVE_TO    60000
#define NET_SOCK_DEFAULT_SEND_TO       60000
#define NET_UDP_MAX_SEND_BLOCK_TO      1024

#if !defined(NET_USE_DEFAULT_INTERFACE)
#define NET_USE_DEFAULT_INTERFACE      1
#endif /* NET_USE_DEFAULT_INTERFACE */

#ifdef NET_USE_RTOS

#if ( osCMSIS < 0x20000U)
#define RTOS_SUSPEND  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) { (void) vTaskSuspendAll(); }
#define RTOS_RESUME   if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) { (void) xTaskResumeAll(); }
#else
#define RTOS_SUSPEND    (void) osKernelLock()
#define RTOS_RESUME     (void) osKernelUnlock()
#endif /* osCMSIS */

#else
#define RTOS_SUSPEND
#define RTOS_RESUME
#endif /* NET_USE_RTOS */

#if !defined(NET_DBG_INFO)
#define NET_DBG_INFO(...)
/*
#define NET_DBG_INFO(...)  do { \
                                RTOS_SUSPEND; \
                                (void) printf(__VA_ARGS__); \
                                RTOS_RESUME; \
                              } while (0)
*/
#endif /* NET_DBG_INFO */

#if !defined(NET_DBG_ERROR)
#define NET_DBG_ERROR(...)  do { \
                                 RTOS_SUSPEND; \
                                 (void) printf("\nERROR: %s:%d ",__FILE__,__LINE__) ;\
                                 (void)printf(__VA_ARGS__);\
                                 (void)printf("\n"); \
                                 RTOS_RESUME; \
                               } while (false)
#endif /* NET_DBG_ERROR */

#if !defined(NET_DBG_PRINT)
#define NET_DBG_PRINT(...)  do { \
                                 RTOS_SUSPEND; \
                                 (void)printf("%s:%d ",__FILE__,__LINE__) ;\
                                 (void)printf(__VA_ARGS__);\
                                 (void)printf("\n"); \
                                 RTOS_RESUME; \
                               } while (false)
#endif /* NET_DBG_PRINT */

#if !defined(NET_ASSERT)
#define NET_ASSERT(test,...)  do { if (!(test)) {\
                                   RTOS_SUSPEND; \
                                   (void) printf("Assert Failed %s %d :",__FILE__,__LINE__);\
                                   (void) printf(__VA_ARGS__);\
                                   RTOS_RESUME; \
                                 while(true) {}; }\
                                 } while (false)
#endif /* NET_ASSERT */

#if !defined(NET_PRINT)
#define NET_PRINT(...)  do { \
                                 RTOS_SUSPEND; \
                                 (void) printf(__VA_ARGS__);\
                                 (void) printf("\n"); \
                                 RTOS_RESUME; \
                               } while (false)
#endif /* NET_PRINT */

#if !defined(NET_PRINT_WO_CR)
#define NET_PRINT_WO_CR(...)   do { \
                                    RTOS_SUSPEND; \
                                    (void) printf(__VA_ARGS__);\
                                    RTOS_RESUME; \
                                  } while (false)
#endif /* NET_PRINT_WO_CR */

#if !defined(NET_WARNING)
#define NET_WARNING(...)  do { \
                                 RTOS_SUSPEND; \
                                 (void) printf("Warning %s:%d ",__FILE__,__LINE__) ;\
                                 (void) printf(__VA_ARGS__);\
                                 (void) printf("\n"); \
                                 RTOS_RESUME; \
                               } while (false)
#endif /* NET_WARNING */



#ifndef NET_PERF_MAXTHREAD
#define NET_PERF_MAXTHREAD      10U
#endif /* NET_PERF_MAXTHREAD  */


#ifdef __cplusplus
}
#endif

#endif /* NET_CONF_TEMPLATE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
