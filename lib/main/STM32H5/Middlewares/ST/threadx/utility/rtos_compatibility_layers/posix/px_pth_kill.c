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
#include "tx_thread.h"  /* Internal ThreadX thread management.  */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    pthread_kill                                        PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*     This function is used to request that a signal be delivered to     */
/*     the specified thread.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_id                      Thread ID                            */
/*    sig                            Signal                               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*     0                             if successful                        */
/*     Value                         in case of any error                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    posix_in_thread_context               Make sure caller is thread    */
/*    posix_internal_error                  Generic error Handler         */
/*    tx_event_flags_set                                                  */
/*    posix_memory_allocate                 Create a byte pool for stack  */
/*    tx_thread_create                                                    */
/*    tx_event_flags_delete                                               */
/*    posix_memory_release                                                */
/*    tx_thread_suspend                                                   */
/*    _tx_thread_system_preempt_check                                     */
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
/*  10-31-2022      Scott Larson            Remove double parenthesis,    */
/*                                            update argument type,       */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
int   pthread_kill(ALIGN_TYPE thread_id, int sig)
{

TX_INTERRUPT_SAVE_AREA

POSIX_TCB   *target_thread;
POSIX_TCB   *new_signal_thread;
VOID        (*handler)(int);
INT         status;
UINT        retval;


    /* Make sure we're calling this routine from a thread context.  */
    if (!posix_in_thread_context())
    {
        /* return POSIX error.  */
        posix_internal_error(444);
        return(ERROR);
    }

    /* Determine if the desired signal is valid.  */
    if ((sig < 0) || (sig > SIGRTMAX))
    {
    
        /* Return an error.  */
        posix_set_pthread_errno(EINVAL);
        return(ERROR);
    }

    /* Pickup target thread.  */
    target_thread =  (POSIX_TCB *) thread_id;

    /* Is it non-NULL?  */
    if (!target_thread)
    {

        /* Invalid target pthread object */
        posix_errno= ESRCH;
        posix_set_pthread_errno(ESRCH);
        return(ERROR);
    }

    /* Pickup signal handler function pointer.  */
    handler =  target_thread -> signals.signal_func[sig];

    /* See if there is a signal handler setup for this signal.  */
    if (!handler)
    {
    
        /* No handler, just set/clear the event flags to handle the sigwait condition.  */
        
        /* Set the event flag corresponding the signal.  */
        tx_event_flags_set(&(target_thread -> signals.signal_event_flags), (((ULONG) 1) << sig), TX_OR);

        /* Ensure the flag is left in a clear state.  */
        tx_event_flags_set(&(target_thread -> signals.signal_event_flags), ~(((ULONG) 1) << sig), TX_AND);

        /* We are done, just return success.  */
        return(OK);
    }

    /* Now, let's look to see if the same signal is already pending.  */
    if (target_thread -> signals.signal_pending.signal_set & (((unsigned long) 1) << sig))
    {
    
        /* Yes, the same signal is already pending, just return.  */
        return(OK);
    }

    /* Now determine if the thread's signals are masked by pthread_sigmask.  */
    if (target_thread -> signals.signal_mask.signal_set & (((unsigned long) 1) << sig))
    {
    
        /* Yes, simply set the pending bit so we know that the signal must be activated later when the 
           signal mask for this signal is cleared.  */
        target_thread -> signals.signal_pending.signal_set =  target_thread -> signals.signal_pending.signal_set | (((unsigned long) 1) << sig);
        return(OK);
    }

    /* At this point we know that we need to create a new signal handler thread for processing this signal.  */
 
    /* Get a pthread control block for this new signal pthread */ 
    
    /* Disable interrupts for protection.  */
    TX_DISABLE

    /* Disable preemption temporarily.  */
    _tx_thread_preempt_disable++;

    /* Allocate a POSIX thread control block. */
    status = posix_allocate_pthread_t(&new_signal_thread); 

    /* Restore interrupts.  */
    TX_RESTORE
    
    /* Make sure we got a new thread control block */ 
    if ((status == ERROR) || (!new_signal_thread))
    {

        /* Disable interrupts.  */
        TX_DISABLE

        /* Enable preemption.  */
        _tx_thread_preempt_disable--;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Configuration/resource error.  */ 
        posix_set_pthread_errno(EAGAIN);
        return(ERROR); 
    }   

    /* Inherit the stack size for the new signal thread.  */
    new_signal_thread -> stack_size =  target_thread -> stack_size ;

    /* Allocate memory for stack.  */
    status =  posix_memory_allocate(new_signal_thread -> stack_size, &new_signal_thread -> stack_address);

    /* problem allocating stack space */
    if (status == ERROR)
    {
        
        /* Mark the previously allocated control block as available.  */
        new_signal_thread -> in_use = FALSE;
    
        /* Disable interrupts.  */
        TX_DISABLE

        /* Enable preemption.  */
        _tx_thread_preempt_disable--;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Configuration/resource error.  */ 
        posix_set_pthread_errno(EAGAIN);
        return(ERROR); 
    }   
    
    /* Inherit scheduling attributes from base thread.  */
    new_signal_thread -> current_priority =         target_thread -> current_priority ;
    new_signal_thread -> detach_state =             target_thread -> detach_state ;
    new_signal_thread -> inherit_sched =            target_thread -> inherit_sched ;
    new_signal_thread -> orig_priority =            target_thread -> orig_priority ;
    new_signal_thread -> sched_attr.sched_priority= target_thread -> sched_attr.sched_priority ;
    new_signal_thread -> pthread_flags =            target_thread -> pthread_flags ;
    new_signal_thread -> sched_policy =             target_thread -> sched_policy;
    new_signal_thread -> is_joined_by =             TX_FALSE;
    new_signal_thread -> joined_by_pthreadID =      TX_FALSE;
    new_signal_thread -> is_joined_to =             TX_FALSE; 
    new_signal_thread -> joined_to_pthreadID =      TX_FALSE;
    new_signal_thread -> cancel_state =             PTHREAD_CANCEL_ENABLE;
    new_signal_thread -> cancel_type =              PTHREAD_CANCEL_DEFERRED;
    new_signal_thread -> cancel_request =           FALSE;
    new_signal_thread -> value_ptr =                NULL;

    /* Increment the target thread's signal nesting depth.  */
    target_thread -> signals.signal_nesting_depth++;

    /* Mark this signal as pending in the signal set.  */
    target_thread -> signals.signal_pending.signal_set =  target_thread -> signals.signal_pending.signal_set | (((unsigned long) 1) << sig);

    /* Mark the new thread as a signal thread, clear signal info, and setup links.  */
    new_signal_thread -> signals.signal_handler =             TRUE;
    new_signal_thread -> signals.signal_nesting_depth =       target_thread -> signals.signal_nesting_depth;
    new_signal_thread -> signals.signal_pending.signal_set =  target_thread -> signals.signal_pending.signal_set;    
    new_signal_thread -> signals.saved_thread_state =         ((TX_THREAD *) target_thread) -> tx_thread_state;
    new_signal_thread -> signals.base_thread_ptr =            target_thread;
    new_signal_thread -> signals.next_signal_thread =         target_thread -> signals.top_signal_thread;

    /* Remember the top signal thread in the base thread.  */
    target_thread -> signals.top_signal_thread =              new_signal_thread;

    /* Now actually create and start the signal thread.  */
    retval = tx_thread_create( (TX_THREAD *) new_signal_thread,
                               "signal pthr",
                               internal_signal_dispatch,
                               (ULONG) sig,
                               new_signal_thread -> stack_address,
                               new_signal_thread -> stack_size,
                               (TX_LOWEST_PRIORITY - new_signal_thread -> current_priority + 1),
                               (TX_LOWEST_PRIORITY - new_signal_thread -> current_priority + 1),
                               new_signal_thread -> time_slice,
                               TX_AUTO_START); 

    /* See if ThreadX encountered an error */ 
    if (retval)
    {

        /* Create an event flags group.  */
        tx_event_flags_delete(&(new_signal_thread -> signals.signal_event_flags));

        /* Release the stack memory.  */
        posix_memory_release(new_signal_thread -> stack_address);
        
        /* Mark the previously allocated control block as available.  */
        new_signal_thread -> in_use = FALSE;
    
        /* Disable interrupts.  */
        TX_DISABLE

        /* Enable preemption.  */
        _tx_thread_preempt_disable--;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Internal error */ 
        posix_error_handler(3333); 
        posix_set_pthread_errno(EACCES);
        return(ERROR); 
    }

    /* Disable interrupts.  */
    TX_DISABLE

    /* Enable preemption.  */
    _tx_thread_preempt_disable--;

    /* At this point, we need to suspend the target thread if we are the first signal handler to run.  */
    if (new_signal_thread -> signals.signal_nesting_depth == 1)
    {

        /* Restore interrupts.  */
        TX_RESTORE
    
        /* Suspend the base thread so that it doesn't run again until all the signals have been processed.  */
        tx_thread_suspend((TX_THREAD *) target_thread);
    }
    else if (new_signal_thread -> signals.next_signal_thread)
    {
    
        /* Restore interrupts.  */
        TX_RESTORE

        /* Make sure the current top level signal handler thread is suspended.  */
        tx_thread_suspend((TX_THREAD *) new_signal_thread -> signals.next_signal_thread);
    }

    /* At this point, the new signal has been set and the signal handler is ready for execution.  */

    /* Check for a preemption condition.  */
    _tx_thread_system_preempt_check();
     
    /* Return success!  */
    return(OK);
}
