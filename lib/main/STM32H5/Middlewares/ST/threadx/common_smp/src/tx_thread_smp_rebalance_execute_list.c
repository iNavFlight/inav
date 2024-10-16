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
/**   Thread - High Level SMP Support                                     */
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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_smp_rebalance_execute_list              PORTABLE SMP     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is responsible for mapping ready ThreadX threads with */
/*    cores in the SMP . The basic idea is the standard ThreadX           */
/*    ready list is traversed to build the _tx_thread_execute_ptr list.   */
/*    Each index represents the and the corresponding entry in this       */
/*    array contains the thread that should be executed by that core. If  */
/*    the was previously running a different thread, it will be           */
/*    preempted and restarted so it can run the new thread.               */
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
/*    _tx_thread_smp_execute_list_clear     Clear the thread execute list */
/*    _tx_thread_smp_execute_list_setup     Setup the thread execute list */
/*    _tx_thread_smp_next_priority_find     Find next priority with one   */
/*                                            or more ready threads       */
/*    _tx_thread_smp_remap_solution_find    Attempt to remap threads to   */
/*                                            schedule another thread     */
/*    _tx_thread_smp_schedule_list_clear    Clear the thread schedule list*/
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_mutex_priority_change             Mutex priority change         */
/*    _tx_thread_create                     Thread create                 */
/*    _tx_thread_preemption_change          Thread preemption change      */
/*    _tx_thread_priority_change            Thread priority change        */
/*    _tx_thread_relinquish                 Thread relinquish             */
/*    _tx_thread_resume                     Thread resume                 */
/*    _tx_thread_smp_core_exclude           Thread SMP core exclude       */
/*    _tx_thread_system_resume              Thread system resume          */
/*    _tx_thread_system_suspend             Thread suspend                */
/*    _tx_thread_time_slice                 Thread time-slice             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
void  _tx_thread_smp_rebalance_execute_list(UINT core_index)
{

UINT            i, j, core;
UINT            next_priority;
UINT            last_priority;
TX_THREAD       *schedule_thread;
#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO
TX_THREAD       *mapped_thread;
#endif
TX_THREAD       *preempted_thread;
ULONG           possible_cores;
ULONG           thread_possible_cores;
ULONG           available_cores;
ULONG           test_possible_cores;
ULONG           test_cores;
UINT            this_pass_complete;
UINT            loop_finished;

#ifdef TX_THREAD_SMP_EQUAL_PRIORITY
TX_THREAD       *highest_priority_thread;
#endif
#ifndef TX_DISABLE_PREEMPTION_THRESHOLD
ULONG           priority_bit;
#if TX_MAX_PRIORITIES > 32
UINT            map_index;
#endif
#endif


    /* It is assumed that the preempt disable flag is still set at this point.  */

    /* Pickup the last schedule thread with preemption-threshold enabled.  */
    preempted_thread =  _tx_thread_preemption__threshold_scheduled;

    /* Clear the schedule list.  */
    _tx_thread_smp_schedule_list_clear();

    /* Initialize the next priority to 0, the highest priority.  */
    next_priority =  ((UINT) 0);

    /* Initialize the last priority.  */
    last_priority =  ((UINT) 0);

#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX

    /* Set the possible cores bit map to all cores.  */
    possible_cores =  ((ULONG) TX_THREAD_SMP_CORE_MASK);
#else

    /* Set the possible cores bit map to all cores.  */
    possible_cores =  (((ULONG) 1) << _tx_thread_smp_max_cores) - 1;
#endif

    /* Setup the available cores bit map.  */
    available_cores =  possible_cores;

    /* Clear the schedule thread pointer.  */
    schedule_thread =  TX_NULL;

#ifdef TX_THREAD_SMP_EQUAL_PRIORITY

    /* Set the highest priority thread to NULL.  */
    highest_priority_thread =  TX_NULL;
#endif

    /* Loop to rebuild the schedule list.  */
    i =  ((UINT) 0);
    loop_finished =  TX_FALSE;
    do
    {

        /* Clear the pass complete flag, which is used to skip the remaining processing
           of this loop on certain conditions.  */
        this_pass_complete =  TX_FALSE;

        /* Determine if there is a thread to schedule.  */
        if (schedule_thread == TX_NULL)
        {

            /* Calculate the next ready priority.  */
            next_priority =  _tx_thread_smp_next_priority_find(next_priority);

            /* Determine if there are no more threads to execute.  */
            if (next_priority == ((UINT) TX_MAX_PRIORITIES))
            {

                /* Break out of loop.  */
                loop_finished =       TX_TRUE;
                this_pass_complete =  TX_TRUE;
            }
            else
            {

                /* Determine if a thread was executed with preemption-threshold set.  */
                if (preempted_thread != TX_NULL)
                {

                    /* Yes, a thread was previously preempted.  Let's first see if we reached the
                       interrupted preemption-threshold level.  */
                    if (next_priority >= preempted_thread -> tx_thread_preempt_threshold)
                    {

                        /* Yes, now lets see if we are within the preemption-threshold level.  */
                        if (next_priority <= preempted_thread -> tx_thread_priority)
                        {

                            /* Yes, move the next priority to the preempted priority.  */
                            next_priority =  preempted_thread -> tx_thread_priority;

                            /* Setup the schedule thread to the preempted thread.  */
                            schedule_thread =  preempted_thread;

                            /* Start at the top of the loop.  */
                            this_pass_complete =  TX_TRUE;
                        }
                        else
                        {

                            /* Nothing else is allowed to execute after the preemption-threshold thread.  */
                            next_priority =  ((UINT) TX_MAX_PRIORITIES);

                            /* Break out of loop.  */
                            loop_finished =       TX_TRUE;
                            this_pass_complete =  TX_TRUE;
                        }
                    }
                }
            }

            /* Determine if this pass through the loop is already complete.  */
            if (this_pass_complete == TX_FALSE)
            {

                /* Pickup the next thread to schedule.  */
                schedule_thread =  _tx_thread_priority_list[next_priority];
            }
        }

        /* Determine if this pass through the loop is already complete.  */
        if (this_pass_complete == TX_FALSE)
        {

            /* Determine what the possible cores are for this thread.  */
            thread_possible_cores =  schedule_thread -> tx_thread_smp_cores_allowed;

            /* Apply the current possible cores.  */
            thread_possible_cores =  thread_possible_cores & (available_cores | possible_cores);

            /* Determine if it is possible to schedule this thread.  */
            if (thread_possible_cores == ((ULONG) 0))
            {

                /* No, this thread can't be scheduled.  */

                /* Look at the next thread at the same priority level.  */
                schedule_thread =  schedule_thread -> tx_thread_ready_next;

                /* Determine if this is the head of the list.  */
                if (schedule_thread == _tx_thread_priority_list[next_priority])
                {

                    /* Set the schedule thread to NULL to force examination of the next priority level.  */
                    schedule_thread =  TX_NULL;

                    /* Move to the next priority level.  */
                    next_priority++;

                    /* Determine if there are no more threads to execute.  */
                    if (next_priority == ((UINT) TX_MAX_PRIORITIES))
                    {

                        /* Break out of loop.  */
                        loop_finished =       TX_TRUE;
                    }
                }
            }
            else
            {

                /* It is possible to schedule this thread.  */

                /* Determine if this thread has preemption-threshold set.  */
                if (schedule_thread -> tx_thread_preempt_threshold < schedule_thread -> tx_thread_priority)
                {

                    /* Yes, preemption-threshold is set.  */

                    /* Determine if the last priority is above the preemption-threshold.  If not, we can't
                       schedule this thread with preemption-threshold set.  */
                    if ((last_priority >= schedule_thread -> tx_thread_preempt_threshold) && (i != ((UINT) 0)))
                    {

                        /* A thread was found that violates the next thread to be scheduled's preemption-threshold. We will simply
                           skip this thread and see if there is anything else we can schedule. */

                        /* Look at the next thread at the same priority level.  */
                        schedule_thread =  schedule_thread -> tx_thread_ready_next;

                        /* Determine if this is the head of the list.  */
                        if (schedule_thread == _tx_thread_priority_list[next_priority])
                        {

                            /* Set the schedule thread to NULL to force examination of the next priority level.  */
                            schedule_thread =  TX_NULL;

                            /* Move to the next priority level.  */
                            next_priority++;

                            /* Determine if there are no more threads to execute.  */
                            if (next_priority == ((UINT) TX_MAX_PRIORITIES))
                            {

                                /* Break out of loop.  */
                                loop_finished =  TX_TRUE;
                            }
                        }

                        /* Restart the loop.  */
                        this_pass_complete =  TX_TRUE;
                    }
                }

                /* Determine if this pass through the loop is already complete.  */
                if (this_pass_complete == TX_FALSE)
                {

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

                    /* Initialize index to an invalid value.  */
                    j =  ((UINT) TX_THREAD_SMP_MAX_CORES);
#endif

                    /* Determine if there is an available core for this thread to execute on.  */
                    if ((thread_possible_cores & available_cores) != ((ULONG) 0))
                    {

                        /* Pickup the last executed core for this thread.  */
                        j =  schedule_thread -> tx_thread_smp_core_mapped;

                        /* Is this core valid and available?  */
                        if ((thread_possible_cores & available_cores & (((ULONG) 1) << j)) == ((ULONG) 0))
                        {

                            /* No, we must find the next core for this thread.  */
                            test_cores =  (thread_possible_cores & available_cores);
                            TX_LOWEST_SET_BIT_CALCULATE(test_cores, j)

                            /* Setup the last executed core for this thread.  */
                            schedule_thread -> tx_thread_smp_core_mapped =  j;
                        }

                        /* Place the this thread on this core.  */
                        _tx_thread_smp_schedule_list[j] =  schedule_thread;

                        /* Clear the associated available cores bit.  */
                        available_cores =  available_cores & ~(((ULONG) 1) << j);
                    }
                    else
                    {

                        /* Note that we know that the thread must have at least one core excluded at this point,
                           since we didn't find a match and we have available cores.  */

                        /* Now we need to see if one of the other threads in the non-excluded cores can be moved to make room
                           for this thread.  */

                        /* Determine the possible core remapping attempt.  */
                        test_possible_cores =  possible_cores & ~(thread_possible_cores);

                        /* Attempt to remap the cores in order to schedule this thread.  */
                        core =  _tx_thread_smp_remap_solution_find(schedule_thread, available_cores, thread_possible_cores, test_possible_cores);

                        /* Determine if remapping was successful.  */
                        if (core != ((UINT) TX_THREAD_SMP_MAX_CORES))
                        {

                            /* Yes, remapping was successful. Update the available cores accordingly. */
                            available_cores =  available_cores & ~(((ULONG) 1) << core);
                        }
                        else
                        {

                            /* We couldn't assign the thread to any of the cores possible for the thread.  */

                            /* Check to see if the thread is the last thread preempted.  */
                            if (schedule_thread == preempted_thread)
                            {

                                /* To honor the preemption-threshold, we cannot schedule any more threads.  */
                                loop_finished =  TX_TRUE;
                            }
                            else
                            {

                                /* update the available cores for the next pass so we don't waste time looking at them again!  */
                                possible_cores =  possible_cores & (~thread_possible_cores);

                                /* No, we couldn't load the thread because none of the required cores were available.  Look at the next thread at the same priority level.  */
                                schedule_thread =  schedule_thread -> tx_thread_ready_next;

                                /* Determine if this is the head of the list.  */
                                if (schedule_thread == _tx_thread_priority_list[next_priority])
                                {

                                    /* Set the schedule thread to NULL to force examination of the next priority level.  */
                                    schedule_thread =  TX_NULL;

                                    /* Move to the next priority level.  */
                                    next_priority++;

                                    /* Determine if there are no more threads to execute.  */
                                    if (next_priority == ((UINT) TX_MAX_PRIORITIES))
                                    {

                                        /* Break out of loop.  */
                                        loop_finished =  TX_TRUE;
                                    }
                                }
                            }

                            /* Restart the loop.  */
                            this_pass_complete =  TX_TRUE;
                        }
                    }

                    /* Determine if this pass through the loop is already complete.  */
                    if (this_pass_complete == TX_FALSE)
                    {

#ifdef TX_THREAD_SMP_EQUAL_PRIORITY

                        /* Determine if this is the highest priority thread.  */
                        if (highest_priority_thread == TX_NULL)
                        {

                            /* No highest priority yet, remember this thread.  */
                            highest_priority_thread =  schedule_thread;
                        }
#endif

                        /* Increment the number of threads loaded. */
                        i++;

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

                        /* Determine if the thread was mapped.  */
                        if (j != ((UINT) TX_THREAD_SMP_MAX_CORES))
                        {

                            /* Pickup the currently mapped thread.  */
                            mapped_thread =  _tx_thread_execute_ptr[j];

                            /* Determine if preemption is present.  */
                            if ((mapped_thread != TX_NULL) && (schedule_thread != mapped_thread))
                            {

                                /* Determine if the previously mapped thread is still ready.  */
                                if (mapped_thread -> tx_thread_state == TX_READY)
                                {

                                    /* Determine if the caller is an interrupt or from a thread.  */
                                    if (_tx_thread_system_state[core_index] == ((ULONG) 0))
                                    {

                                        /* Caller is a thread, so this is a solicited preemption.  */
                                        _tx_thread_performance_solicited_preemption_count++;

                                        /* Increment the thread's solicited preemption counter.  */
                                        mapped_thread -> tx_thread_performance_solicited_preemption_count++;
                                    }
                                    else
                                    {

                                        /* Is this an interrupt?  */
                                        if (_tx_thread_system_state[core_index] < TX_INITIALIZE_IN_PROGRESS)
                                        {

                                            /* Caller is an interrupt, so this is an interrupt preemption.  */
                                            _tx_thread_performance_interrupt_preemption_count++;

                                            /* Increment the thread's interrupt preemption counter.  */
                                            mapped_thread -> tx_thread_performance_interrupt_preemption_count++;
                                        }
                                    }
                                }
                            }
                        }
#endif

                        /* Determine if this thread has preemption-threshold set.  */
                        if (schedule_thread -> tx_thread_preempt_threshold < schedule_thread -> tx_thread_priority)
                        {

#ifndef TX_DISABLE_PREEMPTION_THRESHOLD

                            /* mark the bit map to show that a thread with preemption-threshold has been executed.  */
#if TX_MAX_PRIORITIES > 32

                            /* Calculate the index into the bit map array.  */
                            map_index =  (schedule_thread -> tx_thread_priority)/((UINT) 32);

                            /* Set the active bit to remember that the preempt map has something set.  */
                            TX_DIV32_BIT_SET(schedule_thread -> tx_thread_priority, priority_bit)
                            _tx_thread_preempted_map_active =  _tx_thread_preempted_map_active | priority_bit;
#endif

                            /* Remember that this thread was executed with preemption-threshold set.  */
                            TX_MOD32_BIT_SET(schedule_thread -> tx_thread_priority, priority_bit)
                            _tx_thread_preempted_maps[MAP_INDEX] =  _tx_thread_preempted_maps[MAP_INDEX] | priority_bit;

                            /* Place the thread in the preempted list indicating preemption-threshold is in force.  */
                            _tx_thread_preemption_threshold_list[schedule_thread -> tx_thread_priority] =  schedule_thread;
#endif

                            /* Set the last thread with preemption-threshold enabled.  */
                            _tx_thread_preemption__threshold_scheduled =  schedule_thread;

                            /* Now break out of the scheduling loop.  */
                            loop_finished =  TX_TRUE;
                        }
                        else
                        {

                            /* Remember the last priority.  */
                            last_priority =  next_priority;

                            /* Pickup the next ready thread at the current priority level.  */
                            schedule_thread =  schedule_thread -> tx_thread_ready_next;

                            /* Determine if this is the head of the list, which implies that we have exhausted this priority level.  */
                            if (schedule_thread == _tx_thread_priority_list[next_priority])
                            {

                                /* Set the schedule thread to NULL to force examination of the next priority level.  */
                                schedule_thread =  TX_NULL;

                                /* Move to the next priority level.  */
                                next_priority++;

#ifdef TX_THREAD_SMP_EQUAL_PRIORITY

                                /* Determine if there is a highest priority thread.  */
                                if (highest_priority_thread)
                                {

                                    /* Yes, break out of the loop, since only same priority threads can be
                                       scheduled in this mode.  */
                                    loop_finished =  TX_TRUE;
                                }
#endif

                                /* Determine if there are no more threads to execute.  */
                                if (next_priority == ((UINT) TX_MAX_PRIORITIES))
                                {

                                    /* Break out of loop.  */
                                    loop_finished =  TX_TRUE;
                                }
                            }
                        }
                    }
                }
            }
        }

        /* Determine if the loop is finished.  */
        if (loop_finished == TX_TRUE)
        {

            /* Finished, break the loop.  */
            break;
        }

#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX

    } while (i < ((UINT) TX_THREAD_SMP_MAX_CORES));
#else

    } while (i < _tx_thread_smp_max_cores);
#endif

    /* Clear the execute list.  */
    _tx_thread_smp_execute_list_clear();

    /* Setup the execute list based on the updated schedule list.  */
    _tx_thread_smp_execute_list_setup(core_index);
}

