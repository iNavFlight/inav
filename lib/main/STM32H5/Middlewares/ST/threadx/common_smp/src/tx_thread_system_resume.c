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
/*    _tx_thread_system_resume                           PORTABLE SMP     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function places the specified thread on the list of ready      */
/*    threads at the thread's specific priority.  If a thread preemption  */
/*    is detected, this function returns a TX_TRUE.                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Pointer to thread to resume   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_smp_available_cores_get    Get available cores bitmap    */
/*    _tx_thread_smp_core_preempt           Preempt core for new thread   */
/*    _tx_thread_smp_core_wakeup            Wakeup other core             */
/*    _tx_thread_smp_execute_list_clear     Clear the thread execute list */
/*    _tx_thread_smp_execute_list_setup     Setup the thread execute list */
/*    _tx_thread_smp_core_interrupt   Interrupt other core          */
/*    _tx_thread_smp_lowest_priority_get    Get lowest priority scheduled */
/*                                            thread                      */
/*    _tx_thread_smp_next_priority_find     Find next priority with one   */
/*                                            or more ready threads       */
/*    _tx_thread_smp_possible_cores_get     Get possible cores bitmap     */
/*    _tx_thread_smp_preemptable_threads_get                              */
/*                                          Get list of thread preemption */
/*                                            possibilities               */
/*    [_tx_thread_smp_protect]              Get protection                */
/*    _tx_thread_smp_rebalance_execute_list Rebalance the execution list  */
/*    _tx_thread_smp_remap_solution_find    Attempt to remap threads to   */
/*                                            schedule another thread     */
/*    _tx_thread_smp_schedule_list_clear    Clear the thread schedule list*/
/*    _tx_thread_smp_schedule_list_setup    Inherit schedule list from    */
/*                                            execute list                */
/*    _tx_thread_system_return              Return to the system          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_thread_create                     Thread create function        */
/*    _tx_thread_priority_change            Thread priority change        */
/*    _tx_thread_resume                     Application resume service    */
/*    _tx_thread_timeout                    Thread timeout                */
/*    _tx_thread_wait_abort                 Thread wait abort             */
/*    Other ThreadX Components                                            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
VOID  _tx_thread_system_resume(TX_THREAD *thread_ptr)
{

#ifndef TX_NOT_INTERRUPTABLE

TX_INTERRUPT_SAVE_AREA

#endif

UINT                        priority;
ULONG                       priority_bit;
TX_THREAD                   *head_ptr;
TX_THREAD                   *tail_ptr;
UINT                        core_index;
#ifndef TX_THREAD_SMP_EQUAL_PRIORITY
UINT                        j;
UINT                        lowest_priority;
TX_THREAD                   *next_thread;
ULONG                       test_cores;
UINT                        core;
UINT                        thread_mapped;
TX_THREAD                   *preempt_thread;
ULONG                       possible_cores;
ULONG                       thread_possible_cores;
ULONG                       available_cores;
ULONG                       test_possible_cores;
TX_THREAD                   *possible_preemption_list[TX_THREAD_SMP_MAX_CORES];
#endif
TX_THREAD                   *execute_thread;
UINT                        i;
UINT                        loop_finished;
UINT                        processing_complete;

#ifdef TX_ENABLE_EVENT_TRACE
TX_TRACE_BUFFER_ENTRY       *entry_ptr;
ULONG                       time_stamp =  ((ULONG) 0);
#endif

#if TX_MAX_PRIORITIES > 32
UINT                        map_index;
#endif

#ifndef TX_NO_TIMER
TX_TIMER_INTERNAL           *timer_ptr;
TX_TIMER_INTERNAL           **list_head;
TX_TIMER_INTERNAL           *next_timer;
TX_TIMER_INTERNAL           *previous_timer;
#endif


    /* Set the processing complete flag to false.  */
    processing_complete =  TX_FALSE;

#ifndef TX_NOT_INTERRUPTABLE

    /* Lockout interrupts while the thread is being resumed.  */
    TX_DISABLE
#endif


#ifndef TX_NO_TIMER

    /* Deactivate the timeout timer if necessary.  */
    if ((thread_ptr -> tx_thread_timer.tx_timer_internal_list_head) != TX_NULL)
    {

        /* Deactivate the thread's timeout timer.  This is now done in-line
           for ThreadX SMP so the additional protection logic can be avoided.  */

        /* Deactivate the timer.  */

        /* Pickup internal timer pointer.  */
        timer_ptr =  &(thread_ptr -> tx_thread_timer);

        /* Pickup the list head pointer.  */
        list_head =  timer_ptr -> tx_timer_internal_list_head;

        /* Pickup the next active timer.  */
        next_timer =  timer_ptr -> tx_timer_internal_active_next;

        /* See if this is the only timer in the list.  */
        if (timer_ptr == next_timer)
        {

            /* Yes, the only timer on the list.  */

            /* Determine if the head pointer needs to be updated.  */
            if (*(list_head) == timer_ptr)
            {

                /* Update the head pointer.  */
                *(list_head) =  TX_NULL;
            }
        }
        else
        {

            /* At least one more timer is on the same expiration list.  */

            /* Update the links of the adjacent timers.  */
            previous_timer =                                   timer_ptr -> tx_timer_internal_active_previous;
            next_timer -> tx_timer_internal_active_previous =  previous_timer;
            previous_timer -> tx_timer_internal_active_next =  next_timer;

            /* Determine if the head pointer needs to be updated.  */
            if (*(list_head) == timer_ptr)
            {

                /* Update the next timer in the list with the list head pointer.  */
                next_timer -> tx_timer_internal_list_head =  list_head;

                /* Update the head pointer.  */
                *(list_head) =  next_timer;
            }
        }

        /* Clear the timer's list head pointer.  */
        timer_ptr -> tx_timer_internal_list_head =  TX_NULL;
    }
    else
    {

        /* Clear the remaining time to ensure timer doesn't get activated.  */
        thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  ((ULONG) 0);
    }
#endif

#ifdef TX_ENABLE_STACK_CHECKING

    /* Check this thread's stack.  */
    TX_THREAD_STACK_CHECK(thread_ptr)
#endif


    /* Pickup index.  */
    core_index =  TX_SMP_CORE_ID;

#ifdef TX_ENABLE_EVENT_TRACE

    /* If trace is enabled, save the current event pointer.  */
    entry_ptr =  _tx_trace_buffer_current_ptr;
#endif

    /* Log the thread status change.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_RESUME, thread_ptr, thread_ptr -> tx_thread_state, TX_POINTER_TO_ULONG_CONVERT(&time_stamp), TX_POINTER_TO_ULONG_CONVERT(_tx_thread_execute_ptr[core_index]), TX_TRACE_INTERNAL_EVENTS)

#ifdef TX_THREAD_SMP_DEBUG_ENABLE

    /* Debug entry.  */
    _tx_thread_smp_debug_entry_insert(4, 0, thread_ptr);
#endif

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


    /* Determine if the thread is in the process of suspending.  If so, the thread
       control block is already on the linked list so nothing needs to be done.  */
    if (thread_ptr -> tx_thread_suspending == TX_TRUE)
    {

        /* Make sure the type of suspension under way is not a terminate or
           thread completion.  In either of these cases, do not void the
           interrupted suspension processing.  */
        if (thread_ptr -> tx_thread_state != TX_COMPLETED)
        {

            /* Make sure the thread isn't terminated.  */
            if (thread_ptr -> tx_thread_state != TX_TERMINATED)
            {

                /* No, now check to see if the delayed suspension flag is set.  */
                if (thread_ptr -> tx_thread_delayed_suspend == TX_FALSE)
                {

                    /* Clear the suspending flag.  */
                    thread_ptr -> tx_thread_suspending =   TX_FALSE;

                    /* Restore the state to ready.  */
                    thread_ptr -> tx_thread_state =        TX_READY;

                    /* Thread state change.  */
                    TX_THREAD_STATE_CHANGE(thread_ptr, TX_READY)

                    /* Log the thread status change.  */
                    TX_EL_THREAD_STATUS_CHANGE_INSERT(thread_ptr, TX_READY)
                }
                else
                {

                    /* Clear the delayed suspend flag and change the state.  */
                    thread_ptr -> tx_thread_delayed_suspend =  TX_FALSE;
                    thread_ptr -> tx_thread_state =            TX_SUSPENDED;
                }

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

                /* Increment the total number of thread resumptions.  */
                _tx_thread_performance_resume_count++;

                /* Increment this thread's resume count.  */
                thread_ptr -> tx_thread_performance_resume_count++;
#endif
            }
        }
    }
    else
    {

        /* Check to make sure the thread has not already been resumed.  */
        if (thread_ptr -> tx_thread_state != TX_READY)
        {

            /* Check for a delayed suspend flag.  */
            if (thread_ptr -> tx_thread_delayed_suspend == TX_TRUE)
            {

                /* Clear the delayed suspend flag and change the state.  */
                thread_ptr -> tx_thread_delayed_suspend =  TX_FALSE;
                thread_ptr -> tx_thread_state =            TX_SUSPENDED;
            }
            else
            {

                /* Thread state change.  */
                TX_THREAD_STATE_CHANGE(thread_ptr, TX_READY)

                /* Log the thread status change.  */
                TX_EL_THREAD_STATUS_CHANGE_INSERT(thread_ptr, TX_READY)

                /* Make this thread ready.  */

                /* Change the state to ready.  */
                thread_ptr -> tx_thread_state =  TX_READY;

                /* Pickup priority of thread.  */
                priority =  thread_ptr -> tx_thread_priority;

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

                /* Increment the total number of thread resumptions.  */
                _tx_thread_performance_resume_count++;

                /* Increment this thread's resume count.  */
                thread_ptr -> tx_thread_performance_resume_count++;
#endif

              /* Determine if there are other threads at this priority that are
                   ready.  */
                head_ptr =  _tx_thread_priority_list[priority];
                if (head_ptr != TX_NULL)
                {

                    /* Yes, there are other threads at this priority already ready.  */

                    /* Just add this thread to the priority list.  */
                    tail_ptr =                                 head_ptr -> tx_thread_ready_previous;
                    tail_ptr -> tx_thread_ready_next =         thread_ptr;
                    head_ptr -> tx_thread_ready_previous =     thread_ptr;
                    thread_ptr -> tx_thread_ready_previous =   tail_ptr;
                    thread_ptr -> tx_thread_ready_next =       head_ptr;
                }
                else
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
                }

                /* Determine if a thread with preemption-threshold is currently scheduled.  */
                if (_tx_thread_preemption__threshold_scheduled != TX_NULL)
                {

                    /* Yes, there has been a thread with preemption-threshold scheduled.  */

                    /* Determine if this thread can run with the current preemption-threshold.   */
                    if (priority >= _tx_thread_preemption__threshold_scheduled -> tx_thread_preempt_threshold)
                    {

                        /* The thread cannot run because of the current preemption-threshold. Simply
                           return at this point.  */

#ifndef TX_NOT_INTERRUPTABLE

                        /* Decrement the preemption disable flag.  */
                        _tx_thread_preempt_disable--;
#endif

#ifdef TX_THREAD_SMP_DEBUG_ENABLE

                        /* Debug entry.  */
                        _tx_thread_smp_debug_entry_insert(5, 0, thread_ptr);
#endif

#ifndef TX_NOT_INTERRUPTABLE

                        /* Restore interrupts.  */
                        TX_RESTORE
#endif

                        /* Processing is complete, set the complete flag.  */
                        processing_complete =  TX_TRUE;
                    }
                }

                /* Is the processing complete at this point?  */
                if (processing_complete == TX_FALSE)
                {

                    /* Determine if this newly ready thread has preemption-threshold set. If so, determine
                       if any other threads would need to be unscheduled for this thread to execute.  */
                    if (thread_ptr -> tx_thread_preempt_threshold < priority)
                    {

                        /* Is there a place in the execution list for the newly ready thread?  */
                        i =  ((UINT) 0);
                        loop_finished =  TX_FALSE;
#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX
                        while(i < ((UINT) TX_THREAD_SMP_MAX_CORES))
#else
                        while(i < _tx_thread_smp_max_cores)
#endif
                        {

                            /* Pickup the current execute thread for this core.  */
                            execute_thread =  _tx_thread_execute_ptr[i];

                            /* Is there a thread mapped to this core?  */
                            if (execute_thread == TX_NULL)
                            {

                                /* Get out of the loop.  */
                                loop_finished =  TX_TRUE;
                            }
                            else
                            {

                                /* Determine if this thread should preempt the thread in the execution list.  */
                                if (priority < execute_thread -> tx_thread_preempt_threshold)
                                {

                                    /* Get out of the loop.  */
                                    loop_finished =  TX_TRUE;
                                }
                            }

                            /* Determine if we need to get out of the loop.  */
                            if (loop_finished == TX_TRUE)
                            {

                                /* Get out of the loop.  */
                                break;
                            }

                            /* Move to next index.  */
                            i++;
                        }

                        /* Determine if there is a reason to rebalance the list.  */
#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX
                        if (i < ((UINT) TX_THREAD_SMP_MAX_CORES))
#else
                        if (i < _tx_thread_smp_max_cores)
#endif
                        {

                            /* Yes, the new thread has preemption-threshold set and there is a slot in the
                               execution list for it.  */

                            /* Call the rebalance routine. This routine maps cores and ready threads.  */
                            _tx_thread_smp_rebalance_execute_list(core_index);
                        }
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

                        /* Determine if this thread has any available cores to execute on.  */
                        if (thread_ptr -> tx_thread_smp_cores_allowed != ((ULONG) 0))
                        {

                            /* At this point we know that the newly ready thread does not have preemption-threshold set and that
                               any existing preemption-threshold is not blocking this thread from executing.  */

                            /* Pickup the core this thread was previously executing on.  */
                            i =  thread_ptr -> tx_thread_smp_core_mapped;

                            /* Pickup the currently executing thread for the previously mapped core.  */
                            execute_thread =  _tx_thread_execute_ptr[i];

                            /* First, let's see if the last core this thread executed on is available.  */
                            if (execute_thread == TX_NULL)
                            {

                                /* Yes, simply place this thread into the execute list at the same location.  */
                                _tx_thread_execute_ptr[i] =  thread_ptr;

                                /* If necessary, interrupt the core with the new thread to schedule.  */
                                _tx_thread_smp_core_interrupt(thread_ptr, core_index, i);

                                /* If necessary, wakeup the target core.  */
                                _tx_thread_smp_core_wakeup(core_index, i);
                            }
                            else
                            {

                                /* This core is not able to execute on the core it last executed on
                                   because another thread is already scheduled on that core.  */

                                /* Pickup the available cores for the newly ready thread.  */
                                available_cores =  thread_ptr -> tx_thread_smp_cores_allowed;

                                /* Isolate the lowest set bit so we can determine if more than one core is
                                   available.  */
                                available_cores =  available_cores & ((~available_cores) + ((ULONG) 1));

                                /* Determine if either this thread or the currently schedule thread can
                                   run on more than one core or on a different core and preemption is not
                                   possible.  */
                                if ((available_cores == thread_ptr -> tx_thread_smp_cores_allowed) &&
                                    (available_cores == execute_thread -> tx_thread_smp_cores_allowed))
                                {

                                    /* Both this thread and the execute thread can only execute on the same core,
                                       so this thread can only be scheduled if its priority is less. Otherwise,
                                       there is nothing else to examine.  */
                                    if (thread_ptr -> tx_thread_priority < execute_thread -> tx_thread_priority)
                                    {

                                        /* We know that we have to preempt the executing thread.  */

                                        /* Preempt the executing thread.  */
                                        _tx_thread_execute_ptr[i] =  thread_ptr;

                                        /* If necessary, interrupt the core with the new thread to schedule.  */
                                        _tx_thread_smp_core_interrupt(thread_ptr, core_index, i);

                                        /* If necessary, wakeup the core.  */
                                        _tx_thread_smp_core_wakeup(core_index, i);
                                    }
                                }
                                else
                                {

                                    /* Determine if there are any available cores to execute on.  */
                                    available_cores =  _tx_thread_smp_available_cores_get();

                                    /* Determine what the possible cores are for this thread.  */
                                    thread_possible_cores =  thread_ptr -> tx_thread_smp_cores_allowed;

                                    /* Set the thread mapped flag to false.  */
                                    thread_mapped =  TX_FALSE;

                                    /* Determine if there are available cores.  */
                                    if (available_cores != ((ULONG) 0))
                                    {

                                        /* Determine if one of the available cores is allowed for this thread.  */
                                        if ((available_cores & thread_possible_cores) != ((ULONG) 0))
                                        {

                                            /* Calculate the lowest set bit of allowed cores.  */
                                            test_cores =  (thread_possible_cores & available_cores);
                                            TX_LOWEST_SET_BIT_CALCULATE(test_cores, i)

                                            /* Remember this index in the thread control block.  */
                                            thread_ptr -> tx_thread_smp_core_mapped =  i;

                                            /* Map this thread to the free slot.  */
                                            _tx_thread_execute_ptr[i] =  thread_ptr;

                                            /* Indicate this thread was mapped.  */
                                            thread_mapped =  TX_TRUE;

                                            /* If necessary, wakeup the target core.  */
                                            _tx_thread_smp_core_wakeup(core_index, i);
                                        }
                                        else
                                        {

                                            /* There are available cores, however, they are all excluded.  */

                                            /* Calculate the possible cores from the cores currently scheduled.  */
                                            possible_cores =  _tx_thread_smp_possible_cores_get();

                                            /* Determine if it is worthwhile to try to remap the execution list.  */
                                            if ((available_cores & possible_cores) != ((ULONG) 0))
                                            {

                                                /* Yes, some of the currently scheduled threads can be moved.  */

                                                /* Now determine if there could be a remap solution that will allow us to schedule this thread.  */

                                                /* Narrow to the current possible cores.  */
                                                thread_possible_cores =  thread_possible_cores & possible_cores;

                                                /* Now we need to see if one of the other threads in the non-excluded cores can be moved to make room
                                                   for this thread.  */

                                                /* Default the schedule list to the current execution list.  */
                                                _tx_thread_smp_schedule_list_setup();

                                                /* Determine the possible core mapping.  */
                                                test_possible_cores =  possible_cores & ~(thread_possible_cores);

                                                /* Attempt to remap the cores in order to schedule this thread.  */
                                                core =  _tx_thread_smp_remap_solution_find(thread_ptr, available_cores, thread_possible_cores, test_possible_cores);

                                                /* Determine if remapping was successful.  */
                                                if (core != ((UINT) TX_THREAD_SMP_MAX_CORES))
                                                {

                                                    /* Clear the execute list.  */
                                                    _tx_thread_smp_execute_list_clear();

                                                    /* Setup the execute list based on the updated schedule list.  */
                                                    _tx_thread_smp_execute_list_setup(core_index);

                                                    /* Indicate this thread was mapped.  */
                                                    thread_mapped =  TX_TRUE;
                                                }
                                            }
                                        }
                                    }

                                    /* Determine if we need to investigate thread preemption.  */
                                    if (thread_mapped == TX_FALSE)
                                    {

                                        /* At this point, we need to first check for thread preemption possibilities.  */
                                        lowest_priority =  _tx_thread_smp_lowest_priority_get();

                                        /* Does this thread have a higher priority?  */
                                        if (thread_ptr -> tx_thread_priority < lowest_priority)
                                        {

                                            /* Yes, preemption is possible.  */

                                            /* Pickup the thread to preempt.  */
                                            preempt_thread =  _tx_thread_priority_list[lowest_priority];

                                            /* Determine if there are more than one thread ready at this priority level.  */
                                            if (preempt_thread -> tx_thread_ready_next != preempt_thread)
                                            {

                                                /* Remember the list head.  */
                                                head_ptr =  preempt_thread;

                                                /* Setup thread search pointer to the start of the list.  */
                                                next_thread =  preempt_thread -> tx_thread_ready_next;

                                                /* Loop to find the last thread scheduled at this priority.  */
                                                i =  ((UINT) 0);
#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX
                                                while (i < ((UINT) TX_THREAD_SMP_MAX_CORES))
#else

                                                while (i < _tx_thread_smp_max_cores)
#endif
                                                {

                                                    /* Is this thread currently scheduled?  */
                                                    if (next_thread == _tx_thread_execute_ptr[next_thread -> tx_thread_smp_core_mapped])
                                                    {

                                                        /* Yes, this is the new preempt thread.  */
                                                        preempt_thread =  next_thread;

                                                        /* Increment core count. */
                                                        i++;
                                                    }

                                                    /* Move to the next thread.  */
                                                    next_thread =  next_thread -> tx_thread_ready_next;

                                                    /* Are we at the head of the list?  */
                                                    if (next_thread == head_ptr)
                                                    {

                                                        /* End the loop.  */
                                                        i =  ((UINT) TX_THREAD_SMP_MAX_CORES);
                                                    }
                                                }
                                            }

                                            /* Calculate the core that this thread is scheduled on.  */
                                            possible_cores =  (((ULONG) 1) << preempt_thread -> tx_thread_smp_core_mapped);

                                            /* Determine if preemption is possible.  */
                                            if ((thread_possible_cores & possible_cores) != ((ULONG) 0))
                                            {

                                                /* Pickup the newly available core.  */
                                                i =  preempt_thread -> tx_thread_smp_core_mapped;

                                                /* Remember this index in the thread control block.  */
                                                thread_ptr -> tx_thread_smp_core_mapped =  i;

                                                /* Map this thread to the free slot.  */
                                                _tx_thread_execute_ptr[i] =  thread_ptr;

                                                /* If necessary, interrupt the core with the new thread to schedule.  */
                                                _tx_thread_smp_core_interrupt(thread_ptr, core_index, i);

                                                /* If necessary, wakeup the target core.  */
                                                _tx_thread_smp_core_wakeup(core_index, i);
                                            }
                                            else
                                            {

                                                /* Build the list of possible thread preemptions, ordered lowest priority first.  */
                                                possible_cores =  _tx_thread_smp_preemptable_threads_get(thread_ptr -> tx_thread_priority, possible_preemption_list);

                                                /* Determine if preemption is possible.  */

                                                /* Loop through the potential threads can can be preempted.  */
                                                i =  ((UINT) 0);
                                                loop_finished =  TX_FALSE;
                                                while (possible_preemption_list[i] != TX_NULL)
                                                {

                                                    /* Pickup the thread to preempt.  */
                                                    preempt_thread =  possible_preemption_list[i];

                                                    /* Pickup the core this thread is mapped to.  */
                                                    j =  preempt_thread -> tx_thread_smp_core_mapped;

                                                    /* Calculate the core that this thread is scheduled on.  */
                                                    available_cores =  (((ULONG) 1) << j);

                                                    /* Can this thread execute on this core?  */
                                                    if ((thread_possible_cores & available_cores) != ((ULONG) 0))
                                                    {

                                                        /* Remember this index in the thread control block.  */
                                                        thread_ptr -> tx_thread_smp_core_mapped =  j;

                                                        /* Map this thread to the free slot.  */
                                                        _tx_thread_execute_ptr[j] =  thread_ptr;

                                                        /* If necessary, interrupt the core with the new thread to schedule.  */
                                                        _tx_thread_smp_core_interrupt(thread_ptr, core_index, j);

                                                        /* If necessary, wakeup the target core.  */
                                                        _tx_thread_smp_core_wakeup(core_index, j);

                                                        /* Finished with the preemption condition.  */
                                                        loop_finished =  TX_TRUE;
                                                    }
                                                    else
                                                    {

                                                        /* No, the thread to preempt is not running on a core available to the new thread.
                                                           Attempt to find a remapping solution.  */

                                                        /* Narrow to the current possible cores.  */
                                                        thread_possible_cores =  thread_possible_cores & possible_cores;

                                                        /* Now we need to see if one of the other threads in the non-excluded cores can be moved to make room
                                                           for this thread.  */

                                                        /* Temporarily set the execute thread to NULL.  */
                                                        _tx_thread_execute_ptr[j] =  TX_NULL;

                                                        /* Default the schedule list to the current execution list.  */
                                                        _tx_thread_smp_schedule_list_setup();

                                                        /* Determine the possible core mapping.  */
                                                        test_possible_cores =  possible_cores & ~(thread_possible_cores);

                                                        /* Attempt to remap the cores in order to schedule this thread.  */
                                                        core =  _tx_thread_smp_remap_solution_find(thread_ptr, available_cores, thread_possible_cores, test_possible_cores);

                                                        /* Determine if remapping was successful.  */
                                                        if (core != ((UINT) TX_THREAD_SMP_MAX_CORES))
                                                        {

                                                            /* Clear the execute list.  */
                                                            _tx_thread_smp_execute_list_clear();

                                                            /* Setup the execute list based on the updated schedule list.  */
                                                            _tx_thread_smp_execute_list_setup(core_index);

                                                            /* Finished with the preemption condition.  */
                                                            loop_finished =  TX_TRUE;
                                                        }
                                                        else
                                                        {

                                                            /* Restore the preempted thread and examine the next thread.  */
                                                            _tx_thread_execute_ptr[j] =  preempt_thread;
                                                        }
                                                    }

                                                    /* Determine if we should get out of the loop.  */
                                                    if (loop_finished == TX_TRUE)
                                                    {

                                                        /* Yes, get out of the loop.  */
                                                        break;
                                                    }

                                                    /* Move to the next possible thread preemption.  */
                                                    i++;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
#endif
                }
            }
        }
    }

    /* Determine if there is more processing.  */
    if (processing_complete == TX_FALSE)
    {

#ifdef TX_THREAD_SMP_DEBUG_ENABLE

        /* Debug entry.  */
        _tx_thread_smp_debug_entry_insert(5, 0, thread_ptr);
#endif

#ifdef TX_ENABLE_EVENT_TRACE

        /* Check that the event time stamp is unchanged.  A different
           timestamp means that a later event wrote over the thread
           resume event. In that case, do nothing here.  */
        if ((entry_ptr != TX_NULL) && (time_stamp == entry_ptr -> tx_trace_buffer_entry_time_stamp))
        {

            /* Timestamp is the same, set the "next thread pointer" to NULL. This can
               be used by the trace analysis tool to show idle system conditions.  */
#ifdef TX_MISRA_ENABLE
            entry_ptr -> tx_trace_buffer_entry_info_4 =  TX_POINTER_TO_ULONG_CONVERT(_tx_thread_execute_ptr[core_index]);
#else
            entry_ptr -> tx_trace_buffer_entry_information_field_4 =  TX_POINTER_TO_ULONG_CONVERT(_tx_thread_execute_ptr[core_index]);
#endif
        }
#endif

#ifndef TX_NOT_INTERRUPTABLE

        /* Decrement the preemption disable flag.  */
        _tx_thread_preempt_disable--;
#endif

        if (_tx_thread_current_ptr[core_index] != _tx_thread_execute_ptr[core_index])
        {

#ifdef TX_ENABLE_STACK_CHECKING

            /* Pickup the next thread to execute.  */
            thread_ptr =  _tx_thread_execute_ptr[core_index];

            /* Determine if there is a thread pointer.  */
            if (thread_ptr != TX_NULL)
            {

                /* Check this thread's stack.  */
                TX_THREAD_STACK_CHECK(thread_ptr)
            }
#endif

            /* Now determine if preemption should take place. This is only possible if the current thread pointer is
               not the same as the execute thread pointer AND the system state and preempt disable flags are clear.  */
            if (_tx_thread_system_state[core_index] == ((ULONG) 0))
            {

                /* Is the preempt disable flag set?  */
                if (_tx_thread_preempt_disable == ((UINT) 0))
                {


#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

                    /* No, there is another thread ready to run and will be scheduled upon return.  */
                    _tx_thread_performance_non_idle_return_count++;
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
VOID  _tx_thread_system_ni_resume(TX_THREAD *thread_ptr)
{

    /* Call system resume.  */
    _tx_thread_system_resume(thread_ptr);
}
#endif

