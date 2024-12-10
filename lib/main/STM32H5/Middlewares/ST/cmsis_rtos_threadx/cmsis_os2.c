/**
  ******************************************************************************
  * @file    cmsis_os2.c
  * @author  MCD Application Team
  * @brief   CMSIS RTOS2 wrapper for AzureRTOS ThreadX
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
/**
  * Important note
  * --------------
  * This file is the implementation of functions to wrap CMSIS RTOS2 onto
  * AzureRTOS ThreadX based on API published by Arm Limited in cmsis_os2.h.
  * The implementation of these functions is inspired from an original work from
  * Arm Limited to wrap CMSIS RTOS2 onto FreeRTOS (see copyright and license
  * information below).
  * The whole contents of this file is a creation by STMicroelectronics licensed
  * to you under the License as specified above. However, some functions
  * originally created by Arm Limited have not been strongly reworked by
  * STMicroelectronics and are still available under their Apache License,
  * Version 2.0 original terms; these original functions are:
  *   - osKernelGetInfo
  *   - osKernelStart
  *   - osKernelGetTickCount
  *   - osKernelGetTickFreq
  *   - osKernelGetSysTimerFreq
  *   - osDelay
  *   - osDelayUntil
  *   - osThreadGetId
  *   - osTimerIsRunning
  */
/* --------------------------------------------------------------------------
 * Copyright (c) 2013-2019 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *      Name:    cmsis_os2.c
 *      Purpose: CMSIS RTOS2 wrapper for AzureRTOS ThreadX
 *
 *---------------------------------------------------------------------------*/

#include <string.h>

/* ::CMSIS:RTOS2 */
#include "cmsis_os2.h"

#include "tx_api.h"
#include "tx_initialize.h"
#include "tx_thread.h"
#include "tx_timer.h"
#include "tx_byte_pool.h"
#include "tx_event_flags.h"
#include "tx_mutex.h"
#include "tx_semaphore.h"
#include "tx_queue.h"
#include "tx_block_pool.h"

/* CMSIS compiler specific defines */
#include "cmsis_compiler.h"

/*---------------------------------------------------------------------------*/

/* Kernel version and identification string definition (major.minor.rev: mmnnnrrrr dec) */
#define KERNEL_VERSION                          (((uint32_t)THREADX_MAJOR_VERSION * 10000000UL) + \
                                                ((uint32_t)THREADX_MINOR_VERSION * 10000UL) + \
                                                ((uint32_t)THREADX_PATCH_VERSION * 1UL))

#define KERNEL_ID                               ("Azure RTOS ThreadX")

#define IS_IRQ_MODE()                           (__get_IPSR() != 0U)

/* Default thread stack size */
#define RTOS2_DEFAULT_THREAD_STACK_SIZE         1024
/* Default thread stack size */
#define RTOS2_INTERNAL_BYTE_POOL_SIZE           256

#ifndef USE_DYNAMIC_MEMORY_ALLOCATION
   #ifndef USE_MEMORY_POOL_ALLOCATION
      #error "CMSIS RTOS ThreadX Wrapper cmsis_os2.c: USE_DYNAMIC_MEMORY_ALLOCATION or USE_MEMORY_POOL_ALLOCATION must be defined"
   #endif
#endif

/* Default stack byte pool memory size */
#ifndef RTOS2_BYTE_POOL_STACK_SIZE
  #define RTOS2_BYTE_POOL_STACK_SIZE              3 * 1024
#endif

/* Default stack byte pool memory size */
#ifndef RTOS2_BYTE_POOL_HEAP_SIZE
  #define RTOS2_BYTE_POOL_HEAP_SIZE               4 * 1024
#endif

/* Default time slice for the created threads */
#ifndef RTOS2_DEFAULT_TIME_SLICE
#define RTOS2_DEFAULT_TIME_SLICE                4
#endif

/* Default stack byte pool memory type */
#define RTOS2_BYTE_POOL_STACK_TYPE              1

/* Default stack byte pool memory type */
#define RTOS2_BYTE_POOL_HEAP_TYPE               2

#ifndef TX_THREAD_USER_EXTENSION
#error "CMSIS RTOS ThreadX Wrapper: TX_THREAD_USER_EXTENSION must be defined as tx_thread_detached_joinable (ULONG) in tx_user.h file"
#endif

#ifdef TX_DISABLE_ERROR_CHECKING
#error "CMSIS RTOS ThreadX Wrapper : TX_DISABLE_ERROR_CHECKING must be undefined"
#endif

/* Ensure the maximum number of priorities is modified by the user to 64. */
#if(TX_MAX_PRIORITIES != 64)
#error "CMSIS RTOS ThreadX Wrapper: TX_MAX_PRIORITIES must be fixed to 64 in tx_user.h file"
#endif
/*---------------------------------------------------------------------------*/

static osKernelState_t KernelState = osKernelInactive;
extern uint32_t SystemCoreClock;

TX_BYTE_POOL HeapBytePool;
TX_BYTE_POOL StackBytePool;
TX_BLOCK_POOL BlockPool;

/*---------------------------------------------------------------------------*/
/*-------------------CMSIS RTOS2 Internal Functions--------------------------*/
/*---------------------------------------------------------------------------*/

/**
  * @brief  The function MemAlloc allocates thread, timer, mutex, semaphore,
  *         event flags and message queue block object memory.
  *         Or it allocates the thread or message queue stack memory.
  * @param  [in] memory size to be allocated from BytePool
  *         [in] to be allocated memory type (Heap or Stack)
  * @retval pointer to the allocated memory or NULL in case of error.
  */
static uint8_t *MemAlloc(uint32_t mem_size, uint8_t pool_type)
{
  /* The output pointer to the allocated memory or NULL in case of error */
  uint8_t *mem_ptr;
  /* Allocated memory size */
  uint32_t allocated_mem_size = mem_size;
  /* Pointer to the BytePool to be used for memory allocation */
  TX_BYTE_POOL *byte_pool;

  /* Check if the memory size is invalid or the BytePool type is wrong */
  if ((mem_size == 0) || (pool_type > RTOS2_BYTE_POOL_HEAP_TYPE))
  {
    /* Return NULL in case of error */
    mem_ptr = NULL;
  }
  else
  {
    /* If the memory size the be allocated is less then the TX_BYTE_POOL_MIN */
    if (allocated_mem_size < TX_BYTE_POOL_MIN)
    {
      /* We should at least allocate TX_BYTE_POOL_MIN */
      allocated_mem_size = TX_BYTE_POOL_MIN;
    }

    /* Assign the BytePool to be used (StackBytePool or HeapBytePool) */
    if (pool_type == RTOS2_BYTE_POOL_STACK_TYPE)
    {
      /* Point to the Stack BytePool */
      byte_pool = &StackBytePool;
    }
    else
    {
      /* Point to the Heap BytePool */
      byte_pool = &HeapBytePool;
    }

    /* Allocate the mem_ptr */
    if (tx_byte_allocate(byte_pool, (void **) &mem_ptr, allocated_mem_size, TX_NO_WAIT) != TX_SUCCESS)
    {
      /* Return NULL in case of error */
      mem_ptr = NULL;
    }
  }

  return (mem_ptr);
}

/**
  * @brief  The function MemFree allocates thread, timer, mutex, semaphore,
  *         event flags and message queue block object memory.
  *         Or it allocates the thread or message queue stack memory.
  * @param  [in] memory size to be allocated from BytePool
  *         [in] to be allocated memory type (Heap or Stack)
  * @retval pointer to the allocated memory or NULL in case of error.
  */
static osStatus_t MemFree(VOID *memory_ptr)
{
  /* The output status code that indicates the execution status */
  osStatus_t status = osOK;

  /* Check if the memory_ptr is invalid */
  if (memory_ptr == NULL)
  {
    /* Return osError in case of error */
    status = osError;
  }
  else
  {
    /* Free the allocated memory_ptr */
    if (tx_byte_release(memory_ptr) != TX_SUCCESS)
    {
      /* Return osError in case of error */
      status = osError;
    }
  }

  return (status);
}

/**
  * @brief  The function MemInit creates memory pools for stack and heap.
  *         The stack pool is used for threads and queues stacks allocations.
  *         The heap pool is used for threads, timers, mutex, semaphores,
  *         message queues and events flags control block object memory allocations.
  *         The size of stack and heap pools are user configured using the
  *         RTOS2_BYTE_POOL_STACK_SIZE and RTOS2_BYTE_POOL_HEAP_SIZE flags.
  * @param  none.
  * @retval status code that indicates the execution status of the function.
  */
static osStatus_t MemInit(void)
{
  /* Allocated memory size */
  uint32_t bytepool_size = RTOS2_BYTE_POOL_STACK_SIZE;
#ifdef USE_DYNAMIC_MEMORY_ALLOCATION
  /* Unused memory address */
  CHAR *unused_memory = NULL;
#else
  #ifdef USE_MEMORY_POOL_ALLOCATION
    CHAR *unused_memory_Stack = NULL;
    CHAR *unused_memory_Heap = NULL;
  #endif
#endif

  /* If the memory size the be allocated is less then the TX_BYTE_POOL_MIN */
  if (bytepool_size < TX_BYTE_POOL_MIN)
  {
    /* We should at least allocate TX_BYTE_POOL_MIN */
    bytepool_size = TX_BYTE_POOL_MIN;
  }
/* Initialize the Heap BytePool address */
#ifdef USE_DYNAMIC_MEMORY_ALLOCATION
  unused_memory = (CHAR *)_tx_initialize_unused_memory;
#elif  USE_MEMORY_POOL_ALLOCATION
  static CHAR freememStack[RTOS2_BYTE_POOL_STACK_SIZE + RTOS2_INTERNAL_BYTE_POOL_SIZE];
  static CHAR freememHeap[RTOS2_BYTE_POOL_HEAP_SIZE + RTOS2_INTERNAL_BYTE_POOL_SIZE];
  unused_memory_Stack = (CHAR *)freememStack;
  unused_memory_Heap = (CHAR *)freememHeap;
#endif

#ifdef USE_DYNAMIC_MEMORY_ALLOCATION
  /* Create a byte memory pool from which to allocate the timer control
     block */
  if (tx_byte_pool_create(&StackBytePool, "Byte Pool Stack", unused_memory,
                          RTOS2_INTERNAL_BYTE_POOL_SIZE + bytepool_size) != TX_SUCCESS)
  {
    /* Return osError in case of error */
    return (osError);
  }
  else
  {
    /* Set the tx_initialize_unused_memory address */
    unused_memory += RTOS2_INTERNAL_BYTE_POOL_SIZE + bytepool_size;
  }

  /* Set bytepool_size to the user configured Heap size */
  bytepool_size = RTOS2_BYTE_POOL_HEAP_SIZE;

  /* If the memory size the be allocated is less then the TX_BYTE_POOL_MIN */
  if (bytepool_size < TX_BYTE_POOL_MIN)
  {
    /* We should at least allocate TX_BYTE_POOL_MIN */
    bytepool_size = TX_BYTE_POOL_MIN;
  }

  /* Create a byte memory pool from which to allocate the timer control
     block */
  if (tx_byte_pool_create(&HeapBytePool, "Byte Pool Heap", unused_memory,
                          RTOS2_INTERNAL_BYTE_POOL_SIZE + bytepool_size) != TX_SUCCESS)
  {
    /* Return osError in case of error */
    return (osError);
  }
  else
  {
    /* Set the tx_initialize_unused_memory address */
    unused_memory += RTOS2_INTERNAL_BYTE_POOL_SIZE + bytepool_size;
  }

  /* Update the _tx_initialize_unused_memory */
  _tx_initialize_unused_memory = unused_memory;

#else
  #ifdef USE_MEMORY_POOL_ALLOCATION
  /* Create a byte memory pool from which to allocate the timer control
     block */
  if (tx_byte_pool_create(&StackBytePool, "Byte Pool Stack", unused_memory_Stack,
                          RTOS2_INTERNAL_BYTE_POOL_SIZE + bytepool_size) != TX_SUCCESS)
  {
    /* Return osError in case of error */
    return (osError);
  }
  /* Set bytepool_size to the user configured Heap size */
  bytepool_size = RTOS2_BYTE_POOL_HEAP_SIZE;

  /* If the memory size the be allocated is less then the TX_BYTE_POOL_MIN */
  if (bytepool_size < TX_BYTE_POOL_MIN)
  {
    /* We should at least allocate TX_BYTE_POOL_MIN */
    bytepool_size = TX_BYTE_POOL_MIN;
  }

  /* Create a byte memory pool from which to allocate the timer control
     block */
  if (tx_byte_pool_create(&HeapBytePool, "Byte Pool Heap", unused_memory_Heap,
                          RTOS2_INTERNAL_BYTE_POOL_SIZE + bytepool_size) != TX_SUCCESS)
  {
    /* Return osError in case of error */
    return (osError);
  }
  #endif
#endif
  return (osOK);
}

/*---------------------------------------------------------------------------*/
/*---------------------------Kenel Management APIs---------------------------*/
/*---------------------------------------------------------------------------*/

/**
  * @brief  The function osKernelInitialize initializes the RTOS Kernel. Before
  *         it is successfully executed, only the functions osKernelGetInfo and
  *         osKernelGetState may be called.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  none
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osKernelInitialize(void)
{
  /* The output status code that indicates the execution status */
  osStatus_t status;

  /* Check if this API is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    /* Return osErrorISR in case of error */
    status = osErrorISR;
  }
  else
  {
    /* Check if the kernel state is osKernelInactive */
    if (KernelState == osKernelInactive)
    {
      /* Initialize the kernel */
      _tx_initialize_kernel_setup();

      /* Initialize the Heap and stack memory BytePools */
      if (MemInit() == osOK)
      {
        /* Set the kernel state to osKernelReady */
        KernelState = osKernelReady;

        /* Return osOK in case of success */
        status = osOK;
      }
      else
      {
        /* Return osError in case of error */
        status = osError;
      }
    }
    else
    {
      /* Return osError in case of error */
      status = osError;
    }
  }

  return (status);
}

/**
  * @brief  The function osKernelGetInfo retrieves the API and kernel version
  *         of the underlying RTOS kernel and a human readable identifier string
  *         for the kernel. It can be safely called before the RTOS is
  *         initialized or started (call to osKernelInitialize or osKernelStart)
  *         Note : This function may be called from Interrupt Service
  *         Routines.
  * @param  [out] version pointer to buffer for retrieving version information.
  *         [out] id_buf  pointer to buffer for retrieving kernel identification
  *               string.
  *         [in]  id_size size of buffer for kernel identification string.
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osKernelGetInfo(osVersion_t *version, char *id_buf, uint32_t id_size)
{
  /* The output status code that indicates the execution status */
  osStatus_t status = osOK;

  /* Check if input version pointer is not NULL */
  if (version != NULL)
  {
    /* Version encoding is major.minor.rev: mmnnnrrrr dec */
    version->api    = KERNEL_VERSION;
    version->kernel = KERNEL_VERSION;
  }
  else
  {
    /* Return osError in case of error */
    status = osError;
  }

  /* Check if input id_buf pointer is not NULL and id_size != 0 */
  if ((id_buf != NULL) && (id_size != 0U))
  {
    if (id_size > sizeof(KERNEL_ID))
    {
      id_size = sizeof(KERNEL_ID);
    }
    memcpy(id_buf, KERNEL_ID, id_size);
  }
  else
  {
    /* Return osError in case of error */
    status = osError;
  }

  return (status);
}

/**
  * @brief  The function osKernelGetState returns the current state of the
  *         kernel and can be safely called before the RTOS is initialized or
  *         started (call to osKernelInitialize or osKernelStart). In case it
  *         fails it will return osKernelError, otherwise it returns the kernel
  *         state (refer to osKernelState_t for the list of kernel states).
  *         Note : This function may be called from Interrupt Service
  *         Routines.
  * @param  none
  * @retval current RTOS Kernel state.
  */
osKernelState_t osKernelGetState(void)
{
  return (KernelState);
}

/**
  * @brief  The function osKernelStart starts the RTOS kernel and begins thread
  *         switching. It will not return to its calling function in case of
  *         success. Before it is successfully executed, only the functions
  *         osKernelGetInfo, osKernelGetState, and object creation functions
  *         (osXxxNew) may be called.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  none
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osKernelStart(void)
{
  /* The output status code that indicates the execution status */
  osStatus_t status;

  /* Check if this API is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    /* Return osErrorISR in case of error */
    status = osErrorISR;
  }
  else
  {
    /* Check if the kernel state is osKernelReady */
    if (KernelState == osKernelReady)
    {
      /* Set the kernel state to osKernelRunning */
      KernelState = osKernelRunning;

      /* Return osOK in case of success */
      status = osOK;

      /* Start the Kernel */
      tx_kernel_enter();
    }
    else
    {
      /* Return osError in case of error */
      status = osError;
    }
  }

  return (status);
}

/**
  * @brief  The function osKernelGetTickCount returns the current RTOS kernel
  *         tick count.
  *         Note : This function may be called from Interrupt Service
  *         Routines.
  * @param  none
  * @retval RTOS kernel current tick count.
  */
uint32_t osKernelGetTickCount(void)
{
  /* The output RTOS kernel current tick count */
  uint32_t ticks;

  /* Get the RTOS kernel current tick count */
  ticks = (uint32_t)tx_time_get();

  return (ticks);
}

/**
  * @brief  The function osKernelGetTickFreq returns the frequency of the
  *         current RTOS kernel tick.
  *         Note : This function may be called from Interrupt Service
  *         Routines.
  * @param  none
  * @retval frequency of the kernel tick in hertz, i.e. kernel ticks per second.
  */
uint32_t osKernelGetTickFreq(void)
{
  return (TX_TIMER_TICKS_PER_SECOND);
}

/**
  * @brief  The function osKernelGetSysTimerCount returns the current RTOS
  *         kernel system timer as a 32-bit value. The value is a rolling 32-bit
  *         counter that is composed of the kernel system interrupt timer value
  *         and the counter that counts these interrupts (RTOS kernel ticks).
  *         This function allows the implementation of very short timeout checks
  *         below the RTOS tick granularity. Such checks might be required when
  *         checking for a busy status in a device or peripheral initialization
  *         routine, see code example below.
  *         Note : This function may be called from Interrupt Service
  *         Routines.
  * @param  none
  * @retval RTOS kernel current system timer count as 32-bit value.
  */
uint32_t osKernelGetSysTimerCount(void)
{
  /* The RTOS kernel current tick count */
  uint32_t ticks;
  /* The output RTOS kernel current system timer count as 32-bit value */
  uint32_t val;

  /* Get the RTOS kernel current tick count */
  ticks = (uint32_t)tx_time_get();

  /* Compute the RTOS kernel current system timer count */
  val = (uint32_t)(ticks * (SystemCoreClock / TX_TIMER_TICKS_PER_SECOND));

  return (val);
}

/**
  * @brief  The function osKernelGetSysTimerFreq returns the frequency of the
  *         current RTOS kernel system timer.
  *         Note : This function may be called from Interrupt Service
  *         Routines.
  * @param  none
  * @retval frequency of the system timer in hertz, i.e. timer ticks per second.
  */
uint32_t osKernelGetSysTimerFreq(void)
{
  return (SystemCoreClock);
}

/*---------------------------------------------------------------------------*/
/*-----------------------------Generic Wait APIs-----------------------------*/
/*---------------------------------------------------------------------------*/

/**
  * @brief  The function osDelay waits for a time period specified in kernel
  *         ticks. For a value of 1 the system waits until the next timer tick
  *         occurs. The actual time delay may be up to one timer tick less than
  *         specified, i.e. calling osDelay(1) right before the next system tick
  *         occurs the thread is rescheduled immediately.
  *         The delayed thread is put into the BLOCKED state and a context
  *         switch occurs immediately. The thread is automatically put back to
  *         the READY state after the given amount of ticks has elapsed. If the
  *         thread will have the highest priority in READY state it will being
  *         scheduled immediately.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  [in]  ticks time ticks value
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osDelay(uint32_t ticks)
{
  /* The output status code that indicates the execution status */
  osStatus_t status;

  /* Check if this API is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    /* Return osErrorISR in case of error */
    status = osErrorISR;
  }
  else
  {
    /* Return osOK in case of success */
    status = osOK;

    /* Check that the input ticks != 0 */
    if (ticks != 0U)
    {
      /* Sleep the thread */
      tx_thread_sleep(ticks);
    }
  }

  return (status);
}

/**
  * @brief  The function osDelayUntil waits until an absolute time (specified
  *         in kernel ticks) is reached.
  *         The corner case when the kernel tick counter overflows is handled by
  *         osDelayUntil. Thus it is absolutely legal to provide a value which
  *         is lower than the current tick value, i.e. returned by
  *         osKernelGetTickCount. Typically as a user you do not have to take
  *         care about the overflow. The only limitation you have to have in
  *         mind is that the maximum delay is limited to (231)-1 ticks.
  *         The delayed thread is put into the BLOCKED state and a context
  *         switch occurs immediately. The thread is automatically put back to
  *         the READY state when the given time is reached. If the thread will
  *         have the highest priority in READY state it will being scheduled
  *         immediately.
  * @param  [in]  ticks absolute time in ticks
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osDelayUntil(uint32_t ticks)
{
  uint32_t tcnt, delay;
  /* The output status code that indicates the execution status */
  osStatus_t status;

  /* Check if this API is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    /* Return osErrorISR in case of error */
    status = osErrorISR;
  }
  else
  {
    /* Return osOK in case of success */
    status = osOK;

    /* Return osOK in case of success */
    tcnt = (uint32_t)tx_time_get();

    /* Determine remaining number of ticks to delay */
    delay = ticks - tcnt;

    /* Check if target tick has not expired */
    if ((delay != 0U) && (0 == (delay >> (8 * sizeof(uint32_t) - 1))))
    {
      /* Sleep the thread */
      tx_thread_sleep(delay);
    }
    else
    {
      /* No delay or already expired */
      status = osErrorParameter;
    }
  }

  return (status);
}

/*---------------------------------------------------------------------------*/
/*--------------------------Thread Management APIs---------------------------*/
/*---------------------------------------------------------------------------*/
/**
  * @brief  The function osThreadNew starts a thread function by adding it to
  *         the list of active threads and sets it to state READY. Arguments for
  *         the thread function are passed using the parameter pointer
  *         *argument. When the priority of the created thread function is
  *         higher than the current RUNNING thread, the created thread function
  *         starts instantly and becomes the new RUNNING thread. Thread
  *         attributes are defined with the parameter pointer attr. Attributes
  *         include settings for thread priority, stack size, or memory
  *         allocation.
  *         The function can be safely called before the RTOS is started
  *         (call to osKernelStart), but not before it is initialized (call to
  *         osKernelInitialize).
  *         The function osThreadNew returns the pointer to the thread object
  *         identifier or NULL in case of an error.
  *         Note : This function Cannot be called from Interrupt Service
  *         Routines.
  * @param  [in]  func  thread function.
  *         [in]  argument  pointer that is passed to the thread function as
  *               start argument.
  *         [in]  attr  thread attributes; NULL: default values.
  * @retval thread ID for reference by other functions or NULL in case of error.
  */
osThreadId_t osThreadNew(osThreadFunc_t func, void *argument, const osThreadAttr_t *attr)
{
  /* For ThreadX the control block pointer is the thread identifier */
  TX_THREAD *thread_ptr = NULL;
  /* Pointer to the thread name */
  CHAR *name_ptr = NULL;
  /* Pointer to start address of the thread stack */
  VOID *stack_start;
  /* The thread stack size */
  ULONG stack_size;
  /* The thread control block size */
  ULONG cb_size;
  /* The thread priority */
  UINT priority;
  /* The thread entry input */
  ULONG entry_input = 0;

  /* Check if this API is called from Interrupt Service Routines
     or the thread_id is NULL */
  if (!IS_IRQ_MODE() && (func != NULL))
  {
    /* Initialize the name_ptr to NULL */
    name_ptr = NULL;

    /* Check if the attr is not NULL */
    if (attr != NULL)
    {
      /* Check if the name_ptr is not NULL */
      if (attr->name != NULL)
      {
        /* Set the thread name_ptr */
        name_ptr = (CHAR *)attr->name;
      }

      /* Check the input priority value and attribute bits for osThreadJoinable
         parameter */
      if ((attr->priority < osPriorityIdle) || (attr->priority > osPriorityISR))
      {
        /* Return NULL pointer in case of error */
        return (NULL);
      }
      else
      {
        /* Set the thread priority */
        priority = osPriorityISR - attr->priority;
      }

      /* Check if the argument is not NULL */
      if (argument != NULL)
      {
        /* Set the entry_input */
        entry_input = (ULONG) argument;
      }

      /* Check if the stack size is equal to 0 */
      if (attr->stack_size == 0U)
      {
        /* Set stack size to DEFAULT_THREAD_STACK_SIZE */
        stack_size = RTOS2_DEFAULT_THREAD_STACK_SIZE;
      }
      else if (attr->stack_size < TX_BYTE_POOL_MIN)
      {
        /* Set stack size to TX_BYTE_POOL_MIN */
        stack_size = TX_BYTE_POOL_MIN;
      }
      else
      {
        /* Set stack size to attr->stack_size */
        stack_size = (ULONG)attr->stack_size;
      }

      /* Check if the input stack pointer is NULL */
      if (attr->stack_mem == NULL)
      {
        /* Allocate the stack for the thread to be created */
        stack_start = MemAlloc(stack_size, RTOS2_BYTE_POOL_STACK_TYPE);
        if (stack_start == NULL)
        {
          /* Return NULL pointer in case of error */
          return (NULL);
        }
      }
      else
      {
        if (attr->stack_size == 0U)
        {
          /* Return NULL pointer in case of error */
          return (NULL);
        }
        else
        {
          /* Set stack size to the input attr->stack_size */
          stack_size = (ULONG)attr->stack_size;
        }

        /* The stack shall point to the input stack memory address */
        stack_start = attr->stack_mem;
      }

      /* Check if the control block size is equal to 0 */
      if (attr->cb_size == 0U)
      {
        /* Set control block size to sizeof(TX_THREAD) */
        cb_size = sizeof(TX_THREAD);
      }
      else if (attr->cb_size < sizeof(TX_THREAD))
      {
        /* Return NULL pointer in case of error */
        return (NULL);
      }
      else
      {
        /* Set stack size to attr->cb_size */
        cb_size = (ULONG)attr->cb_size;
      }

      /* Check if the input control block pointer is NULL */
      if (attr->cb_mem == NULL)
      {
        /* Allocate the thread_ptr structure for the thread to be created */
        thread_ptr = (TX_THREAD *)MemAlloc(cb_size, RTOS2_BYTE_POOL_HEAP_TYPE);
        if (thread_ptr == NULL)
        {
          /* Check if the memory for thread stack has been internally
             allocated */
          if (attr->stack_mem == NULL)
          {
            /* Free the already allocated memory for thread stack */
            MemFree(stack_start);
          }
          /* Return NULL pointer in case of error */
          return (NULL);
        }
      }
      else
      {
        /* The control block shall point to the input cb_mem memory address */
        thread_ptr = attr->cb_mem;
      }
    }
    else
    {
      /* Set the thread priority to default osPriorityNormal*/
      priority = osPriorityISR - osPriorityNormal;

      /* Initialize the name_ptr to NULL */
      name_ptr = NULL;

      /* Initialize the stack_size to RTOS2_DEFAULT_THREAD_STACK_SIZE */
      stack_size = RTOS2_DEFAULT_THREAD_STACK_SIZE;

      /* Check if the argument is not NULL */
      if (argument != NULL)
      {
        /* Set the entry_input */
        entry_input = (ULONG) argument;
      }

      /* Allocate the stack for the thread to be created */
      stack_start = MemAlloc(stack_size, RTOS2_BYTE_POOL_STACK_TYPE);
      if (stack_start == NULL)
      {
        /* Return NULL pointer in case of error */
        return (NULL);
      }

      /* Allocate the thread_ptr structure for the thread to be created */
      thread_ptr = (TX_THREAD *)MemAlloc(sizeof(TX_THREAD), RTOS2_BYTE_POOL_HEAP_TYPE);
      if (thread_ptr == NULL)
      {
        /* Free the already allocated memory for thread stack */
        MemFree(stack_start);

        /* Return NULL pointer in case of error */
        return (NULL);
      }
    }

    /* Call the tx_thread_create function to create the new thread.
       Note: By default the preempt_threshold shall be deactivated by setting
       its value to the priority or deactivated using
       TX_DISABLE_PREEMPTION_THRESHOLD */
    if (tx_thread_create(thread_ptr, name_ptr, (void(*)(ULONG))func, entry_input, stack_start, stack_size, priority,
                         priority, RTOS2_DEFAULT_TIME_SLICE, TX_AUTO_START) != TX_SUCCESS)
    {
      /* Check if the memory for thread control block has been internally
         allocated */
      if ((attr->cb_mem == NULL) || (attr == NULL))
      {
        /* Free the already allocated memory for thread control block */
        MemFree(thread_ptr);
      }

      /* Check if the memory for thread stack has been internally allocated */
      if ((attr->stack_mem == NULL) || (attr == NULL))
      {
        /* Free the already allocated memory for thread stack */
        MemFree(stack_start);
      }

      /* Return NULL pointer in case of error */
      thread_ptr = NULL;
    }
    else
    {
      /* Check if the thread shall be created joinable */
      if ((attr != NULL) && (attr->attr_bits == osThreadJoinable))
      {
        /* Set the thread to Joinable state */
        thread_ptr->tx_thread_detached_joinable = osThreadJoinable;
      }
      else
      {
        /* Set the thread to Detached state */
        thread_ptr->tx_thread_detached_joinable = osThreadDetached;
      }
    }
  }

  return ((osThreadId_t)thread_ptr);
}

/**
  * @brief  The function osThreadGetName returns the pointer to the name_ptr
  *         string of the thread identified by parameter thread_id or NULL in
  *         case of an error.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  [in] thread_id thread ID obtained by osThreadNew or osThreadGetId
  * @retval name_ptr as null-terminated string.
  */
const char *osThreadGetName(osThreadId_t thread_id)
{
  /* For ThreadX the control block pointer is the thread identifier */
  TX_THREAD *thread_ptr = (TX_THREAD *)thread_id;
  /* The output name_ptr as null-terminated string */
  CHAR *name_ptr = NULL;

  /* Check if this API is called from Interrupt Service Routines, the thread_id
     is NULL or thread_id->tx_thread_id != TX_THREAD_ID */
  if (IS_IRQ_MODE() || (thread_ptr == NULL) || (thread_ptr->tx_thread_id != TX_THREAD_ID))
  {
    /* Return NULL in case of an error */
    name_ptr = NULL;
  }
  else
  {
    /* Call the tx_thread_info_get to get the thread name_ptr */
    if (tx_thread_info_get(thread_ptr, &name_ptr, NULL, NULL, NULL, NULL, NULL, NULL, NULL) != TX_SUCCESS)
    {
      /* Return NULL in case of an error */
      name_ptr = NULL;
    }
  }

  return (name_ptr);
}

/**
  * @brief  The function osThreadGetId returns the thread object ID of the
  *         currently running thread or NULL in case of an error.
  *         Note : This function may be called from Interrupt Service Routines.
  * @param  none
  * @retval thread ID for reference by other functions or NULL in case of error.
  */
osThreadId_t osThreadGetId(void)
{
  /* For ThreadX the control block pointer is the thread identifier */
  osThreadId_t thread_id;

  /* Call the tx_thread_identify to get the control block pointer of the
     currently executing thread. */
  thread_id = (osThreadId_t)tx_thread_identify();

  return (thread_id);
}

/**
  * @brief  The function osThreadGetState returns the state of the thread
  *         identified by parameter thread_id. In case it fails or if it is
  *         called from an ISR, it will return osThreadError, otherwise it
  *         returns the thread state (refer to osThreadState_t for the list of
  *         thread states).
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  [in] thread_id thread ID obtained by osThreadNew or osThreadGetId
  * @retval state current thread state of the specified thread.
  */
osThreadState_t osThreadGetState(osThreadId_t thread_id)
{
  /* For ThreadX the control block pointer is the thread identifier */
  TX_THREAD *thread_ptr = (TX_THREAD *)thread_id;
  /* The control block pointer of the current running thread */
  TX_THREAD *current_thread = NULL;
  /* The current thread state of the specified thread. */
  osThreadState_t state;
  /* The current thread state of the specified thread as specified by threadx */
  UINT threadx_state;

  /* Check if this API is called from Interrupt Service Routines, the thread_id
     is NULL or thread_id->tx_thread_id != TX_THREAD_ID */
  if (IS_IRQ_MODE() || (thread_ptr == NULL) || (thread_ptr->tx_thread_id != TX_THREAD_ID))
    /* Return osThreadError in case of an error */
  {
    state = osThreadError;
  }
  else
  {
    /* Get the current running thread */
    TX_THREAD_GET_CURRENT(current_thread);
    if (current_thread == thread_id)
    {
      /* The state is running */
      state = osThreadRunning;
    }
    else
    {
      /* Call the tx_thread_info_get to get the thread threadx_state */
      if (tx_thread_info_get(thread_ptr, NULL, &threadx_state, NULL, NULL, NULL, NULL, NULL, NULL) != TX_SUCCESS)
      {
        /* Return osThreadError in case of an error */
        state = osThreadError;
      }
      else
      {
        /* Link the ThreadX thread states to CMSIS RTOS2 states */
        switch (threadx_state)
        {
          /* The thread is in READY state */
          case TX_READY:
          {
            state = osThreadReady;
            break;
          }
          /* The thread is in COMPLETED state */
          case TX_COMPLETED:
          {
            state = osThreadTerminated;
            break;
          }
          /* The thread is in TERMINATED state */
          case TX_TERMINATED:
          {
            state = osThreadTerminated;
            break;
          }
          /* The thread is in SUSPENDED state */
          case TX_SUSPENDED:
          case TX_QUEUE_SUSP:
          case TX_SEMAPHORE_SUSP:
          case TX_EVENT_FLAG:
          case TX_BLOCK_MEMORY:
          case TX_BYTE_MEMORY:
          case TX_IO_DRIVER:
          case TX_FILE:
          case TX_TCP_IP:
          case TX_MUTEX_SUSP:
          case TX_PRIORITY_CHANGE:
          {
            state = osThreadBlocked;
            break;
          }
          /* The thread is in SLEEP state */
          case TX_SLEEP:
          {
            state = osThreadBlocked;
            break;
          }
          /* The thread is in unknown state */
          default:
          {
            state = osThreadError;
            break;
          }
        }
      }
    }
  }

  return (state);
}

/**
  * @brief  The function osThreadGetStackSize returns the stack size of the
  *         thread specified by parameter thread_id. In case of an error, it
  *         returns 0.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  [in] thread_id thread ID obtained by osThreadNew or osThreadGetId.
  * @retval remaining_stack_space remaining stack space in bytes.
  */
uint32_t osThreadGetStackSize(osThreadId_t thread_id)
{
  /* For ThreadX the control block pointer is the thread identifier */
  TX_THREAD *thread_ptr = (TX_THREAD *)thread_id;
  /* The specific thread stack size in bytes */
  unsigned int stack_size;

  /* Check if this API is called from Interrupt Service Routines, the thread_id
     is NULL or thread_id->tx_thread_id != TX_THREAD_ID */
  if (IS_IRQ_MODE() || (thread_ptr == NULL) || (thread_ptr->tx_thread_id != TX_THREAD_ID))
  {
    /* Return 0 in case of error */
    stack_size = 0U;
  }
  else
  {
    /* The stack_size get the allocated thread stack size in the thread creation step */
    stack_size = thread_ptr->tx_thread_stack_size;
  }

  return (stack_size);
}

/**
  * @brief  The function osThreadGetStackSpace returns the size of unused stack
  *         space for the thread specified by parameter thread_id. Stack
  *         watermark recording during execution needs to be enabled (refer to
  *         Thread Configuration). In case of an error, it returns 0.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  [in] thread_id thread ID obtained by osThreadNew or osThreadGetId.
  * @retval remaining_stack_space remaining stack space in bytes.
  */
uint32_t osThreadGetStackSpace(osThreadId_t thread_id)
{
  /* For ThreadX the control block pointer is the thread identifier */
  TX_THREAD *thread_ptr = (TX_THREAD *)thread_id;
  /* Remaining stack space in bytes */
  uint32_t remaining_stack_space;

  /* Check if this API is called from Interrupt Service Routines, the thread_id
     is NULL or thread_id->tx_thread_id != TX_THREAD_ID */
  if (IS_IRQ_MODE() || (thread_ptr == NULL) || (thread_ptr->tx_thread_id != TX_THREAD_ID))
  {
    /* Return 0 in case of error */
    remaining_stack_space = 0U;
  }
  else
  {
    /* Compute the remaining free stack size for the given thread */
    remaining_stack_space = (unsigned int)((CHAR *)thread_ptr->tx_thread_stack_ptr -
                                           (CHAR *)thread_ptr->tx_thread_stack_start);
  }

  return (remaining_stack_space);
}

/**
  * @brief  The function osThreadSetPriority changes the priority of an active
  *         thread specified by the parameter thread_id to the priority
  *         specified by the parameter priority.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  [in] thread_id thread ID obtained by osThreadNew or osThreadGetId.
  *         [in] priority new priority value for the thread function.
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osThreadSetPriority(osThreadId_t thread_id, osPriority_t priority)
{
  /* For ThreadX the control block pointer is the thread identifier */
  TX_THREAD *thread_ptr = (TX_THREAD *)thread_id;
  /* Old priority */
  UINT old_priority;
  /* The returned status or error */
  osStatus_t status;

  /* Check if this API is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    /* Return osErrorISR error */
    status = osErrorISR;
  }
  /* Check if thread_ptr is NULL or thread_id->tx_thread_id != TX_THREAD_ID or
     the input priority is out of range */
  else if ((thread_ptr == NULL) || (thread_ptr->tx_thread_id != TX_THREAD_ID) || (priority < osPriorityIdle)
           || (priority > osPriorityISR))
  {
    /* Return osErrorParameter error */
    status = osErrorParameter;
  }
  else
  {
    /* Convert input CMSIS osPriority_t to threadX priority */
    priority = (osPriority_t)(osPriorityISR - priority);

    /* Call the tx_thread_priority_change to change the thread priority */
    if (tx_thread_priority_change(thread_ptr, priority, &old_priority) == TX_SUCCESS)
    {
      /* Return osOK in case of success */
      status = osOK;
    }
    else
    {
      /* Return osErrorResource in case of error */
      status = osErrorResource;
    }
  }

  return (status);
}

/**
  * @brief  The function osThreadGetPriority returns the priority of an active
  *         thread specified by the parameter thread_id.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  [in] thread_id thread ID obtained by osThreadNew or osThreadGetId.
  * @retval priority current priority value of the specified thread.
  */
osPriority_t osThreadGetPriority(osThreadId_t thread_id)
{
  /* For ThreadX the control block pointer is the thread identifier */
  TX_THREAD *thread_ptr = (TX_THREAD *)thread_id;
  /* The returned thread priority or error */
  osPriority_t priority;

  /* Check if this API is called from Interrupt Service Routines
     or the thread_id is NULL */
  if (IS_IRQ_MODE() || (thread_ptr == NULL))
  {
    /* Return osPriorityError in case of an error */
    priority = osPriorityError;
  }
  else
  {
    /* Call the tx_thread_info_get to get the thread priority */
    if (tx_thread_info_get(thread_ptr, NULL, NULL, NULL, (UINT *)&priority, NULL, NULL, NULL, NULL) != TX_SUCCESS)
    {
      /* Return osPriorityError in case of an error */
      priority = osPriorityError;
    }
    else
    {
      /* Convert the threadX priority to CMSIS osPriority_t */
      priority = (osPriority_t)(osPriorityISR - priority);
    }
  }

  return (priority);
}

/**
  * @brief  The function osThreadYield passes control to the next thread with
  *         the same priority that is in the READY state. If there is no other
  *         thread with the same priority in state READY, then the current
  *         thread continues execution and no thread switch occurs.
  *         osThreadYield does not set the thread to state BLOCKED. Thus no
  *         thread with a lower priority will be scheduled even if threads in
  *         state READY are available.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  none.
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osThreadYield(void)
{
  /* The returned status or error */
  osStatus_t status;

  /* Check if this API is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    /* Return osErrorISR error */
    status = osErrorISR;
  }
  else
  {
    /* Call the tx_thread_relinquish to relinquishes processor control to
       other ready-to-run threads at the same or higher priority. */
    tx_thread_relinquish();

    /* Return osOK for success */
    status = osOK;
  }

  return (status);
}

/**
  * @brief  The function osThreadSuspend suspends the execution of the thread
  *         identified by parameter thread_id. The thread is put into the
  *         BLOCKED state (osThreadBlocked). Suspending the running thread will
  *         cause a context switch to another thread in READY state immediately.
  *         The suspended thread is not executed until explicitly resumed with
  *         the function osThreadResume.
  *         Threads that are already BLOCKED are removed from any wait list and
  *         become ready when they are resumed. Thus it is not recommended to
  *         suspend an already blocked thread.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  [in] thread_id thread ID obtained by osThreadNew or osThreadGetId.
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osThreadSuspend(osThreadId_t thread_id)
{
  /* For ThreadX the control block pointer is the thread identifier */
  TX_THREAD *thread_ptr = (TX_THREAD *)thread_id;
  /* The returned status or error */
  osStatus_t status;

  /* Check if this API is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    /* Return osErrorISR error */
    status = osErrorISR;
  }
  /* Check if the thread ID is NULL or (tx_thread_id != TX_THREAD_ID) */
  else if ((thread_id == NULL) || (thread_ptr->tx_thread_id != TX_THREAD_ID))
  {
    /* Return osErrorParameter error */
    status = osErrorParameter;
  }
  else
  {
    /* Call the tx_thread_suspend to suspends the specified application
      thread. A thread may call this service to suspend itself. */
    if (tx_thread_suspend(thread_ptr) == TX_SUCCESS)
    {
      /* Return osOK for success */
      status = osOK;
    }
    else
    {
      /* Return osErrorResource in case of error */
      status = osErrorResource;
    }
  }

  return (status);
}

/**
  * @brief  The function osThreadResume puts the thread identified by parameter
  *         thread_id (which has to be in BLOCKED state) back to the READY
  *         state. If the resumed thread has a higher priority than the running
  *         thread a context switch occurs immediately.
  *         The thread becomes ready regardless of the reason why the thread was
  *         blocked. Thus it is not recommended to resume a thread not suspended
  *         by osThreadSuspend.
  *         Functions that will put a thread into BLOCKED state are:
  *         osEventFlagsWait and osThreadFlagsWait, osDelay and osDelayUntil,
  *         osMutexAcquire and osSemaphoreAcquire, osMessageQueueGet,
  *         osMemoryPoolAlloc, osThreadJoin, osThreadSuspend..
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  [in] thread_id thread ID obtained by osThreadNew or osThreadGetId.
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osThreadResume(osThreadId_t thread_id)
{
  /* For ThreadX the control block pointer is the thread identifier */
  TX_THREAD *thread_ptr = (TX_THREAD *)thread_id;
  /* The returned status or error */
  osStatus_t status;

  /* Check if this API is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    /* Return osErrorISR error */
    status = osErrorISR;
  }
  /* Check if the thread ID is NULL or (tx_thread_id != TX_THREAD_ID) */
  else if ((thread_id == NULL) || (thread_ptr->tx_thread_id != TX_THREAD_ID))
  {
    /* Return osErrorParameter error */
    status = osErrorParameter;
  }
  else
  {
    /* Call the tx_thread_resume to resumes or prepares for execution a thread
      that was previously suspended by a tx_thread_suspend call. In addition,
      this service resumes threads that were created without an automatic
      start. */
    if (tx_thread_resume(thread_ptr) == TX_SUCCESS)
    {
      /* Return osOK for success */
      status = osOK;
    }
    else
    {
      /* Return osErrorResource in case of error */
      status = osErrorResource;
    }
  }

  return (status);
}

/**
  * @brief  The function osThreadExit terminates the calling thread. This allows
  *         the thread to be synchronized with osThreadJoin.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  none.
  * @retval none.
  */
__NO_RETURN void osThreadExit(void)
{
  /* For ThreadX the control block pointer is the thread identifier */
  TX_THREAD *thread_ptr = NULL;

  /* Check if this API is called from Interrupt Service Routines */
  if (!IS_IRQ_MODE())
  {
    /* Call the tx_thread_identify to get the control block pointer of the
      currently executing thread. */
    thread_ptr = tx_thread_identify();

    /* Check if the current running thread pointer is not NULL */
    if (thread_ptr != NULL)
    {
      /* Call the tx_thread_terminate to terminates the specified application
         thread regardless of whether the thread is suspended or not. A thread
         may call this service to terminate itself. */
      tx_thread_terminate(thread_ptr);
    }
  }
  /* Infinite loop */
  for (;;);
}

/**
  * @brief  The function osThreadTerminate removes the thread specified by
  *         parameter thread_id from the list of active threads. If the
  *         thread is currently RUNNING, the thread terminates and the
  *         execution continues with the next READY thread. If no such thread
  *         exists, the function will not terminate the running thread, but
  *         return osErrorResource.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  [in] thread_id thread ID obtained by osThreadNew or osThreadGetId.
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osThreadTerminate(osThreadId_t thread_id)
{
  /* For ThreadX the control block pointer is the thread identifier */
  TX_THREAD *thread_ptr = (TX_THREAD *)thread_id;
  /* The returned status or error */
  osStatus_t status;

  /* Check if this API is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    /* Return osErrorISR error */
    status = osErrorISR;
  }
  /* Check if the thread ID is NULL or (tx_thread_id != TX_THREAD_ID) */
  else if ((thread_id == NULL) || (thread_ptr->tx_thread_id != TX_THREAD_ID))
  {
    /* Return osErrorParameter error */
    status = osErrorParameter;
  }
  else
  {
    /* Call the tx_thread_terminate to terminates the specified application
      thread regardless of whether the thread is suspended or not. A thread
      may call this service to terminate itself. */
    if (tx_thread_terminate(thread_ptr) == TX_SUCCESS)
    {
      /* Free the thread resources if it is Detached */
      if (thread_ptr->tx_thread_detached_joinable == osThreadDetached)
      {
        /* Free the already allocated memory for thread stack */
        MemFree(thread_ptr->tx_thread_stack_start);

        /* Free the already allocated memory for thread control block */
        MemFree(thread_ptr);
      }

      /* Return osOK for success */
      status = osOK;
    }
    else
    {
      /* Return osErrorResource in case of error */
      status = osErrorResource;
    }
  }

  return (status);
}

/**
  * @brief  The function osThreadDetach changes the attribute of a thread
  *         (specified by thread_id) to osThreadDetached. Detached threads are
  *         not joinable with osThreadJoin. When a detached thread is
  *         terminated, all resources are returned to the system. The behavior
  *         of osThreadDetach on an already detached thread is undefined.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  [in] thread_id thread ID obtained by osThreadNew or osThreadGetId.
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osThreadDetach(osThreadId_t thread_id)
{
  /* For ThreadX the control block pointer is the thread identifier */
  TX_THREAD *thread_ptr = (TX_THREAD *)thread_id;
  /* The returned status or error */
  osStatus_t status;

  /* Check if this API is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    /* Return osErrorISR error */
    status = osErrorISR;
  }
  /* Check if the thread ID is NULL or (tx_thread_id != TX_THREAD_ID) */
  else if ((thread_id == NULL) || (thread_ptr->tx_thread_id != TX_THREAD_ID))
  {
    /* Return osErrorParameter error */
    status = osErrorParameter;
  }
  else
  {
    /* Change the status of the specific thread to Detached */
    thread_ptr->tx_thread_detached_joinable = osThreadDetached;

    /* Return osOK for success */
    status = osOK;
  }

  return (status);
}


/**
  * @brief  The function osThreadJoin waits for the thread specified by
  *         thread_id to terminate. If that thread has already terminated, then
  *         osThreadJoin returns immediately. The thread must be joinable.
  *         By default threads are created with the attribute osThreadDetached.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  [in] thread_id thread ID obtained by osThreadNew or osThreadGetId.
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osThreadJoin(osThreadId_t thread_id)
{
  /* For ThreadX the control block pointer is the thread identifier */
  TX_THREAD *thread_ptr = (TX_THREAD *)thread_id;
  /* The control block pointer of the current running thread */
  TX_THREAD *current_thread = NULL;
  /* The current thread state of the specified thread as specified by threadx */
  UINT threadx_state;
  /* The returned status or error */
  osStatus_t status;

  /* Check if this API is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    /* Return osErrorISR error */
    status = osErrorISR;
  }
  /* Check if the thread ID is NULL or (tx_thread_id != TX_THREAD_ID) */
  else if ((thread_id == NULL) || (thread_ptr->tx_thread_id != TX_THREAD_ID))
  {
    /* Return osErrorParameter error */
    status = osErrorParameter;
  }
  else
  {
    /* Get the current running thread */
    TX_THREAD_GET_CURRENT(current_thread);
    if (current_thread == thread_id)
    {
      /* Return osErrorParameter error */
      status = osErrorParameter;
    }
    else
    {
      /* Call the tx_thread_info_get to get the thread threadx_state */
      if (tx_thread_info_get(thread_ptr, NULL, &threadx_state, NULL, NULL, NULL, NULL, NULL, NULL) == TX_SUCCESS)
      {
        /* Check if the thread is joinable */
        if (thread_ptr->tx_thread_detached_joinable == osThreadJoinable)
        {
          /* Only one thread can Join a thread in the same time so we will
             detach it momentally until it will be terminated */
          thread_ptr->tx_thread_detached_joinable = osThreadDetached;

          /* Wait until the state of the specific thread turn to terminated */
          while ((threadx_state != TX_TERMINATED) && (threadx_state != TX_COMPLETED))
          {
            /* Call the tx_thread_info_get to get the thread threadx_state */
            tx_thread_info_get(thread_ptr, NULL, &threadx_state, NULL, NULL, NULL, NULL, NULL, NULL);
          }
          /* Once the thread is terminated it can be again Joinable */
          thread_ptr->tx_thread_detached_joinable = osThreadJoinable;

          /* Return osOK for success */
          status = osOK;
        }
        else
        {
          /* Return osErrorResource error */
          status = osErrorResource;
        }
      }
      else
      {
        /* Return osErrorResource error */
        status = osErrorResource;
      }
    }
  }

  return (status);
}

/**
  * @brief  The function osThreadGetCount returns the number of active threads
  *         or 0 in case of an error.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  none.
  * @retval count number of active threads.
  */
uint32_t osThreadGetCount(void)
{
  /* For ThreadX the control block pointer is the thread identifier */
  TX_THREAD *thread_ptr = NULL;
  /* Returned number of active threads */
  unsigned int count = 0U;

  /* Check if this API is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    /* Return 0 in case of error */
    count = 0;
  }
  else
  {
    /* thread_ptr = the head pointer of the created thread list */
    thread_ptr = _tx_thread_created_ptr;

    /* Compute the number of active threads */
    while (thread_ptr != NULL)
    {
      /* Check the next thread validity */
      if (thread_ptr->tx_thread_id == TX_THREAD_ID)
      {
        /* Increment the number of active threads */
        count++;

        /* Check the state of next created thread */
        if ((thread_ptr->tx_thread_created_next != thread_ptr)
            && (thread_ptr->tx_thread_created_next != _tx_thread_created_ptr))
        {
          thread_ptr = thread_ptr->tx_thread_created_next;
        }
        else
        {
          break;
        }
      }
      else
      {
        /* Return 0 in case of error */
        count = 0;
        break;
      }
    }
  }

  return (count);
}

/**
  * @brief  The function osThreadEnumerate returns the number of enumerated
  *         threads or 0 in case of an error.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  [out] thread_array  pointer to array for retrieving thread IDs.
  *         [in]  array_items maximum number of items in array for retrieving
  *               thread IDs.
  * @retval count number of enumerated threads.
  */
uint32_t osThreadEnumerate(osThreadId_t *thread_array, uint32_t array_items)
{
  /* For ThreadX the control block pointer is the thread identifier */
  TX_THREAD *thread_ptr = NULL;
  /* The number of enumerated threads */
  uint32_t count = 0U;

  /* Check if this API is called from Interrupt Service Routines or
     if the thread_array is NULL or the array_items is equal to 0 */
  if (IS_IRQ_MODE() || (thread_array == NULL) || (array_items == 0U))
  {
    /* Return 0 in case of error */
    count = 0U;
  }
  else
  {
    /* thread_ptr point to the head pointer of the created thread list */
    thread_ptr = _tx_thread_created_ptr;

    /* Compute the number of active threads */
    while ((thread_ptr != NULL) && (count < array_items))
    {
      /* Check the next thread validity */
      if (thread_ptr->tx_thread_id == TX_THREAD_ID)
      {
        /* Retrieve the thread IDs and increment the number of active
           threads */
        thread_array[count++] = thread_ptr;

        /* Check the state of next created thread */
        if ((thread_ptr->tx_thread_created_next != thread_ptr)
            && (thread_ptr->tx_thread_created_next != _tx_thread_created_ptr))
        {
          thread_ptr = thread_ptr->tx_thread_created_next;
        }
        else
        {
          break;
        }
      }
      else
      {
        /* Return 0 in case of error */
        count = 0;
        break;
      }
    }
  }

  return (count);
}

/*---------------------------------------------------------------------------*/
/*---------------------------Timer Management APIs---------------------------*/
/*---------------------------------------------------------------------------*/

/**
  * @brief  The function osTimerNew creates an one-shot or periodic timer and
  *         associates it with a callback function with argument. The timer is
  *         in stopped state until it is started with osTimerStart.
  *         The function can be safely called before the RTOS is started (call
  *         to osKernelStart), but not before it is initialized (call to
  *         osKernelInitialize).
  *         The function osTimerNew returns the pointer to the timer object
  *         identifier or NULL in case of an error.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  [in] func function pointer to callback function.
  *         [in] type osTimerOnce for one-shot or osTimerPeriodic for periodic
  *              behavior.
  *         [in] argument argument to the timer callback function.
  *         [in] attr timer attributes; NULL: default values.
  * @retval timer ID for reference by other functions or NULL in case of error.
  */
osTimerId_t osTimerNew(osTimerFunc_t func, osTimerType_t type, void *argument, const osTimerAttr_t *attr)
{
  /* For TX_TIMER the control block pointer is the timer identifier */
  TX_TIMER *timer_ptr = NULL;
  /* The name_ptr as null-terminated string */
  CHAR *name_ptr = NULL;
  /* The timer expiration input */
  ULONG expiration_input = 0U;
  /* The timer reschedule ticks */
  ULONG reschedule_ticks = 0U;
  /* The size of control block */
  ULONG cb_size = sizeof(TX_TIMER);

  /* Check if this API is called from Interrupt Service Routines,
     the timer callback function handler is NULL or the type is not valid */
  if (!IS_IRQ_MODE() && (func != NULL) && (type <= osTimerPeriodic))
  {
    /* Initialize the name_ptr to NULL */
    name_ptr = NULL;

    /* Check if the attr is not NULL */
    if (attr != NULL)
    {
      /* Check if the name_ptr is not NULL */
      if (attr->name != NULL)
      {
        /* Set the timer name_ptr */
        name_ptr = (CHAR *)attr->name;
      }

      /* Check if the control block size is equal to 0 */
      if (attr->cb_size == 0U)
      {
        /* Set control block size to sizeof(TX_TIMER) */
        cb_size = sizeof(TX_TIMER);
      }
      else if (attr->cb_size < sizeof(TX_TIMER))
      {
        /* Return NULL pointer in case of error */
        return (NULL);
      }
      else
      {
        /* Set control block size to attr->cb_size */
        cb_size = (ULONG) attr->cb_size;
      }

      /* Check if the input control block pointer is NULL */
      if (attr->cb_mem == NULL)
      {
        /* Allocate the timer_ptr structure for the timer to be created */
        timer_ptr = (TX_TIMER *)MemAlloc(cb_size, RTOS2_BYTE_POOL_HEAP_TYPE);
        if (timer_ptr == NULL)
        {
          /* Return NULL pointer in case of error */
          return (NULL);
        }
      }
      else
      {
        /* The control block shall point to the input cb_mem memory address */
        timer_ptr = attr->cb_mem;
      }
    }
    else
    {
      /* Allocate the timer_ptr structure for the timer to be created */
      timer_ptr = (TX_TIMER *)MemAlloc(cb_size, RTOS2_BYTE_POOL_HEAP_TYPE);
    }

    /* Check the timer type to set timer periodicity */
    if (type == osTimerPeriodic)
    {
      /* Specifies the number of ticks for all timer expirations after the
         first. A zero for this parameter makes the timer a one-shot timer.
         Otherwise, for periodic timers, legal values range from 1 through
         0xFFFFFFFF.
         For periodic timer the reschedule_ticks is initiated to 1 then set to ticks
         given as input in osTimerStart APIs */
      reschedule_ticks = 1;
    }

    /* Check if the argument is not NULL */
    if (argument != NULL)
    {
      /* Set the expiration_input */
      expiration_input = (ULONG)argument;
    }

    /* Call the tx_timer_create function to create the new timer */
    if (tx_timer_create(timer_ptr, name_ptr, (void(*)(ULONG))func, expiration_input, 1, reschedule_ticks,
                        TX_NO_ACTIVATE) != TX_SUCCESS)
    {
      /* Check if the memory for timer control block has been internally
         allocated */
      if ((attr->cb_mem == NULL) || (attr == NULL))
      {
        /* Free the already allocated memory for timer control block */
        MemFree(timer_ptr);
      }

      /* Return NULL pointer in case of error */
      timer_ptr = NULL;
    }
  }

  return ((osThreadId_t)timer_ptr);
}

/**
  * @brief  The function osTimerGetName returns the pointer to the name string
  *         of the timer identified by parameter timer_id or NULL in case of an
  *         error.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  [in] timer_id timer ID obtained by osTimerNew.
  * @retval name as null-terminated string.
  */
const char *osTimerGetName(osTimerId_t timer_id)
{
  /* For TX_TIMER the control block pointer is the timer identifier */
  TX_TIMER *timer_ptr = (TX_TIMER *)timer_id;
  /* The output name_ptr as null-terminated string */
  CHAR *name_ptr;

  /* Check if this API is called from Interrupt Service Routines
     or the timer_id is NULL */
  if (IS_IRQ_MODE() || (timer_ptr == NULL))
  {
    /* Return NULL in case of an error */
    name_ptr = NULL;
  }
  else
  {
    /* Call the tx_timer_info_get to get the timer name_ptr */
    if (tx_timer_info_get(timer_ptr, &name_ptr, NULL, NULL, NULL, NULL) != TX_SUCCESS)
    {
      /* Return NULL in case of an error */
      name_ptr = NULL;
    }
  }

  return ((const char *)name_ptr);
}

/**
  * @brief  The function osTimerIsRunning checks whether a timer specified by
  *         parameter timer_id is running. It returns 1 if the timer is running
  *         and 0 if the timer is stopped or an error occurred.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  [in] timer_id timer ID obtained by osTimerNew.
  * @retval 0 not running, 1 running.
  */
uint32_t osTimerIsRunning(osTimerId_t timer_id)
{
  /* For TX_TIMER the control block pointer is the timer identifier */
  TX_TIMER *timer_ptr = (TX_TIMER *)timer_id;
  /* The active state of the timer */
  UINT active;

  /* Check if this API is called from Interrupt Service Routines or the input
     timer_id in NULL */
  if (IS_IRQ_MODE() || (timer_id == NULL))
  {
    /* Return 0 in case of error */
    active = 0U;
  }
  else
  {
    /* Check if the timer is valid by calling the tx_timer_info_get to get
       the timer active state */
    if (tx_timer_info_get(timer_ptr, NULL, &active, NULL, NULL, NULL) != TX_SUCCESS)
    {
      /* Return 0 in case of error */
      active = 0U;
    }
  }

  return ((uint32_t)active);
}

/**
  * @brief  The function osTimerStop stops a timer specified by the parameter
  *         timer_id.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  [in] timer_id timer ID obtained by osTimerNew.
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osTimerStop(osTimerId_t timer_id)
{
  /* For TX_TIMER the control block pointer is the timer identifier */
  TX_TIMER *timer_ptr = (TX_TIMER *)timer_id;
  /* The returned status or error */
  osStatus_t status;
  /* The active state of the timer */
  UINT active;

  /* Check if this API is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    /* Return osErrorISR error */
    status = osErrorISR;
  }
  /* Check if the timer control block is valid */
  else if ((timer_id == NULL) || (timer_ptr->tx_timer_id != TX_TIMER_ID))
  {
    /* Return osErrorParameter error */
    status = osErrorParameter;
  }
  else
  {
    /* Get the timer running state */
    if (tx_timer_info_get(timer_ptr, NULL, &active, NULL, NULL, NULL) == TX_SUCCESS)
    {
      /* Check if the timer is running (active) */
      if (active == TX_TRUE)
      {
        /* Call the tx_timer_deactivate to deactivates the specified application
           timer. If the timer is already deactivated, this service has no
           effect. */
        if (tx_timer_deactivate(timer_ptr) == TX_SUCCESS)
        {
          /* Return osOK for success */
          status = osOK;
        }
        else
        {
          /* Return osErrorResource in case of error */
          status = osErrorResource;
        }
      }
      else
      {
        /* Return osErrorResource in case of error */
        status = osErrorResource;
      }
    }
    else
    {
      /* Return osErrorResource error */
      status = osErrorResource;
    }
  }

  return (status);
}

/**
  * @brief  The function osTimerStart starts or restarts a timer specified by
  *         the parameter timer_id. The parameter ticks specifies the value of
  *         the timer in time ticks.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  [in] timer_id timer ID obtained by osTimerNew.
  *         [in] ticks  time ticks value of the timer.
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osTimerStart(osTimerId_t timer_id, uint32_t ticks)
{
  /* For TX_TIMER the control block pointer is the timer identifier */
  TX_TIMER *timer_ptr = (TX_TIMER *)timer_id;
  /* The returned status or error */
  osStatus_t status = osOK;
  /* The active state of the timer */
  UINT active;
  /* The reschedule ticks of the timer */
  ULONG reschedule_ticks;

  /* Check if this API is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    /* Return osErrorISR error */
    status = osErrorISR;
  }
  /* Check if the timer control block is valid */
  else if ((timer_id == NULL) || (timer_ptr->tx_timer_id != TX_TIMER_ID))
  {
    /* Return osErrorParameter error */
    status = osErrorParameter;
  }
  else
  {
    /* Get the timer running state and reschedule ticks parameters */
    if (tx_timer_info_get(timer_ptr, NULL, &active, NULL, &reschedule_ticks, NULL) == TX_SUCCESS)
    {
      /* Check if the timer is active. If so, it shall be stopped before being
         activated again */
      if (active == TX_TRUE)
      {
        /* Call the tx_timer_deactivate to deactivates the specified application
           timer. If the timer is already deactivated, this service has no
           effect. */
        if (tx_timer_deactivate(timer_ptr) == TX_SUCCESS)
        {
          /* Set status osOK for success */
          status = osOK;
        }
        else
        {
          /* Return osErrorResource in case of error */
          status = osErrorResource;
        }
      }

      if (status == osOK)
      {
        /* Check the timer reschedule_ticks parameter for periodicity */
        if (reschedule_ticks > 0)
        {
          /* Set the reschedule_ticks to the input ticks */
          reschedule_ticks = ticks;
        }

        /* An expired one-shot timer must be reset via tx_timer_change before
           it can be activated again. */
        if (tx_timer_change(timer_ptr, ticks, reschedule_ticks) == TX_SUCCESS)
        {
          /* Call the tx_timer_activate to activates the specified application
             timer. The expiration routines of timers that expire at the same
             time are executed in the order they were activated. */
          if (tx_timer_activate(timer_ptr) == TX_SUCCESS)
          {
            /* Return osOK for success */
            status = osOK;
          }
          else
          {
            /* Return osErrorResource in case of error */
            status = osErrorResource;
          }
        }
        else
        {
          /* Return osErrorResource in case of error */
          status = osErrorResource;
        }
      }
    }
    else
    {
      /* Return osErrorResource error */
      status = osErrorResource;
    }
  }

  return (status);
}

/**
  * @brief  The function osTimerDelete deletes the timer specified by parameter
  *         timer_id.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  [in] timer_id timer ID obtained by osTimerNew.
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osTimerDelete(osTimerId_t timer_id)
{
  /* For TX_TIMER the control block pointer is the timer identifier */
  TX_TIMER *timer_ptr = (TX_TIMER *)timer_id;
  /* The returned status or error */
  osStatus_t status;

  /* Check if this API is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    /* Return osErrorISR error */
    status = osErrorISR;
  }
  /* Check if the timer control block is valid */
  else if ((timer_id == NULL) || (timer_ptr->tx_timer_id != TX_TIMER_ID))
  {
    /* Return osErrorParameter error */
    status = osErrorParameter;
  }
  else
  {
    /* Call the tx_timer_delete to delete the specified application timer. */
    if (tx_timer_delete(timer_ptr) == TX_SUCCESS)
    {
      /* Free the already allocated memory for timer control block */
      MemFree(timer_ptr);

      /* Return osOK for success */
      status = osOK;
    }
    else
    {
      /* Return osErrorResource in case of error */
      status = osErrorResource;
    }
  }

  return (status);
}

/*---------------------------------------------------------------------------*/
/*-----------------------------Event flags APIs------------------------------*/
/*---------------------------------------------------------------------------*/

/**
  * @brief  The function osEventFlagsNew creates a new event flags object that
  *      is used to send events across threads and returns the pointer to the
  *      event flags object identifier or NULL in case of an error. It can be
  *      safely called before the RTOS is started (call to osKernelStart), but
  *      not before it is initialized (call to osKernelInitialize).
  *      Note : This function cannot be called from Interrupt Service
  *      Routines.
  * @param  [in]  attr  event flags attributes; NULL: default values.
  * @retval event flags ID for reference by other functions or NULL in
  *      case of error.
  */
osEventFlagsId_t osEventFlagsNew(const osEventFlagsAttr_t *attr)
{
  /* For ThreadX the control block pointer is the event flags identifier */
  TX_EVENT_FLAGS_GROUP *eventflags_ptr = NULL;
  /* Pointer to the event flags name */
  CHAR *name_ptr = NULL;
  /* The size of control block */
  ULONG cb_size = sizeof(TX_EVENT_FLAGS_GROUP);

  /* Check if this API is called from Interrupt Service Routines */
  if (!IS_IRQ_MODE())
  {
    /* Check if the attr is not NULL */
    if (attr != NULL)
    {
      /* Check if the name_ptr is not NULL */
      if (attr->name != NULL)
      {
        /* Set the event flags name_ptr */
        name_ptr = (CHAR *)attr->name;
      }

      /* Check if the control block size is equal to 0 */
      if (attr->cb_size == 0U)
      {
        /* Set control block size to sizeof(TX_EVENT_FLAGS_GROUP) */
        cb_size = sizeof(TX_EVENT_FLAGS_GROUP);
      }
      else if (attr->cb_size < sizeof(TX_EVENT_FLAGS_GROUP))
      {
        /* Return NULL pointer in case of error */
        return (NULL);
      }
      else
      {
        /* Set control block size to attr->cb_size */
        cb_size = (ULONG) attr->cb_size;
      }

      /* Check if the input control block pointer is NULL */
      if (attr->cb_mem == NULL)
      {
        /* Allocate the eventflags_ptr structure for the event flags to be created */
        eventflags_ptr = (TX_EVENT_FLAGS_GROUP *)MemAlloc(cb_size, RTOS2_BYTE_POOL_HEAP_TYPE);
        if (eventflags_ptr == NULL)
        {
          /* Return NULL pointer in case of error */
          return (NULL);
        }
      }
      else
      {
        /* The control block shall point to the input cb_mem memory address */
        eventflags_ptr = attr->cb_mem;
      }
    }
    else
    {
      /* Allocate the eventflags_ptr structure for the event flags to be created */
      eventflags_ptr = (TX_EVENT_FLAGS_GROUP *)MemAlloc(cb_size, RTOS2_BYTE_POOL_HEAP_TYPE);
    }

    /* Call the tx_event_flags_create function to create the new event flags */
    if (tx_event_flags_create(eventflags_ptr, name_ptr) != TX_SUCCESS)
    {
      if ((attr->cb_mem == NULL) || (attr == NULL))
      {
        /* Free the already allocated memory for event flags control block */
        MemFree(eventflags_ptr);
      }
    }
  }

  return ((osEventFlagsId_t)eventflags_ptr);
}

/**
  * @brief  The function osEventFlagsGetName returns the pointer to the name
  *      string of the event flags object identified by parameter ef_id or NULL
  *      in case of an error.
  *      Note : This function cannot be called from Interrupt Service
  *      Routines.
  * @param  [in]  ef_id  event flags ID obtained by osEventFlagsNew.
  * @retval name as null-terminated string.
  */
const char *osEventFlagsGetName(osEventFlagsId_t ef_id)
{
  /* For ThreadX the control block pointer is the event flags identifier */
  TX_EVENT_FLAGS_GROUP *eventflags_ptr = (TX_EVENT_FLAGS_GROUP *)ef_id;
  /* The output name_ptr as null-terminated string */
  CHAR *name_ptr;

  /* Check if this API is called from Interrupt Service Routines
     or the ef_id is NULL */
  if (IS_IRQ_MODE() || (eventflags_ptr == NULL))
  {
    /* Return NULL in case of an error */
    name_ptr = NULL;
  }
  else
  {
    /* Call the tx_event_flags_info_get to get the event flags name_ptr */
    if (tx_event_flags_info_get(eventflags_ptr, &name_ptr, NULL, NULL, NULL, NULL) != TX_SUCCESS)
    {
      /* Return NULL in case of an error */
      name_ptr = NULL;
    }
  }

  return (name_ptr);
}

/**
  * @brief  The function osEventFlagsSet sets the event flags specified by the
  *      parameter flags in an event flags object specified by parameter ef_id.
  *      All threads waiting for the flag set will be notified to resume from
  *      BLOCKED state. The function returns the event flags stored in the event
  *      control block or an error code (highest bit is set, refer to Flags
  *      Functions Error Codes).
  *      Note : This function cannot be called from Interrupt Service
  *      Routines.
  * @param  [in]  ef_id  event flags ID obtained by osEventFlagsNew.
  *         [in]  flags specifies the flags that shall be set.
  * @retval event flags after setting or error code if highest bit set.
  */
uint32_t osEventFlagsSet(osEventFlagsId_t ef_id, uint32_t flags)
{
  /* For ThreadX the control block pointer is the semaphore identifier */
  TX_EVENT_FLAGS_GROUP *eventflags_ptr = (TX_EVENT_FLAGS_GROUP *)ef_id;
  /* The returned flags_status or error */
  uint32_t flags_status;
  /* Flags to set */
  ULONG flags_to_set = (ULONG) flags;

  /* Check if the event flags ID is NULL or the event flags is invalid */
  if ((ef_id == NULL) || (eventflags_ptr -> tx_event_flags_group_id != TX_EVENT_FLAGS_ID))
  {
    /* Return osFlagsErrorParameter error */
    flags_status = osFlagsErrorParameter;
  }
  else
  {
    /* Call the tx_event_flags_set to set flags */
    if (tx_event_flags_set(eventflags_ptr, flags_to_set, TX_OR) == TX_SUCCESS)
    {
      /* Call the tx_event_flags_info_get to get the current flags */
      if (tx_event_flags_info_get(eventflags_ptr, NULL, (ULONG *) &flags_status, NULL, NULL, NULL) != TX_SUCCESS)
      {
        /* Return osFlagsErrorUnknown in case of an error */
        flags_status = osFlagsErrorUnknown;
      }
    }
    else
    {
      /* Return osFlagsErrorResource in case of error */
      flags_status = osFlagsErrorResource;
    }
  }

  return (flags_status);
}

/**
  * @brief  The function osEventFlagsClear clears the event flags specified by
  *      the parameter flags in an event flags object specified by parameter
  *      ef_id. The function returns the event flags before clearing or an error
  *      code (highest bit is set, refer to Flags Functions Error Codes).
  *      Note : This function cannot be called from Interrupt Service
  *      Routines.
  * @param  [in]  ef_id  event flags ID obtained by osEventFlagsNew.
  *         [in]  flags specifies the flags that shall be cleared.
  * @retval event flags before clearing or error code if highest bit set.
  */
uint32_t osEventFlagsClear(osEventFlagsId_t ef_id, uint32_t flags)
{
  /* For ThreadX the control block pointer is the semaphore identifier */
  TX_EVENT_FLAGS_GROUP *eventflags_ptr = (TX_EVENT_FLAGS_GROUP *)ef_id;
  /* The returned flags_status or error */
  uint32_t flags_status;
  /* Flags to clear */
  ULONG flags_to_clear = (ULONG) ~flags;

  /* Check if the event flags ID is NULL or the event flags is invalid */
  if ((ef_id == NULL) || (eventflags_ptr -> tx_event_flags_group_id != TX_EVENT_FLAGS_ID))
  {
    /* Return osFlagsErrorParameter error */
    flags_status = osFlagsErrorParameter;
  }
  else
  {
    /* Call the tx_event_flags_info_get to get the current flags */
    if (tx_event_flags_info_get(eventflags_ptr, NULL, (ULONG *) &flags_status, NULL, NULL, NULL) != TX_SUCCESS)
    {
      /* Return osFlagsErrorUnknown in case of an error */
      flags_status = osFlagsErrorUnknown;
    }
    else
    {
      /* Call the tx_event_flags_set to set flags */
      if (tx_event_flags_set(eventflags_ptr, flags_to_clear, TX_AND) != TX_SUCCESS)
      {
        /* Return osFlagsErrorResource in case of error */
        flags_status = osFlagsErrorResource;
      }
    }
  }

  return (flags_status);
}

/**
  * @brief  The function osEventFlagsGet returns the event flags currently set
  *      in an event flags object specified by parameter ef_id or 0 in case of
  *      an error.
  *      Note : This function cannot be called from Interrupt Service
  *      Routines.
  * @param  [in]  ef_id  event flags ID obtained by osEventFlagsNew.
  * @retval current event flags.
  */
uint32_t osEventFlagsGet(osEventFlagsId_t ef_id)
{
  /* For ThreadX the control block pointer is the event flags identifier */
  TX_EVENT_FLAGS_GROUP *eventflags_ptr = (TX_EVENT_FLAGS_GROUP *)ef_id;
  /* The current flags */
  uint32_t current_flags = 0;

  /* Check if the event flags ID is NULL or the event flags is invalid */
  if ((ef_id == NULL) || (eventflags_ptr -> tx_event_flags_group_id != TX_EVENT_FLAGS_ID))
  {
    /* Return 0 in case of an error */
    current_flags = 0;
  }
  else
  {
    /* Call the tx_event_flags_info_get to get the current flags */
    if (tx_event_flags_info_get(eventflags_ptr, NULL, (ULONG *) &current_flags, NULL, NULL, NULL) != TX_SUCCESS)
    {
      /* Return 0 in case of an error */
      current_flags = 0;
    }
  }

  return (current_flags);
}

/**
  * @brief  The function osEventFlagsWait suspends the execution of the
  *      currently RUNNING thread until any or all event flags specified by the
  *      parameter flags in the event object specified by parameter ef_id are
  *      set. When these event flags are already set, the function returns
  *      instantly. Otherwise, the thread is put into the state BLOCKED.
  *      Note : This function cannot be called from Interrupt Service
  *      Routines.
  * @param  [in]  ef_id  event flags ID obtained by osEventFlagsNew.
  *         [in]  flags  specifies the flags to wait for.
  *         [in]  options  specifies flags options (osFlagsXxxx).
  *         [in]  timeout  Timeout Value or 0 in case of no time-out.
  * @retval event flags before clearing or error code if highest bit set.
  */
uint32_t osEventFlagsWait(osEventFlagsId_t ef_id, uint32_t flags,
                          uint32_t options, uint32_t timeout)
{
  /* For ThreadX the control block pointer is the semaphore identifier */
  TX_EVENT_FLAGS_GROUP *eventflags_ptr = (TX_EVENT_FLAGS_GROUP *)ef_id;
  /* Flags to get */
  ULONG requested_flags = (ULONG) flags;
  /* The ThreadX wait option */
  ULONG wait_option = (ULONG) timeout;
  /* The ThreadX get options */
  UINT get_option = 0;
  /* The actual flags */
  ULONG actual_flags;
  /* ThreadX APIs status */
  UINT status;

  /* Check if the event flags ID is NULL or the event flags is invalid or non-zero timeout specified in an ISR */
  if ((IS_IRQ_MODE() && (timeout != 0)) || (ef_id == NULL) ||
      (eventflags_ptr -> tx_event_flags_group_id != TX_EVENT_FLAGS_ID))
  {
    /* Return osFlagsErrorParameter error */
    actual_flags = osFlagsErrorParameter;
  }
  else
  {
    /* No clear option is used */
    if ((options & osFlagsNoClear) == osFlagsNoClear)
    {
      if ((options & osFlagsWaitAll) == osFlagsWaitAll)
      {
        /* AND event option is used */
        get_option = TX_AND;
      }
      else
      {
        /* OR event option is used */
        get_option = TX_OR;
      }
    }
    /* Clear option is used */
    else
    {
      if ((options & osFlagsWaitAll) == osFlagsWaitAll)
      {
        /* AND clear event option is used */
        get_option = TX_AND_CLEAR;
      }
      else
      {
        /* OR clear event option is used */
        get_option = TX_OR_CLEAR;
      }
    }
    /* Call the tx_event_flags_get to get flags */
    status = tx_event_flags_get(eventflags_ptr, requested_flags, get_option, &actual_flags, wait_option);
    /* Check the status */
    if ((status == TX_NO_EVENTS) || (status == TX_WAIT_ERROR))
    {
      /* Return osFlagsErrorTimeout for timeout error */
      actual_flags = osFlagsErrorTimeout;
    }
    else if (status != TX_SUCCESS)
    {
      /* Return osFlagsErrorResource in case of an error */
      actual_flags = osFlagsErrorResource ;
    }
  }

  return ((uint32_t)actual_flags);
}

/**
  * @brief  The function osEventFlagsDelete deletes the event flags object
  *      specified by parameter ef_id and releases the internal memory obtained
  *      for the event flags handling. After this call, the ef_id is no longer
  *      valid and cannot be used. This can cause starvation of threads that are
  *      waiting for flags of this event object. The ef_id may be created again
  *      using the function osEventFlagsNew.
  *      Note : This function cannot be called from Interrupt Service
  *      Routines.
  * @param  [in]  ef_id  event flags ID obtained by osEventFlagsNew.
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osEventFlagsDelete(osEventFlagsId_t ef_id)
{
  /* For ThreadX the control block pointer is the event flags identifier */
  TX_EVENT_FLAGS_GROUP *eventflags_ptr = (TX_EVENT_FLAGS_GROUP *)ef_id;
  /* The returned status or error */
  osStatus_t status;

  /* Check if this API is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    /* Return osErrorISR error */
    status = osErrorISR;
  }
  /* Check if the event flags ID is NULL or the event flags is invalid */
  else if ((ef_id == NULL) || (eventflags_ptr -> tx_event_flags_group_id != TX_EVENT_FLAGS_ID))
  {
    /* Return osErrorParameter error */
    status = osErrorParameter;
  }
  else
  {
    /* Call the tx_event_flags_delete to delete the event flags object */
    if (tx_event_flags_delete(eventflags_ptr) == TX_SUCCESS)
    {
      /* Free the already allocated memory for thread control block */
      if (MemFree(eventflags_ptr) == osOK)
      {
        /* Return osOK for success */
        status = osOK;
      }
      else
      {
        /* Return osErrorResource in case of error */
        status = osErrorResource;
      }
    }
    else
    {
      /* Return osErrorResource in case of error */
      status = osErrorResource;
    }
  }
  return (status);
}

/*---------------------------------------------------------------------------*/
/*---------------------------Mutex Management APIs---------------------------*/
/*---------------------------------------------------------------------------*/

/**
  * @brief  The function osMutexNew creates and initializes a new mutex
  *       object and returns the pointer to the mutex object identifier or
  *       NULL in case of an error. It can be safely called before the RTOS
  *       is started (call to osKernelStart), but not before it is initialized
  *       call to osKernelInitialize).
  *       Note : This function cannot be called from Interrupt Service
  *       Routines.
  * @param  [in]  attr  mutex attributes; NULL: default values.
  * @retval mutex ID for reference by other functions or NULL in case of error.
  */
osMutexId_t osMutexNew(const osMutexAttr_t *attr)
{
  /* For ThreadX the control block pointer is the mutex identifier */
  TX_MUTEX *mutex_ptr = NULL;
  /* Pointer to the mutex name */
  CHAR *name_ptr = NULL;
  /* The mutex inherit status */
  UINT inherit = TX_NO_INHERIT;
  /* The size of control block */
  ULONG cb_size = sizeof(TX_MUTEX);

  /* Check if this API is called from Interrupt Service Routines */
  if (!IS_IRQ_MODE())
  {
    /* Check if the attr is not NULL */
    if (attr != NULL)
    {
      /* Check if the name_ptr is not NULL */
      if (attr->name != NULL)
      {
        /* Set the mutex name_ptr */
        name_ptr = (CHAR *)attr->name;
      }
      /* Check if the attr_bits is not zero */
      if (attr->attr_bits != 0)
      {
        /* Check the mutex inherit status */
        if ((attr->attr_bits & osMutexPrioInherit) == osMutexPrioInherit)
        {
          inherit = TX_INHERIT;
        }
        else
        {
          inherit = TX_NO_INHERIT;
        }
      }
      else
      {
        inherit = TX_NO_INHERIT;
      }
      /* Check if the control block size is equal to 0 */
      if (attr->cb_size == 0U)
      {
        /* Set control block size to sizeof(TX_MUTEX) */
        cb_size = sizeof(TX_MUTEX);
      }
      else if (attr->cb_size < sizeof(TX_MUTEX))
      {
        /* Return NULL pointer in case of error */
        return (NULL);
      }
      else
      {
        /* Set control block size to attr->cb_size */
        cb_size = (ULONG) attr->cb_size;
      }

      /* Check if the input control block pointer is NULL */
      if (attr->cb_mem == NULL)
      {
        /* Allocate the mutex_ptr structure for the mutex to be created */
        mutex_ptr = (TX_MUTEX *)MemAlloc(cb_size, RTOS2_BYTE_POOL_HEAP_TYPE);
        if (mutex_ptr == NULL)
        {
          /* Return NULL pointer in case of error */
          return (NULL);
        }
      }
      else
      {
        /* The control block shall point to the input cb_mem memory address */
        mutex_ptr = attr->cb_mem;
      }
    }
    else
    {
      /* Allocate the mutex_ptr structure for the mutex to be created */
      mutex_ptr = (TX_MUTEX *)MemAlloc(cb_size, RTOS2_BYTE_POOL_HEAP_TYPE);
    }

    /* Call the tx_mutex_create function to create the new mutex */
    if (tx_mutex_create(mutex_ptr, name_ptr, inherit) != TX_SUCCESS)
    {
      if ((attr->cb_mem == NULL) || (attr == NULL))
      {
        /* Free the already allocated memory for mutex control block */
        MemFree(mutex_ptr);
      }
    }
  }

  return ((osMutexId_t)mutex_ptr);
}

/**
  * @brief  The function osMutexGetName returns the pointer to the name string
  *       of the mutex identified by parameter mutex_id or NULL in case of
  *       an error.
  *       Note : This function cannot be called from Interrupt Service
  *       Routines.
  * @param  [in]  mutex_id mutex ID obtained by osMutexNew.
  * @retval name as null-terminated string.
  */
const char *osMutexGetName(osMutexId_t mutex_id)
{
  /* For ThreadX the control block pointer is the mutex identifier */
  TX_MUTEX *mutex_ptr = (TX_MUTEX *)mutex_id;
  /* The output name_ptr as null-terminated string */
  CHAR *name_ptr;

  /* Check if this API is called from Interrupt Service Routines
     or the mutex_id is NULL  or the mutex is invalid */
  if (IS_IRQ_MODE() || (mutex_ptr == NULL) || (mutex_ptr -> tx_mutex_id != TX_MUTEX_ID))
  {
    /* Return NULL in case of an error */
    name_ptr = NULL;
  }
  else
  {
    /* Call the tx_mutex_info_get to get the mutex name_ptr */
    if (tx_mutex_info_get(mutex_ptr, &name_ptr, NULL, NULL, NULL, NULL, NULL) != TX_SUCCESS)
    {
      /* Return NULL in case of an error */
      name_ptr = NULL;
    }
  }

  return (name_ptr);
}

/**
  * @brief  The blocking function osMutexAcquire waits until a mutex object
  *      specified by parameter mutex_id becomes available. If no other thread
  *      has obtained the mutex, the function instantly returns and blocks
  *      the mutex object.
  *      Note : This function cannot be called from Interrupt Service
  *      Routines.
  * @param  [in]  mutex_id  mutex ID obtained by osMutexNew.
  *         [in]  timeout  Timeout Value or 0 in case of no time-out.
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osMutexAcquire(osMutexId_t mutex_id, uint32_t timeout)
{
  /* For ThreadX the control block pointer is the mutex identifier */
  TX_MUTEX *mutex_ptr = (TX_MUTEX *)mutex_id;
  /* The returned status or error */
  osStatus_t status;
  /* The ThreadX wait option */
  ULONG wait_option = (ULONG) timeout;
  /* The tx_mutex_get returned status */
  UINT tx_status;

  /* Check if this API is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    /* Return osErrorISR error */
    status = osErrorISR;
  }
  /* Check if the mutex ID is NULL or the mutex is invalid */
  else if ((mutex_id == NULL) || (mutex_ptr -> tx_mutex_id != TX_MUTEX_ID))
  {
    /* Return osErrorParameter error */
    status = osErrorParameter;
  }
  else
  {
    /* Call the tx_mutex_get to get the mutex object */
    tx_status = tx_mutex_get(mutex_ptr, wait_option);
    if (tx_status == TX_SUCCESS)
    {
      /* Return osOK for success */
      status = osOK;
    }
    else if ((tx_status == TX_WAIT_ABORTED) || (tx_status == TX_NOT_AVAILABLE))
    {
      /* Return osErrorTimeout when the mutex is not obtained in the given time */
      status = osErrorTimeout;
    }
    else
    {
      /* Return osErrorResource in case of error */
      status = osErrorResource;
    }
  }

  return (status);
}

/**
  * @brief  The function osMutexRelease releases a mutex specified by parameter
  *      mutex_id. Other threads that currently wait for this mutex will be
  *      put into the READY state.
  *      Note : This function cannot be called from Interrupt Service
  *      Routines.
  * @param  [in]  mutex_id  mutex ID obtained by osMutexNew.
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osMutexRelease(osMutexId_t mutex_id)
{
  /* For ThreadX the control block pointer is the mutex identifier */
  TX_MUTEX *mutex_ptr = (TX_MUTEX *)mutex_id;
  /* The returned status or error */
  osStatus_t status;

  /* Check if this API is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    /* Return osErrorISR error */
    status = osErrorISR;
  }
  /* Check if the mutex ID is NULL or the mutex is invalid */
  else if ((mutex_id == NULL) || (mutex_ptr -> tx_mutex_id != TX_MUTEX_ID))
  {
    /* Return osErrorParameter error */
    status = osErrorParameter;
  }
  else
  {
    /* Call the tx_mutex_put to put the mutex object */
    if (tx_mutex_put(mutex_ptr) == TX_SUCCESS)
    {
      /* Return osOK for success */
      status = osOK;
    }
    else
    {
      /* Return osErrorResource in case of error */
      status = osErrorResource;
    }
  }

  return (status);
}

/**
  * @brief  The function osMutexGetOwner returns the thread ID of the thread
  *      that acquired a mutex specified by parameter mutex_id. In case of an
  *      error or if the mutex is not blocked by any thread, it returns NULL.
  *      Note : This function cannot be called from Interrupt Service
  *      Routines.
  * @param  [in]  mutex_id  mutex ID obtained by osMutexNew.
  * @retval thread ID of owner thread or NULL when mutex was not acquired.
  */
osThreadId_t osMutexGetOwner(osMutexId_t mutex_id)
{
  /* For ThreadX the control block pointer is the mutex identifier */
  TX_MUTEX *mutex_ptr = (TX_MUTEX *)mutex_id;
  /* The owner thread of the mutex object */
  TX_THREAD *thread_ptr = NULL;

  /* Check if this API is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    /* Return NULL when the API is called from ISR */
    thread_ptr = NULL;
  }
  /* Check if the mutex ID is NULL or the mutex is invalid */
  else if ((mutex_id == NULL) || (mutex_ptr -> tx_mutex_id != TX_MUTEX_ID))
  {
    /* Return NULL when mutex_id is NULL */
    thread_ptr = NULL;
  }
  else
  {
    /* Call the tx_mutex_info_get to get the mutex thread owner */
    if (tx_mutex_info_get(mutex_ptr, NULL, NULL, &thread_ptr, NULL, NULL, NULL) == TX_SUCCESS)
    {
      if (thread_ptr == TX_NULL)
      {
        /* Return NULL when mutex object is not acquired */
        thread_ptr = NULL;
      }
    }
    else
    {
      /* Return NULL when mutex_id is invalid */
      thread_ptr = NULL;
    }
  }

  return ((osThreadId_t)thread_ptr);
}

/**
  * @brief  The function osMutexDelete deletes a mutex object specified by
  *      parameter mutex_id. It releases internal memory obtained for mutex
  *      handling. After this call, the mutex_id is no longer valid and cannot
  *      be used. The mutex may be created again using the function osMutexNew.
  *      Note : This function cannot be called from Interrupt Service
  *      Routines.
  * @param  [in]  mutex_id  mutex ID obtained by osMutexNew.
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osMutexDelete(osMutexId_t mutex_id)
{
  /* For ThreadX the control block pointer is the mutex identifier */
  TX_MUTEX *mutex_ptr = (TX_MUTEX *)mutex_id;
  /* The returned status or error */
  osStatus_t status;

  /* Check if this API is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    /* Return osErrorISR error */
    status = osErrorISR;
  }
  /* Check if the mutex ID is NULL or the mutex is invalid*/
  else if ((mutex_id == NULL) || (mutex_ptr -> tx_mutex_id != TX_MUTEX_ID))
  {
    /* Return osErrorParameter error */
    status = osErrorParameter;
  }
  else
  {
    /* Call the tx_mutex_delete to delete the mutex object */
    if (tx_mutex_delete(mutex_ptr) == TX_SUCCESS)
    {
      /* Free the already allocated memory for mutex control block */
      if (MemFree(mutex_ptr) == osOK)
      {
        /* Return osOK for success */
        status = osOK;
      }
      else
      {
        /* Return osErrorResource in case of error */
        status = osErrorResource;
      }
    }
    else
    {
      /* Return osErrorResource in case of error */
      status = osErrorResource;
    }
  }

  return (status);
}

/*---------------------------------------------------------------------------*/
/*------------------------------Semaphores APIs------------------------------*/
/*---------------------------------------------------------------------------*/

/**
  * @brief  The function osSemaphoreNew creates and initializes a semaphore
  *       object that is used to manage access to shared resources and returns
  *       the pointer to the semaphore object identifier or NULL in case of an
  *       error. It can be safely called before the RTOS is started
  *      (call to osKernelStart), but not before it is initialized
  *      (call to osKernelInitialize).
  *       Note : This function cannot be called from Interrupt Service
  *       Routines.
  * @param  [in]  max_count  maximum number of available tokens.
  *         [in]  initial_count  initial number of available tokens.
  *         [in]  attr  semaphore attributes; NULL: default values.
  * @retval semaphore ID for reference by other functions or NULL in case of error.
  */
osSemaphoreId_t osSemaphoreNew(uint32_t max_count, uint32_t initial_count, const osSemaphoreAttr_t *attr)
{
  /* For ThreadX the control block pointer is the semaphore identifier */
  TX_SEMAPHORE *semaphore_ptr = NULL;
  /* Pointer to the semaphore name */
  CHAR *name_ptr = NULL;
  /* The semaphore initial count */
  ULONG init_count = (ULONG) initial_count;
  /* The size of control block */
  ULONG cb_size = sizeof(TX_SEMAPHORE);

  /* Check if this API is called from Interrupt Service Routines */
  if (!IS_IRQ_MODE())
  {
    /* Check if the attr is not NULL */
    if (attr != NULL)
    {
      /* Check if the name_ptr is not NULL */
      if (attr->name != NULL)
      {
        /* Set the semaphore name_ptr */
        name_ptr = (CHAR *)attr->name;
      }

      /* Check if the control block size is equal to 0 */
      if (attr->cb_size == 0U)
      {
        /* Set control block size to sizeof(TX_SEMAPHORE) */
        cb_size = sizeof(TX_SEMAPHORE);
      }
      else if (attr->cb_size < sizeof(TX_SEMAPHORE))
      {
        /* Return NULL pointer in case of error */
        return (NULL);
      }
      else
      {
        /* Set control block size to attr->cb_size */
        cb_size = (ULONG) attr->cb_size;
      }

      /* Check if the input control block pointer is NULL */
      if (attr->cb_mem == NULL)
      {
        /* Allocate the semaphore_ptr structure for the semaphore to be created */
        semaphore_ptr = (TX_SEMAPHORE *)MemAlloc(cb_size, RTOS2_BYTE_POOL_HEAP_TYPE);
        if (semaphore_ptr == NULL)
        {
          /* Return NULL pointer in case of error */
          return (NULL);
        }
      }
      else
      {
        /* The control block shall point to the input cb_mem memory address */
        semaphore_ptr = attr->cb_mem;
      }
    }
    else
    {
      /* Allocate the semaphore_ptr structure for the semaphore to be created */
      semaphore_ptr = (TX_SEMAPHORE *)MemAlloc(cb_size, RTOS2_BYTE_POOL_HEAP_TYPE);
    }


    /* Call the tx_semaphore_create function to create the new semaphore */
    if (tx_semaphore_create(semaphore_ptr, name_ptr, init_count) != TX_SUCCESS)
    {
      if ((attr->cb_mem == NULL) || (attr == NULL))
      {
        /* Free the already allocated memory for semaphore control block */
        MemFree(semaphore_ptr);
      }
    }
  }

  return ((osSemaphoreId_t)semaphore_ptr);
}

/**
  * @brief  The function osSemaphoreGetName returns the pointer to the name
  *       string of the semaphore identified by parameter semaphore_id or NULL
  *       in case of an error.
  *       Note : This function cannot be called from Interrupt Service
  *       Routines.
  * @param  [in]  semaphore_id  semaphore ID obtained by osSemaphoreNew.
  * @retval name as null-terminated string.
  */
const char *osSemaphoreGetName(osSemaphoreId_t semaphore_id)
{
  /* For ThreadX the control block pointer is the semaphore identifier */
  TX_SEMAPHORE *semaphore_ptr = (TX_SEMAPHORE *)semaphore_id;
  /* The output name_ptr as null-terminated string */
  CHAR *name_ptr;

  /* Check if this API is called from Interrupt Service Routines or the semaphore_id is NULL
     and the semaphore is invalid  */
  if (IS_IRQ_MODE() || (semaphore_ptr == NULL) || (semaphore_ptr -> tx_semaphore_id != TX_SEMAPHORE_ID))
  {
    /* Return NULL in case of an error */
    name_ptr = NULL;
  }
  else
  {
    /* Call the tx_semaphore_info_get to get the semaphore name_ptr */
    if (tx_semaphore_info_get(semaphore_ptr, &name_ptr, NULL, NULL, NULL, NULL) != TX_SUCCESS)
    {
      /* Return NULL in case of an error */
      name_ptr = NULL;
    }
  }

  return (name_ptr);
}

/**
  * @brief  The blocking function osSemaphoreAcquire waits until a token of the
  *      semaphore object specified by parameter semaphore_id becomes available.
  *      If a token is available, the function instantly returns and decrements
  *      the token count.
  *      Note : This function cannot be called from Interrupt Service
  *      Routines.
  * @param  [in]  semaphore_id  semaphore ID obtained by osSemaphoreNew.
  *         [in]  timeout  Timeout Value or 0 in case of no time-out.
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osSemaphoreAcquire(osSemaphoreId_t semaphore_id, uint32_t timeout)
{
  /* For ThreadX the control block pointer is the semaphore identifier */
  TX_SEMAPHORE *semaphore_ptr = (TX_SEMAPHORE *)semaphore_id;
  /* The returned status or error */
  osStatus_t status;
  /* The ThreadX wait option */
  ULONG wait_option = (ULONG) timeout;
  /* The tx_semaphore_get returned status */
  UINT tx_status;

  /* Check if the semaphore ID is NULL or the semaphore is invalid or non-zero timeout specified in an ISR  */
  if ((IS_IRQ_MODE() && (timeout != 0)) || (semaphore_id == NULL) ||
      (semaphore_ptr -> tx_semaphore_id != TX_SEMAPHORE_ID))
  {
    /* Return osErrorParameter error */
    status = osErrorParameter;
  }
  else
  {
    /* Call the tx_semaphore_get to get the semaphore object */
    tx_status = tx_semaphore_get(semaphore_ptr, wait_option);
    if (tx_status == TX_SUCCESS)
    {
      /* Return osOK for success */
      status = osOK;
    }
    else if ((tx_status == TX_WAIT_ABORTED) || (tx_status == TX_NO_INSTANCE))
    {
      /* Return osErrorTimeout when the semaphore is not obtained
      in the given time */
      status = osErrorTimeout;
    }
    else
    {
      /* Return osErrorResource in case of error */
      status = osErrorResource;
    }
  }

  return (status);
}

/**
  * @brief  The function osSemaphoreRelease releases a token of the semaphore
  *      object specified by parameter semaphore_id. Tokens can only be released
  *      up to the maximum count specified at creation time, see osSemaphoreNew.
  *      Other threads that currently wait for a token of this semaphore object
  *      will be put into the READY state.
  *      Note : This function cannot be called from Interrupt Service
  *      Routines.
  * @param  [in]  semaphore_id  semaphore ID obtained by osSemaphoreNew.
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osSemaphoreRelease(osSemaphoreId_t semaphore_id)
{
  /* For ThreadX the control block pointer is the semaphore identifier */
  TX_SEMAPHORE *semaphore_ptr = (TX_SEMAPHORE *)semaphore_id;
  /* The returned status or error */
  osStatus_t status;

  /* Check if the semaphore ID is NULL or the semaphore is invalid */
  if ((semaphore_id == NULL) || (semaphore_ptr -> tx_semaphore_id != TX_SEMAPHORE_ID))
  {
    /* Return osErrorParameter error */
    status = osErrorParameter;
  }
  else
  {
    /* Call the tx_semaphore_put to put the semaphore object */
    if (tx_semaphore_put(semaphore_ptr) == TX_SUCCESS)
    {
      /* Return osOK for success */
      status = osOK;
    }
    else
    {
      /* Return osErrorResource in case of error */
      status = osErrorResource;
    }
  }

  return (status);
}

/**
  * @brief  The function osSemaphoreDelete deletes a semaphore object specified
  *      by parameter semaphore_id. It releases internal memory obtained for
  *      semaphore handling. After this call, the semaphore_id is no longer
  *      valid and cannot be used. The semaphore may be created again using
  *      the function osSemaphoreNew.
  *      Note : This function cannot be called from Interrupt Service
  *      Routines.
  * @param  [in]  semaphore_id  semaphore ID obtained by osSemaphoreNew.
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osSemaphoreDelete(osSemaphoreId_t semaphore_id)
{
  /* For ThreadX the control block pointer is the semaphore identifier */
  TX_SEMAPHORE *semaphore_ptr = (TX_SEMAPHORE *)semaphore_id;
  /* The returned status or error */
  osStatus_t status;

  /* Check if this API is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    /* Return osErrorISR error */
    status = osErrorISR;
  }
  /* Check if the semaphore ID is NULL or the semaphore is invalid */
  else if ((semaphore_id == NULL) || (semaphore_ptr -> tx_semaphore_id != TX_SEMAPHORE_ID))
  {
    /* Return osErrorParameter error */
    status = osErrorParameter;
  }
  else
  {
    /* Call the tx_semaphore_delete to delete the semaphore object */
    if (tx_semaphore_delete(semaphore_ptr) == TX_SUCCESS)
    {
      /* Free the already allocated memory for semaphore control block */
      if (MemFree(semaphore_ptr) == osOK)
      {
        /* Return osOK for success */
        status = osOK;
      }
      else
      {
        /* Return osErrorResource in case of error */
        status = osErrorResource;
      }
    }
    else
    {
      /* Return osErrorResource in case of error */
      status = osErrorResource;
    }
  }
  return (status);
}

/*---------------------------------------------------------------------------*/
/*------------------------------Message Queue APIs---------------------------*/
/*---------------------------------------------------------------------------*/

/**
  * @brief   The function osMessageQueueNew creates and initializes a message queue
  *         object. The function returns a message queue object identifier
  *         or NULL in case of an error.
  *         The function can be called after kernel initialization with osKernelInitialize.
  *         It is possible to create message queue objects before the RTOS kernel is
  *         started with osKernelStart.
  *         The total amount of memory required for the message queue data is at least
  *         msg_count * msg_size. The msg_size is rounded up to a double even number to
  *         ensure 32-bit alignment of the memory blocks.
  *         The memory blocks allocated from the message queue have a fixed size defined
  *         with the parameter msg_size.
  *         Note : This function Cannot be called from Interrupt Service
  *         Routines.
  * @param  [in] msg_count maximum number of messages in queue.
  *         [in] msg_size maximum message size in bytes.
  *         [in] attr message queue attributes; NULL: default values.
  * @retval message queue ID for reference by other functions or NULL in case of error.
  */
osMessageQueueId_t osMessageQueueNew(uint32_t msg_count, uint32_t msg_size, const osMessageQueueAttr_t *attr)
{
  /* For TX_QUEUE the control block pointer is the queue identifier */
  TX_QUEUE *queue_ptr = NULL;
  /* Pointer to the message queue name */
  CHAR *name_ptr = NULL;
  /* Pointer to start address of the message queue data */
  VOID *queue_start = NULL;
  /* The message queue data size */
  ULONG queue_size = 0U;
  /* The message queue control block size */
  ULONG cb_size = 0U;

  /* Check if this API is called from Interrupt Service Routines or the msg_count
     is equal to 0 or msg_size is equal to 0 */
  if (!IS_IRQ_MODE() && (msg_count > 0) && (msg_size > 0))
  {
    /* Initialize the name_ptr to NULL */
    name_ptr = NULL;

    /* The msg_size is rounded up to a double even number to ensure 32-bit alignment of the memory blocks. */
    msg_size = msg_size + (msg_size % sizeof(ULONG));

    /* Check if the attr is not NULL */
    if (attr != NULL)
    {
      /* Check if the name_ptr is not NULL */
      if (attr->name != NULL)
      {
        /* Set the message queue name_ptr */
        name_ptr = (CHAR *)attr->name;
      }

      /* Check if the message queue data size is equal to 0 */
      if (attr->mq_size == 0U)
      {
        /* Set message queue data size to msg_count * msg_size (The total amount of memory required
           for the message queue data) */
        queue_size = msg_count * msg_size;
      }
      else if (attr->mq_size < (msg_count * msg_size))
      {
        /* Return NULL pointer in case of error */
        return (NULL);
      }
      else if (attr->mq_size < TX_BYTE_POOL_MIN)
      {
        /* Set message queue data size to TX_BYTE_POOL_MIN */
        queue_size = TX_BYTE_POOL_MIN;
      }
      else
      {
        /* Set message queue data size to attr->mq_size */
        queue_size = (ULONG)attr->mq_size;
      }

      /* Check if the input message queue data pointer is NULL */
      if (attr->mq_mem == NULL)
      {
        /* Allocate the data for the message queue to be created */
        queue_start = MemAlloc(queue_size, RTOS2_BYTE_POOL_STACK_TYPE);
        if (queue_start == NULL)
        {
          /* Return NULL pointer in case of error */
          return (NULL);
        }
      }
      else
      {
        if (attr->mq_size == 0U)
        {
          /* Return NULL pointer in case of error */
          return (NULL);
        }
        else
        {
          /* Set message queue data size to attr->mq_size */
          queue_size = (ULONG)attr->mq_size;
        }

        /* The message queue data shall point to the input message queue data memory address */
        queue_start = attr->mq_mem;
      }

      /* Check if the control block size is equal to 0 */
      if (attr->cb_size == 0U)
      {
        /* Set control block size to sizeof(TX_QUEUE) */
        cb_size = sizeof(TX_QUEUE);
      }
      else if (attr->cb_size < sizeof(TX_QUEUE))
      {
        /* Return NULL pointer in case of error */
        return (NULL);
      }
      else
      {
        /* Set message queue data size to attr->cb_size */
        cb_size = (ULONG)attr->cb_size;
      }

      /* Check if the input control block pointer is NULL */
      if (attr->cb_mem == NULL)
      {
        /* Allocate the queue_ptr structure for the message queue to be created */
        queue_ptr = (TX_QUEUE *)MemAlloc(cb_size, RTOS2_BYTE_POOL_HEAP_TYPE);
        if (queue_ptr == NULL)
        {
          /* Check if the memory for message queue data has been internally allocated */
          if (attr->mq_mem == NULL)
          {
            /* Free the already allocated memory for message queue data */
            MemFree(queue_start);
          }
          /* Return NULL pointer in case of error */
          return (NULL);
        }
      }
      else
      {
        /* The control block shall point to the input cb_mem memory address */
        queue_ptr = attr->cb_mem;
      }
    }
    else
    {
      /* Initialize the name_ptr to NULL */
      name_ptr = NULL;

      /* Initialize the message queue data size to msg_count * msg_size (The total amount of memory required
         for the message queue data) */
      queue_size = msg_count * msg_size;

      /* Allocate the data for the message queue to be created */
      queue_start = MemAlloc(queue_size, RTOS2_BYTE_POOL_STACK_TYPE);
      if (queue_start == NULL)
      {
        /* Return NULL pointer in case of error */
        return (NULL);
      }

      /* Allocate the queue_ptr structure for the message queue to be created */
      queue_ptr = (TX_QUEUE *)MemAlloc(sizeof(TX_QUEUE), RTOS2_BYTE_POOL_HEAP_TYPE);
      if (queue_ptr == NULL)
      {
        /* Free the already allocated memory for message queue data */
        MemFree(queue_start);

        /* Return NULL pointer in case of error */
        return (NULL);
      }
    }

    /* For threadX the message size is in 32-Bits */
    msg_size = msg_size / sizeof(ULONG);

    /* Call the tx_queue_create function to create the new message queue */
    if (tx_queue_create(queue_ptr, name_ptr, msg_size, queue_start, queue_size) != TX_SUCCESS)
    {
      /* Check if the memory for message queue control block has been internally
         allocated */
      if ((attr->cb_mem == NULL) || (attr == NULL))
      {
        /* Free the already allocated memory for message queue control block */
        MemFree(queue_ptr);
      }

      /* Check if the memory for message queue data has been internally allocated */
      if ((attr->mq_mem == NULL) || (attr == NULL))
      {
        /* Free the already allocated memory for message queue data */
        MemFree(queue_start);
      }

      /* Return NULL pointer in case of error */
      queue_ptr = NULL;
    }
  }

  return ((osMessageQueueId_t)queue_ptr);
}

/**
  * @brief  The function osMessageQueueGetName returns the pointer to the name
  *         string of the message queue identified by parameter mq_id or NULL in
  *         case of an error.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  [in] mq_id  message queue ID obtained by osMessageQueueNew.
  * @retval name_ptr as null-terminated string.
  */
const char *osMessageQueueGetName(osMessageQueueId_t mq_id)
{
  /* For TX_QUEUE the control block pointer is the queue identifier */
  TX_QUEUE *queue_ptr = (TX_QUEUE *)mq_id;
  /* The output name_ptr as null-terminated string */
  CHAR *name_ptr = NULL;

  /* Check if this API is called from Interrupt Service Routines, the mq_id is
     NULL or mq_id->tx_queue_id != TX_QUEUE_ID */
  if (IS_IRQ_MODE() || (queue_ptr == NULL) || (queue_ptr->tx_queue_id != TX_QUEUE_ID))
  {
    /* Return NULL in case of an error */
    name_ptr = NULL;
  }
  else
  {
    /* Call the tx_queue_info_get to get the queue name_ptr */
    if (tx_queue_info_get(queue_ptr, &name_ptr, NULL, NULL, NULL, NULL, NULL) != TX_SUCCESS)
    {
      /* Return NULL in case of an error */
      name_ptr = NULL;
    }
  }

  return (name_ptr);
}

/**
  * @brief  The function osMessageQueueGetCapacity returns the maximum number of
  *         messages in the message queue object specified by parameter mq_id or
  *         0 in case of an error.
  *         Note : This function may be called from Interrupt Service
  *         Routines.
  * @param  [in] mq_id  message queue ID obtained by osMessageQueueNew.
  * @retval maximum number of messages.
  */
uint32_t osMessageQueueGetCapacity(osMessageQueueId_t mq_id)
{
  /* For TX_QUEUE the control block pointer is the queue identifier */
  TX_QUEUE *queue_ptr = (TX_QUEUE *)mq_id;
  /* The specific maximum number of messages */
  uint32_t max_messages_number;

  /* Check if the mq_id is NULL or queue_ptr->tx_queue_id != TX_QUEUE_ID */
  if ((queue_ptr == NULL) || (queue_ptr->tx_queue_id != TX_QUEUE_ID))
  {
    /* Return 0 in case of error */
    max_messages_number = 0U;
  }
  else
  {
    /* Return the total number of messages in the queue */
    max_messages_number = queue_ptr->tx_queue_capacity;
  }

  return (max_messages_number);
}

/**
  * @brief  The function osMessageQueueGetMsgSize returns the maximum message
  *         size in bytes for the message queue object specified by parameter
  *         mq_id or 0 in case of an error.
  *         Note : This function may be called from Interrupt Service
  *         Routines.
  * @param  [in] mq_id  message queue ID obtained by osMessageQueueNew.
  * @retval maximum message size in bytes.
  */
uint32_t osMessageQueueGetMsgSize(osMessageQueueId_t mq_id)
{
  /* For TX_QUEUE the control block pointer is the queue identifier */
  TX_QUEUE *queue_ptr = (TX_QUEUE *)mq_id;
  /* The specific maximum message size in bytes */
  uint32_t max_messages_size;

  /* Check if the mq_id is NULL or queue_ptr->tx_queue_id != TX_QUEUE_ID */
  if ((queue_ptr == NULL) || (queue_ptr->tx_queue_id != TX_QUEUE_ID))
  {
    /* Return 0 in case of error */
    max_messages_size = 0U;
  }
  else
  {
    /* Return the message size that was specified in queue creation */
    max_messages_size = queue_ptr->tx_queue_message_size * sizeof(ULONG);
  }

  return (max_messages_size);
}

/**
  * @brief  The function osMessageQueueGetCount returns the number of queued
  *         messages in the message queue object specified by parameter mq_id or
  *         0 in case of an error.
  *         Note : This function may be called from Interrupt Service
  *         Routines.
  * @param  [in] mq_id  message queue ID obtained by osMessageQueueNew.
  * @retval number of queued messages.
  */
uint32_t osMessageQueueGetCount(osMessageQueueId_t mq_id)
{
  /* For TX_QUEUE the control block pointer is the queue identifier */
  TX_QUEUE *queue_ptr = (TX_QUEUE *)mq_id;
  /* The specific number of queued messages */
  uint32_t queued_messages_number = 0U;

  /* Check if the mq_id is NULL or queue_ptr->tx_queue_id != TX_QUEUE_ID */
  if ((queue_ptr == NULL) || (queue_ptr->tx_queue_id != TX_QUEUE_ID))
  {
    /* Return 0 in case of error */
    queued_messages_number = 0U;
  }
  else
  {
    /* Call the tx_queue_info_get to get the number of queued messages */
    if (tx_queue_info_get(queue_ptr, NULL, (ULONG *)&queued_messages_number, NULL, NULL, NULL, NULL) != TX_SUCCESS)
    {
      /* Return 0 in case of error */
      queued_messages_number = 0U;
    }
  }

  return (queued_messages_number);
}

/**
  * @brief  The function osMessageQueueGetSpace returns the number available
  *         slots for messages in the message queue object specified by
  *         parameter mq_id or 0 in case of an error.
  *         Note : This function may be called from Interrupt Service
  *         Routines.
  * @param  [in] mq_id  message queue ID obtained by osMessageQueueNew.
  * @retval number of available slots for messages.
  */
uint32_t osMessageQueueGetSpace(osMessageQueueId_t mq_id)
{
  /* For TX_QUEUE the control block pointer is the queue identifier */
  TX_QUEUE *queue_ptr = (TX_QUEUE *)mq_id;
  /* The specific number of queued messages */
  uint32_t queue_messages_free_slots = 0U;

  /* Check if the mq_id is NULL or queue_ptr->tx_queue_id != TX_QUEUE_ID */
  if ((queue_ptr == NULL) || (queue_ptr->tx_queue_id != TX_QUEUE_ID))
  {
    /* Return 0 in case of error */
    queue_messages_free_slots = 0U;
  }
  else
  {
    /* Call the tx_queue_info_get to get the number of available slots for
       messages */
    if (tx_queue_info_get(queue_ptr, NULL, NULL, (ULONG *)&queue_messages_free_slots, NULL, NULL, NULL) != TX_SUCCESS)
    {
      /* Return 0 in case of error */
      queue_messages_free_slots = 0U;
    }
  }

  return (queue_messages_free_slots);
}

/**
  * @brief  The function osMessageQueueReset resets the message queue specified
  *         by the parameter mq_id.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  [in] mq_id message queue ID obtained by osMessageQueueNew.
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osMessageQueueReset(osMessageQueueId_t mq_id)
{
  /* For TX_QUEUE the control block pointer is the queue identifier */
  TX_QUEUE *queue_ptr = (TX_QUEUE *)mq_id;
  /* The returned status or error */
  osStatus_t status;

  /* Check if this API is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    /* Return osErrorISR error */
    status = osErrorISR;
  }
  /* Check if the mq_id is NULL or queue_ptr->tx_queue_id != TX_QUEUE_ID */
  else if ((queue_ptr == NULL) || (queue_ptr->tx_queue_id != TX_QUEUE_ID))
  {
    /* Return osErrorParameter error */
    status = osErrorParameter;
  }
  else
  {
    /* Call the tx_queue_flush deletes all messages stored in the specified
       message queue */
    if (tx_queue_flush(queue_ptr) == TX_SUCCESS)
    {
      /* Return osOK in case of success */
      status = osOK;
    }
    else
    {
      /* Return osErrorResource in case of error */
      status = osErrorResource;
    }
  }

  return (status);
}

/**
  * @brief  The function osMessageQueueDelete deletes a message queue object
  *         specified by parameter mq_id. It releases internal memory obtained
  *         for message queue handling. After this call, the mq_id is no longer
  *         valid and cannot be used. The message queue may be created again
  *         using the function osMessageQueueNew.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  [in] mq_id message queue ID obtained by osMessageQueueNew.
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osMessageQueueDelete(osMessageQueueId_t mq_id)
{
  /* For TX_QUEUE the control block pointer is the queue identifier */
  TX_QUEUE *queue_ptr = (TX_QUEUE *)mq_id;
  /* The returned status or error */
  osStatus_t status;

  /* Check if this API is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    /* Return osErrorISR error */
    status = osErrorISR;
  }
  /* Check if the mq_id is NULL or queue_ptr->tx_queue_id != TX_QUEUE_ID */
  else if ((queue_ptr == NULL) || (queue_ptr->tx_queue_id != TX_QUEUE_ID))
  {
    /* Return osErrorParameter error */
    status = osErrorParameter;
  }
  else
  {
    /* Call the tx_queue_delete deletes the specified message queue */
    if (tx_queue_delete(queue_ptr) == TX_SUCCESS)
    {
      /* Free the already allocated memory for message queue data */
      MemFree(queue_ptr->tx_queue_start);

      /* Free the already allocated memory for message queue control block */
      MemFree(queue_ptr);

      /* Return osOK in case of success */
      status = osOK;
    }
    else
    {
      /* Return osErrorResource in case of error */
      status = osErrorResource;
    }
  }

  return (status);
}

/**
  * @brief  The blocking function osMessageQueuePut puts the message pointed to
  *         by msg_ptr into the the message queue specified by parameter mq_id.
  *         The parameter msg_prio is used to sort message according their
  *         priority (higher numbers indicate a higher priority) on insertion.
  *         The parameter timeout specifies how long the system waits to put the
  *         message into the queue. While the system waits, the thread that is
  *         calling this function is put into the BLOCKED state. The parameter
  *         timeout can have the following values:
  *           * when timeout is 0, the function returns instantly (i.e. try
  *           semantics).
  *           * when timeout is set to osWaitForever the function will wait for an
  *           infinite time until the message is delivered (i.e. wait semantics).
  *           * all other values specify a time in kernel ticks for a timeout
  *           (i.e. timed-wait semantics).
  *         Note : This function may be called from Interrupt Service
  *         Routines.
  * @param  [in] mq_id message queue ID obtained by osMessageQueueNew.
  *         [in] msg_ptr pointer to buffer with message to put into a queue.
  *         [in] msg_prio message priority.
  *         [in] timeout Timeout Value or 0 in case of no time-out.
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osMessageQueuePut(osMessageQueueId_t mq_id, const void *msg_ptr, uint8_t msg_prio, uint32_t timeout)
{
  /* For TX_QUEUE the control block pointer is the queue identifier */
  TX_QUEUE *queue_ptr = (TX_QUEUE *)mq_id;
  /* The returned value from ThreadX call */
  UINT tx_status;
  /* The returned status or error */
  osStatus_t status;

  /* Check if the mq_id is NULL or queue_ptr->tx_queue_id != TX_QUEUE_ID or non-zero timeout specified in an ISR */
  if ((IS_IRQ_MODE() && (timeout != 0)) || (queue_ptr == NULL) || (queue_ptr->tx_queue_id != TX_QUEUE_ID) ||
      (msg_ptr == NULL))
  {
    /* Return osErrorParameter error */
    status = osErrorParameter;
  }
  else
  {
    /* Call the tx_queue_send to send a message to the specified message queue */
    tx_status = tx_queue_send(queue_ptr, (void *)msg_ptr, timeout);
    if (tx_status == TX_SUCCESS)
    {
      /* Return osOK in case of success */
      status = osOK;
    }
    else if (tx_status == TX_QUEUE_FULL)
    {
      /* Return osErrorResource in case of error */
      status = osErrorResource;
    }
    else
    {
      /* Return osErrorTimeout in case of error */
      status = osErrorTimeout;
    }
  }

  return (status);
}

/**
  * @brief  The function osMessageQueueGet retrieves a message from the message
  *         queue specified by the parameter mq_id and saves it to the buffer pointed
  *         to by the parameter msg_ptr. The message priority is stored to parameter
  *         msg_prio if not token{NULL}.
  *         The parameter timeout specifies how long the system waits to retrieve
  *         the message from the queue. While the system waits, the thread that is
  *         calling this function is put into the BLOCKED state.
  *         The parameter timeout can have the following values:
  *           * when timeout is 0, the function returns instantly (i.e. try
  *           semantics).
  *           * when timeout is set to osWaitForever the function will wait for an
  *           infinite time until the message is delivered (i.e. wait semantics).
  *           * all other values specify a time in kernel ticks for a timeout
  *           (i.e. timed-wait semantics).
  *         Note : This function may be called from Interrupt Service
  *         Routines.
  * @param  [in] mq_id  message queue ID obtained by osMessageQueueNew.
  *         [out] msg_ptr pointer to buffer for message to get from a queue.
  *         [out] msg_prio  pointer to buffer for message priority or NULL.
  *         [in] timeout  Timeout Value or 0 in case of no time-out.
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osMessageQueueGet(osMessageQueueId_t mq_id, void *msg_ptr, uint8_t *msg_prio, uint32_t timeout)
{
  /* For TX_QUEUE the control block pointer is the queue identifier */
  TX_QUEUE *queue_ptr = (TX_QUEUE *)mq_id;
  /* The returned value from ThreadX call */
  UINT tx_status;
  /* The returned status or error */
  osStatus_t status;

  /* Check if the mq_id is NULL or queue_ptr->tx_queue_id != TX_QUEUE_ID or non-zero timeout specified in an ISR */
  if ((IS_IRQ_MODE() && (timeout != 0)) || (queue_ptr == NULL) || (queue_ptr->tx_queue_id != TX_QUEUE_ID) ||
      (msg_ptr == NULL))
  {
    /* Return osErrorParameter error */
    status = osErrorParameter;
  }
  else
  {
    /* Call the tx_queue_receive to retrieves a message from the specified message queue */
    tx_status = tx_queue_receive(queue_ptr, msg_ptr, timeout);
    if (tx_status == TX_SUCCESS)
    {
      /* Return osOK in case of success */
      status = osOK;
    }
    else if (tx_status == TX_QUEUE_EMPTY)
    {
      /* Return osErrorTimeout in case of error */
      status = osErrorTimeout;
    }
    else if (tx_status == TX_WAIT_ERROR)
    {
      /* Return osErrorParameter when a non-zero timeout is used in ISR context */
      status = osErrorParameter;
    }
    else
    {
      /* Return osErrorResource in case of error */
      status = osErrorResource;
    }
  }

  return (status);
}

/*---------------------------------------------------------------------------*/
/*---------------------------Memory Pool Management APIs---------------------*/
/*---------------------------------------------------------------------------*/
/**
  * @brief  The function osMemoryPoolNew creates and initializes a memory pool
  *         object and returns the pointer to the memory pool object identifier
  *         or NULL in case of an error. It can be safely called before the RTOS
  *         is started (call to osKernelStart), but not before it is initialized
  *         (call to osKernelInitialize).
  *         The total amount of memory needed is at least block_count * block_size.
  *         Memory from the pool can only be allocated/freed in fixed portions
  *         of block_size.
  *         Note : This function cannot be called from Interrupt Service Routines.
  * @param  [in] block_count maximum number of memory blocks in memory pool.
  *         [in] block_size memory block size in bytes.
  *         [in] attr memory pool attributes; NULL: default values.
  * @retval memory pool ID for reference by other functions or NULL in case of error.
  */
osMemoryPoolId_t osMemoryPoolNew (uint32_t block_count, uint32_t block_size, const osMemoryPoolAttr_t *attr)
{
  /* For TX_BLOCK_POOL the control block pointer is the memory pool identifier */
  TX_BLOCK_POOL *block_pool_ptr = NULL;
  /* Pointer to memory pool named */
  CHAR *name_ptr = NULL;
  /* Pointer to start address of the memory pool storage data */
  VOID * pool_start = NULL;
  /* The memory pool data size */
  UINT pool_size = 0U;
  /* The memory pool control block size */
  ULONG cb_size = 0U;

  /* Check if this API is called from Interrupt Service Routines or the block_count
     is equal to 0 or block_size is equal to 0 */
  if ((!IS_IRQ_MODE() && (block_size != 0) && (block_count != 0)))
  {
     /* Initialize the name_ptr to NULL */
     name_ptr = NULL;

     if (attr != NULL)
     {
      /* Check if the name_ptr is not NULL */
      if (attr->name != NULL)
      {
        /* Set the message queue name_ptr */
        name_ptr = (CHAR *)attr->name;
      }
      /* Check if the memory pool data storage size is equal to 0 */
      if (attr->mp_size == 0U)
      {
        /* Set memory pool data size to block_count * block_size (The total amount of memory required
           for the memory pool data storage) */
        pool_size = block_count * block_size;
      }
      else if (attr->mp_size < (pool_size))
      {
        /* Return NULL pointer in case of error */
        return (NULL);
      }
      else if (attr->mp_size < TX_BYTE_POOL_MIN)
      {
        /* Set memory pool data size to TX_BYTE_POOL_MIN */
        pool_size = TX_BYTE_POOL_MIN;
      }
      else
      {
        /* Set memory pool data size to attr->mp_size */
        pool_size = (ULONG)attr->mp_size;
      }
      /* Check if the input memory pool data pointer is NULL */
      if (attr->mp_mem == NULL)
      {
        /* Allocate the data for the memory pool to be created */
        pool_start = MemAlloc(pool_size, RTOS2_BYTE_POOL_STACK_TYPE);
        if (pool_start == NULL)
        {
          /* Return NULL pointer in case of error */
          return (NULL);
        }
      }
      else
      {
        if (attr->mp_size == 0U)
        {
          /* Return NULL pointer in case of error */
          return (NULL);
        }
        else
        {
          /* Set memory pool data size to attr->mp_size */
          pool_size = (ULONG)attr->mp_size;
        }

        /* The memory pool shall point to the memory pool data memory address */
        pool_start = attr->mp_mem;
      }
      /* Check if the control block size is equal to 0 */
      if (attr->cb_size == 0U)
      {
        /* Set control block size to sizeof(TX_BLOCK_POOL) */
        cb_size = sizeof(TX_BLOCK_POOL);
      }
      else if (attr->cb_size < sizeof(TX_BLOCK_POOL))
      {
        /* Return NULL pointer in case of error */
        return (NULL);
      }
      else
      {
        /* Set memory pool data size to attr->cb_size */
        cb_size = (ULONG)attr->cb_size;
      }
      /* Check if the input control block pointer is NULL */
      if (attr->cb_mem == NULL)
      {
        /* Allocate the block_pool_ptr structure for the memory pool to be created */
        block_pool_ptr = (TX_BLOCK_POOL *)MemAlloc(cb_size, RTOS2_BYTE_POOL_HEAP_TYPE);
        if (block_pool_ptr == NULL)
        {
          /* Check if the memory for memory pool data has been internally allocated */
          if (attr->mp_mem == NULL)
          {
            /* Free the already allocated memory for memory pool data */
            MemFree(pool_start);
          }
          /* Return NULL pointer in case of error */
          return (NULL);
        }
      }
      else
      {
        /* The control block shall point to the input cb_mem memory address */
        block_pool_ptr = attr->cb_mem;
      }
    }
    else /* attr == NULL*/
    {
      /* Initialize the name_ptr to NULL */
      name_ptr = NULL;

      /* Initialize the memory pool data size to block_count * block_size (The total amount of memory required
           for the memory pool data storage) */
      pool_size = block_count * block_size;

      /* Allocate the data for pool_start to be created */
      pool_start = MemAlloc(pool_size, RTOS2_BYTE_POOL_STACK_TYPE);

      if (pool_start == NULL)
      {
        /* Return NULL pointer in case of error */
        return (NULL);
      }
      /* Allocate the block_pool_ptr structure for the block pool to be created */
      block_pool_ptr = (TX_BLOCK_POOL *)MemAlloc(sizeof(TX_BLOCK_POOL), RTOS2_BYTE_POOL_HEAP_TYPE);

      if (block_pool_ptr == NULL)
      {
        /* Free the already allocated memory for block pool data */
        MemFree(pool_start);
        /* Return NULL pointer in case of error */
        return (NULL);
      }
    }
    /* Call the tx_block_pool_create function to create the new  memory pool */
    if (tx_block_pool_create(block_pool_ptr, name_ptr, block_size, pool_start, pool_size) != TX_SUCCESS)
    {
      /* Check if the memory for  memory pool control block has been internally
         allocated */
      if ((attr->cb_mem == NULL) || (attr == NULL))
      {
        /* Free the already allocated memory for  memory pool control block */
        MemFree(block_pool_ptr);
      }

      /* Check if the memory for  memory pool data has been internally allocated */
      if ((attr->mp_mem == NULL) || (attr == NULL))
      {
        /* Free the already allocated memory for memory pool data */
        MemFree(pool_start);
      }

      /* Return NULL pointer in case of error */
      block_pool_ptr = NULL;
    }
  }
  else
  {
    block_pool_ptr = NULL;
  }

  return((osMemoryPoolId_t)(block_pool_ptr));
}

/**
  * @brief  The function osMemoryPoolGetName returns the pointer to the name
  *         string of the memory pool identified by parameter mp_id or NULL in
  *         case of an error.
  *         Note : This function cannot be called from Interrupt Service
  *         Routines.
  * @param  [in] mp_id memory pool ID obtained by osMemoryPoolNew.
  * @retval maximum number of memory blocks in the memory pool object.
  */
const char *osMemoryPoolGetName(osMemoryPoolId_t mp_id)
{
  /* For TX_BLOCK_POOL the control block pointer is the Block pool identifier */
  TX_BLOCK_POOL *block_pool_ptr = (TX_BLOCK_POOL *)mp_id;
  /* The output name_ptr as null-terminated string */
  CHAR *name_ptr = NULL;

  /* Check if this API is called from Interrupt Service Routines, the mq_id is
     NULL or mp_id->tx_block_pool_id != TX_BLOCK_POOL_ID */
  if (IS_IRQ_MODE() || (block_pool_ptr == NULL) || (block_pool_ptr->tx_block_pool_id != TX_BLOCK_POOL_ID))
  {
    /* Return NULL in case of an error */
    name_ptr = NULL;
  }
  else
  {
    /* Call the tx_block_pool_info_get to get the block_pool name_ptr */
    if (tx_block_pool_info_get(block_pool_ptr, &name_ptr, TX_NULL,TX_NULL, TX_NULL,TX_NULL, TX_NULL)!= TX_SUCCESS)
    {
      /* Return NULL in case of an error */
      name_ptr = NULL;
    }
  }

  return (name_ptr);
}
/**
  * @brief  The blocking function osMemoryPoolAlloc allocates the memory pool
  *         parameter mp_id and returns a pointer to the address of the allocated
  *         memory or 0 in case of an error.
  *         The parameter timeout specifies how long the system waits to allocate
  *         the memory. While the system waits, the thread that is calling this
  *         function is put into the BLOCKED state. The thread will become READY
  *         as soon as at least one block of memory gets available.
  *         The parameter timeout can have the following values:
  *          - when timeout is 0, the function returns instantly (i.e. try semantics).
  *          - when timeout is set to osWaitForever the function will wait for an
  *            infinite time until the memory is allocated (i.e. wait semantics).
  *          - All other values specify a time in kernel ticks for a timeout
  *           (i.e. timed-wait semantics).
  *         Note :
  *         - This function may be called from Interrupt Service Routines.
  *         - It is in the responsibility of the user to respect the block size,
  *         i.e. not access memory beyond the blocks limit.
  * @param  [in] mp_id memory pool ID obtained by osMemoryPoolNew.
  * @param  [in] timeout Timeout Value or 0 in case of no time-out.
  * @retval address of the allocated memory block or NULL in case of no memory
  *         is available.
  */
void * osMemoryPoolAlloc(osMemoryPoolId_t mp_id, uint32_t timeout)
{
  /* For TX_BLOCK_POOL the control block pointer is the Block pool identifier */
  TX_BLOCK_POOL *block_pool_ptr = (TX_BLOCK_POOL *)mp_id;

  /* The output name_ptr as null-terminated string */
  void *block;

  /* Check if the mq_id is NULL or mp_id->tx_block_pool_id != TX_BLOCK_POOL_ID */
  if((block_pool_ptr == NULL) || (block_pool_ptr->tx_block_pool_id != TX_BLOCK_POOL_ID))
  {
    /* Return NULL in case of an error */
    block = NULL;
  }
  else
  {
    block = NULL;
    /* Get a block from the free-list */
    if(tx_block_allocate(block_pool_ptr, &block, timeout) != TX_SUCCESS)
    {
      /* Return NULL in case of an error */
      block = NULL;
    }
  }
  return (block);
}
/**
  * @brief  The function osMemoryPoolFree frees the memory pool block specified
  *         by the parameter block in the memory pool object specified by the
  *         parameter mp_id. The memory block is put back to the list of
  *         available blocks
  *         If another thread is waiting for memory to become available the
  *         thread is put to READY state.
  *         Possible osStatus_t return values:
  *           - osOK: the memory has been freed.
  *           - oosErrorParameter: parameter mp_id is NULL or invalid, block
  *              points to invalid memory.
  *           - oosErrorResource: the memory pool is in an invalid state.
  *         Note : osMemoryPoolFree may perform certain checks on the block
  *                pointer given. But using osMemoryPoolFree with a pointer other
  *                than one received from osMemoryPoolAlloc has UNPREDICTED behaviour.
  *                This function may be called from Interrupt Service Routines.
  * @param  [in] mp_id memory pool ID obtained by osMemoryPoolNew.
  *         [in] [in] block address of the allocated memory block to be
  *         returned to the memory pool
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osMemoryPoolFree(osMemoryPoolId_t mp_id, void * block)
{
   /* For TX_BLOCK_POOL the control block pointer is the Block pool identifier */
   TX_BLOCK_POOL *block_pool_ptr = (TX_BLOCK_POOL *)mp_id;

   osStatus_t status;

  /* Check if the mq_id is NULL or mp_id->tx_block_pool_id != TX_BLOCK_POOL_ID */
  if((block_pool_ptr == NULL) || (block_pool_ptr->tx_block_pool_id != TX_BLOCK_POOL_ID) || (block == NULL))
  {
    /* Invalid input parameters */
    status = osErrorParameter;
  }
  else
  {
    if(tx_block_release((VOID *)block) != TX_SUCCESS)
    {
      status = osErrorResource;
    }
    else
    {
      status = osOK;
    }
  }
  return (status);
}
/**
  * @brief  The function osMemoryPoolGetCapacity returns the maximum number
  *         of memory blocks in the memory pool object specified by
  *         parameter mp_id or 0 in case of an error.
  *         Note : This function may be called from Interrupt Service
  *         Routines.
  * @param  [in] mp_id memory pool ID obtained by osMemoryPoolNew.
  * @retval maximum number of memory blocks in the memory pool object.
  */
uint32_t osMemoryPoolGetCapacity(osMemoryPoolId_t mp_id)
{
  /* For TX_BLOCK_POOL the control block pointer is the Block pool identifier */
  TX_BLOCK_POOL *block_pool_ptr = (TX_BLOCK_POOL *)mp_id;
  /* The output name_ptr as null-terminated string */
  ULONG total_blocks = 0;

  /* Check if this API is called from Interrupt Service Routines, the mq_id is
     NULL or mp_id->tx_block_pool_id != TX_BLOCK_POOL_ID */
  if ((block_pool_ptr == NULL) || (block_pool_ptr->tx_block_pool_id != TX_BLOCK_POOL_ID))
  {
    /* Return 0 in case of an error */
    total_blocks = 0;
  }
  else
  {
    /* Call the tx_block_pool_info_get to get the total_blocks */
    if (tx_block_pool_info_get(block_pool_ptr, TX_NULL, TX_NULL, &total_blocks, TX_NULL,TX_NULL, TX_NULL)!= TX_SUCCESS)
    {
      /* Return 0 in case of an error */
      total_blocks = 0;
    }
  }

  return (total_blocks);
}
/**
  * @brief  The function osMemoryPoolGetBlockSize returns the memory block size
  *         in bytes in the memory pool object specified by parameter mp_id
  *         or 0 in case of an error.
  *         Note : This function may be called from Interrupt Service
  *         Routines.
  * @param  [in] mp_id memory pool ID obtained by osMemoryPoolNew.
  * @retval memory block size in bytes.
  */
uint32_t osMemoryPoolGetBlockSize(osMemoryPoolId_t mp_id)
{
  /* For TX_BLOCK_POOL the control block pointer is the Block pool identifier */
  TX_BLOCK_POOL *block_pool_ptr = (TX_BLOCK_POOL *)mp_id;
  /* The output name_ptr as null-terminated string */
  uint32_t block_pool_size = 0;

  /* Check if this API is called from Interrupt Service Routines, the mq_id is
     NULL or mp_id->tx_block_pool_id != TX_BLOCK_POOL_ID */
  if ((block_pool_ptr == NULL) || (block_pool_ptr->tx_block_pool_id != TX_BLOCK_POOL_ID))
  {
    /* Return 0 in case of an error */
    block_pool_size = 0;
  }
  else
  {
    block_pool_size = block_pool_ptr->tx_block_pool_block_size;
  }

  return (block_pool_size);
}
/**
  * @brief  The function osMemoryPoolGetCount returns the number of memory blocks
  *         used in the memory pool object specified by parameter mp_id or 0 in
  *         case of an error.
  *         Note : This function may be called from Interrupt Service
  *         Routines.
  * @param  [in] mp_id memory pool ID obtained by osMemoryPoolNew.
  * @retval number of memory blocks used.
  */
uint32_t osMemoryPoolGetCount(osMemoryPoolId_t mp_id)
{
  /* For TX_BLOCK_POOL the control block pointer is the Block pool identifier */
  TX_BLOCK_POOL *block_pool_ptr = (TX_BLOCK_POOL *)mp_id;
  /* The output name_ptr as null-terminated string */
  ULONG block_pool_total = 0;
  ULONG block_pool_available = 0;
  ULONG block_pool_used = 0;

  /* Check if this API is called from Interrupt Service Routines, the mq_id is
     NULL or mp_id->tx_block_pool_id != TX_BLOCK_POOL_ID */
  if ((block_pool_ptr == NULL) || (block_pool_ptr->tx_block_pool_id != TX_BLOCK_POOL_ID))
  {
    /* Return 0 in case of an error */
    block_pool_used = 0;
  }
  else
  {
   /* Call the tx_block_pool_info_get to get the block_pool name_ptr */
    if (tx_block_pool_info_get(block_pool_ptr, TX_NULL, &block_pool_available, &block_pool_total, TX_NULL, TX_NULL, TX_NULL)!= TX_SUCCESS)
    {
      /* Return 0 in case of an error */
      block_pool_used = 0;
    }
    else
    {
      /* Return number of used blocks */
      block_pool_used = (block_pool_total - block_pool_available);
    }
  }

  return ((uint32_t)(block_pool_used));
}
/**
  * @brief  The function osMemoryPoolGetSpace returns the number of memory blocks
  *         available in the memory pool object specified by parameter mp_id
  *         or 0 in case of an error.
  *         Note : This function may be called from Interrupt Service
  *         Routines.
  * @param  [in] mp_id memory pool ID obtained by osMemoryPoolNew.
  * @retval number of memory blocks available.
  */
uint32_t osMemoryPoolGetSpace(osMemoryPoolId_t mp_id)
{
  /* For TX_BLOCK_POOL the control block pointer is the Block pool identifier */
  TX_BLOCK_POOL *block_pool_ptr = (TX_BLOCK_POOL *)mp_id;
  /* The output name_ptr as null-terminated string */
  ULONG block_pool_available = 0;

  /* Check if this API is called from Interrupt Service Routines, the mq_id is
     NULL or mp_id->tx_block_pool_id != TX_BLOCK_POOL_ID */
  if ((block_pool_ptr == NULL) || (block_pool_ptr->tx_block_pool_id != TX_BLOCK_POOL_ID))
  {
    /* Return 0 in case of an error */
    block_pool_available = 0;
  }
  else
  {
   /* Call the tx_block_pool_info_get to get the block_pool name_ptr */
    if (tx_block_pool_info_get(block_pool_ptr, TX_NULL, &block_pool_available, TX_NULL, TX_NULL, TX_NULL, TX_NULL)!= TX_SUCCESS)
    {
      /* Return 0 in case of an error */
      block_pool_available = 0;
    }
  }

  return (block_pool_available);
}
/**
  * @brief  The function osMemoryPoolDelete deletes a memory pool object
  *         specified by parameter mp_id. It releases internal memory obtained
  *         for memory pool handling. After this call, the mp_id is no longer
  *         valid and cannot be used. The memory pool may be created again using
  *         the function osMemoryPoolNew
  *         Possible osStatus_t return values:
  *           - osOK: the memory pool object has been deleted.
  *           - osErrorParameter: parameter mp_id is NULL or invalid.
  *           - osErrorResource: the memory pool is in an invalid state.
  *           - osErrorISR: osMemoryPoolDelete cannot be called from interrupt service routines.
  *         Note : This function cannot be called from Interrupt Service Routines.
  * @param  [in] mp_id memory pool ID obtained by osMemoryPoolNew.
  * @retval status code that indicates the execution status of the function.
  */
osStatus_t osMemoryPoolDelete(osMemoryPoolId_t mp_id)
{
  /* For TX_BLOCK_POOL the control block pointer is the Block pool identifier */
  TX_BLOCK_POOL *block_pool_ptr = (TX_BLOCK_POOL *)mp_id;

  osStatus_t status;

  /* Check if this API is called from Interrupt Service Routines, the mq_id is
     NULL or mp_id->tx_block_pool_id != TX_BLOCK_POOL_ID */
  if ((block_pool_ptr == NULL) || (block_pool_ptr->tx_block_pool_id != TX_BLOCK_POOL_ID))
  {
    status = osErrorParameter;
  }
  else if(IS_IRQ_MODE())
  {
    status = osErrorISR;
  }
  else
  {
    if(tx_block_pool_delete(block_pool_ptr) != TX_SUCCESS)
    {
      status = osErrorResource;
    }
    else
    {
      status = osOK;
    }
  }
  return(status);
}

/*---------------------------------------------------------------------------*/
/*------------------------tx_application_define API--------------------------*/
/*---------------------------------------------------------------------------*/
__WEAK VOID tx_application_define(VOID *first_unused_memory)
{
  /* Empty tx_application_define() */
}

