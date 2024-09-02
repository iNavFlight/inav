/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MX_WIFI_AZURE_RTOS_CONF_H
#define MX_WIFI_AZURE_RTOS_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#if (MX_WIFI_USE_CMSIS_OS != 0)
#error The NetXDuo wifi driver does not support CMSIS OS
#endif

#include <stdint.h>
#include <stddef.h>

#include "tx_api.h"
#include "nx_api.h"

UINT mx_wifi_alloc_init();
void * mx_wifi_malloc(size_t size);
void mx_wifi_free(void * p);

#define MX_WIFI_MALLOC(size) mx_wifi_malloc(size)
#define MX_WIFI_FREE(p) mx_wifi_free(p)

#define NET_MALLOC(size) mx_wifi_malloc(size)
#define NET_FREE(p) mx_wifi_free(p)


typedef NX_PACKET mx_buf_t;

NX_PACKET *mx_net_buffer_alloc(uint32_t n);
void mx_net_buffer_free(NX_PACKET *nx_packet);

#define MX_NET_BUFFER_ALLOC                             mx_net_buffer_alloc
#define MX_NET_BUFFER_FREE                              mx_net_buffer_free
#define MX_NET_BUFFER_PAYLOAD(packet_ptr)               packet_ptr->nx_packet_prepend_ptr
#define MX_NET_BUFFER_SET_PAYLOAD_SIZE(packet_ptr,n)    \
  do {                                                  \
    packet_ptr->nx_packet_length = n;                   \
    packet_ptr->nx_packet_append_ptr =                  \
      packet_ptr->nx_packet_prepend_ptr + n;            \
  } while (0)
#define MX_NET_BUFFER_GET_PAYLOAD_SIZE(packet_ptr)      packet_ptr->nx_packet_length
#define MX_NET_BUFFER_HIDE_HEADER(packet_ptr,n)         \
  do {                                                  \
    packet_ptr->nx_packet_prepend_ptr += n;             \
    packet_ptr->nx_packet_length -=n ;                  \
  } while (0)

#define DELAYms(n)                                              \
  do {                                                          \
    if (n == 1) {                                               \
      tx_thread_relinquish();                                   \
    } else {                                                    \
      tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND * n / 1000);    \
    }                                                           \
  } while(0)

#define MX_ASSERT(a)                                    do {} while(!(a))

#define LOCK_DECLARE(A)                                 TX_MUTEX A
#define LOCK_INIT(A)                                    tx_mutex_create(&A, #A, TX_NO_INHERIT)
#define LOCK_DEINIT(A)                                  tx_mutex_delete(&A)
#define LOCK(A)                                         tx_mutex_get(&A, TX_WAIT_FOREVER)
#define UNLOCK(A)                                       tx_mutex_put(&A)

#define SEM_DECLARE(A)                                  TX_SEMAPHORE A
#define SEM_INIT(A,COUNT)                               tx_semaphore_create(&A, #A, 0)
#define SEM_DEINIT(A)                                   tx_semaphore_delete(&A)
#define SEM_SIGNAL(A)                                   tx_semaphore_put(&A)
#define SEM_WAIT(A,TIMEOUT,IDLE_FUNC)                   tx_semaphore_get(&A, TIMEOUT)

UINT mx_wifi_thread_init(TX_THREAD * thread_ptr, CHAR *name_ptr, VOID (*entry_function)(ULONG), ULONG entry_input, ULONG stack_size, UINT priority);
UINT mx_wifi_thread_deinit(TX_THREAD * thread_ptr);
void mx_wifi_thread_terminate();

#define THREAD_CONTEXT_TYPE                             ULONG

#define OSPRIORITYNORMAL                                5
#define OSPRIORITYABOVENORMAL                           4
#define OSPRIORITYREALTIME                              1

#define THREAD_DECLARE(A)                               TX_THREAD A
#define THREAD_INIT(A,THREAD_FUNC,THREAD_CONTEXT,STACKSIZE,PRIORITY) \
  mx_wifi_thread_init(&A, #A, (VOID (*)(ULONG)) THREAD_FUNC, (ULONG) THREAD_CONTEXT, STACKSIZE, PRIORITY)
#define THREAD_DEINIT(A)                                mx_wifi_thread_deinit(&A)
#define THREAD_TERMINATE()                              mx_wifi_thread_terminate()

UINT mx_wifi_fifo_init(TX_QUEUE * queue_ptr, char * name_ptr, ULONG size);
UINT mx_wifi_fifo_deinit(TX_QUEUE * queue_ptr);
UINT mx_wifi_fifo_push(TX_QUEUE * queue_ptr, void * source_ptr, ULONG wait_option);
void * mx_wifi_fifo_pop(TX_QUEUE * queue_ptr, ULONG wait_option);

#define FIFO_DECLARE(QUEUE)                             TX_QUEUE QUEUE
#define FIFO_INIT(QUEUE,QSIZE)                          mx_wifi_fifo_init(&QUEUE, #QUEUE, QSIZE)
#define FIFO_DEINIT(QUEUE)                              mx_wifi_fifo_deinit(&QUEUE)
#define FIFO_PUSH(QUEUE,VALUE,TIMEOUT,IDLE_FUNC)        mx_wifi_fifo_push(&QUEUE,&VALUE,TIMEOUT)
#define FIFO_POP(QUEUE,TIMEOUT,IDLE_FUNC)               mx_wifi_fifo_pop(&QUEUE,TIMEOUT)

#define WAIT_FOREVER                                    TX_WAIT_FOREVER
#define SEM_OK                                          TX_SUCCESS
#define THREAD_OK                                       TX_SUCCESS
#define FIFO_OK                                         TX_SUCCESS

#ifdef __cplusplus
}
#endif

#endif /* MX_WIFI_AZURE_RTOS_CONF_H */
