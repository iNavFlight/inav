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
/**   Queue                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_thread.h"
#include "tx_queue.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_queue_flush                                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function resets the specified queue, if there are any messages */
/*    in it.  Messages waiting to be placed on the queue are also thrown  */
/*    out.                                                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    queue_ptr                         Pointer to queue control block    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_SUCCESS                        Successful completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_preempt_check   Check for preemption              */
/*    _tx_thread_system_resume          Resume thread service             */
/*    _tx_thread_system_ni_resume       Non-interruptable resume thread   */
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
UINT  _tx_queue_flush(TX_QUEUE *queue_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD       *suspension_list;
UINT            suspended_count;
TX_THREAD       *thread_ptr;


    /* Initialize the suspended count and list.  */
    suspended_count =  TX_NO_SUSPENSIONS;
    suspension_list =  TX_NULL;

    /* Disable interrupts to reset various queue parameters.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_QUEUE_FLUSH, queue_ptr, TX_POINTER_TO_ULONG_CONVERT(&thread_ptr), 0, 0, TX_TRACE_QUEUE_EVENTS)

    /* Log this kernel call.  */
    TX_EL_QUEUE_FLUSH_INSERT

    /* Determine if there is something on the queue.  */
    if (queue_ptr -> tx_queue_enqueued != TX_NO_MESSAGES)
    {

        /* Yes, there is something in the queue.  */

        /* Reset the queue parameters to erase all of the queued messages.  */
        queue_ptr -> tx_queue_enqueued =           TX_NO_MESSAGES;
        queue_ptr -> tx_queue_available_storage =  queue_ptr -> tx_queue_capacity;
        queue_ptr -> tx_queue_read =               queue_ptr -> tx_queue_start;
        queue_ptr -> tx_queue_write =              queue_ptr -> tx_queue_start;

        /* Now determine if there are any threads suspended on a full queue.  */
        if (queue_ptr -> tx_queue_suspended_count != TX_NO_SUSPENSIONS)
        {

            /* Yes, there are threads suspended on this queue, they must be
               resumed!  */

            /* Copy the information into temporary variables.  */
            suspension_list =  queue_ptr -> tx_queue_suspension_list;
            suspended_count =  queue_ptr -> tx_queue_suspended_count;

            /* Clear the queue variables.  */
            queue_ptr -> tx_queue_suspension_list =  TX_NULL;
            queue_ptr -> tx_queue_suspended_count =  TX_NO_SUSPENSIONS;

            /* Temporarily disable preemption.  */
            _tx_thread_preempt_disable++;
        }
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Walk through the queue list to resume any and all threads suspended
       on this queue.  */
    if (suspended_count != TX_NO_SUSPENSIONS)
    {

        /* Pickup the thread to resume.  */
        thread_ptr =  suspension_list;
        while (suspended_count != ((ULONG) 0))
        {

            /* Decrement the suspension count.  */
            suspended_count--;

            /* Check for a NULL thread pointer.  */
            if (thread_ptr == TX_NULL)
            {

                /* Get out of the loop.  */
                break;
            }

            /* Resume the next suspended thread.  */

            /* Lockout interrupts.  */
            TX_DISABLE

            /* Clear the cleanup pointer, this prevents the timeout from doing
               anything.  */
            thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

            /* Set the return status in the thread to TX_SUCCESS.  */
            thread_ptr -> tx_thread_suspend_status =  TX_SUCCESS;

            /* Move the thread pointer ahead.  */
            thread_ptr =  thread_ptr -> tx_thread_suspended_next;

#ifdef TX_NOT_INTERRUPTABLE

            /* Resume the thread!  */
            _tx_thread_system_ni_resume(thread_ptr -> tx_thread_suspended_previous);

            /* Restore interrupts.  */
            TX_RESTORE
#else

            /* Temporarily disable preemption again.  */
            _tx_thread_preempt_disable++;

            /* Restore interrupts.  */
            TX_RESTORE

            /* Resume the thread.  */
            _tx_thread_system_resume(thread_ptr -> tx_thread_suspended_previous);
#endif
        }

        /* Disable interrupts.  */
        TX_DISABLE

        /* Restore previous preempt posture.  */
        _tx_thread_preempt_disable--;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Check for preemption.  */
        _tx_thread_system_preempt_check();
    }

    /* Return TX_SUCCESS.  */
    return(TX_SUCCESS);
}

