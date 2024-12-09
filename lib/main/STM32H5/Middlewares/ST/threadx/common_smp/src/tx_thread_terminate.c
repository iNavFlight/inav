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
#include "tx_trace.h"
#include "tx_thread.h"
#include "tx_timer.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_terminate                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles application thread terminate requests.  Once  */
/*    a thread is terminated, it cannot be executed again unless it is    */
/*    deleted and recreated.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Pointer to thread to suspend  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Return completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_timer_system_deactivate           Timer deactivate function     */
/*    _tx_thread_system_suspend             Actual thread suspension      */
/*    _tx_thread_system_ni_suspend          Non-interruptable suspend     */
/*                                            thread                      */
/*    _tx_thread_system_preempt_check       Check for preemption          */
/*    Suspend Cleanup Routine               Suspension cleanup function   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
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
UINT  _tx_thread_terminate(TX_THREAD *thread_ptr)
{

TX_INTERRUPT_SAVE_AREA

VOID        (*suspend_cleanup)(struct TX_THREAD_STRUCT *suspend_thread_ptr, ULONG suspension_sequence);
#ifndef TX_DISABLE_NOTIFY_CALLBACKS
VOID        (*entry_exit_notify)(TX_THREAD *notify_thread_ptr, UINT id);
#endif
UINT        status;
ULONG       suspension_sequence;


    /* Default to successful completion.  */
    status =  TX_SUCCESS;

    /* Lockout interrupts while the thread is being terminated.  */
    TX_DISABLE

    /* Deactivate thread timer, if active.  */
    _tx_timer_system_deactivate(&thread_ptr -> tx_thread_timer);

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_TERMINATE, thread_ptr, thread_ptr -> tx_thread_state, TX_POINTER_TO_ULONG_CONVERT(&suspend_cleanup), 0, TX_TRACE_THREAD_EVENTS)

    /* Log this kernel call.  */
    TX_EL_THREAD_TERMINATE_INSERT

    /* Is the thread already terminated?  */
    if (thread_ptr -> tx_thread_state == TX_TERMINATED)
    {

        /* Restore interrupts.  */
        TX_RESTORE

        /* Return success since thread is already terminated.  */
        status =  TX_SUCCESS;
    }

    /* Check the specified thread's current status.  */
    else if (thread_ptr -> tx_thread_state != TX_COMPLETED)
    {

        /* Disable preemption.  */
        _tx_thread_preempt_disable++;

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

        /* Pickup the entry/exit application callback routine.  */
        entry_exit_notify =  thread_ptr -> tx_thread_entry_exit_notify;
#endif

        /* Check to see if the thread is currently ready.  */
        if (thread_ptr -> tx_thread_state == TX_READY)
        {

            /* Set the state to terminated.  */
            thread_ptr -> tx_thread_state =  TX_TERMINATED;

            /* Thread state change.  */
            TX_THREAD_STATE_CHANGE(thread_ptr, TX_TERMINATED)

#ifdef TX_NOT_INTERRUPTABLE

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

            /* Determine if an application callback routine is specified.  */
            if (entry_exit_notify != TX_NULL)
            {

                /* Yes, notify application that this thread has exited!  */
                (entry_exit_notify)(thread_ptr, TX_THREAD_EXIT);
            }
#endif

            /* Call actual non-interruptable thread suspension routine.  */
            _tx_thread_system_ni_suspend(thread_ptr, ((ULONG) 0));
#else

            /* Set the suspending flag.  */
            thread_ptr -> tx_thread_suspending =  TX_TRUE;

            /* Setup for no timeout period.  */
            thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  ((ULONG) 0);

            /* Disable preemption.  */
            _tx_thread_preempt_disable++;

            /* Since the thread is currently ready, we don't need to
               worry about calling the suspend cleanup routine!  */

            /* Restore interrupts.  */
            TX_RESTORE

            /* Perform any additional activities for tool or user purpose.  */
            TX_THREAD_TERMINATED_EXTENSION(thread_ptr)

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

            /* Disable interrupts.  */
            TX_DISABLE
#endif
        }
        else
        {

            /* Change the state to terminated.  */
            thread_ptr -> tx_thread_state =    TX_TERMINATED;

            /* Thread state change.  */
            TX_THREAD_STATE_CHANGE(thread_ptr, TX_TERMINATED)

            /* Set the suspending flag.  This prevents the thread from being
               resumed before the cleanup routine is executed.  */
            thread_ptr -> tx_thread_suspending =  TX_TRUE;

            /* Pickup the cleanup routine address.  */
            suspend_cleanup =  thread_ptr -> tx_thread_suspend_cleanup;

#ifndef TX_NOT_INTERRUPTABLE

            /* Pickup the suspension sequence number that is used later to verify that the
               cleanup is still necessary.  */
            suspension_sequence =  thread_ptr -> tx_thread_suspension_sequence;
#else

            /* When not interruptable is selected, the suspension sequence is not used - just set to 0.  */
            suspension_sequence =  ((ULONG) 0);
#endif

#ifndef TX_NOT_INTERRUPTABLE

            /* Restore interrupts.  */
            TX_RESTORE
#endif

            /* Call any cleanup routines.  */
            if (suspend_cleanup != TX_NULL)
            {

                /* Yes, there is a function to call.  */
                (suspend_cleanup)(thread_ptr, suspension_sequence);
            }

#ifndef TX_NOT_INTERRUPTABLE

            /* Disable interrupts.  */
            TX_DISABLE
#endif

            /* Clear the suspending flag.  */
            thread_ptr -> tx_thread_suspending =  TX_FALSE;

#ifndef TX_NOT_INTERRUPTABLE

            /* Restore interrupts.  */
            TX_RESTORE
#endif

            /* Perform any additional activities for tool or user purpose.  */
            TX_THREAD_TERMINATED_EXTENSION(thread_ptr)

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

            /* Determine if an application callback routine is specified.  */
            if (entry_exit_notify != TX_NULL)
            {

                /* Yes, notify application that this thread has exited!  */
                (entry_exit_notify)(thread_ptr, TX_THREAD_EXIT);
            }
#endif

#ifndef TX_NOT_INTERRUPTABLE

            /* Disable interrupts.  */
            TX_DISABLE
#endif
        }

#ifndef TX_NOT_INTERRUPTABLE

        /* Restore interrupts.  */
        TX_RESTORE
#endif

        /* Determine if the application is using mutexes.  */
        if (_tx_thread_mutex_release != TX_NULL)
        {

            /* Yes, call the mutex release function via a function pointer that
               is setup during initialization.  */
            (_tx_thread_mutex_release)(thread_ptr);
        }

#ifndef TX_NOT_INTERRUPTABLE

        /* Disable interrupts.  */
        TX_DISABLE
#endif

        /* Enable preemption.  */
        _tx_thread_preempt_disable--;

        /* Restore interrupts.  */
        TX_RESTORE
    }
    else
    {

        /* Restore interrupts.  */
        TX_RESTORE
    }

    /* Check for preemption.  */
    _tx_thread_system_preempt_check();

    /* Return completion status.  */
    return(status);
}

