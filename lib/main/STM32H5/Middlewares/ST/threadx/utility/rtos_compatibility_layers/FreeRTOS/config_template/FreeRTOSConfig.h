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
/*  09-30-2020      William E. Lamie        Initial Version 6.1           */
/*  10-31-2022      Scott Larson            Change configSTACK_DEPTH_TYPE */
/*                                           to 32 bit instead of 16 bit, */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* #define configENABLE_FPU                         0 */
/* #define configENABLE_MPU                         0 */

/* #define configUSE_PREEMPTION                     1 */
/* #define configSUPPORT_STATIC_ALLOCATION          1 */
/* #define configSUPPORT_DYNAMIC_ALLOCATION         1 */
/* #define configUSE_IDLE_HOOK                      0 */
/* #define configUSE_TICK_HOOK                      0 */
/* #define configCPU_CLOCK_HZ                       (SystemCoreClock) */
#define configTICK_RATE_HZ                         (1000u)
#define configMAX_PRIORITIES                       (32u)
#define configMINIMAL_STACK_SIZE                   (512u)
#define configTOTAL_HEAP_SIZE                      (1024u * 128u)
/* #define configMAX_TASK_NAME_LEN                  (16) */
/* #define configUSE_TRACE_FACILITY                 0 */
#define configUSE_16_BIT_TICKS                      0
/* #define configUSE_MUTEXES                        1 */
/* #define configQUEUE_REGISTRY_SIZE                0 */
/* #define configUSE_RECURSIVE_MUTEXES              1 */
/* #define configUSE_COUNTING_SEMAPHORES            1 */
/* #define configUSE_PORT_OPTIMISED_TASK_SELECTION  0 */

/* #define configMESSAGE_BUFFER_LENGTH_TYPE         size_t */
#define configSTACK_DEPTH_TYPE                     uint32_t

/* #define configUSE_CO_ROUTINES                    0   */
/* #define configMAX_CO_ROUTINE_PRIORITIES          (2) */

/* Software timer definitions. */
/* #define configUSE_TIMERS                         1   */
/* #define configTIMER_TASK_PRIORITY                (2) */
/* #define configTIMER_QUEUE_LENGTH                 10  */
/* #define configTIMER_TASK_STACK_DEPTH             256 */

/* Set the following definitions to 1 to include the API function, or zero
   to exclude the API function. */
/* #define INCLUDE_vTaskPrioritySet             1 */
/* #define INCLUDE_uxTaskPriorityGet            1 */
#define INCLUDE_vTaskDelete                     1  /* Set to 0 to disable task deletion and the idle task. */
/* #define INCLUDE_vTaskCleanUpResources        0 */
/* #define INCLUDE_vTaskSuspend                 1 */
/* #define INCLUDE_vTaskDelayUntil              1 */
/* #define INCLUDE_vTaskDelay                   1 */
/* #define INCLUDE_xTaskGetSchedulerState       1 */
/* #define INCLUDE_xTimerPendFunctionCall       1 */
/* #define INCLUDE_xQueueGetMutexHolder         1 */
/* #define INCLUDE_uxTaskGetStackHighWaterMark  0 */
/* #define INCLUDE_eTaskGetState                1 */

/* Define to a macro invoked to check for invalid arguments. */
#define configASSERT(x)
/* #define configASSERT(x) if ((x) == 0) {taskDISABLE_INTERRUPTS(); for(;;) {};} */

/* Define to a macro invoked on internal assertion failures from within the adaptation layer. */
#define TX_FREERTOS_ASSERT_FAIL()
/* #define TX_FREERTOS_ASSERT_FAIL() {taskDISABLE_INTERRUPTS(); for(;;) {};} */

/* Set to 1 to support auto initialization, see documentation for details. */
#define TX_FREERTOS_AUTO_INIT 0

#endif /* #ifndef FREERTOS_CONFIG_H */
