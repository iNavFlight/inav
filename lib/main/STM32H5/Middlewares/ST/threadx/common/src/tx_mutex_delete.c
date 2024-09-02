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
/**   Mutex                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_thread.h"
#include "tx_mutex.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_mutex_delete                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes the specified mutex.  All threads             */
/*    suspended on the mutex are resumed with the TX_DELETED status       */
/*    code.                                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mutex_ptr                         Pointer to mutex control block    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_SUCCESS                        Successful completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_mutex_put                     Release an owned mutex            */
/*    _tx_thread_system_preempt_check   Check for preemption              */
/*    _tx_thread_system_resume          Resume thread service             */
/*    _tx_thread_system_ni_resume       Non-interruptable resume thread   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _tx_mutex_delete(TX_MUTEX *mutex_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD       *thread_ptr;
TX_THREAD       *next_thread;
TX_THREAD       *owner_thread;
UINT            suspended_count;
TX_MUTEX        *next_mutex;
TX_MUTEX        *previous_mutex;
#ifdef TX_MISRA_ENABLE
UINT            status;
#endif

    /* Disable interrupts to remove the mutex from the created list.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_MUTEX_DELETE, mutex_ptr, TX_POINTER_TO_ULONG_CONVERT(&thread_ptr), 0, 0, TX_TRACE_MUTEX_EVENTS)

    /* Optional mutex delete extended processing.  */
    TX_MUTEX_DELETE_EXTENSION(mutex_ptr)

    /* If trace is enabled, unregister this object.  */
    TX_TRACE_OBJECT_UNREGISTER(mutex_ptr)

    /* Log this kernel call.  */
    TX_EL_MUTEX_DELETE_INSERT

    /* Clear the mutex ID to make it invalid.  */
    mutex_ptr -> tx_mutex_id =  TX_CLEAR_ID;

    /* Decrement the created count.  */
    _tx_mutex_created_count--;

    /* See if the mutex is the only one on the list.  */
    if (_tx_mutex_created_count == TX_EMPTY)
    {

        /* Only created mutex, just set the created list to NULL.  */
        _tx_mutex_created_ptr =  TX_NULL;
    }
    else
    {

        /* Link-up the neighbors.  */
        next_mutex =                               mutex_ptr -> tx_mutex_created_next;
        previous_mutex =                           mutex_ptr -> tx_mutex_created_previous;
        next_mutex -> tx_mutex_created_previous =  previous_mutex;
        previous_mutex -> tx_mutex_created_next =  next_mutex;

        /* See if we have to update the created list head pointer.  */
        if (_tx_mutex_created_ptr == mutex_ptr)
        {

            /* Yes, move the head pointer to the next link. */
            _tx_mutex_created_ptr =  next_mutex;
        }
    }

    /* Temporarily disable preemption.  */
    _tx_thread_preempt_disable++;

    /* Pickup the suspension information.  */
    thread_ptr =                             mutex_ptr -> tx_mutex_suspension_list;
    mutex_ptr -> tx_mutex_suspension_list =  TX_NULL;
    suspended_count =                        mutex_ptr -> tx_mutex_suspended_count;
    mutex_ptr -> tx_mutex_suspended_count =  TX_NO_SUSPENSIONS;


    /* Determine if the mutex is currently on a thread's ownership list.  */

    /* Setup pointer to owner of mutex.  */
    owner_thread =  mutex_ptr -> tx_mutex_owner;

    /* Determine if there is a valid thread pointer.  */
    if (owner_thread != TX_NULL)
    {

        /* Yes, remove this mutex from the owned list.  */

        /* Set the ownership count to 1.  */
        mutex_ptr -> tx_mutex_ownership_count =  ((UINT) 1);

        /* Restore interrupts.   */
        TX_RESTORE

#ifdef TX_MISRA_ENABLE
        /* Release the mutex.  */
        do
        {
            status =  _tx_mutex_put(mutex_ptr);
        } while (status != TX_SUCCESS);
#else
        _tx_mutex_put(mutex_ptr);
#endif

        /* Disable interrupts.  */
        TX_DISABLE
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Walk through the mutex list to resume any and all threads suspended
       on this mutex.  */
    while (suspended_count != ((ULONG) 0))
    {

        /* Decrement the suspension count.  */
        suspended_count--;

        /* Lockout interrupts.  */
        TX_DISABLE

        /* Clear the cleanup pointer, this prevents the timeout from doing
           anything.  */
        thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

        /* Set the return status in the thread to TX_DELETED.  */
        thread_ptr -> tx_thread_suspend_status =  TX_DELETED;

        /* Move the thread pointer ahead.  */
        next_thread =  thread_ptr -> tx_thread_suspended_next;

#ifdef TX_NOT_INTERRUPTABLE

        /* Resume the thread!  */
        _tx_thread_system_ni_resume(thread_ptr);

        /* Restore interrupts.  */
        TX_RESTORE
#else

        /* Temporarily disable preemption again.  */
        _tx_thread_preempt_disable++;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Resume the thread.  */
        _tx_thread_system_resume(thread_ptr);
#endif

        /* Move to next thread.  */
        thread_ptr =  next_thread;
    }

    /* Execute Port-Specific completion processing. If needed, it is typically defined in tx_port.h.  */
    TX_MUTEX_DELETE_PORT_COMPLETION(mutex_ptr)

    /* Disable interrupts.  */
    TX_DISABLE

    /* Release previous preempt disable.  */
    _tx_thread_preempt_disable--;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Check for preemption.  */
    _tx_thread_system_preempt_check();

    /* Return TX_SUCCESS.  */
    return(TX_SUCCESS);
}

