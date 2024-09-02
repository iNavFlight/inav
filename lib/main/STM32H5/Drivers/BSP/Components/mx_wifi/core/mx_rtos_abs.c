/**
  ******************************************************************************
  * @file    mx_rtos_abs.c
  * @author  MCD Application Team
  * @brief   mx_wifi CMSIS RTOS abstraction module
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

/* Includes ------------------------------------------------------------------*/
#include "mx_wifi_conf.h"


#if (MX_WIFI_USE_CMSIS_OS == 1)
osThreadId thread_new(const char *name, void (*thread)(void const *), void *arg, int stacksize, int prio)
{
#if (osCMSIS < 0x20000U)
  const osThreadDef_t os_thread_def =
  {
    .name = (char *)name,
    .pthread = (os_pthread)thread,
    .tpriority = (osPriority)prio,
    .instances = 0,
    .stacksize = stacksize
  };
  return osThreadCreate(&os_thread_def, arg);

#else
  const osThreadAttr_t attributes =
  {
    .name = name,
    .stack_size = (uint32_t)stacksize,
    .priority = (osPriority_t)prio
  };
  return osThreadNew((osThreadFunc_t) thread, arg, &attributes);
#endif /* (osCMSIS < 0x20000U) */
}

#if (osCMSIS < 0x20000U)
void thread_exit(volatile osThreadId *thread_id)
{
  if (NULL != thread_id)
  {
    if (NULL != *thread_id)
    {
      const osThreadId to_delete = *thread_id;
      *thread_id = NULL;
      osThreadTerminate(to_delete);

      for (;;);
    }
  }
}
#endif /* (osCMSIS < 0x20000U)*/

void *fifo_get(osMessageQId queue, uint32_t timeout)
{
  void *p = NULL;

#if (osCMSIS < 0x20000U)
  osEvent evt = osMessageGet(queue, timeout);
  if (evt.status == osEventMessage)
  {
    p = evt.value.p;
  }
  return p;

#else
  osStatus_t status = osMessageQueueGet(queue, &p, NULL, timeout);
  if (status != osOK)
  {
    p = NULL;
  }

  return p;
#endif /* osCMSIS < 0x20000U */
}


#else  /* MX_WIFI_USE_CMSIS_OS */
/* No OS implementation. */

int32_t noos_sem_signal(volatile uint32_t *sem)
{
  int32_t rc = 0;

  if (sem != NULL)
  {
    *sem = *sem + 1U;
  }
  else
  {
    rc = -1;
  }

  return rc;
}


int32_t noos_sem_wait(__IO uint32_t *sem, uint32_t timeout, void (*idle_func)(uint32_t duration))
{
  int32_t rc = 0;
  const uint32_t tickstart = HAL_GetTick();

  while ((*sem < 1U))
  {
    if ((HAL_GetTick() - tickstart) > timeout)
    {
      rc = -1;
      break;
    }
    if (idle_func != NULL)
    {
      (*idle_func)(timeout - (HAL_GetTick() - tickstart));
    }
  }
  /* no timeout */
  if (rc == 0)
  {
    *sem = *sem - 1U;
  }
  return rc;
}


int32_t noos_fifo_init(noos_queue_t **qret, uint16_t len)
{
  int32_t rc = -1;
  noos_queue_t *q;

  if (len > 0U)
  {
    q = (noos_queue_t *)MX_WIFI_MALLOC(sizeof(noos_queue_t));
    if (q != NULL)
    {
      q->in = 0U;
      q->len = len;
      q->idx = 0;
      q->rd = 0;
      q->wr = 0;
      q->fifo = (void **) MX_WIFI_MALLOC(sizeof(void *) * (len + 1U));
      if (q->fifo != NULL)
      {
        rc = 0;
        *qret = q;
      }
      else
      {
        MX_WIFI_FREE(q);
        *qret = NULL;
      }
    }
  }
  return rc;
}


void noos_fifo_deinit(noos_queue_t *q)
{
  if (q != NULL)
  {
    MX_WIFI_FREE(q->fifo);
    MX_WIFI_FREE(q);
  }
}

int32_t noos_fifo_push(noos_queue_t *queue, void *p, uint32_t timeout, void (*idle_func)(uint32_t duration))
{
  int32_t rc = 0;
  const uint32_t tickstart = HAL_GetTick();

  while (queue->in == queue->len)
  {
    if ((HAL_GetTick() - tickstart) > timeout)
    {
      rc = -1;
      break;
    }
    if (NULL != idle_func)
    {
      (*idle_func)(timeout - (HAL_GetTick() - tickstart));
    }
  }
  if (0 == rc)
  {
    queue->in++;
    queue->fifo[queue->wr] = p;
    queue->wr = (queue->wr + 1U) % queue->len;
  }
  return rc;
}


void *noos_fifo_pop(noos_queue_t *queue, uint32_t timeout, void (*idle_func)(uint32_t duration))
{
  int32_t rc = 0;
  const uint32_t tickstart = HAL_GetTick();
  void *p = NULL;

  while (0U == queue->in)
  {
    if ((HAL_GetTick() - tickstart) > timeout)
    {
      rc = -1;
      break;
    }
    if (NULL != idle_func)
    {
      (*idle_func)(timeout - (HAL_GetTick() - tickstart));
    }
  }

  if (0 == rc)
  {
    p = queue->fifo[queue->rd];
    queue->rd = (queue->rd + 1U) % queue->len;
    queue->in--;
  }

  return p;
}

#endif /* MX_WIFI_USE_CMSIS_OS */
