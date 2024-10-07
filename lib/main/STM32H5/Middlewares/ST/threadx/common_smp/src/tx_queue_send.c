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
/*    _tx_queue_send                                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function places a message into the specified queue.  If there  */
/*    is no room in the queue, this function waits according to the       */
/*    option specified.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    queue_ptr                         Pointer to queue control block    */
/*    source_ptr                        Pointer to message source         */
/*    wait_option                       Suspension option                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_resume          Resume thread routine             */
/*    _tx_thread_system_ni_resume       Non-interruptable resume thread   */
/*    _tx_thread_system_suspend         Suspend thread routine            */
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
UINT  _tx_queue_send(TX_QUEUE *queue_ptr, VOID *source_ptr, ULONG wait_option)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD       *thread_ptr;
ULONG           *source;
ULONG           *destination;
UINT            size;
UINT            suspended_count;
TX_THREAD       *next_thread;
TX_THREAD       *previous_thread;
UINT            status;
#ifndef TX_DISABLE_NOTIFY_CALLBACKS
VOID            (*queue_send_notify)(struct TX_QUEUE_STRUCT *notify_queue_ptr);
#endif


    /* Default the status to TX_SUCCESS.  */
    status =  TX_SUCCESS;

    /* Disable interrupts to place message in the queue.  */
    TX_DISABLE

#ifdef TX_QUEUE_ENABLE_PERFORMANCE_INFO

    /* Increment the total messages sent counter.  */
    _tx_queue_performance_messages_sent_count++;

    /* Increment the number of messages sent to this queue.  */
    queue_ptr -> tx_queue_performance_messages_sent_count++;
#endif

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_QUEUE_SEND, queue_ptr, TX_POINTER_TO_ULONG_CONVERT(source_ptr), wait_option, queue_ptr -> tx_queue_enqueued, TX_TRACE_QUEUE_EVENTS)

    /* Log this kernel call.  */
    TX_EL_QUEUE_SEND_INSERT

    /* Pickup the thread suspension count.  */
    suspended_count =  queue_ptr -> tx_queue_suspended_count;

    /* Determine if there is room in the queue.  */
    if (queue_ptr -> tx_queue_available_storage != TX_NO_MESSAGES)
    {

        /* There is room for the message in the queue.  */

        /* Determine if there are suspended on this queue.  */
        if (suspended_count == TX_NO_SUSPENSIONS)
        {

            /* No suspended threads, simply place the message in the queue.  */

            /* Reduce the amount of available storage.  */
            queue_ptr -> tx_queue_available_storage--;

            /* Increase the enqueued count.  */
            queue_ptr -> tx_queue_enqueued++;

            /* Setup source and destination pointers.  */
            source =       TX_VOID_TO_ULONG_POINTER_CONVERT(source_ptr);
            destination =  queue_ptr -> tx_queue_write;
            size =         queue_ptr -> tx_queue_message_size;

            /* Copy message. Note that the source and destination pointers are
               incremented by the macro.  */
            TX_QUEUE_MESSAGE_COPY(source, destination, size)

            /* Determine if we are at the end.  */
            if (destination == queue_ptr -> tx_queue_end)
            {

                /* Yes, wrap around to the beginning.  */
                destination =  queue_ptr -> tx_queue_start;
            }

            /* Adjust the write pointer.  */
            queue_ptr -> tx_queue_write =  destination;

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

            /* Pickup the notify callback routine for this queue.  */
            queue_send_notify =  queue_ptr -> tx_queue_send_notify;
#endif

            /* No thread suspended, just return to caller.  */

            /* Restore interrupts.  */
            TX_RESTORE

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

            /* Determine if a notify callback is required.  */
            if (queue_send_notify != TX_NULL)
            {

                /* Call application queue send notification.  */
                (queue_send_notify)(queue_ptr);
            }
#endif
        }
        else
        {

            /* There is a thread suspended on an empty queue. Simply
               copy the message to the suspended thread's destination
               pointer.  */

            /* Pickup the head of the suspension list.  */
            thread_ptr =  queue_ptr -> tx_queue_suspension_list;

            /* See if this is the only suspended thread on the list.  */
            suspended_count--;
            if (suspended_count == TX_NO_SUSPENSIONS)
            {

                /* Yes, the only suspended thread.  */

                /* Update the head pointer.  */
                queue_ptr -> tx_queue_suspension_list =  TX_NULL;
            }
            else
            {

                /* At least one more thread is on the same expiration list.  */

                /* Update the list head pointer.  */
                queue_ptr -> tx_queue_suspension_list =  thread_ptr -> tx_thread_suspended_next;

                /* Update the links of the adjacent threads.  */
                next_thread =                            thread_ptr -> tx_thread_suspended_next;
                queue_ptr -> tx_queue_suspension_list =  next_thread;

                /* Update the links of the adjacent threads.  */
                previous_thread =                               thread_ptr -> tx_thread_suspended_previous;
                next_thread -> tx_thread_suspended_previous =   previous_thread;
                previous_thread -> tx_thread_suspended_next =   next_thread;
            }

            /* Decrement the suspension count.  */
            queue_ptr -> tx_queue_suspended_count =  suspended_count;

            /* Prepare for resumption of the thread.  */

            /* Clear cleanup routine to avoid timeout.  */
            thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

            /* Setup source and destination pointers.  */
            source =       TX_VOID_TO_ULONG_POINTER_CONVERT(source_ptr);
            destination =  TX_VOID_TO_ULONG_POINTER_CONVERT(thread_ptr -> tx_thread_additional_suspend_info);
            size =         queue_ptr -> tx_queue_message_size;

            /* Copy message. Note that the source and destination pointers are
               incremented by the macro.  */
            TX_QUEUE_MESSAGE_COPY(source, destination, size)

            /* Put return status into the thread control block.  */
            thread_ptr -> tx_thread_suspend_status =  TX_SUCCESS;

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

            /* Pickup the notify callback routine for this queue.  */
            queue_send_notify =  queue_ptr -> tx_queue_send_notify;
#endif

#ifdef TX_NOT_INTERRUPTABLE

            /* Resume the thread!  */
            _tx_thread_system_ni_resume(thread_ptr);

            /* Restore interrupts.  */
            TX_RESTORE
#else

            /* Temporarily disable preemption.  */
            _tx_thread_preempt_disable++;

            /* Restore interrupts.  */
            TX_RESTORE

            /* Resume thread.  */
            _tx_thread_system_resume(thread_ptr);
#endif

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

            /* Determine if a notify callback is required.  */
            if (queue_send_notify != TX_NULL)
            {

                /* Call application queue send notification.  */
                (queue_send_notify)(queue_ptr);
            }
#endif
        }
    }

    /* At this point, the queue is full. Determine if suspension is requested.  */
    else if (wait_option != TX_NO_WAIT)
    {

        /* Determine if the preempt disable flag is non-zero.  */
        if (_tx_thread_preempt_disable != ((UINT) 0))
        {

            /* Restore interrupts.  */
            TX_RESTORE

            /* Suspension is not allowed if the preempt disable flag is non-zero at this point - return error completion.  */
            status =  TX_QUEUE_FULL;
        }
        else
        {

            /* Yes, prepare for suspension of this thread.  */

#ifdef TX_QUEUE_ENABLE_PERFORMANCE_INFO

            /* Increment the total number of queue full suspensions.  */
            _tx_queue_performance_full_suspension_count++;

            /* Increment the number of full suspensions on this queue.  */
            queue_ptr -> tx_queue_performance_full_suspension_count++;
#endif

            /* Pickup thread pointer.  */
            TX_THREAD_GET_CURRENT(thread_ptr)

            /* Setup cleanup routine pointer.  */
            thread_ptr -> tx_thread_suspend_cleanup =  &(_tx_queue_cleanup);

            /* Setup cleanup information, i.e. this queue control
               block and the source pointer.  */
            thread_ptr -> tx_thread_suspend_control_block =    (VOID *) queue_ptr;
            thread_ptr -> tx_thread_additional_suspend_info =  (VOID *) source_ptr;
            thread_ptr -> tx_thread_suspend_option =           TX_FALSE;

#ifndef TX_NOT_INTERRUPTABLE

            /* Increment the suspension sequence number, which is used to identify
               this suspension event.  */
            thread_ptr -> tx_thread_suspension_sequence++;
#endif

            /* Setup suspension list.  */
            if (suspended_count == TX_NO_SUSPENSIONS)
            {

                /* No other threads are suspended.  Setup the head pointer and
                   just setup this threads pointers to itself.  */
                queue_ptr -> tx_queue_suspension_list =         thread_ptr;
                thread_ptr -> tx_thread_suspended_next =        thread_ptr;
                thread_ptr -> tx_thread_suspended_previous =    thread_ptr;
            }
            else
            {

                /* This list is not NULL, add current thread to the end. */
                next_thread =                                   queue_ptr -> tx_queue_suspension_list;
                thread_ptr -> tx_thread_suspended_next =        next_thread;
                previous_thread =                               next_thread -> tx_thread_suspended_previous;
                thread_ptr -> tx_thread_suspended_previous =    previous_thread;
                previous_thread -> tx_thread_suspended_next =   thread_ptr;
                next_thread -> tx_thread_suspended_previous =   thread_ptr;
            }

            /* Increment the suspended thread count.  */
            queue_ptr -> tx_queue_suspended_count =  suspended_count + ((UINT) 1);

            /* Set the state to suspended.  */
            thread_ptr -> tx_thread_state =    TX_QUEUE_SUSP;

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

            /* Pickup the notify callback routine for this queue.  */
            queue_send_notify =  queue_ptr -> tx_queue_send_notify;
#endif

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

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

            /* Determine if a notify callback is required.  */
            if (thread_ptr -> tx_thread_suspend_status == TX_SUCCESS)
            {

                /* Determine if there is a notify callback.  */
                if (queue_send_notify != TX_NULL)
                {

                    /* Call application queue send notification.  */
                    (queue_send_notify)(queue_ptr);
                }
            }
#endif

            /* Return the completion status.  */
            status =  thread_ptr -> tx_thread_suspend_status;
        }
    }
    else
    {

        /* Otherwise, just return a queue full error message to the caller.  */

#ifdef TX_QUEUE_ENABLE_PERFORMANCE_INFO

        /* Increment the number of full non-suspensions on this queue.  */
        queue_ptr -> tx_queue_performance_full_error_count++;

        /* Increment the total number of full non-suspensions.  */
        _tx_queue_performance_full_error_count++;
#endif

        /* Restore interrupts.  */
        TX_RESTORE

        /* Return error completion.  */
        status =  TX_QUEUE_FULL;
    }

    /* Return completion status.  */
    return(status);
}

