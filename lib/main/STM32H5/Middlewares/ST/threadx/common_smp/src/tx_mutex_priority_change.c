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
#define TX_THREAD_SMP_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_thread.h"
#include "tx_mutex.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_mutex_priority_change                          PORTABLE SMP     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function changes the priority of the specified thread for the  */
/*    priority inheritance option of the mutex service.                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Pointer to thread to suspend  */
/*    new_priority                          New thread priority           */
/*    new_threshold                         New preemption-threshold      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_smp_rebalance_execute_list                               */
/*                                      Rebalance the execution list      */
/*    _tx_thread_smp_simple_priority_change                               */
/*                                      Change priority                   */
/*    _tx_thread_system_resume          Resume thread                     */
/*    _tx_thread_system_ni_resume       Non-interruptable resume thread   */
/*    _tx_thread_system_suspend         Suspend thread                    */
/*    _tx_thread_system_ni_suspend      Non-interruptable suspend thread  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_mutex_get                     Inherit priority                  */
/*    _tx_mutex_put                     Restore previous priority         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
VOID  _tx_mutex_priority_change(TX_THREAD *thread_ptr, UINT new_priority)
{

#ifndef TX_NOT_INTERRUPTABLE

TX_INTERRUPT_SAVE_AREA
#endif

TX_THREAD       *execute_ptr;
UINT            core_index;
UINT            original_priority;
UINT            lowest_priority;
TX_THREAD       *original_pt_thread;
#ifndef TX_DISABLE_PREEMPTION_THRESHOLD
TX_THREAD       *new_pt_thread;
UINT            priority;
ULONG           priority_bit;
#if TX_MAX_PRIORITIES > 32
UINT            map_index;
#endif
#endif
UINT            finished;


    /* Default finished to false.  */
    finished =  TX_FALSE;

#ifndef TX_NOT_INTERRUPTABLE

    /* Lockout interrupts while the thread is being suspended.  */
    TX_DISABLE
#endif

    /* Determine if there is anything to do.  */
    if (thread_ptr -> tx_thread_priority == new_priority)
    {

        if (thread_ptr -> tx_thread_preempt_threshold == new_priority)
        {

            /* Set the finished flag to true.  */
            finished =  TX_TRUE;
        }
    }

    /* Determine if there is still more to do.  */
    if (finished == TX_FALSE)
    {

        /* Default the execute pointer to NULL.  */
        execute_ptr =  TX_NULL;

        /* Determine if this thread is currently ready.  */
        if (thread_ptr -> tx_thread_state != TX_READY)
        {

            /* Change thread priority to the new mutex priority-inheritance priority.  */
            thread_ptr -> tx_thread_priority =  new_priority;

            /* Determine how to setup the thread's preemption-threshold.  */
            if (thread_ptr -> tx_thread_user_preempt_threshold < new_priority)
            {

                /* Change thread preemption-threshold to the user's preemption-threshold.  */
                thread_ptr -> tx_thread_preempt_threshold =  thread_ptr -> tx_thread_user_preempt_threshold;
            }
            else
            {

                /* Change the thread preemption-threshold to the new threshold.  */
                thread_ptr -> tx_thread_preempt_threshold =  new_priority;
            }

        }
        else
        {

            /* Pickup the core index.  */
            core_index =  thread_ptr -> tx_thread_smp_core_mapped;

            /* Save the original priority.  */
            original_priority =  thread_ptr -> tx_thread_priority;

            /* Determine if this thread is the currently executing thread.  */
            if (thread_ptr == _tx_thread_execute_ptr[core_index])
            {

                /* Yes, this thread is scheduled.  */

                /* Remember this thread as the currently executing thread.  */
                execute_ptr =  thread_ptr;

                /* Determine if the thread is being set to a higher-priority and it does't have
                   preemption-threshold set.  */
                if (new_priority < thread_ptr -> tx_thread_priority)
                {

                    /* Check for preemption-threshold.  */
                    if (thread_ptr -> tx_thread_user_priority == thread_ptr -> tx_thread_user_preempt_threshold)
                    {

                        /* Simple case, remove the thread from the current priority list and place in
                           the higher priority list.   */
                        _tx_thread_smp_simple_priority_change(thread_ptr, new_priority);

                        /* Set the finished flag to true.  */
                        finished =  TX_TRUE;
                    }
                }
            }
            else
            {

                /* Thread is not currently executing, so it can just be moved to the lower priority in the list.  */

                /* Determine if the thread is being set to a lower-priority and it does't have
                   preemption-threshold set.  */
                if (new_priority > thread_ptr -> tx_thread_priority)
                {

                    /* Check for preemption-threshold.  */
                    if (thread_ptr -> tx_thread_user_priority == thread_ptr -> tx_thread_user_preempt_threshold)
                    {

                        /* Simple case, remove the thread from the current priority list and place in
                           the lower priority list.   */
                        if (new_priority < thread_ptr -> tx_thread_user_priority)
                        {

                            /* Use the new priority.  */
                            _tx_thread_smp_simple_priority_change(thread_ptr, new_priority);
                        }
                        else
                        {

                            /* Use the user priority.  */
                            _tx_thread_smp_simple_priority_change(thread_ptr, thread_ptr -> tx_thread_user_priority);
                        }

                        /* Set the finished flag to true.  */
                        finished =  TX_TRUE;
                    }
                }
            }

            /* Now determine if we are finished.  */
            if (finished == TX_FALSE)
            {

                /* Save the original preemption-threshold thread.  */
                original_pt_thread =  _tx_thread_preemption__threshold_scheduled;

#ifdef TX_NOT_INTERRUPTABLE

                /* Increment the preempt disable flag.  */
                _tx_thread_preempt_disable++;

                /* Set the state to priority change.  */
                thread_ptr -> tx_thread_state =    TX_PRIORITY_CHANGE;

                /* Call actual non-interruptable thread suspension routine.  */
                _tx_thread_system_ni_suspend(thread_ptr, ((ULONG) 0));

                /* At this point, the preempt disable flag is still set, so we still have
                   protection against all preemption.  */

                /* Determine how to setup the thread's priority.  */
                if (thread_ptr -> tx_thread_user_priority < new_priority)
                {

                    /* Change thread priority to the user's priority.  */
                    thread_ptr -> tx_thread_priority =  thread_ptr -> tx_thread_user_priority;
                }
                else
                {

                    /* Change thread priority to the new mutex priority-inheritance priority.  */
                    thread_ptr -> tx_thread_priority =  new_priority;
                }

                /* Determine how to setup the thread's preemption-threshold.  */
                if (thread_ptr -> tx_thread_user_preempt_threshold < new_priority)
                {

                    /* Change thread preemption-threshold to the user's preemption-threshold.  */
                    thread_ptr -> tx_thread_preempt_threshold =  thread_ptr -> tx_thread_user_preempt_threshold;
                }
                else
                {

                    /* Change the thread preemption-threshold to the new threshold.  */
                    thread_ptr -> tx_thread_preempt_threshold =  new_priority;
                }

                /* Resume the thread with the new priority.  */
                _tx_thread_system_ni_resume(thread_ptr);

                /* Decrement the preempt disable flag.  */
                _tx_thread_preempt_disable--;
#else

                /* Increment the preempt disable flag.  */
                _tx_thread_preempt_disable =  _tx_thread_preempt_disable + ((UINT) 2);

                /* Set the state to priority change.  */
                thread_ptr -> tx_thread_state =    TX_PRIORITY_CHANGE;

                /* Set the suspending flag. */
                thread_ptr -> tx_thread_suspending =  TX_TRUE;

                /* Setup the timeout period.  */
                thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  ((UINT) 0);

                /* Restore interrupts.  */
                TX_RESTORE

                /* The thread is ready and must first be removed from the list.  Call the
                   system suspend function to accomplish this.  */
                _tx_thread_system_suspend(thread_ptr);

                /* Lockout interrupts again.  */
                TX_DISABLE

                /* At this point, the preempt disable flag is still set, so we still have
                   protection against all preemption.  */

                /* Determine how to setup the thread's priority.  */
                if (thread_ptr -> tx_thread_user_priority < new_priority)
                {

                    /* Change thread priority to the user's priority.  */
                    thread_ptr -> tx_thread_priority =  thread_ptr -> tx_thread_user_priority;
                }
                else
                {

                    /* Change thread priority to the new mutex priority-inheritance priority.  */
                    thread_ptr -> tx_thread_priority =  new_priority;
                }

                /* Determine how to setup the thread's preemption-threshold.  */
                if (thread_ptr -> tx_thread_user_preempt_threshold < new_priority)
                {

                    /* Change thread preemption-threshold to the user's preemption-threshold.  */
                    thread_ptr -> tx_thread_preempt_threshold =  thread_ptr -> tx_thread_user_preempt_threshold;
                }
                else
                {

                    /* Change the thread preemption-threshold to the new threshold.  */
                    thread_ptr -> tx_thread_preempt_threshold =  new_priority;
                }

                /* Restore interrupts.  */
                TX_RESTORE

                /* Resume the thread with the new priority.  */
                _tx_thread_system_resume(thread_ptr);
#endif

                /* Optional processing extension.  */
                TX_MUTEX_PRIORITY_CHANGE_EXTENSION

#ifndef TX_NOT_INTERRUPTABLE

                /* Disable interrupts.  */
                TX_DISABLE
#endif

                /* Determine if the thread was previously executing.  */
                if (thread_ptr == execute_ptr)
                {

                    /* Make sure the thread is still ready.  */
                    if (thread_ptr -> tx_thread_state == TX_READY)
                    {

#ifndef TX_DISABLE_PREEMPTION_THRESHOLD
                        /* Determine if preemption-threshold is in force at the new priority level.  */
                        if (_tx_thread_preemption_threshold_list[thread_ptr -> tx_thread_priority] == TX_NULL)
                        {

                            /* Ensure that this thread is placed at the front of the priority list.  */
                            _tx_thread_priority_list[thread_ptr -> tx_thread_priority] =  thread_ptr;
                        }
#else

                        /* Ensure that this thread is placed at the front of the priority list.  */
                        _tx_thread_priority_list[thread_ptr -> tx_thread_priority] =  thread_ptr;
#endif
                    }
                }

                /* Pickup the core index.  */
                core_index =  thread_ptr -> tx_thread_smp_core_mapped;

#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX

                /* Pickup the next thread to execute.   */
                if (core_index < ((UINT) TX_THREAD_SMP_MAX_CORES))
#else

                /* Pickup the next thread to execute.   */
                if (core_index < _tx_thread_smp_max_cores)
#endif
                {

                    /* Determine if this thread is not the next thread to execute.  */
                    if (thread_ptr != _tx_thread_execute_ptr[core_index])
                    {

                        /* Now determine if this thread was previously executing thread.  */
                        if (thread_ptr == execute_ptr)
                        {

                            /* Determine if we moved to a lower priority. If so, move the thread to the front of the priority list.  */
                            if (original_priority < new_priority)
                            {

                                /* Make sure the thread is still ready.  */
                                if (thread_ptr -> tx_thread_state == TX_READY)
                                {

                                    /* Determine the lowest priority scheduled thread.  */
                                    lowest_priority =  _tx_thread_smp_lowest_priority_get();

                                    /* Determine if this thread has a higher or same priority as the lowest priority
                                       in the list.  */
                                    if (thread_ptr -> tx_thread_priority <= lowest_priority)
                                    {

                                        /* Yes, we need to rebalance to make it possible for this thread to execute.  */

                                        /* Determine if the thread with preemption-threshold thread has changed... and is
                                           not the scheduled thread.   */
                                        if ((original_pt_thread != _tx_thread_preemption__threshold_scheduled) &&
                                            (original_pt_thread != thread_ptr))
                                        {

                                            /* Yes, preemption-threshold has changed.  Determine if it can or should
                                               be reversed.  */

#ifndef TX_DISABLE_PREEMPTION_THRESHOLD

                                            /* Pickup the preemption-threshold thread.  */
                                            new_pt_thread =  _tx_thread_preemption__threshold_scheduled;
#endif

                                            /* Restore the original preemption-threshold thread.  */
                                            _tx_thread_preemption__threshold_scheduled =  original_pt_thread;


#ifndef TX_DISABLE_PREEMPTION_THRESHOLD

                                            /* Determine if there is a new preemption-threshold thread to reverse.  */
                                            if (new_pt_thread != TX_NULL)
                                            {

                                                /* Clear the information associated with the new preemption-threshold thread.  */

                                                /* Pickup the priority.  */
                                                priority =  new_pt_thread -> tx_thread_priority;

                                                /* Clear the preempted list entry.  */
                                                _tx_thread_preemption_threshold_list[priority] =  TX_NULL;

#if TX_MAX_PRIORITIES > 32
                                                /* Calculate the bit map array index.  */
                                                map_index =  new_priority/((UINT) 32);
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
                                            }
#endif
                                        }

                                        /* Pickup the index.  */
                                        core_index =  TX_SMP_CORE_ID;

                                        /* Call the rebalance routine. This routine maps cores and ready threads.  */
                                        _tx_thread_smp_rebalance_execute_list(core_index);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

#ifndef TX_NOT_INTERRUPTABLE

    /* Restore interrupts.  */
    TX_RESTORE
#endif
}

