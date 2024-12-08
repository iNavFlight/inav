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

#include <winbase.h>

/* Define small routines used for the TX_DISABLE/TX_RESTORE macros.  */

UINT   _tx_thread_interrupt_disable(void)
{

UINT    previous_value;


    previous_value =  _tx_thread_interrupt_control(TX_INT_DISABLE);
    return(previous_value);
}


VOID   _tx_thread_interrupt_restore(UINT previous_posture)
{

    previous_posture =  _tx_thread_interrupt_control(previous_posture);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_thread_interrupt_control                      Win32/Visual      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function is responsible for changing the interrupt lockout     */ 
/*    posture of the system.                                              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    new_posture                           New interrupt lockout posture */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    old_posture                           Old interrupt lockout posture */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    ExitThread                            Win32 thread exit             */ 
/*    GetCurrentThread                      Win32 get current thread      */ 
/*    GetCurrentThreadId                    Win32 get current thread ID   */ 
/*    GetThreadPriority                     Win32 get thread priority     */ 
/*    _tx_win32_critical_section_obtain     Obtain critical section       */ 
/*    _tx_win32_critical_section_release    Release critical section      */ 
/*    _tx_win32_critical_section_release_all                              */ 
/*                                          Release critical section      */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT   _tx_thread_interrupt_control(UINT new_posture)
{

UINT        old_posture;
HANDLE      threadhandle;
int         threadpriority; 
DWORD       threadid;
TX_THREAD   *thread_ptr; 


    /* Enter Win32 critical section.  */
    _tx_win32_critical_section_obtain(&_tx_win32_critical_section);

#ifdef TX_WIN32_DEBUG_ENABLE

    /* Determine if this is a disable or enable request.  */
    if (new_posture == TX_INT_ENABLE)
    {
        /* Enable.  */
        _tx_win32_debug_entry_insert("RESTORE", __FILE__, __LINE__);
    }
    else
    {   
        /* Disable.  */
        _tx_win32_debug_entry_insert("DISABLE", __FILE__, __LINE__);
    }
#endif

    /* Determine if the thread was terminated.  */

    /* Pickup the handle of the current thread.  */
    threadhandle =    GetCurrentThread();

    /* Pickup the current thread pointer.  */
    thread_ptr =      _tx_thread_current_ptr;

    /* Pickup the priority of the current thread.  */
    threadpriority =  GetThreadPriority(threadhandle); 

    /* Pickup the ID of the current thread.  */
    threadid =        GetCurrentThreadId();

    /* Determine if this is a thread (THREAD_PRIORITY_LOWEST) and it does not 
       match the current thread pointer.  */
    if ((threadpriority == THREAD_PRIORITY_LOWEST) && 
        ((!thread_ptr) || (thread_ptr -> tx_thread_win32_thread_id != threadid))) 
    { 

        /* This indicates the Win32 thread was actually terminated by ThreadX is only 
           being allowed to run in order to cleanup its resources.  */
        _tx_win32_critical_section_release_all(&_tx_win32_critical_section);

        /* Exit this thread.  */
        ExitThread(0); 
    } 

    /* Determine the current interrupt lockout condition.  */
    if (_tx_win32_critical_section.tx_win32_critical_section_nested_count == 1)
    {

        /* First pass through, interrupts are enabled.  */
        old_posture =  TX_INT_ENABLE;
    }
    else
    {

        /* Interrupts are disabled.  */
        old_posture =  TX_INT_DISABLE;
    }

    /* First, determine if this call is from a non-thread.  */
    if (_tx_thread_system_state)
    {

        /* Determine how to apply the new posture.  */
        if (new_posture == TX_INT_ENABLE)
        {

            /* Clear the disabled flag.  */
            _tx_win32_global_int_disabled_flag =  TX_FALSE;

            /* Determine if the critical section is locked.  */
            _tx_win32_critical_section_release_all(&_tx_win32_critical_section);
        }
        else if (new_posture == TX_INT_DISABLE)
        {

            /* Set the disabled flag.  */
            _tx_win32_global_int_disabled_flag =  TX_TRUE;
        }
    }
    else if (thread_ptr) 
    {

        /* Determine how to apply the new posture.  */
        if (new_posture == TX_INT_ENABLE)
        {

            /* Clear the disabled flag.  */
            _tx_thread_current_ptr -> tx_thread_win32_int_disabled_flag =  TX_FALSE;

            /* Determine if the critical section is locked.  */
            _tx_win32_critical_section_release_all(&_tx_win32_critical_section);
        }
        else if (new_posture == TX_INT_DISABLE)
        {

            /* Set the disabled flag.  */
            _tx_thread_current_ptr -> tx_thread_win32_int_disabled_flag =  TX_TRUE;
        }
    }

    /* Return the previous interrupt disable posture.  */
    return(old_posture);
}

