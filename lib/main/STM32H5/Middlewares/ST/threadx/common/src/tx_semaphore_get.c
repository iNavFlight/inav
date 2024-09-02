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
/**   Semaphore                                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_thread.h"
#include "tx_semaphore.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_semaphore_get                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets an instance from the specified counting          */
/*    semaphore.                                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    semaphore_ptr                     Pointer to semaphore control block*/
/*    wait_option                       Suspension option                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_suspend         Suspend thread service            */
/*    _tx_thread_system_ni_suspend      Non-interruptable suspend thread  */
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
UINT  _tx_semaphore_get(TX_SEMAPHORE *semaphore_ptr, ULONG wait_option)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD       *thread_ptr;
TX_THREAD       *next_thread;
TX_THREAD       *previous_thread;
UINT            status;


    /* Default the status to TX_SUCCESS.  */
    status =  TX_SUCCESS;

    /* Disable interrupts to get an instance from the semaphore.  */
    TX_DISABLE

#ifdef TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO

    /* Increment the total semaphore get counter.  */
    _tx_semaphore_performance_get_count++;

    /* Increment the number of attempts to get this semaphore.  */
    semaphore_ptr -> tx_semaphore_performance_get_count++;
#endif

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_SEMAPHORE_GET, semaphore_ptr, wait_option, semaphore_ptr -> tx_semaphore_count, TX_POINTER_TO_ULONG_CONVERT(&thread_ptr), TX_TRACE_SEMAPHORE_EVENTS)

    /* Log this kernel call.  */
    TX_EL_SEMAPHORE_GET_INSERT

    /* Determine if there is an instance of the semaphore.  */
    if (semaphore_ptr -> tx_semaphore_count != ((ULONG) 0))
    {

        /* Decrement the semaphore count.  */
        semaphore_ptr -> tx_semaphore_count--;

        /* Restore interrupts.  */
        TX_RESTORE
    }

    /* Determine if the request specifies suspension.  */
    else if (wait_option != TX_NO_WAIT)
    {

        /* Determine if the preempt disable flag is non-zero.  */
        if (_tx_thread_preempt_disable != ((UINT) 0))
        {

            /* Restore interrupts.  */
            TX_RESTORE

            /* Suspension is not allowed if the preempt disable flag is non-zero at this point - return error completion.  */
            status =  TX_NO_INSTANCE;
        }
        else
        {

            /* Prepare for suspension of this thread.  */

#ifdef TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO

            /* Increment the total semaphore suspensions counter.  */
            _tx_semaphore_performance_suspension_count++;

            /* Increment the number of suspensions on this semaphore.  */
            semaphore_ptr -> tx_semaphore_performance_suspension_count++;
#endif

            /* Pickup thread pointer.  */
            TX_THREAD_GET_CURRENT(thread_ptr)

            /* Setup cleanup routine pointer.  */
            thread_ptr -> tx_thread_suspend_cleanup =  &(_tx_semaphore_cleanup);

            /* Setup cleanup information, i.e. this semaphore control
               block.  */
            thread_ptr -> tx_thread_suspend_control_block =  (VOID *) semaphore_ptr;

#ifndef TX_NOT_INTERRUPTABLE

            /* Increment the suspension sequence number, which is used to identify
               this suspension event.  */
            thread_ptr -> tx_thread_suspension_sequence++;
#endif

            /* Setup suspension list.  */
            if (semaphore_ptr -> tx_semaphore_suspended_count == TX_NO_SUSPENSIONS)
            {

                /* No other threads are suspended.  Setup the head pointer and
                   just setup this threads pointers to itself.  */
                semaphore_ptr -> tx_semaphore_suspension_list =         thread_ptr;
                thread_ptr -> tx_thread_suspended_next =                thread_ptr;
                thread_ptr -> tx_thread_suspended_previous =            thread_ptr;
            }
            else
            {

                /* This list is not NULL, add current thread to the end. */
                next_thread =                                   semaphore_ptr -> tx_semaphore_suspension_list;
                thread_ptr -> tx_thread_suspended_next =        next_thread;
                previous_thread =                               next_thread -> tx_thread_suspended_previous;
                thread_ptr -> tx_thread_suspended_previous =    previous_thread;
                previous_thread -> tx_thread_suspended_next =   thread_ptr;
                next_thread -> tx_thread_suspended_previous =   thread_ptr;
            }

            /* Increment the number of suspensions.  */
            semaphore_ptr -> tx_semaphore_suspended_count++;

            /* Set the state to suspended.  */
            thread_ptr -> tx_thread_state =    TX_SEMAPHORE_SUSP;

#ifdef TX_NOT_INTERRUPTABLE

            /* Call actual non-interruptable thread suspension routine.  */
            _tx_thread_system_ni_suspend(thread_ptr, wait_option);

            /* Restore interrupts.  */
            TX_RESTORE
#else

            /* Set the suspending flag.  */
            thread_ptr -> tx_thread_suspending =  TX_TRUE;

            /* Setup the timeout period.  */
            thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  wait_option;

            /* Temporarily disable preemption.  */
            _tx_thread_preempt_disable++;

            /* Restore interrupts.  */
            TX_RESTORE

            /* Call actual thread suspension routine.  */
            _tx_thread_system_suspend(thread_ptr);
#endif

            /* Return the completion status.  */
            status =  thread_ptr -> tx_thread_suspend_status;
        }
    }
    else
    {

        /* Restore interrupts.  */
        TX_RESTORE

        /* Immediate return, return error completion.  */
        status =  TX_NO_INSTANCE;
    }

    /* Return completion status.  */
    return(status);
}

