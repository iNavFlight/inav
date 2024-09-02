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
#ifndef TX_NO_TIMER
#include "tx_timer.h"
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_relinquish                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function moves the currently executing thread to the end of    */
/*    the list of threads ready at the same priority. If no other threads */
/*    of the same or higher priority are ready, this function simply      */
/*    returns.                                                            */
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
/*    _tx_thread_system_return              Return to the system          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
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
VOID  _tx_thread_relinquish(VOID)
{

TX_INTERRUPT_SAVE_AREA

UINT            priority;
TX_THREAD       *thread_ptr;


    /* Pickup thread pointer.  */
    TX_THREAD_GET_CURRENT(thread_ptr)

#ifdef TX_ENABLE_STACK_CHECKING

    /* Check this thread's stack.  */
    TX_THREAD_STACK_CHECK(thread_ptr)
#endif

    /* Disable interrupts.  */
    TX_DISABLE

#ifndef TX_NO_TIMER

    /* Reset time slice for current thread.  */
    _tx_timer_time_slice =  thread_ptr -> tx_thread_new_time_slice;
#endif

    /* Pickup the thread's priority.  */
    priority =  thread_ptr -> tx_thread_priority;

    /* Determine if there is another thread at the same priority.  */
    if (thread_ptr -> tx_thread_ready_next != thread_ptr)
    {

        /* Yes, there is another thread at this priority, make it the highest at
           this priority level.  */
        _tx_thread_priority_list[priority] =  thread_ptr -> tx_thread_ready_next;

        /* Mark the new thread as the one to execute.  */
        _tx_thread_execute_ptr = thread_ptr -> tx_thread_ready_next;
    }

    /* Determine if there is a higher-priority thread ready.  */
    if (_tx_thread_highest_priority < priority)
    {

        /* Yes, there is a higher priority thread ready to execute.  Make
           it visible to the thread scheduler.  */
        _tx_thread_execute_ptr =  _tx_thread_priority_list[_tx_thread_highest_priority];

        /* No need to clear the preempted bit in this case, since the currently running
           thread must already have its preempted bit clear.  */
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_RELINQUISH, &thread_ptr, TX_POINTER_TO_ULONG_CONVERT(_tx_thread_execute_ptr), 0, 0, TX_TRACE_THREAD_EVENTS)

    /* Log this kernel call.  */
    TX_EL_THREAD_RELINQUISH_INSERT

    /* Restore previous interrupt posture.  */
    TX_RESTORE

    /* Determine if this thread needs to return to the system.  */
    if (_tx_thread_execute_ptr != thread_ptr)
    {

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

        /* Increment the number of thread relinquishes.  */
        thread_ptr -> tx_thread_performance_relinquish_count++;

        /* Increment the total number of thread relinquish operations.  */
        _tx_thread_performance_relinquish_count++;

        /* Increment the non-idle return count.  */
        _tx_thread_performance_non_idle_return_count++;
#endif

#ifdef TX_ENABLE_STACK_CHECKING

        /* Pickup the next execute pointer.  */
        thread_ptr =  _tx_thread_execute_ptr;

        /* Check this thread's stack.  */
        TX_THREAD_STACK_CHECK(thread_ptr)
#endif

        /* Transfer control to the system so the scheduler can execute
           the next thread.  */
        _tx_thread_system_return();
    }
}

