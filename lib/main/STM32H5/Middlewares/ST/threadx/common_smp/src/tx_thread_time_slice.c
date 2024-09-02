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
#define TX_THREAD_SMP_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_timer.h"
#include "tx_thread.h"
#include "tx_trace.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_time_slice                              PORTABLE SMP     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function moves the currently executing thread to the end of    */
/*    the threads ready at the same priority level as a result of a       */
/*    time-slice interrupt.  If no other thread of the same priority is   */
/*    ready, this function simply returns.                                */
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
/*    _tx_thread_smp_rebalance_execute_list Rebalance the execution list  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_timer_interrupt                   Timer interrupt handling      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
VOID  _tx_thread_time_slice(VOID)
{

ULONG           core_index, current_core;
UINT            priority;
TX_THREAD       *thread_ptr;
TX_THREAD       *next_thread;
TX_THREAD       *previous_thread;
TX_THREAD       *head_ptr;
TX_THREAD       *tail_ptr;
UINT            loop_finished;
UINT            rebalance;
ULONG           excluded;
#ifdef TX_ENABLE_EVENT_TRACE
ULONG           system_state;
UINT            preempt_disable;
#endif


    /* Quick check for expiration.  */
#if TX_THREAD_SMP_MAX_CORES == 1
    if (_tx_timer_time_slice[0] != 0)
    {
#endif
#if TX_THREAD_SMP_MAX_CORES == 2
    if ((_tx_timer_time_slice[0] != ((ULONG) 0)) || (_tx_timer_time_slice[1] != ((ULONG) 0)))
    {
#endif
#if TX_THREAD_SMP_MAX_CORES == 3
    if ((_tx_timer_time_slice[0] != ((ULONG) 0)) || (_tx_timer_time_slice[1] != ((ULONG) 0)) || (_tx_timer_time_slice[2] != ((ULONG) 0)))
    {
#endif
#if TX_THREAD_SMP_MAX_CORES == 4
    if ((_tx_timer_time_slice[0] != ((ULONG) 0)) || (_tx_timer_time_slice[1] != ((ULONG) 0)) || (_tx_timer_time_slice[2] != ((ULONG) 0)) || (_tx_timer_time_slice[3] != ((ULONG) 0)))
    {
#endif

        /* Initialize the rebalance flag to false.  */
        rebalance =  TX_FALSE;

        /* Get the core index.  */
        current_core =  TX_SMP_CORE_ID;

        /* Loop to process all time-slices.  */

#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX

        for (core_index = ((ULONG) 0); core_index < ((ULONG) TX_THREAD_SMP_MAX_CORES); core_index++)
#else

        for (core_index = ((ULONG) 0); core_index < _tx_thread_smp_max_cores; core_index++)
#endif
        {

            /* Determine if there is a time-slice active on this core.  */
            if (_tx_timer_time_slice[core_index] != ((ULONG) 0))
            {

                /* Time-slice is active, decrement it for this core.  */
                _tx_timer_time_slice[core_index]--;

                /* Has the time-slice expired?  */
                if (_tx_timer_time_slice[core_index] == ((ULONG) 0))
                {

                    /* Yes, time-slice on this core has expired.  */

                    /* Pickup the current thread pointer.  */
                    thread_ptr =  _tx_thread_current_ptr[core_index];

                    /* Make sure the thread is still active, i.e. not suspended.  */
                    if ((thread_ptr != TX_NULL) && (thread_ptr -> tx_thread_state == TX_READY) && (thread_ptr -> tx_thread_time_slice != ((ULONG) 0)))
                    {

                        /* Yes, thread is still active and time-slice has expired.  */

                        /* Setup a fresh time-slice for the thread.  */
                        thread_ptr -> tx_thread_time_slice =  thread_ptr -> tx_thread_new_time_slice;

                        /* Reset the actual time-slice variable.  */
                        _tx_timer_time_slice[core_index] =  thread_ptr -> tx_thread_time_slice;

#ifdef TX_THREAD_SMP_DEBUG_ENABLE

                        /* Debug entry.  */
                        _tx_thread_smp_debug_entry_insert(10, 0, thread_ptr);
#endif

#ifdef TX_ENABLE_STACK_CHECKING

                        /* Check this thread's stack.  */
                        TX_THREAD_STACK_CHECK(thread_ptr)
#endif

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

                        /* Increment the thread's time-slice counter.  */
                        thread_ptr -> tx_thread_performance_time_slice_count++;

                        /* Increment the total number of thread time-slice operations.  */
                        _tx_thread_performance_time_slice_count++;
#endif

                        /* Setup the priority.  */
                        priority =  thread_ptr -> tx_thread_priority;

                        /* Determine if preemption-threshold is set. If so, don't time-slice.  */
                        if (priority == thread_ptr -> tx_thread_preempt_threshold)
                        {

                            /* Preemption-threshold is not set.  */

                            /* Pickup the next thread.  */
                            next_thread =  thread_ptr -> tx_thread_ready_next;

                            /* Pickup the head of the list.  */
                            head_ptr =  _tx_thread_priority_list[priority];

                            /* Pickup the list tail.  */
                            tail_ptr =  head_ptr -> tx_thread_ready_previous;

                            /* Determine if this thread is not the tail pointer.  */
                            if (thread_ptr != tail_ptr)
                            {

                                /* Not the tail pointer, this thread must be moved to the end of the ready list.  */

                                /* Determine if this thread is at the head of the list.  */
                                if (head_ptr == thread_ptr)
                                {

                                    /* Simply move the head pointer to put this thread at the end of the ready list at this priority.  */
                                    _tx_thread_priority_list[priority] =  next_thread;
                                }
                                else
                                {

                                    /* Now we need to remove this thread from its current position and place it at the end of the list.   */

                                    /* Pickup the previous thread pointer.  */
                                    previous_thread =  thread_ptr -> tx_thread_ready_previous;

                                    /* Remove the thread from the ready list.  */
                                    next_thread -> tx_thread_ready_previous =    previous_thread;
                                    previous_thread -> tx_thread_ready_next =    next_thread;

                                    /* Insert the thread at the end of the list.  */
                                    tail_ptr -> tx_thread_ready_next =         thread_ptr;
                                    head_ptr -> tx_thread_ready_previous =     thread_ptr;
                                    thread_ptr -> tx_thread_ready_previous =   tail_ptr;
                                    thread_ptr -> tx_thread_ready_next =       head_ptr;
                                }

                                /* Make sure the current core execute pointer is still this thread.  If not, a higher priority thread has already
                                   preempted it either from another ISR or from the timer processing.  */
                                if (_tx_thread_execute_ptr[core_index] != thread_ptr)
                                {

                                    /* Set the rebalance flag.  */
                                    rebalance =  TX_TRUE;
                                }

                                /* Determine if the rebalance flag has been set already.  If so, don't bother trying to update the
                                   execute list from this routine.  */
                                if (rebalance == TX_FALSE)
                                {

                                    /* Set the loop finished flag to false.  */
                                    loop_finished =  TX_FALSE;

                                    /* Determine if there is a thread at the same priority that isn't currently executing.  */
                                    do
                                    {

                                        /* Isolate the exclusion for this core.  */
                                        excluded =  (next_thread -> tx_thread_smp_cores_excluded >> core_index) & ((ULONG) 1);

                                        /* Determine if the next thread has preemption-threshold set.  */
                                        if (next_thread -> tx_thread_preempt_threshold < next_thread -> tx_thread_priority)
                                        {

                                            /* Set the rebalance flag.  */
                                            rebalance =  TX_TRUE;

                                            /* Get out of the loop.  */
                                            loop_finished =  TX_TRUE;
                                        }

                                        /* Determine if the next thread is excluded from running on this core.  */
                                        else if (excluded == ((ULONG) 1))
                                        {

                                            /* Set the rebalance flag.  */
                                            rebalance =  TX_TRUE;

                                            /* Get out of the loop.  */
                                            loop_finished =  TX_TRUE;
                                        }
                                        else
                                        {

                                            /* Is the next thread not scheduled  */
                                            if (next_thread != _tx_thread_execute_ptr[next_thread -> tx_thread_smp_core_mapped])
                                            {

                                                /* Remember this index in the thread control block.  */
                                                next_thread -> tx_thread_smp_core_mapped =  core_index;

                                                /* Setup the entry in the execution list.  */
                                                _tx_thread_execute_ptr[core_index] =  next_thread;

#ifdef TX_THREAD_SMP_INTER_CORE_INTERRUPT

                                                /* Determine if we need to preempt the core.  */
                                                if (core_index != current_core)
                                                {

                                                    /* Preempt the mapped thread.  */
                                                    _tx_thread_smp_core_preempt(core_index);
                                                }
#endif
                                                /* Get out of the loop.  */
                                                loop_finished =  TX_TRUE;
                                            }
                                        }

                                        /* Is the loop fininshed?  */
                                        if (loop_finished == TX_TRUE)
                                        {

                                            /* Yes, break out of the loop.  */
                                            break;
                                        }

                                        /* Move to the next thread at this priority.  */
                                        next_thread =  next_thread -> tx_thread_ready_next;

                                    } while (next_thread != thread_ptr);
                                }
                            }

#ifdef TX_THREAD_SMP_DEBUG_ENABLE

                            /* Debug entry.  */
                            _tx_thread_smp_debug_entry_insert(11, 0, thread_ptr);
#endif
                        }
                    }
                }
            }
        }

        /* Determine if rebalance was set.  */
        if (rebalance == TX_TRUE)
        {

            /* Call the rebalance routine. This routine maps cores and ready threads.  */
            _tx_thread_smp_rebalance_execute_list(current_core);
        }

#ifdef TX_ENABLE_EVENT_TRACE

        /* Pickup the volatile information.  */
        system_state =  TX_THREAD_GET_SYSTEM_STATE();
        preempt_disable =  _tx_thread_preempt_disable;

        /* Insert this event into the trace buffer.  */
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_TIME_SLICE, _tx_thread_execute_ptr[0], system_state, preempt_disable, TX_POINTER_TO_ULONG_CONVERT(&thread_ptr), TX_TRACE_INTERNAL_EVENTS)
#endif

#if TX_THREAD_SMP_MAX_CORES == 1
    }
#endif
#if TX_THREAD_SMP_MAX_CORES == 2
    }
#endif
#if TX_THREAD_SMP_MAX_CORES == 3
    }
#endif
#if TX_THREAD_SMP_MAX_CORES == 4
    }
#endif
}

