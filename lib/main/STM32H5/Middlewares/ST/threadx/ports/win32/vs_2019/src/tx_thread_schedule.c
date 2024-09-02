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


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_thread_schedule                               Win32/Visual      */ 
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
/*    ReleaseSemaphore                      Win32 release semaphore       */ 
/*    ResumeThread                          Win32 resume thread           */ 
/*    Sleep                                 Win32 thread sleep            */ 
/*    WaitForSingleObject                   Win32 wait on a semaphore     */ 
/*    _tx_win32_critical_section_obtain     Obtain critical section       */ 
/*    _tx_win32_critical_section_release    Release critical section      */ 
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


    /* Loop forever.  */
    while(1)
    {

        /* Wait for a thread to execute and all ISRs to complete.  */
        while(1)
        {


            /* Enter Win32 critical section.  */
            _tx_win32_critical_section_obtain(&_tx_win32_critical_section);

            /* Debug entry.  */
            _tx_win32_debug_entry_insert("SCHEDULE-wake_up", __FILE__, __LINE__);

            /* Determine if there is a thread ready to execute AND all ISRs 
               are complete.  */
            if ((_tx_thread_execute_ptr != TX_NULL) && (_tx_thread_system_state == 0))
            {

                /* Get out of this loop and schedule the thread!  */
                break;
            }
            else
            {

                /* Leave the critical section.  */
                _tx_win32_critical_section_release(&_tx_win32_critical_section);

                /* Now sleep so we don't block forever.  */
                Sleep(2);
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
        if (_tx_thread_current_ptr -> tx_thread_win32_suspension_type)
        {

            /* Debug entry.  */
            _tx_win32_debug_entry_insert("SCHEDULE-resume_thread", __FILE__, __LINE__);

            /* Pseudo interrupt suspension.  The thread is not waiting on
               its run semaphore.  */
            ResumeThread(_tx_thread_current_ptr -> tx_thread_win32_thread_handle);
        }
        else
        {

            /* Debug entry.  */
            _tx_win32_debug_entry_insert("SCHEDULE-release_sem", __FILE__, __LINE__);

            /* Let the thread run again by releasing its run semaphore.  */
            ReleaseSemaphore(_tx_thread_current_ptr -> tx_thread_win32_thread_run_semaphore, 1, NULL);
        }

        /* Debug entry.  */
        _tx_win32_debug_entry_insert("SCHEDULE-self_suspend_sem", __FILE__, __LINE__);

        /* Exit Win32 critical section.  */
        _tx_win32_critical_section_release(&_tx_win32_critical_section);

        /* Now suspend the main thread so the application thread can run.  */
        WaitForSingleObject(_tx_win32_scheduler_semaphore, INFINITE);
    }
}


/* Define the ThreadX Win32 critical section get, release, and release all functions.  */

void    _tx_win32_critical_section_obtain(TX_WIN32_CRITICAL_SECTION *critical_section)
{

TX_THREAD     *thread_ptr;


    /* Is the protection owned?  */
    if (critical_section -> tx_win32_critical_section_owner == GetCurrentThreadId())
    {
    
        /* Simply increment the nested counter.  */
        critical_section -> tx_win32_critical_section_nested_count++;
    }
    else
    {        

        /* Pickup the current thread pointer.  */
        thread_ptr =  _tx_thread_current_ptr;

        /* Get the Win32 critical section.  */
        while (WaitForSingleObject(critical_section -> tx_win32_critical_section_mutex_handle, 3) != WAIT_OBJECT_0)
        {
        }
    
        /* At this point we have the mutex.  */
    
        /* Increment the nesting counter.  */
        critical_section -> tx_win32_critical_section_nested_count =  1;
    
        /* Remember the owner.  */
        critical_section -> tx_win32_critical_section_owner =  GetCurrentThreadId();
    }
}


void    _tx_win32_critical_section_release(TX_WIN32_CRITICAL_SECTION *critical_section)
{


    /* Ensure the caller is the mutex owner.  */
    if (critical_section -> tx_win32_critical_section_owner == GetCurrentThreadId())
    {
    
        /* Determine if there is protection.  */
        if (critical_section -> tx_win32_critical_section_nested_count)
        {
    
            /* Decrement the nesting counter.  */
            critical_section -> tx_win32_critical_section_nested_count--;
    
            /* Determine if the critical section is now being released.  */
            if (critical_section -> tx_win32_critical_section_nested_count == 0)
            {
        
                /* Yes, it is being released clear the owner.  */
                critical_section -> tx_win32_critical_section_owner =  0;

                /* Finally, release the mutex.  */
                if (ReleaseMutex(critical_section -> tx_win32_critical_section_mutex_handle) != TX_TRUE)
                {
                
                    /* Increment the system error counter.  */
                    _tx_win32_system_error++;
                }

                /* Just in case, make sure there the mutex is not owned.  */
                while (ReleaseMutex(critical_section -> tx_win32_critical_section_mutex_handle) == TX_TRUE)
                {

                    /* Increment the system error counter.  */
                    _tx_win32_system_error++;
                }

                /* Sleep for 0, just to relinquish to other ready threads.  */
                Sleep(0);
			}
        }
    }
    else
    {
    
        /* Increment the system error counter.  */
        _tx_win32_system_error++;
    }
}


void    _tx_win32_critical_section_release_all(TX_WIN32_CRITICAL_SECTION    *critical_section)
{

    /* Ensure the caller is the mutex owner.  */
    if (critical_section -> tx_win32_critical_section_owner == GetCurrentThreadId())
    {
    
        /* Determine if there is protection.  */
        if (critical_section -> tx_win32_critical_section_nested_count)
        {
    
            /* Clear the nesting counter.  */
            critical_section -> tx_win32_critical_section_nested_count =  0;
    
            /* Yes, it is being release clear the owner.  */
            critical_section -> tx_win32_critical_section_owner =  0;

            /* Finally, release the mutex.  */
            if (ReleaseMutex(critical_section -> tx_win32_critical_section_mutex_handle) != TX_TRUE)
            {

                /* Increment the system error counter.  */
                _tx_win32_system_error++;
            }
            
            /* Just in case, make sure there the mutex is not owned.  */
            while (ReleaseMutex(critical_section -> tx_win32_critical_section_mutex_handle) == TX_TRUE)
            {

                /* Increment the system error counter.  */
                _tx_win32_system_error++;
            }
        }
    }
    else
    {
    
        /* Increment the system error counter.  */
        _tx_win32_system_error++;
    }
}

