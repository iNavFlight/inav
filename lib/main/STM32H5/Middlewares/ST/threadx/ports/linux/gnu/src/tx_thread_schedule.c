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
#include "tx_thread.h"
#include "tx_timer.h"
#include <stdio.h>
#include <errno.h>

extern sem_t _tx_linux_timer_semaphore;
extern sem_t _tx_linux_isr_semaphore;
extern UINT _tx_linux_timer_waiting;
extern pthread_t _tx_linux_timer_id;
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_schedule                                 Linux/GNU       */ 
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
/*    tx_linux_mutex_lock                                                 */
/*    tx_linux_mutex_unlock                                               */
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
struct timespec ts;

    /* Set timer. */
    ts.tv_sec = 0;
    ts.tv_nsec = 200000;

    /* Loop forever.  */
    while(1)
    {

        /* Wait for a thread to execute and all ISRs to complete.  */
        while(1)
        {

            /* Lock Linux mutex.  */
            tx_linux_mutex_lock(_tx_linux_mutex);

            /* Determine if there is a thread ready to execute AND all ISRs
               are complete.  */
            if ((_tx_thread_execute_ptr != TX_NULL) && (_tx_thread_system_state == 0))
            {

                /* Get out of this loop and schedule the thread!  */
                break;
            }
            else
            {

                /* Unlock linux mutex. */
                tx_linux_mutex_unlock(_tx_linux_mutex);

                /* Don't waste all the processor time here in the master thread...  */
#ifdef TX_LINUX_NO_IDLE_ENABLE
                while(!sem_trywait(&_tx_linux_timer_semaphore));
                tx_linux_sem_post(&_tx_linux_timer_semaphore);
                /*nanosleep(&ts, &ts);*/

                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_nsec += 200000;
                if (ts.tv_nsec > 1000000000)
                {
                    ts.tv_nsec -= 1000000000;
                    ts.tv_sec++;
                }
                sem_timedwait(&_tx_linux_semaphore_no_idle, &ts);
#else                
                nanosleep(&ts, &ts);
#endif /* TX_LINUX_NO_IDLE_ENABLE */                
            }
        }

        /* Yes! We have a thread to execute. Note that the critical section is already
           active from the scheduling loop above.  */

        /* Setup the current thread pointer.  */
        _tx_thread_current_ptr =  _tx_thread_execute_ptr;

        /* Increment the run count for this thread.  */
        _tx_thread_current_ptr -> tx_thread_run_count++;

        /* Setup time-slice, if present.  */
        _tx_timer_time_slice =  _tx_thread_current_ptr -> tx_thread_time_slice;

        /* Determine how the thread was suspended.  */
        if (_tx_thread_current_ptr -> tx_thread_linux_suspension_type)
        {

            /* Debug entry.  */
            _tx_linux_debug_entry_insert("SCHEDULE-resume_thread", __FILE__, __LINE__);

            /* Pseudo interrupt suspension.  The thread is not waiting on
               its run semaphore.  */
            _tx_linux_thread_resume(_tx_thread_current_ptr -> tx_thread_linux_thread_id);
        }
        else
        {

            /* Debug entry.  */
            _tx_linux_debug_entry_insert("SCHEDULE-release_sem", __FILE__, __LINE__);

            /* Make sure semaphore is 0. */
            while(!sem_trywait(&_tx_thread_current_ptr -> tx_thread_linux_thread_run_semaphore));

            /* Let the thread run again by releasing its run semaphore.  */
            tx_linux_sem_post(&_tx_thread_current_ptr -> tx_thread_linux_thread_run_semaphore);

            /* Block timer ISR. */
            if(_tx_linux_timer_waiting)
            {

                /* It is woken up by timer ISR. */
                /* Let ThreadX thread wake up first. */
                tx_linux_sem_wait(&_tx_linux_semaphore);

                /* Wake up timer ISR. */
                tx_linux_sem_post_nolock(&_tx_linux_isr_semaphore);
            }
            else
            {

                /* It is woken up by TX_THREAD. */
                /* Suspend timer thread and let ThreadX thread wake up first. */
                _tx_linux_thread_suspend(_tx_linux_timer_id);
                tx_linux_sem_wait(&_tx_linux_semaphore);
                _tx_linux_thread_resume(_tx_linux_timer_id);

            }
        }

        /* Unlock linux mutex. */
        tx_linux_mutex_unlock(_tx_linux_mutex);

        /* Debug entry.  */
        _tx_linux_debug_entry_insert("SCHEDULE-self_suspend_sem", __FILE__, __LINE__);

        /* Now suspend the main thread so the application thread can run.  */
        tx_linux_sem_wait(&_tx_linux_semaphore);

        /* Debug entry.  */
        _tx_linux_debug_entry_insert("SCHEDULE-wake_up", __FILE__, __LINE__);

    }
}

void _tx_thread_delete_port_completion(TX_THREAD *thread_ptr, UINT tx_saved_posture)
{
INT             linux_status;
sem_t           *threadrunsemaphore;
pthread_t       thread_id;
struct          timespec ts;

    thread_id = thread_ptr -> tx_thread_linux_thread_id;
    threadrunsemaphore = &(thread_ptr -> tx_thread_linux_thread_run_semaphore);
    ts.tv_sec = 0;
    ts.tv_nsec = 1000000;
    TX_RESTORE
    do
    {
        linux_status = pthread_cancel(thread_id);
        if(linux_status != EAGAIN)
        {
            break;
        }
        _tx_linux_thread_resume(thread_id);
        tx_linux_sem_post(threadrunsemaphore);
        nanosleep(&ts, &ts);
    } while (1);
    pthread_join(thread_id, NULL);
    sem_destroy(threadrunsemaphore);
    TX_DISABLE
}

void _tx_thread_reset_port_completion(TX_THREAD *thread_ptr, UINT tx_saved_posture)
{
INT             linux_status;
sem_t           *threadrunsemaphore;
pthread_t       thread_id;
struct          timespec ts;

    thread_id = thread_ptr -> tx_thread_linux_thread_id;
    threadrunsemaphore = &(thread_ptr -> tx_thread_linux_thread_run_semaphore);
    ts.tv_sec = 0;
    ts.tv_nsec = 1000000;
    TX_RESTORE
    do
    {
        linux_status = pthread_cancel(thread_id);
        if(linux_status != EAGAIN)
        {
            break;
        }
        _tx_linux_thread_resume(thread_id);
        tx_linux_sem_post(threadrunsemaphore);
        nanosleep(&ts, &ts);
    } while (1);
    pthread_join(thread_id, NULL);
    sem_destroy(threadrunsemaphore);
    TX_DISABLE
}
