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
#include <stdio.h>
#include <errno.h>

extern sem_t _tx_linux_isr_semaphore;
extern UINT _tx_linux_timer_waiting;
extern pthread_t _tx_linux_timer_id;
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_schedule                               SMP/Linux/GCC     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function waits for a thread control block pointer to appear in */
/*    the _tx_thread_execute_ptr variable.  Once a thread pointer appears */
/*    in the variable, the corresponding thread is resumed.               */
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
/*    _tx_linux_mutex_obtain                                              */
/*    _tx_linux_debug_entry_insert                                        */
/*    _tx_linux_thread_resume                                             */
/*    tx_linux_sem_post                                                   */
/*    sem_trywait                                                         */
/*    tx_linux_sem_wait                                                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_initialize_kernel_enter          ThreadX entry function         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
VOID   _tx_thread_schedule(VOID)
{
UINT            core;
TX_THREAD       *current_thread;
TX_THREAD       *execute_thread;
struct timespec ts;
UCHAR preemt_retry = TX_FALSE;

    /* Loop forever.  */
    while(1)
    {

        /* Lock Linux mutex.  */
        _tx_linux_mutex_obtain(&_tx_linux_mutex);

        /* Check for a system error condition.  */
        if (_tx_linux_global_int_disabled_flag != TX_FALSE)
        {

            /* This should not happen... increment the system error counter.  */
            _tx_linux_system_error++;
        }

        /* Debug entry.  */
        _tx_linux_debug_entry_insert("SCHEDULE-wake_up", __FILE__, __LINE__);

        /* Loop through each virtual core to look for an idle core.  */
        for (core = 0; core <  TX_THREAD_SMP_MAX_CORES; core++)
        {

            /* Pickup the current thread pointer for this core.  */
            current_thread =  _tx_thread_current_ptr[core];

            /* Determine if the thread's deferred preemption flag is set.  */
            if ((current_thread) && (current_thread -> tx_thread_linux_deferred_preempt))
            {
                if (_tx_thread_preempt_disable)
                {

                    /* Preemption disabled. Retry. */
                    preemt_retry = TX_TRUE;
                    break;
                }

                if (current_thread -> tx_thread_state != TX_TERMINATED)
                {

                    /* Suspend the thread to simulate preemption.  Note that the thread is suspended BEFORE the protection get
                       flag is checked to ensure there is not a race condition between this thread and the update of that flag.  */
                    _tx_linux_thread_suspend(current_thread -> tx_thread_linux_thread_id);

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
                }

                /* Clear the current thread pointer.  */
                _tx_thread_current_ptr[core] =  TX_NULL;

                /* Clear this mapping entry.  */
                _tx_linux_virtual_cores[core].tx_thread_smp_core_mapping_thread =          TX_NULL;
                _tx_linux_virtual_cores[core].tx_thread_smp_core_mapping_linux_thread_id = 0;

                /* Indicate that this thread is now ready for scheduling again by another core.  */
                current_thread -> tx_thread_smp_core_control =  1;

                /* Debug entry.  */
                _tx_linux_debug_entry_insert("SCHEDULE-core_preempt_complete", __FILE__, __LINE__);
            }

            /* Determine if this core is idle.  */
            if (_tx_thread_current_ptr[core] == TX_NULL)
            {

                /* Yes, this core is idle, determine if there is a thread that can be scheduled for it.  */

                /* Pickup the execute thread pointer.  */
                execute_thread =  _tx_thread_execute_ptr[core];

                /* Is there a thread that is ready to execute on this core?  */
                if ((execute_thread) && (execute_thread -> tx_thread_smp_core_control))
                {

                    /* Yes! We have a thread to execute. Note that the critical section is already
                       active from the scheduling loop above.  */

                    /* Setup the current thread pointer.  */
                    _tx_thread_current_ptr[core] =  execute_thread;

                    /* Remember the virtual core in the thread control block.  */
                    execute_thread -> tx_thread_linux_virtual_core =  core;

                    /* Setup the virtual core mapping structure.  */
                    _tx_linux_virtual_cores[core].tx_thread_smp_core_mapping_thread =          execute_thread;
                    _tx_linux_virtual_cores[core].tx_thread_smp_core_mapping_linux_thread_id = execute_thread -> tx_thread_linux_thread_id;

                    /* Clear the execution control flag.  */
                    execute_thread -> tx_thread_smp_core_control =  0;

                    /* Increment the run count for this thread.  */
                    execute_thread -> tx_thread_run_count++;

                    /* Setup time-slice, if present.  */
                    _tx_timer_time_slice[core] =  execute_thread -> tx_thread_time_slice;

                    /* Determine how the thread was last suspended.  */
                    if (execute_thread -> tx_thread_linux_suspension_type == 1)
                    {

                        /* Clear the suspension type.  */
                        execute_thread -> tx_thread_linux_suspension_type =  0;

                        /* Debug entry.  */
                        _tx_linux_debug_entry_insert("SCHEDULE-resume_thread", __FILE__, __LINE__);

                        /* Pseudo interrupt suspension.  The thread is not waiting on
                           its run semaphore.  */
                        _tx_linux_thread_resume(execute_thread -> tx_thread_linux_thread_id);
                    }
                    else if (execute_thread -> tx_thread_linux_suspension_type == 2)
                    {

                        /* Clear the suspension type.  */
                        execute_thread -> tx_thread_linux_suspension_type =  0;

                        /* Debug entry.  */
                        _tx_linux_debug_entry_insert("SCHEDULE-release_sem", __FILE__, __LINE__);

                        /* Make sure semaphore is 0. */
                        while(!sem_trywait(&execute_thread -> tx_thread_linux_thread_run_semaphore));

                        /* Let the thread run again by releasing its run semaphore.  */
                        tx_linux_sem_post(&execute_thread -> tx_thread_linux_thread_run_semaphore);

                        /* Block timer ISR. */
                        if(_tx_linux_timer_waiting)
                        {

                            /* It is woken up by timer ISR. */
                            /* Let ThreadX thread wake up first. */
                            tx_linux_sem_wait(&_tx_linux_scheduler_semaphore);

                            /* Wake up timer ISR. */
                            tx_linux_sem_post(&_tx_linux_isr_semaphore);
                        }
                        else
                        {

                            /* It is woken up by TX_THREAD. */
                            /* Suspend timer thread and let ThreadX thread wake up first. */
                            _tx_linux_thread_suspend(_tx_linux_timer_id);
                            tx_linux_sem_wait(&_tx_linux_scheduler_semaphore);
                            _tx_linux_thread_resume(_tx_linux_timer_id);

                        }
                    }
                    else
                    {

                        /* System error, increment the counter.  */
                        _tx_linux_system_error++;
                    }
                }
            }
        }

        if (preemt_retry)
        {

            /* Unlock linux mutex. */
            _tx_linux_mutex_release_all(&_tx_linux_mutex);

            /* Let user thread run to reset _tx_thread_preempt_disable. */
            _tx_linux_thread_sleep(1);

            preemt_retry = TX_FALSE;

            continue;
        }

        /* Debug entry.  */
        _tx_linux_debug_entry_insert("SCHEDULE-self_suspend_sem", __FILE__, __LINE__);

        /* Unlock linux mutex. */
        _tx_linux_mutex_release_all(&_tx_linux_mutex);

        /* Now suspend the main thread so the application thread can run.  */
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += 2000000;
        if (ts.tv_nsec >= 1000000000)
        {
            ts.tv_nsec -= 1000000000;
            ts.tv_sec++;
        }
        tx_linux_sem_timedwait(&_tx_linux_scheduler_semaphore, &ts);
        clock_gettime(CLOCK_REALTIME, &ts);
    }
}


/* Define the ThreadX Linux mutex get, release, and release all functions.  */

void _tx_linux_mutex_obtain(TX_LINUX_MUTEX *mutex)
{

TX_THREAD       *thread_ptr;
pthread_t       current_thread_id;
UINT            i;

    /* Pickup the current thread ID.  */
    current_thread_id =  pthread_self();

    /* Is the protection owned?  */
    if (mutex -> tx_linux_mutex_owner == current_thread_id)
    {

        /* Simply increment the nested counter.  */
        mutex -> tx_linux_mutex_nested_count++;
    }
    else
    {

        /* Loop to find a thread matching this ID.  */
        i =  0;
        do
        {

            /* Pickup the thread pointer.  */
            thread_ptr =  _tx_thread_current_ptr[i];

            /* Is this thread obtaining the mutex?  */
            if ((thread_ptr) && (thread_ptr -> tx_thread_linux_thread_id == current_thread_id))
            {

                /* We have found the thread, get out of the loop.  */
                break;
            }

            /* Look at next core.  */
            i++;

        } while (i < TX_THREAD_SMP_MAX_CORES);

        /* Determine if we found a thread.  */
        if (i >= TX_THREAD_SMP_MAX_CORES)
        {

            /* Set the thread pointer to NULL to indicate a thread was not found.  */
            thread_ptr =  TX_NULL;
        }

        /* If a thread was found, indicate the thread is attempting to access the mutex.  */
        if (thread_ptr)
        {

            /* Yes, current ThreadX thread attempting to get the mutex - set the flag.  */
            thread_ptr -> tx_thread_linux_mutex_access =  TX_TRUE;
        }

        /* Get the Linux mutex.  */
        pthread_mutex_lock(&mutex -> tx_linux_mutex);

        /* At this point we have the mutex.  */

        /* Clear the mutex access flag for the thread.  */
        if (thread_ptr)
        {

            /* Yes, clear the current ThreadX thread attempting to get the mutex.  */
            thread_ptr -> tx_thread_linux_mutex_access =  TX_FALSE;
        }

        /* Increment the nesting counter.  */
        mutex -> tx_linux_mutex_nested_count =  1;

        /* Remember the owner.  */
        mutex -> tx_linux_mutex_owner = pthread_self();
    }
}


void _tx_linux_mutex_release(TX_LINUX_MUTEX *mutex)
{

pthread_t   current_thread_id;


    /* Pickup the current thread ID.  */
    current_thread_id =  pthread_self();

    /* Ensure the caller is the mutex owner.  */
    if (mutex -> tx_linux_mutex_owner == current_thread_id)
    {

        /* Determine if there is protection.  */
        if (mutex -> tx_linux_mutex_nested_count)
        {

            /* Decrement the nesting counter.  */
            mutex -> tx_linux_mutex_nested_count--;

            /* Determine if the critical section is now being released.  */
            if (mutex -> tx_linux_mutex_nested_count == 0)
            {

                /* Yes, it is being released clear the owner.  */
                mutex -> tx_linux_mutex_owner =  0;

                /* Finally, release the mutex.  */
                if (pthread_mutex_unlock(&mutex -> tx_linux_mutex) != 0)
                {

                    /* Increment the system error counter.  */
                    _tx_linux_system_error++;
                }

                /* Just in case, make sure there the mutex is not owned.  */
                while (pthread_mutex_unlock(&mutex -> tx_linux_mutex) == 0)
                {

                    /* Increment the system error counter.  */
                    _tx_linux_system_error++;
                }

                /* Relinquish to other ready threads.  */
                _tx_linux_thread_sleep(1000);
            }
        }
    }
    else
    {

        /* Increment the system error counter.  */
        _tx_linux_system_error++;
    }
}


void _tx_linux_mutex_release_all(TX_LINUX_MUTEX *mutex)
{

    /* Ensure the caller is the mutex owner.  */
    if (mutex -> tx_linux_mutex_owner == pthread_self())
    {

        /* Determine if there is protection.  */
        if (mutex -> tx_linux_mutex_nested_count)
        {

            /* Clear the nesting counter.  */
            mutex -> tx_linux_mutex_nested_count =  0;

            /* Yes, it is being release clear the owner.  */
            mutex -> tx_linux_mutex_owner =  0;

            /* Finally, release the mutex.  */
            if (pthread_mutex_unlock(&mutex -> tx_linux_mutex) != 0)
            {

                /* Increment the system error counter.  */
                _tx_linux_system_error++;
            }

            /* Just in case, make sure there the mutex is not owned.  */
            while (pthread_mutex_unlock(&mutex -> tx_linux_mutex) == 0)
            {

                /* Increment the system error counter.  */
                _tx_linux_system_error++;
            }
        }
    }
    else
    {

        /* Increment the system error counter.  */
        _tx_linux_system_error++;
    }
}

void _tx_thread_delete_port_completion(TX_THREAD *thread_ptr, UINT tx_interrupt_save)
{
INT             linux_status;
sem_t           *threadrunsemaphore;
pthread_t       thread_id;
    thread_id = thread_ptr -> tx_thread_linux_thread_id;
    threadrunsemaphore = &(thread_ptr -> tx_thread_linux_thread_run_semaphore);
    _tx_thread_smp_unprotect(tx_interrupt_save);
    do
    {
        linux_status = pthread_cancel(thread_id);
        if(linux_status != EAGAIN)
        {
            break;
        }
        _tx_linux_thread_resume(thread_id);
        tx_linux_sem_post(threadrunsemaphore);
        _tx_linux_thread_sleep(1000000);
    } while (1);
    pthread_join(thread_id, NULL);
    sem_destroy(threadrunsemaphore);
    tx_interrupt_save =   _tx_thread_smp_protect();
}

void _tx_thread_reset_port_completion(TX_THREAD *thread_ptr, UINT tx_interrupt_save)
{
INT             linux_status;
sem_t           *threadrunsemaphore;
pthread_t       thread_id;
    thread_id = thread_ptr -> tx_thread_linux_thread_id;
    threadrunsemaphore = &(thread_ptr -> tx_thread_linux_thread_run_semaphore);
    _tx_thread_smp_unprotect(tx_interrupt_save);
    do
    {
        linux_status = pthread_cancel(thread_id);
        if(linux_status != EAGAIN)
        {
            break;
        }
        _tx_linux_thread_resume(thread_id);
        tx_linux_sem_post(threadrunsemaphore);
        _tx_linux_thread_sleep(1000000);
    } while (1);
    pthread_join(thread_id, NULL);
    sem_destroy(threadrunsemaphore);
    tx_interrupt_save =   _tx_thread_smp_protect();
}
