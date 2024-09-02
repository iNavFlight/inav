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
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*  pthread_cond_broadcast                                PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    These functions shall unblock all threads currently blocked on a    */
/*    specified condition variable cond.                                  */
/*    If more than one thread is blocked on a condition variable,         */
/*    the scheduling policy shall determine the order in which threads are*/
/*    unblocked. When each thread unblocked as a result of this function  */
/*    call returns from its call to pthread_cond_wait or                  */
/*    pthread_cond_timedwait, the thread shall own the mutex with which it*/
/*    called pthread_cond_wait or pthread_cond_timedwait. The thread(s)   */
/*    that are unblocked shall contend for the mutex according to the     */
/*    scheduling policy (if applicable), and as if each had called        */
/*    pthread_mutex_lock.The pthread_cond_broadcast function may be called*/
/*    by a thread whether or not it currently owns the mutex that threads */
/*    calling pthread_cond_wait or pthread_cond_timedwait have associated */
/*    with the condition variable during their waits; however,            */
/*    if predictable scheduling behavior is required, then that mutex     */
/*    shall be locked by the thread calling pthread_cond_broadcast.       */      
/*    The pthread_cond_broadcast function shall have no effect if there   */
/*    are no threads currently blocked on cond.                           */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */
/*     cond                       condition variable                      */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*     Ok                        if successful                            */
/*     Error                     in case of any errors                    */
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */
/*     tx_semaphore_prioritize       line up pthreads waiting at semaphore*/
/*     tx_thread_identify            to check which pthread?              */
/*     tx_thread_preemption_change   to disable thread preemption         */
/*     tx_semaphore_put              ThreadX semaphore put service        */
/*     tx_thread_preemption_change   to enable thread preemption          */
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
INT pthread_cond_broadcast(pthread_cond_t *cond)
{
    
TX_SEMAPHORE        *semaphore_ptr;
TX_THREAD           *thread;
UINT                 status;
ULONG                sem_count;
UINT                 old_threshold,dummy;


    /* Get the condition variable's internal semaphore.  */
    /* Simply convert the condition variable control block into a semaphore  a cast */ 
    semaphore_ptr = (&( cond->cond_semaphore ));
    sem_count = semaphore_ptr->tx_semaphore_suspended_count;

    if (!sem_count)
        return(OK);
    
    status = tx_semaphore_prioritize(semaphore_ptr);

    if ( status != TX_SUCCESS)
    {
        posix_errno = EINVAL;
        posix_set_pthread_errno(EINVAL);
        return(EINVAL);
    }
    
    /* get to know about current thread */
    thread = tx_thread_identify();

    /* Got the current thread , now raise its preemption threshold */
    /* that way the current thread does not get descheduled when   */
    /* threads with higher priority are activated */
    tx_thread_preemption_change(thread,0,&old_threshold); 

    while( sem_count)
    {   

        status = tx_semaphore_put(semaphore_ptr);
        if ( status != TX_SUCCESS)
        {

            /* restore changed preemption threshold */
            tx_thread_preemption_change(thread,old_threshold,&dummy);

            posix_errno = EINVAL;

            posix_set_pthread_errno(EINVAL);

            return(EINVAL);
        }

        sem_count--;
    }

    /* restore changed preemption threshold */
    tx_thread_preemption_change(thread,old_threshold,&dummy);
    return(OK);
}
