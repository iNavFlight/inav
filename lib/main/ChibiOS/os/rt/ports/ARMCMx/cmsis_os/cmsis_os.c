/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio.

    This file is part of ChibiOS.

    ChibiOS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*
   Concepts and parts of this file have been contributed by Andre R.
 */

/**
 * @file    cmsis_os.c
 * @brief   CMSIS RTOS module code.
 *
 * @addtogroup CMSIS_OS
 * @{
 */

#include "cmsis_os.h"
#include <string.h>

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

int32_t cmsis_os_started;

/*===========================================================================*/
/* Module local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables.                                                   */
/*===========================================================================*/

static memory_pool_t sempool;
static semaphore_t semaphores[CMSIS_CFG_NUM_SEMAPHORES];

static memory_pool_t timpool;
static struct os_timer_cb timers[CMSIS_CFG_NUM_TIMERS];

/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Virtual timers common callback.
 */
static void timer_cb(void const *arg) {

  osTimerId timer_id = (osTimerId)arg;
  timer_id->ptimer(timer_id->argument);
  if (timer_id->type == osTimerPeriodic) {
    chSysLockFromISR();
    chVTDoSetI(&timer_id->vt, MS2ST(timer_id->millisec),
               (vtfunc_t)timer_cb, timer_id);
    chSysUnlockFromISR();
  }
}

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Kernel initialization.
 */
osStatus osKernelInitialize(void) {

  cmsis_os_started = 0;

  chSysInit();
  chThdSetPriority(HIGHPRIO);

  chPoolObjectInit(&sempool, sizeof(semaphore_t), chCoreAlloc);
  chPoolLoadArray(&sempool, semaphores, CMSIS_CFG_NUM_SEMAPHORES);

  chPoolObjectInit(&timpool, sizeof(virtual_timer_t), chCoreAlloc);
  chPoolLoadArray(&timpool, timers, CMSIS_CFG_NUM_TIMERS);

  return osOK;
}

/**
 * @brief   Kernel start.
 */
osStatus osKernelStart(void) {

  cmsis_os_started = 1;

  chThdSetPriority(NORMALPRIO);

  return osOK;
}

/**
 * @brief   Creates a thread.
 */
osThreadId osThreadCreate(const osThreadDef_t *thread_def, void *argument) {
  size_t size;

  size = thread_def->stacksize == 0 ? CMSIS_CFG_DEFAULT_STACK :
                                      thread_def->stacksize;
  return (osThreadId)chThdCreateFromHeap(0,
                                         THD_WORKING_AREA_SIZE(size),
                                         NORMALPRIO+thread_def->tpriority,
                                         (tfunc_t)thread_def->pthread,
                                         argument);
}

/**
 * @brief   Thread termination.
 * @note    The thread is not really terminated but asked to terminate which
 *          is not compliant.
 */
osStatus osThreadTerminate(osThreadId thread_id) {

  if (thread_id == osThreadGetId()) {
    /* Note, no memory will be recovered unless a cleaner thread is
       implemented using the registry.*/
    chThdExit(0);
  }
  chThdTerminate(thread_id);
  chThdWait((thread_t *)thread_id);

  return osOK;
}

/**
 * @brief   Change thread priority.
 * @note    This can interfere with the priority inheritance mechanism.
 */
osStatus osThreadSetPriority(osThreadId thread_id, osPriority newprio) {
  osPriority oldprio;
  thread_t * tp = (thread_t *)thread_id;

  chSysLock();

  /* Changing priority.*/
#if CH_CFG_USE_MUTEXES
  oldprio = (osPriority)tp->p_realprio;
  if ((tp->p_prio == tp->p_realprio) || ((tprio_t)newprio > tp->p_prio))
    tp->p_prio = (tprio_t)newprio;
  tp->p_realprio = (tprio_t)newprio;
#else
  oldprio = tp->p_prio;
  tp->p_prio = (tprio_t)newprio;
#endif

  /* The following states need priority queues reordering.*/
  switch (tp->p_state) {
#if CH_CFG_USE_MUTEXES |                                                    \
    CH_CFG_USE_CONDVARS |                                                   \
    (CH_CFG_USE_SEMAPHORES && CH_CFG_USE_SEMAPHORES_PRIORITY) |             \
    (CH_CFG_USE_MESSAGES && CH_CFG_USE_MESSAGES_PRIORITY)
#if CH_CFG_USE_MUTEXES
  case CH_STATE_WTMTX:
#endif
#if CH_CFG_USE_CONDVARS
  case CH_STATE_WTCOND:
#endif
#if CH_CFG_USE_SEMAPHORES && CH_CFG_USE_SEMAPHORES_PRIORITY
  case CH_STATE_WTSEM:
#endif
#if CH_CFG_USE_MESSAGES && CH_CFG_USE_MESSAGES_PRIORITY
  case CH_STATE_SNDMSGQ:
#endif
    /* Re-enqueues tp with its new priority on the queue.*/
    queue_prio_insert(queue_dequeue(tp),
                      (threads_queue_t *)tp->p_u.wtobjp);
    break;
#endif
  case CH_STATE_READY:
#if CH_DBG_ENABLE_ASSERTS
    /* Prevents an assertion in chSchReadyI().*/
    tp->p_state = CH_STATE_CURRENT;
#endif
    /* Re-enqueues tp with its new priority on the ready list.*/
    chSchReadyI(queue_dequeue(tp));
    break;
  }

  /* Rescheduling.*/
  chSchRescheduleS();

  chSysUnlock();

  return oldprio;
}

/**
 * @brief   Create a timer.
 */
osTimerId osTimerCreate(const osTimerDef_t *timer_def,
                        os_timer_type type,
                        void *argument) {

  osTimerId timer = chPoolAlloc(&timpool);
  chVTObjectInit(&timer->vt);
  timer->ptimer = timer_def->ptimer;
  timer->type = type;
  timer->argument = argument;
  return timer;
}

/**
 * @brief   Start a timer.
 */
osStatus osTimerStart(osTimerId timer_id, uint32_t millisec) {

  if ((millisec == 0) || (millisec == osWaitForever))
    return osErrorValue;

  timer_id->millisec = millisec;
  chVTSet(&timer_id->vt, MS2ST(millisec), (vtfunc_t)timer_cb, timer_id);

  return osOK;
}

/**
 * @brief   Stop a timer.
 */
osStatus osTimerStop(osTimerId timer_id) {

  chVTReset(&timer_id->vt);

  return osOK;
}

/**
 * @brief   Delete a timer.
 */
osStatus osTimerDelete(osTimerId timer_id) {

  chVTReset(&timer_id->vt);
  chPoolFree(&timpool, (void *)timer_id);

  return osOK;
}

/**
 * @brief   Send signals.
 */
int32_t osSignalSet(osThreadId thread_id, int32_t signals) {
  int32_t oldsignals;

  syssts_t sts = chSysGetStatusAndLockX();
  oldsignals = (int32_t)thread_id->p_epending;
  chEvtSignalI((thread_t *)thread_id, (eventmask_t)signals);
  chSysRestoreStatusX(sts);

  return oldsignals;
}

/**
 * @brief   Clear signals.
 */
int32_t osSignalClear(osThreadId thread_id, int32_t signals) {
  eventmask_t m;

  chSysLock();

  m = thread_id->p_epending & (eventmask_t)signals;
  thread_id->p_epending &= ~(eventmask_t)signals;

  chSysUnlock();

  return (int32_t)m;
}

/**
 * @brief   Wait for signals.
 */
osEvent osSignalWait(int32_t signals, uint32_t millisec) {
  osEvent event;
  systime_t timeout = ((millisec == 0) || (millisec == osWaitForever)) ?
                      TIME_INFINITE : MS2ST(millisec);

  if (signals == 0)
    event.value.signals = (uint32_t)chEvtWaitAnyTimeout(ALL_EVENTS, timeout);
  else
    event.value.signals = (uint32_t)chEvtWaitAllTimeout((eventmask_t)signals,
                                                        timeout);

  /* Type of event.*/
  if (event.value.signals == 0)
    event.status = osEventTimeout;
  else
    event.status = osEventSignal;

  return event;
}

/**
 * @brief   Create a semaphore.
 * @note    @p semaphore_def is not used.
 * @note    Can involve memory allocation.
 */
osSemaphoreId osSemaphoreCreate(const osSemaphoreDef_t *semaphore_def,
                                int32_t count) {

  (void)semaphore_def;

  semaphore_t *sem = chPoolAlloc(&sempool);
  chSemObjectInit(sem, (cnt_t)count);
  return sem;
}

/**
 * @brief   Wait on a semaphore.
 */
int32_t osSemaphoreWait(osSemaphoreId semaphore_id, uint32_t millisec) {
  systime_t timeout = ((millisec == 0) || (millisec == osWaitForever)) ?
                      TIME_INFINITE : MS2ST(millisec);

  msg_t msg = chSemWaitTimeout((semaphore_t *)semaphore_id, timeout);
  switch (msg) {
  case MSG_OK:
    return osOK;
  case MSG_TIMEOUT:
    return osErrorTimeoutResource;
  }
  return osErrorResource;
}

/**
 * @brief   Release a semaphore.
 */
osStatus osSemaphoreRelease(osSemaphoreId semaphore_id) {

  syssts_t sts = chSysGetStatusAndLockX();
  chSemSignalI((semaphore_t *)semaphore_id);
  chSysRestoreStatusX(sts);

  return osOK;
}

/**
 * @brief   Deletes a semaphore.
 * @note    After deletion there could be references in the system to a
 *          non-existent semaphore.
 */
osStatus osSemaphoreDelete(osSemaphoreId semaphore_id) {

  chSemReset((semaphore_t *)semaphore_id, 0);
  chPoolFree(&sempool, (void *)semaphore_id);

  return osOK;
}

/**
 * @brief   Create a mutex.
 * @note    @p mutex_def is not used.
 * @note    Can involve memory allocation.
 */
osMutexId osMutexCreate(const osMutexDef_t *mutex_def) {

  (void)mutex_def;

  binary_semaphore_t *mtx = chPoolAlloc(&sempool);
  chBSemObjectInit(mtx, false);
  return mtx;
}

/**
 * @brief   Wait on a mutex.
 */
osStatus osMutexWait(osMutexId mutex_id, uint32_t millisec) {
  systime_t timeout = ((millisec == 0) || (millisec == osWaitForever)) ?
                      TIME_INFINITE : MS2ST(millisec);

  msg_t msg = chBSemWaitTimeout((binary_semaphore_t *)mutex_id, timeout);
  switch (msg) {
  case MSG_OK:
    return osOK;
  case MSG_TIMEOUT:
    return osErrorTimeoutResource;
  }
  return osErrorResource;
}

/**
 * @brief   Release a mutex.
 */
osStatus osMutexRelease(osMutexId mutex_id) {

  syssts_t sts = chSysGetStatusAndLockX();
  chBSemSignalI((binary_semaphore_t *)mutex_id);
  chSysRestoreStatusX(sts);

  return osOK;
}

/**
 * @brief   Deletes a mutex.
 * @note    After deletion there could be references in the system to a
 *          non-existent semaphore.
 */
osStatus osMutexDelete(osMutexId mutex_id) {

  chSemReset((semaphore_t *)mutex_id, 0);
  chPoolFree(&sempool, (void *)mutex_id);

  return osOK;
}

/**
 * @brief   Create a memory pool.
 * @note    The pool is not really created because it is allocated statically,
 *          this function just re-initializes it.
 */
osPoolId osPoolCreate(const osPoolDef_t *pool_def) {

  chPoolObjectInit(pool_def->pool, (size_t)pool_def->item_sz, NULL);
  chPoolLoadArray(pool_def->pool, pool_def->items, (size_t)pool_def->pool_sz);

  return (osPoolId)pool_def->pool;
}

/**
 * @brief   Allocate an object.
 */
void *osPoolAlloc(osPoolId pool_id) {
  void *object;

  syssts_t sts = chSysGetStatusAndLockX();
  object = chPoolAllocI((memory_pool_t *)pool_id);
  chSysRestoreStatusX(sts);

  return object;
}

/**
 * @brief   Allocate an object clearing it.
 */
void *osPoolCAlloc(osPoolId pool_id) {
  void *object;

  object = chPoolAllocI((memory_pool_t *)pool_id);
  memset(object, 0, pool_id->mp_object_size);
  return object;
}

/**
 * @brief   Free an object.
 */
osStatus osPoolFree(osPoolId pool_id, void *block) {

  syssts_t sts = chSysGetStatusAndLockX();
  chPoolFreeI((memory_pool_t *)pool_id, block);
  chSysRestoreStatusX(sts);

  return osOK;
}

/**
 * @brief   Create a message queue.
 * @note    The queue is not really created because it is allocated statically,
 *          this function just re-initializes it.
 */
osMessageQId osMessageCreate(const osMessageQDef_t *queue_def,
                             osThreadId thread_id) {

  /* Ignoring this parameter for now.*/
  (void)thread_id;

  if (queue_def->item_sz > sizeof (msg_t))
    return NULL;

  chMBObjectInit(queue_def->mailbox,
                 queue_def->items,
                 (size_t)queue_def->queue_sz);

  return (osMessageQId) queue_def->mailbox;
}

/**
 * @brief   Put a message in the queue.
 */
osStatus osMessagePut(osMessageQId queue_id,
                      uint32_t info,
                      uint32_t millisec) {
  msg_t msg;
  systime_t timeout = ((millisec == 0) || (millisec == osWaitForever)) ?
                      TIME_INFINITE : MS2ST(millisec);

  if (port_is_isr_context()) {

    /* Waiting makes no sense in ISRs so any value except "immediate"
       makes no sense.*/
    if (millisec != 0)
      return osErrorValue;

    chSysLockFromISR();
    msg = chMBPostI((mailbox_t *)queue_id, (msg_t)info);
    chSysUnlockFromISR();
  }
  else
    msg = chMBPost((mailbox_t *)queue_id, (msg_t)info, timeout);

  return msg == MSG_OK ? osOK : osEventTimeout;
}

/**
 * @brief   Get a message from the queue.
 */
osEvent osMessageGet(osMessageQId queue_id,
                     uint32_t millisec) {
  msg_t msg;
  osEvent event;
  systime_t timeout = ((millisec == 0) || (millisec == osWaitForever)) ?
                      TIME_INFINITE : MS2ST(millisec);

  event.def.message_id = queue_id;

  if (port_is_isr_context()) {

    /* Waiting makes no sense in ISRs so any value except "immediate"
       makes no sense.*/
    if (millisec != 0) {
      event.status = osErrorValue;
      return event;
    }

    chSysLockFromISR();
    msg = chMBFetchI((mailbox_t *)queue_id, (msg_t*)&event.value.v);
    chSysUnlockFromISR();
  }
  else {
    msg = chMBFetch((mailbox_t *)queue_id, (msg_t*)&event.value.v, timeout);
  }

  /* Returned event type.*/
  event.status = msg == MSG_OK ? osEventMessage : osEventTimeout;
  return event;
}

/** @} */
