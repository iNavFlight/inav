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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_wait_abort                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function aborts the wait condition that the specified thread   */
/*    is in - regardless of what object the thread is waiting on - and    */
/*    returns a TX_WAIT_ABORTED status to the specified thread.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Thread to abort the wait on   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Return completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    Suspension Cleanup Functions                                        */
/*    _tx_thread_system_resume                                            */
/*    _tx_thread_system_ni_resume       Non-interruptable resume thread   */
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
UINT  _tx_thread_wait_abort(TX_THREAD  *thread_ptr)
{

TX_INTERRUPT_SAVE_AREA

VOID            (*suspend_cleanup)(struct TX_THREAD_STRUCT *suspend_thread_ptr, ULONG suspension_sequence);
UINT            status;
ULONG           suspension_sequence;


    /* Disable interrupts.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_WAIT_ABORT, thread_ptr, thread_ptr -> tx_thread_state, 0, 0, TX_TRACE_THREAD_EVENTS)

    /* Log this kernel call.  */
    TX_EL_THREAD_WAIT_ABORT_INSERT

    /* Determine if the thread is currently suspended.  */
    if (thread_ptr -> tx_thread_state < TX_SLEEP)
    {

        /* Thread is either ready, completed, terminated, or in a pure
           suspension condition.  */

        /* Restore interrupts.  */
        TX_RESTORE

        /* Just return with an error message to indicate that
           nothing was done.  */
        status =  TX_WAIT_ABORT_ERROR;
    }
    else
    {

        /* Check for a sleep condition.  */
        if (thread_ptr -> tx_thread_state == TX_SLEEP)
        {

            /* Set the state to terminated.  */
            thread_ptr -> tx_thread_state =    TX_SUSPENDED;

            /* Set the TX_WAIT_ABORTED status in the thread that is
               sleeping.  */
            thread_ptr -> tx_thread_suspend_status =  TX_WAIT_ABORTED;

            /* Make sure there isn't a suspend cleanup routine.  */
            thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

#ifndef TX_NOT_INTERRUPTABLE

            /* Increment the disable preemption flag.  */
            _tx_thread_preempt_disable++;

            /* Restore interrupts.  */
            TX_RESTORE
#endif
        }
        else
        {

            /* Process all other suspension timeouts.  */

            /* Set the state to suspended.  */
            thread_ptr -> tx_thread_state =    TX_SUSPENDED;

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

            /* Set the TX_WAIT_ABORTED status in the thread that was
               suspended.  */
            thread_ptr -> tx_thread_suspend_status =  TX_WAIT_ABORTED;

#ifndef TX_NOT_INTERRUPTABLE

            /* Increment the disable preemption flag.  */
            _tx_thread_preempt_disable++;

            /* Restore interrupts.  */
            TX_RESTORE
#endif

            /* Call any cleanup routines.  */
            if (suspend_cleanup != TX_NULL)
            {

                /* Yes, there is a function to call.  */
                (suspend_cleanup)(thread_ptr, suspension_sequence);
            }
        }

        /* If the abort of the thread wait was successful, if so resume the thread.  */
        if (thread_ptr -> tx_thread_suspend_status == TX_WAIT_ABORTED)
        {

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

            /* Increment the total number of thread wait aborts.  */
            _tx_thread_performance_wait_abort_count++;

            /* Increment this thread's wait abort count.  */
            thread_ptr -> tx_thread_performance_wait_abort_count++;
#endif

#ifdef TX_NOT_INTERRUPTABLE

            /* Resume the thread!  */
            _tx_thread_system_ni_resume(thread_ptr);

            /* Restore interrupts.  */
            TX_RESTORE
#else

            /* Lift the suspension on the previously waiting thread.  */
            _tx_thread_system_resume(thread_ptr);
#endif

            /* Return a successful status.  */
            status =  TX_SUCCESS;
        }
        else
        {

#ifdef TX_NOT_INTERRUPTABLE

            /* Restore interrupts.  */
            TX_RESTORE

#else

            /* Disable interrupts.  */
            TX_DISABLE

            /* Decrement the disable preemption flag.  */
            _tx_thread_preempt_disable--;

            /* Restore interrupts.  */
            TX_RESTORE
#endif

            /* Return with an error message to indicate that
               nothing was done.  */
            status =  TX_WAIT_ABORT_ERROR;
        }
    }

    /* Return completion status.  */
    return(status);
}

