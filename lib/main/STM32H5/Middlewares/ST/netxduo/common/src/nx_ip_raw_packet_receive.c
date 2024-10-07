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
/** NetX Component                                                        */
/**                                                                       */
/**   Internet Protocol (IP)                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "tx_thread.h"
#include "nx_ip.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_raw_packet_receive                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves the first packet from the received raw      */
/*    packet list and returns it to the caller.  If no packet is present  */
/*    the routine may optionally suspend waiting on the list for a        */
/*    packet.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP control block pointer      */
/*    packet_ptr                            Pointer to return allocated   */
/*                                            packet                      */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_suspend             Suspend thread                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_ip_raw_packet_receive(NX_IP *ip_ptr, NX_PACKET **packet_ptr, ULONG wait_option)
{
TX_INTERRUPT_SAVE_AREA

UINT                   status;      /* Return status           */
TX_THREAD             *thread_ptr;  /* Working thread pointer  */
NX_PACKET             *work_ptr;    /* Working packet pointer  */

#ifdef TX_ENABLE_EVENT_TRACE
TX_TRACE_BUFFER_ENTRY *trace_event;
ULONG                  trace_timestamp;
#endif


    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_IP_RAW_PACKET_RECEIVE, ip_ptr, 0, wait_option, 0, NX_TRACE_IP_EVENTS, &trace_event, &trace_timestamp);

    /* Disable interrupts to get a packet from the pool.  */
    TX_DISABLE

    /* Determine if there is an available packet.  */
    if (ip_ptr -> nx_ip_raw_received_packet_count)
    {

        /* Yes, a packet is available.  Decrement the raw packet receive count.  */
        ip_ptr -> nx_ip_raw_received_packet_count--;

        /* Pickup the first raw packet pointer.  */
        work_ptr =  ip_ptr -> nx_ip_raw_received_packet_head;

        /* Modify the available list to point at the next packet in the pool. */
        ip_ptr -> nx_ip_raw_received_packet_head =  work_ptr -> nx_packet_queue_next;

        /* Determine if the tail pointer points to the same packet.  */
        if (ip_ptr -> nx_ip_raw_received_packet_tail == work_ptr)
        {

            /* Yes, just set tail pointer to NULL since we must be at the end of the queue.  */
            ip_ptr -> nx_ip_raw_received_packet_tail =  NX_NULL;
        }

        /* Place the raw packet pointer in the return destination.  */
        *packet_ptr =  work_ptr;

        /* Update the trace event with the status.  */
        NX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, NX_TRACE_IP_RAW_PACKET_RECEIVE, 0, *packet_ptr, 0, 0);

        /* Set status to success.  */
        status =  NX_SUCCESS;
    }
    else
    {

        /* Determine if the request specifies suspension.  */
        if (wait_option)
        {

            /* Prepare for suspension of this thread.  */

            /* Pickup thread pointer.  */
            thread_ptr =  _tx_thread_current_ptr;

            /* Setup cleanup routine pointer.  */
            thread_ptr -> tx_thread_suspend_cleanup =  _nx_ip_raw_packet_cleanup;

            /* Setup cleanup information, i.e. this IP control
               block.  */
            thread_ptr -> tx_thread_suspend_control_block =  (void *)ip_ptr;

            /* Save the return packet pointer address as well.  */
            thread_ptr -> tx_thread_additional_suspend_info =  (void *)packet_ptr;

            /* Setup suspension list.  */
            if (ip_ptr -> nx_ip_raw_packet_suspension_list)
            {

                /* This list is not NULL, add current thread to the end. */
                thread_ptr -> tx_thread_suspended_next =
                    ip_ptr -> nx_ip_raw_packet_suspension_list;
                thread_ptr -> tx_thread_suspended_previous =
                    (ip_ptr -> nx_ip_raw_packet_suspension_list) -> tx_thread_suspended_previous;
                ((ip_ptr -> nx_ip_raw_packet_suspension_list) -> tx_thread_suspended_previous) -> tx_thread_suspended_next = thread_ptr;
                (ip_ptr -> nx_ip_raw_packet_suspension_list) -> tx_thread_suspended_previous =   thread_ptr;
            }
            else
            {

                /* No other threads are suspended.  Setup the head pointer and
                   just setup this threads pointers to itself.  */
                ip_ptr -> nx_ip_raw_packet_suspension_list =    thread_ptr;
                thread_ptr -> tx_thread_suspended_next =        thread_ptr;
                thread_ptr -> tx_thread_suspended_previous =    thread_ptr;
            }

            /* Increment the suspended thread count.  */
            ip_ptr -> nx_ip_raw_packet_suspended_count++;

            /* Set the state to suspended.  */
            thread_ptr -> tx_thread_state =  TX_TCP_IP;

            /* Set the suspending flag.  */
            thread_ptr -> tx_thread_suspending =  TX_TRUE;

            /* Temporarily disable preemption.  */
            _tx_thread_preempt_disable++;

            /* Save the timeout value.  */
            thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  wait_option;

            /* Restore interrupts.  */
            TX_RESTORE

            /* Call actual thread suspension routine.  */
            _tx_thread_system_suspend(thread_ptr);

            /* Update the trace event with the status.  */
            NX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, NX_TRACE_IP_RAW_PACKET_RECEIVE, 0, *packet_ptr, 0, 0);

            /* Return the completion status.  */
            return(thread_ptr -> tx_thread_suspend_status);
        }
        else
        {

            /* Immediate return, return error completion.  */
            status =  NX_NO_PACKET;
        }
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return completion status.  */
    return(status);
}

