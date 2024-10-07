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


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_thread.h"
#include "tx_timer.h"
#include <stdio.h>


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_thread_system_return                            Linux/GNU       */ 
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
/*    tx_linux_mutex_lock                                                 */ 
/*    pthread_self                                                        */ 
/*    pthread_getschedparam                                               */ 
/*    pthread_equal                                                       */ 
/*    tx_linux_mutex_recursive_unlock                                     */ 
/*    tx_linux_mutex_unlock                                               */ 
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


    /* Debug entry.  */
    _tx_linux_debug_entry_insert("SYSTEM_RETURN", __FILE__, __LINE__);

    /* Lock Linux mutex.  */
    tx_linux_mutex_lock(_tx_linux_mutex);

    /* First, determine if the thread was terminated.  */

    /* Pickup the id of the current thread.  */
    thread_id = pthread_self();

    /* Pickup the current thread pointer.  */
    temp_thread_ptr =  _tx_thread_current_ptr;

    /* Determine if this is a thread (0) and it does not 
       match the current thread pointer.  */
    if ((_tx_linux_threadx_thread) && 
        ((!temp_thread_ptr) || (!pthread_equal(temp_thread_ptr -> tx_thread_linux_thread_id, thread_id)))) 
    { 

        /* This indicates the Linux thread was actually terminated by ThreadX is only 
           being allowed to run in order to cleanup its resources.  */
        /* Unlock linux mutex. */
        tx_linux_mutex_recursive_unlock(_tx_linux_mutex);
        pthread_exit((void *)&exit_code);
    }

    /* Determine if the time-slice is active.  */
    if (_tx_timer_time_slice)
    {

        /* Preserve current remaining time-slice for the thread and clear the current time-slice.  */
        temp_thread_ptr -> tx_thread_time_slice =  _tx_timer_time_slice;
        _tx_timer_time_slice =  0;
    }

    /* Save the run semaphore into a temporary variable as well.  */
    temp_run_semaphore =  &temp_thread_ptr -> tx_thread_linux_thread_run_semaphore;

    /* Pickup the current thread state.  */
    temp_thread_state =  temp_thread_ptr -> tx_thread_state;

    /* Setup the suspension type for this thread.  */
    temp_thread_ptr -> tx_thread_linux_suspension_type  =  0;

    /* Set the current thread pointer to NULL.  */
    _tx_thread_current_ptr =  TX_NULL;

    /* Unlock Linux mutex.  */
    tx_linux_mutex_recursive_unlock(_tx_linux_mutex);

    /* Debug entry.  */
    _tx_linux_debug_entry_insert("SYSTEM_RETURN-release_sem", __FILE__, __LINE__);

    /* Make sure semaphore is 0. */
    while(!sem_trywait(&_tx_linux_semaphore));

    /* Release the semaphore that the main scheduling thread is waiting
       on.  Note that the main scheduling algorithm will take care of
       setting the current thread pointer to NULL.  */
    tx_linux_sem_post(&_tx_linux_semaphore);

    /* Determine if the thread was self-terminating.  */
    if (temp_thread_state ==  TX_TERMINATED)
    {

        /* Exit the thread instead of waiting on the semaphore!  */
        pthread_exit((void *)&exit_code);
    }

    /* Wait on the run semaphore for this thread.  This won't get set again
       until the thread is scheduled.  */
    tx_linux_sem_wait(temp_run_semaphore);
    tx_linux_sem_post_nolock(&_tx_linux_semaphore);

    /* Lock Linux mutex.  */
    tx_linux_mutex_lock(_tx_linux_mutex);

    /* Debug entry.  */
    _tx_linux_debug_entry_insert("SYSTEM_RETURN-wake_up", __FILE__, __LINE__);

    /* Determine if the thread was terminated.  */

    /* Pickup the current thread pointer.  */
    temp_thread_ptr =  _tx_thread_current_ptr;

    /* Determine if this is a thread and it does not 
       match the current thread pointer.  */
    if ((_tx_linux_threadx_thread) && 
        ((!temp_thread_ptr) || (!pthread_equal(temp_thread_ptr -> tx_thread_linux_thread_id, thread_id)))) 
    { 

        /* Unlock Linux mutex.  */
        tx_linux_mutex_recursive_unlock(_tx_linux_mutex);

        /* This indicates the Linux thread was actually terminated by ThreadX and is only 
           being allowed to run in order to cleanup its resources.  */
        pthread_exit((void *)&exit_code);
    } 

    /* Now determine if the application thread last had interrupts disabled.  */

    /* Determine if this thread had interrupts disabled.  */
    if (!_tx_thread_current_ptr -> tx_thread_linux_int_disabled_flag)
    {

        /* Unlock Linux mutex.  */
        tx_linux_mutex_recursive_unlock(_tx_linux_mutex);
    }

    /* Debug entry.  */
    _tx_linux_debug_entry_insert("SYSTEM_RETURN-finish", __FILE__, __LINE__);
}

