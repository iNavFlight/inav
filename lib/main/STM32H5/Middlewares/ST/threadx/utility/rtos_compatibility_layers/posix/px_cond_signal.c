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
/*  pthread_cond_signal                                   PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function shall unblock at least one of the threads that are    */
/*    blocked on the specified condition variable cond                    */
/*    (if any threads are blocked on cond).If more than one thread is     */
/*    blocked on a condition variable, the scheduling policy shall        */
/*    determine the order in which threads are unblocked.When each thread */
/*    unblocked as a result of a pthread_cond_signal returns from its call*/
/*    to pthread_cond_wait or pthread_cond_timedwait, the thread shall own*/
/*    the mutex with which it called pthread_cond_wait or                 */
/*    pthread_cond_timedwait.The thread(s) that are unblocked shall       */
/*    contend for the mutex according to the scheduling policy            */
/*    (if applicable), and as if each had called pthread_mutex_lock.      */
/*    The pthread_cond_broadcast or pthread_cond_signal functions may be  */
/*    called by a thread whether or not it currently owns the mutex that  */
/*    threads calling pthread_cond_wait or pthread_cond_timedwait have    */
/*    associated with the condition variable during their waits; however, */
/*    if predictable scheduling behavior is required,then that mutex shall*/
/*    be locked by the thread calling pthread_cond_signal.                */
/*    This function shall have no effect if there are no threads currently*/
/*    blocked on cond.                                                    */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */
/*     Nothing                                                            */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*     Nothing                                                            */
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */
/*     tx_semaphore_prioritize       line up pthreads waiting at semaphore*/
/*     tx_semaphore_put              ThreadX semaphore put service        */
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
INT  pthread_cond_signal(pthread_cond_t *cond)
{

TX_SEMAPHORE        *semaphore_ptr;
UINT                 status;

    /* Get the condition variable's internal semaphore.  */
    /* Simply convert the COndition variable control block into a semaphore  a cast */ 
    semaphore_ptr = (&( cond->cond_semaphore ));
    if ( semaphore_ptr->tx_semaphore_suspended_count)
    {
    status = tx_semaphore_prioritize(semaphore_ptr);
        if ( status != TX_SUCCESS)
        {
            posix_errno = EINVAL;
	        posix_set_pthread_errno(EINVAL);
            return(EINVAL);
        }
        status = tx_semaphore_put(semaphore_ptr);
        if ( status != TX_SUCCESS)
        {
            posix_errno = EINVAL;
	        posix_set_pthread_errno(EINVAL);
            return(EINVAL);
        }
    }   
    return(OK);
}
