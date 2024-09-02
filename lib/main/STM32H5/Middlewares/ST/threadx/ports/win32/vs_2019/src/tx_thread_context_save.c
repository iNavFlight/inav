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
/*    _tx_thread_context_save                           Win32/Visual      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function saves the context of an executing thread in the       */ 
/*    beginning of interrupt processing.  The function also ensures that  */ 
/*    the system stack is used upon return to the calling ISR.            */ 
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
/*    SuspendThread                         Win32 thread suspend          */ 
/*    _tx_win32_critical_section_obtain     Obtain critical section       */ 
/*    _tx_win32_critical_section_release    Release critical section      */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    ISRs                                                                */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
VOID   _tx_thread_context_save(VOID)
{

TX_THREAD   *thread_ptr;


    /* Enter critical section to ensure other threads are not playing with
       the core ThreadX data structures.  */
    _tx_win32_critical_section_obtain(&_tx_win32_critical_section);

    /* Debug entry.  */
    _tx_win32_debug_entry_insert("CONTEXT_SAVE", __FILE__, __LINE__);

    /* Pickup the current thread pointer.  */
    thread_ptr =  _tx_thread_current_ptr;

    /* If an application thread is running, suspend it to simulate preemption. */
    if ((thread_ptr) && (_tx_thread_system_state == 0))
    {

        /* Yes, this is the first interrupt and an application thread is running...
           suspend it!  */

        /* Suspend the thread to simulate preemption.  Note that the thread is suspended BEFORE the protection get
           flag is checked to ensure there is not a race condition between this thread and the update of that flag.  */
        SuspendThread(thread_ptr -> tx_thread_win32_thread_handle);

        /* Debug entry.  */
        _tx_win32_debug_entry_insert("CONTEXT_SAVE-suspend_thread", __FILE__, __LINE__);

    }

    /* Increment the nested interrupt condition.  */
    _tx_thread_system_state++;

    /* Exit Win32 critical section.  */
    _tx_win32_critical_section_release(&_tx_win32_critical_section);
}

