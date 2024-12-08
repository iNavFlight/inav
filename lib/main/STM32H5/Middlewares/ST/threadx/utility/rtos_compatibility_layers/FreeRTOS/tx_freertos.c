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

/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** ThreadX Component                                                     */
/**                                                                       */
/**   FreeRTOS compatibility Kit                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*  10-15-2021     William E. Lamie         Modified comment(s), and      */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.1.7  */
/*  01-31-2022     William E. Lamie         Modified comment(s), and      */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.1.10 */
/*  07-29-2022     Cindy Deng               Added simple static scheduler */
/*                                            start flag, corrected stack */
/*                                            allocation size,            */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/

#include <stdint.h>
#include <limits.h>

#include <tx_api.h>
#include <tx_thread.h>
#include <tx_semaphore.h>
#include <tx_queue.h>

#include "FreeRTOS.h"

#if (INCLUDE_vTaskDelete == 1)
static TX_THREAD txfr_idle_task;
#ifdef TX_FREERTOS_IDLE_STACK
static UINT txfr_idle_stack[TX_FREERTOS_IDLE_STACK];
#else
static UINT txfr_idle_stack[configMINIMAL_STACK_SIZE];
#endif
static TX_SEMAPHORE txfr_idle_sem;

static txfr_task_t *p_delete_task_head;
#endif // #if (INCLUDE_vTaskDelete == 1)

UBaseType_t g_txfr_task_count;

#ifdef configTOTAL_HEAP_SIZE
static uint8_t txfr_heap_mem[configTOTAL_HEAP_SIZE];
static TX_BYTE_POOL txfr_heap;
#endif

static UINT txfr_heap_initialized;
#if (TX_FREERTOS_AUTO_INIT == 1)
static UINT txfr_initialized;
static UINT txfr_scheduler_started;
#endif // #if (TX_FREERTOS_AUTO_INIT == 1)

// TODO - do something with malloc.
void *txfr_malloc(size_t len)
{
    void *p;
    UINT ret;

    if(txfr_heap_initialized == 1u) {
        ret = tx_byte_allocate(&txfr_heap, &p, len, 0u);
        if(ret != TX_SUCCESS) {
            return NULL;
        }
    } else {
        return NULL;
    }

    return p;
}

void txfr_free(void *p)
{
    UINT ret;

    if(txfr_heap_initialized == 1u) {
        ret = tx_byte_release(p);
        if(ret != TX_SUCCESS) {
            TX_FREERTOS_ASSERT_FAIL();
        }
    }

    return;
}

#if (INCLUDE_vTaskDelete == 1)
static void txfr_idle_task_entry(ULONG id)
{
    txfr_task_t *p_task;
    UINT ret;
    TX_INTERRUPT_SAVE_AREA;


    for(;;) {
        ret = tx_semaphore_get(&txfr_idle_sem, TX_WAIT_FOREVER);
        if(ret != TX_SUCCESS) {
            TX_FREERTOS_ASSERT_FAIL();
        }

        TX_DISABLE;
        p_task = p_delete_task_head;

        if(p_task != NULL) {
            p_delete_task_head = p_task->p_next;
        }
        g_txfr_task_count--;
        TX_RESTORE;

        if(p_task != NULL) {

            // Make sure the task is terminated, which may return an error if that's already the case so the return value is ignored.
            (void)tx_thread_terminate(&p_task->thread);

            ret = tx_thread_delete(&p_task->thread);
            if(ret != TX_SUCCESS) {
                TX_FREERTOS_ASSERT_FAIL();
            }

            ret = tx_semaphore_delete(&p_task->notification_sem);
            if(ret != TX_SUCCESS) {
                TX_FREERTOS_ASSERT_FAIL();
            }

            if(p_task->allocated == 1u) {
                txfr_free(p_task);
            }
        }

    }
}
#endif // #if (INCLUDE_vTaskDelete == 1)

static uint32_t txfr_prio_fr_to_tx(uint32_t prio)
{
    return TX_MAX_PRIORITIES - 1u - prio;
}


static uint32_t txfr_prio_tx_to_fr(uint32_t prio)
{
    return TX_MAX_PRIORITIES - 1u - prio;
}

#if (TX_FREERTOS_AUTO_INIT == 1)
static void tx_freertos_auto_init(void)
{
    UINT ret;

    tx_kernel_enter();

    ret = tx_freertos_init();
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return;
    }

    txfr_initialized = 1u;
}
#endif // #if (TX_FREERTOS_AUTO_INIT == 1)

#if (TX_FREERTOS_AUTO_INIT == 1)
VOID tx_application_define(VOID * first_unused_memory)
{
    // Empty tx_application_define() to support auto initialization.
}
#endif // #if (TX_FREERTOS_AUTO_INIT == 1)

UINT tx_freertos_init(void)
{
    UINT ret;

#ifdef configTOTAL_HEAP_SIZE
    if(configTOTAL_HEAP_SIZE > 0u) {
        ret = tx_byte_pool_create(&txfr_heap, "txfr_byte_pool", txfr_heap_mem, configTOTAL_HEAP_SIZE);
        if(ret != TX_SUCCESS) {
            return ret;
        }
        txfr_heap_initialized = 1u;
    }
#endif

#if (INCLUDE_vTaskDelete == 1)
    ret = tx_semaphore_create(&txfr_idle_sem, "txfr_idle_semaphore", 0u);
    if(ret != TX_SUCCESS) {
        return ret;
    }

    ret = tx_thread_create(&txfr_idle_task, "txfr_idle_task", txfr_idle_task_entry, 0u,
            txfr_idle_stack, sizeof(txfr_idle_stack), TX_MAX_PRIORITIES - 1u, TX_MAX_PRIORITIES - 1u, 0u, TX_AUTO_START);
    if(ret != TX_SUCCESS) {
        return ret;
    }
#endif // #if (INCLUDE_vTaskDelete == 1)

    return TX_SUCCESS;
}

void txfr_thread_wrapper(ULONG id)
{
    TX_THREAD *p_thread;
    txfr_task_t *p_txfr_task;

    p_thread = tx_thread_identify();

    p_txfr_task = p_thread->txfr_thread_ptr;

    p_txfr_task->p_task_func(p_txfr_task->p_task_arg);

#if (INCLUDE_vTaskDelete == 1)
    vTaskDelete(NULL);
#else
    // Returning from a task is not allowed when vTaskDelete is disabled.
    TX_FREERTOS_ASSERT_FAIL();
#endif // #if (INCLUDE_vTaskDelete == 1)
}


void *pvPortMalloc(size_t xWantedSize)
{
    return txfr_malloc(xWantedSize);
}

void vPortFree(void *pv)
{
    txfr_free(pv);

    return;
}

void vPortEnterCritical(void)
{
    portDISABLE_INTERRUPTS();
    _tx_thread_preempt_disable++;
}

void vPortExitCritical(void)
{
    if(_tx_thread_preempt_disable == 0u) {
        TX_FREERTOS_ASSERT_FAIL();
    }

    _tx_thread_preempt_disable--;

    if(_tx_thread_preempt_disable == 0u) {
        portENABLE_INTERRUPTS();
    }
}

void vTaskStartScheduler(void)
{
#if (TX_FREERTOS_AUTO_INIT == 1)
    txfr_scheduler_started = 1u;
    _tx_thread_schedule();
#else
    // Nothing to do, THREADX scheduler is already started.
#endif
}


BaseType_t xTaskGetSchedulerState(void)
{
#if (TX_FREERTOS_AUTO_INIT == 1)
    if(txfr_scheduler_started == 0u) {
        return taskSCHEDULER_NOT_STARTED;
    }
#endif
    if(_tx_thread_preempt_disable > 0u) {
        return taskSCHEDULER_SUSPENDED;
    } else {
        return taskSCHEDULER_RUNNING;
    }
}


void vTaskSuspendAll(void)
{
    TX_INTERRUPT_SAVE_AREA;

    TX_DISABLE;
    _tx_thread_preempt_disable++;
    TX_RESTORE;
}

BaseType_t xTaskResumeAll(void)
{
    TX_INTERRUPT_SAVE_AREA;

    TX_DISABLE;
    _tx_thread_preempt_disable--;
    TX_RESTORE;

    return pdFALSE;
}

void vTaskDelay(const TickType_t xTicksToDelay)
{
    tx_thread_sleep(xTicksToDelay);
}

TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,
                               const char *const pcName,
                               const configSTACK_DEPTH_TYPE ulStackDepth,
                               void *const pvParameters,
                               UBaseType_t uxPriority,
                               StackType_t *const puxStackBuffer,
                               StaticTask_t *const pxTaskBuffer)
{
    UINT prio;
    UINT ret;
    ULONG stack_depth_bytes;
    TX_INTERRUPT_SAVE_AREA;

    configASSERT(pxTaskCode != NULL);
    configASSERT(ulStackDepth >= configMINIMAL_STACK_SIZE);
    configASSERT(uxPriority < configMAX_PRIORITIES);
    configASSERT(puxStackBuffer != NULL);
    configASSERT(pxTaskBuffer != NULL);

#if (TX_FREERTOS_AUTO_INIT == 1)
    if(txfr_initialized != 1u) {
        tx_freertos_auto_init();
    }
#endif

    if(ulStackDepth > (ULONG_MAX / sizeof(StackType_t))) {
        /* Integer overflow in stack depth */
        TX_FREERTOS_ASSERT_FAIL();
        return NULL;
    }
    stack_depth_bytes = ulStackDepth * sizeof(StackType_t);

    TX_MEMSET(pxTaskBuffer, 0, sizeof(*pxTaskBuffer));
    pxTaskBuffer->p_task_arg = pvParameters;
    pxTaskBuffer->p_task_func = pxTaskCode;

    ret = tx_semaphore_create(&pxTaskBuffer->notification_sem, "", 0u);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return NULL;
    }

    prio = txfr_prio_fr_to_tx(uxPriority);

    ret = tx_thread_create(&pxTaskBuffer->thread, (CHAR *)pcName, txfr_thread_wrapper, (ULONG)pvParameters,
            puxStackBuffer, stack_depth_bytes, prio, prio, 0u, TX_DONT_START);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return NULL;
    }

    pxTaskBuffer->thread.txfr_thread_ptr = pxTaskBuffer;

    ret = tx_thread_resume(&pxTaskBuffer->thread);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return NULL;
    }

    TX_DISABLE;
    g_txfr_task_count++;
    TX_RESTORE;

    return pxTaskBuffer;
}


BaseType_t xTaskCreate(TaskFunction_t pvTaskCode,
                       const char * const pcName,
                       const configSTACK_DEPTH_TYPE usStackDepth,
                       void *pvParameters,
                       UBaseType_t uxPriority,
                       TaskHandle_t * const pxCreatedTask)
{
    void *p_stack;
    txfr_task_t *p_task;
    UINT ret;
    UINT prio;
    ULONG stack_depth_bytes;
    TX_INTERRUPT_SAVE_AREA;

    configASSERT(pvTaskCode != NULL);
    configASSERT(usStackDepth >= configMINIMAL_STACK_SIZE);
    configASSERT(uxPriority < configMAX_PRIORITIES);
    configASSERT(pxCreatedTask != NULL);

#if (TX_FREERTOS_AUTO_INIT == 1)
    if(txfr_initialized != 1u) {
        tx_freertos_auto_init();
    }
#endif
    if((usStackDepth > (SIZE_MAX / sizeof(StackType_t)))
        || (usStackDepth > (ULONG_MAX / sizeof(StackType_t)))) {
        /* Integer overflow in stack depth */
        return errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
    }
    stack_depth_bytes = usStackDepth * sizeof(StackType_t);

    p_stack = txfr_malloc((size_t)stack_depth_bytes);
    if(p_stack == NULL) {
        return errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
    }

    p_task = txfr_malloc(sizeof(txfr_task_t));
    if(p_task == NULL) {
        txfr_free(p_stack);
        return errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
    }

    *pxCreatedTask = p_task;

    TX_MEMSET(p_task, 0, sizeof(*p_task));
    p_task->allocated = 1u;
    p_task->p_task_arg = pvParameters;
    p_task->p_task_func = pvTaskCode;

    ret = tx_semaphore_create(&p_task->notification_sem, "", 0u);
    if(ret != TX_SUCCESS) {
        txfr_free(p_stack);
        txfr_free(p_task);
        TX_FREERTOS_ASSERT_FAIL();
        return (BaseType_t)NULL;
    }

    prio = txfr_prio_fr_to_tx(uxPriority);

    ret = tx_thread_create(&p_task->thread, (CHAR *)pcName, txfr_thread_wrapper, (ULONG)pvParameters,
            p_stack, stack_depth_bytes, prio, prio, 0u, TX_DONT_START);
    if(ret != TX_SUCCESS) {
        (void)tx_semaphore_delete(&p_task->notification_sem);
        txfr_free(p_stack);
        txfr_free(p_task);
        TX_FREERTOS_ASSERT_FAIL();
        return (BaseType_t)NULL;
    }

    p_task->thread.txfr_thread_ptr = p_task;

    ret = tx_thread_resume(&p_task->thread);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
    }

    TX_DISABLE;
    g_txfr_task_count++;
    TX_RESTORE;

    return pdPASS;
}


UBaseType_t uxTaskGetNumberOfTasks(void)
{
    UBaseType_t count;
    TX_INTERRUPT_SAVE_AREA;

    TX_DISABLE;
    count = g_txfr_task_count;
    TX_RESTORE;

    return count;
}

#if (INCLUDE_vTaskDelete == 1)
void vTaskDelete(TaskHandle_t xTask)
{
    UINT ret;
    TX_THREAD *p_thread;
    txfr_task_t *p_txfr_thread;
    TX_INTERRUPT_SAVE_AREA;

    if(xTask == NULL) {
        TX_THREAD_GET_CURRENT(p_thread);
        p_txfr_thread = (txfr_task_t *)p_thread->txfr_thread_ptr;
    } else {
        p_txfr_thread = xTask;
        p_thread = &xTask->thread;
    }

    TX_DISABLE;

    p_txfr_thread->p_next = p_delete_task_head;
    p_delete_task_head = p_txfr_thread;

    _tx_thread_preempt_disable++;

    ret = tx_semaphore_put(&txfr_idle_sem);
    if(ret != TX_SUCCESS) {
        _tx_thread_preempt_disable--;
        TX_RESTORE;
        TX_FREERTOS_ASSERT_FAIL();
        return;
    }

    _tx_thread_preempt_disable--;

    TX_RESTORE;

    ret = tx_thread_terminate(p_thread);
    if(ret != TX_SUCCESS) {
        TX_RESTORE;
        TX_FREERTOS_ASSERT_FAIL();
        return;
    }

    return;
}
#endif


TaskHandle_t xTaskGetCurrentTaskHandle(void)
{
    TX_THREAD *p_thread;

    p_thread = tx_thread_identify();

    return p_thread->txfr_thread_ptr;
}


void vTaskSuspend(TaskHandle_t xTaskToSuspend)
{
    TX_THREAD *p_thread;
    UINT ret;

    if(xTaskToSuspend == NULL) {
        p_thread = tx_thread_identify();
    } else {
        p_thread = &xTaskToSuspend->thread;
    }

    ret = tx_thread_suspend(p_thread);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return;
    }
}


void vTaskResume(TaskHandle_t xTaskToResume)
{
    UINT ret;

    configASSERT(xTaskToResume != NULL);

    ret = tx_thread_resume(&xTaskToResume->thread);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return;
    }
}


BaseType_t xTaskResumeFromISR(TaskHandle_t xTaskToResume)
{
    configASSERT(xTaskToResume != NULL);

    vTaskResume(xTaskToResume);

    return pdFALSE;
}


BaseType_t xTaskAbortDelay(TaskHandle_t xTask)
{
    TX_THREAD *p_thread;
    UINT ret;

    configASSERT(xTask != NULL);

    p_thread = &xTask->thread;

    ret = tx_thread_wait_abort(p_thread);
    if(ret != TX_SUCCESS) {
        return pdFAIL;
    }

    return pdPASS;
}


UBaseType_t uxTaskPriorityGet(const TaskHandle_t xTask)
{
    TX_THREAD *p_thread;
    UINT priority;
    UINT ret;

    if(xTask == NULL) {
        p_thread = tx_thread_identify();
    } else {
        p_thread = &xTask->thread;
    }

    ret = tx_thread_info_get(p_thread, NULL, NULL, NULL, &priority, NULL, NULL, NULL, NULL);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return 0;
    }

    priority = txfr_prio_tx_to_fr(priority);

    return priority;
}


UBaseType_t uxTaskPriorityGetFromISR(const TaskHandle_t xTask)
{
    return uxTaskPriorityGet(xTask);
}


void vTaskPrioritySet(TaskHandle_t xTask,
                      UBaseType_t uxNewPriority)
{
    TX_THREAD *p_thread;
    UINT priority;
    UINT old_priority;
    UINT ret;

    configASSERT(uxNewPriority < configMAX_PRIORITIES);

    if(xTask == NULL) {
        p_thread = tx_thread_identify();
    } else {
        p_thread = &xTask->thread;
    }

    priority = uxNewPriority;
    priority = txfr_prio_fr_to_tx(priority);

    ret = tx_thread_priority_change(p_thread, priority, &old_priority);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return;
    }

}


char *pcTaskGetName(TaskHandle_t xTaskToQuery)
{
    TX_THREAD *p_thread;
    char *p_task_name;
    UINT ret;

    if(xTaskToQuery == NULL) {
        p_thread = tx_thread_identify();
    } else {
        p_thread = &xTaskToQuery->thread;
    }

    ret = tx_thread_info_get(p_thread, &p_task_name, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return 0;
    }

    return p_task_name;
}


eTaskState eTaskGetState(TaskHandle_t xTask)
{
    UINT thread_state;
    eTaskState ret_state;
    TX_THREAD *p_thread;

    TX_INTERRUPT_SAVE_AREA;

    if(xTask == NULL) {
        return eInvalid;
    }

    TX_DISABLE;
    thread_state = xTask->thread.tx_thread_state;
    p_thread = &xTask->thread;
    TX_RESTORE;

    if(p_thread == tx_thread_identify()) {
        return eRunning;
    }

    switch(thread_state) {
        case TX_READY:
            ret_state = eReady;
            break;

        case TX_COMPLETED:
        case TX_TERMINATED:
            ret_state = eDeleted;
            break;

        case TX_SUSPENDED:
        case TX_SLEEP:
            ret_state = eSuspended;
            break;

        case TX_QUEUE_SUSP:
        case TX_SEMAPHORE_SUSP:
        case TX_EVENT_FLAG:
        case TX_BLOCK_MEMORY:
        case TX_BYTE_MEMORY:
        case TX_IO_DRIVER:
        case TX_FILE:
        case TX_TCP_IP:
        case TX_MUTEX_SUSP:
            ret_state = eBlocked;
            break;

        default:
            ret_state = eInvalid;
            break;
    }

    return ret_state;
}


void vTaskDelayUntil(TickType_t * const pxPreviousWakeTime, const TickType_t xTimeIncrement)
{
    TickType_t tick_cur;

    tick_cur = (uint16_t)tx_time_get();

    tx_thread_sleep(xTimeIncrement - (tick_cur - *pxPreviousWakeTime));

    *pxPreviousWakeTime = *pxPreviousWakeTime + xTimeIncrement;
}


BaseType_t xTaskNotifyGive(TaskHandle_t xTaskToNotify)
{
    configASSERT(xTaskToNotify != NULL);

    return xTaskNotify(xTaskToNotify, 0u, eIncrement);
}


void vTaskNotifyGiveFromISR(TaskHandle_t xTaskToNotify,
                            BaseType_t *pxHigherPriorityTaskWoken)
{
    configASSERT(xTaskToNotify != NULL);

    (void)xTaskNotify(xTaskToNotify, 0u, eIncrement);
}


uint32_t ulTaskNotifyTake(BaseType_t xClearCountOnExit,
                          TickType_t xTicksToWait)
{
    TX_THREAD *p_thread;
    uint32_t val;
    UINT ret;
    UCHAR pend;
    txfr_task_t *p_task;
    UINT timeout;

    if(xTicksToWait ==  portMAX_DELAY) {
        timeout = TX_WAIT_FOREVER;
    } else {
        timeout = (UINT)xTicksToWait;
    }

    pend = TX_FALSE;
    p_thread = tx_thread_identify();
    p_task = p_thread->txfr_thread_ptr;

    TX_INTERRUPT_SAVE_AREA;

    TX_DISABLE;

    ret = tx_semaphore_get(&p_task->notification_sem, 0u);
    if(ret == TX_SUCCESS) {
        val = p_task->task_notify_val;
        p_task->p_notify_val_ret = NULL;
        if(xClearCountOnExit != pdFALSE) {
            p_task->task_notify_val = 0u;
        } else {
            p_task->task_notify_val--;
        }
    } else {
        pend = TX_TRUE;
        p_task->p_notify_val_ret = &val;
        p_task->clear_on_pend = xClearCountOnExit;
        p_task->clear_mask = (uint32_t)-1;
    }

    TX_RESTORE;

    if(pend == TX_TRUE) {
        ret = tx_semaphore_get(&p_task->notification_sem, timeout);
        p_task->p_notify_val_ret = NULL;
        if(ret != TX_SUCCESS) {
            return 0u;
        }
    }

    return val;
}

BaseType_t xTaskNotifyWait(uint32_t ulBitsToClearOnEntry,
                           uint32_t ulBitsToClearOnExit,
                           uint32_t *pulNotificationValue,
                           TickType_t xTicksToWait)
{
    TX_INTERRUPT_SAVE_AREA;
    TX_THREAD *p_thread;
    uint32_t val;
    BaseType_t ret_val;
    UINT ret;
    UCHAR pend;
    txfr_task_t *p_task;
    UINT timeout;

    ret_val = pdPASS;

    if(xTicksToWait ==  portMAX_DELAY) {
        timeout = TX_WAIT_FOREVER;
    } else {
        timeout = (UINT)xTicksToWait;
    }

    pend = TX_FALSE;
    p_thread = tx_thread_identify();
    p_task = p_thread->txfr_thread_ptr;

    TX_DISABLE;

    ret = tx_semaphore_get(&p_task->notification_sem, 0u);
    if(ret == TX_SUCCESS) {
        val = p_task->task_notify_val;
        p_task->p_notify_val_ret = NULL;
        if(ulBitsToClearOnExit != 0u) {
            p_task->task_notify_val &= ~ulBitsToClearOnExit;
        }
    } else {
        pend = TX_TRUE;
        p_task->p_notify_val_ret = &val;
        p_task->clear_on_pend = 1u;
        p_task->clear_mask = ulBitsToClearOnExit;
    }

    TX_RESTORE;

    if(pend == TX_TRUE) {
        ret = tx_semaphore_get(&p_task->notification_sem, timeout);
        p_task->p_notify_val_ret = NULL;
        if(ret != TX_SUCCESS) {
            return 0u;
        }
    }

    *pulNotificationValue = val;

    return ret_val;
}


BaseType_t xTaskNotify(TaskHandle_t xTaskToNotify,
                       uint32_t ulValue,
                       eNotifyAction eAction)
{

    configASSERT(TXFR_NOTIFYACTION_VALID(eAction));

    return xTaskNotifyAndQuery(xTaskToNotify, ulValue, eAction, NULL);
}

BaseType_t xTaskNotifyFromISR(TaskHandle_t xTaskToNotify,
                              uint32_t ulValue,
                              eNotifyAction eAction,
                              BaseType_t *pxHigherPriorityTaskWoken)
{

    configASSERT(xTaskToNotify != NULL);
    configASSERT(TXFR_NOTIFYACTION_VALID(eAction));

    return xTaskNotify(xTaskToNotify, ulValue, eAction);
}

BaseType_t xTaskNotifyAndQuery(TaskHandle_t xTaskToNotify,
                               uint32_t ulValue,
                               eNotifyAction eAction,
                               uint32_t *pulPreviousNotifyValue)
{
    UINT ret;
    UCHAR notified;
    TX_INTERRUPT_SAVE_AREA;
    BaseType_t ret_val;
    UCHAR waiting;

    configASSERT(xTaskToNotify != NULL);
    configASSERT(TXFR_NOTIFYACTION_VALID(eAction));

    TX_DISABLE;

    if(pulPreviousNotifyValue != NULL) {
        *pulPreviousNotifyValue = xTaskToNotify->task_notify_val;
    }

    waiting = TX_FALSE;
    notified = TX_FALSE;
    ret_val = pdPASS;

    if(xTaskToNotify->notification_sem.tx_semaphore_suspended_count != 0u) {
        waiting = TX_TRUE;
    }

    if(xTaskToNotify->notification_sem.tx_semaphore_count == 0u) {
        _tx_thread_preempt_disable++;

        ret = tx_semaphore_put(&xTaskToNotify->notification_sem);

        _tx_thread_preempt_disable--;

        if(ret != TX_SUCCESS) {
            TX_RESTORE;
            TX_FREERTOS_ASSERT_FAIL();
            return pdFAIL;
        }
        xTaskToNotify->task_notify_val_pend = xTaskToNotify->task_notify_val;

        notified = TX_TRUE;
    }

    switch (eAction) {
        case eNoAction:
            break;

        case eSetBits:
            xTaskToNotify->task_notify_val |= ulValue;
            break;

        case eIncrement:
            xTaskToNotify->task_notify_val++;
            break;

        case eSetValueWithOverwrite:
            xTaskToNotify->task_notify_val = ulValue;
            break;

        case eSetValueWithoutOverwrite:
            if(notified == TX_TRUE) {
                xTaskToNotify->task_notify_val = ulValue;
            } else {
                ret_val = pdFALSE;
            }
            break;

        default:
            TX_RESTORE;
            return pdFAIL;
            break;
    }

    if(waiting == TX_TRUE) {
        *xTaskToNotify->p_notify_val_ret = xTaskToNotify->task_notify_val;

        if(xTaskToNotify->clear_on_pend == TX_TRUE) {
            xTaskToNotify->task_notify_val &= ~xTaskToNotify->clear_mask;
        } else {
            xTaskToNotify->task_notify_val--;
        }
    }

    TX_RESTORE;

    _tx_thread_system_preempt_check();

    return ret_val;
}


BaseType_t xTaskNotifyAndQueryFromISR(TaskHandle_t xTaskToNotify,
                                      uint32_t ulValue,
                                      eNotifyAction eAction,
                                      uint32_t *pulPreviousNotifyValue,
                                      BaseType_t *pxHigherPriorityTaskWoken)
{
    configASSERT(xTaskToNotify != NULL);
    configASSERT(TXFR_NOTIFYACTION_VALID(eAction));


    return xTaskNotifyAndQuery(xTaskToNotify, ulValue, eAction, pulPreviousNotifyValue);
}


BaseType_t xTaskNotifyStateClear(TaskHandle_t xTask)
{
    BaseType_t ret_val;
    UINT ret;
    TX_THREAD *p_thread;
    txfr_task_t *p_task;
    TX_INTERRUPT_SAVE_AREA;

    if(xTask == NULL) {
        p_thread = tx_thread_identify();
        p_task = p_thread->txfr_thread_ptr;
    } else {
        p_thread = &xTask->thread;
        p_task = xTask;
    }

    TX_DISABLE;

    if(p_task->notification_sem.tx_semaphore_suspended_count != 0u) {
        ret_val = pdTRUE;
    } else {
        ret_val = pdFALSE;
    }

    ret = tx_semaphore_get(&p_task->notification_sem, 0u);
    if(ret != TX_SUCCESS) {
        TX_RESTORE;
        TX_FREERTOS_ASSERT_FAIL();
        return pdFALSE;
    }

    TX_RESTORE;

    return ret_val;
}


uint32_t ulTaskNotifyValueClear(TaskHandle_t xTask,
                                uint32_t ulBitsToClear)
{
    BaseType_t ret_val;
    TX_THREAD *p_thread;
    txfr_task_t *p_task;
    TX_INTERRUPT_SAVE_AREA;

    if(xTask == NULL) {
        p_thread = tx_thread_identify();
        p_task = p_thread->txfr_thread_ptr;
    } else {
        p_thread = &xTask->thread;
        p_task = xTask;
    }

    TX_DISABLE;

    ret_val = p_task->task_notify_val;

    p_task->task_notify_val &= ~ulBitsToClear;

    TX_RESTORE;

    return ret_val;
}


SemaphoreHandle_t xSemaphoreCreateCounting(UBaseType_t uxMaxCount,
                                           UBaseType_t uxInitialCount)
{
    txfr_sem_t *p_sem;
    UINT ret;

    configASSERT(uxMaxCount != 0u);
    configASSERT(uxInitialCount <= uxMaxCount);

#if (TX_FREERTOS_AUTO_INIT == 1)
    if(txfr_initialized != 1u) {
        tx_freertos_auto_init();
    }
#endif

    p_sem = txfr_malloc(sizeof(txfr_sem_t));
    if(p_sem == NULL) {
        return NULL;
    }

    TX_MEMSET(p_sem, 0, sizeof(*p_sem));
    p_sem->max_count = uxMaxCount;
    p_sem->allocated = 1u;
    p_sem->is_mutex = 0u;

    ret = tx_semaphore_create(&p_sem->sem, "", uxInitialCount);
    if(ret != TX_SUCCESS) {
        txfr_free(p_sem);
        return NULL;
    }

    return p_sem;
}

SemaphoreHandle_t xSemaphoreCreateCountingStatic(UBaseType_t uxMaxCount,
                                                 UBaseType_t uxInitialCount,
                                                 StaticSemaphore_t *pxSemaphoreBuffer)
{
    UINT ret;

    configASSERT(uxMaxCount != 0u);
    configASSERT(uxInitialCount <= uxMaxCount);
    configASSERT(pxSemaphoreBuffer != NULL);

#if (TX_FREERTOS_AUTO_INIT == 1)
    if(txfr_initialized != 1u) {
        tx_freertos_auto_init();
    }
#endif

    TX_MEMSET(pxSemaphoreBuffer, 0, sizeof(*pxSemaphoreBuffer));
    pxSemaphoreBuffer->max_count = uxMaxCount;
    pxSemaphoreBuffer->allocated = 0u;
    pxSemaphoreBuffer->is_mutex = 0u;

    ret = tx_semaphore_create(&pxSemaphoreBuffer->sem, "", uxInitialCount);
    if(ret != TX_SUCCESS) {
        return NULL;
    }

    return pxSemaphoreBuffer;
}


SemaphoreHandle_t xSemaphoreCreateBinary(void)
{
    return xSemaphoreCreateCounting(1u, 0u);
}


SemaphoreHandle_t xSemaphoreCreateBinaryStatic(StaticSemaphore_t *pxSemaphoreBuffer)
{
    configASSERT(pxSemaphoreBuffer != NULL);

    return xSemaphoreCreateCountingStatic(1u, 0u, pxSemaphoreBuffer);
}


SemaphoreHandle_t xSemaphoreCreateMutex(void)
{
    txfr_sem_t *p_sem;
    UINT ret;

#if (TX_FREERTOS_AUTO_INIT == 1)
    if(txfr_initialized != 1u) {
        tx_freertos_auto_init();
    }
#endif

    p_sem = txfr_malloc(sizeof(txfr_sem_t));
    if(p_sem == NULL) {
        return NULL;
    }

    TX_MEMSET(p_sem, 0, sizeof(*p_sem));
    p_sem->max_count = 1u;
    p_sem->allocated = 1u;
    p_sem->is_mutex = 1u;

    ret = tx_mutex_create(&p_sem->mutex, "", TX_NO_INHERIT);
    if(ret != TX_SUCCESS) {
        txfr_free(p_sem);
        return NULL;
    }

    return p_sem;
}


SemaphoreHandle_t xSemaphoreCreateMutexStatic(StaticSemaphore_t *pxMutexBuffer)
{
    UINT ret;

    configASSERT(pxMutexBuffer != NULL);

    TX_MEMSET(pxMutexBuffer, 0, sizeof(*pxMutexBuffer));
    pxMutexBuffer->max_count = 1u;
    pxMutexBuffer->allocated = 0u;
    pxMutexBuffer->is_mutex = 1u;

    ret = tx_mutex_create(&pxMutexBuffer->mutex, "", TX_NO_INHERIT);
    if(ret != TX_SUCCESS) {
        return NULL;
    }

    return pxMutexBuffer;
}


SemaphoreHandle_t xSemaphoreCreateRecursiveMutex(void)
{
    txfr_sem_t *p_sem;
    UINT ret;

#if (TX_FREERTOS_AUTO_INIT == 1)
    if(txfr_initialized != 1u) {
        tx_freertos_auto_init();
    }
#endif

    p_sem = txfr_malloc(sizeof(txfr_sem_t));
    if(p_sem == NULL) {
        return NULL;
    }

    TX_MEMSET(p_sem, 0, sizeof(*p_sem));
    p_sem->max_count = 1u;
    p_sem->allocated = 1u;
    p_sem->is_mutex = 1u;

    ret = tx_mutex_create(&p_sem->mutex, "", TX_INHERIT);
    if(ret != TX_SUCCESS) {
        txfr_free(p_sem);
        return NULL;
    }

    return p_sem;
}


SemaphoreHandle_t xSemaphoreCreateRecursiveMutexStatic(StaticSemaphore_t *pxMutexBuffer)
{
    UINT ret;

    configASSERT(pxMutexBuffer != NULL);

#if (TX_FREERTOS_AUTO_INIT == 1)
    if(txfr_initialized != 1u) {
        tx_freertos_auto_init();
    }
#endif

    TX_MEMSET(pxMutexBuffer, 0, sizeof(*pxMutexBuffer));
    pxMutexBuffer->max_count = 1u;
    pxMutexBuffer->allocated = 0u;
    pxMutexBuffer->is_mutex = 1u;

    ret = tx_mutex_create(&pxMutexBuffer->mutex, "", TX_INHERIT);
    if(ret != TX_SUCCESS) {
        return NULL;
    }

    return pxMutexBuffer;
}

void vSemaphoreDelete(SemaphoreHandle_t xSemaphore)
{
    UINT ret;

    configASSERT(xSemaphore != NULL);

    if(xSemaphore->is_mutex == 0u) {
        ret = tx_semaphore_delete(&xSemaphore->sem);
    } else {
        ret = tx_mutex_delete(&xSemaphore->mutex);
    }

    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return;
    }

    if(xSemaphore->allocated == 1u) {
        vPortFree(xSemaphore);
    }
}


BaseType_t xSemaphoreTake(SemaphoreHandle_t xSemaphore, TickType_t xTicksToWait)
{
    UINT timeout;
    UINT ret;

    configASSERT(xSemaphore != NULL);

    if(xTicksToWait ==  portMAX_DELAY) {
        timeout = TX_WAIT_FOREVER;
    } else {
        timeout = (UINT)xTicksToWait;
    }

    if(xSemaphore->is_mutex == 1u) {
        if(xSemaphore->mutex.tx_mutex_owner == tx_thread_identify()) {
            return pdFALSE;
        }
        ret = tx_mutex_get(&xSemaphore->mutex, timeout);
        if(ret != TX_SUCCESS) {
            return pdFALSE;
        }
    } else {
        ret = tx_semaphore_get(&xSemaphore->sem, timeout);
        if(ret != TX_SUCCESS) {
            return pdFALSE;
        }
    }

    return pdTRUE;
}


BaseType_t xSemaphoreTakeFromISR(SemaphoreHandle_t xSemaphore, BaseType_t *pxHigherPriorityTaskWoken)
{
    UINT ret;

    configASSERT(xSemaphore != NULL);

    if(xSemaphore->is_mutex == 1u) {
        return pdFALSE;
    } else {
        ret = tx_semaphore_get(&xSemaphore->sem, 0u);
        if(ret != TX_SUCCESS) {
            return pdFALSE;
        }
    }

    return pdTRUE;
}


BaseType_t xSemaphoreTakeRecursive(SemaphoreHandle_t xMutex, TickType_t xTicksToWait)
{
    UINT timeout;
    UINT ret;

    configASSERT(xMutex != NULL);

    if(xTicksToWait ==  portMAX_DELAY) {
        timeout = TX_WAIT_FOREVER;
    } else {
        timeout = (UINT)xTicksToWait;
    }

    if(xMutex->is_mutex == 1u) {
        ret = tx_mutex_get(&xMutex->mutex, timeout);
        if(ret != TX_SUCCESS) {
            return pdFALSE;
        }
    } else {
        ret = tx_semaphore_get(&xMutex->sem, timeout);
        if(ret != TX_SUCCESS) {
            return pdFALSE;
        }
    }

    return pdTRUE;
}


BaseType_t xSemaphoreGive(SemaphoreHandle_t xSemaphore)
{
    TX_INTERRUPT_SAVE_AREA
    UINT ret;

    configASSERT(xSemaphore != NULL);

    if(xSemaphore->is_mutex == 1u) {
        ret = tx_mutex_put(&xSemaphore->mutex);
        if(ret != TX_SUCCESS) {
            return pdFALSE;
        }

      return pdTRUE;
    }

    TX_DISABLE;
    _tx_thread_preempt_disable++;

    if(xSemaphore->sem.tx_semaphore_count >= xSemaphore->max_count) {
        /* Maximum semaphore count reached return failure. */
        _tx_thread_preempt_disable--;
         TX_RESTORE
        return pdFALSE;
    }

    ret = tx_semaphore_put(&xSemaphore->sem);
    if(ret != TX_SUCCESS) {
        _tx_thread_preempt_disable--;
        TX_RESTORE;
        return pdFALSE;
    }

    if(xSemaphore->p_set != NULL) {
        // To prevent deadlocks don't wait when posting on a queue set.
        ret = tx_queue_send(&xSemaphore->p_set->queue, &xSemaphore, TX_NO_WAIT);
        if((ret != TX_SUCCESS) && (ret != TX_QUEUE_FULL)) {
            // Fatal error, queue full errors are ignored on purpose to match the original behaviour.
            _tx_thread_preempt_disable--;
            TX_RESTORE;
            TX_FREERTOS_ASSERT_FAIL();
            return pdFALSE;
        }
    }

    _tx_thread_preempt_disable--;
    TX_RESTORE;

    _tx_thread_system_preempt_check();

    return pdTRUE;
}

BaseType_t xSemaphoreGiveFromISR(SemaphoreHandle_t xSemaphore, BaseType_t *pxHigherPriorityTaskWoken)
{

    configASSERT(xSemaphore != NULL);

    return xSemaphoreGive(xSemaphore);
}

BaseType_t xSemaphoreGiveRecursive(SemaphoreHandle_t xMutex)
{

    configASSERT(xMutex != NULL);

    return xSemaphoreGive(xMutex);
}

UBaseType_t uxSemaphoreGetCount(SemaphoreHandle_t xSemaphore)
{
    UINT ret;
    ULONG count;

    configASSERT(xSemaphore != NULL);

    ret = tx_semaphore_info_get(&xSemaphore->sem, NULL, &count, NULL, NULL, NULL);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return 0;
    }

    return count;
}

TaskHandle_t xSemaphoreGetMutexHolder(SemaphoreHandle_t xMutex)
{
    configASSERT(xMutex != NULL);

    return xMutex->mutex.tx_mutex_owner->txfr_thread_ptr;
}


TaskHandle_t xSemaphoreGetMutexHolderFromISR(SemaphoreHandle_t xMutex)
{
    return xSemaphoreGetMutexHolder(xMutex);
}


TickType_t xTaskGetTickCount(void)
{
    return tx_time_get();
}

TickType_t xTaskGetTickCountFromISR(void)
{
    return tx_time_get();
}


QueueHandle_t xQueueCreateStatic(UBaseType_t uxQueueLength,
                                 UBaseType_t uxItemSize,
                                 uint8_t *pucQueueStorageBuffer,
                                 StaticQueue_t *pxQueueBuffer)
{
    UINT ret;

    configASSERT(uxQueueLength != 0u);
    configASSERT(uxItemSize >= sizeof(UINT));
    configASSERT(pucQueueStorageBuffer != NULL);
    configASSERT(pxQueueBuffer != NULL);

#if (TX_FREERTOS_AUTO_INIT == 1)
    if(txfr_initialized != 1u) {
        tx_freertos_auto_init();
    }
#endif

    TX_MEMSET(pucQueueStorageBuffer, 0, uxQueueLength * uxItemSize);
    TX_MEMSET(pxQueueBuffer, 0, sizeof(*pxQueueBuffer));
    pxQueueBuffer->allocated = 0u;
    pxQueueBuffer->p_mem = pucQueueStorageBuffer;
    pxQueueBuffer->id = TX_QUEUE_ID;

    pxQueueBuffer->p_write = (uint8_t *)pucQueueStorageBuffer;
    pxQueueBuffer->p_read = (uint8_t *)pucQueueStorageBuffer;
    pxQueueBuffer->msg_size = uxItemSize;
    pxQueueBuffer->queue_length = uxQueueLength;

    ret = tx_semaphore_create(&pxQueueBuffer->read_sem, "", 0u);
    if(ret != TX_SUCCESS) {
        return NULL;
    }

    ret = tx_semaphore_create(&pxQueueBuffer->write_sem, "", uxQueueLength);
    if(ret != TX_SUCCESS) {
        return NULL;
    }

    return pxQueueBuffer;
}


QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize)
{
    txfr_queue_t *p_queue;
    void *p_mem;
    size_t mem_size;
    UINT ret;

    configASSERT(uxQueueLength != 0u);
    configASSERT(uxItemSize >= sizeof(UINT));

#if (TX_FREERTOS_AUTO_INIT == 1)
    if(txfr_initialized != 1u) {
        tx_freertos_auto_init();
    }
#endif

    p_queue = txfr_malloc(sizeof(txfr_queue_t));
    if(p_queue == NULL) {
        return NULL;
    }

    mem_size = uxQueueLength*(uxItemSize);

    p_mem = txfr_malloc(mem_size);
    if(p_mem == NULL) {
        txfr_free(p_queue);
        return NULL;
    }

    TX_MEMSET(p_mem, 0, mem_size);
    TX_MEMSET(p_queue, 0, sizeof(*p_queue));
    p_queue->allocated = 1u;
    p_queue->p_mem = p_mem;
    p_queue->id = TX_QUEUE_ID;

    p_queue->p_write = (uint8_t *)p_mem;
    p_queue->p_read = (uint8_t *)p_mem;
    p_queue->msg_size = uxItemSize;
    p_queue->queue_length = uxQueueLength;

    ret = tx_semaphore_create(&p_queue->read_sem, "", 0u);
    if(ret != TX_SUCCESS) {
        return NULL;
    }

    ret = tx_semaphore_create(&p_queue->write_sem, "", uxQueueLength);
    if(ret != TX_SUCCESS) {
        return NULL;
    }

    return p_queue;
}

void vQueueDelete(QueueHandle_t xQueue)
{
    UINT ret;

    configASSERT(xQueue != NULL);

    ret = tx_semaphore_delete(&xQueue->read_sem);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
    }

    ret = tx_semaphore_delete(&xQueue->write_sem);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
    }

    if(xQueue->allocated == 1u) {
        vPortFree(xQueue->p_mem);
        vPortFree(xQueue);
    }
}

BaseType_t xQueueSend(QueueHandle_t xQueue,
                      const void *pvItemToQueue,
                      TickType_t xTicksToWait)
{
    TX_INTERRUPT_SAVE_AREA;
    UINT timeout;
    UINT ret;

    configASSERT(xQueue != NULL);
    configASSERT(pvItemToQueue != NULL);

    if(xTicksToWait ==  portMAX_DELAY) {
        timeout = TX_WAIT_FOREVER;
    } else {
        timeout = (UINT)xTicksToWait;
    }

    // Wait for space to be available on the queue.
    ret = tx_semaphore_get(&xQueue->write_sem, timeout);
    if(ret != TX_SUCCESS) {
        return pdFALSE;
    }

    // Enqueue the message.
    TX_DISABLE;
    memcpy(xQueue->p_write, pvItemToQueue, xQueue->msg_size);
    if(xQueue->p_write >= (xQueue->p_mem + (xQueue->msg_size * (xQueue->queue_length - 1u)))) {
        xQueue->p_write = xQueue->p_mem;
    } else {
        xQueue->p_write += xQueue->msg_size;
    }
    TX_RESTORE;

    // Signal that there is an additional message available on the queue.
    ret = tx_semaphore_put(&xQueue->read_sem);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return pdFALSE;
    }

    if(xQueue->p_set != NULL) {
        // To prevent deadlocks don't wait when posting on a queue set.
        ret = tx_queue_send(&xQueue->p_set->queue, &xQueue, TX_NO_WAIT);
        if((ret != TX_SUCCESS) && (ret != TX_QUEUE_FULL)) {
            // Fatal error, queue full errors are ignored on purpose to match the original behaviour.
            TX_FREERTOS_ASSERT_FAIL();
            return pdFALSE;
        }

    }

    return pdPASS;
}

BaseType_t xQueueSendFromISR(QueueHandle_t xQueue,
                             const void * pvItemToQueue,
                             BaseType_t *pxHigherPriorityTaskWoken)
{
    configASSERT(xQueue != NULL);
    configASSERT(pvItemToQueue != NULL);

    return xQueueSend(xQueue, pvItemToQueue, 0u);
}

BaseType_t xQueueSendToBack(QueueHandle_t xQueue,
                            const void * pvItemToQueue,
                            TickType_t xTicksToWait)
{
    configASSERT(xQueue != NULL);
    configASSERT(pvItemToQueue != NULL);

    return xQueueSend(xQueue, pvItemToQueue, xTicksToWait);
}

BaseType_t xQueueSendToBackFromISR(QueueHandle_t xQueue,
                             const void * pvItemToQueue,
                             BaseType_t *pxHigherPriorityTaskWoken)
{
    configASSERT(xQueue != NULL);
    configASSERT(pvItemToQueue != NULL);

    return xQueueSend(xQueue, pvItemToQueue, 0u);
}

BaseType_t xQueueSendToFront(QueueHandle_t xQueue,
                             const void *pvItemToQueue,
                             TickType_t xTicksToWait)
{
    TX_INTERRUPT_SAVE_AREA;
    UINT timeout;
    UINT ret;
    // TODO-

    configASSERT(xQueue != NULL);
    configASSERT(pvItemToQueue != NULL);

    if(xTicksToWait ==  portMAX_DELAY) {
        timeout = TX_WAIT_FOREVER;
    } else {
        timeout = (UINT)xTicksToWait;
    }

    if(xQueue->p_set != NULL) {
        TX_DISABLE;
        _tx_thread_preempt_disable++;
    }

    // Wait for space to be available on the queue.
    ret = tx_semaphore_get(&xQueue->write_sem, timeout);
    if(ret != TX_SUCCESS) {
        return pdFALSE;
    }

    // Enqueue the message at the front.
    TX_DISABLE;
    // Push back the read pointer.
    if(xQueue->p_read == xQueue->p_mem) {
        xQueue->p_read = xQueue->p_mem + (xQueue->msg_size * (xQueue->queue_length - 1u));
    } else {
        xQueue->p_read -= xQueue->msg_size;
    }

    memcpy(xQueue->p_read, pvItemToQueue, xQueue->msg_size);
    TX_RESTORE;

    // Signal that there is an additional message available on the queue.
    ret = tx_semaphore_put(&xQueue->read_sem);
    if(ret != TX_SUCCESS) {
        if(xQueue->p_set != NULL) {
            _tx_thread_preempt_disable--;
            TX_RESTORE;
        }
        TX_FREERTOS_ASSERT_FAIL();
        return pdFALSE;
    }

    if(xQueue->p_set != NULL) {
        // To prevent deadlocks don't wait when posting on a queue set.
        ret = tx_queue_send(&xQueue->p_set->queue, &xQueue, TX_NO_WAIT);
        if((ret != TX_SUCCESS) && (ret != TX_QUEUE_FULL)) {
            // Fatal error, queue full errors are ignored on purpose to match the original behaviour.
            _tx_thread_preempt_disable--;
            TX_RESTORE;
            TX_FREERTOS_ASSERT_FAIL();
            return pdFALSE;
        }

        TX_RESTORE;
        _tx_thread_preempt_disable--;

        _tx_thread_system_preempt_check();
    }

    return pdPASS;
}

BaseType_t xQueueSendToFrontFromISR(QueueHandle_t xQueue,
                             const void * pvItemToQueue,
                             BaseType_t *pxHigherPriorityTaskWoken)
{
    configASSERT(xQueue != NULL);
    configASSERT(pvItemToQueue != NULL);

    return xQueueSendToFront(xQueue, pvItemToQueue, 0u);
}

BaseType_t xQueueReceive(QueueHandle_t xQueue,
                         void *pvBuffer,
                         TickType_t xTicksToWait)
{
    TX_INTERRUPT_SAVE_AREA;
    UINT timeout;
    UINT ret;

    configASSERT(xQueue != NULL);
    configASSERT(pvBuffer != NULL);

    if(xTicksToWait == portMAX_DELAY) {
        timeout = TX_WAIT_FOREVER;
    } else {
        timeout = (UINT)xTicksToWait;
    }

    // Wait for a message to be available on the queue.
    ret = tx_semaphore_get(&xQueue->read_sem, timeout);
    if(ret != TX_SUCCESS) {
        return pdFAIL;
    }

    // Retrieve the message.
    TX_DISABLE
    memcpy(pvBuffer, xQueue->p_read, xQueue->msg_size);
    if(xQueue->p_read >= (xQueue->p_mem + (xQueue->msg_size * (xQueue->queue_length - 1u)))) {
        xQueue->p_read = xQueue->p_mem;
    } else {
        xQueue->p_read += xQueue->msg_size;
    }
    TX_RESTORE

    // Signal that there's additional space available on the queue.
    ret = tx_semaphore_put(&xQueue->write_sem);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return pdFALSE;
    }

    return pdPASS;
}

BaseType_t xQueueReceiveFromISR(QueueHandle_t xQueue,
                                void *pvBuffer,
                                BaseType_t *pxHigherPriorityTaskWoken)
{
    BaseType_t ret;

    configASSERT(xQueue != NULL);
    configASSERT(pvBuffer != NULL);

    ret = xQueueReceive(xQueue, pvBuffer, 0u);

    return ret;
}

BaseType_t xQueuePeek(QueueHandle_t xQueue,
                      void *pvBuffer,
                      TickType_t xTicksToWait)
{
    TX_INTERRUPT_SAVE_AREA;
    UINT timeout;
    UINT ret;

    configASSERT(xQueue != NULL);
    configASSERT(pvBuffer != NULL);

    if(xTicksToWait ==  portMAX_DELAY) {
        timeout = TX_WAIT_FOREVER;
    } else {
        timeout = (UINT)xTicksToWait;
    }

    // Wait for a message to be available on the queue.
    ret = tx_semaphore_get(&xQueue->read_sem, timeout);
    if(ret != TX_SUCCESS) {
        return pdFAIL;
    }

    // Retrieve the message.
    TX_DISABLE;
    _tx_thread_preempt_disable++;

    memcpy(pvBuffer, xQueue->p_read, xQueue->msg_size);

    // Restore the original space on the queue.
    ret = tx_semaphore_put(&xQueue->read_sem);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return pdFALSE;
    }

    _tx_thread_preempt_disable--;
    TX_RESTORE;

    return pdPASS;
}

BaseType_t xQueuePeekFromISR(QueueHandle_t xQueue,
                             void *pvBuffer)
{
    configASSERT(xQueue != NULL);
    configASSERT(pvBuffer != NULL);

    return xQueuePeek(xQueue, pvBuffer, 0u);
}

UBaseType_t uxQueueMessagesWaiting(QueueHandle_t xQueue)
{
    ULONG count;
    UINT ret;

    configASSERT(xQueue != NULL);

    ret = tx_semaphore_info_get(&xQueue->read_sem, NULL, &count, NULL, NULL, NULL);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return 0;
    }

    return count;
}

UBaseType_t uxQueueMessagesWaitingFromISR(QueueHandle_t xQueue)
{
    configASSERT(xQueue != NULL);

    return uxQueueMessagesWaiting(xQueue);
}

UBaseType_t uxQueueSpacesAvailable(QueueHandle_t xQueue)
{
    ULONG count;
    UINT ret;

    configASSERT(xQueue != NULL);

    ret = tx_semaphore_info_get(&xQueue->write_sem, NULL, &count, NULL, NULL, NULL);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return 0;
    }

    return count;
}

BaseType_t xQueueIsQueueEmptyFromISR(const QueueHandle_t xQueue)
{
    ULONG count;
    UINT ret;

    configASSERT(xQueue != NULL);

    ret = tx_semaphore_info_get(&xQueue->read_sem, NULL, &count, NULL, NULL, NULL);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return 0;
    }

    if(count == 0u) {
        return pdTRUE;
    } else {
        return pdFALSE;
    }
}

BaseType_t xQueueIsQueueFullFromISR(const QueueHandle_t xQueue)
{
    ULONG count;
    UINT ret;

    configASSERT(xQueue != NULL);

    ret = tx_semaphore_info_get(&xQueue->write_sem, NULL, &count, NULL, NULL, NULL);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return 0;
    }

    if(count == 0u) {
        return pdTRUE;
    } else {
        return pdFALSE;
    }
}


BaseType_t xQueueReset(QueueHandle_t xQueue)
{
    TX_INTERRUPT_SAVE_AREA;
    UINT ret;
    UINT write_post;

    configASSERT(xQueue != NULL);

    write_post = 0u;
    TX_DISABLE;
    _tx_thread_preempt_disable++;

    // Reset pointers.
    xQueue->p_write = xQueue->p_mem;
    xQueue->p_read = xQueue->p_mem;

    // Reset read semaphore.
    xQueue->read_sem.tx_semaphore_count = 0u;

    // Reset write semaphore.
    if(xQueue->write_sem.tx_semaphore_count != xQueue->queue_length) {
        write_post = 1u;
        xQueue->write_sem.tx_semaphore_count = xQueue->queue_length - 1u;
    }

    _tx_thread_preempt_disable--;
    TX_RESTORE;

    if(write_post == 1u) {
        // Signal that there's space available on the queue in case a writer was waiting before the reset.
        ret = tx_semaphore_put(&xQueue->write_sem);
        if(ret != TX_SUCCESS) {
            TX_FREERTOS_ASSERT_FAIL();
            return pdFALSE;
        }
    } else {
        _tx_thread_system_preempt_check();
    }

    return pdPASS;
}


BaseType_t xQueueOverwrite(QueueHandle_t xQueue,
                           const void * pvItemToQueue)
{
    TX_INTERRUPT_SAVE_AREA;
    UINT ret;
    UINT read_post;
    uint8_t *p_write_temp;

    configASSERT(xQueue != NULL);
    configASSERT(pvItemToQueue != NULL);

    read_post = 0u;
    TX_DISABLE;

    if(xQueue->read_sem.tx_semaphore_count != 0u) {
        // Go back one message.
        p_write_temp = xQueue->p_write;
        if(p_write_temp == xQueue->p_mem) {
            p_write_temp = (xQueue->p_mem + (xQueue->msg_size * (xQueue->queue_length - 1u)));
        } else {
            p_write_temp -= xQueue->msg_size;
        }

        memcpy(p_write_temp, pvItemToQueue, xQueue->msg_size);
    } else {
        memcpy(xQueue->p_write, pvItemToQueue, xQueue->msg_size);
        if(xQueue->p_write >= (xQueue->p_mem + (xQueue->msg_size * (xQueue->queue_length - 1u)))) {
            xQueue->p_write = xQueue->p_mem;
        } else {
            xQueue->p_write += xQueue->msg_size;
        }
        read_post = 1u;
    }

    TX_RESTORE;

    if(read_post == 1u) {
        // Signal that there is an additional message available on the queue.
        ret = tx_semaphore_put(&xQueue->read_sem);
        if(ret != TX_SUCCESS) {
            TX_FREERTOS_ASSERT_FAIL();
            return pdFALSE;
        }
    }

    return pdPASS;
}


BaseType_t xQueueOverwriteFromISR(QueueHandle_t xQueue,
                                  const void * pvItemToQueue,
                                  BaseType_t *pxHigherPriorityTaskWoken)
{
    configASSERT(xQueue != NULL);
    configASSERT(pvItemToQueue != NULL);

    return xQueueOverwrite(xQueue, pvItemToQueue);
}


EventGroupHandle_t xEventGroupCreate(void)
{
    txfr_event_t *p_event;
    UINT ret;

#if (TX_FREERTOS_AUTO_INIT == 1)
    if(txfr_initialized != 1u) {
        tx_freertos_auto_init();
    }
#endif

    p_event = txfr_malloc(sizeof(txfr_event_t));
    if(p_event == NULL) {
        return NULL;
    }

    TX_MEMSET(p_event, 0, sizeof(*p_event));
    p_event->allocated = 1u;

    ret = tx_event_flags_create(&p_event->event, "");
    if(ret != TX_SUCCESS) {
        txfr_free(p_event);
        return NULL;
    }

    return p_event;
}

EventGroupHandle_t xEventGroupCreateStatic(StaticEventGroup_t *pxEventGroupBuffer)
{
    UINT ret;

    configASSERT(pxEventGroupBuffer != NULL);

#if (TX_FREERTOS_AUTO_INIT == 1)
    if(txfr_initialized != 1u) {
        tx_freertos_auto_init();
    }
#endif

    TX_MEMSET(pxEventGroupBuffer, 0, sizeof(*pxEventGroupBuffer));
    pxEventGroupBuffer->allocated = 0u;

    ret = tx_event_flags_create(&pxEventGroupBuffer->event, "");
    if(ret != TX_SUCCESS) {
        return NULL;
    }

    return pxEventGroupBuffer;
}

void vEventGroupDelete(EventGroupHandle_t xEventGroup)
{
    UINT ret;

    configASSERT(xEventGroup != NULL);

    ret = tx_event_flags_delete(&xEventGroup->event);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return;
    }

    if(xEventGroup->allocated == 1u) {
        vPortFree(xEventGroup);
    }
}

EventBits_t xEventGroupWaitBits(const EventGroupHandle_t xEventGroup,
                                const EventBits_t uxBitsToWaitFor,
                                const BaseType_t xClearOnExit,
                                const BaseType_t xWaitForAllBits,
                                TickType_t xTicksToWait)
{
    ULONG bits;
    UINT timeout;
    UINT ret;
    UINT get_option;

    configASSERT(xEventGroup != NULL);

    if(xTicksToWait == portMAX_DELAY) {
        timeout = TX_WAIT_FOREVER;
    } else {
        timeout = (UINT)xTicksToWait;
    }

    if(xWaitForAllBits == pdFALSE) {
        if(xClearOnExit == pdFALSE) {
            get_option = TX_OR;
        } else {
            get_option = TX_OR_CLEAR;
        }
    } else {
        if(xClearOnExit == pdFALSE) {
            get_option = TX_AND;
        } else {
            get_option = TX_AND_CLEAR;
        }
    }

    ret = tx_event_flags_get(&xEventGroup->event, uxBitsToWaitFor, get_option, &bits, timeout);
    if(ret != TX_SUCCESS) {
        return 0;
    }

    return bits;
}

EventBits_t xEventGroupSetBits(EventGroupHandle_t xEventGroup,
                               const EventBits_t uxBitsToSet)
{
    UINT ret;
    ULONG bits;

    configASSERT(xEventGroup != NULL);

    ret = tx_event_flags_set(&xEventGroup->event, uxBitsToSet, TX_OR);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return 0u;
    }

    ret = tx_event_flags_info_get(&xEventGroup->event, NULL, &bits, NULL, NULL, NULL);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return 0u;
    }

    return bits;
}

BaseType_t xEventGroupSetBitsFromISR(EventGroupHandle_t xEventGroup,
                                     const EventBits_t uxBitsToSet,
                                     BaseType_t *pxHigherPriorityTaskWoken)
{
    configASSERT(xEventGroup != NULL);

    return xEventGroupSetBits(xEventGroup, uxBitsToSet);
}

EventBits_t xEventGroupClearBits(EventGroupHandle_t xEventGroup,
                                 const EventBits_t uxBitsToClear)
{
    UINT ret;
    ULONG bits;
    ULONG bits_before;
    TX_INTERRUPT_SAVE_AREA;

    configASSERT(xEventGroup != NULL);

    TX_DISABLE;

    ret = tx_event_flags_info_get(&xEventGroup->event, NULL, &bits_before, NULL, NULL, NULL);
    if(ret != TX_SUCCESS) {
        TX_RESTORE;
        TX_FREERTOS_ASSERT_FAIL();
        return 0u;
    }

    bits = uxBitsToClear;
    ret = tx_event_flags_set(&xEventGroup->event, ~bits, TX_AND);
    if(ret != TX_SUCCESS) {
        TX_RESTORE;
        TX_FREERTOS_ASSERT_FAIL();
        return 0u;
    }

    TX_RESTORE;

    return bits_before;
}

BaseType_t xEventGroupClearBitsFromISR(EventGroupHandle_t xEventGroup,
                                       const EventBits_t uxBitsToClear)
{
    configASSERT(xEventGroup != NULL);

    return xEventGroupClearBits(xEventGroup, uxBitsToClear);
}

EventBits_t xEventGroupGetBits(EventGroupHandle_t xEventGroup)
{
    UINT ret;
    ULONG bits;

    configASSERT(xEventGroup != NULL);

    ret = tx_event_flags_info_get(&xEventGroup->event, NULL, &bits, NULL, NULL, NULL);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return 0u;
    }

    return bits;
}


EventBits_t xEventGroupGetBitsFromISR(EventGroupHandle_t xEventGroup)
{
    configASSERT(xEventGroup != NULL);

    return xEventGroupGetBits(xEventGroup);
}


void txfr_timer_callback_wrapper(ULONG id)
{
    txfr_timer_t *p_timer;

    p_timer = (txfr_timer_t *)id;

    if(p_timer == NULL) {
        TX_FREERTOS_ASSERT_FAIL();
    }

    p_timer->callback(p_timer);
}


TimerHandle_t xTimerCreate(const char * const pcTimerName,
                           const TickType_t xTimerPeriod,
                           const UBaseType_t uxAutoReload,
                           void * const pvTimerID,
                           TimerCallbackFunction_t pxCallbackFunction)
{
    txfr_timer_t *p_timer;
    UINT ret;
    ULONG resch_ticks;

    configASSERT(xTimerPeriod != 0u);
    configASSERT(pxCallbackFunction != NULL);

#if (TX_FREERTOS_AUTO_INIT == 1)
    if(txfr_initialized != 1u) {
        tx_freertos_auto_init();
    }
#endif

    p_timer = txfr_malloc(sizeof(txfr_timer_t));
    if(p_timer == NULL) {
        return NULL;
    }

    TX_MEMSET(p_timer, 0, sizeof(*p_timer));
    p_timer->allocated = 1u;
    p_timer->period = xTimerPeriod;
    p_timer->id = pvTimerID;
    p_timer->callback = pxCallbackFunction;

    if(uxAutoReload != pdFALSE) {
        resch_ticks = xTimerPeriod;
        p_timer->one_shot = 1u;
    } else {
        p_timer->one_shot = 0u;
        resch_ticks = 0u;
    }

    ret = tx_timer_create(&p_timer->timer, (char *)pcTimerName, txfr_timer_callback_wrapper, (ULONG)p_timer, xTimerPeriod, resch_ticks, TX_NO_ACTIVATE);
    if(ret != TX_SUCCESS) {
        txfr_free(p_timer);
        return NULL;
    }

    return p_timer;
}

TimerHandle_t xTimerCreateStatic(const char * const pcTimerName,
                                 const TickType_t xTimerPeriod,
                                 const UBaseType_t uxAutoReload,
                                 void * const pvTimerID,
                                 TimerCallbackFunction_t pxCallbackFunction,
                                 StaticTimer_t *pxTimerBuffer)
{
    UINT ret;
    ULONG resch_ticks;

    configASSERT(xTimerPeriod != 0u);
    configASSERT(pxCallbackFunction != NULL);
    configASSERT(pxTimerBuffer != NULL);

#if (TX_FREERTOS_AUTO_INIT == 1)
    if(txfr_initialized != 1u) {
        tx_freertos_auto_init();
    }
#endif

    TX_MEMSET(pxTimerBuffer, 0, sizeof(*pxTimerBuffer));
    pxTimerBuffer->allocated = 0u;
    pxTimerBuffer->period = xTimerPeriod;
    pxTimerBuffer->id = pvTimerID;
    pxTimerBuffer->callback = pxCallbackFunction;

    if(uxAutoReload != pdFALSE) {
        resch_ticks = xTimerPeriod;
    } else {
        resch_ticks = 0u;
    }

    ret = tx_timer_create(&pxTimerBuffer->timer, (char *)pcTimerName, txfr_timer_callback_wrapper, (ULONG)pxTimerBuffer, xTimerPeriod, resch_ticks, TX_NO_ACTIVATE);
    if(ret != TX_SUCCESS) {
        return NULL;
    }

    return pxTimerBuffer;
}


BaseType_t xTimerDelete(TimerHandle_t xTimer, TickType_t xBlockTime)
{
    UINT ret;

    configASSERT(xTimer != NULL);

    ret = tx_timer_delete(&xTimer->timer);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return pdFAIL;
    }

    if(xTimer->allocated == 1u) {
        vPortFree(xTimer);
    }

    return pdPASS;
}


BaseType_t xTimerIsTimerActive(TimerHandle_t xTimer)
{
    UINT ret;
    UINT is_active;

    configASSERT(xTimer != NULL);

    ret = tx_timer_info_get(&xTimer->timer, NULL, &is_active, NULL, NULL, NULL);
    if(ret !=  TX_SUCCESS) {
        return pdFALSE;
    }

    if(is_active == TX_TRUE) {
        return pdTRUE;
    } else {
        return pdFALSE;
    }
}


BaseType_t xTimerStart(TimerHandle_t xTimer,
                       TickType_t xBlockTime)
{
    UINT ret;

    configASSERT(xTimer != NULL);

    ret = tx_timer_activate(&xTimer->timer);
    if(ret != TX_SUCCESS) {
        return pdFAIL;
    }

    return pdPASS;
}


BaseType_t xTimerStop(TimerHandle_t xTimer,
                      TickType_t xBlockTime)
{
    UINT ret;

    configASSERT(xTimer != NULL);

    ret = tx_timer_deactivate(&xTimer->timer);
    if(ret != TX_SUCCESS) {
        return pdFAIL;
    }

    return pdPASS;
}


BaseType_t xTimerChangePeriod(TimerHandle_t xTimer,
                              TickType_t xNewPeriod,
                              TickType_t xBlockTime)
{
    UINT ret;
    TX_INTERRUPT_SAVE_AREA;

    configASSERT(xTimer != NULL);
    configASSERT(xNewPeriod != 0u);

    TX_DISABLE;

    ret = tx_timer_deactivate(&xTimer->timer);
    if(ret != TX_SUCCESS) {
        TX_RESTORE;
        return pdFAIL;
    }

    if(xTimer->one_shot != 0u) {
        ret = tx_timer_change(&xTimer->timer, xNewPeriod, xNewPeriod);
    } else {
        ret = tx_timer_change(&xTimer->timer, xNewPeriod, 0u);
    }
    if(ret != TX_SUCCESS) {
        TX_RESTORE;
        return pdFAIL;
    }

    ret = tx_timer_activate(&xTimer->timer);
    if(ret != TX_SUCCESS) {
        TX_RESTORE;
        return pdFAIL;
    }

    TX_RESTORE;

    return pdPASS;
}


BaseType_t xTimerReset(TimerHandle_t xTimer,
                       TickType_t xBlockTime)
{
    UINT ret;
    TX_INTERRUPT_SAVE_AREA;

    configASSERT(xTimer != NULL);

    TX_DISABLE;

    ret = tx_timer_deactivate(&xTimer->timer);
    if(ret != TX_SUCCESS) {
        TX_RESTORE;
        return pdFAIL;
    }

    if(xTimer->one_shot != 0u) {
        ret = tx_timer_change(&xTimer->timer, xTimer->period, xTimer->period);
    } else {
        ret = tx_timer_change(&xTimer->timer, xTimer->period, 0u);
    }
    if(ret != TX_SUCCESS) {
        TX_RESTORE;
        return pdFAIL;
    }

    ret = tx_timer_activate(&xTimer->timer);
    if(ret != TX_SUCCESS) {
        TX_RESTORE;
        return pdFAIL;
    }

    TX_RESTORE;

    return pdPASS;
}


BaseType_t xTimerStartFromISR(TimerHandle_t xTimer,
                              BaseType_t *pxHigherPriorityTaskWoken)
{
    configASSERT(xTimer != NULL);

    return xTimerStart(xTimer, 0u);
}


BaseType_t xTimerStopFromISR(TimerHandle_t xTimer,
                             BaseType_t *pxHigherPriorityTaskWoken)
{
    configASSERT(xTimer != NULL);

    return xTimerStop(xTimer, 0u);
}


BaseType_t xTimerChangePeriodFromISR(TimerHandle_t xTimer,
                                     TickType_t xNewPeriod,
                                     BaseType_t *pxHigherPriorityTaskWoken)
{
    configASSERT(xTimer != NULL);
    configASSERT(xNewPeriod != 0u);

    return xTimerChangePeriod(xTimer, xNewPeriod, 0u);
}


BaseType_t xTimerResetFromISR(TimerHandle_t xTimer,
                              BaseType_t *pxHigherPriorityTaskWoken)
{
    configASSERT(xTimer != NULL);

    return xTimerReset(xTimer, 0u);
}


void *pvTimerGetTimerID(TimerHandle_t xTimer)
{
    TX_INTERRUPT_SAVE_AREA;
    void *p_id;

    configASSERT(xTimer != NULL);

    TX_DISABLE;
    p_id = xTimer->id;
    TX_RESTORE;

    return p_id;
}

void vTimerSetTimerID(TimerHandle_t xTimer, void *pvNewID)
{
    TX_INTERRUPT_SAVE_AREA;

    configASSERT(xTimer != NULL);

    TX_DISABLE;
    xTimer->id = pvNewID;
    TX_RESTORE;

    return;
}

void vTimerSetReloadMode(TimerHandle_t xTimer,
                         const UBaseType_t uxAutoReload)
{
    UINT ret;
    TX_INTERRUPT_SAVE_AREA;
    ULONG left;

    configASSERT(xTimer != NULL);

    TX_DISABLE;

    ret = tx_timer_deactivate(&xTimer->timer);
    if(ret != TX_SUCCESS) {
        TX_RESTORE;
        return;
    }

    left = xTimer->timer.tx_timer_internal.tx_timer_internal_remaining_ticks;

    if(uxAutoReload != pdFALSE) {
        ret = tx_timer_change(&xTimer->timer, left, xTimer->period);
    } else {
        ret = tx_timer_change(&xTimer->timer, left, 0u);
    }
    if(ret != TX_SUCCESS) {
        TX_RESTORE;
        return;
    }

    ret = tx_timer_activate(&xTimer->timer);
    if(ret != TX_SUCCESS) {
        TX_RESTORE;
        return;
    }

    TX_RESTORE;

    return;
}


const char * pcTimerGetName(TimerHandle_t xTimer)
{
    configASSERT(xTimer != NULL);

    return (const char *)xTimer->timer.tx_timer_name;
}


TickType_t xTimerGetPeriod(TimerHandle_t xTimer)
{
    TX_INTERRUPT_SAVE_AREA;
    TickType_t period;

    configASSERT(xTimer != NULL);

    TX_DISABLE;

    period = xTimer->period;

    TX_RESTORE;

    return period;
}


TickType_t xTimerGetExpiryTime(TimerHandle_t xTimer)
{
    TX_INTERRUPT_SAVE_AREA;
    ULONG time_tx;
    TickType_t time;
    UINT ret;

    configASSERT(xTimer != NULL);

    TX_DISABLE;

    ret = tx_timer_info_get(&xTimer->timer, NULL, NULL, &time_tx, NULL, NULL);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return 0u;
    }

    time = (TickType_t)(tx_time_get() + time_tx);

    TX_RESTORE;

    return time;
}


UBaseType_t uxTimerGetReloadMode(TimerHandle_t xTimer)
{
    TX_INTERRUPT_SAVE_AREA;
    UBaseType_t type;

    configASSERT(xTimer != NULL);

    TX_DISABLE;

    if(xTimer->one_shot == 0u) {
        type = pdTRUE;
    } else {
        type = pdFALSE;
    }

    TX_RESTORE;

    return type;
}


QueueSetHandle_t xQueueCreateSet(const UBaseType_t uxEventQueueLength)
{
    txfr_queueset_t *p_set;
    void *p_mem;
    ULONG queue_size;
    UINT ret;

    configASSERT(uxEventQueueLength != 0u);

#if (TX_FREERTOS_AUTO_INIT == 1)
    if(txfr_initialized != 1u) {
        tx_freertos_auto_init();
    }
#endif

    p_set = txfr_malloc(sizeof(txfr_queueset_t));
    if(p_set == NULL) {
        return NULL;
    }

    queue_size = sizeof(void *) * uxEventQueueLength;
    p_mem = txfr_malloc(queue_size);
    if(p_mem == NULL) {
        txfr_free(p_set);
        return NULL;
    }

    ret = tx_queue_create(&p_set->queue, "", sizeof(void *) / sizeof(UINT), p_mem, queue_size);
    if(ret != TX_SUCCESS) {
        TX_FREERTOS_ASSERT_FAIL();
        return NULL;
    }

    return p_set;
}


BaseType_t xQueueAddToSet(QueueSetMemberHandle_t xQueueOrSemaphore,
                          QueueSetHandle_t xQueueSet)
{
    txfr_sem_t *p_sem;
    txfr_queue_t *p_queue;
    TX_INTERRUPT_SAVE_AREA;

    configASSERT(xQueueOrSemaphore != NULL);
    configASSERT(xQueueSet != NULL);

    TX_DISABLE;
    if(*((ULONG *)(xQueueOrSemaphore)) == TX_SEMAPHORE_ID) {
        p_sem = (txfr_sem_t *)xQueueOrSemaphore;
        if(p_sem->p_set != NULL) {
            TX_RESTORE;
            return pdFAIL;
        }

        p_sem->p_set = xQueueSet;

    } else if(*((ULONG *)(xQueueOrSemaphore)) == TX_QUEUE_ID) {
        p_queue = (txfr_queue_t *)xQueueOrSemaphore;
        if(p_queue->p_set != NULL) {
            TX_RESTORE;
            return pdFAIL;
        }

        p_queue->p_set = xQueueSet;

    } else {
        TX_RESTORE;
        configASSERT(0u);
        return pdFAIL;
    }

    TX_RESTORE;

    return pdPASS;
}


BaseType_t xQueueRemoveFromSet(QueueSetMemberHandle_t xQueueOrSemaphore,
                               QueueSetHandle_t xQueueSet)
{
    txfr_sem_t *p_sem;
    txfr_queue_t *p_queue;
    TX_INTERRUPT_SAVE_AREA;

    configASSERT(xQueueOrSemaphore != NULL);
    configASSERT(xQueueSet != NULL);

    TX_DISABLE;

    if(*((ULONG *)(xQueueOrSemaphore)) == TX_SEMAPHORE_ID) {
        p_sem = (txfr_sem_t *)xQueueOrSemaphore;

        if(p_sem->p_set != xQueueSet) {
            TX_RESTORE;
            return pdFAIL;
        } else  {
            p_sem->p_set = NULL;
        }

    } else if(*((ULONG *)(xQueueOrSemaphore)) == TX_QUEUE_ID) {
        p_queue = (txfr_queue_t *)xQueueOrSemaphore;

        if(p_queue->p_set != xQueueSet) {
            TX_RESTORE;
            return pdFAIL;
        } else  {
            p_queue->p_set = NULL;
        }

    } else {
        TX_RESTORE;
        configASSERT(0u);
        return pdFAIL;
    }

    TX_RESTORE;

    return pdPASS;
}


QueueSetMemberHandle_t xQueueSelectFromSet(QueueSetHandle_t xQueueSet,
                                           const TickType_t xTicksToWait)
{
    void *p_ptr;
    UINT ret;
    UINT timeout;

    configASSERT(xQueueSet != NULL);

    if(xTicksToWait == portMAX_DELAY) {
        timeout = TX_WAIT_FOREVER;
    } else {
        timeout = (UINT)xTicksToWait;
    }

    ret = tx_queue_receive(&xQueueSet->queue, &p_ptr, timeout);
    if(ret != TX_SUCCESS) {
        return NULL;
    }

    return p_ptr;
}


QueueSetMemberHandle_t xQueueSelectFromSetFromISR(QueueSetHandle_t xQueueSet)
{
    return xQueueSelectFromSet(xQueueSet, 0u);
}
