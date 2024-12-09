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
#include "tx_thread.h"
#include "tx_timer.h"
#include "tx_trace.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_relinquish                              PORTABLE SMP     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function determines if there is another higher or equal        */
/*    priority, non-executing thread that can execute on this processor.  */
/*    such a thread is found, the calling thread relinquishes control.    */
/*    Otherwise, this function simply returns.                            */
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
/*    _tx_thread_system_return              Return to the system          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
VOID  _tx_thread_relinquish(VOID)
{

TX_INTERRUPT_SAVE_AREA

UINT            priority;
TX_THREAD       *thread_ptr;
TX_THREAD       *head_ptr;
TX_THREAD       *tail_ptr;
TX_THREAD       *next_thread;
TX_THREAD       *previous_thread;
UINT            core_index;
UINT            rebalance;
UINT            mapped_core;
ULONG           excluded;

#ifndef TX_DISABLE_PREEMPTION_THRESHOLD
UINT            base_priority;
UINT            priority_bit_set;
UINT            next_preempted;
ULONG           priority_bit;
ULONG           priority_map;
TX_THREAD       *preempted_thread;
#if TX_MAX_PRIORITIES > 32
UINT            map_index;
#endif
#endif
UINT            finished;


    /* Default finished to false.  */
    finished =  TX_FALSE;

    /* Initialize the rebalance flag to false.  */
    rebalance =  TX_FALSE;

    /* Lockout interrupts while thread attempts to relinquish control.  */
    TX_DISABLE

    /* Pickup the index.  */
    core_index =  TX_SMP_CORE_ID;

    /* Pickup the current thread pointer.  */
    thread_ptr =  _tx_thread_current_ptr[core_index];

#ifndef TX_NO_TIMER

    /* Reset time slice for current thread.  */
    _tx_timer_time_slice[core_index] =  thread_ptr -> tx_thread_new_time_slice;
#endif

#ifdef TX_ENABLE_STACK_CHECKING

    /* Check this thread's stack.  */
    TX_THREAD_STACK_CHECK(thread_ptr)
#endif

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_RELINQUISH, &thread_ptr, TX_POINTER_TO_ULONG_CONVERT(thread_ptr -> tx_thread_ready_next), 0, 0, TX_TRACE_THREAD_EVENTS)

    /* Log this kernel call.  */
    TX_EL_THREAD_RELINQUISH_INSERT

    /* Pickup the thread's priority.  */
    priority =  thread_ptr -> tx_thread_priority;

#ifdef TX_THREAD_SMP_DEBUG_ENABLE

    /* Debug entry.  */
    _tx_thread_smp_debug_entry_insert(0, 0, thread_ptr);
#endif

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

        /* Pickup the mapped core of the relinquishing thread - this can be different from the current core.  */
        mapped_core =  thread_ptr -> tx_thread_smp_core_mapped;

        /* Determine if the relinquishing thread is no longer present in the execute list.  */
        if (thread_ptr != _tx_thread_execute_ptr[mapped_core])
        {

            /* Yes, the thread is no longer mapped.  Set the rebalance flag to determine if there is a new mapping due to moving
               this thread to the end of the priority list.  */

            /* Set the rebalance flag to true.  */
            rebalance =  TX_TRUE;
        }

        /* Determine if preemption-threshold is in force. */
        else if (thread_ptr -> tx_thread_preempt_threshold == priority)
        {

            /* No preemption-threshold is in force.  */

            /* Determine if there is a thread at the same priority that isn't currently executing.  */
            do
            {

                /* Isolate the exclusion for this core.  */
                excluded =  (next_thread -> tx_thread_smp_cores_excluded >> mapped_core) & ((ULONG) 1);

                /* Determine if the next thread has preemption-threshold set or is excluded from running on the
                   mapped core.  */
                if ((next_thread -> tx_thread_preempt_threshold  < next_thread -> tx_thread_priority) ||
                    (excluded == ((ULONG) 1)))
                {

                    /* Set the rebalance flag.  */
                    rebalance =  TX_TRUE;

                    /* Get out of the loop. We need to rebalance the list when we detect preemption-threshold.  */
                    break;
                }
                else
                {

                    /* Is the next thread already in the execute list?  */
                    if (next_thread != _tx_thread_execute_ptr[next_thread -> tx_thread_smp_core_mapped])
                    {

                        /* No, we can place this thread in the position the relinquishing thread
                           was in.  */

                        /* Remember this index in the thread control block.  */
                        next_thread -> tx_thread_smp_core_mapped =  mapped_core;

                        /* Setup the entry in the execution list.  */
                        _tx_thread_execute_ptr[mapped_core] =  next_thread;

#ifdef TX_THREAD_SMP_DEBUG_ENABLE

                        /* Debug entry.  */
                        _tx_thread_smp_debug_entry_insert(1, 0, next_thread);
#endif

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

                        /* Increment the number of thread relinquishes.  */
                        thread_ptr -> tx_thread_performance_relinquish_count++;

                        /* Increment the total number of thread relinquish operations.  */
                        _tx_thread_performance_relinquish_count++;

                        /* No, there is another thread ready to run and will be scheduled upon return.  */
                        _tx_thread_performance_non_idle_return_count++;
#endif

#ifdef TX_ENABLE_STACK_CHECKING

                        /* Check this thread's stack.  */
                        TX_THREAD_STACK_CHECK(next_thread)
#endif

#ifndef TX_NOT_INTERRUPTABLE

                        /* Increment the preempt disable flag in order to keep the protection.  */
                        _tx_thread_preempt_disable++;

                        /* Restore interrupts.  */
                        TX_RESTORE
#endif

                        /* Transfer control to the system so the scheduler can execute
                          the next thread.  */
                        _tx_thread_system_return();


#ifdef TX_NOT_INTERRUPTABLE

                        /* Restore interrupts.  */
                        TX_RESTORE
#endif

                        /* Set the finished flag.  */
                        finished =  TX_TRUE;

                    }

                    /* Move to the next thread at this priority.  */
                    next_thread =  next_thread -> tx_thread_ready_next;

                }
            }  while ((next_thread != thread_ptr) && (finished == TX_FALSE));

            /* Determine if we are finished.  */
            if (finished == TX_FALSE)
            {

                /* No other thread is ready at this priority... simply return.  */

#ifdef TX_THREAD_SMP_DEBUG_ENABLE

                /* Debug entry.  */
                _tx_thread_smp_debug_entry_insert(1, 0, thread_ptr);
#endif

                /* Restore interrupts.  */
                TX_RESTORE

                /* Set the finished flag.  */
                finished =  TX_TRUE;
            }
        }
        else
        {

            /* Preemption-threshold is in force.  */

            /* Set the rebalance flag.  */
            rebalance =  TX_TRUE;
        }
    }

    /* Determine if preemption-threshold is in force. */
    if (thread_ptr -> tx_thread_preempt_threshold < priority)
    {

        /* Set the rebalance flag.  */
        rebalance =  TX_TRUE;

#ifndef TX_DISABLE_PREEMPTION_THRESHOLD

#if TX_MAX_PRIORITIES > 32

        /* Calculate the index into the bit map array.  */
        map_index =  priority/((UINT) 32);
#endif

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

        /* Clear the entry in the preempted list.  */
        _tx_thread_preemption_threshold_list[priority] =  TX_NULL;

        /* Does this thread have preemption-threshold?  */
        if (_tx_thread_preemption__threshold_scheduled == thread_ptr)
        {

            /* Yes, set the preempted thread to NULL.  */
            _tx_thread_preemption__threshold_scheduled =  TX_NULL;
        }

        /* Calculate the first thread with preemption-threshold active.  */
#if TX_MAX_PRIORITIES > 32
        if (_tx_thread_preempted_map_active != ((ULONG) 0))
#else
        if (_tx_thread_preempted_maps[0] != ((ULONG) 0))
#endif
        {
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
            TX_LOWEST_SET_BIT_CALCULATE(priority_map, priority_bit_set)

            /* Move priority bit set into priority bit.  */
            priority_bit =  (ULONG) priority_bit_set;

            /* Setup the highest priority preempted thread.  */
            next_preempted =  base_priority + priority_bit;

            /* Pickup the previously preempted thread.  */
            preempted_thread =  _tx_thread_preemption_threshold_list[next_preempted];

            /* Setup the preempted thread.  */
            _tx_thread_preemption__threshold_scheduled =  preempted_thread;
        }
#else

        /* Determine if this thread has preemption-threshold disabled.  */
        if (thread_ptr == _tx_thread_preemption__threshold_scheduled)
        {

            /* Clear the global preemption disable flag.  */
            _tx_thread_preemption__threshold_scheduled =  TX_NULL;
        }
#endif
    }

    /* Check to see if there is still work to do.  */
    if (finished == TX_FALSE)
    {

#ifdef TX_THREAD_SMP_DEBUG_ENABLE

        /* Debug entry.  */
        _tx_thread_smp_debug_entry_insert(1, 0, thread_ptr);
#endif

        /* Determine if we need to rebalance the execute list.  */
        if (rebalance == TX_TRUE)
        {

            /* Rebalance the excute list.  */
            _tx_thread_smp_rebalance_execute_list(core_index);
        }

        /* Determine if this thread needs to return to the system.  */
        if (_tx_thread_execute_ptr[core_index] != thread_ptr)
        {

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

            /* Increment the number of thread relinquishes.  */
            thread_ptr -> tx_thread_performance_relinquish_count++;

            /* Increment the total number of thread relinquish operations.  */
            _tx_thread_performance_relinquish_count++;

            /* Determine if an idle system return is present.  */
            if (_tx_thread_execute_ptr[core_index] == TX_NULL)
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

#ifdef TX_ENABLE_STACK_CHECKING

            /* Pickup new thread pointer.  */
            thread_ptr =  _tx_thread_execute_ptr[core_index];

            /* Check this thread's stack.  */
            TX_THREAD_STACK_CHECK(thread_ptr)
#endif

#ifndef TX_NOT_INTERRUPTABLE

            /* Increment the preempt disable flag in order to keep the protection.  */
            _tx_thread_preempt_disable++;

            /* Restore interrupts.  */
            TX_RESTORE
#endif

            /* Transfer control to the system so the scheduler can execute
               the next thread.  */
            _tx_thread_system_return();

#ifdef TX_NOT_INTERRUPTABLE

            /* Restore interrupts.  */
            TX_RESTORE
#endif
       }
       else
       {

           /* Restore interrupts.  */
           TX_RESTORE
        }
    }
}

