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
#include "tx_initialize.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_resume                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes application resume thread services. Actual  */
/*    thread resumption is performed in the core service.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Pointer to thread to resume   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Service return status         */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_resume          Resume thread                     */
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
UINT  _tx_thread_resume(TX_THREAD *thread_ptr)
{

TX_INTERRUPT_SAVE_AREA

UINT        status;
TX_THREAD   *saved_thread_ptr;
UINT        saved_threshold =  ((UINT) 0);

#ifdef TX_INLINE_THREAD_RESUME_SUSPEND
UINT            priority;
ULONG           priority_bit;
TX_THREAD       *head_ptr;
TX_THREAD       *tail_ptr;
TX_THREAD       *execute_ptr;
TX_THREAD       *current_thread;
ULONG           combined_flags;

#ifdef TX_ENABLE_EVENT_TRACE
TX_TRACE_BUFFER_ENTRY       *entry_ptr;
ULONG                       time_stamp =  ((ULONG) 0);
#endif

#if TX_MAX_PRIORITIES > 32
UINT            map_index;
#endif


#ifdef TX_ENABLE_STACK_CHECKING

    /* Check this thread's stack.  */
    TX_THREAD_STACK_CHECK(thread_ptr)
#endif
#endif

    /* Lockout interrupts while the thread is being resumed.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_RESUME_API, thread_ptr, thread_ptr -> tx_thread_state, TX_POINTER_TO_ULONG_CONVERT(&status), 0, TX_TRACE_THREAD_EVENTS)

    /* Log this kernel call.  */
    TX_EL_THREAD_RESUME_INSERT

    /* Determine if the thread is suspended or in the process of suspending.
       If so, call the thread resume processing.  */
    if (thread_ptr -> tx_thread_state == TX_SUSPENDED)
    {

        /* Determine if the create call is being called from initialization.  */
        if (TX_THREAD_GET_SYSTEM_STATE() >= TX_INITIALIZE_IN_PROGRESS)
        {

            /* Yes, this resume call was made from initialization.  */

            /* Pickup the current thread execute pointer, which corresponds to the
               highest priority thread ready to execute.  Interrupt lockout is
               not required, since interrupts are assumed to be disabled during
               initialization.  */
            saved_thread_ptr =  _tx_thread_execute_ptr;

            /* Determine if there is thread ready for execution.  */
            if (saved_thread_ptr != TX_NULL)
            {

                /* Yes, a thread is ready for execution when initialization completes.  */

                /* Save the current preemption-threshold.  */
                saved_threshold =  saved_thread_ptr -> tx_thread_preempt_threshold;

                /* For initialization, temporarily set the preemption-threshold to the
                   priority level to make sure the highest-priority thread runs once
                   initialization is complete.  */
                saved_thread_ptr -> tx_thread_preempt_threshold =  saved_thread_ptr -> tx_thread_priority;
            }
        }
        else
        {

            /* Simply set the saved thread pointer to NULL.  */
            saved_thread_ptr =  TX_NULL;
        }

#ifndef TX_INLINE_THREAD_RESUME_SUSPEND

#ifdef TX_NOT_INTERRUPTABLE

        /* Resume the thread!  */
        _tx_thread_system_ni_resume(thread_ptr);

        /* Restore interrupts.  */
        TX_RESTORE
#else

        /* Temporarily disable preemption.  */
        _tx_thread_preempt_disable++;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Call the actual resume service to resume the thread.  */
        _tx_thread_system_resume(thread_ptr);
#endif

        /* Determine if the thread's preemption-threshold needs to be restored.  */
        if (saved_thread_ptr != TX_NULL)
        {

            /* Yes, restore the previous highest-priority thread's preemption-threshold. This
               can only happen if this routine is called from initialization.  */
            saved_thread_ptr -> tx_thread_preempt_threshold =  saved_threshold;
        }

#ifdef TX_MISRA_ENABLE

        /* Disable interrupts.  */
        TX_DISABLE

        /* Setup successful return status.  */
        status =  TX_SUCCESS;
#else

        /* Return successful completion.  */
        return(TX_SUCCESS);
#endif


#else

        /* In-line thread resumption processing follows, which is effectively just taking the
           logic in tx_thread_system_resume.c and placing it here!  */

        /* Resume the thread!  */

#ifdef TX_ENABLE_EVENT_TRACE

        /* If trace is enabled, save the current event pointer.  */
        entry_ptr =  _tx_trace_buffer_current_ptr;
#endif

        /* Log the thread status change.  */
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_RESUME, thread_ptr, thread_ptr -> tx_thread_state, TX_POINTER_TO_ULONG_CONVERT(&execute_ptr), TX_POINTER_TO_ULONG_CONVERT(_tx_thread_execute_ptr), TX_TRACE_INTERNAL_EVENTS)

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

        /* Make this thread ready.  */

        /* Change the state to ready.  */
        thread_ptr -> tx_thread_state =  TX_READY;

        /* Pickup priority of thread.  */
        priority =  thread_ptr -> tx_thread_priority;

        /* Thread state change.  */
        TX_THREAD_STATE_CHANGE(thread_ptr, TX_READY)

        /* Log the thread status change.  */
        TX_EL_THREAD_STATUS_CHANGE_INSERT(thread_ptr, TX_READY)

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

        /* Increment the total number of thread resumptions.  */
        _tx_thread_performance_resume_count++;

        /* Increment this thread's resume count.  */
        thread_ptr -> tx_thread_performance_resume_count++;
#endif

        /* Determine if there are other threads at this priority that are
           ready.  */
        head_ptr =  _tx_thread_priority_list[priority];
        if (head_ptr == TX_NULL)
        {

            /* First thread at this priority ready.  Add to the front of the list.  */
            _tx_thread_priority_list[priority] =       thread_ptr;
            thread_ptr -> tx_thread_ready_next =       thread_ptr;
            thread_ptr -> tx_thread_ready_previous =   thread_ptr;

#if TX_MAX_PRIORITIES > 32

            /* Calculate the index into the bit map array.  */
            map_index =  priority/((UINT) 32);

            /* Set the active bit to remember that the priority map has something set.  */
            TX_DIV32_BIT_SET(priority, priority_bit)
            _tx_thread_priority_map_active =  _tx_thread_priority_map_active | priority_bit;
#endif

            /* Or in the thread's priority bit.  */
            TX_MOD32_BIT_SET(priority, priority_bit)
            _tx_thread_priority_maps[MAP_INDEX] =  _tx_thread_priority_maps[MAP_INDEX] | priority_bit;

            /* Determine if this newly ready thread is the highest priority.  */
            if (priority < _tx_thread_highest_priority)
            {

                /* A new highest priority thread is present. */

                /* Update the highest priority variable.  */
                _tx_thread_highest_priority =  priority;

                /* Pickup the execute pointer. Since it is going to be referenced multiple
                   times, it is placed in a local variable.  */
                execute_ptr =  _tx_thread_execute_ptr;

                /* Determine if no thread is currently executing.  */
                if (execute_ptr == TX_NULL)
                {

                    /* Simply setup the execute pointer.  */
                    _tx_thread_execute_ptr =  thread_ptr;
                }
                else
                {

                    /* Another thread has been scheduled for execution.  */

                    /* Check to see if this is a higher priority thread and determine if preemption is allowed.  */
                    if (priority < execute_ptr -> tx_thread_preempt_threshold)
                    {

#ifndef TX_DISABLE_PREEMPTION_THRESHOLD

                        /* Determine if the preempted thread had preemption-threshold set.  */
                        if (execute_ptr -> tx_thread_preempt_threshold != execute_ptr -> tx_thread_priority)
                        {

#if TX_MAX_PRIORITIES > 32

                            /* Calculate the index into the bit map array.  */
                            map_index =  (execute_ptr -> tx_thread_priority)/((UINT) 32);

                            /* Set the active bit to remember that the preempt map has something set.  */
                            TX_DIV32_BIT_SET(execute_ptr -> tx_thread_priority, priority_bit)
                            _tx_thread_preempted_map_active =  _tx_thread_preempted_map_active | priority_bit;
#endif

                            /* Remember that this thread was preempted by a thread above the thread's threshold.  */
                            TX_MOD32_BIT_SET(execute_ptr -> tx_thread_priority, priority_bit)
                            _tx_thread_preempted_maps[MAP_INDEX] =  _tx_thread_preempted_maps[MAP_INDEX] | priority_bit;
                        }
#endif

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

                        /* Determine if the caller is an interrupt or from a thread.  */
                        if (TX_THREAD_GET_SYSTEM_STATE() == ((ULONG) 0))
                        {

                            /* Caller is a thread, so this is a solicited preemption.  */
                            _tx_thread_performance_solicited_preemption_count++;

                            /* Increment the thread's solicited preemption counter.  */
                            execute_ptr -> tx_thread_performance_solicited_preemption_count++;
                        }
                        else
                        {

                            if (TX_THREAD_GET_SYSTEM_STATE() < TX_INITIALIZE_IN_PROGRESS)
                            {

                                /* Caller is an interrupt, so this is an interrupt preemption.  */
                                _tx_thread_performance_interrupt_preemption_count++;

                                /* Increment the thread's interrupt preemption counter.  */
                                execute_ptr -> tx_thread_performance_interrupt_preemption_count++;
                            }
                        }

                        /* Remember the thread that preempted this thread.  */
                        execute_ptr -> tx_thread_performance_last_preempting_thread =  thread_ptr;
#endif

                        /* Yes, modify the execute thread pointer.  */
                        _tx_thread_execute_ptr =  thread_ptr;

#ifndef TX_MISRA_ENABLE

                        /* If MISRA is not-enabled, insert a preemption and return in-line for performance.  */

                        /* Determine if the thread's preemption-threshold needs to be restored.  */
                        if (saved_thread_ptr != TX_NULL)
                        {

                            /* Yes, restore the previous highest-priority thread's preemption-threshold. This
                               can only happen if this routine is called from initialization.  */
                            saved_thread_ptr -> tx_thread_preempt_threshold =  saved_threshold;
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
                           resume event. In that case, do nothing here.  */
                        if (entry_ptr != TX_NULL)
                        {

                            /* Is the timestamp the same?  */
                            if (time_stamp == entry_ptr -> tx_trace_buffer_entry_time_stamp)
                            {

                                /* Timestamp is the same, set the "next thread pointer" to NULL. This can
                                   be used by the trace analysis tool to show idle system conditions.  */
                                entry_ptr -> tx_trace_buffer_entry_information_field_4 =  TX_POINTER_TO_ULONG_CONVERT(_tx_thread_execute_ptr);
                            }
                        }
#endif

                        /* Restore interrupts.  */
                        TX_RESTORE

#ifdef TX_ENABLE_STACK_CHECKING

                        /* Pickup the next execute pointer.  */
                        thread_ptr =  _tx_thread_execute_ptr;

                        /* Check this thread's stack.  */
                        TX_THREAD_STACK_CHECK(thread_ptr)
#endif

                        /* Now determine if preemption should take place. This is only possible if the current thread pointer is
                           not the same as the execute thread pointer AND the system state and preempt disable flags are clear.  */
                        TX_THREAD_SYSTEM_RETURN_CHECK(combined_flags)
                        if (combined_flags == ((ULONG) 0))
                        {

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

                            /* There is another thread ready to run and will be scheduled upon return.  */
                            _tx_thread_performance_non_idle_return_count++;
#endif

                            /* Preemption is needed - return to the system!  */
                            _tx_thread_system_return();
                        }

                        /* Return in-line when MISRA is not enabled.  */
                        return(TX_SUCCESS);
#endif
                    }
                }
            }
        }
        else
        {

            /* No, there are other threads at this priority already ready.  */

            /* Just add this thread to the priority list.  */
            tail_ptr =                                 head_ptr -> tx_thread_ready_previous;
            tail_ptr -> tx_thread_ready_next =         thread_ptr;
            head_ptr -> tx_thread_ready_previous =     thread_ptr;
            thread_ptr -> tx_thread_ready_previous =   tail_ptr;
            thread_ptr -> tx_thread_ready_next =       head_ptr;
        }

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

        /* Determine if we should log the execute pointer.  */

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
           resume event. In that case, do nothing here.  */
        if (entry_ptr != TX_NULL)
        {

            /* Is the timestamp the same?  */
            if (time_stamp == entry_ptr -> tx_trace_buffer_entry_time_stamp)
            {

                /* Timestamp is the same, set the "next thread pointer" to NULL. This can
                   be used by the trace analysis tool to show idle system conditions.  */
#ifdef TX_MISRA_ENABLE
              entry_ptr -> tx_trace_buffer_entry_info_4 =  TX_POINTER_TO_ULONG_CONVERT(_tx_thread_execute_ptr);
#else
              entry_ptr -> tx_trace_buffer_entry_information_field_4 =  TX_POINTER_TO_ULONG_CONVERT(_tx_thread_execute_ptr);
#endif
            }
        }
#endif

        /* Determine if the thread's preemption-threshold needs to be restored.  */
        if (saved_thread_ptr != TX_NULL)
        {

            /* Yes, restore the previous highest-priority thread's preemption-threshold. This
               can only happen if this routine is called from initialization.  */
            saved_thread_ptr -> tx_thread_preempt_threshold =  saved_threshold;
        }

        /* Setup successful return status.  */
        status =  TX_SUCCESS;
#endif
    }
    else if (thread_ptr -> tx_thread_delayed_suspend == TX_TRUE)
    {

        /* Clear the delayed suspension.  */
        thread_ptr -> tx_thread_delayed_suspend =  TX_FALSE;

        /* Setup delayed suspend lifted return status.  */
        status =  TX_SUSPEND_LIFTED;
    }
    else
    {

        /* Setup invalid resume return status.  */
        status =  TX_RESUME_ERROR;
    }

    /* Restore interrupts.  */
    TX_RESTORE

#ifdef TX_INLINE_THREAD_RESUME_SUSPEND

    /* Pickup thread pointer.  */
    TX_THREAD_GET_CURRENT(current_thread)

    /* Determine if a preemption condition is present.  */
    if (current_thread != _tx_thread_execute_ptr)
    {

#ifdef TX_ENABLE_STACK_CHECKING

        /* Pickup the next execute pointer.  */
        thread_ptr =  _tx_thread_execute_ptr;

        /* Check this thread's stack.  */
        TX_THREAD_STACK_CHECK(thread_ptr)
#endif

        /* Now determine if preemption should take place. This is only possible if the current thread pointer is
           not the same as the execute thread pointer AND the system state and preempt disable flags are clear.  */
        TX_THREAD_SYSTEM_RETURN_CHECK(combined_flags)
        if (combined_flags == ((ULONG) 0))
        {

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

            /* There is another thread ready to run and will be scheduled upon return.  */
            _tx_thread_performance_non_idle_return_count++;
#endif

            /* Preemption is needed - return to the system!  */
            _tx_thread_system_return();
        }
    }
#endif

    /* Return completion status. */
    return(status);
}

