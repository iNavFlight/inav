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
/*    pthread_join                                        PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function shall suspend execution of the calling thread until   */
/*    the target thread terminates, unless the target thread has already  */
/*    terminated. On return from a successful pthread_join ( ) call with  */
/*    a non-NULL value_ptr argument, the value passed to pthread_exit( )  */
/*    by the terminating thread shall be made available in the location   */
/*    referenced by value_ptr. When a pthread_join returns successfully,  */
/*    the target thread has been terminated. The results of multiple      */
/*    simultaneous calls to pthread_join specifying the same target thread*/
/*    are undefined. If the thread calling pthread_join is canceled, then */
/*    the target thread shall not be detached.                            */
/*    Eventually, you should call pthread_join() or pthread_detach() for  */
/*    every thread that is created joinable (with a detachstate of        */
/*    PTHREAD_CREATE_JOINABLE)so that the system can reclaim all resources*/
/*    associated with the thread. Failure to join to or detach joinable   */ 
/*    threads will result in memory and other resource leaks until the    */
/*    process ends. If thread doesn't represent a valid undetached thread,*/
/*    pthread_detach() will return ESRCH.                                 */ 
/*                                                                        */ 
/*    Note: this function must be called from a POSIX context; if it is   */  
/*    called from  ThreadX context an error is returned.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread                      pthread handle to the target thread     */
/*    value_ptr                   To receive return value of terminating  */
/*                                thread                                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    zero                        If successful                           */
/*    error number                If fails                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
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
INT pthread_join(pthread_t thread, VOID **value_ptr)

{
    TX_INTERRUPT_SAVE_AREA

        POSIX_TCB   *current_ptr, *target_ptr;
    TX_THREAD   *target_thread;


    /* Get the TCB for the currently running pthread   */ 
    current_ptr = posix_thread2tcb(tx_thread_identify());
    
    /* Make sure that a TCB was returned.  */
    if (!current_ptr)
    {
        posix_errno = ECANCELED;
        posix_set_pthread_errno(ECANCELED);
        return(ECANCELED);
    }

    /* Check trying to join to self ! */
    if ( current_ptr->pthreadID == thread)
    {
        posix_errno= EDEADLK;
        posix_set_pthread_errno(EDEADLK);
        return(EDEADLK);
    }
    /* Check if calling thread is already joined to other thread */

    if ( current_ptr->is_joined_to == TX_TRUE)
    {
        posix_errno= EINVAL;
        posix_set_pthread_errno(EINVAL);
        return(EINVAL);
    }
    /* Sure, calling pthread can be joined to target pthread */

    target_thread = posix_tid2thread(thread);

    if (!target_thread)
    {
        /* Invalid target pthread object */
        posix_errno= ESRCH;
        posix_set_pthread_errno(ESRCH);
        return(ESRCH);
    }

    if ( (target_thread->tx_thread_state == TX_COMPLETED) || (target_thread->tx_thread_state == TX_TERMINATED) )
    {
        /* but target pthread is already terminated */
        /* return the return value of the terminated thread */
        target_ptr = posix_tid2tcb(thread);
        if(value_ptr)*value_ptr = target_ptr->value_ptr; 
        return(OK);
    }

    /* Now check the target thread, whether it is already joined to any other thread? */
    target_ptr = posix_tid2tcb(thread);

    /* Now check the target thread, whether it is already joined to any other thread? */
    if (target_ptr->is_joined_by == TX_TRUE)
    {
        /* but target pthread is already terminated */
        posix_errno= EINVAL;
        posix_set_pthread_errno(EINVAL);
        return(EINVAL);
    }

    /* check joinability of target thread */
    if ( target_ptr->detach_state != PTHREAD_CREATE_JOINABLE)
    {
        posix_errno= EINVAL;
        posix_set_pthread_errno(EINVAL);
        return(EINVAL);
    }

    /* Now it is Okay to join */

    TX_DISABLE

    /* declare that this calling pthread is joined to other pthread */
        current_ptr->is_joined_to  = TX_TRUE;
    /* register the target pthread */
    current_ptr->joined_to_pthreadID = thread;
    /* declare that the target pthread is joined by calling pthread */
    target_ptr->is_joined_by = TX_TRUE;
    /* and register the caller pthread with target pthread */
    target_ptr->joined_by_pthreadID = current_ptr->pthreadID ;

    TX_RESTORE

    /* Now calling pthread will suspend itself and wait till target pthread exits */
        tx_thread_suspend ( &(current_ptr->thread_info));  
    /* target pthread exited and thus current pthread will resume now */
    /* store return value if value_ptr is valid */
    if(value_ptr)*value_ptr = target_ptr->value_ptr; 
    return(OK);
}
