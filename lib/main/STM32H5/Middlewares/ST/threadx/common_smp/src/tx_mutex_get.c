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
/*    _tx_mutex_get                                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the specified mutex.  If the calling thread      */
/*    already owns the mutex, an ownership count is simply increased.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mutex_ptr                         Pointer to mutex control block    */
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
/*    _tx_mutex_priority_change         Inherit thread priority           */
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
UINT  _tx_mutex_get(TX_MUTEX *mutex_ptr, ULONG wait_option)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD       *thread_ptr;
TX_MUTEX        *next_mutex;
TX_MUTEX        *previous_mutex;
TX_THREAD       *mutex_owner;
TX_THREAD       *next_thread;
TX_THREAD       *previous_thread;
UINT            status;


    /* Disable interrupts to get an instance from the mutex.  */
    TX_DISABLE

#ifdef TX_MUTEX_ENABLE_PERFORMANCE_INFO

    /* Increment the total mutex get counter.  */
    _tx_mutex_performance_get_count++;

    /* Increment the number of attempts to get this mutex.  */
    mutex_ptr -> tx_mutex_performance_get_count++;
#endif

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_MUTEX_GET, mutex_ptr, wait_option, TX_POINTER_TO_ULONG_CONVERT(mutex_ptr -> tx_mutex_owner), mutex_ptr -> tx_mutex_ownership_count, TX_TRACE_MUTEX_EVENTS)

    /* Log this kernel call.  */
    TX_EL_MUTEX_GET_INSERT

    /* Pickup thread pointer.  */
    TX_THREAD_GET_CURRENT(thread_ptr)

    /* Determine if this mutex is available.  */
    if (mutex_ptr -> tx_mutex_ownership_count == ((UINT) 0))
    {

        /* Set the ownership count to 1.  */
        mutex_ptr -> tx_mutex_ownership_count =  ((UINT) 1);

        /* Remember that the calling thread owns the mutex.  */
        mutex_ptr -> tx_mutex_owner =  thread_ptr;

        /* Determine if the thread pointer is valid.  */
        if (thread_ptr != TX_NULL)
        {

            /* Determine if priority inheritance is required.  */
            if (mutex_ptr -> tx_mutex_inherit == TX_TRUE)
            {

                /* Remember the current priority of thread.  */
                mutex_ptr -> tx_mutex_original_priority =   thread_ptr -> tx_thread_priority;

                /* Setup the highest priority waiting thread.  */
                mutex_ptr -> tx_mutex_highest_priority_waiting =  ((UINT) TX_MAX_PRIORITIES);
            }

            /* Pickup next mutex pointer, which is the head of the list.  */
            next_mutex =  thread_ptr -> tx_thread_owned_mutex_list;

            /* Determine if this thread owns any other mutexes that have priority inheritance.  */
            if (next_mutex != TX_NULL)
            {

                /* Non-empty list. Link up the mutex.  */

                /* Pickup the next and previous mutex pointer.  */
                previous_mutex =  next_mutex -> tx_mutex_owned_previous;

                /* Place the owned mutex in the list.  */
                next_mutex -> tx_mutex_owned_previous =  mutex_ptr;
                previous_mutex -> tx_mutex_owned_next =  mutex_ptr;

                /* Setup this mutex's next and previous created links.  */
                mutex_ptr -> tx_mutex_owned_previous =  previous_mutex;
                mutex_ptr -> tx_mutex_owned_next =      next_mutex;
            }
            else
            {

                /* The owned mutex list is empty.  Add mutex to empty list.  */
                thread_ptr -> tx_thread_owned_mutex_list =     mutex_ptr;
                mutex_ptr -> tx_mutex_owned_next =             mutex_ptr;
                mutex_ptr -> tx_mutex_owned_previous =         mutex_ptr;
            }

            /* Increment the number of mutexes owned counter.  */
            thread_ptr -> tx_thread_owned_mutex_count++;
        }

        /* Restore interrupts.  */
        TX_RESTORE

        /* Return success.  */
        status =  TX_SUCCESS;
    }

    /* Otherwise, see if the owning thread is trying to obtain the same mutex.  */
    else if (mutex_ptr -> tx_mutex_owner == thread_ptr)
    {

        /* The owning thread is requesting the mutex again, just
           increment the ownership count.  */
        mutex_ptr -> tx_mutex_ownership_count++;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Return success.  */
        status =  TX_SUCCESS;
    }
    else
    {

        /* Determine if the request specifies suspension.  */
        if (wait_option != TX_NO_WAIT)
        {

            /* Determine if the preempt disable flag is non-zero.  */
            if (_tx_thread_preempt_disable != ((UINT) 0))
            {

                /* Restore interrupts.  */
                TX_RESTORE

                /* Suspension is not allowed if the preempt disable flag is non-zero at this point - return error completion.  */
                status =  TX_NOT_AVAILABLE;
            }
            else
            {

                /* Prepare for suspension of this thread.  */

                /* Pickup the mutex owner.  */
                mutex_owner =  mutex_ptr -> tx_mutex_owner;

#ifdef TX_MUTEX_ENABLE_PERFORMANCE_INFO

                /* Increment the total mutex suspension counter.  */
                _tx_mutex_performance_suspension_count++;

                /* Increment the number of suspensions on this mutex.  */
                mutex_ptr -> tx_mutex_performance_suspension_count++;

                /* Determine if a priority inversion is present.  */
                if (thread_ptr -> tx_thread_priority < mutex_owner -> tx_thread_priority)
                {

                    /* Yes, priority inversion is present!  */

                    /* Increment the total mutex priority inversions counter.  */
                    _tx_mutex_performance_priority_inversion_count++;

                    /* Increment the number of priority inversions on this mutex.  */
                    mutex_ptr -> tx_mutex_performance_priority_inversion_count++;

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

                    /* Increment the number of total thread priority inversions.  */
                    _tx_thread_performance_priority_inversion_count++;

                    /* Increment the number of priority inversions for this thread.  */
                    thread_ptr -> tx_thread_performance_priority_inversion_count++;
#endif
                }
#endif

                /* Setup cleanup routine pointer.  */
                thread_ptr -> tx_thread_suspend_cleanup =  &(_tx_mutex_cleanup);

                /* Setup cleanup information, i.e. this mutex control
                   block.  */
                thread_ptr -> tx_thread_suspend_control_block =  (VOID *) mutex_ptr;

#ifndef TX_NOT_INTERRUPTABLE

                /* Increment the suspension sequence number, which is used to identify
                   this suspension event.  */
                thread_ptr -> tx_thread_suspension_sequence++;
#endif

                /* Setup suspension list.  */
                if (mutex_ptr -> tx_mutex_suspended_count == TX_NO_SUSPENSIONS)
                {

                    /* No other threads are suspended.  Setup the head pointer and
                       just setup this threads pointers to itself.  */
                    mutex_ptr -> tx_mutex_suspension_list =         thread_ptr;
                    thread_ptr -> tx_thread_suspended_next =        thread_ptr;
                    thread_ptr -> tx_thread_suspended_previous =    thread_ptr;
                }
                else
                {

                    /* This list is not NULL, add current thread to the end. */
                    next_thread =                                   mutex_ptr -> tx_mutex_suspension_list;
                    thread_ptr -> tx_thread_suspended_next =        next_thread;
                    previous_thread =                               next_thread -> tx_thread_suspended_previous;
                    thread_ptr -> tx_thread_suspended_previous =    previous_thread;
                    previous_thread -> tx_thread_suspended_next =   thread_ptr;
                    next_thread -> tx_thread_suspended_previous =   thread_ptr;
                }

                /* Increment the suspension count.  */
                mutex_ptr -> tx_mutex_suspended_count++;

                /* Set the state to suspended.  */
                thread_ptr -> tx_thread_state =    TX_MUTEX_SUSP;

#ifdef TX_NOT_INTERRUPTABLE

                /* Determine if we need to raise the priority of the thread
                   owning the mutex.  */
                if (mutex_ptr -> tx_mutex_inherit == TX_TRUE)
                {

                    /* Determine if this is the highest priority to raise for this mutex.  */
                    if (mutex_ptr -> tx_mutex_highest_priority_waiting > thread_ptr -> tx_thread_priority)
                    {

                        /* Remember this priority.  */
                        mutex_ptr -> tx_mutex_highest_priority_waiting =  thread_ptr -> tx_thread_priority;
                    }

                    /* Determine if we have to update inherit priority level of the mutex owner.  */
                    if (thread_ptr -> tx_thread_priority < mutex_owner -> tx_thread_inherit_priority)
                    {

                        /* Remember the new priority inheritance priority.  */
                        mutex_owner -> tx_thread_inherit_priority =  thread_ptr -> tx_thread_priority;
                    }

                    /* Priority inheritance is requested, check to see if the thread that owns the mutex is lower priority.  */
                    if (mutex_owner -> tx_thread_priority > thread_ptr -> tx_thread_priority)
                    {

                        /* Yes, raise the suspended, owning thread's priority to that
                           of the current thread.  */
                        _tx_mutex_priority_change(mutex_owner, thread_ptr -> tx_thread_priority);

#ifdef TX_MUTEX_ENABLE_PERFORMANCE_INFO

                        /* Increment the total mutex priority inheritance counter.  */
                        _tx_mutex_performance__priority_inheritance_count++;

                        /* Increment the number of priority inheritance situations on this mutex.  */
                        mutex_ptr -> tx_mutex_performance__priority_inheritance_count++;
#endif
                    }
                }

                /* Call actual non-interruptable thread suspension routine.  */
                _tx_thread_system_ni_suspend(thread_ptr, wait_option);

                /* Restore interrupts.  */
                TX_RESTORE
#else

                /* Set the suspending flag.  */
                thread_ptr -> tx_thread_suspending =  TX_TRUE;

                /* Setup the timeout period.  */
                thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  wait_option;

                /* Temporarily disable preemption.  */
                _tx_thread_preempt_disable++;

                /* Restore interrupts.  */
                TX_RESTORE

                /* Determine if we need to raise the priority of the thread
                   owning the mutex.  */
                if (mutex_ptr -> tx_mutex_inherit == TX_TRUE)
                {

                    /* Determine if this is the highest priority to raise for this mutex.  */
                    if (mutex_ptr -> tx_mutex_highest_priority_waiting > thread_ptr -> tx_thread_priority)
                    {

                        /* Remember this priority.  */
                        mutex_ptr -> tx_mutex_highest_priority_waiting =  thread_ptr -> tx_thread_priority;
                    }

                    /* Determine if we have to update inherit priority level of the mutex owner.  */
                    if (thread_ptr -> tx_thread_priority < mutex_owner -> tx_thread_inherit_priority)
                    {

                        /* Remember the new priority inheritance priority.  */
                        mutex_owner -> tx_thread_inherit_priority =  thread_ptr -> tx_thread_priority;
                    }

                    /* Priority inheritance is requested, check to see if the thread that owns the mutex is lower priority.  */
                    if (mutex_owner -> tx_thread_priority > thread_ptr -> tx_thread_priority)
                    {

                        /* Yes, raise the suspended, owning thread's priority to that
                           of the current thread.  */
                        _tx_mutex_priority_change(mutex_owner, thread_ptr -> tx_thread_priority);

#ifdef TX_MUTEX_ENABLE_PERFORMANCE_INFO

                        /* Increment the total mutex priority inheritance counter.  */
                        _tx_mutex_performance__priority_inheritance_count++;

                        /* Increment the number of priority inheritance situations on this mutex.  */
                        mutex_ptr -> tx_mutex_performance__priority_inheritance_count++;
#endif
                    }
                }

                /* Call actual thread suspension routine.  */
                _tx_thread_system_suspend(thread_ptr);
#endif
                /* Return the completion status.  */
                status =  thread_ptr -> tx_thread_suspend_status;
            }
        }
        else
        {

            /* Restore interrupts.  */
            TX_RESTORE

            /* Immediate return, return error completion.  */
            status =  TX_NOT_AVAILABLE;
        }
    }

    /* Return completion status.  */
    return(status);
}

