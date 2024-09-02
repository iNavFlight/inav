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
/**   Thread                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/
#define TX_SOURCE_CODE

/* Include necessary system files.  */
#include "tx_api.h"
#include "tx_trace.h"
#include "tx_thread.h"
#ifdef TX_INLINE_THREAD_RESUME_SUSPEND
#ifndef TX_NO_TIMER
#include "tx_timer.h"
#endif
#endif
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_suspend                                  PORTABLE C      */
/*                                                           6.1.1        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles application suspend requests.  If the suspend */
/*    requires actual processing, this function calls the actual suspend  */
/*    thread routine.                                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Pointer to thread to suspend  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Return completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_suspend         Actual thread suspension          */
/*    _tx_thread_system_ni_suspend      Non-interruptable suspend thread  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-16-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            added type cast to address  */
/*                                            a MISRA compliance issue,   */
/*                                            resulting in version 6.1.1  */
/*                                                                        */
/**************************************************************************/
UINT  _tx_thread_suspend(TX_THREAD *thread_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD  *current_thread;
UINT        status;


#ifndef TX_INLINE_THREAD_RESUME_SUSPEND

    /* Lockout interrupts while the thread is being suspended.  */
    TX_DISABLE

    /* Pickup thread pointer.  */
    TX_THREAD_GET_CURRENT(current_thread)

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_SUSPEND_API, thread_ptr, thread_ptr -> tx_thread_state, TX_POINTER_TO_ULONG_CONVERT(&status), 0, TX_TRACE_THREAD_EVENTS)

    /* Log this kernel call.  */
    TX_EL_THREAD_SUSPEND_INSERT

    /* Check the specified thread's current status.  */
    if (thread_ptr -> tx_thread_state == TX_READY)
    {

        /* Initialize status to success.  */
        status =  TX_SUCCESS;

        /* Determine if we are in a thread context.  */
        if (TX_THREAD_GET_SYSTEM_STATE() == ((ULONG) 0))
        {

            /* Yes, we are in a thread context.  */

            /* Determine if the current thread is also the suspending thread.  */
            if (current_thread == thread_ptr)
            {

                /* Now determine if the preempt disable flag is non-zero.  */
                if (_tx_thread_preempt_disable != ((UINT) 0))
                {

                    /* Current thread cannot suspend when the preempt disable flag is non-zero,
                       return an error.  */
                    status =  TX_SUSPEND_ERROR;
                }
            }
        }

        /* Determine if the status is still successful.  */
        if (status == TX_SUCCESS)
        {

            /* Set the state to suspended.  */
            thread_ptr -> tx_thread_state =    TX_SUSPENDED;

#ifdef TX_NOT_INTERRUPTABLE

            /* Call actual non-interruptable thread suspension routine.  */
            _tx_thread_system_ni_suspend(thread_ptr, ((ULONG) 0));

            /* Restore interrupts.  */
            TX_RESTORE
#else

            /* Set the suspending flag. */
            thread_ptr -> tx_thread_suspending =  TX_TRUE;

            /* Setup for no timeout period.  */
            thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  ((ULONG) 0);

            /* Temporarily disable preemption.  */
            _tx_thread_preempt_disable++;

            /* Restore interrupts.  */
            TX_RESTORE

            /* Call actual thread suspension routine.  */
            _tx_thread_system_suspend(thread_ptr);
#endif

#ifdef TX_MISRA_ENABLE

            /* Disable interrupts.  */
            TX_DISABLE

            /* Return success.  */
            status =  TX_SUCCESS;
#else

            /* If MISRA is not enabled, return directly.  */
            return(TX_SUCCESS);
#endif
        }
    }
    else if (thread_ptr -> tx_thread_state == TX_TERMINATED)
    {

        /* Thread is terminated.  */
        status =  TX_SUSPEND_ERROR;
    }
    else if (thread_ptr -> tx_thread_state == TX_COMPLETED)
    {

        /* Thread is completed.  */
        status =  TX_SUSPEND_ERROR;
    }
    else if (thread_ptr -> tx_thread_state == TX_SUSPENDED)
    {

        /* Already suspended, just set status to success.  */
        status =  TX_SUCCESS;
    }
    else
    {

        /* Just set the delayed suspension flag.  */
        thread_ptr -> tx_thread_delayed_suspend =  TX_TRUE;

        /* Set status to success.  */
        status =  TX_SUCCESS;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Always return success, since this function does not perform error
       checking.  */
    return(status);

#else

    /* In-line thread suspension processing follows, which is effectively just taking the
       logic in tx_thread_system_suspend.c and placing it here!  */

UINT            priority;
UINT            base_priority;
ULONG           priority_map;
ULONG           priority_bit;
ULONG           combined_flags;
TX_THREAD       *ready_next;
TX_THREAD       *ready_previous;

#if TX_MAX_PRIORITIES > 32
UINT            map_index;
#endif

#ifdef TX_ENABLE_EVENT_TRACE
TX_TRACE_BUFFER_ENTRY       *entry_ptr;
ULONG                       time_stamp =  ((ULONG) 0);
#endif


    /* Pickup thread pointer.  */
    TX_THREAD_GET_CURRENT(current_thread)

#ifdef TX_ENABLE_STACK_CHECKING

    /* Check this thread's stack.  */
    TX_THREAD_STACK_CHECK(thread_ptr)
#endif

    /* Lockout interrupts while the thread is being suspended.  */
    TX_DISABLE

#ifndef TX_NO_TIMER

    /* Determine if this is the current thread.  */
    if (thread_ptr == current_thread)
    {

        /* Yes, current thread is suspending - reset time slice for current thread.  */
        _tx_timer_time_slice =  thread_ptr -> tx_thread_new_time_slice;
    }
#endif

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_SUSPEND_API, thread_ptr, thread_ptr -> tx_thread_state, TX_POINTER_TO_ULONG_CONVERT(&status), 0, TX_TRACE_THREAD_EVENTS)

    /* Log this kernel call.  */
    TX_EL_THREAD_SUSPEND_INSERT

    /* Check the specified thread's current status.  */
    if (thread_ptr -> tx_thread_state == TX_READY)
    {

        /* Initialize status to success.  */
        status =  TX_SUCCESS;

        /* Determine if we are in a thread context.  */
        if (TX_THREAD_GET_SYSTEM_STATE() == ((ULONG) 0))
        {

            /* Yes, we are in a thread context.  */

            /* Determine if the current thread is also the suspending thread.  */
            if (current_thread == thread_ptr)
            {

                /* Now determine if the preempt disable flag is non-zero.  */
                if (_tx_thread_preempt_disable != ((UINT) 0))
                {

                    /* Current thread cannot suspend when the preempt disable flag is non-zero,
                       return an error.  */
                    status =  TX_SUSPEND_ERROR;
                }
            }
        }

        /* Determine if the status is still successful.  */
        if (status == TX_SUCCESS)
        {

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

            /* Increment the thread's suspend count.  */
            thread_ptr -> tx_thread_performance_suspend_count++;

            /* Increment the total number of thread suspensions.  */
            _tx_thread_performance_suspend_count++;
#endif

            /* Set the state to suspended.  */
            thread_ptr -> tx_thread_state =    TX_SUSPENDED;

            /* Thread state change.  */
            TX_THREAD_STATE_CHANGE(thread_ptr, TX_SUSPENDED)

            /* Log the thread status change.  */
            TX_EL_THREAD_STATUS_CHANGE_INSERT(thread_ptr, thread_ptr -> tx_thread_state)

#ifdef TX_ENABLE_EVENT_TRACE

            /* If trace is enabled, save the current event pointer.  */
            entry_ptr =  _tx_trace_buffer_current_ptr;
#endif

            /* Log the thread status change.  */
            TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_SUSPEND, thread_ptr, ((ULONG) thread_ptr -> tx_thread_state), TX_POINTER_TO_ULONG_CONVERT(&priority), TX_POINTER_TO_ULONG_CONVERT(_tx_thread_execute_ptr), TX_TRACE_INTERNAL_EVENTS)

#ifdef TX_ENABLE_EVENT_TRACE

            /* Save the time stamp for later comparison to verify that
               the event hasn't been overwritten by the time we have
               computed the next thread to execute.  */
            if (entry_ptr != TX_NULL)
            {

                /* Save time stamp.  */
                time_stamp =  entry_ptr -> tx_trace_buffer_entry_time_stamp;
            }
#endif

            /* Pickup priority of thread.  */
            priority =  thread_ptr -> tx_thread_priority;

            /* Pickup the previous and next ready thread pointers.  */
            ready_next =      thread_ptr -> tx_thread_ready_next;
            ready_previous =  thread_ptr -> tx_thread_ready_previous;

            /* Determine if there are other threads at this priority that are
               ready.  */
            if (ready_next != thread_ptr)
            {

                /* Yes, there are other threads at this priority ready.  */

                /* Just remove this thread from the priority list.  */
                ready_next -> tx_thread_ready_previous =    ready_previous;
                ready_previous -> tx_thread_ready_next =    ready_next;

                /* Determine if this is the head of the priority list.  */
                if (_tx_thread_priority_list[priority] == thread_ptr)
                {

                    /* Update the head pointer of this priority list.  */
                    _tx_thread_priority_list[priority] =  ready_next;

#ifndef TX_DISABLE_PREEMPTION_THRESHOLD

#if TX_MAX_PRIORITIES > 32

                    /* Calculate the index into the bit map array.  */
                    map_index =  priority/((UINT) 32);
#endif

                    /* Check for a thread preempted that had preemption threshold set.  */
                    if (_tx_thread_preempted_maps[MAP_INDEX] != ((ULONG) 0))
                    {

                        /* Ensure that this thread's priority is clear in the preempt map.  */
                        TX_MOD32_BIT_SET(priority, priority_bit)
                        _tx_thread_preempted_maps[MAP_INDEX] =  _tx_thread_preempted_maps[MAP_INDEX] & (~(priority_bit));

#if TX_MAX_PRIORITIES > 32

                        /* Determine if there are any other bits set in this preempt map.  */
                        if (_tx_thread_preempted_maps[MAP_INDEX] == ((ULONG) 0))
                        {

                            /* No, clear the active bit to signify this preempt map has nothing set.  */
                            TX_DIV32_BIT_SET(priority, priority_bit)
                            _tx_thread_preempted_map_active =  _tx_thread_preempted_map_active & (~(priority_bit));
                        }
#endif
                    }
#endif
                }
            }
            else
            {

                /* This is the only thread at this priority ready to run.  Set the head
                   pointer to NULL.  */
                _tx_thread_priority_list[priority] =    TX_NULL;

#if TX_MAX_PRIORITIES > 32

                /* Calculate the index into the bit map array.  */
                map_index =  priority/((UINT) 32);
#endif

                /* Clear this priority bit in the ready priority bit map.  */
                TX_MOD32_BIT_SET(priority, priority_bit)
                _tx_thread_priority_maps[MAP_INDEX] =  _tx_thread_priority_maps[MAP_INDEX] & (~(priority_bit));

#if TX_MAX_PRIORITIES > 32

                /* Determine if there are any other bits set in this priority map.  */
                if (_tx_thread_priority_maps[MAP_INDEX] == ((ULONG) 0))
                {

                    /* No, clear the active bit to signify this priority map has nothing set.  */
                    TX_DIV32_BIT_SET(priority, priority_bit)
                    _tx_thread_priority_map_active =  _tx_thread_priority_map_active & (~(priority_bit));
                }
#endif

#ifndef TX_DISABLE_PREEMPTION_THRESHOLD

                /* Check for a thread preempted that had preemption-threshold set.  */
                if (_tx_thread_preempted_maps[MAP_INDEX] != ((ULONG) 0))
                {

                    /* Ensure that this thread's priority is clear in the preempt map.  */
                    TX_MOD32_BIT_SET(priority, priority_bit)
                    _tx_thread_preempted_maps[MAP_INDEX] =  _tx_thread_preempted_maps[MAP_INDEX] & (~(priority_bit));

#if TX_MAX_PRIORITIES > 32

                    /* Determine if there are any other bits set in this preempt map.  */
                    if (_tx_thread_preempted_maps[MAP_INDEX] == ((ULONG) 0))
                    {

                        /* No, clear the active bit to signify this preempted map has nothing set.  */
                        TX_DIV32_BIT_SET(priority, priority_bit)
                        _tx_thread_preempted_map_active =  _tx_thread_preempted_map_active & (~(priority_bit));
                    }
#endif
                }
#endif

#if TX_MAX_PRIORITIES > 32

                /* Calculate the index to find the next highest priority thread ready for execution.  */
                priority_map =    _tx_thread_priority_map_active;

                /* Determine if there is anything.   */
                if (priority_map != ((ULONG) 0))
                {

                    /* Calculate the lowest bit set in the priority map. */
                    TX_LOWEST_SET_BIT_CALCULATE(priority_map, map_index)
                }

                /* Calculate the base priority as well.  */
                base_priority =  map_index * ((UINT) 32);
#else

                /* Setup the base priority to zero.  */
                base_priority =   ((UINT) 0);
#endif

                /* Setup working variable for the priority map.  */
                priority_map =    _tx_thread_priority_maps[MAP_INDEX];

                /* Make a quick check for no other threads ready for execution.  */
                if (priority_map == ((ULONG) 0))
                {

                    /* Nothing else is ready.  Set highest priority and execute thread
                       accordingly.  */
                    _tx_thread_highest_priority =  ((UINT) TX_MAX_PRIORITIES);
                    _tx_thread_execute_ptr =       TX_NULL;

#ifndef TX_MISRA_ENABLE

#ifdef TX_ENABLE_EVENT_TRACE

                    /* Check that the event time stamp is unchanged.  A different
                       timestamp means that a later event wrote over the thread
                       suspend event. In that case, do nothing here.  */
                    if (entry_ptr != TX_NULL)
                    {

                        /* Is the timestamp the same?  */
                        if (time_stamp == entry_ptr -> tx_trace_buffer_entry_time_stamp)
                        {

                            /* Timestamp is the same, set the "next thread pointer" to the new value of the
                               next thread to execute. This can be used by the trace analysis tool to keep
                               track of next thread execution.  */
                            entry_ptr -> tx_trace_buffer_entry_information_field_4 =  0;
                        }
                    }
#endif

                    /* Restore interrupts.  */
                    TX_RESTORE

                    /* Determine if preemption should take place. This is only possible if the current thread pointer is
                       not the same as the execute thread pointer AND the system state and preempt disable flags are clear.  */
                    TX_THREAD_SYSTEM_RETURN_CHECK(combined_flags)
                    if (combined_flags == ((ULONG) 0))
                    {

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

                        /* Yes, increment the return to idle return count.  */
                        _tx_thread_performance_idle_return_count++;
#endif

                        /* Preemption is needed - return to the system!  */
                        _tx_thread_system_return();
                    }

                    /* Return to caller.  */
                    return(TX_SUCCESS);
#endif
                }
                else
                {

                    /* Calculate the lowest bit set in the priority map. */
                    TX_LOWEST_SET_BIT_CALCULATE(priority_map, priority_bit)

                    /* Setup the next highest priority variable.  */
                    _tx_thread_highest_priority =  base_priority + priority_bit;
                }
            }

            /* Determine if this thread is the thread designated to execute.  */
            if (thread_ptr == _tx_thread_execute_ptr)
            {

                /* Pickup the highest priority thread to execute.  */
                _tx_thread_execute_ptr =  _tx_thread_priority_list[_tx_thread_highest_priority];

#ifndef TX_DISABLE_PREEMPTION_THRESHOLD

                /* Determine if a previous thread with preemption-threshold was preempted.  */
#if TX_MAX_PRIORITIES > 32
                if (_tx_thread_preempted_map_active != ((ULONG) 0))
#else
                if (_tx_thread_preempted_maps[MAP_INDEX] != ((ULONG) 0))
#endif
                {

                    /* Yes, there was a thread preempted when it was using preemption-threshold.  */

#ifndef TX_NOT_INTERRUPTABLE

                    /* Disable preemption.  */
                    _tx_thread_preempt_disable++;

                    /* Restore interrupts.  */
                    TX_RESTORE

                    /* Interrupts are enabled briefly here to keep the interrupt
                       lockout time deterministic.  */

                    /* Disable interrupts again.  */
                    TX_DISABLE

                    /* Decrement the preemption disable variable.  */
                    _tx_thread_preempt_disable--;
#endif

                    /* Calculate the thread with preemption threshold set that
                       was interrupted by a thread above the preemption level.  */

#if TX_MAX_PRIORITIES > 32

                    /* Calculate the index to find the next highest priority thread ready for execution.  */
                    priority_map =    _tx_thread_preempted_map_active;

                    /* Calculate the lowest bit set in the priority map. */
                    TX_LOWEST_SET_BIT_CALCULATE(priority_map, map_index)

                    /* Calculate the base priority as well.  */
                    base_priority =  map_index * ((UINT) 32);
#else

                    /* Setup the base priority to zero.  */
                    base_priority =   ((UINT) 0);
#endif

                    /* Setup temporary preempted map.  */
                    priority_map =  _tx_thread_preempted_maps[MAP_INDEX];

                    /* Calculate the lowest bit set in the priority map. */
                    TX_LOWEST_SET_BIT_CALCULATE(priority_map, priority_bit)

                    /* Setup the highest priority preempted thread.  */
                    priority =  base_priority + priority_bit;

                    /* Determine if the next highest priority thread is above the highest priority threshold value.  */
                    if (_tx_thread_highest_priority >= (_tx_thread_priority_list[priority] -> tx_thread_preempt_threshold))
                    {

                        /* Thread not allowed to execute until earlier preempted thread finishes or lowers its
                           preemption-threshold.  */
                        _tx_thread_execute_ptr =  _tx_thread_priority_list[priority];

#ifdef TX_ENABLE_EVENT_TRACE

                        /* Check that the event time stamp is unchanged.  A different
                           timestamp means that a later event wrote over the thread
                           suspend event. In that case, do nothing here.  */
                        if (entry_ptr != TX_NULL)
                        {

                            /* Is the timestamp the same?  */
                            if (time_stamp == entry_ptr -> tx_trace_buffer_entry_time_stamp)
                            {

                                /* Timestamp is the same, set the "next thread pointer" to the new value of the
                                   next thread to execute. This can be used by the trace analysis tool to keep
                                   track of next thread execution.  */
#ifdef TX_MISRA_ENABLE
                                entry_ptr -> tx_trace_buffer_entry_info_4 =  TX_POINTER_TO_ULONG_CONVERT(_tx_thread_execute_ptr);
#else
                                entry_ptr -> tx_trace_buffer_entry_information_field_4 =  TX_POINTER_TO_ULONG_CONVERT(_tx_thread_execute_ptr);
#endif
                            }
                        }
#endif

                        /* Clear the corresponding bit in the preempted map, since the preemption has been restored.  */
                        TX_MOD32_BIT_SET(priority, priority_bit)
                        _tx_thread_preempted_maps[MAP_INDEX] =  _tx_thread_preempted_maps[MAP_INDEX] & (~(priority_bit));

#if TX_MAX_PRIORITIES > 32

                        /* Determine if there are any other bits set in this preempt map.  */
                        if (_tx_thread_preempted_maps[MAP_INDEX] == ((ULONG) 0))
                        {

                            /* No, clear the active bit to signify this preempt map has nothing set.  */
                            TX_DIV32_BIT_SET(priority, priority_bit)
                            _tx_thread_preempted_map_active =  _tx_thread_preempted_map_active & (~(priority_bit));
                        }
#endif
                    }
                }
#endif

#ifndef TX_MISRA_ENABLE

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

                /* Is the execute pointer different?  */
                if (_tx_thread_performance_execute_log[_tx_thread_performance__execute_log_index] != _tx_thread_execute_ptr)
                {

                    /* Move to next entry.  */
                    _tx_thread_performance__execute_log_index++;

                    /* Check for wrap condition.  */
                    if (_tx_thread_performance__execute_log_index >= TX_THREAD_EXECUTE_LOG_SIZE)
                    {

                        /* Set the index to the beginning.  */
                        _tx_thread_performance__execute_log_index =  ((UINT) 0);
                    }

                    /* Log the new execute pointer.  */
                    _tx_thread_performance_execute_log[_tx_thread_performance__execute_log_index] =  _tx_thread_execute_ptr;
                }
#endif

#ifdef TX_ENABLE_EVENT_TRACE

                /* Check that the event time stamp is unchanged.  A different
                   timestamp means that a later event wrote over the thread
                   suspend event. In that case, do nothing here.  */
                if (entry_ptr != TX_NULL)
                {

                    /* Is the timestamp the same?  */
                    if (time_stamp == entry_ptr -> tx_trace_buffer_entry_time_stamp)
                    {

                        /* Timestamp is the same, set the "next thread pointer" to the new value of the
                           next thread to execute. This can be used by the trace analysis tool to keep
                           track of next thread execution.  */
                        entry_ptr -> tx_trace_buffer_entry_information_field_4 =  0;
                    }
                }
#endif

                /* Restore interrupts.  */
                TX_RESTORE

                /* Determine if preemption should take place. This is only possible if the current thread pointer is
                   not the same as the execute thread pointer AND the system state and preempt disable flags are clear.  */
                TX_THREAD_SYSTEM_RETURN_CHECK(combined_flags)
                if (combined_flags == ((ULONG) 0))
                {

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

                    /* No, there is another thread ready to run and will be scheduled upon return.  */
                    _tx_thread_performance_non_idle_return_count++;
#endif

                    /* Preemption is needed - return to the system!  */
                    _tx_thread_system_return();
                }

                /* Return to caller.  */
                return(TX_SUCCESS);
#endif
            }

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

            /* Is the execute pointer different?  */
            if (_tx_thread_performance_execute_log[_tx_thread_performance__execute_log_index] != _tx_thread_execute_ptr)
            {

                /* Move to next entry.  */
                _tx_thread_performance__execute_log_index++;

                /* Check for wrap condition.  */
                if (_tx_thread_performance__execute_log_index >= TX_THREAD_EXECUTE_LOG_SIZE)
                {

                    /* Set the index to the beginning.  */
                    _tx_thread_performance__execute_log_index =  ((UINT) 0);
                }

                /* Log the new execute pointer.  */
                _tx_thread_performance_execute_log[_tx_thread_performance__execute_log_index] =  _tx_thread_execute_ptr;
            }
#endif

#ifdef TX_ENABLE_EVENT_TRACE

            /* Check that the event time stamp is unchanged.  A different
               timestamp means that a later event wrote over the thread
               suspend event. In that case, do nothing here.  */
            if (entry_ptr != TX_NULL)
            {

                /* Is the timestamp the same?  */
                if (time_stamp == entry_ptr -> tx_trace_buffer_entry_time_stamp)
                {

                    /* Timestamp is the same, set the "next thread pointer" to the new value of the
                       next thread to execute. This can be used by the trace analysis tool to keep
                       track of next thread execution.  */
#ifdef TX_MISRA_ENABLE
                    entry_ptr -> tx_trace_buffer_entry_info_4 =  TX_POINTER_TO_ULONG_CONVERT(_tx_thread_execute_ptr);
#else
                    entry_ptr -> tx_trace_buffer_entry_information_field_4 =  TX_POINTER_TO_ULONG_CONVERT(_tx_thread_execute_ptr);
#endif
                }
            }
#endif

            /* Restore interrupts.  */
            TX_RESTORE

            /* Determine if a preemption condition is present.  */
            if (current_thread != _tx_thread_execute_ptr)
            {

#ifdef TX_ENABLE_STACK_CHECKING

                /* Pickup the next execute pointer.  */
                thread_ptr =  _tx_thread_execute_ptr;

                /* Check this thread's stack.  */
                TX_THREAD_STACK_CHECK(thread_ptr)
#endif

                /* Determine if preemption should take place. This is only possible if the current thread pointer is
                   not the same as the execute thread pointer AND the system state and preempt disable flags are clear.  */
                TX_THREAD_SYSTEM_RETURN_CHECK(combined_flags)
                if (combined_flags == ((ULONG) 0))
                {

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

                    /* Determine if an idle system return is present.  */
                    if (_tx_thread_execute_ptr == TX_NULL)
                    {

                        /* Yes, increment the return to idle return count.  */
                        _tx_thread_performance_idle_return_count++;
                    }
                    else
                    {

                        /* No, there is another thread ready to run and will be scheduled upon return.  */
                        _tx_thread_performance_non_idle_return_count++;
                    }
#endif

                    /* Preemption is needed - return to the system!  */
                    _tx_thread_system_return();
                }
            }

            /* Disable interrupts.  */
            TX_DISABLE

            /* Return success.  */
            status =  TX_SUCCESS;
        }
    }
    else if (thread_ptr -> tx_thread_state == TX_TERMINATED)
    {

        /* Thread is terminated.  */
        status =  TX_SUSPEND_ERROR;
    }
    else if (thread_ptr -> tx_thread_state == TX_COMPLETED)
    {

        /* Thread is completed.  */
        status =  TX_SUSPEND_ERROR;
    }
    else if (thread_ptr -> tx_thread_state == TX_SUSPENDED)
    {

        /* Already suspended, just set status to success.  */
        status =  TX_SUCCESS;
    }
    else
    {

        /* Just set the delayed suspension flag.  */
        thread_ptr -> tx_thread_delayed_suspend =  TX_TRUE;

        /* Set status to success.  */
        status =  TX_SUCCESS;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return completion status.  */
    return(status);
#endif
}

