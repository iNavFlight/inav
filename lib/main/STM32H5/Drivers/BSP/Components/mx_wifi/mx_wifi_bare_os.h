/**
  ******************************************************************************
  * @file    mx_wifi_bare_os.h
  * @author  MCD Application Team
  * @brief   Header for mx_wifi_conf module
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
#ifndef MX_WIFI_BARE_OS_H
#define MX_WIFI_BARE_OS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/* Memory management ---------------------------------------------------------*/
#ifndef MX_WIFI_MALLOC
#define MX_WIFI_MALLOC malloc
#endif /* MX_WIFI_MALLOC */

#ifndef MX_WIFI_FREE
#define MX_WIFI_FREE free
#endif /* MX_WIFI_FREE */


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

#define OSPRIORITYNORMAL                          0

#define MX_ASSERT(A)     \
  do                     \
  {                      \
    ((void)0);           \
  } while((A) != true)     /* ; */


#define LOCK_DECLARE(A)                                   \
  volatile bool /*(*/ A /*)*/

#define LOCK_INIT(A)                                      \
  (A) = false

#define LOCK_DEINIT(A)
#define LOCK(A)                                           \
  while(A){}; (A) = true
#define UNLOCK(A)                                         \
  (A) = false

#define SEM_DECLARE(A)                                    \
  volatile uint32_t /*(*/ A /*)*/

#define SEM_INIT(A, COUNT)                               \
  (A) = 0

#define SEM_DEINIT(A)

#define SEM_SIGNAL(A)                                    \
  noos_sem_signal(&(A))

#define SEM_WAIT(A, TIMEOUT, IDLE_FUNC)                  \
  noos_sem_wait(&(A), (TIMEOUT), (IDLE_FUNC))


#define THREAD_DECLARE(A)                                \
  uint32_t /*(*/ A /*)*/

#define THREAD_INIT(A, THREAD_FUNC, THREAD_CONTEXT, STACKSIZE, PRIORITY) \
  THREAD_OK

#define THREAD_DEINIT(A)                          (void) 0
#define THREAD_TERMINATE()                          \

#define THREAD_CONTEXT_TYPE                         \
  void*


#define FIFO_DECLARE(QUEUE)                         \
  noos_queue_t* /*(*/ QUEUE /*)*/

#define FIFO_INIT(QUEUE, QSIZE)                     \
  noos_fifo_init(&(QUEUE), (QSIZE))

#define FIFO_PUSH(QUEUE, VALUE, TIMEOUT, IDLE_FUNC) \
  noos_fifo_push((QUEUE), (VALUE), (TIMEOUT), (IDLE_FUNC))

#define FIFO_POP(QUEUE, TIMEOUT, IDLE_FUNC)         \
  noos_fifo_pop((QUEUE), (TIMEOUT), (IDLE_FUNC))

#define FIFO_DEINIT(QUEUE)                          \
  noos_fifo_deinit((QUEUE))


#define WAIT_FOREVER   \
  0xFFFFFFFFU
#define SEM_OK         \
  0
#define THREAD_OK      \
  0
#define QUEUE_OK       \
  0
#define FIFO_OK        \
  0
#define DELAY_MS(N)    \
  HAL_Delay((N))



typedef struct noos_queue
{
  uint16_t   len;
  uint16_t   in;
  void     **fifo;
  uint32_t   idx;
  uint32_t   rd;
  uint32_t   wr;
} noos_queue_t;


/**
  * @brief  Init FIFO queue without OS
  * @param  qret: pointer to the queue created
  * @param  len: queue size
  * @retval 0 success, otherwise failed.
  */
int32_t noos_fifo_init(noos_queue_t **qret, uint16_t len);


/**
  * @brief  DeInit FIFO queue without OS
  * @param  q: pointer to the queue
  */
void noos_fifo_deinit(noos_queue_t *q);

/**
  * @brief  Push msg into the FIFO queue without OS
  * @param  queue: msg queue handle
  * @param  p: pointer to the msg to be pushed
  * @param  timeout: timeout in milliseconds
  * @param  idle_func: the function call this idle_func internally before timeout
  * @retval 0 success, otherwise failed.
  */
int32_t noos_fifo_push(noos_queue_t *queue, void *p, uint32_t timeout, void (*idle_func)(uint32_t duration));


/**
  * @brief  Get msg from FIFO queue.
  * @param  queue: msg queue handle
  * @param  timeout: timeout in milliseconds
  * @param  idle_func: the function call this idle_func internally before timeout
  * @retval pointer to the msg got from the queue, NULL if failed.
  */
void *noos_fifo_pop(noos_queue_t *queue, uint32_t timeout, void (*idle_func)(uint32_t duration));


/**
  * @brief  Wait a semaphore without OS
  * @param  sem: pointer to the semaphore
  * @param  timeout: timeout in milliseconds
  * @param  idle_func: the function call this idle_func internally before timeout
  * @retval 0 success, otherwise timeout.
  */
int32_t noos_sem_wait(volatile uint32_t *sem, uint32_t timeout, void (*idle_func)(uint32_t duration));


/**
  * @brief  Signal a semaphore without OS
  * @param  sem: pointer to the semaphore
  * @retval 0 success, otherwise failed.
  */
int32_t noos_sem_signal(volatile uint32_t *sem);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MX_WIFI_BARE_OS_H */
