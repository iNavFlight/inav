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
/*  03-02-2021     Andres Mlinar            Modified comment(s), fixed    */
/*                                             interrupt macros,          */
/*                                             resulting in version 6.1.5 */
/**************************************************************************/

#ifndef FREERTOS_H
#define FREERTOS_H

#include <stdint.h>

#include <tx_api.h>

#include <FreeRTOSConfig.h>

//// Hard coded configurations and other preprocessor definitions for compatibility.
#define portCRITICAL_NESTING_IN_TCB 0
#define portCLEAN_UP_TCB( pxTCB ) ( void ) pxTCB
#define portPRE_TASK_DELETE_HOOK( pvTaskToDelete, pxYieldPending )
#define portSETUP_TCB( pxTCB ) ( void ) pxTCB
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
#define portPRIVILEGE_BIT ((UBaseType_t)0x00)
#define portYIELD_WITHIN_API portYIELD
#define portSUPPRESS_TICKS_AND_SLEEP(xExpectedIdleTime)
#define portTASK_USES_FLOATING_POINT()
#define portALLOCATE_SECURE_CONTEXT(ulSecureStackSize)
#define portDONT_DISCARD
#define portASSERT_IF_INTERRUPT_PRIORITY_INVALID()
#define mtCOVERAGE_TEST_MARKER()
#define mtCOVERAGE_TEST_DELAY()
#define portASSERT_IF_IN_ISR()
#define portTICK_TYPE_IS_ATOMIC 1
#define portTICK_TYPE_ENTER_CRITICAL()
#define portTICK_TYPE_EXIT_CRITICAL()
#define portTICK_TYPE_SET_INTERRUPT_MASK_FROM_ISR() 0
#define portTICK_TYPE_CLEAR_INTERRUPT_MASK_FROM_ISR(x) (void) x

#if (configENABLE_BACKWARD_COMPATIBILITY == 1)
    #define eTaskStateGet eTaskGetState
    #define portTickType TickType_t
    #define xTaskHandle TaskHandle_t
    #define xQueueHandle QueueHandle_t
    #define xSemaphoreHandle SemaphoreHandle_t
    #define xQueueSetHandle QueueSetHandle_t
    #define xQueueSetMemberHandle QueueSetMemberHandle_t
    #define xTimeOutType TimeOut_t
    #define xMemoryRegion MemoryRegion_t
    #define xTaskParameters TaskParameters_t
    #define xTaskStatusType TaskStatus_t
    #define xTimerHandle TimerHandle_t
    #define xCoRoutineHandle CoRoutineHandle_t
    #define pdTASK_HOOK_CODE TaskHookFunction_t
    #define portTICK_RATE_MS portTICK_PERIOD_MS
    #define pcTaskGetTaskName pcTaskGetName
    #define pcTimerGetTimerName pcTimerGetName
    #define pcQueueGetQueueName pcQueueGetName
    #define vTaskGetTaskInfo vTaskGetInfo
    #define tmrTIMER_CALLBACK TimerCallbackFunction_t
    #define pdTASK_CODE TaskFunction_t
    #define xListItem ListItem_t
    #define xList List_t
    #define pxContainer pvContainer
#endif // (#if configENABLE_BACKWARD_COMPATIBILITY == 1)

#define tskSTATIC_AND_DYNAMIC_ALLOCATION_POSSIBLE   ( ( ( portUSING_MPU_WRAPPERS == 0 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) && ( configSUPPORT_STATIC_ALLOCATION == 1 ) ) || \
                                                      ( ( portUSING_MPU_WRAPPERS == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) ) )

//// Trace is not supported.
#define traceSTART()
#define traceEND()

//// Other
#define vQueueAddToRegistry(xQueue, pcName)
#define vQueueUnregisterQueue(xQueue)

// Assertion failure macro invoked on internal errors.
#ifndef TX_FREERTOS_ASSERT_FAIL
#define TX_FREERTOS_ASSERT_FAIL()
#endif

// Assertion check macro used to check for invalid arguments.
#ifndef configASSERT
#define configASSERT(x)
#endif

#ifndef configSTACK_DEPTH_TYPE
#define configSTACK_DEPTH_TYPE uint16_t
#endif

typedef LONG BaseType_t;
typedef ULONG UBaseType_t;
typedef UINT StackType_t;

#ifndef TX_FREERTOS_AUTO_INIT
#define TX_FREERTOS_AUTO_INIT 0
#endif

#ifndef configMINIMAL_STACK_SIZE
#error "configMINIMAL_STACK_SIZE must be defined in FreeRTOSConfig.h"
#endif

#ifndef configUSE_16_BIT_TICKS
#define configUSE_16_BIT_TICKS 0
#endif

#if (configUSE_16_BIT_TICKS == 1)
    typedef uint16_t TickType_t;
    #define portMAX_DELAY (TickType_t) (0xffffU)
#else
    typedef uint32_t TickType_t;
    #define portMAX_DELAY (TickType_t) (0xffffffffUL)

    #define portTICK_TYPE_IS_ATOMIC 1 // TODO - is this needed.
#endif

typedef void (*TaskFunction_t)(void *);

typedef enum
{
    eNoAction = 0,
    eSetBits,
    eIncrement,
    eSetValueWithOverwrite,
    eSetValueWithoutOverwrite,
} eNotifyAction;


typedef enum
{
    eRunning = 0,
    eReady,
    eBlocked,
    eSuspended,
    eDeleted,
    eInvalid
} eTaskState;

#define TXFR_NOTIFYACTION_VALID(x) (((int)x >= (int)eNoAction) && ((int)x <= (int)eSetValueWithoutOverwrite))

typedef struct txfr_queueset txfr_queueset_t;

typedef struct txfr_task txfr_task_t;

// Task related structures and type definitions.
struct txfr_task {
    txfr_task_t *p_next;
    TX_THREAD thread;
    TaskFunction_t p_task_func;
    void *p_task_arg;
    uint32_t task_notify_val;
    uint32_t task_notify_val_pend;
    uint32_t *p_notify_val_ret;
    TX_SEMAPHORE notification_sem;
    uint8_t notification_pending;
    uint8_t clear_on_pend;
    uint32_t clear_mask;
    uint8_t allocated;
};

typedef txfr_task_t StaticTask_t;
typedef txfr_task_t* TaskHandle_t;


// Semaphore related structures and type definitions.
typedef struct txfr_sem {
    TX_SEMAPHORE sem;
    TX_MUTEX mutex;
    UBaseType_t max_count;
    uint8_t allocated;
    uint8_t is_mutex;
    txfr_queueset_t *p_set;
} txfr_sem_t;

typedef txfr_sem_t *SemaphoreHandle_t;
typedef txfr_sem_t StaticSemaphore_t;


// Queue related structures and type definitions.
typedef struct txfr_queue {
    ULONG id;
    uint8_t allocated;
    txfr_queueset_t *p_set;
    uint8_t *p_mem;
    TX_SEMAPHORE read_sem;
    TX_SEMAPHORE write_sem;
    uint8_t *p_write;
    uint8_t *p_read;
    UBaseType_t queue_length;
    UBaseType_t msg_size;
} txfr_queue_t;

typedef txfr_queue_t *QueueHandle_t;
typedef txfr_queue_t StaticQueue_t;


// Event group related structures and type definitions.
typedef TickType_t EventBits_t;

typedef struct txfr_event {
    TX_EVENT_FLAGS_GROUP event;
    uint8_t allocated;
} txfr_event_t;

typedef txfr_event_t *EventGroupHandle_t;
typedef txfr_event_t StaticEventGroup_t;


// Timers related structures and type definitions.
typedef struct txfr_timer txfr_timer_t;
typedef txfr_timer_t *TimerHandle_t;
typedef txfr_timer_t StaticTimer_t;

typedef void (*TimerCallbackFunction_t)(TimerHandle_t xTimer);

struct txfr_timer {
    TX_TIMER timer;
    uint32_t period;
    uint8_t one_shot;
    uint8_t allocated;
    void *id;
    TimerCallbackFunction_t callback;
};


// Queue set related structures and type definitions.
struct txfr_queueset {
    TX_QUEUE queue;
};

typedef txfr_queueset_t *QueueSetHandle_t;
typedef void *QueueSetMemberHandle_t;

// Common definitions.
#define errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY   ( -1 )
#define errQUEUE_BLOCKED                        ( -4 )
#define errQUEUE_YIELD                          ( -5 )

#define pdFALSE         ((BaseType_t)0)
#define pdTRUE          ((BaseType_t)1)

#define pdPASS          (pdTRUE)
#define pdFAIL          (pdFALSE)
#define errQUEUE_EMPTY  ((BaseType_t)0)
#define errQUEUE_FULL   ((BaseType_t)0)


// Initialize the adaptation layer.
UINT tx_freertos_init(void);

#define tskIDLE_PRIORITY ((UBaseType_t)0U)

#define taskYIELD() tx_thread_relinquish()
#define taskYIELD_FROM_ISR()

void *pvPortMalloc(size_t xWantedSize);
void vPortFree(void *pv);
void vPortEnterCritical(void);
void vPortExitCritical(void);

////
// Task API
#ifndef taskENTER_CRITICAL_FROM_ISR
#define taskENTER_CRITICAL_FROM_ISR() __get_interrupt_state(); __disable_interrupt();
#endif

#ifndef taskEXIT_CRITICAL_FROM_ISR
#define taskEXIT_CRITICAL_FROM_ISR(x) __set_interrupt_state(x);
#endif

#ifndef portDISABLE_INTERRUPTS
#if defined(__IAR_SYSTEMS_ICC__)
#define portDISABLE_INTERRUPTS() __disable_interrupt()
#elif defined(__GNUC__ )
#define portDISABLE_INTERRUPTS() __disable_interrupts()
#elif defined(__ARMCC_VERSION)
#define portDISABLE_INTERRUPTS() __disable_irq()
#else
UINT _tx_thread_interrupt_disable(VOID);
#define portDISABLE_INTERRUPTS() _tx_thread_interrupt_disable()
#endif
#endif

#ifndef portENABLE_INTERRUPTS
#if defined(__IAR_SYSTEMS_ICC__)
#define portENABLE_INTERRUPTS() __enable_interrupt()
#elif defined(__GNUC__ )
#define portENABLE_INTERRUPTS() __enable_interrupts()
#elif defined(__ARMCC_VERSION)
#define portENABLE_INTERRUPTS() __enable_irq()
#else
VOID _tx_thread_interrupt_restore(UINT previous_posture);     
#define portENABLE_INTERRUPTS() _tx_thread_interrupt_restore(TX_INT_ENABLE)
#endif
#endif

#define taskENTER_CRITICAL() portENTER_CRITICAL()
#define taskEXIT_CRITICAL() portEXIT_CRITICAL()
#define portENTER_CRITICAL() vPortEnterCritical()
#define portEXIT_CRITICAL() vPortExitCritical()

#define taskDISABLE_INTERRUPTS() portDISABLE_INTERRUPTS()
#define taskENABLE_INTERRUPTS() portENABLE_INTERRUPTS()

#define taskSCHEDULER_SUSPENDED ((BaseType_t)0)
#define taskSCHEDULER_NOT_STARTED ((BaseType_t)1)
#define taskSCHEDULER_RUNNING ((BaseType_t)2)

void vTaskStartScheduler(void);

BaseType_t xTaskGetSchedulerState(void);

void vTaskSuspendAll(void);

BaseType_t xTaskResumeAll(void);

TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,
                               const char *const pcName,
                               const configSTACK_DEPTH_TYPE ulStackDepth,
                               void *const pvParameters,
                               UBaseType_t uxPriority,
                               StackType_t *const puxStackBuffer,
                               StaticTask_t *const pxTaskBuffer);

BaseType_t xTaskCreate(TaskFunction_t pvTaskCode,
                       const char * const pcName,
                       const configSTACK_DEPTH_TYPE usStackDepth,
                       void *pvParameters,
                       UBaseType_t uxPriority,
                       TaskHandle_t * const pxCreatedTask);

UBaseType_t uxTaskGetNumberOfTasks(void);

void vTaskDelete(TaskHandle_t xTask);

void vTaskDelay(const TickType_t xTicksToDelay);

void vTaskDelayUntil(TickType_t * const pxPreviousWakeTime, const TickType_t xTimeIncrement);

TaskHandle_t xTaskGetCurrentTaskHandle(void);

void vTaskSuspend(TaskHandle_t xTaskToSuspend);

void vTaskResume(TaskHandle_t xTaskToResume);

BaseType_t xTaskResumeFromISR(TaskHandle_t xTaskToResume);

BaseType_t xTaskAbortDelay(TaskHandle_t xTask);

UBaseType_t uxTaskPriorityGet(const TaskHandle_t xTask);

UBaseType_t uxTaskPriorityGetFromISR(const TaskHandle_t xTask);

void vTaskPrioritySet(TaskHandle_t xTask,
                      UBaseType_t uxNewPriority);

char *pcTaskGetName(TaskHandle_t xTaskToQuery);

eTaskState eTaskGetState(TaskHandle_t xTask);

TickType_t xTaskGetTickCount(void);

TickType_t xTaskGetTickCountFromISR(void);

////
// Task notification API.

BaseType_t xTaskNotifyGive(TaskHandle_t xTaskToNotify);

void vTaskNotifyGiveFromISR(TaskHandle_t xTaskToNotify,
                            BaseType_t *pxHigherPriorityTaskWoken);

uint32_t ulTaskNotifyTake(BaseType_t xClearCountOnExit,
                          TickType_t xTicksToWait);

BaseType_t xTaskNotifyWait(uint32_t ulBitsToClearOnEntry,
                           uint32_t ulBitsToClearOnExit,
                           uint32_t *pulNotificationValue,
                           TickType_t xTicksToWait );

BaseType_t xTaskNotify(TaskHandle_t xTaskToNotify,
                        uint32_t ulValue,
                        eNotifyAction eAction);

BaseType_t xTaskNotifyFromISR(TaskHandle_t xTaskToNotify,
                              uint32_t ulValue,
                              eNotifyAction eAction,
                              BaseType_t *pxHigherPriorityTaskWoken);

BaseType_t xTaskNotifyAndQuery(TaskHandle_t xTaskToNotify,
                               uint32_t ulValue,
                               eNotifyAction eAction,
                               uint32_t *pulPreviousNotifyValue);

#define xTaskGenericNotify(a, b, c, d) xTaskNotifyAndQuery(a, b, c, d)

BaseType_t xTaskNotifyAndQueryFromISR(TaskHandle_t xTaskToNotify,
                                      uint32_t ulValue,
                                      eNotifyAction eAction,
                                      uint32_t *pulPreviousNotifyValue,
                                      BaseType_t *pxHigherPriorityTaskWoken);

#define xTaskGenericNotifyFromISR(a, b, c, d, e) xTaskNotifyAndQueryFromISR(a, b, c, d, e)

BaseType_t xTaskNotifyStateClear(TaskHandle_t xTask);

uint32_t ulTaskNotifyValueClear(TaskHandle_t xTask,
                                uint32_t ulBitsToClear);

////
// Semaphore API.

#define semBINARY_SEMAPHORE_QUEUE_LENGTH ((uint8_t)1U)
#define semSEMAPHORE_QUEUE_ITEM_LENGTH ((uint8_t)0U)
#define semGIVE_BLOCK_TIME ((TickType_t)0U)


SemaphoreHandle_t xSemaphoreCreateCounting(UBaseType_t uxMaxCount,
                                           UBaseType_t uxInitialCount);

SemaphoreHandle_t xSemaphoreCreateCountingStatic(UBaseType_t uxMaxCount,
                                                 UBaseType_t uxInitialCount,
                                                 StaticSemaphore_t *pxSemaphoreBuffer);

SemaphoreHandle_t xSemaphoreCreateBinary(void);

SemaphoreHandle_t xSemaphoreCreateBinaryStatic(StaticSemaphore_t *pxSemaphoreBuffer);

SemaphoreHandle_t xSemaphoreCreateMutex(void);

SemaphoreHandle_t xSemaphoreCreateMutexStatic(StaticSemaphore_t *pxMutexBuffer);

SemaphoreHandle_t xSemaphoreCreateRecursiveMutex(void);

SemaphoreHandle_t xSemaphoreCreateRecursiveMutexStatic(StaticSemaphore_t *pxMutexBuffer);

void vSemaphoreDelete(SemaphoreHandle_t xSemaphore);

BaseType_t xSemaphoreTake(SemaphoreHandle_t xSemaphore, TickType_t xTicksToWait);

BaseType_t xSemaphoreTakeFromISR(SemaphoreHandle_t xSemaphore, BaseType_t *pxHigherPriorityTaskWoken);

BaseType_t xSemaphoreTakeRecursive(SemaphoreHandle_t xMutex, TickType_t xTicksToWait);

BaseType_t xSemaphoreGive(SemaphoreHandle_t xSemaphore);

BaseType_t xSemaphoreGiveFromISR(SemaphoreHandle_t xSemaphore, BaseType_t *pxHigherPriorityTaskWoken);

BaseType_t xSemaphoreGiveRecursive(SemaphoreHandle_t xMutex);

UBaseType_t uxSemaphoreGetCount(SemaphoreHandle_t xSemaphore);

TaskHandle_t xSemaphoreGetMutexHolder(SemaphoreHandle_t xMutex);

TaskHandle_t xSemaphoreGetMutexHolderFromISR(SemaphoreHandle_t xMutex);


////
// Queue API.

QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize);

QueueHandle_t xQueueCreateStatic(UBaseType_t uxQueueLength,
                                 UBaseType_t uxItemSize,
                                 uint8_t *pucQueueStorageBuffer,
                                 StaticQueue_t *pxQueueBuffer );

void vQueueDelete(QueueHandle_t xQueue);

BaseType_t xQueueSend(QueueHandle_t xQueue,
                      const void * pvItemToQueue,
                      TickType_t xTicksToWait);

BaseType_t xQueueSendFromISR(QueueHandle_t xQueue,
                             const void * pvItemToQueue,
                             BaseType_t *pxHigherPriorityTaskWoken);

BaseType_t xQueueSendToBack(QueueHandle_t xQueue,
                            const void * pvItemToQueue,
                            TickType_t xTicksToWait);

BaseType_t xQueueSendToBackFromISR(QueueHandle_t xQueue,
                                   const void * pvItemToQueue,
                                   BaseType_t *pxHigherPriorityTaskWoken);

BaseType_t xQueueSendToFront(QueueHandle_t xQueue,
                             const void * pvItemToQueue,
                             TickType_t xTicksToWait);

BaseType_t xQueueSendToFrontFromISR(QueueHandle_t xQueue,
                                    const void * pvItemToQueue,
                                    BaseType_t *pxHigherPriorityTaskWoken);

BaseType_t xQueueReceive(QueueHandle_t xQueue,
                         void *pvBuffer,
                         TickType_t xTicksToWait);

BaseType_t xQueueReceiveFromISR(QueueHandle_t xQueue,
                                void *pvBuffer,
                                BaseType_t *pxHigherPriorityTaskWoken);

BaseType_t xQueuePeek(QueueHandle_t xQueue,
                      void *pvBuffer,
                      TickType_t xTicksToWait);

BaseType_t xQueuePeekFromISR(QueueHandle_t xQueue,
                             void *pvBuffer);

UBaseType_t uxQueueMessagesWaiting(QueueHandle_t xQueue);

UBaseType_t uxQueueMessagesWaitingFromISR(QueueHandle_t xQueue);

UBaseType_t uxQueueSpacesAvailable(QueueHandle_t xQueue);

BaseType_t xQueueIsQueueEmptyFromISR(const QueueHandle_t xQueue);

BaseType_t xQueueIsQueueFullFromISR(const QueueHandle_t xQueue);

BaseType_t xQueueReset(QueueHandle_t xQueue);

BaseType_t xQueueOverwrite(QueueHandle_t xQueue,
                           const void * pvItemToQueue);

BaseType_t xQueueOverwriteFromISR(QueueHandle_t xQueue,
                                  const void * pvItemToQueue,
                                  BaseType_t *pxHigherPriorityTaskWoken);


////
// Event group API.

EventGroupHandle_t xEventGroupCreate(void);

EventGroupHandle_t xEventGroupCreateStatic(StaticEventGroup_t *pxEventGroupBuffer);

void vEventGroupDelete(EventGroupHandle_t xEventGroup);

EventBits_t xEventGroupWaitBits(const EventGroupHandle_t xEventGroup,
                                const EventBits_t uxBitsToWaitFor,
                                const BaseType_t xClearOnExit,
                                const BaseType_t xWaitForAllBits,
                                TickType_t xTicksToWait);

EventBits_t xEventGroupSetBits(EventGroupHandle_t xEventGroup,
                               const EventBits_t uxBitsToSet);

BaseType_t xEventGroupSetBitsFromISR(EventGroupHandle_t xEventGroup,
                                     const EventBits_t uxBitsToSet,
                                     BaseType_t *pxHigherPriorityTaskWoken);

EventBits_t xEventGroupClearBits(EventGroupHandle_t xEventGroup,
                                 const EventBits_t uxBitsToClear);

BaseType_t xEventGroupClearBitsFromISR(EventGroupHandle_t xEventGroup,
                                       const EventBits_t uxBitsToClear);

EventBits_t xEventGroupGetBits(EventGroupHandle_t xEventGroup);

EventBits_t xEventGroupGetBitsFromISR(EventGroupHandle_t xEventGroup);


////
// Software timer API.

#ifndef pdMS_TO_TICKS
#ifndef configTICK_RATE_HZ
#error "configTICK_RATE_HZ must be defined in FreeRTOSConfig.h"
#endif // #ifndef configTICK_RATE_HZ
    #define pdMS_TO_TICKS(xTimeInMs) ((TickType_t) (((TickType_t) (xTimeInMs) * (TickType_t)configTICK_RATE_HZ) / (TickType_t)1000))
#endif // #ifndef pdMS_TO_TICKS

TimerHandle_t xTimerCreate(const char * const pcTimerName,
                           const TickType_t xTimerPeriod,
                           const UBaseType_t uxAutoReload,
                           void * const pvTimerID,
                           TimerCallbackFunction_t pxCallbackFunction);

TimerHandle_t xTimerCreateStatic(const char * const pcTimerName,
                                 const TickType_t xTimerPeriod,
                                 const UBaseType_t uxAutoReload,
                                 void * const pvTimerID,
                                 TimerCallbackFunction_t pxCallbackFunction,
                                 StaticTimer_t *pxTimerBuffer);

BaseType_t xTimerDelete(TimerHandle_t xTimer, TickType_t xBlockTime);

BaseType_t xTimerIsTimerActive(TimerHandle_t xTimer);

BaseType_t xTimerStart(TimerHandle_t xTimer,
                       TickType_t xBlockTime);

BaseType_t xTimerStop(TimerHandle_t xTimer,
                      TickType_t xBlockTime);

BaseType_t xTimerChangePeriod(TimerHandle_t xTimer,
                              TickType_t xNewPeriod,
                              TickType_t xBlockTime);

BaseType_t xTimerReset(TimerHandle_t xTimer,
                       TickType_t xBlockTime);

BaseType_t xTimerStartFromISR(TimerHandle_t xTimer,
                              BaseType_t *pxHigherPriorityTaskWoken);

BaseType_t xTimerStopFromISR(TimerHandle_t xTimer,
                             BaseType_t *pxHigherPriorityTaskWoken);

BaseType_t xTimerChangePeriodFromISR(TimerHandle_t xTimer,
                                     TickType_t xNewPeriod,
                                     BaseType_t *pxHigherPriorityTaskWoken);

BaseType_t xTimerResetFromISR(TimerHandle_t xTimer,
                              BaseType_t *pxHigherPriorityTaskWoken);

void *pvTimerGetTimerID(TimerHandle_t xTimer);

void vTimerSetTimerID(TimerHandle_t xTimer, void *pvNewID);

void vTimerSetReloadMode(TimerHandle_t xTimer,
                         const UBaseType_t uxAutoReload);

const char * pcTimerGetName(TimerHandle_t xTimer);

TickType_t xTimerGetPeriod(TimerHandle_t xTimer);

TickType_t xTimerGetExpiryTime(TimerHandle_t xTimer);

UBaseType_t uxTimerGetReloadMode(TimerHandle_t xTimer);

////
// Queue set API.

QueueSetHandle_t xQueueCreateSet(const UBaseType_t uxEventQueueLength);

BaseType_t xQueueAddToSet(QueueSetMemberHandle_t xQueueOrSemaphore,
                          QueueSetHandle_t xQueueSet);

BaseType_t xQueueRemoveFromSet(QueueSetMemberHandle_t xQueueOrSemaphore,
                               QueueSetHandle_t xQueueSet);

QueueSetMemberHandle_t xQueueSelectFromSet(QueueSetHandle_t xQueueSet,
                                           const TickType_t xTicksToWait);

QueueSetMemberHandle_t xQueueSelectFromSetFromISR(QueueSetHandle_t xQueueSet);

#endif /* FREERTOS_H */
