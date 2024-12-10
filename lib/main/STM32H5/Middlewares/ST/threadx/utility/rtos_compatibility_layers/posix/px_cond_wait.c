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

#include "tx_api.h"     /* Threadx API */
#include "pthread.h"    /* Posix API */
#include "px_int.h"     /* Posix helper functions */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*  pthread_cond_wait                                     PORTABLE C      */ 
/*                                                           6.1.7        */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function shall block on a condition variable. They shall be    */
/*    called with mutex locked by the calling thread or undefined behavior*/
/*    results. These functions atomically release the mutex and cause the */
/*    calling thread to block on the condition variable cond; atomically  */
/*    here means atomically with respect to access by another thread to   */
/*    the mutex and then the request for semaphore.                       */
/*                                                                        */
/*    Upon successful return, the mutex shall have been locked and shall  */
/*    be owned by the calling thread.                                     */
/*                                                                        */
/*    When using condition variables there is always a Boolean predicate  */
/*    involving shared variables associated with each condition wait that */
/*    is true if the thread should proceed. Spurious wakeups from the     */
/*    pthread_cond_timedwait or pthread_cond_wait functions may occur.    */
/*    Since the return from pthread_cond_timedwait or pthread_cond_wait   */
/*    does not imply anything about the value of this predicate, the      */
/*    predicate should be re-evaluated upon such return. The effect of    */
/*    using more than one mutex for concurrent pthread_cond_timedwait or  */
/*    pthread_cond_wait operations on the same condition variable is      */
/*    undefined; that is, a condition variable becomes bound to a unique  */
/*    mutex when a thread waits on the condition variable, and this       */
/*    (dynamic) binding shall end when the wait returns.                  */ 
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */
/*     cond                         condition variable                    */
/*     mutex                        mutex to be associated with condition */
/*                                  variable                              */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*     OK                           if succesfull                         */
/*     ERROR                        in case of any error                  */
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */
/*   pthread_mutex_unlock          unlocks the mutex held by the caller   */
/*   tx_semaphore_get              try to get sempaphore internal to cond */
/*   tx_semaphore_prioritize       prioritize all suspended pthreads      */
/*   pthread_mutex_lock            lock the mutex                         */
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
INT pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{

TX_SEMAPHORE   *semaphore_ptr;
UINT            status;
UINT            old_threshold,dummy;
TX_THREAD       *thread;

    /* Find the current thread. */
    thread = tx_thread_identify();

    /* Raise its preemption threshold so it does not get descheduled. */
    tx_thread_preemption_change(thread,0,&old_threshold); 

    pthread_mutex_unlock(mutex);

    semaphore_ptr = (&( cond->cond_semaphore ));

    status = tx_semaphore_get(semaphore_ptr, TX_WAIT_FOREVER);

    /* Restore original preemption threshold. */
    tx_thread_preemption_change(thread, old_threshold, &dummy);


    if ( status != TX_SUCCESS)
    {
        posix_errno = EINVAL;
        posix_set_pthread_errno(EINVAL);
        return(EINVAL);
    }

    status = tx_semaphore_prioritize(semaphore_ptr);

    if ( status != TX_SUCCESS)
    {
        posix_errno = EINVAL;
        posix_set_pthread_errno(EINVAL);
        return(EINVAL);
    }
    
    pthread_mutex_lock(mutex);
    return(OK);
}
