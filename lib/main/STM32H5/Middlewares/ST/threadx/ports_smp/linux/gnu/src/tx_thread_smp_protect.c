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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_smp_protect                            SMP/Linux/GCC     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets protection for running inside the ThreadX        */
/*    source. This is acomplished by a combination of a test-and-set      */
/*    flag and periodically disabling interrupts.                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Previous Status Register                                            */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    pthread_self                          Get Linux thread ID           */
/*    GetThreadPriority                     Get current thread priority   */
/*    _tx_thread_smp_core_get               Get the current core ID       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    ThreadX Source                                                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT  _tx_thread_smp_protect(void)
{

pthread_t   current_thread_id;
int         exit_code = 0;
struct sched_param sp;
UINT        core;
UINT        interrupt_posture;
TX_THREAD   *current_thread;
UINT        current_state;

    /* Loop to attempt to get the protection.  */
    do
    {

        /* First, get the critical section.  */
        do
        {


            /* Lock Linux mutex.  */
            _tx_linux_mutex_obtain(&_tx_linux_mutex);

            /* Pickup the current thread ID.  */
            current_thread_id = pthread_self();

            /* Pickup the current core.   */
            core =  _tx_thread_smp_core_get();

            /* Pickup the current thread pointer.  */
            current_thread = _tx_thread_current_ptr[core];

            /* Determine if this is a thread (THREAD_PRIORITY_LOWEST) and it does not
               match the current thread pointer.  */
            if ((_tx_linux_threadx_thread) &&
                ((!current_thread) || (current_thread -> tx_thread_linux_thread_id != current_thread_id)))
            {

                /* This indicates the Linux thread was actually terminated by ThreadX is only
                   being allowed to run in order to cleanup its resources.  */
                _tx_linux_mutex_release_all(&_tx_linux_mutex);

                /* Exit this thread.  */
                pthread_exit((void *)&exit_code);
            }

            /* Determine if this is not actually a thread.  */
            if (!_tx_linux_threadx_thread)
                break;

            /* Now check for terminated or completed state... and preempt disable is not set!   */
            if ((current_thread) && (_tx_thread_preempt_disable == 0))
            {

                /* Pickup current state.  */
                current_state =  current_thread -> tx_thread_state;

                /* Now check for terminated or completed state.  */
                if ((current_state == TX_TERMINATED) || (current_state == TX_COMPLETED))
                {

                    /* Clear the preemption flag.  */
                    current_thread -> tx_thread_linux_deferred_preempt =  TX_FALSE;

                    /* Indicate that this thread was suspended asynchronously.  */
                    current_thread -> tx_thread_linux_suspension_type =  1;

                    /* Save the remaining time-slice and disable it.  */
                    if (_tx_timer_time_slice[core])
                    {

                        current_thread -> tx_thread_time_slice =  _tx_timer_time_slice[core];
                        _tx_timer_time_slice[core] =  0;
                    }

                    /* Clear the current thread pointer.  */
                    _tx_thread_current_ptr[core] =  TX_NULL;

                    /* Clear this mapping entry.  */
                    _tx_linux_virtual_cores[core].tx_thread_smp_core_mapping_thread =               TX_NULL;
                    _tx_linux_virtual_cores[core].tx_thread_smp_core_mapping_linux_thread_id =      0;

                    /* Indicate that this thread is now ready for scheduling again by another core.  */
                    current_thread -> tx_thread_smp_core_control =  1;

                    /* Debug entry.  */
                    _tx_linux_debug_entry_insert("SCHEDULE-thread_terminate_preempt_complete", __FILE__, __LINE__);

                    /* Release the scheduler's semaphore to immediately try again.  */
                    tx_linux_sem_post(&_tx_linux_scheduler_semaphore);

                    /* This indicates the Linux thread was actually terminated by ThreadX is only
                       being allowed to run in order to cleanup its resources.  */
                    _tx_linux_mutex_release_all(&_tx_linux_mutex);

                    /* Exit this thread.  */
                    pthread_exit((void *)&exit_code);
                }
            }

            /* Determine if the deferred preempt flag is set.  */
            if ((current_thread) && (current_thread -> tx_thread_linux_deferred_preempt))
            {

                /* Release the scheduler's semaphore to immediately try again.  */
                tx_linux_sem_post(&_tx_linux_scheduler_semaphore);

                /* Release the protection that is nested.  */
                _tx_linux_mutex_release_all(&_tx_linux_mutex);

                /* Sleep just to let other threads run.  */
                _tx_linux_thread_sleep(1000000);
            }
            else
            {

                /* Get out of the protection loop.  */
                break;
            }
        } while (1);

        /* Setup the returned interrupt posture.  */
        interrupt_posture =  _tx_linux_global_int_disabled_flag;

        /* Determine if the protection is already active for this core.  */
        if (_tx_thread_smp_protection.tx_thread_smp_protect_core == core)
        {

            /* Yes, we have the protection already.  */

            /* Increment the protection count.  */
            _tx_thread_smp_protection.tx_thread_smp_protect_count++;

            /* Set the global interrupt disable value.  */
            _tx_linux_global_int_disabled_flag =  TX_TRUE;

            /* Debug entry.  */
            _tx_linux_debug_entry_insert("PROTECT-obtained-nested", __FILE__, __LINE__);

            /* Get out of the retry loop.  */
            break;
        }
        /* Determine if the protection is available.  */
        else if (_tx_thread_smp_protection.tx_thread_smp_protect_core == 0xFFFFFFFF)
        {

            /* At this point we have the protection.  Setup the protection structure.   */
            _tx_thread_smp_protection.tx_thread_smp_protect_in_force =         TX_TRUE;
            _tx_thread_smp_protection.tx_thread_smp_protect_thread =           current_thread;
            _tx_thread_smp_protection.tx_thread_smp_protect_core =             core;
            _tx_thread_smp_protection.tx_thread_smp_protect_count =            1;
            _tx_thread_smp_protection.tx_thread_smp_protect_linux_thread_id =  current_thread_id;

            /* Set the global interrupt disable value.  */
            _tx_linux_global_int_disabled_flag =  TX_TRUE;

            /* Debug entry.  */
            _tx_linux_debug_entry_insert("PROTECT-obtained", __FILE__, __LINE__);

            /* Get out of the retry loop.  */
            break;
        }
        else
        {

            /* Protection is owned by another core.  */

            /* Release the protection and start over.  */
            _tx_linux_mutex_release(&_tx_linux_mutex);
        }
    } while (1);

    /* Set the global interrupt disable value.  */
    _tx_linux_global_int_disabled_flag =  TX_TRUE;

    /* Return the interrupt posture.  */
    return(interrupt_posture);
}


