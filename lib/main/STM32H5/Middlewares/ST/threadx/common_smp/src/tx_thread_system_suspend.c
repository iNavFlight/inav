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
#include "tx_initialize.h"
#include "tx_timer.h"
#include "tx_thread.h"
#include "tx_trace.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_system_suspend                          PORTABLE SMP     */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function suspends the specified thread and changes the thread  */
/*    state to the value specified.  Note: delayed suspension processing  */
/*    is handled outside of this routine.                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Pointer to thread to suspend  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_smp_available_cores_get    Get available cores bitmap    */
/*    _tx_thread_smp_core_preempt           Preempt core for new thread   */
/*    _tx_thread_smp_execute_list_clear     Clear the thread execute list */
/*    _tx_thread_smp_execute_list_setup     Setup the thread execute list */
/*    _tx_thread_smp_next_priority_find     Find next priority with one   */
/*                                            or more ready threads       */
/*    _tx_thread_smp_possible_cores_get     Get possible cores bitmap     */
/*    [_tx_thread_smp_protect]              Get protection                */
/*    _tx_thread_smp_rebalance_execute_list Rebalance the execution list  */
/*    _tx_thread_smp_remap_solution_find    Attempt to remap threads to   */
/*                                            schedule another thread     */
/*    _tx_thread_smp_schedule_list_setup    Inherit schedule list from    */
/*                                            execute list                */
/*    _tx_thread_system_return              Return to system              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_thread_priority_change            Thread priority change        */
/*    _tx_thread_shell_entry                Thread shell function         */
/*    _tx_thread_sleep                      Thread sleep                  */
/*    _tx_thread_suspend                    Application thread suspend    */
/*    _tx_thread_terminate                  Thread terminate              */
/*    Other ThreadX Components                                            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      William E. Lamie        Initial Version 6.1           */
/*  04-25-2022      Scott Larson            Modified comments and fixed   */
/*                                            loop to find next thread,   */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
VOID  _tx_thread_system_suspend(TX_THREAD *thread_ptr)
{

#ifndef TX_NOT_INTERRUPTABLE

TX_INTERRUPT_SAVE_AREA
#endif

UINT                        priority;
UINT                        i;
ULONG                       priority_bit;
ULONG                       combined_flags;
ULONG                       priority_map;
UINT                        core_index;
#ifndef TX_THREAD_SMP_EQUAL_PRIORITY
ULONG                       complex_path_possible;
UINT                        core;
ULONG                       possible_cores;
ULONG                       thread_possible_cores;
ULONG                       available_cores;
ULONG                       test_possible_cores;
UINT                        next_priority;
TX_THREAD                   *next_thread;
UINT                        loop_finished;
#endif
#ifndef TX_DISABLE_PREEMPTION_THRESHOLD
UINT                        next_preempted;
UINT                        base_priority;
UINT                        priority_bit_set;
TX_THREAD                   *preempted_thread;
#endif
#if TX_MAX_PRIORITIES > 32
UINT                        map_index;
#endif

#ifndef TX_NO_TIMER
TX_TIMER_INTERNAL           **timer_list;
TX_TIMER_INTERNAL           *next_timer;
TX_TIMER_INTERNAL           *timer_ptr;
TX_TIMER_INTERNAL           *previous_timer;
ULONG                       expiration_time;
ULONG                       delta;
ULONG                       timeout;
#endif

#ifdef TX_ENABLE_EVENT_TRACE
TX_TRACE_BUFFER_ENTRY       *entry_ptr =  TX_NULL;
ULONG                       time_stamp =  ((ULONG) 0);
#endif
UINT                        processing_complete;


    /* Set the processing complete flag to false.  */
    processing_complete =  TX_FALSE;

#ifndef TX_NOT_INTERRUPTABLE

    /* Disable interrupts.  */
    TX_DISABLE
#endif

    /* Pickup the index.  */
    core_index =  TX_SMP_CORE_ID;

#ifdef TX_THREAD_SMP_DEBUG_ENABLE

    /* Debug entry.  */
    _tx_thread_smp_debug_entry_insert(6, 1, thread_ptr);
#endif

#ifndef TX_NO_TIMER

    /* Determine if a timeout needs to be activated.  */
    if (thread_ptr == _tx_thread_current_ptr[core_index])
    {

        /* Reset time slice for current thread.  */
        _tx_timer_time_slice[core_index] =  thread_ptr -> tx_thread_new_time_slice;

        /* Pickup the wait option.  */
        timeout =  thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks;

        /* Determine if an activation is needed.  */
        if (timeout != TX_NO_WAIT)
        {

            /* Make sure the suspension is not a wait-forever.  */
            if (timeout != TX_WAIT_FOREVER)
            {

                /* Activate the thread timer with the timeout value setup in the caller. This is now done in-line
                   for ThreadX SMP so the additional protection logic can be avoided.  */

                /* Activate the thread's timeout timer.  */

                /* Setup pointer to internal timer.  */
                timer_ptr =  &(thread_ptr -> tx_thread_timer);

                /* Calculate the amount of time remaining for the timer.  */
                if (timeout > TX_TIMER_ENTRIES)
                {

                    /* Set expiration time to the maximum number of entries.  */
                    expiration_time =  TX_TIMER_ENTRIES - ((ULONG) 1);
                }
                else
                {

                    /* Timer value fits in the timer entries.  */

                    /* Set the expiration time.  */
                    expiration_time =  (UINT) (timeout - ((ULONG) 1));
                }

                /* At this point, we are ready to put the timer on one of
                   the timer lists.  */

                /* Calculate the proper place for the timer.  */
                timer_list =  TX_TIMER_POINTER_ADD(_tx_timer_current_ptr, expiration_time);
                if (TX_TIMER_INDIRECT_TO_VOID_POINTER_CONVERT(timer_list) >= TX_TIMER_INDIRECT_TO_VOID_POINTER_CONVERT(_tx_timer_list_end))
                {

                    /* Wrap from the beginning of the list.  */
                    delta =  TX_TIMER_POINTER_DIF(timer_list, _tx_timer_list_end);
                    timer_list =  TX_TIMER_POINTER_ADD(_tx_timer_list_start, delta);
                }

                /* Now put the timer on this list.  */
                if ((*timer_list) == TX_NULL)
                {

                    /* This list is NULL, just put the new timer on it.  */

                    /* Setup the links in this timer.  */
                    timer_ptr -> tx_timer_internal_active_next =      timer_ptr;
                    timer_ptr -> tx_timer_internal_active_previous =  timer_ptr;

                    /* Setup the list head pointer.  */
                    *timer_list =  timer_ptr;
                }
                else
                {

                    /* This list is not NULL, add current timer to the end. */
                    next_timer =                                        *timer_list;
                    previous_timer =                                    next_timer -> tx_timer_internal_active_previous;
                    previous_timer -> tx_timer_internal_active_next =   timer_ptr;
                    next_timer -> tx_timer_internal_active_previous =   timer_ptr;
                    timer_ptr -> tx_timer_internal_active_next =        next_timer;
                    timer_ptr -> tx_timer_internal_active_previous =    previous_timer;
                }

                /* Setup list head pointer.  */
                timer_ptr -> tx_timer_internal_list_head =  timer_list;
            }
        }
    }
#endif


#ifdef TX_ENABLE_STACK_CHECKING

    /* Check this thread's stack.  */
    TX_THREAD_STACK_CHECK(thread_ptr)
#endif


#ifndef TX_NOT_INTERRUPTABLE

    /* Decrement the preempt disable flag.  */
    _tx_thread_preempt_disable--;
#endif

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

    /* Increment the thread's suspend count.  */
    thread_ptr -> tx_thread_performance_suspend_count++;

    /* Increment the total number of thread suspensions.  */
    _tx_thread_performance_suspend_count++;
#endif


#ifndef TX_NOT_INTERRUPTABLE

    /* Check to make sure the thread suspending flag is still set.  If not, it
       has already been resumed.  */
    if ((thread_ptr -> tx_thread_suspending) == TX_TRUE)
    {
#endif

        /* Thread state change.  */
        TX_THREAD_STATE_CHANGE(thread_ptr, thread_ptr -> tx_thread_state)

        /* Log the thread status change.  */
        TX_EL_THREAD_STATUS_CHANGE_INSERT(thread_ptr, thread_ptr -> tx_thread_state)

#ifdef TX_ENABLE_EVENT_TRACE

        /* If trace is enabled, save the current event pointer.  */
        entry_ptr =  _tx_trace_buffer_current_ptr;

        /* Log the thread status change.  */
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_SUSPEND, thread_ptr, thread_ptr -> tx_thread_state, TX_POINTER_TO_ULONG_CONVERT(&priority), TX_POINTER_TO_ULONG_CONVERT(_tx_thread_execute_ptr[core_index]), TX_TRACE_INTERNAL_EVENTS)

        /* Save the time stamp for later comparison to verify that
           the event hasn't been overwritten by the time we have
           computed the next thread to execute.  */
        if (entry_ptr != TX_NULL)
        {

            /* Save time stamp.  */
            time_stamp =  entry_ptr -> tx_trace_buffer_entry_time_stamp;
        }
#endif

        /* Actually suspend this thread.  But first, clear the suspending flag.  */
        thread_ptr -> tx_thread_suspending =  TX_FALSE;

        /* Pickup priority of thread.  */
        priority =  thread_ptr -> tx_thread_priority;

#ifndef TX_DISABLE_PREEMPTION_THRESHOLD

#if TX_MAX_PRIORITIES > 32

        /* Calculate the index into the bit map array.  */
        map_index =  priority/((UINT) 32);
#endif

        /* Determine if this thread has preemption-threshold set.  */
        if (thread_ptr -> tx_thread_preempt_threshold < priority)
        {

            /* Was this thread with preemption-threshold set actually preempted with preemption-threshold set?  */
            if (_tx_thread_preemption_threshold_list[priority] == thread_ptr)
            {

                /* Clear the preempted list entry.  */
                _tx_thread_preemption_threshold_list[priority] =  TX_NULL;

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
        }
#endif

        /* Determine if this thread has global preemption disabled.  */
        if (thread_ptr == _tx_thread_preemption__threshold_scheduled)
        {

            /* Clear the global preemption disable flag.  */
            _tx_thread_preemption__threshold_scheduled =  TX_NULL;

#ifndef TX_DISABLE_PREEMPTION_THRESHOLD

            /* Clear the entry in the preempted list.  */
            _tx_thread_preemption_threshold_list[thread_ptr -> tx_thread_priority] =  TX_NULL;

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

                /* Pickup the next preempted thread.  */
                preempted_thread =  _tx_thread_preemption_threshold_list[next_preempted];

                /* Setup the preempted thread.  */
                _tx_thread_preemption__threshold_scheduled =  preempted_thread;
            }
#endif
        }

        /* Determine if there are other threads at this priority that are
           ready.  */
        if (thread_ptr -> tx_thread_ready_next != thread_ptr)
        {

            /* Yes, there are other threads at this priority ready.  */

#ifndef TX_THREAD_SMP_EQUAL_PRIORITY

            /* Remember the head of the priority list.  */
            next_thread =  _tx_thread_priority_list[priority];
#endif

            /* Just remove this thread from the priority list.  */
            (thread_ptr -> tx_thread_ready_next) -> tx_thread_ready_previous =    thread_ptr -> tx_thread_ready_previous;
            (thread_ptr -> tx_thread_ready_previous) -> tx_thread_ready_next =    thread_ptr -> tx_thread_ready_next;

            /* Determine if this is the head of the priority list.  */
            if (_tx_thread_priority_list[priority] == thread_ptr)
            {

                /* Update the head pointer of this priority list.  */
                _tx_thread_priority_list[priority] =  thread_ptr -> tx_thread_ready_next;

#ifndef TX_THREAD_SMP_EQUAL_PRIORITY

                /* Update the next pointer as well.  */
                next_thread =  thread_ptr -> tx_thread_ready_next;
#endif
            }
        }
        else
        {

#ifndef TX_THREAD_SMP_EQUAL_PRIORITY

            /* Remember the head of the priority list.  */
            next_thread =  thread_ptr;
#endif

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
        }

#if TX_MAX_PRIORITIES > 32

        /* Calculate the index to find the next highest priority thread ready for execution.  */
        priority_map =    _tx_thread_priority_map_active;

        /* Determine if there is anything.   */
        if (priority_map != ((ULONG) 0))
        {

            /* Calculate the lowest bit set in the priority map. */
            TX_LOWEST_SET_BIT_CALCULATE(priority_map, map_index)
        }
#endif

        /* Setup working variable for the priority map.  */
        priority_map =    _tx_thread_priority_maps[MAP_INDEX];

        /* Make a quick check for no other threads ready for execution.  */
        if (priority_map == ((ULONG) 0))
        {

#ifdef TX_ENABLE_EVENT_TRACE

            /* Check that the event time stamp is unchanged.  A different
               timestamp means that a later event wrote over the thread
               suspend event. In that case, do nothing here.  */
            if ((entry_ptr != TX_NULL) && (time_stamp == entry_ptr -> tx_trace_buffer_entry_time_stamp))
            {

                /* Timestamp is the same, set the "next thread pointer" to NULL. This can
                   be used by the trace analysis tool to show idle system conditions.  */
#ifdef TX_MISRA_ENABLE
                entry_ptr -> tx_trace_buffer_entry_info_4 =  ((ULONG) 0);
#else
                entry_ptr -> tx_trace_buffer_entry_information_field_4 =  ((ULONG) 0);
#endif
            }
#endif

            /* Check to see if the thread is in the execute list.  */
            i =  thread_ptr -> tx_thread_smp_core_mapped;

            /* Clear the entry in the thread execution list.  */
            _tx_thread_execute_ptr[i] =  TX_NULL;

#ifdef TX_THREAD_SMP_INTER_CORE_INTERRUPT

            /* Determine if we need to preempt the core.  */
            if (i != core_index)
            {

                if (_tx_thread_system_state[i] < TX_INITIALIZE_IN_PROGRESS)
                {

                    /* Preempt the mapped thread.  */
                    _tx_thread_smp_core_preempt(i);
                }
            }
#endif

#ifdef TX_THREAD_SMP_WAKEUP_LOGIC

            /* Does this need to be waked up?  */
            if ((i != core_index) && (_tx_thread_execute_ptr[i] != TX_NULL))
            {

                /* Wakeup based on application's macro.  */
                TX_THREAD_SMP_WAKEUP(i);
            }
#endif

#ifdef TX_THREAD_SMP_DEBUG_ENABLE

            /* Debug entry.  */
            _tx_thread_smp_debug_entry_insert(7, 1, thread_ptr);
#endif

#ifdef TX_ENABLE_EVENT_TRACE

            /* Check that the event time stamp is unchanged.  A different
               timestamp means that a later event wrote over the system suspend
               event.  In that case, do nothing here.  */
            if ((entry_ptr != TX_NULL) && (time_stamp == entry_ptr -> tx_trace_buffer_entry_time_stamp))
            {

                /* Timestamp is the same, set the "next thread pointer" to the next thread scheduled
                   for this core.  */
#ifdef TX_MISRA_ENABLE
                entry_ptr -> tx_trace_buffer_entry_info_4 =  TX_POINTER_TO_ULONG_CONVERT(_tx_thread_execute_ptr[core_index]);
#else
                entry_ptr -> tx_trace_buffer_entry_information_field_4 =  TX_POINTER_TO_ULONG_CONVERT(_tx_thread_execute_ptr[core_index]);
#endif
            }
#endif

            /* Check to see if the caller is a thread and the preempt disable flag is clear.  */
            combined_flags =  ((ULONG) _tx_thread_system_state[core_index]);
            combined_flags =  combined_flags | ((ULONG) _tx_thread_preempt_disable);
            if (combined_flags == ((ULONG) 0))
            {


#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

                /* Yes, increment the return to idle return count.  */
                _tx_thread_performance_idle_return_count++;
#endif


#ifndef TX_NOT_INTERRUPTABLE

                /* Increment the preempt disable flag in order to keep the protection.  */
                _tx_thread_preempt_disable++;

                /* Restore interrupts.  */
                TX_RESTORE
#endif

                /* If so, return control to the system.  */
                _tx_thread_system_return();

#ifdef TX_NOT_INTERRUPTABLE

                /* Setup protection again since caller is expecting that it is still in force.  */
                _tx_thread_smp_protect();
#endif

                /* Processing complete, set the flag.  */
                processing_complete =  TX_TRUE;
            }
        }
        else
        {

            /* There are more threads ready to execute.  */

            /* Check to see if the thread is in the execute list. If not, there is nothing else to do.  */
            i =  thread_ptr -> tx_thread_smp_core_mapped;
            if (_tx_thread_execute_ptr[i] == thread_ptr)
            {

                /* Clear the entry in the thread execution list.  */
                _tx_thread_execute_ptr[i] =  TX_NULL;

                /* Determine if preemption-threshold is present in the suspending thread or present in another executing or previously executing
                   thread.  */
                if ((_tx_thread_preemption__threshold_scheduled != TX_NULL) || (thread_ptr -> tx_thread_preempt_threshold < thread_ptr -> tx_thread_priority))
                {

                    /* Call the rebalance routine. This routine maps cores and ready threads.  */
                    _tx_thread_smp_rebalance_execute_list(core_index);
                }
#ifdef TX_THREAD_SMP_EQUAL_PRIORITY
                else
                {

                    /* For equal priority SMP, we simply use the rebalance list function.  */

                    /* Call the rebalance routine. This routine maps cores and ready threads.  */
                    _tx_thread_smp_rebalance_execute_list(core_index);
                }
#else
                else
                {

                    /* Now we need to find the next, highest-priority thread ready for execution.  */

                    /* Start at the priority of the thread suspending, since we know that higher priority threads
                       have already been evaluated when preemption-threshold is not in effect.  */
                    next_priority =  thread_ptr -> tx_thread_priority;

                    /* Determine if there are other threads at the same priority level as the suspending thread.  */
                    if (next_thread == thread_ptr)
                    {

                        /* No more threads at this priority level.  */

                        /* Start at the priority after that of the thread suspending, since we know there are no
                           other threads at the suspending thread's priority ready to execute.  */
                        next_priority++;

                        /* Set next thread to NULL..  */
                        next_thread =  TX_NULL;
                    }

                    /* Get the possible cores bit map, based on what has already been scheduled.  */
                    possible_cores =  _tx_thread_smp_possible_cores_get();

                    /* Setup the available cores bit map. In the suspend case, this is simply the core that is now available. */
                    available_cores =  (((ULONG) 1) << i);

                    /* Calculate the possible complex path.  */
                    complex_path_possible =  possible_cores & available_cores;

                    /* Check if we need to loop to find the next highest priority thread.  */
                    if (next_priority == TX_MAX_PRIORITIES)
                    {
                        loop_finished = TX_TRUE;
                    }
                    else
                    {
                        loop_finished = TX_FALSE;
                    }

                    /* Loop to find the next highest priority ready thread that is allowed to run on this core.  */
                    while (loop_finished == TX_FALSE)
                    {

                        /* Determine if there is a thread to examine.  */
                        if (next_thread == TX_NULL)
                        {

                            /* Calculate the next ready priority.  */
                            next_priority =  _tx_thread_smp_next_priority_find(next_priority);

                            /* Determine if there are no more threads to execute.  */
                            if (next_priority == ((UINT) TX_MAX_PRIORITIES))
                            {

                                /* Break out of loop.  */
                                loop_finished =  TX_TRUE;
                            }
                            else
                            {

                                /* Pickup the next thread to schedule.  */
                                next_thread =  _tx_thread_priority_list[next_priority];
                            }
                        }

                        /* Determine if the processing is not complete.  */
                        if (loop_finished == TX_FALSE)
                        {

                            /* Is the this thread already in the execute list?  */
                            if (next_thread != _tx_thread_execute_ptr[next_thread -> tx_thread_smp_core_mapped])
                            {

                                /* No, not already on the execute list.   */

                                /* Check to see if the thread has preemption-threshold set.  */
                                if (next_thread -> tx_thread_preempt_threshold != next_thread -> tx_thread_priority)
                                {

                                    /* Call the rebalance routine. This routine maps cores and ready threads.  */
                                    _tx_thread_smp_rebalance_execute_list(core_index);

                                    /* Get out of the loop.  */
                                    loop_finished =  TX_TRUE;
                                }
                                else
                                {

                                    /* Now determine if this thread is allowed to run on this core.  */
                                    if ((((next_thread -> tx_thread_smp_cores_allowed >> i) & ((ULONG) 1))) != ((ULONG) 0))
                                    {

                                        /* Remember this index in the thread control block.  */
                                        next_thread -> tx_thread_smp_core_mapped =  i;

                                        /* Setup the entry in the execution list.  */
                                        _tx_thread_execute_ptr[i] =  next_thread;

                                        /* Found the thread to execute.  */
                                        loop_finished =  TX_TRUE;
                                    }
                                    else
                                    {

                                        /* Determine if nontrivial scheduling is possible.  */
                                        if (complex_path_possible != ((ULONG) 0))
                                        {

                                            /* Check for nontrivial scheduling, i.e., can other threads be remapped to allow this thread to be
                                               scheduled.  */

                                            /* Determine what the possible cores are for this thread.  */
                                            thread_possible_cores =  next_thread -> tx_thread_smp_cores_allowed;

                                            /* Apply the current possible cores.  */
                                            thread_possible_cores =  thread_possible_cores & possible_cores;
                                            if (thread_possible_cores != ((ULONG) 0))
                                            {

                                                /* Note that we know that the thread must have the target core excluded at this point,
                                                   since we failed the test above.  */

                                                /* Now we need to see if one of the other threads in the non-excluded cores can be moved to make room
                                                   for this thread.  */

                                                /* Default the schedule list to the current execution list.  */
                                                _tx_thread_smp_schedule_list_setup();

                                                /* Determine the possible core mapping.  */
                                                test_possible_cores =  possible_cores & ~(thread_possible_cores);

                                                /* Attempt to remap the cores in order to schedule this thread.  */
                                                core =  _tx_thread_smp_remap_solution_find(next_thread, available_cores, thread_possible_cores, test_possible_cores);

                                                /* Determine if remapping was successful.  */
                                                if (core != ((UINT) TX_THREAD_SMP_MAX_CORES))
                                                {

                                                    /* Clear the execute list.  */
                                                    _tx_thread_smp_execute_list_clear();

                                                    /* Setup the execute list based on the updated schedule list.  */
                                                    _tx_thread_smp_execute_list_setup(core_index);

                                                    /* At this point, we are done since we have found a solution for one core.  */
                                                    loop_finished =  TX_TRUE;
                                                }
                                                else
                                                {

                                                    /* We couldn't assign the thread to any of the cores possible for the thread so update the possible cores for the
                                                       next pass so we don't waste time looking at them again!  */
                                                    possible_cores =  possible_cores & (~thread_possible_cores);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        /* Determine if the loop is finished.  */
                        if (loop_finished == TX_FALSE)
                        {

                            /* Move to the next thread.  */
                            next_thread =  next_thread -> tx_thread_ready_next;

                            /* Determine if we are at the head of the list.  */
                            if (next_thread == _tx_thread_priority_list[next_priority])
                            {

                                /* Yes, set the next thread pointer to NULL, increment the priority, and continue.  */
                                next_thread =  TX_NULL;
                                next_priority++;

                                /* Determine if there are no more threads to execute.  */
                                if (next_priority == ((UINT) TX_MAX_PRIORITIES))
                                {

                                    /* Break out of loop.  */
                                    loop_finished =  TX_TRUE;
                                }
                            }
                        }
                    }

#ifdef TX_THREAD_SMP_INTER_CORE_INTERRUPT

                    /* Determine if we need to preempt the core.  */
                    if (i != core_index)
                    {

                        /* Make sure thread execution has started.  */
                        if (_tx_thread_system_state[i] < ((ULONG) TX_INITIALIZE_IN_PROGRESS))
                        {

                            /* Preempt the mapped thread.  */
                            _tx_thread_smp_core_preempt(i);
                        }
                    }
#endif

#ifdef TX_THREAD_SMP_WAKEUP_LOGIC

                    /* Does this need to be waked up?  */
                    if (i != core_index)
                    {

                        /* Check to make sure there a thread to execute for this core.  */
                        if (_tx_thread_execute_ptr[i] != TX_NULL)
                        {

                            /* Wakeup based on application's macro.  */
                            TX_THREAD_SMP_WAKEUP(i);
                        }
                    }
#endif
                }
#endif
            }
        }

#ifndef TX_NOT_INTERRUPTABLE

    }
#endif

    /* Check to see if the processing is complete.  */
    if (processing_complete == TX_FALSE)
    {

#ifdef TX_THREAD_SMP_DEBUG_ENABLE

        /* Debug entry.  */
        _tx_thread_smp_debug_entry_insert(7, 1, thread_ptr);
#endif

#ifdef TX_ENABLE_EVENT_TRACE

        /* Check that the event time stamp is unchanged.  A different
           timestamp means that a later event wrote over the thread
           suspend event. In that case, do nothing here.  */
        if ((entry_ptr != TX_NULL) && (time_stamp == entry_ptr -> tx_trace_buffer_entry_time_stamp))
        {

            /* Timestamp is the same, set the "next thread pointer" to the next thread scheduled
               for this core.  */
#ifdef TX_MISRA_ENABLE
            entry_ptr -> tx_trace_buffer_entry_info_4 =  TX_POINTER_TO_ULONG_CONVERT(_tx_thread_execute_ptr[core_index]);
#else
            entry_ptr -> tx_trace_buffer_entry_information_field_4 =  TX_POINTER_TO_ULONG_CONVERT(_tx_thread_execute_ptr[core_index]);
#endif
        }
#endif

        /* Determine if a preemption condition is present.  */
        if (_tx_thread_current_ptr[core_index] != _tx_thread_execute_ptr[core_index])
        {

#ifdef TX_ENABLE_STACK_CHECKING

            /* Pickup the next execute pointer.  */
            thread_ptr =  _tx_thread_execute_ptr[core_index];

            /* Determine if there is a thread pointer.  */
            if (thread_ptr != TX_NULL)
            {

                /* Check this thread's stack.  */
                TX_THREAD_STACK_CHECK(thread_ptr)
            }
#endif

            /* Determine if preemption should take place. This is only possible if the current thread pointer is
               not the same as the execute thread pointer AND the system state and preempt disable flags are clear.  */
            if (_tx_thread_system_state[core_index] == ((ULONG) 0))
            {

                /* Check the preempt disable flag.  */
                if (_tx_thread_preempt_disable == ((UINT) 0))
                {

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

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


#ifndef TX_NOT_INTERRUPTABLE

                    /* Increment the preempt disable flag in order to keep the protection.  */
                    _tx_thread_preempt_disable++;

                    /* Restore interrupts.  */
                    TX_RESTORE
#endif

                    /* Preemption is needed - return to the system!  */
                    _tx_thread_system_return();

#ifdef TX_NOT_INTERRUPTABLE

                    /* Setup protection again since caller is expecting that it is still in force.  */
                    _tx_thread_smp_protect();
#endif

#ifndef TX_NOT_INTERRUPTABLE

                    /* Set the processing complete flag.  */
                    processing_complete =  TX_TRUE;
#endif
                }
            }
        }

#ifndef TX_NOT_INTERRUPTABLE

        /* Determine if processing is complete.  If so, no need to restore interrupts.  */
        if (processing_complete == TX_FALSE)
        {

            /* Restore interrupts.  */
            TX_RESTORE
        }
#endif
    }
}

#ifdef TX_NOT_INTERRUPTABLE
VOID  _tx_thread_system_ni_suspend(TX_THREAD *thread_ptr, ULONG timeout)
{

    /* Setup timeout.   */
    thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  timeout;

    /* Call system suspend function.  */
    _tx_thread_system_suspend(thread_ptr);
}
#endif

