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

#define    TX_SOURCE_CODE
#define TX_THREAD_SMP_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_thread.h"
#include "tx_timer.h"
#include <stdio.h>


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_system_return                          SMP/Linux/GCC     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is target processor specific.  It is used to transfer */
/*    control from a thread back to the system.  Only a minimal context   */
/*    is saved since the compiler assumes temp registers are going to get */
/*    slicked by a function call anyway.                                  */
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
/*    _tx_linux_debug_entry_insert                                        */
/*    _tx_linux_mutex_obtain                                              */
/*    pthread_self                                                        */
/*    pthread_getschedparam                                               */
/*    pthread_equal                                                       */
/*    _tx_linux_mutex_release_all                                         */
/*    pthread_exit                                                        */
/*    tx_linux_sem_post                                                   */
/*    sem_trywait                                                         */
/*    tx_linux_sem_wait                                                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    ThreadX components                                                  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
VOID   _tx_thread_system_return(VOID)
{

TX_THREAD   *temp_thread_ptr;
sem_t       *temp_run_semaphore;
UINT        temp_thread_state;
pthread_t   thread_id;
int         exit_code = 0;
UINT        core;


    /* Lock Linux mutex.  */
    _tx_linux_mutex_obtain(&_tx_linux_mutex);

    /* Pickup current core.  */
    core =  _tx_thread_smp_core_get();

    /* Debug entry.  */
    _tx_linux_debug_entry_insert("SYSTEM_RETURN", __FILE__, __LINE__);

    /* First, determine if the thread was terminated.  */

    /* Pickup the id of the current thread.  */
    thread_id = pthread_self();

    /* Pickup the current thread pointer.  */
    temp_thread_ptr =  _tx_thread_current_ptr[core];

    /* Determine if this is a thread (0) and it does not
       match the current thread pointer.  */
    if ((_tx_linux_threadx_thread) &&
        ((!temp_thread_ptr) || (!pthread_equal(temp_thread_ptr -> tx_thread_linux_thread_id, thread_id))))
    {

        /* This indicates the Linux thread was actually terminated by ThreadX is only
           being allowed to run in order to cleanup its resources.  */
        /* Unlock linux mutex. */
        _tx_linux_mutex_release_all(&_tx_linux_mutex);
        pthread_exit((void *)&exit_code);
    }

    /* Determine if the time-slice is active.  */
    if (_tx_timer_time_slice[core])
    {

        /* Preserve current remaining time-slice for the thread and clear the current time-slice.  */
        temp_thread_ptr -> tx_thread_time_slice =  _tx_timer_time_slice[core];
        _tx_timer_time_slice[core] =  0;
    }

    /* Save the run semaphore into a temporary variable as well.  */
    temp_run_semaphore =  &temp_thread_ptr -> tx_thread_linux_thread_run_semaphore;

    /* Pickup the current thread state.  */
    temp_thread_state =  temp_thread_ptr -> tx_thread_state;

    /* Setup the suspension type for this thread.  */
    temp_thread_ptr -> tx_thread_linux_suspension_type  =  2;

    /* Set the current thread pointer to NULL.  */
    _tx_thread_current_ptr[core] =  TX_NULL;

    /* Clear this mapping entry.  */
    _tx_linux_virtual_cores[core].tx_thread_smp_core_mapping_thread =          TX_NULL;
    _tx_linux_virtual_cores[core].tx_thread_smp_core_mapping_linux_thread_id = 0;

    /* Debug entry.  */
    _tx_linux_debug_entry_insert("SYSTEM_RETURN-release_sem", __FILE__, __LINE__);

    /* Determine if there is a system error.  */
    if (temp_thread_ptr != _tx_thread_smp_protection.tx_thread_smp_protect_thread)
    {

        /* This should not happen... increment the system error counter.  */
        _tx_linux_system_error++;
    }

    /* Clear the protection structure.  */
    _tx_thread_smp_protection.tx_thread_smp_protect_count =            0;
    _tx_thread_smp_protection.tx_thread_smp_protect_core =             0xFFFFFFFF;
    _tx_thread_smp_protection.tx_thread_smp_protect_thread =           TX_NULL;
    _tx_thread_smp_protection.tx_thread_smp_protect_in_force =         TX_FALSE;
    _tx_thread_smp_protection.tx_thread_smp_protect_linux_thread_id =  0;

    /* Indicate that this thread is now ready for scheduling again by another core.  */
    temp_thread_ptr -> tx_thread_smp_core_control =  1;

    /* Clear the interrupt disable flag.  */
    _tx_linux_global_int_disabled_flag =  TX_FALSE;

    /* Clear the preempt disable flag. */
    _tx_thread_preempt_disable =   0;

    /* Make sure semaphore is 0. */
    while(!sem_trywait(&_tx_linux_scheduler_semaphore));

    /* Release the semaphore that the main scheduling thread is waiting
       on.  Note that the main scheduling algorithm will take care of
       setting the current thread pointer to NULL.  */
    tx_linux_sem_post(&_tx_linux_scheduler_semaphore);

    /* Unlock Linux mutex.  */
    _tx_linux_mutex_release_all(&_tx_linux_mutex);

    /* Determine if the thread was self-terminating.  */
    if (temp_thread_state ==  TX_TERMINATED)
    {

        /* Exit the thread instead of waiting on the semaphore!  */
        pthread_exit((void *)&exit_code);
    }

    /* Wait on the run semaphore for this thread.  This won't get set again
       until the thread is scheduled.  */
    tx_linux_sem_wait(temp_run_semaphore);
    tx_linux_sem_post(&_tx_linux_scheduler_semaphore);

    /* Lock Linux mutex.  */
    _tx_linux_mutex_obtain(&_tx_linux_mutex);

    /* Debug entry.  */
    _tx_linux_debug_entry_insert("SYSTEM_RETURN-wake_up", __FILE__, __LINE__);

    /* Determine if the thread was terminated.  */

    /* Pickup current core.  */
    core =  _tx_thread_smp_core_get();

    /* Pickup the current thread pointer.  */
    temp_thread_ptr =  _tx_thread_current_ptr[core];

    /* Determine if this is a thread and it does not
       match the current thread pointer.  */
    if ((_tx_linux_threadx_thread) &&
        ((!temp_thread_ptr) || (!pthread_equal(temp_thread_ptr -> tx_thread_linux_thread_id, thread_id))))
    {

        /* Unlock Linux mutex.  */
        _tx_linux_mutex_release_all(&_tx_linux_mutex);

        /* This indicates the Linux thread was actually terminated by ThreadX and is only
           being allowed to run in order to cleanup its resources.  */
        pthread_exit((void *)&exit_code);
    }

    /* Now determine if the application thread last had interrupts disabled.  */

    /* Debug entry.  */
    _tx_linux_debug_entry_insert("SYSTEM_RETURN-finish", __FILE__, __LINE__);

    /* Unlock Linux mutex.  */
    _tx_linux_mutex_release_all(&_tx_linux_mutex);
}

