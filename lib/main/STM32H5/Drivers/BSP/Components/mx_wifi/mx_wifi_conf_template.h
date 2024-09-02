/**
  ******************************************************************************
  * @file    mx_wifi_conf_template.h
  * @author  MCD Application Team
  * @brief   Header for mx_wifi_conf_template module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MX_WIFI_CONF_TEMPLATE_H
#define MX_WIFI_CONF_TEMPLATE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <inttypes.h>
#include "main.h" /* Inherit some declarations from the current platform. */


#ifndef MX_WIFI_USE_SPI
#define MX_WIFI_USE_SPI                             (0)
#endif /* MX_WIFI_USE_SPI */

/* SPI DMA usage, default on */
#ifndef DMA_ON_USE
#define DMA_ON_USE                                  (1)
#endif /* DMA_ON_USE */

/* Do not use RTOS but bare metal approach by default. */
#ifndef MX_WIFI_USE_CMSIS_OS
#define MX_WIFI_USE_CMSIS_OS                        (0)
#endif /* MX_WIFI_USE_CMSIS_OS */

/* Use AT command mode by default, TCP/IP stack is running on the module. */
#ifndef MX_WIFI_NETWORK_BYPASS_MODE
#define MX_WIFI_NETWORK_BYPASS_MODE                 (0)
#endif /* MX_WIFI_NETWORK_BYPASS_MODE */


/* Do not copy TX buffer. */
#ifndef MX_WIFI_TX_BUFFER_NO_COPY
#define MX_WIFI_TX_BUFFER_NO_COPY                   (1)
#endif /* MX_WIFI_TX_BUFFER_NO_COPY */


/* DEBUG LOG */
/* #define MX_WIFI_API_DEBUG  */
/* #define MX_WIFI_IPC_DEBUG  */
/* #define MX_WIFI_HCI_DEBUG  */
/* #define MX_WIFI_SLIP_DEBUG */
/* #define MX_WIFI_IO_DEBUG   */



#if (MX_WIFI_USE_CMSIS_OS == 1)
#include "mx_wifi_cmsis_os.h"

#else
#include "mx_wifi_bare_os.h"
#endif /* MX_WIFI_USE_CMSIS_OS */

#define MX_WIFI_PRODUCT_NAME                        ("MXCHIP-WIFI")

#define MX_WIFI_PRODUCT_ID                          ("EMW3080B")

#ifndef MX_WIFI_UART_BAUDRATE
#define MX_WIFI_UART_BAUDRATE                       (115200*2)
#endif /* MX_WIFI_UART_BAUDRATE */

#ifndef MX_WIFI_MTU_SIZE
#define MX_WIFI_MTU_SIZE                            (1500)
#endif /* MX_WIFI_MTU_SIZE */

#define MX_WIFI_BYPASS_HEADER_SIZE                  (28)  /* MIPC_HEADER_SIZE(6) + sizeof(wifi_bypass_in_rparams_t)(22)
                                                           * set with PBUF_LINK_ENCAPSULATION_HLEN in case of LwIP usage. */

#define MX_WIFI_PBUF_LINK_HLEN                      (14)  /* Link header (PBUF_LINK_HLEN) in case of LwIP usage. */

#if (MX_WIFI_NETWORK_BYPASS_MODE == 1)
/* Use LwIP PBUF_POOL_BUFSIZE = TCP_MSS + 40 + PBUF_LINK_ENCAPSULATION_HLEN + PBUF_LINK_HLEN. */
#define MX_WIFI_BUFFER_SIZE \
  (MX_WIFI_MTU_SIZE + MX_WIFI_BYPASS_HEADER_SIZE + MX_WIFI_PBUF_LINK_HLEN)

#else
#define MX_WIFI_BUFFER_SIZE                         (2500)  /* Bigger buffer size (needed with some other use cases) */
#endif /* (MX_WIFI_NETWORK_BYPASS_MODE == 1) */

#define MX_WIFI_IPC_PAYLOAD_SIZE                    (MX_WIFI_BUFFER_SIZE - 6) /* MX_WIFI_BUFFER_SIZE - MIPC_HEADER_SIZE */

#define MX_WIFI_SOCKET_DATA_SIZE                    (MX_WIFI_IPC_PAYLOAD_SIZE - 12) /* MX_WIFI_IPC_PAYLOAD_SIZE - socket_api_params_header */

#ifndef MX_WIFI_CMD_TIMEOUT
#define MX_WIFI_CMD_TIMEOUT                         (10000)  /* default 10s timeout */
#endif /* MX_WIFI_CMD_TIMEOUT */

#define MX_WIFI_MAX_SOCKET_NBR                      (8)

#define MX_WIFI_MAX_DETECTED_AP                     (10)

#define MX_WIFI_MAX_SSID_NAME_SIZE                  (32)

#define MX_WIFI_MAX_PSWD_NAME_SIZE                  (64)

#define MX_WIFI_PRODUCT_NAME_SIZE                   (32)

#define MX_WIFI_PRODUCT_ID_SIZE                     (32)

#define MX_WIFI_FW_REV_SIZE                         (24)

#ifndef MX_WIFI_SPI_THREAD_PRIORITY
#define MX_WIFI_SPI_THREAD_PRIORITY                 (OSPRIORITYREALTIME)
#endif /* MX_WIFI_SPI_THREAD_PRIORITY */

#ifndef MX_WIFI_SPI_THREAD_STACK_SIZE
#define MX_WIFI_SPI_THREAD_STACK_SIZE               (1024)
#endif /* MX_WIFI_SPI_THREAD_STACK_SIZE */

#ifndef MX_WIFI_UART_THREAD_PRIORITY
#define MX_WIFI_UART_THREAD_PRIORITY                (OSPRIORITYREALTIME)
#endif /* MX_WIFI_UART_THREAD_PRIORITY */

#ifndef MX_WIFI_UART_THREAD_STACK_SIZE
#define MX_WIFI_UART_THREAD_STACK_SIZE              (1024)
#endif /* MX_WIFI_UART_THREAD_STACK_SIZE */

#ifndef MX_WIFI_RECEIVED_THREAD_PRIORITY
#define MX_WIFI_RECEIVED_THREAD_PRIORITY            (OSPRIORITYABOVENORMAL)
#endif /* MX_WIFI_RECEIVED_THREAD_PRIORITY */

#ifndef MX_WIFI_RECEIVED_THREAD_STACK_SIZE
#define MX_WIFI_RECEIVED_THREAD_STACK_SIZE          (1024)
#endif /* MX_WIFI_RECEIVED_THREAD_STACK_SIZE*/

#ifndef MX_WIFI_TRANSMIT_THREAD_PRIORITY
#define MX_WIFI_TRANSMIT_THREAD_PRIORITY            (OSPRIORITYNORMAL)
#endif /* MX_WIFI_TRANSMIT_THREAD_PRIORITY */

#ifndef MX_WIFI_TRANSMIT_THREAD_STACK_SIZE
#define MX_WIFI_TRANSMIT_THREAD_STACK_SIZE          (1024)
#endif /* MX_WIFI_TRANSMIT_THREAD_STACK_SIZE */


/* Maximum number of RX buffer that can be queued by Hardware interface (SPI/UART)                         */
/* This is used to size internal queue, and avoid to block the IP thread if it can still push some buffers */
/* Impact on memory foot print is one single void* per place in the queue, but it may lead to              */
/* over allocation TCP/IP stack.                                                                           */
#ifndef MX_WIFI_MAX_RX_BUFFER_COUNT
#define MX_WIFI_MAX_RX_BUFFER_COUNT                 (2)
#endif /* MX_WIFI_MAX_RX_BUFFER_COUNT */


/* Maximum number of TX buffer that can be queued by IP stack (LwIP or Netx) without blocking the calling thread */
/* This is used to size internal queue, and avoid to block the IP thread if it can still push some buffers       */
/* Impact on memory foot print is one single void* per place in the queue, but it may lead to over allocation    */
/* TCP/IP stack (LwIP for instance )                                                                             */
#ifndef MX_WIFI_MAX_TX_BUFFER_COUNT
#define MX_WIFI_MAX_TX_BUFFER_COUNT                 (4)
#endif /* MX_WIFI_MAX_TX_BUFFER_COUNT */


/**
  * For the TX buffer, by default no-copy feature is enabled, meaning that
  * the IP buffer are used in the whole process and should come with
  * available room in front of payload to accommodate transport header buffer.
  * This is managed in interface between driver and IP stack.
  */

/**
  * For LwIP "PBUF_LINK_ENCAPSULATION_HLEN" must be defined as > MX_WIFI_MIN_TX_HEADER_SIZE,
  * see net_mx_wifi/c file for implementation.
  */
#define MX_WIFI_MIN_TX_HEADER_SIZE                   (28)

/* Size of the circular buffer for UART mode, when buffer is half full data are transmitted to next stage. */

#ifndef MX_CIRCULAR_UART_RX_BUFFER_SIZE
#define MX_CIRCULAR_UART_RX_BUFFER_SIZE              (400)
#endif /* MX_CIRCULAR_UART_RX_BUFFER_SIZE */

#ifndef MX_STAT_ON
#define MX_STAT_ON      0
#endif /* MX_STAT_ON */

#if (MX_STAT_ON == 1)
typedef struct
{
  uint32_t alloc;
  uint32_t free;
  uint32_t cmd_get_answer;
  uint32_t callback;
  uint32_t in_fifo;
  uint32_t out_fifo;
} mx_stat_t;

extern mx_stat_t mx_stat;

#define MX_STAT_LOG()                                                                                                                  \
  (void) printf("\n Number of allocated buffer for Rx and command answer %" PRIu32 "\n", mx_stat.alloc);                               \
  (void) printf(" Number of freed buffer %" PRIu32 "\n", mx_stat.free);                                                                \
  (void) printf(" Number of command answer %" PRIu32 ", callback %" PRIu32 ", sum of both %" PRIu32 " (should match alloc && free)\n", \
                mx_stat.cmd_get_answer, mx_stat.callback, mx_stat.cmd_get_answer + mx_stat.callback);                                  \
  (void) printf(" Number of posted answer (callback + cmd answer) %" PRIu32 ", processed answer %" PRIu32 "\n\n",                      \
                mx_stat.in_fifo, mx_stat.out_fifo);

#define MX_STAT_INIT()        (void) memset((void*)&mx_stat, 0, sizeof(mx_stat))
#define MX_STAT(A)            mx_stat.A++
#define MX_STAT_DECLARE()     mx_stat_t mx_stat

#else /* MX_STAT_ON */
#define MX_STAT_INIT()
#define MX_STAT(A)
#define MX_STAT_LOG()
#define MX_STAT_DECLARE()
#endif /* MX_STAT_ON */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MX_WIFI_CONF_TEMPLATE_H */
