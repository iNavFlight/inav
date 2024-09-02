/**
  ******************************************************************************
  * @file    mx_wifi_cmsis_os.h
  * @author  MCD Application Team
  * @brief   CMSIS OS Header for mx_wifi_conf module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MX_WIFI_CMSIS_OS_H
#define MX_WIFI_CMSIS_OS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os.h"

/* Memory management ---------------------------------------------------------*/
#ifndef MX_WIFI_MALLOC
#define MX_WIFI_MALLOC pvPortMalloc
#endif /* MX_WIFI_MALLOC */

#ifndef MX_WIFI_FREE
#define MX_WIFI_FREE vPortFree
#endif /* MX_WIFI_FREE */

#if (MX_WIFI_NETWORK_BYPASS_MODE == 1)
/* Definition for LwIP usage. */

#include "lwip/pbuf.h"
typedef struct pbuf mx_buf_t;

#define MX_NET_BUFFER_ALLOC(len)                  pbuf_alloc(PBUF_RAW, (len), PBUF_POOL)

#define MX_NET_BUFFER_FREE(p)                     pbuf_free((p))

#define MX_NET_BUFFER_HIDE_HEADER(p, n)           pbuf_header((p), (int16_t) - (n))

#define MX_NET_BUFFER_PAYLOAD(p)                  (p)->payload

#define MX_NET_BUFFER_SET_PAYLOAD_SIZE(p, size)   pbuf_realloc((p), (size))

#define MX_NET_BUFFER_GET_PAYLOAD_SIZE(p)         (p)->len

#else

typedef struct mx_buf
{
  uint32_t len;
  uint32_t header_len;
  uint8_t  data[1];
} mx_buf_t;

static inline mx_buf_t *mx_buf_alloc(uint32_t len)
{
  mx_buf_t *p = (mx_buf_t *) MX_WIFI_MALLOC(len + sizeof(mx_buf_t) -1U);
  if (NULL != p)
  {
    p->len = len;
    p->header_len = 0;
  }
  return p;
}

#define MX_NET_BUFFER_ALLOC(len)                  mx_buf_alloc(len)
#define MX_NET_BUFFER_FREE(p)                     MX_WIFI_FREE(p)
#define MX_NET_BUFFER_HIDE_HEADER(p, n)           (p)->header_len += (n)
#define MX_NET_BUFFER_PAYLOAD(p)                  &(p)->data[(p)->header_len]
#define MX_NET_BUFFER_SET_PAYLOAD_SIZE(p, size)   (p)->len = (size)
#define MX_NET_BUFFER_GET_PAYLOAD_SIZE(p)         (p)->len

#endif /* MX_WIFI_NETWORK_BYPASS_MODE */


#define OSPRIORITYNORMAL                          osPriorityNormal
#define OSPRIORITYABOVENORMAL                     osPriorityAboveNormal
#define OSPRIORITYREALTIME                        osPriorityRealtime

#define MX_ASSERT(A)     \
  do                     \
  {                      \
    ((void)0);           \
  } while((A) != true)     /* ; */


#define LOCK_DECLARE(A)                                   \
  osMutexId /*(*/ A /*)*/

#if (osCMSIS < 0x20000U)
#define LOCK_INIT(A)                                      \
  osMutexDef(MUTEX); (A) = osMutexCreate(osMutex(MUTEX))

#else
#define LOCK_INIT(A)                                      \
  osMutexDef(MUTEX); (A) = osMutexNew(osMutex(MUTEX))
#endif /* (osCMSIS < 0x20000U) */

#define LOCK_DEINIT(A)                                    \
  osMutexDelete((A))

#define LOCK(A)                                           \
  if (osMutexWait((A), osWaitForever) != osOK) {while(true){};}

#define UNLOCK(A)                                         \
  if (osMutexRelease((A)) != osOK) {while(true){};}

#define SEM_DECLARE(A)                                    \
  osSemaphoreId /*(*/ A /*)*/

#define SEMAPHORE_DEF(NAME)                               \
  osSemaphoreDef(NAME)

#define SEMAPHORE(NAME)                                   \
  osSemaphore(NAME)

#if (osCMSIS < 0x20000U)
#define SEM_INIT(A, COUNT)                               \
  SEMAPHORE_DEF(__LINE__);                               \
  (A) = osSemaphoreCreate(SEMAPHORE(__LINE__), (COUNT)); \
  for(uint32_t jj = 0; jj < (COUNT); jj++) {             \
    (void)osSemaphoreWait((A), 0);                       \
  }

#else
#define SEM_INIT(A, COUNT)                               \
  SEMAPHORE_DEF(__LINE__);                               \
  (A) = osSemaphoreNew((COUNT), 0, SEMAPHORE(__LINE__))
#endif /* (osCMSIS < 0x20000U) */

#define SEM_DEINIT(A)                                    \
  osSemaphoreDelete((A))

#define SEM_SIGNAL(A)                                    \
  osSemaphoreRelease((A))

#if (osCMSIS < 0x20000U)
#define SEM_WAIT(A, TIMEOUT, IDLE_FUNC)                  \
  osSemaphoreWait((A), (TIMEOUT))

#else
#define SEM_WAIT(A, TIMEOUT, IDLE_FUNC)                  \
  osSemaphoreAcquire((A), (TIMEOUT))
#endif /* (osCMSIS < 0x20000U) */


#define THREAD_DECLARE(A)                                \
  osThreadId /*(*/ A /*)*/

#define THREAD_INIT(A, THREAD_FUNC, THREAD_CONTEXT, STACKSIZE, PRIORITY)           \
  ((A) = thread_new(#A, (THREAD_FUNC), (THREAD_CONTEXT), (STACKSIZE), (PRIORITY)), \
   ((A) != NULL) ? osOK : osErrorOS)

#if (osCMSIS < 0x20000U)
#define THREAD_DEINIT(A)                            \
  thread_exit(&(A))
#else
#define THREAD_DEINIT(A)                            \
  osThreadTerminate(A)
#endif /* (osCMSIS < 0x20000U) */

#if (osCMSIS < 0x20000U)
#define THREAD_TERMINATE()
#else
#define THREAD_TERMINATE()                          \
  osThreadExit()
#endif /* (osCMSIS < 0x20000U) */

#define THREAD_CONTEXT_TYPE                         \
  void const *


#define FIFO_DECLARE(QUEUE)                         \
  osMessageQId /*(*/ QUEUE /*)*/

#if (osCMSIS < 0x20000U)
#define FIFO_INIT(QUEUE, QSIZE)                     \
  osMessageQDef(/*(*/ QUEUE /*)*/, (QSIZE), void *);\
  QUEUE = osMessageCreate(osMessageQ(QUEUE), NULL)
#else
#define FIFO_INIT(QUEUE, QSIZE)                     \
  QUEUE = osMessageQueueNew((QSIZE), sizeof(void*), NULL)
#endif /* (osCMSIS < 0x20000U) */

#if (osCMSIS < 0x20000U)
#define FIFO_PUSH(QUEUE, VALUE, TIMEOUT, IDLE_FUNC) \
  osMessagePut((QUEUE), (uint32_t) (VALUE), (TIMEOUT))
#else
#define FIFO_PUSH(QUEUE, VALUE, TIMEOUT, IDLE_FUNC) \
  osMessageQueuePut((QUEUE), (void const*) &VALUE, 0, (TIMEOUT))
#endif /* (osCMSIS < 0x20000U) */

#define FIFO_POP(QUEUE, TIMEOUT, IDLE_FUNC)         \
  (void*)fifo_get((QUEUE), (TIMEOUT))

#if (osCMSIS < 0x20000U)
#define FIFO_DEINIT(QUEUE)                          \
  osMessageDelete((QUEUE))
#else
#define FIFO_DEINIT(QUEUE)                          \
  osMessageQueueDelete((QUEUE))
#endif /* (osCMSIS < 0x20000U) */


#define WAIT_FOREVER   \
  osWaitForever
#define SEM_OK         \
  osOK
#define THREAD_OK      \
  osOK
#define QUEUE_OK       \
  osOK
#define FIFO_OK        \
  osOK
#define DELAY_MS(N)    \
  osDelay((N))


/**
  * @brief  Get msg from FIFO queue.
  * @param  queue: msg queue handle
  * @param  timeout: timeout in milliseconds
  * @retval pointer to the msg got from the queue, NULL if failed.
  */
void *fifo_get(osMessageQId queue, uint32_t timeout);


/**
  * @brief  Create a new thread with CMSIS-RTOS API.
  * @param  name: name of the thread
  * @param  thread: thread function
  * @param  arg: argument for the thread
  * @param  stacksize: stack size of the thread
  * @param  prio: priority of the thread
  * @retval thread id of the new thread, NULL if failed.
  */
osThreadId thread_new(const char *name, void (*thread)(void const *), void *arg, int stacksize, int prio);

#if (osCMSIS < 0x20000U)
void thread_exit(volatile osThreadId *thread_id);
#endif /* (osCMSIS < 0x20000U) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MX_WIFI_CMSIS_OS_H */
