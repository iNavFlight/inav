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

/* this define will force declaration of the  */
/* posix objects to happen in this file       */
#define PX_OBJECT_INIT

#include "px_int.h"    /* Posix helper functions */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    is_posix_thread                                     PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Verify that the control block belongs to a POSIX thread and not     */ 
/*    a ThreadX thread                                                    */ 
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                       Pointer to a thread control block  */ 
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_FALSE if not POSIX thread. TX_TRUE if POSIX thread.              */  
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
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
static INT is_posix_thread(TX_THREAD *thread_ptr)
{
    if (((POSIX_TCB *)thread_ptr < ptcb_pool) ||
        ((POSIX_TCB *)thread_ptr > &ptcb_pool[PTHREAD_THREADS_MAX - 1]))
    {
        return TX_FALSE; 
    }
    return TX_TRUE; 
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    posix_pthread_init                                  PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets up / configures / initializes all the            */ 
/*    pthread Control Blocks that we define at compile-time in order to   */ 
/*    ensure that there is sufficient memory.                             */ 
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
/*    posix_reset_pthread_t                 Reset a task control block    */ 
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
 VOID    posix_pthread_init(VOID)
{

ULONG  index; 

    /* Loop through array of TCBs and initialize each one.  */ 
    for (index = 0;  index < PTHREAD_THREADS_MAX;  index++)
    {
        posix_reset_pthread_t(&(ptcb_pool[index])); 
    }
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    posix_reset_pthread_t                               PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function resets a pthread w/ default information.              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ptcb                                  pthread control block pointer */ 
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
/*    Start-up code                                                       */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
 VOID posix_reset_pthread_t (POSIX_TCB *ptcb)
{
        /* Indicate this entry is not in use.  */ 
        ptcb->in_use = TX_FALSE; 
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    posix_copy_pthread_attr                             PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function copies pthread attributes from a pthread_attr object  */
/*    to a pthread TCB                                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    attr                                  pthread attr object   pointer */ 
/*    pthread_ptr                           target pthread TCB            */ 
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
/*    Start-up code                                                       */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
 VOID posix_copy_pthread_attr(POSIX_TCB *pthread_ptr,pthread_attr_t *attr)
{

     pthread_ptr->current_priority = attr->sched_attr.sched_priority ;
     pthread_ptr->detach_state = attr->detach_state ;
     pthread_ptr->inherit_sched = attr->inherit_sched ;
     pthread_ptr->orig_priority = attr->sched_attr.sched_priority ;  
     pthread_ptr->sched_attr.sched_priority= attr->sched_attr.sched_priority ;
     pthread_ptr->pthread_flags = attr->pthread_flags ;
     pthread_ptr->sched_policy = attr->sched_policy;
     pthread_ptr->stack_size = attr->stack_size ;
     pthread_ptr->stack_address = attr->stack_address;
     
     return;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    posix_allocate_pthread_t                            PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function attempts to allocate memory for a pthread stack and   */ 
/*    a POSIX pthread Control Block (PTCB).                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    stack_size                            Requested task stack size     */  
/*    tcb_ptr                               Pointer to tcb pointer        */  
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */  
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    Nothing                                                             */  
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
 INT posix_allocate_pthread_t(POSIX_TCB **ptcb_ptr)
{

POSIX_TCB      *ptcb; 
ULONG           index; 

    /* Assume the worst.  */ 
    *ptcb_ptr = (POSIX_TCB *)0; 

    /* This next search is optimized for simplicity, not speed.  */ 
    for (index = 0, ptcb = ptcb_pool; 
         index < PTHREAD_THREADS_MAX; 
         index++, ptcb++)
    {
        /* Is this guy in use?  If not, we can use it.  */ 
        if (ptcb->in_use == TX_FALSE)
        {
            /* This pTCB is now in use.  */ 
            ptcb->in_use = TX_TRUE; 

            /* Stop searching.  */ 
            break; 
        }
    }    /* for each POSIX Thread Control Block */ 

    /* Did we search all pTCBs and come up empty?  */ 
    if (index == PTHREAD_THREADS_MAX)
    {
        /* No more pTCBs available - user configuration error.  */ 
        return(ERROR); 
    }
    else
    {

        /* Make sure the signal handler information is cleared when the new TCB is allocated.  */
        memset(&(ptcb -> signals), 0, sizeof(signal_info));
        
        /* Found one.  */ 
        *ptcb_ptr = ptcb; 
    }
    return(OK);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    posix_thread_wrapper                                PORTABLE C      */ 
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    Every thread that is modeling a ThreadX thread has this routine as  */ 
/*    its entry point.This routine simply calls the pthread entry routine */
/*    with its sole argument passed in pthread-create().                  */ 
/*                                                                        */ 
/*    The main purpose of this function is to mimic the pthread interface */ 
/*    which allows 1 argument to be passed to the entry point of a thread */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pthread_ptr                           pthread control block pointer */  
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    *(pthread_start_routine)             Application pthread entry      */  
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    POSIX  only (internal)                                              */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      William E. Lamie        Initial Version 6.1.7         */
/*  10-31-2022      Scott Larson            Add 64-bit support,           */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
 VOID posix_thread_wrapper(ULONG pthr_ptr)
{

POSIX_TCB        *pthread_ptr;
VOID             *value_ptr;

    /* The input argument is really a pointer to the pthread's control block */
    TX_THREAD_EXTENSION_PTR_GET(pthread_ptr, POSIX_TCB, pthr_ptr)

    /* Invoke the pthread start routine with appropriate arguments */ 
    value_ptr = (pthread_ptr->start_routine)((VOID *)pthread_ptr->entry_parameter);
 
    /* In ThreadX, when a thread returns from its entry point, it enters the   */ 
    /* "completed" state, which is basically an infinite suspension.           */ 
    /* now use pthread_exit call to end this pthread                           */
    pthread_exit(value_ptr);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    posix_thread2tcb                                    PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function converts a ThreadX thread identifier into             */ 
/*    a posix pthread control block (TCB)                                 */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    thread_ptr                            Thread pointer                */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    pthread                               pthread Task control block    */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    posix_internal_error                  Internal error                */  
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    posix internal code                                                 */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
 POSIX_TCB *posix_thread2tcb(TX_THREAD *thread_ptr)
{

POSIX_TCB   *p_tcb; 

    /* Make sure we were called from a thread.  */ 
    if (!thread_ptr)
    {
        /* Not called from a thread - error!  */ 
        posix_internal_error(333); 
    }

    /* Make sure thread is a POSIX thread else following case is illegal.  */ 
    if (!is_posix_thread(thread_ptr)) {
        /* Not called from a POSIX thread - error!  */ 
        return NULL; 
    }

    /* We can do this because the Thread information is intentionally  */
    /* located as the first field in the structure.                    */
    p_tcb = (POSIX_TCB *)thread_ptr; 

    /* All done.  */ 
    return(p_tcb); 
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    posix_tcb2thread                                    PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function converts a POSIX TCB into ThreadX thread              */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pthread_ptr                            pthread TCB                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    thread                                 ThreadX thread               */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    posix_internal_error                  Internal error                */  
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    posix internal code                                                 */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
 TX_THREAD  *posix_tcb2thread (POSIX_TCB *pthread_ptr)
{

TX_THREAD   *thread; 

    /* Make sure we don't have a NULL pointer.  */ 
    if (pthread_ptr)
    {
        /* Simply convert the TCB to a Thread via a cast */ 
        thread = (&(pthread_ptr->thread_info )); 
    }
    else
    {
        thread = ((TX_THREAD *)0); 
    }

    return(thread); 
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    posix_thread2tid                                    PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function converts a ThreadX thread identifier into             */ 
/*    posix thread ID                                                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    thread_ptr                            Thread pointer                */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    thread_ID                             thread_ID                     */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    posix_internal_error                  Internal error                */  
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    posix internal code                                                 */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
 pthread_t  posix_thread2tid(TX_THREAD *thread_ptr)
{

pthread_t           thread_ID;
POSIX_TCB          *p_tcb;

    /* Make sure we were called from a thread.  */ 
    if (!thread_ptr)
    {
        /* Not called from a thread - error!  */ 
           posix_internal_error(222); 
    }

    /* Get the TCB for this pthread */

    p_tcb = posix_thread2tcb(thread_ptr);
    thread_ID = p_tcb->pthreadID; 

    /* All done.  */ 
    return(thread_ID); 
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    posix_tid2thread                                    PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function converts a posix thread ID into a thread.             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tid                                   Thread ID                     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    thread_ptr                            Thread pointer                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    posix internal code                                                 */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
 TX_THREAD *posix_tid2thread(pthread_t ptid)
{

TX_THREAD   *thread; 
POSIX_TCB   *pthread;

    /* Make sure we don't have a NULL TID.  */ 
    if (ptid)
    {
        /* convert the pthread ID to a pThread TCB  */ 
        pthread = posix_tid2tcb(ptid);
    /* convert the pthread TCB to a pThread TCB  */ 
    thread= posix_tcb2thread(pthread);
    }
    else
    {
        thread = ((TX_THREAD *)0); 
    }
    return(thread); 
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    posix_tid2tcb                                       PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function converts a posix thread ID into a posix pthread TCB   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tid                                   Thread ID                     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    pthread_ptr                           pthread pointer               */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    posix internal code                                                 */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
 POSIX_TCB *posix_tid2tcb(pthread_t ptid)
{

POSIX_TCB   *pthread;

    /* Make sure we don't have a NULL TID.  */ 
    if (ptid)
        /* Simply convert the thread ID to a pthread TCB via a cast */ 
        pthread  = (POSIX_TCB *)ptid; 
    else
        pthread = ((POSIX_TCB *)0);
    
    return(pthread); 
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    posix_destroy_pthread                               PORTABLE C      */ 
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs internal cleanup and housekeeping            */ 
/*    when a pthread exits.                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pthread_ptr                          pointer to TCB of the pthread  */
/*                                         to be deleted                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    OK                                   If successful                  */
/*    ERROR                                If fails                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_queue_send                         Send to system mgr queue      */  
/*    posix_internal_error                  Internal error handling       */  
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    posix internal code                                                 */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      William E. Lamie        Initial Version 6.1.7         */
/*  10-31-2022      Scott Larson            Add 64-bit support,           */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
 VOID posix_destroy_pthread(POSIX_TCB *pthread_ptr, VOID *value_ptr)
{

ULONG       request[WORK_REQ_SIZE];
UINT        status;

    /* Build the request. */ 

#ifdef TX_64_BIT
    request[0] = (ULONG)((ALIGN_TYPE)pthread_ptr >> 32);
    request[1] = (ULONG)((ALIGN_TYPE)pthread_ptr);
    request[2] = (ULONG)((ALIGN_TYPE)value_ptr >> 32);
    request[3] = (ULONG)((ALIGN_TYPE)value_ptr);
#else
    request[0] = (ULONG)pthread_ptr;
    request[1] = (ULONG)value_ptr;
#endif

    /* Send a message to the SysMgr supervisor thread, asking it to delete */
    /* the pthread. Since the SysMgr supervisor thread is the highest      */ 
    /* possible priority, this routine will be preempted when we           */ 
    /* post the message to the SysMgr's work queue.                        */ 
        
    status = tx_queue_send(&posix_work_queue, request, TX_NO_WAIT); 
    /* This should always succeed.  */ 
    if (status != TX_SUCCESS)
    {
        posix_internal_error(1001); 
    }
    
    /* Return the pthread's TCB to the pool of available TCB's.  */
    posix_reset_pthread_t(pthread_ptr);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    posix_do_pthread_delete                             PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes the pthread and reclaims the stack memory.    */ 
/*    Also it resumes any pthread joined to this exiting pthread.         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pthread_ptr                          pointer to TCB of the pthread  */
/*                                         to be deleted                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    OK                                   If successful                  */
/*    ERROR                                If fails                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_thread_terminate                   Terminate ThreadX thread      */  
/*    tx_thread_delete                      Delete the ThreadX thread     */  
/*    posix_memory_release                  Release the task's stack      */  
/*    posix_free_tcb                        Free pthread control block    */  
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    posix internal code                                                 */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
 VOID  posix_do_pthread_delete(POSIX_TCB *pthread_ptr, VOID *value_ptr)
{

TX_INTERRUPT_SAVE_AREA

POSIX_TCB   *joined_pthread_ptr;
TX_THREAD   *thread_ptr,*thread1_ptr;
pthread_t    joined_pthread_ID; 
ULONG        status; 

    TX_DISABLE
        /* preserve the thread's return value regardless */
     pthread_ptr->value_ptr = value_ptr;
        
    if ( pthread_ptr->is_joined_by == TX_TRUE)
    {
        joined_pthread_ID = pthread_ptr->joined_by_pthreadID ;
        joined_pthread_ptr = posix_tid2tcb(joined_pthread_ID);
        
        joined_pthread_ptr->is_joined_to = TX_FALSE;
        joined_pthread_ptr->joined_to_pthreadID =TX_FALSE;
        
        thread_ptr = (TX_THREAD *)joined_pthread_ptr;
        
        /* Now resume the suspended pthread joined to this pthread */
        tx_thread_resume(thread_ptr);
    }          
    /* Terminate the pthread's ThreadX thread.  */
    thread1_ptr = posix_tcb2thread(pthread_ptr);
    status = tx_thread_terminate(thread1_ptr); 
    if (status != TX_SUCCESS)
    {
        posix_internal_error(2244); 
    }
    
    /* Delete the pthread's ThreadX thread.  */ 
    status = tx_thread_delete(&(pthread_ptr->thread_info)); 
    if (status != TX_SUCCESS)
    {
        posix_internal_error(2255); 
    }
    /* Free the memory allocated for pthread's stack allocated from the posix heap  */
    /* if the memory was not from the posix heap this call has no effect            */
    /* it will be the user's responsibility to manage such memory                   */
    
    posix_memory_release(pthread_ptr->stack_address); 

    /* Determine if this thread is NOT a signal handler thread. If this is the case,
       delete the event flag group.  */
    if (pthread_ptr -> signals.signal_handler == FALSE)
    {

        /* Delete the event flag group.  */
        tx_event_flags_delete(&(pthread_ptr -> signals.signal_event_flags));
    }

    /* Return the pthread's TCB to the pool of available TCB's.  */ 
    pthread_ptr->in_use = TX_FALSE; 

    TX_RESTORE
    
    /* All done.  */ 
    return; 
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    posix_set_pthread_errno                              PORTABLE C     */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the pthread error number.                        */ 
/*    Each pthread has got its very own erron number.                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    errno_set                            error number to set            */
/*                                                                        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    OK                                   Always return successful       */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_thread_identify                    get calling ThreadX thread    */  
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    posix internal code                                                 */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
INT  posix_set_pthread_errno(ULONG errno_set)
{    

TX_THREAD   *thread_ptr;
POSIX_TCB   *pthread_ptr;

    /* Get the thread identifier of the currently running thread */ 
    thread_ptr = tx_thread_identify(); 
    /* get posix TCB for this pthread */
    pthread_ptr = (POSIX_TCB *)thread_ptr;
    /* Set the error number */
    pthread_ptr->perrno = errno_set;

    /* Always return success!*/
    return(OK);
}    

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    posix_get_pthread_errno                             PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function gets the erron number for a pthread.                  */ 
/*    Each pthread has got its very own erron number.                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ptid                                 pthread id                     */
/*                                                                        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    error_number                        error number for the pthread    */
/*    ERROR                               In case of any error            */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_thread_identify                    get calling ThreadX thread    */  
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    posix internal code                                                 */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
INT   posix_get_pthread_errno(pthread_t ptid)
{

TX_INTERRUPT_SAVE_AREA

INT          error_number;
POSIX_TCB    *pthread_ptr;

    TX_DISABLE

    /* Get the POSIX pthread structure pointer for the ptid */
    pthread_ptr = posix_tid2tcb(ptid);
        /* Check whether we got NULL pointer */
        if (pthread_ptr)
            /* Retrive the stored error number for this pthread */
            error_number = pthread_ptr->perrno;
        else
        error_number = ERROR;
        
    TX_RESTORE
    return(error_number);
}
