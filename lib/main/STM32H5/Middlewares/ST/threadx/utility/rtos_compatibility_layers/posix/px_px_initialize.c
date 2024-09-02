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
/** POSIX wrapper for THREADX                                             */ 
/**                                                                       */
/**                                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/* Include necessary system files.  */

#include "tx_api.h"    /* Threadx API */
#include "pthread.h"  /* Posix API */
#include "px_int.h"    /* Posix helper functions */

/**************************************************************************/
/**************************************************************************/
/**                                                                       */ 
/** POSIX wrapper for THREADX                                             */ 
/**                                                                       */
/**                                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/* Include necessary system files.  */

#include "tx_api.h"    /* Threadx API */
#include "pthread.h"  /* Posix API */
#include "px_int.h"    /* Posix helper functions */




/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    posix_memory_init                                   PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function attempts to create a ThreadX byte pool that will      */
/*    act as a "heap" for the POSIX's dynamic, "behind-the-scenes"        */
/*    memory needs.                                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    reqion0_ptr                           Pointer to region0 memory     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_byte_pool_create                   Create region0 byte pool      */
/*    posix_internal_error                  Generic posix error           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    POSIX internal code                                                 */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static VOID posix_memory_init(VOID * posix_heap_ptr)
{

INT    retval;

    /* Create a ThreadX byte pool that will provide memory
       needed by the POSIX.  */
    retval = tx_byte_pool_create((TX_BYTE_POOL *)&posix_heap_byte_pool,
                                 "POSIX HEAP",
                                 posix_heap_ptr,
                                 POSIX_HEAP_SIZE_IN_BYTES); 

    /* Make sure the byte pool was created successfully.  */
    if (retval)
    {
        /* Error creating byte pool.  */
        posix_internal_error(9999);
    }
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    posix_semaphore_init                                PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the POSIX's internal semaphore pool.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    POSIX internal Code                                                 */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static VOID posix_semaphore_init(VOID)
{

ULONG       i; 
sem_t      *sem_ptr; 

    /* Start at the front of the pool.  */
    sem_ptr = &(posix_sem_pool[0]); 

    for (i = 0;  i < SEM_NSEMS_MAX;  i++, sem_ptr++)
    {
        /* This semaphore is not currently in use.  */
        sem_ptr->in_use = TX_FALSE;
        sem_ptr->count = 0;
        sem_ptr->sem_name = "";
        sem_ptr->refCnt = 0;
        sem_ptr->psemId = 0;
        sem_ptr->unlink_flag = TX_FALSE;
    }
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    posix_initialize                                    PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets up / configures / initializes all the            */
/*    "behind-the-scenes" data structures, tables, memory regions, etc.   */
/*    used by the POSIX at run-time.                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    posix_memory                           POSIX memory pointer         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    pointer                                Next free POSIX memory       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    posix_memory_init                     Initialize posix memory       */
/*    tx_thread_create                      Create System Manager Thread  */
/*    tx_queue_create                       Create system manager queue   */
/*    posix_queue_init                      Initialize posix queues       */
/*    posix_qattr_init                      Initialize mqattr structure   */
/*    posix_semaphore_init                  Initialize posix semaphores   */
/*    set_default_mutexattr                 Set up default mutexattr obj  */
/*    posix_pthread_init                    Initialize a static pool of   */
/*                                          pthreads Control blocks       */
/*    set_default_pthread_attr              Set defualt pthread_attr obj  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Start-up code                                                       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
VOID *posix_initialize(VOID * posix_memory)
{

UCHAR   *pointer;

    /*  Setup temporary memory pointer, so we can start allocating 
        space for the posix data structures.  The important thing to 
        remember here is that the system thread's stack, the region0
        memory, and the queue are allocated sequentially from the
        address specified by posix_memory.                          */

    pointer =  (UCHAR *)posix_memory;
    
    /* Start up the System Manager thread.  */ 
    tx_thread_create(&posix_system_manager, "POSIX System Manager", posix_system_manager_entry,
                       0, pointer, POSIX_SYSTEM_STACK_SIZE,
                       SYSMGR_PRIORITY, SYSMGR_THRESHOLD,
                       TX_NO_TIME_SLICE, TX_AUTO_START); 
    
    pointer =  pointer + POSIX_SYSTEM_STACK_SIZE;

    /* Set up a memory "heap" used internally by the POSIX.  */
    posix_memory_init(pointer);

    pointer =  pointer + POSIX_HEAP_SIZE_IN_BYTES;

    /* Create the work item message queue.  */
    tx_queue_create(&posix_work_queue, "POSIX work queue", WORK_REQ_SIZE,
                       pointer, WORK_QUEUE_DEPTH*WORK_REQ_SIZE);
    
    pointer = pointer + (WORK_QUEUE_DEPTH * WORK_REQ_SIZE);

    /* Initialize static pool of pthreads Control blocks.  */ 
    posix_pthread_init(); 

    /* Create a default pthread_attr */
    set_default_pthread_attr(&posix_default_pthread_attr);

#if POSIX_MAX_QUEUES != 0
    /* Set up a pool of message queues used internally by the POSIX.  */
    posix_queue_init();

    /* Set up a pool of q_attr used internally by the POSIX.  */
    posix_qattr_init();
#endif

#if SEM_NSEMS_MAX != 0
    /* Set up a pool of semaphores used internally by the POSIX.  */
    posix_semaphore_init();
#endif
    
    /* Create a default mutex_attr */
    set_default_mutexattr(&posix_default_mutex_attr);

    pointer = (VOID *) pointer;

    /* All done.  */
    return(pointer);
}
