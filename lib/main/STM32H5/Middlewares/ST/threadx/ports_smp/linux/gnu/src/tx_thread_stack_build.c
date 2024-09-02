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
/**   Thread                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


#define TX_SOURCE_CODE
#define TX_THREAD_SMP_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_thread.h"
#include <stdio.h>
#include <unistd.h>


/* Prototype for new thread entry function.  */

void *_tx_linux_thread_entry(void *ptr);


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_stack_build                            SMP/Linux/GCC     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function builds a stack frame on the supplied thread's stack.  */
/*    The stack frame results in a fake interrupt return to the supplied  */
/*    function pointer.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Pointer to thread control blk */
/*    function_ptr                          Pointer to return function    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    pthread_create                                                      */
/*    pthread_setschedparam                                               */
/*    _tx_linux_thread_suspend                                            */
/*    sem_init                                                            */
/*    printf                                                              */
/*    _tx_linux_thread_resume                                             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_thread_create                     Create thread service         */
/*    _tx_thread_reset                      Reset thread service          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
VOID   _tx_thread_stack_build(TX_THREAD *thread_ptr, VOID (*function_ptr)(VOID))
{
struct sched_param sp;
pthread_attr_t attrs;

    /* Create the run semaphore for the thread.  This will allow the scheduler
       control over when the thread actually runs.  */
    if(sem_init(&thread_ptr -> tx_thread_linux_thread_run_semaphore, 0, 0))
    {

        /* Display an error message.  */
        printf("ThreadX Linux error creating thread running semaphore!\n");
        while(1)
        {
        }
    }

    /* Create a Linux thread for the application thread.  */
    pthread_attr_init(&attrs);
    pthread_attr_setstacksize(&attrs, TX_LINUX_THREAD_STACK_SIZE);
    if(pthread_create(&thread_ptr -> tx_thread_linux_thread_id, &attrs, _tx_linux_thread_entry, thread_ptr))
    {

        /* Display an error message.  */
        printf("ThreadX Linux error creating thread!\n");
        while(1)
        {
        }
    }

    /* Otherwise, we have a good thread create. */
    sp.sched_priority = TX_LINUX_PRIORITY_USER_THREAD;
    pthread_setschedparam(thread_ptr -> tx_thread_linux_thread_id, SCHED_FIFO, &sp);

    /* Setup the thread suspension type to solicited thread suspension.
       Pseudo interrupt handlers will suspend with this field set to 1.  */
    thread_ptr -> tx_thread_linux_suspension_type =  2;

    /* Clear the disabled count that will keep track of the
       tx_interrupt_control nesting.  */
    thread_ptr -> tx_thread_linux_mutex_access = TX_FALSE;

    /* Setup a fake thread stack pointer.   */
    thread_ptr -> tx_thread_stack_ptr =  (VOID *) (((CHAR *) thread_ptr -> tx_thread_stack_end) - 8);

    /* Clear the first word of the stack.  */
    *(((ULONG *) thread_ptr -> tx_thread_stack_ptr) - 1) =  0;

    /* Indicate that this thread is now ready for scheduling again by another core.  */
    thread_ptr -> tx_thread_smp_core_control =  1;
}


void *_tx_linux_thread_entry(void *ptr)
{

TX_THREAD  *thread_ptr;

    /* Pickup the current thread pointer.  */
    thread_ptr =  (TX_THREAD *) ptr;
    _tx_linux_threadx_thread = 1;
    nice(20);

    /* Now suspend the thread initially.  If the thread has already
       been scheduled, this will return immediately.  */
    tx_linux_sem_wait(&thread_ptr -> tx_thread_linux_thread_run_semaphore);
    tx_linux_sem_post(&_tx_linux_scheduler_semaphore);

    /* Call ThreadX thread entry point.  */
    _tx_thread_shell_entry();

    return EXIT_SUCCESS;
}

