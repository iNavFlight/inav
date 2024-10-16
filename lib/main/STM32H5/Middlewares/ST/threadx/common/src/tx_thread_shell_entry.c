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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_shell_entry                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calls the specified entry function of the thread.  It */
/*    also provides a place for the thread's entry function to return.    */
/*    If the thread returns, this function places the thread in a         */
/*    "COMPLETED" state.                                                  */
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
/*    thread_entry                      Thread's entry function           */
/*    _tx_thread_system_suspend         Thread suspension routine         */
/*    _tx_thread_system_ni_suspend      Non-interruptable suspend thread  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Initial thread stack frame                                          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID  _tx_thread_shell_entry(VOID)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD       *thread_ptr;
#ifndef TX_DISABLE_NOTIFY_CALLBACKS
VOID            (*entry_exit_notify)(TX_THREAD *notify_thread_ptr, UINT type);
#endif


    /* Pickup thread pointer.  */
    TX_THREAD_GET_CURRENT(thread_ptr)

    /* Perform any additional activities for tool or user purpose.  */
    TX_THREAD_STARTED_EXTENSION(thread_ptr)

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

    /* Disable interrupts.  */
    TX_DISABLE

    /* Pickup the entry/exit application callback routine.  */
    entry_exit_notify =  thread_ptr -> tx_thread_entry_exit_notify;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Determine if an application callback routine is specified.  */
    if (entry_exit_notify != TX_NULL)
    {

        /* Yes, notify application that this thread has been entered!  */
        (entry_exit_notify)(thread_ptr, TX_THREAD_ENTRY);
    }
#endif

    /* Call current thread's entry function.  */
    (thread_ptr -> tx_thread_entry) (thread_ptr -> tx_thread_entry_parameter);

    /* Suspend thread with a "completed" state.  */

    /* Determine if the application is using mutexes.  */
    if (_tx_thread_mutex_release != TX_NULL)
    {

        /* Yes, call the mutex release function via a function pointer that
           is setup during mutex initialization.  */
        (_tx_thread_mutex_release)(thread_ptr);
    }

    /* Lockout interrupts while the thread state is setup.  */
    TX_DISABLE

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

    /* Pickup the entry/exit application callback routine again.  */
    entry_exit_notify =  thread_ptr -> tx_thread_entry_exit_notify;
#endif

    /* Set the status to suspending, in order to indicate the suspension
       is in progress.  */
    thread_ptr -> tx_thread_state =  TX_COMPLETED;

    /* Thread state change.  */
    TX_THREAD_STATE_CHANGE(thread_ptr, TX_COMPLETED)

#ifdef TX_NOT_INTERRUPTABLE

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

    /* Determine if an application callback routine is specified.  */
    if (entry_exit_notify != TX_NULL)
    {

        /* Yes, notify application that this thread has exited!  */
        (entry_exit_notify)(thread_ptr, TX_THREAD_EXIT);
    }
#endif

    /* Perform any additional activities for tool or user purpose.  */
    TX_THREAD_COMPLETED_EXTENSION(thread_ptr)

    /* Call actual non-interruptable thread suspension routine.  */
    _tx_thread_system_ni_suspend(thread_ptr, ((ULONG) 0));

    /* Restore interrupts.  */
    TX_RESTORE
#else

    /* Set the suspending flag. */
    thread_ptr -> tx_thread_suspending =  TX_TRUE;

    /* Setup for no timeout period.  */
    thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  ((ULONG) 0);

    /* Temporarily disable preemption.  */
    _tx_thread_preempt_disable++;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Perform any additional activities for tool or user purpose.  */
    TX_THREAD_COMPLETED_EXTENSION(thread_ptr)

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

    /* Determine if an application callback routine is specified.  */
    if (entry_exit_notify != TX_NULL)
    {

        /* Yes, notify application that this thread has exited!  */
        (entry_exit_notify)(thread_ptr, TX_THREAD_EXIT);
    }
#endif

    /* Call actual thread suspension routine.  */
    _tx_thread_system_suspend(thread_ptr);
#endif


#ifdef TX_SAFETY_CRITICAL

    /* If we ever get here, raise safety critical exception.  */
    TX_SAFETY_CRITICAL_EXCEPTION(__FILE__, __LINE__, 0);
#endif
}

