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
/*    _tx_event_flags_set                                 PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the specified flags in the event group based on  */
/*    the set option specified.  All threads suspended on the group whose */
/*    get request can now be satisfied are resumed.                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    group_ptr                         Pointer to group control block    */
/*    flags_to_set                      Event flags to set                */
/*    set_option                        Specified either AND or OR        */
/*                                        operation on the event flags    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_SUCCESS                        Always returns success            */
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
/*  05-19-2020      William E. Lamie        Initial Version 6.0           */
/*  09-30-2020      Yuxin Zhou              Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-25-2022      William E. Lamie        Modified comment(s), and      */
/*                                            added corrected preemption  */
/*                                            check logic, resulting in   */
/*                                            version 6.1.11              */
/*                                                                        */
/**************************************************************************/
UINT  _tx_event_flags_set(TX_EVENT_FLAGS_GROUP *group_ptr, ULONG flags_to_set, UINT set_option)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD       *thread_ptr;
TX_THREAD       *next_thread_ptr;
TX_THREAD       *next_thread;
TX_THREAD       *previous_thread;
TX_THREAD       *satisfied_list;
TX_THREAD       *last_satisfied;
TX_THREAD       *suspended_list;
UINT            suspended_count;
ULONG           current_event_flags;
ULONG           requested_flags;
ULONG           flags_satisfied;
ULONG           *suspend_info_ptr;
UINT            and_request;
UINT            get_option;
UINT            clear_request;
UINT            preempt_check;
#ifndef TX_NOT_INTERRUPTABLE
UINT            interrupted_set_request;
#endif
#ifndef TX_DISABLE_NOTIFY_CALLBACKS
VOID            (*events_set_notify)(struct TX_EVENT_FLAGS_GROUP_STRUCT *notify_group_ptr);
#endif


    /* Disable interrupts to remove the semaphore from the created list.  */
    TX_DISABLE

#ifdef TX_EVENT_FLAGS_ENABLE_PERFORMANCE_INFO

    /* Increment the total event flags set counter.  */
    _tx_event_flags_performance_set_count++;

    /* Increment the number of event flags sets on this semaphore.  */
    group_ptr -> tx_event_flags_group_performance_set_count++;
#endif

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_EVENT_FLAGS_SET, group_ptr, flags_to_set, set_option, group_ptr -> tx_event_flags_group_suspended_count, TX_TRACE_EVENT_FLAGS_EVENTS)

    /* Log this kernel call.  */
    TX_EL_EVENT_FLAGS_SET_INSERT

    /* Determine how to set this group's event flags.  */
    if ((set_option & TX_EVENT_FLAGS_AND_MASK) == TX_AND)
    {

#ifndef TX_NOT_INTERRUPTABLE

        /* Set interrupted set request flag to false.  */
        interrupted_set_request =  TX_FALSE;

        /* Determine if the suspension list is being processed by an interrupted
           set request.  */
        if (group_ptr -> tx_event_flags_group_suspended_count != TX_NO_SUSPENSIONS)
        {

            if (group_ptr -> tx_event_flags_group_suspension_list == TX_NULL)
            {

                /* Set the interrupted set request flag.  */
                interrupted_set_request =  TX_TRUE;
            }
        }

        /* Was a set request interrupted?  */
        if (interrupted_set_request == TX_TRUE)
        {

            /* A previous set operation was interrupted, we need to defer the
               event clearing until the set operation is complete.  */

            /* Remember the events to clear.  */
            group_ptr -> tx_event_flags_group_delayed_clear =
                                        group_ptr -> tx_event_flags_group_delayed_clear | ~flags_to_set;
        }
        else
        {
#endif

            /* Previous set operation was not interrupted, simply clear the
               specified flags by "ANDing" the flags into the current events
               of the group.  */
            group_ptr -> tx_event_flags_group_current =
                group_ptr -> tx_event_flags_group_current & flags_to_set;

#ifndef TX_NOT_INTERRUPTABLE

        }
#endif

        /* Restore interrupts.  */
        TX_RESTORE
    }
    else
    {

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

        /* Pickup the notify callback routine for this event flag group.  */
        events_set_notify =  group_ptr -> tx_event_flags_group_set_notify;
#endif

        /* "OR" the flags into the current events of the group.  */
        group_ptr -> tx_event_flags_group_current =
            group_ptr -> tx_event_flags_group_current | flags_to_set;

#ifndef TX_NOT_INTERRUPTABLE

        /* Determine if there are any delayed flags to clear.  */
        if (group_ptr -> tx_event_flags_group_delayed_clear != ((ULONG) 0))
        {

            /* Yes, we need to neutralize the delayed clearing as well.  */
            group_ptr -> tx_event_flags_group_delayed_clear =
                                        group_ptr -> tx_event_flags_group_delayed_clear & ~flags_to_set;
        }
#endif

        /* Clear the preempt check flag.  */
        preempt_check =  TX_FALSE;

        /* Pickup the thread suspended count.  */
        suspended_count =  group_ptr -> tx_event_flags_group_suspended_count;

        /* Determine if there are any threads suspended on the event flag group.  */
        if (group_ptr -> tx_event_flags_group_suspension_list != TX_NULL)
        {

            /* Determine if there is just a single thread waiting on the event
               flag group.  */
            if (suspended_count == ((UINT) 1))
            {

                /* Single thread waiting for event flags.  Bypass the multiple thread
                   logic.  */

                /* Setup thread pointer.  */
                thread_ptr =  group_ptr -> tx_event_flags_group_suspension_list;

                /* Pickup the current event flags.  */
                current_event_flags =  group_ptr -> tx_event_flags_group_current;

                /* Pickup the suspend information.  */
                requested_flags =  thread_ptr -> tx_thread_suspend_info;

                /* Pickup the suspend option.  */
                get_option =  thread_ptr -> tx_thread_suspend_option;

                /* Isolate the AND selection.  */
                and_request =  (get_option & TX_AND);

                /* Check for AND condition. All flags must be present to satisfy request.  */
                if (and_request == TX_AND)
                {

                    /* AND request is present.  */

                    /* Calculate the flags present.  */
                    flags_satisfied =  (current_event_flags & requested_flags);

                    /* Determine if they satisfy the AND request.  */
                    if (flags_satisfied != requested_flags)
                    {

                        /* No, not all the requested flags are present. Clear the flags present variable.  */
                        flags_satisfied =  ((ULONG) 0);
                    }
                }
                else
                {

                    /* OR request is present. Simply or the requested flags and the current flags.  */
                    flags_satisfied =  (current_event_flags & requested_flags);
                }

                /* Determine if the request is satisfied.  */
                if (flags_satisfied != ((ULONG) 0))
                {

                    /* Yes, resume the thread and apply any event flag
                       clearing.  */

                    /* Return the actual event flags that satisfied the request.  */
                    suspend_info_ptr =   TX_VOID_TO_ULONG_POINTER_CONVERT(thread_ptr -> tx_thread_additional_suspend_info);
                    *suspend_info_ptr =  current_event_flags;

                    /* Pickup the clear bit.  */
                    clear_request =  (get_option & TX_EVENT_FLAGS_CLEAR_MASK);

                    /* Determine whether or not clearing needs to take place.  */
                    if (clear_request == TX_TRUE)
                    {

                        /* Yes, clear the flags that satisfied this request.  */
                        group_ptr -> tx_event_flags_group_current =  group_ptr -> tx_event_flags_group_current & (~requested_flags);
                    }

                    /* Clear the suspension information in the event flag group.  */
                    group_ptr -> tx_event_flags_group_suspension_list =  TX_NULL;
                    group_ptr -> tx_event_flags_group_suspended_count =  TX_NO_SUSPENSIONS;

                    /* Clear cleanup routine to avoid timeout.  */
                    thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

                    /* Put return status into the thread control block.  */
                    thread_ptr -> tx_thread_suspend_status =  TX_SUCCESS;

#ifdef TX_NOT_INTERRUPTABLE

                    /* Resume the thread!  */
                    _tx_thread_system_ni_resume(thread_ptr);
#else

                    /* Temporarily disable preemption.  */
                    _tx_thread_preempt_disable++;

                    /* Restore interrupts.  */
                    TX_RESTORE

                    /* Resume thread.  */
                    _tx_thread_system_resume(thread_ptr);

                    /* Disable interrupts to remove the semaphore from the created list.  */
                    TX_DISABLE
#endif
                }
            }
            else
            {

                /* Otherwise, the event flag requests of multiple threads must be
                   examined.  */

                /* Setup thread pointer, keep a local copy of the head pointer.  */
                suspended_list =  group_ptr -> tx_event_flags_group_suspension_list;
                thread_ptr =      suspended_list;

                /* Clear the suspended list head pointer to thwart manipulation of
                   the list in ISR's while we are processing here.  */
                group_ptr -> tx_event_flags_group_suspension_list =  TX_NULL;

                /* Setup the satisfied thread pointers.  */
                satisfied_list =  TX_NULL;
                last_satisfied =  TX_NULL;

                /* Pickup the current event flags.  */
                current_event_flags =  group_ptr -> tx_event_flags_group_current;

                /* Disable preemption while we process the suspended list.  */
                _tx_thread_preempt_disable++;

                /* Since we have temporarily disabled preemption globally, set the preempt 
                   check flag to check for any preemption condition - including from 
                   unrelated ISR processing.  */
                preempt_check =  TX_TRUE;

                /* Loop to examine all of the suspended threads. */
                do
                {

#ifndef TX_NOT_INTERRUPTABLE

                    /* Restore interrupts temporarily.  */
                    TX_RESTORE

                    /* Disable interrupts again.  */
                    TX_DISABLE
#endif

                    /* Determine if we need to reset the search.  */
                    if (group_ptr -> tx_event_flags_group_reset_search != TX_FALSE)
                    {

                        /* Clear the reset search flag.  */
                        group_ptr -> tx_event_flags_group_reset_search =  TX_FALSE;

                        /* Move the thread pointer to the beginning of the search list.  */
                        thread_ptr =  suspended_list;

                        /* Reset the suspended count.  */
                        suspended_count =  group_ptr -> tx_event_flags_group_suspended_count;

                        /* Update the current events with any new ones that might
                           have been set in a nested set events call from an ISR.  */
                        current_event_flags =  current_event_flags | group_ptr -> tx_event_flags_group_current;
                    }

                    /* Save next thread pointer.  */
                    next_thread_ptr =  thread_ptr -> tx_thread_suspended_next;

                    /* Pickup the suspend information.  */
                    requested_flags =  thread_ptr -> tx_thread_suspend_info;

                    /* Pickup this thread's suspension get option.  */
                    get_option =  thread_ptr -> tx_thread_suspend_option;

                    /* Isolate the AND selection.  */
                    and_request =  (get_option & TX_AND);

                    /* Check for AND condition. All flags must be present to satisfy request.  */
                    if (and_request == TX_AND)
                    {

                        /* AND request is present.  */

                        /* Calculate the flags present.  */
                        flags_satisfied =  (current_event_flags & requested_flags);

                        /* Determine if they satisfy the AND request.  */
                        if (flags_satisfied != requested_flags)
                        {

                            /* No, not all the requested flags are present. Clear the flags present variable.  */
                            flags_satisfied =  ((ULONG) 0);
                        }
                    }
                    else
                    {

                        /* OR request is present. Simply or the requested flags and the current flags.  */
                        flags_satisfied =  (current_event_flags & requested_flags);
                    }

                    /* Check to see if the thread had a timeout or wait abort during the event search processing.
                       If so, just set the flags satisfied to ensure the processing here removes the thread from
                       the suspension list.  */
                    if (thread_ptr -> tx_thread_state != TX_EVENT_FLAG)
                    {

                       /* Simply set the satisfied flags to 1 in order to remove the thread from the suspension list.  */
                        flags_satisfied =  ((ULONG) 1);
                    }

                    /* Determine if the request is satisfied.  */
                    if (flags_satisfied != ((ULONG) 0))
                    {

                        /* Yes, this request can be handled now.  */

                        /* Determine if the thread is still suspended on the event flag group. If not, a wait
                           abort must have been done from an ISR.  */
                        if (thread_ptr -> tx_thread_state == TX_EVENT_FLAG)
                        {

                            /* Return the actual event flags that satisfied the request.  */
                            suspend_info_ptr =   TX_VOID_TO_ULONG_POINTER_CONVERT(thread_ptr -> tx_thread_additional_suspend_info);
                            *suspend_info_ptr =  current_event_flags;

                            /* Pickup the clear bit.  */
                            clear_request =  (get_option & TX_EVENT_FLAGS_CLEAR_MASK);

                            /* Determine whether or not clearing needs to take place.  */
                            if (clear_request == TX_TRUE)
                            {

                                /* Yes, clear the flags that satisfied this request.  */
                                group_ptr -> tx_event_flags_group_current =  group_ptr -> tx_event_flags_group_current & ~requested_flags;
                            }

                            /* Prepare for resumption of the first thread.  */

                            /* Clear cleanup routine to avoid timeout.  */
                            thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

                            /* Put return status into the thread control block.  */
                            thread_ptr -> tx_thread_suspend_status =  TX_SUCCESS;
                        }

                        /* We need to remove the thread from the suspension list and place it in the
                           expired list.  */

                        /* See if this is the only suspended thread on the list.  */
                        if (thread_ptr == thread_ptr -> tx_thread_suspended_next)
                        {

                            /* Yes, the only suspended thread.  */

                            /* Update the head pointer.  */
                            suspended_list =  TX_NULL;
                        }
                        else
                        {

                            /* At least one more thread is on the same expiration list.  */

                            /* Update the links of the adjacent threads.  */
                            next_thread =                                  thread_ptr -> tx_thread_suspended_next;
                            previous_thread =                              thread_ptr -> tx_thread_suspended_previous;
                            next_thread -> tx_thread_suspended_previous =  previous_thread;
                            previous_thread -> tx_thread_suspended_next =  next_thread;

                            /* Update the list head pointer, if removing the head of the
                               list.  */
                            if (suspended_list == thread_ptr)
                            {

                                /* Yes, head pointer needs to be updated.  */
                                suspended_list =  thread_ptr -> tx_thread_suspended_next;
                            }
                        }

                        /* Decrement the suspension count.  */
                        group_ptr -> tx_event_flags_group_suspended_count--;

                        /* Place this thread on the expired list.  */
                        if (satisfied_list == TX_NULL)
                        {

                            /* First thread on the satisfied list.  */
                            satisfied_list =  thread_ptr;
                            last_satisfied =  thread_ptr;

                            /* Setup initial next pointer.  */
                            thread_ptr -> tx_thread_suspended_next =  TX_NULL;
                        }
                        else
                        {

                            /* Not the first thread on the satisfied list.  */

                            /* Link it up at the end.  */
                            last_satisfied -> tx_thread_suspended_next =  thread_ptr;
                            thread_ptr -> tx_thread_suspended_next =      TX_NULL;
                            last_satisfied =                              thread_ptr;
                        }
                    }

                    /* Copy next thread pointer to working thread ptr.  */
                    thread_ptr =  next_thread_ptr;

                    /* Decrement the suspension count.  */
                    suspended_count--;

                } while (suspended_count != TX_NO_SUSPENSIONS);

                /* Setup the group's suspension list head again.  */
                group_ptr -> tx_event_flags_group_suspension_list =  suspended_list;

#ifndef TX_NOT_INTERRUPTABLE

                /* Determine if there is any delayed event clearing to perform.  */
                if (group_ptr -> tx_event_flags_group_delayed_clear != ((ULONG) 0))
                {

                    /* Perform the delayed event clearing.  */
                    group_ptr -> tx_event_flags_group_current =
                        group_ptr -> tx_event_flags_group_current & ~(group_ptr -> tx_event_flags_group_delayed_clear);

                    /* Clear the delayed event flag clear value.  */
                    group_ptr -> tx_event_flags_group_delayed_clear =  ((ULONG) 0);
                }
#endif

                /* Restore interrupts.  */
                TX_RESTORE

                /* Walk through the satisfied list, setup initial thread pointer. */
                thread_ptr =  satisfied_list;
                while(thread_ptr != TX_NULL)
                {

                    /* Get next pointer first.  */
                    next_thread_ptr =  thread_ptr -> tx_thread_suspended_next;

                    /* Disable interrupts.  */
                    TX_DISABLE

#ifdef TX_NOT_INTERRUPTABLE

                    /* Resume the thread!  */
                    _tx_thread_system_ni_resume(thread_ptr);

                    /* Restore interrupts.  */
                    TX_RESTORE
#else

                    /* Disable preemption again.  */
                    _tx_thread_preempt_disable++;

                    /* Restore interrupt posture.  */
                    TX_RESTORE

                    /* Resume the thread.  */
                    _tx_thread_system_resume(thread_ptr);
#endif

                    /* Move next thread to current.  */
                    thread_ptr =  next_thread_ptr;
                }

                /* Disable interrupts.  */
                TX_DISABLE

                /* Release thread preemption disable.  */
                _tx_thread_preempt_disable--;
            }
        }
        else
        {

            /* Determine if we need to set the reset search field.  */
            if (group_ptr -> tx_event_flags_group_suspended_count != TX_NO_SUSPENSIONS)
            {

                /* We interrupted a search of an event flag group suspension
                   list.  Make sure we reset the search.  */
                group_ptr -> tx_event_flags_group_reset_search =  TX_TRUE;
            }
        }

        /* Restore interrupts.  */
        TX_RESTORE

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

        /* Determine if a notify callback is required.  */
        if (events_set_notify != TX_NULL)
        {

            /* Call application event flags set notification.  */
            (events_set_notify)(group_ptr);
        }
#endif

        /* Determine if a check for preemption is necessary.  */
        if (preempt_check == TX_TRUE)
        {

            /* Yes, one or more threads were resumed, check for preemption.  */
            _tx_thread_system_preempt_check();
        }
    }

    /* Return completion status.  */
    return(TX_SUCCESS);
}

