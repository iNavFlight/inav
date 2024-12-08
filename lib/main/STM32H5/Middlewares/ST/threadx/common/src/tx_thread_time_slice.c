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

#ifndef TX_NO_TIMER

/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_timer.h"
#include "tx_thread.h"
#include "tx_trace.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_time_slice                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function moves the currently executing thread to the end of    */
/*    the threads ready at the same priority level as a result of a       */
/*    time-slice interrupt.  If no other thread of the same priority is   */
/*    ready, this function simply returns.                                */
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
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_timer_interrupt                   Timer interrupt handling      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     Scott Larson             Modified comment(s), and      */
/*                                            opt out of function when    */
/*                                            TX_NO_TIMER is defined,     */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID  _tx_thread_time_slice(VOID)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD       *thread_ptr;
#ifdef TX_ENABLE_STACK_CHECKING
TX_THREAD       *next_thread_ptr;
#endif
#ifdef TX_ENABLE_EVENT_TRACE
ULONG           system_state;
UINT            preempt_disable;
#endif

    /* Pickup thread pointer.  */
    TX_THREAD_GET_CURRENT(thread_ptr)

#ifdef TX_ENABLE_STACK_CHECKING

    /* Check this thread's stack.  */
    TX_THREAD_STACK_CHECK(thread_ptr)

    /* Set the next thread pointer to NULL.  */
    next_thread_ptr =  TX_NULL;
#endif

    /* Lockout interrupts while the time-slice is evaluated.  */
    TX_DISABLE

    /* Clear the expired time-slice flag.  */
    _tx_timer_expired_time_slice =  TX_FALSE;

    /* Make sure the thread pointer is valid.  */
    if (thread_ptr != TX_NULL)
    {

        /* Make sure the thread is still active, i.e. not suspended.  */
        if (thread_ptr -> tx_thread_state == TX_READY)
        {

            /* Setup a fresh time-slice for the thread.  */
            thread_ptr -> tx_thread_time_slice =  thread_ptr -> tx_thread_new_time_slice;

            /* Reset the actual time-slice variable.  */
            _tx_timer_time_slice =  thread_ptr -> tx_thread_time_slice;

            /* Determine if there is another thread at the same priority and preemption-threshold
               is not set.  Preemption-threshold overrides time-slicing.  */
            if (thread_ptr -> tx_thread_ready_next != thread_ptr)
            {

                /* Check to see if preemption-threshold is not being used.  */
                if (thread_ptr -> tx_thread_priority == thread_ptr -> tx_thread_preempt_threshold)
                {

                    /* Preemption-threshold is not being used by this thread.  */

                    /* There is another thread at this priority, make it the highest at
                       this priority level.  */
                    _tx_thread_priority_list[thread_ptr -> tx_thread_priority] =  thread_ptr -> tx_thread_ready_next;

                    /* Designate the highest priority thread as the one to execute.  Don't use this
                       thread's priority as an index just in case a higher priority thread is now
                       ready!  */
                    _tx_thread_execute_ptr =  _tx_thread_priority_list[_tx_thread_highest_priority];

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

                    /* Increment the thread's time-slice counter.  */
                    thread_ptr -> tx_thread_performance_time_slice_count++;

                    /* Increment the total number of thread time-slice operations.  */
                    _tx_thread_performance_time_slice_count++;
#endif


#ifdef TX_ENABLE_STACK_CHECKING

                    /* Pickup the next execute pointer.  */
                    next_thread_ptr =  _tx_thread_execute_ptr;
#endif
                }
            }
        }
    }

#ifdef TX_ENABLE_EVENT_TRACE

    /* Pickup the volatile information.  */
    system_state =  TX_THREAD_GET_SYSTEM_STATE();
    preempt_disable =  _tx_thread_preempt_disable;

    /* Insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_TIME_SLICE, _tx_thread_execute_ptr, system_state, preempt_disable, TX_POINTER_TO_ULONG_CONVERT(&thread_ptr), TX_TRACE_INTERNAL_EVENTS)
#endif

    /* Restore previous interrupt posture.  */
    TX_RESTORE

#ifdef TX_ENABLE_STACK_CHECKING

    /* Determine if there is a next thread pointer to perform stack checking on.  */
    if (next_thread_ptr != TX_NULL)
    {

        /* Yes, check this thread's stack.  */
        TX_THREAD_STACK_CHECK(next_thread_ptr)
    }
#endif
}

#endif
