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
/*    _tx_event_flags_get                                 PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the specified event flags from the group,        */
/*    according to the get option.  The get option also specifies whether */
/*    or not the retrieved flags are cleared.                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    group_ptr                         Pointer to group control block    */
/*    requested_event_flags             Event flags requested             */
/*    get_option                        Specifies and/or and clear options*/
/*    actual_flags_ptr                  Pointer to place the actual flags */
/*                                        the service retrieved           */
/*    wait_option                       Suspension option                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_suspend         Suspend thread service            */
/*    _tx_thread_system_ni_suspend      Non-interruptable suspend thread  */
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
/*  04-25-2022      Scott Larson            Modified comment(s),          */
/*                                            handle 0 flags case,        */
/*                                            resulting in version 6.1.11 */
/*  10-31-2022      Scott Larson            Modified comment(s), always   */
/*                                            return actual flags,        */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT  _tx_event_flags_get(TX_EVENT_FLAGS_GROUP *group_ptr, ULONG requested_flags,
                    UINT get_option, ULONG *actual_flags_ptr, ULONG wait_option)
{

TX_INTERRUPT_SAVE_AREA

UINT            status;
UINT            and_request;
UINT            clear_request;
ULONG           current_flags;
ULONG           flags_satisfied;
#ifndef TX_NOT_INTERRUPTABLE
ULONG           delayed_clear_flags;
#endif
UINT            suspended_count;
TX_THREAD       *thread_ptr;
TX_THREAD       *next_thread;
TX_THREAD       *previous_thread;
#ifndef TX_NOT_INTERRUPTABLE
UINT            interrupted_set_request;
#endif


    /* Disable interrupts to examine the event flags group.  */
    TX_DISABLE

#ifdef TX_EVENT_FLAGS_ENABLE_PERFORMANCE_INFO

    /* Increment the total event flags get counter.  */
    _tx_event_flags_performance_get_count++;

    /* Increment the number of event flags gets on this semaphore.  */
    group_ptr -> tx_event_flags_group__performance_get_count++;
#endif

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_EVENT_FLAGS_GET, group_ptr, requested_flags, group_ptr -> tx_event_flags_group_current, get_option, TX_TRACE_EVENT_FLAGS_EVENTS)

    /* Log this kernel call.  */
    TX_EL_EVENT_FLAGS_GET_INSERT

    /* Pickup current flags.  */
    current_flags =  group_ptr -> tx_event_flags_group_current;

    /* Return the actual event flags and apply delayed clearing.  */
    *actual_flags_ptr =  current_flags & ~group_ptr -> tx_event_flags_group_delayed_clear;

    /* Apply the event flag option mask.  */
    and_request =  (get_option & TX_AND);

#ifdef TX_NOT_INTERRUPTABLE

    /* Check for AND condition. All flags must be present to satisfy request.  */
    if (and_request == TX_AND)
    {

        /* AND request is present.  */

        /* Calculate the flags present.  */
        flags_satisfied =  (current_flags & requested_flags);

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
        flags_satisfied =  (current_flags & requested_flags);
    }

    /* Determine if the request is satisfied.  */
    if (flags_satisfied != ((ULONG) 0))
    {

        /* Pickup the clear bit.  */
        clear_request =  (get_option & TX_EVENT_FLAGS_CLEAR_MASK);

        /* Determine whether or not clearing needs to take place.  */
        if (clear_request == TX_TRUE)
        {

             /* Yes, clear the flags that satisfied this request.  */
             group_ptr -> tx_event_flags_group_current =
                                        group_ptr -> tx_event_flags_group_current & (~requested_flags);
        }

        /* Return success.  */
        status =  TX_SUCCESS;
    }

#else

    /* Pickup delayed clear flags.  */
    delayed_clear_flags =  group_ptr -> tx_event_flags_group_delayed_clear;

    /* Determine if there are any delayed clear operations pending.  */
    if (delayed_clear_flags != ((ULONG) 0))
    {

        /* Yes, apply them to the current flags.  */
        current_flags =  current_flags & (~delayed_clear_flags);
    }

    /* Check for AND condition. All flags must be present to satisfy request.  */
    if (and_request == TX_AND)
    {

        /* AND request is present.  */

        /* Calculate the flags present.  */
        flags_satisfied =  (current_flags & requested_flags);

        /* Determine if they satisfy the AND request.  */
        if (flags_satisfied != requested_flags)
        {

            /* No, not all the requested flags are present. Clear the flags present variable.  */
            flags_satisfied =  ((ULONG) 0);
        }
    }
    else
    {

        /* OR request is present. Simply AND together the requested flags and the current flags
           to see if any are present.  */
        flags_satisfied =  (current_flags & requested_flags);
    }

    /* Determine if the request is satisfied.  */
    if (flags_satisfied != ((ULONG) 0))
    {

        /* Yes, this request can be handled immediately.  */

        /* Pickup the clear bit.  */
        clear_request =  (get_option & TX_EVENT_FLAGS_CLEAR_MASK);

        /* Determine whether or not clearing needs to take place.  */
        if (clear_request == TX_TRUE)
        {

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

                /* A previous set operation is was interrupted, we need to defer the
                   event clearing until the set operation is complete.  */

                /* Remember the events to clear.  */
                group_ptr -> tx_event_flags_group_delayed_clear =
                                        group_ptr -> tx_event_flags_group_delayed_clear | requested_flags;
            }
            else
            {

                /* Yes, clear the flags that satisfied this request.  */
                group_ptr -> tx_event_flags_group_current =
                                        group_ptr -> tx_event_flags_group_current & ~requested_flags;
            }
        }

        /* Set status to success.  */
        status =  TX_SUCCESS;
    }

#endif
    else
    {
        /* flags_satisfied is 0.  */
        /* Determine if the request specifies suspension.  */
        if (wait_option != TX_NO_WAIT)
        {

            /* Determine if the preempt disable flag is non-zero OR the requested events is 0.  */
            if ((_tx_thread_preempt_disable != ((UINT) 0)) || (requested_flags == (UINT) 0))
            {

                /* Suspension is not allowed if the preempt disable flag is non-zero at this point,
                   or if requested_flags is 0, return error completion.  */
                status =  TX_NO_EVENTS;
            }
            else
            {

                /* Prepare for suspension of this thread.  */

#ifdef TX_EVENT_FLAGS_ENABLE_PERFORMANCE_INFO

                /* Increment the total event flags suspensions counter.  */
                _tx_event_flags_performance_suspension_count++;

                /* Increment the number of event flags suspensions on this semaphore.  */
                group_ptr -> tx_event_flags_group___performance_suspension_count++;
#endif

                /* Pickup thread pointer.  */
                TX_THREAD_GET_CURRENT(thread_ptr)

                /* Setup cleanup routine pointer.  */
                thread_ptr -> tx_thread_suspend_cleanup =  &(_tx_event_flags_cleanup);

                /* Remember which event flags we are looking for.  */
                thread_ptr -> tx_thread_suspend_info =  requested_flags;

                /* Save the get option as well.  */
                thread_ptr -> tx_thread_suspend_option =  get_option;

                /* Save the destination for the current events.  */
                thread_ptr -> tx_thread_additional_suspend_info =  (VOID *) actual_flags_ptr;

                /* Setup cleanup information, i.e. this event flags group control
                   block.  */
                thread_ptr -> tx_thread_suspend_control_block =  (VOID *) group_ptr;

#ifndef TX_NOT_INTERRUPTABLE

                /* Increment the suspension sequence number, which is used to identify
                   this suspension event.  */
                thread_ptr -> tx_thread_suspension_sequence++;
#endif

                /* Pickup the suspended count.  */
                suspended_count =  group_ptr -> tx_event_flags_group_suspended_count;

                /* Setup suspension list.  */
                if (suspended_count == TX_NO_SUSPENSIONS)
                {

                    /* No other threads are suspended.  Setup the head pointer and
                       just setup this threads pointers to itself.  */
                    group_ptr -> tx_event_flags_group_suspension_list =   thread_ptr;
                    thread_ptr -> tx_thread_suspended_next =              thread_ptr;
                    thread_ptr -> tx_thread_suspended_previous =          thread_ptr;
                }
                else
                {

                    /* This list is not NULL, add current thread to the end. */
                    next_thread =                                   group_ptr -> tx_event_flags_group_suspension_list;
                    thread_ptr -> tx_thread_suspended_next =        next_thread;
                    previous_thread =                               next_thread -> tx_thread_suspended_previous;
                    thread_ptr -> tx_thread_suspended_previous =    previous_thread;
                    previous_thread -> tx_thread_suspended_next =   thread_ptr;
                    next_thread -> tx_thread_suspended_previous =   thread_ptr;
                }

                /* Increment the number of threads suspended.  */
                group_ptr -> tx_event_flags_group_suspended_count++;

                /* Set the state to suspended.  */
                thread_ptr -> tx_thread_state =    TX_EVENT_FLAG;

#ifdef TX_NOT_INTERRUPTABLE

                /* Call actual non-interruptable thread suspension routine.  */
                _tx_thread_system_ni_suspend(thread_ptr, wait_option);

                /* Return the completion status.  */
                status =  thread_ptr -> tx_thread_suspend_status;
#else

                /* Set the suspending flag.  */
                thread_ptr -> tx_thread_suspending =  TX_TRUE;

                /* Setup the timeout period.  */
                thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  wait_option;

                /* Temporarily disable preemption.  */
                _tx_thread_preempt_disable++;

                /* Restore interrupts.  */
                TX_RESTORE

                /* Call actual thread suspension routine.  */
                _tx_thread_system_suspend(thread_ptr);

                /* Disable interrupts.  */
                TX_DISABLE

                /* Return the completion status.  */
                status =  thread_ptr -> tx_thread_suspend_status;
#endif
            }
        }
        else
        {

            /* Immediate return, return error completion.  */
            status =  TX_NO_EVENTS;
        }
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return completion status.  */
    return(status);
}

