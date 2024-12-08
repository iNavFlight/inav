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
/*    pthread_create                                      PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This creates a new thread with attributes specified by attr within  */
/*    a process.If the attr is NULL,then default attributes are used.     */
/*    Upon successful completion, pthread_create() stores the ID of the   */
/*    created thread in the location referenced by thread.                */
/*    The thread is created executing start_routine with arg as its sole  */
/*    argument.                                                           */
/*    Initially, threads are created from within a process. Once created, */
/*    threads are peers, and may create other threads. Note that an       */
/*    "initial thread" exists by default  which runs 'main'               */
/*    If pthread_create( ) fails,no new thread is created and the contents*/
/*    of the location referenced by thread are undefined.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread                      place to store newly created thread ID  */
/*                                Each thread in a process is uniquely    */
/*                                identified during its lifetime by a     */
/*                                value of type pthread_t called as       */
/*                                thread ID.                              */
/*                                                                        */
/*    attr                        attributes object , NULL = Default attr.*/
/*    start_routine               thread start up routine                 */
/*    arg                         arguments to start up routine by ref.   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    zero                        If successful                           */
/*    error number                If fails                                */
/*                                pthread_create() function will fail if: */
/*                                [EAGAIN] system lacked the necessary    */
/*                                         resources to create a thread,  */
/*                                         or the system-imposed limit on */
/*                                         the number of pthreads in      */
/*                                         a process PTHREAD_THREADS_MAX  */
/*                                         would be exceeded.             */ 
/*                                [EINVAL] value specified by attr is     */
/*                                         invalid.                       */
/*                                [EPERM]  The caller does not have       */
/*                                         appropriate permission to set  */
/*                                         the required scheduling        */
/*                                         parameters or scheduling policy*/ 
/*                                                                        */   
/*                      This call will not return an error code of [EINTR]*/ 
/*                                                                        */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    posix_in_thread_context         To check calling context.           */
/*    posix_internal_error            Internal error                      */
/*    posix_allocate_pthread_t        Allocate control block for pthread  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      William E. Lamie        Initial Version 6.1.7         */
/*  10-31-2022      Scott Larson            Add 64-bit support,           */
/*                                            remove double parenthesis,  */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
INT pthread_create (pthread_t *thread, pthread_attr_t *attr,
                   void *(*start_routine)(void*),void *arg)
{ 

TX_INTERRUPT_SAVE_AREA


TX_THREAD        *thread_ptr;
POSIX_TCB        *pthread_ptr, *current_thread_ptr; 
INT               status,retval;


   
    /* Make sure we're calling this routine from a thread context.  */
    if (!posix_in_thread_context())
    {
        /* return POSIX error.  */
        posix_internal_error(444);
    }

    /* Disable interrupts.  */ 


    /*  check for any pthread_t attr suggested */
    if (!attr)
    {
        /* no attributes passed so assume default attributes */
        attr = &(posix_default_pthread_attr);
    }
    else
    {
        /* Check attributes passed , check for validity */
        if ( (attr->inuse) == TX_FALSE)
        {   
            /* attributes passed are not valid, return with an error */
            posix_errno = EINVAL;    
            posix_set_pthread_errno(EINVAL);
            return(EINVAL);
        }
    }


    /* Get a pthread control block for this new pthread */ 
    TX_DISABLE
    status = posix_allocate_pthread_t(&pthread_ptr); 
    TX_RESTORE
    /* Make sure we got a Thread control block */ 
    if ((status == ERROR) || (!pthread_ptr))
    {
        /* Configuration/resource error.  */ 
        return(EAGAIN); 
    }   

    if(attr->inherit_sched==PTHREAD_INHERIT_SCHED)
    {
        /* get current thread tcb */
        current_thread_ptr=posix_tid2tcb(pthread_self());

        /* copy scheduling attributes */
        pthread_ptr->current_priority = current_thread_ptr->current_priority ;
        pthread_ptr->detach_state = current_thread_ptr->detach_state ;
        pthread_ptr->inherit_sched = current_thread_ptr->inherit_sched ;
        pthread_ptr->orig_priority = current_thread_ptr->orig_priority ;
        pthread_ptr->sched_attr.sched_priority= current_thread_ptr->sched_attr.sched_priority ;
        pthread_ptr->pthread_flags = current_thread_ptr->pthread_flags ;
        pthread_ptr->sched_policy = current_thread_ptr->sched_policy;
        pthread_ptr->stack_size = current_thread_ptr->stack_size ;
        pthread_ptr->stack_address = NULL;                          /* will allocate our own stack */
        pthread_ptr->threshold = pthread_ptr->current_priority ;    /* preemption threshold */
    }
    else  /* assume PTHREAD_EXPLICIT_SCHED */
    {

        /* copy pthread-attr to TCB */
        posix_copy_pthread_attr(pthread_ptr,attr);

        /* preemption treshold */
        pthread_ptr->threshold = pthread_ptr->current_priority ;
        /* scheduling */
        if(pthread_ptr->sched_policy==SCHED_RR) pthread_ptr->time_slice = SCHED_RR_TIME_SLICE;
        else pthread_ptr->time_slice = 0;
    }

    /* Now set up pthread initial parameters */
    
    pthread_ptr->entry_parameter = arg;
    pthread_ptr->start_routine = start_routine;
    /* Newly created pthread not joined by anybody! */
    pthread_ptr->is_joined_by = TX_FALSE;
    pthread_ptr->joined_by_pthreadID =TX_FALSE;
    /* Newly created pthread not yet joined to any other pthread */ 
    pthread_ptr->is_joined_to = TX_FALSE; 
    pthread_ptr->joined_to_pthreadID = TX_FALSE;
    
    
    /* Allow cancel */
    pthread_ptr->cancel_state = PTHREAD_CANCEL_ENABLE;
    pthread_ptr->cancel_type = PTHREAD_CANCEL_DEFERRED;
    pthread_ptr->cancel_request = FALSE;

    pthread_ptr->value_ptr = NULL;


    /* default stack allocation */
    if((attr->stack_address)==NULL)
    {
        pthread_ptr->stack_size = attr->stack_size ;
        status = posix_memory_allocate( pthread_ptr->stack_size, &(pthread_ptr->stack_address));

        /* problem allocating stack space */
        if (status == ERROR)
        {
          /* Configuration/resource error.  */ 
          return(EAGAIN); 
        }
        
    }

    /* Create an event flags group for sigwait.  */
    retval =  tx_event_flags_create(&(pthread_ptr -> signals.signal_event_flags), "posix sigwait events");
    
   /* Get the thread info from the TCB.  */ 
    thread_ptr = posix_tcb2thread(pthread_ptr); 
    
   /* Now actually create and start the thread.  */
   /* convert Posix priorities to Threadx priority */
    retval += tx_thread_create(thread_ptr,
                               "pthr",
                               posix_thread_wrapper,
                               (ULONG)(ALIGN_TYPE)pthread_ptr,
                               pthread_ptr->stack_address,
                               pthread_ptr->stack_size,
                               (TX_LOWEST_PRIORITY - pthread_ptr->current_priority + 1),
                               (TX_LOWEST_PRIORITY - pthread_ptr->threshold + 1),
                               pthread_ptr->time_slice,
                               TX_AUTO_START); 
    
    TX_THREAD_EXTENSION_PTR_SET(thread_ptr, pthread_ptr)
    
    /* See if ThreadX encountered an error */ 
    if (retval)
    {
        /* Internal error */ 
        posix_error_handler(3333); 
        retval = EACCES; 
    }
    else
    {
        /* Everything is fine.  */ 
        /* create a pthreadID by type casting POSIX_TCB into pthread_t */
        pthread_ptr->pthreadID = (pthread_t )pthread_ptr;
        *thread = pthread_ptr->pthreadID ;
        retval = OK; 
    }
    
    /* Everything worked fine if we got here */ 
    return(retval); 

}
