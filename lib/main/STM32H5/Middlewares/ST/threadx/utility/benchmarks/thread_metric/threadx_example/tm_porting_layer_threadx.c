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
/** Thread-Metric Component                                               */
/**                                                                       */
/**   Porting Layer (ThreadX Example)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/* Turn off ThreadX error checking.  */

#ifndef TX_DISABLE_ERROR_CHECKING
#define TX_DISABLE_ERROR_CHECKING
#endif


/* For smallest size, the ThreadX library and application code should be built 
   with the following options defined (easiest to add in tx_port.h):

#define TX_ENABLE_EXECUTION_CHANGE_NOTIFY
#define TX_DISABLE_PREEMPTION_THRESHOLD
#define TX_DISABLE_NOTIFY_CALLBACKS
#define TX_DISABLE_REDUNDANT_CLEARING
#define TX_DISABLE_STACK_FILLING
#define TX_NOT_INTERRUPTABLE
#define TX_TIMER_PROCESS_IN_ISR

  For the fastest performance, these additional options should also be used:

#define TX_REACTIVATE_INLINE
#define TX_INLINE_THREAD_RESUME_SUSPEND

*/



/* Include necessary files.  */

#include    "tx_api.h"
#include    "tm_api.h"


/* Define ThreadX mapping constants.  */

#define TM_THREADX_MAX_THREADS          10
#define TM_THREADX_MAX_QUEUES           1
#define TM_THREADX_MAX_SEMAPHORES       1
#define TM_THREADX_MAX_MEMORY_POOLS     1


/* Define the default ThreadX stack size.  */

#define TM_THREADX_THREAD_STACK_SIZE    2096


/* Define the default ThreadX queue size.  */

#define TM_THREADX_QUEUE_SIZE           200


/* Define the default ThreadX memory pool size.  */

#define TM_THREADX_MEMORY_POOL_SIZE     2048


/* Define the number of timer interrupt ticks per second.  */

#define TM_THREADX_TICKS_PER_SECOND     100


/* Define ThreadX data structures.  */

TX_THREAD       tm_thread_array[TM_THREADX_MAX_THREADS];
TX_QUEUE        tm_queue_array[TM_THREADX_MAX_QUEUES];
TX_SEMAPHORE    tm_semaphore_array[TM_THREADX_MAX_SEMAPHORES];
TX_BLOCK_POOL   tm_block_pool_array[TM_THREADX_MAX_MEMORY_POOLS];


/* Define ThreadX object data areas.  */

unsigned char   tm_thread_stack_area[TM_THREADX_MAX_THREADS*TM_THREADX_THREAD_STACK_SIZE];
unsigned char   tm_queue_memory_area[TM_THREADX_MAX_QUEUES*TM_THREADX_QUEUE_SIZE];
unsigned char   tm_pool_memory_area[TM_THREADX_MAX_MEMORY_POOLS*TM_THREADX_MEMORY_POOL_SIZE];


/* Define array to remember the test entry function.  */

void           *tm_thread_entry_functions[TM_THREADX_MAX_THREADS];


/* Remember the test initialization function.  */

void            (*tm_initialization_function)(void);


/* Define our shell entry function to match ThreadX.  */

VOID  tm_thread_entry(ULONG thread_input);


/* This function called from main performs basic RTOS initialization, 
   calls the test initialization function, and then starts the RTOS function.  */
void  tm_initialize(void (*test_initialization_function)(void))
{

    /* Save the test initialization function.  */
    tm_initialization_function =  test_initialization_function;

    /* Call the previously defined initialization function.  */
    (tm_initialization_function)();   
}


/* This function takes a thread ID and priority and attempts to create the
   file in the underlying RTOS.  Valid priorities range from 1 through 31, 
   where 1 is the highest priority and 31 is the lowest. If successful, 
   the function should return TM_SUCCESS. Otherwise, TM_ERROR should be returned.   */
int  tm_thread_create(int thread_id, int priority, void (*entry_function)(void))
{

UINT    status;

    /* Remember the actual thread entry.  */
    tm_thread_entry_functions[thread_id] =  (void *) entry_function;

    /* Create the thread under ThreadX.  */
    status =  tx_thread_create(&tm_thread_array[thread_id], "Thread-Metric test", tm_thread_entry, (ULONG) thread_id,
                    &tm_thread_stack_area[thread_id*TM_THREADX_THREAD_STACK_SIZE], TM_THREADX_THREAD_STACK_SIZE,
                    (UINT) priority, (UINT) priority, TX_NO_TIME_SLICE, TX_DONT_START);

    /* Determine if the thread create was successful.  */
    if (status == TX_SUCCESS)
        return(TM_SUCCESS);
    else
        return(TM_ERROR);
}


/* This function resumes the specified thread.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_thread_resume(int thread_id)
{

UINT    status;


    /* Attempt to resume the thread.  */
    status =  tx_thread_resume(&tm_thread_array[thread_id]);

    /* Determine if the thread resume was successful.  */
    if (status == TX_SUCCESS)
        return(TM_SUCCESS);
    else
        return(TM_ERROR);
}


/* This function suspends the specified thread.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_thread_suspend(int thread_id)
{

UINT    status;


    /* Attempt to suspend the thread.  */
    status =  tx_thread_suspend(&tm_thread_array[thread_id]);

    /* Determine if the thread suspend was successful.  */
    if (status == TX_SUCCESS)
        return(TM_SUCCESS);
    else
        return(TM_ERROR);
}


/* This function relinquishes to other ready threads at the same
   priority.  */
void tm_thread_relinquish(void)
{

    /* Relinquish to other threads at the same priority.  */
    tx_thread_relinquish();
}


/* This function suspends the specified thread for the specified number
   of seconds.  If successful, the function should return TM_SUCCESS. 
   Otherwise, TM_ERROR should be returned.  */
void tm_thread_sleep(int seconds)
{

    /* Attempt to sleep.  */
    tx_thread_sleep(((UINT) seconds)*TM_THREADX_TICKS_PER_SECOND);
}


/* This function creates the specified queue.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_queue_create(int queue_id)
{

UINT    status;


    /* Create the specified queue with 16-byte messages.  */
    status =  tx_queue_create(&tm_queue_array[queue_id], "Thread-Metric test", TX_4_ULONG, 
                              &tm_queue_memory_area[queue_id*TM_THREADX_QUEUE_SIZE], TM_THREADX_QUEUE_SIZE);

    /* Determine if the queue create was successful.  */
    if (status == TX_SUCCESS)
        return(TM_SUCCESS);
    else
        return(TM_ERROR);
}


/* This function sends a 16-byte message to the specified queue.  If successful, 
   the function should return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_queue_send(int queue_id, unsigned long *message_ptr)
{

UINT    status;


    /* Send the 16-byte message to the specified queue.  */
    status =  tx_queue_send(&tm_queue_array[queue_id], message_ptr, TX_NO_WAIT);

    /* Determine if the queue send was successful.  */
    if (status == TX_SUCCESS)
        return(TM_SUCCESS);
    else
        return(TM_ERROR);
}


/* This function receives a 16-byte message from the specified queue.  If successful, 
   the function should return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_queue_receive(int queue_id, unsigned long *message_ptr)
{

UINT    status;


    /* Receive a 16-byte message from the specified queue.  */
    status =  tx_queue_receive(&tm_queue_array[queue_id], message_ptr, TX_NO_WAIT);

    /* Determine if the queue receive was successful.  */
    if (status == TX_SUCCESS)
        return(TM_SUCCESS);
    else
        return(TM_ERROR);
}


/* This function creates the specified semaphore.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_semaphore_create(int semaphore_id)
{

UINT    status;


    /*  Create semaphore.  */
    status =  tx_semaphore_create(&tm_semaphore_array[semaphore_id], "Thread-Metric test", 1);

    /* Determine if the semaphore create was successful.  */
    if (status == TX_SUCCESS)
        return(TM_SUCCESS);
    else
        return(TM_ERROR);
}


/* This function gets the specified semaphore.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_semaphore_get(int semaphore_id)
{

UINT    status;


    /*  Get the semaphore.  */
    status =  tx_semaphore_get(&tm_semaphore_array[semaphore_id], TX_NO_WAIT);

    /* Determine if the semaphore get was successful.  */
    if (status == TX_SUCCESS)
        return(TM_SUCCESS);
    else
        return(TM_ERROR);
}


/* This function puts the specified semaphore.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_semaphore_put(int semaphore_id)
{

UINT    status;


    /*  Put the semaphore.  */
    status =  tx_semaphore_put(&tm_semaphore_array[semaphore_id]);

    /* Determine if the semaphore put was successful.  */
    if (status == TX_SUCCESS)
        return(TM_SUCCESS);
    else
        return(TM_ERROR);
}


/* This function creates the specified memory pool that can support one or more
   allocations of 128 bytes.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_memory_pool_create(int pool_id)
{

UINT    status;


    /*  Create the memory pool.  */
    status =  tx_block_pool_create(&tm_block_pool_array[pool_id], "Thread-Metric test", 128, &tm_pool_memory_area[pool_id*TM_THREADX_MEMORY_POOL_SIZE], TM_THREADX_MEMORY_POOL_SIZE);

    /* Determine if the block pool memory was successful.  */
    if (status == TX_SUCCESS)
        return(TM_SUCCESS);
    else
        return(TM_ERROR);
}


/* This function allocates a 128 byte block from the specified memory pool.  
   If successful, the function should return TM_SUCCESS. Otherwise, TM_ERROR 
   should be returned.  */
int  tm_memory_pool_allocate(int pool_id, unsigned char **memory_ptr)
{

UINT    status;


    /*  Allocate a 128-byte block from the specified memory pool.  */
    status =  tx_block_allocate(&tm_block_pool_array[pool_id], (void **) memory_ptr, TX_NO_WAIT);

    /* Determine if the block pool allocate was successful.  */
    if (status == TX_SUCCESS)
        return(TM_SUCCESS);
    else
        return(TM_ERROR);
}


/* This function releases a previously allocated 128 byte block from the specified 
   memory pool. If successful, the function should return TM_SUCCESS. Otherwise, TM_ERROR 
   should be returned.  */
int  tm_memory_pool_deallocate(int pool_id, unsigned char *memory_ptr)
{

UINT    status;


    /*  Release the 128-byte block back to the specified memory pool.  */
    status =  tx_block_release((void *) memory_ptr);

    /* Determine if the block pool release was successful.  */
    if (status == TX_SUCCESS)
        return(TM_SUCCESS);
    else
        return(TM_ERROR);
}


/* This is the ThreadX thread entry.  It is going to call the Thread-Metric
   entry function saved earlier.  */
VOID  tm_thread_entry(ULONG thread_input)
{

void (*entry_function)(void);


    /* Pickup the entry function from the saved array.  */
    entry_function =  (void (*)(void)) tm_thread_entry_functions[thread_input];

    /* Call the entry function.   */
    (entry_function)();
}


