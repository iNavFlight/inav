# FreeRTOS adaptation layer

Introduction
------------
Welcome to the FreeRTOS adaptation layer for ThreadX documentation. This document will go over configuration, initialization and usage of the adaptation layer as well as important guidelines and limitations.

Files
-----
The adaptation layer is comprised of the following files:
-	tx_freertos.c
-	FreeRTOS.h
-	event_groups.h
-	queue.h
-	semphr.h
-	task.h
-	timers.h

The main source file for the adaptation layer is “tx_freertos.c” as well as “FreeRTOS.h” the other headers are provided for compatibility with FreeRTOS.
In addition, the following configuration file must be available within the project. A template of the configuration file is provided in the source distribution.
-	FreeRTOSConfig.h

ThreadX Configuration
---------------------
A few ThreadX configurations should be looked at prior to using the adaptation layer. Please note that if a configuration is changed within `tx_user.h` the preprocessor definition `TX_INCLUDE_USER_DEFINE_FILE` should be defined at the compiler command line. This is to ensure that `tx_user.h` is properly included from `tx_port.h`.

## Thread Extension:
The adaptation layer requires a custom field within the `TX_THREAD` data structure to store a reference to the adaptation layer thread-related data. The following preprocessor definitions should be added to `tx_user.h`. Failure to add this configuration will result in a compilation error.

`#define TX_THREAD_USER_EXTENSION VOID *txfr_thread_ptr;`

## Timer Processing Task:
To better emulate the FreeRTOS timer behaviour it is recommended, but not necessary, to enable processing of ThreadX timers within a task instead of within an ISR. To do so the `TX_TIMER_PROCESS_IN_ISR` preprocessor definition should NOT  be defined within `tx_user.h` or `tx_port.h` It is also recommended, but not required to have the timer task priority  set at priority 0, which is the highest priority within ThreadX, like this:

`#define TX_TIMER_THREAD_PRIORITY 0`

If desired to reduce resource usage, timer processing can be done within the timer tick ISR by defining `TX_TIMER_PROCESS_IN_ISR` within `tx_user.h`. This won’t have any negative side effect but may change the sequencing of timer firing compared to FreeRTOS.

Adaptation Layer Setup and Configuration
----------------------------------------
To include the adaptation layer in a ThreadX project it is sufficient to add to the makefile or project the `tx_freertos.c` source file as well as create or copy the `FreeRTOSConfig.h` configuration file. The configuration can be taken from an existing project but care should be taken to ensure that it contains no extraneous declarations and definitions that may cause compilation errors. A template of the configuration file can be found within the config_template directory. Every uncommented definitions within the template configuration file are understood by the adaptation layer while every other configuration definitions are ignored. The various FreeRTOS headers can be found at the root of the adaptation layer source directory.
For simplicity only a selected set of the usual configuration defines are supported by the adaptation layer. All other configurations not explicitly listed are ignored. In addition a few additional definitions can be added to `FreeRTOSConfig.h` to tune the behaviour of the adaptation layer.

| Name | Default Value | Description |
|------|---------------|-------------|
| configUSE_16_BIT_TICKS | 0 |	Set to 1 to use 16-bit tick |
| configSTACK_DEPTH_TYPE | uint16_t | Use to override the type used to specify stack depth |
| configTICK_RATE_HZ | - | Set the kernel tick rate, used by the pdMS_TO_TICKS() macro |
| configMAX_PRIORITIES | - | Maximum number of priorities. Must be less than or equal to the configured number of ThreadX priorities. |
| configMINIMAL_STACK_SIZE | 512u | Minimum stack size, used as the stack size of the idle task if `TX_FREERTOS_IDLE_STACK` is not defined.
| configTOTAL_HEAP_SIZE | - | Amount of internal memory allocated to the adaptation layer when creating FreeRTOS objects. Can be set to 0 to disable dynamic allocation. |
| INCLUDE_vTaskDelete | 1 | Set to 0 to disable the task delete API. When disabled the adaptation layer will not create the idle task to save resources. | 
| TX_FREERTOS_IDLE_STACK |	512u | Stack size of the idle task. |
| TX_FREERTOS_ASSERT_FAIL | | Define to a macro invoked on internal assertion failures from within the adaptation layer |
| configASSERT | | Define to a macro invoked for invalid arguments |

## Initialization
Unless auto-initialization is used, see below, early initialization should proceed as is usual for any ThreadX application. The adaptation layer should be initialized upon reaching `tx_application_define()` by calling `tx_freertos_init()`. Internally `tx_freertos_init()` will initialize a ThreadX byte pool that will be used by the adaptation layer to allocate and free kernel objects.
FreeRTOS tasks and objects can be created from within `tx_application_define()`. Usually, at least the initial application task should be created.

Here’s an example of a minimal initialization of the adaptation layer.

```cpp
VOID tx_application_define(VOID * first_unused_memory)
{
    BaseType_t error;
    TaskHandle_t task_handle;

    tx_freertos_init();

    error = xTaskCreate(first_thread_entry, "Initial Task", STACK_SIZE, NULL, 10, &task_handle);
    if(error != pdPASS) {
        // Handle error.
    }
}
```
It is also possible to initialize the FreeRTOS adaptation layer later once ThreadX is started as long as `tx_freertos_init()` is called before attempting to create any FreeRTOS kernel objects or tasks.

## Auto-initialization
The default method of initializing the adaptation layer requires some modifications to the application to initialize ThreadX and the adaptation layer prior to using the FreeRTOS API. An alternative method is available when modifications to the application isn't desirable. To do so the following configuration should be added and set to one in FreeRTOSConfig.h:

`#define TX_FREERTOS_AUTO_INIT 1`

Additionally the following preprocessor definition should be added to `tx_user.h`:

`#define TX_PORT_SPECIFIC_PRE_SCHEDULER_INITIALIZATION return;`

When both of those configurations are done, the adaptation layer will be initialized  automatically by the first call to an object create function, no other call is allowed prior to starting the kernel by calling `vTaskStartScheduler()`.

## Port Macros
Four port macros to manipulate ISRs are required by the adaptation layer. Default implementations are provided for IAR. They can be defined in `FreeRTOSConfig.h` if needed.

`taskENTER_CRITICAL_FROM_ISR()`

`taskEXIT_CRITICAL_FROM_ISR`

`portDISABLE_INTERRUPTS`

`portENABLE_INTERRUPTS`

Usage Guidelines and Limitations
--------------------------------
While the adaptation layer attempts to emulate the behaviour and feature set of FreeRTOS it must be understood that some limitations and deviations exist. Every application developer using the adaptation layer should review this document, especially the exhaustive list of supported API presented later in this document along with important deviations in behaviour from FreeRTOS. In addition, the following general guidelines should be followed.

## Usage of FreeRTOS API Calls Within a Native ThreadX Thread
The scenario of using FreeRTOS calls within a thread created using `tx_thread_create()` is not supported. While it is not explicitly disallowed by the adaptation layer some FreeRTOS API may function erratically when they are called from outside a FreeRTOS task.

## Usage of ThreadX Native API Calls Within a FreeRTOS Task
Similarly to the above scenario, usage of native ThreadX calls from within a task created using `xTaskCreate()` or `xTaskCreateStatic()` is not recommended, with exceptions. It is possible to use any of the ThreadX synchronizations objects, such as Semaphore, Mutexes, Queues and event flags directly from within a FreeRTOS task. It is not supported to use any of the ThreadX thread manipulation functions, however.

## Mix of ThreadX Threads and FreeRTOS Tasks
It is possible to mix native ThreadX threads and FreeRTOS threads in the same applications assuming that previous two guidelines are followed.

## Idle Task and Idle Priority
ThreadX by its design does not have an idle task. FreeRTOS, however, does have an idle task, and it is responsible for performing the final cleanup and freeing of memory when deleting a task. The adaptation layer creates, during initialization, an idle task to perform the same duty. The idle task has the lowest possible priority allowed by ThreadX namely `TX_MAX_PRIORITIES – 1`. The idle task will only run if there is a deleted thread to cleanup, otherwise it will be waiting on an internal semaphore posted from the adaptation layer `vTaskDelete()` function.

## Task Yielding and Preemption From ISR
When returning from an ISR, ThreadX will automatically switch to the highest priority task ready to run. As such the `taskYIELD_FROM_ISR()` macro has no effect and yielding will always occur if a higher priority task was made ready to run within and ISR. `taskYIELD()` however works as expected yielding control to the next ready-to-run task with the same priority as the current task.

## Task Deletion and the Idle Task
FreeRTOS uses the Idle task, which is created during initialization, to cleanup deleted threads. The Threadx adaptation layer mimics this behaviour by deleting and freeing any resources allocated to a task within an internal idle task.  If the `vTaskDelete()` call is disabled by setting `INCLUDE_vTaskDelete` to 0, the idle task will not be created and the task delete functionality won’t be available.

## Returning From a Task
FreeRTOS does not allow simply returning from a task while this behaviour is permitted within ThreadX. The adaptation layer allows returning from a task when `vTaskDelete()` is available although for portability it is recommended to always explicitly delete tasks. When `vTaskDelete()` is disabled by setting `INCLUDE_vTaskDelete` to 0, returning from a task will cause an assertion failure.

## Memory Management and Heap Configuration
The total memory size available to the adaptation layer when creating FreeRTOS kernel objects dynamically is configurable through the `configTOTAL_HEAP_SIZE` definition located in `FreeRTOSConfig.h` configuration file. An area of memory of the specified size is created internally and managed using a ThreadX byte pool. Setting `configTOTAL_HEAP_SIZE` to 0 will effectively disable dynamic allocation of kernel objects.

## Tickless Mode
Tickless mode, which can be selected using `configUSE_TICKLESS_IDLE` is not supported by the adaptation layer or ThreadX.

API Support by Category
-----------------------
This release of the FreeRTOS adaptation layer for ThreadX broadly supports the following API groups:
-	Task creation, control and utilities
-	Semaphores and Mutexes
-	Queues
-	Queue Sets
-	Direct to Task Notifications
-	Software Timers
-	Event Groups

The tables that follow list the individual along with any notable limitations or deviations from the FreeRTOS behaviour of each API or API group.

## Task
The task API represents the core of the adaptation layer enabling creation and control of FreeRTOS tasks using underlying ThreadX threads. FreeRTOS priorities are transparently mapped to ThreadX priorities in reverse orders since under FreeRTOS increasing priority values means an increasing task priority which is the reverse of the ThreadX convention.

### Macros
| Name | Notes |
|------|-------|
| taskSCHEDULER_SUSPENDED
| taskSCHEDULER_NOT_STARTED	
| taskSCHEDULER_RUNNING	
| taskENTER_CRITICAL()	
| taskEXIT_CRITICAL()	
| taskENTER_CRITICAL_FROM_ISR()	
| taskEXIT_CRITICAL_FROM_ISR()	
| taskDISABLE_INTERRUPTS()	
| taskENABLE_INTERRUPTS()	
| tskIDLE_PRIORITY	
| taskYIELD()	
| taskYIELD_FROM_ISR()	| Has no effect, ThreadX will automatically pre-empt when a higher priority task is available to run upon returning from an ISR. |

### Functions
| Names | Notes |
|-------|-------|
| vTaskStartScheduler() | Has no effect if `TX_FREERTOS_AUTO_INIT` is undefined or set to 0, scheduler is started automatically when returning from `tx_application_define().` Otherwise this call will start the scheduler for the first time. |
| xTaskGetSchedulerState()
| vTaskSuspendAll()
| xTaskResumeAll() | Always return pdFALSE since pre-emption is handled automatically by ThreadX. |
| xTaskCreateStatic()
| xTaskCreate()
| uxTaskGetNumberOfTasks() | Only returns the number of task created by either `xTaskCreate()` or `xTaskcreateStatic()`. Task created internally by ThreadX or by the application using `tx_thread_create()` are not counted. |
| vTaskDelete()
| vTaskDelay()	
| vTaskDelayUntil() | The implementation of `vTaskDelayUntil()` cannot perform a wait in an atomic fashion. As such there might be additional jitter when using this function with the adaptation layer. The implementation will, however, not accumulate any drift. |
| xTaskGetCurrentTaskHandle() | This will only work when called from a task created by either `xTaskCreate()` or `xTaskcreateStatic()`. |
| vTaskSuspend()
| vTaskResume()	
| xTaskResumeFromISR()	
| xTaskAbortDelay()	
| uxTaskPriorityGet()	
| uxTaskPriorityGetFromISR()	
| vTaskPrioritySet()	
| pcTaskGetName()	
| eTaskGetState()	
| uxTaskGetStackHighWaterMark() | Not implemented. |
| uxTaskGetStackHighWaterMark2() | Not implemented. |
| xTaskCallApplicationTaskHook() | Not implemented. |
| xTaskGetIdleTaskHandle() | Not implemented since the idle task is not a FreeRTOS task. |
| uxTaskGetSystemState() | Not implemented. |
| vTaskList() | Not implemented. |
| vTaskGetRunTimeStats() | Not implemented. |
| xTaskGetIdleRunTimeCounter() | Not implemented since the idle task is not free-running but waiting for delete events. |

## Task Notification
Task notifications are fully implemented.

| Name | Notes |
|------|-------|
| xTaskNotifyGive()	
| vTaskNotifyGiveFromISR()	
| ulTaskNotifyTake()	
| xTaskNotifyWait()	
| xTaskNotify()	
| xTaskNotifyFromISR()	
| xTaskNotifyAndQuery()	
| xTaskGenericNotify()	
| xTaskNotifyAndQueryFromISR()	
| xTaskGenericNotifyFromISR()	
| xTaskNotifyStateClear()	
| ulTaskNotifyValueClear()	

## Semaphore and Mutex
Semaphores, either counting or binary as well as Mutexes are fully implemented. Mutexes under the adaptation layer cannot be taken or given from an ISR as this is not allowed in ThreadX as well as recent version of FreeRTOS. Due to differences between ThreadX and FreeRTOS it is possible that the ordering task wakeup may slightly differ.

| Name | Notes |
|------|-------|
| xSemaphoreCreateCounting()	
| xSemaphoreCreateCountingStatic()	
| xSemaphoreCreateBinary()	
| xSemaphoreCreateBinaryStatic()	
| xSemaphoreCreateMutex()	
| xSemaphoreCreateMutexStatic()	
| xSemaphoreCreateRecursiveMutex()	
| xSemaphoreCreateRecursiveMutexStatic()	
| vSemaphoreDelete()	
| xSemaphoreTake()	
| xSemaphoreTakeFromISR() | It’s not possible to take a mutex from an ISR. |
| xSemaphoreTakeRecursive()	
| xSemaphoreGive()	
| xSemaphoreGiveFromISR()	
| xSemaphoreGiveRecursive() | It’s not possible to give a mutex from an ISR. |
| uxSemaphoreGetCount()
| xSemaphoreGetMutexHolder()	
| xSemaphoreGetMutexHolderFromISR()	
| vSemaphoreCreateBinary() | Not implemented since it’s marked as deprecated in the FreeRTOS documentation. |

## Queue
The FreeRTOS queue API is implemented with the help of ThreadX semaphores and is designed to mimic the behaviour of FreeRTOS queues. Due to differences between ThreadX and FreeRTOS it is possible that the ordering task wakeup may slightly differ. 

| Name | Notes |
|------|-------|
| xQueueCreate() |
| xQueueCreateStatic() |
| vQueueDelete()	
| xQueueSend()	
| xQueueSendFromISR()	
| xQueueSendToBack()	
| xQueueSendToBackFromISR()	
| xQueueSendToFront() |
| xQueueSendToFrontFromISR() |
| xQueueReceive()
| xQueueReceiveFromISR()	
| xQueuePeek() |
| xQueuePeekFromISR() |
| uxQueueMessagesWaiting() |
| uxQueueMessagesWaitingFromISR() |
| uxQueueSpacesAvailable() |
| xQueueIsQueueEmptyFromISR()	
| xQueueIsQueueFullFromISR()	
| xQueueReset()	
| xQueueOverwrite() |
| xQueueOverwriteFromISR() |
| pcQueueGetName() | Not implemented. |

## Queue Sets
Queue sets are implemented with support for adding queues, semaphores and mutexes to a set. Due to the way ThreadX deliver messages, it is possible that the order of events returned by `xQueueSelectFromSet()` and `xQueueSelectFromSetFromISR()` differs from the order they would be returned by FreeRTOS.

| Name | Notes |
|------|-------|
| xQueueCreateSet()	
| xQueueAddToSet()	
| xQueueRemoveFromSet()	
| xQueueSelectFromSet()	
| xQueueSelectFromSetFromISR()	

## Event Group
Event groups are implemented using ThreadX’s event flags. It is important to note however that `xEventGroupSync()` is not atomic.

| Name | Notes|
|------|-------|
| xEventGroupCreate()	
| xEventGroupCreateStatic()	
| vEventGroupDelete()	
| xEventGroupWaitBits()	
| xEventGroupSetBits()	
| xEventGroupSetBitsFromISR()	
| xEventGroupClearBits()	
| xEventGroupClearBitsFromISR()	
| xEventGroupGetBits()	
| xEventGroupGetBitsFromISR()	
| xEventGroupSync() | Not atomic. |

## Timer
The timer API is fully implemented except for the pend function all functionality provided by `xTimerPendFunctionCall()` and `xTimerPendFunctionCallFromISR()`. Also since the timer handling thread is not a FreeRTOS task, `xTimerGetTimerDaemonTaskHandle()` is not supported as well.

| Name | Notes |
|------|-------|
| xTimerCreate()	
| xTimerCreateStatic()	
| xTimerDelete()	
| xTimerIsTimerActive()	
| xTimerStart()	
| xTimerStop()	
| xTimerChangePeriod()	
| xTimerReset()	
| xTimerStartFromISR()	
| xTimerStopFromISR()	
| xTimerChangePeriodFromISR()	
| xTimerResetFromISR()	
| pvTimerGetTimerID()	
| vTimerSetTimerID()	
| vTimerSetReloadMode()	
| pcTimerGetName()	
| xTimerGetPeriod()	
| xTimerGetExpiryTime()	
| uxTimerGetReloadMode()	
| xTimerPendFunctionCall() | Not implemented. |
| xTimerPendFunctionCallFromISR() | Not implemented. |
| xTimerGetTimerDaemonTaskHandle() | Not implemented. |

Document Revision History
-------------------------

| Version | Release | Notes |
|---------|---------|-------|
| 1 | 2020-09-30 | - Initial release. |
