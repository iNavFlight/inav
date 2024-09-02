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
/*    _tx_thread_system_return                          Win32/Visual      */ 
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
/*    _tx_win32_critical_section_obtain     Obtain critical section       */ 
/*    _tx_win32_critical_section_release    Release critical section      */ 
/*    _tx_win32_critical_section_release_all                              */ 
/*                                          Release critical section      */ 
/*    ExitThread                            Win32 thread exit             */ 
/*    GetCurrentThread                      Win32 get current thread      */ 
/*    GetCurrentThreadId                    Win32 get current thread ID   */ 
/*    GetThreadPriority                     Win32 get thread priority     */ 
/*    ReleaseSemaphore                      Win32 release semaphore       */ 
/*    WaitForSingleObject                   Win32 wait on semaphore       */ 
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
HANDLE      temp_run_semaphore;
UINT        temp_thread_state;
HANDLE      threadhandle;
int         threadpriority; 
DWORD       threadid;


    /* Enter Win32 critical section.  */
    _tx_win32_critical_section_obtain(&_tx_win32_critical_section);

    /* Pickup the handle of the current thread.  */
    threadhandle =  GetCurrentThread();

    /* Debug entry.  */
    _tx_win32_debug_entry_insert("SYSTEM_RETURN", __FILE__, __LINE__);

    /* First, determine if the thread was terminated.  */

    /* Pickup the priority of the current thread.  */
    threadpriority =  GetThreadPriority(threadhandle); 

    /* Pickup the ID of the current thread.  */
    threadid =  GetCurrentThreadId();

    /* Pickup the current thread pointer.  */
    temp_thread_ptr =  _tx_thread_current_ptr;

    /* Determine if this is a thread (THREAD_PRIORITY_LOWEST) and it does not 
       match the current thread pointer.  */
    if ((threadpriority == THREAD_PRIORITY_LOWEST) && 
        ((!temp_thread_ptr) || (temp_thread_ptr -> tx_thread_win32_thread_id != threadid))) 
    { 

        /* This indicates the Win32 thread was actually terminated by ThreadX and is only 
           being allowed to run in order to cleanup its resources.  */
        
        /* Release critical section.  */
        _tx_win32_critical_section_release_all(&_tx_win32_critical_section);
        
        /* Exit thread.  */
        ExitThread(0); 
    } 

    /* Determine if the time-slice is active.  */
    if (_tx_timer_time_slice)
    {

        /* Preserve current remaining time-slice for the thread and clear the current time-slice.  */
        temp_thread_ptr -> tx_thread_time_slice =  _tx_timer_time_slice;
        _tx_timer_time_slice =  0;
    }

    /* Save the run semaphore into a temporary variable as well.  */
    temp_run_semaphore =  temp_thread_ptr -> tx_thread_win32_thread_run_semaphore;

    /* Pickup the current thread state.  */
    temp_thread_state =  temp_thread_ptr -> tx_thread_state;

    /* Setup the suspension type for this thread.  */
    temp_thread_ptr -> tx_thread_win32_suspension_type  =  0;

    /* Set the current thread pointer to NULL.  */
    _tx_thread_current_ptr =  TX_NULL;

    /* Debug entry.  */
    _tx_win32_debug_entry_insert("SYSTEM_RETURN-release_sem", __FILE__, __LINE__);

    /* Release the semaphore that the main scheduling thread is waiting
       on.  Note that the main scheduling algorithm will take care of
       setting the current thread pointer to NULL.  */
    ReleaseSemaphore(_tx_win32_scheduler_semaphore, 1, NULL);

    /* Leave Win32 critical section.  */
    _tx_win32_critical_section_release_all(&_tx_win32_critical_section);

    /* Determine if the thread was self-terminating.  */
    if (temp_thread_state ==  TX_TERMINATED)
    {

        /* Exit the thread instead of waiting on the semaphore!  */
        ExitThread(0);
    }

    /* Wait on the run semaphore for this thread.  This won't get set again
       until the thread is scheduled.  */
    WaitForSingleObject(temp_run_semaphore, INFINITE);

    /* Enter Win32 critical section.  */
    _tx_win32_critical_section_obtain(&_tx_win32_critical_section);

    /* Debug entry.  */
    _tx_win32_debug_entry_insert("SYSTEM_RETURN-wake_up", __FILE__, __LINE__);

    /* Determine if the thread was terminated.  */

    /* Pickup the current thread pointer.  */
    temp_thread_ptr =  _tx_thread_current_ptr;

    /* Determine if this is a thread (THREAD_PRIORITY_LOWEST) and it does not 
       match the current thread pointer.  */
    if ((threadpriority == THREAD_PRIORITY_LOWEST) && 
        ((!temp_thread_ptr) || (temp_thread_ptr -> tx_thread_win32_thread_id != threadid))) 
    { 

        /* Leave Win32 critical section.  */
        _tx_win32_critical_section_release_all(&_tx_win32_critical_section);

        /* This indicates the Win32 thread was actually terminated by ThreadX and is only 
           being allowed to run in order to cleanup its resources.  */
        ExitThread(0); 
    } 

    /* Now determine if the application thread last had interrupts disabled.  */

    /* Debug entry.  */
    _tx_win32_debug_entry_insert("SYSTEM_RETURN-finish", __FILE__, __LINE__);

    /* Determine if this thread had interrupts disabled.  */
    if (!_tx_thread_current_ptr -> tx_thread_win32_int_disabled_flag)
    {

        /* Leave Win32 critical section.  */
        _tx_win32_critical_section_release(&_tx_win32_critical_section);
    }
}

