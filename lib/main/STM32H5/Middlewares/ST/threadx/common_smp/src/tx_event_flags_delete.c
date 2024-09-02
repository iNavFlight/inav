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
/**   Event Flags                                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_thread.h"
#include "tx_event_flags.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_event_flags_delete                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes the specified event flag group.  All threads  */
/*    suspended on the group are resumed with the TX_DELETED status       */
/*    code.                                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    group_ptr                         Pointer to group control block    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_SUCCESS                        Successful completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
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
UINT  _tx_event_flags_delete(TX_EVENT_FLAGS_GROUP *group_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD               *thread_ptr;
TX_THREAD               *next_thread;
UINT                    suspended_count;
TX_EVENT_FLAGS_GROUP    *next_group;
TX_EVENT_FLAGS_GROUP    *previous_group;


    /* Disable interrupts to remove the group from the created list.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_EVENT_FLAGS_DELETE, group_ptr, TX_POINTER_TO_ULONG_CONVERT(&thread_ptr), 0, 0, TX_TRACE_EVENT_FLAGS_EVENTS)

    /* Optional event flags group delete extended processing.  */
    TX_EVENT_FLAGS_GROUP_DELETE_EXTENSION(group_ptr)

    /* If trace is enabled, unregister this object.  */
    TX_TRACE_OBJECT_UNREGISTER(group_ptr)

    /* Log this kernel call.  */
    TX_EL_EVENT_FLAGS_DELETE_INSERT

    /* Clear the event flag group ID to make it invalid.  */
    group_ptr -> tx_event_flags_group_id =  TX_CLEAR_ID;

    /* Decrement the number of created event flag groups.  */
    _tx_event_flags_created_count--;

    /* See if this group is the only one on the list.  */
    if (_tx_event_flags_created_count == TX_EMPTY)
    {

        /* Only created event flag group, just set the created list to NULL.  */
        _tx_event_flags_created_ptr =  TX_NULL;
    }
    else
    {

        /* Link-up the neighbors.  */
        next_group =                                           group_ptr -> tx_event_flags_group_created_next;
        previous_group =                                       group_ptr -> tx_event_flags_group_created_previous;
        next_group -> tx_event_flags_group_created_previous =  previous_group;
        previous_group -> tx_event_flags_group_created_next =  next_group;

        /* See if we have to update the created list head pointer.  */
        if (_tx_event_flags_created_ptr == group_ptr)
        {

            /* Yes, move the head pointer to the next link. */
            _tx_event_flags_created_ptr =  next_group;
        }
    }

    /* Temporarily disable preemption.  */
    _tx_thread_preempt_disable++;

    /* Pickup the suspension information.  */
    thread_ptr =                                         group_ptr -> tx_event_flags_group_suspension_list;
    group_ptr -> tx_event_flags_group_suspension_list =  TX_NULL;
    suspended_count =                                    group_ptr -> tx_event_flags_group_suspended_count;
    group_ptr -> tx_event_flags_group_suspended_count =  TX_NO_SUSPENSIONS;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Walk through the event flag suspension list to resume any and all threads
       suspended on this group.  */
    while (suspended_count != TX_NO_SUSPENSIONS)
    {

        /* Decrement the number of suspended threads.  */
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
    TX_EVENT_FLAGS_GROUP_DELETE_PORT_COMPLETION(group_ptr)

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

