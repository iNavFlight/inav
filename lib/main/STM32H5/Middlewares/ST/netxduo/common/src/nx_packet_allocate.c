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
/**   Packet Pool Management (Packet)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "tx_thread.h"
#include "nx_packet.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_packet_allocate                                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function allocates a packet from the specified packet pool.    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    pool_ptr                              Pool to allocate packet from  */
/*    packet_ptr                            Pointer to place allocated    */
/*                                            packet pointer              */
/*    packet_type                           Type of packet to allocate    */
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
UINT  _nx_packet_allocate(NX_PACKET_POOL *pool_ptr,  NX_PACKET **packet_ptr,
                          ULONG packet_type, ULONG wait_option)
{
TX_INTERRUPT_SAVE_AREA

UINT       status;              /* Return status           */
TX_THREAD *thread_ptr;          /* Working thread pointer  */
NX_PACKET *work_ptr;            /* Working packet pointer  */

#ifdef TX_ENABLE_EVENT_TRACE
TX_TRACE_BUFFER_ENTRY *trace_event;
ULONG                  trace_timestamp;
#endif

    /* Make sure the packet_type does not go beyond nx_packet_data_end. */
    if (pool_ptr -> nx_packet_pool_payload_size < packet_type)
    {
        return(NX_INVALID_PARAMETERS);
    }

    /* Set the return pointer to NULL initially.  */
    *packet_ptr =   NX_NULL;

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_PACKET_ALLOCATE, pool_ptr, 0, packet_type, pool_ptr -> nx_packet_pool_available, NX_TRACE_PACKET_EVENTS, &trace_event, &trace_timestamp);

    /* Disable interrupts to get a packet from the pool.  */
    TX_DISABLE

    /* Determine if there is an available packet.  */
    if (pool_ptr -> nx_packet_pool_available)
    {

        /* Yes, a packet is available.  Decrement the available count.  */
        pool_ptr -> nx_packet_pool_available--;

        /* Pickup the current packet pointer.  */
        work_ptr =  pool_ptr -> nx_packet_pool_available_list;

        /* Modify the available list to point at the next packet in the pool. */
        pool_ptr -> nx_packet_pool_available_list =  work_ptr -> nx_packet_queue_next;

        /* Setup various fields for this packet.  */
        work_ptr -> nx_packet_queue_next =   NX_NULL;
#ifndef NX_DISABLE_PACKET_CHAIN
        work_ptr -> nx_packet_next =         NX_NULL;
        work_ptr -> nx_packet_last =         NX_NULL;
#endif /* NX_DISABLE_PACKET_CHAIN */
        work_ptr -> nx_packet_length =       0;
        work_ptr -> nx_packet_prepend_ptr =  work_ptr -> nx_packet_data_start + packet_type;
        work_ptr -> nx_packet_append_ptr =   work_ptr -> nx_packet_prepend_ptr;
        work_ptr -> nx_packet_address.nx_packet_interface_ptr = NX_NULL;
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
        work_ptr -> nx_packet_interface_capability_flag = 0;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
        /* Set the TCP queue to the value that indicates it has been allocated.  */
        /*lint -e{923} suppress cast of ULONG to pointer.  */
        work_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next =  (NX_PACKET *)NX_PACKET_ALLOCATED;

#ifdef FEATURE_NX_IPV6

        /* Clear the option state. */
        work_ptr -> nx_packet_option_state = 0;
#endif /* FEATURE_NX_IPV6 */

#ifdef NX_IPSEC_ENABLE

        /* Clear the ipsec state. */
        work_ptr -> nx_packet_ipsec_state = 0;
        work_ptr -> nx_packet_ipsec_sa_ptr = NX_NULL;
#endif /* NX_IPSEC_ENABLE */

#ifndef NX_DISABLE_IPV4
        /* Initialize the IP version field */
        work_ptr -> nx_packet_ip_version = NX_IP_VERSION_V4;
#endif /* !NX_DISABLE_IPV4  */

        /* Initialize the IP identification flag.  */
        work_ptr -> nx_packet_identical_copy = NX_FALSE;

        /* Initialize the IP header length. */
        work_ptr -> nx_packet_ip_header_length = 0;

#ifdef NX_ENABLE_THREAD
        work_ptr -> nx_packet_type = 0;
#endif /* NX_ENABLE_THREAD  */

        /* Place the new packet pointer in the return destination.  */
        *packet_ptr =  work_ptr;

        /* Set status to success.  */
        status =  NX_SUCCESS;

        /* Add debug information. */
        NX_PACKET_DEBUG(__FILE__, __LINE__, work_ptr);
    }
    else
    {

#ifndef NX_DISABLE_PACKET_INFO
        /* Increment the packet pool empty request count.  */
        pool_ptr -> nx_packet_pool_empty_requests++;
#endif

        /* Determine if the request specifies suspension.  */
        if (wait_option)
        {

            /* Prepare for suspension of this thread.  */

#ifndef NX_DISABLE_PACKET_INFO
            /* Increment the packet pool empty request suspension count.  */
            pool_ptr -> nx_packet_pool_empty_suspensions++;
#endif

            /* Pickup thread pointer.  */
            thread_ptr =  _tx_thread_current_ptr;

            /* Setup cleanup routine pointer.  */
            thread_ptr -> tx_thread_suspend_cleanup =  _nx_packet_pool_cleanup;

            /* Setup cleanup information, i.e. this pool control
               block.  */
            thread_ptr -> tx_thread_suspend_control_block =  (void *)pool_ptr;

            /* Save the return packet pointer address as well.  */
            thread_ptr -> tx_thread_additional_suspend_info =  (void *)packet_ptr;

            /* Save the packet type (or prepend offset) so this can be added
               after a new packet becomes available.  */
            thread_ptr -> tx_thread_suspend_info =  packet_type;

            /* Setup suspension list.  */
            if (pool_ptr -> nx_packet_pool_suspension_list)
            {

                /* This list is not NULL, add current thread to the end. */
                thread_ptr -> tx_thread_suspended_next =
                    pool_ptr -> nx_packet_pool_suspension_list;
                thread_ptr -> tx_thread_suspended_previous =
                    (pool_ptr -> nx_packet_pool_suspension_list) -> tx_thread_suspended_previous;
                ((pool_ptr -> nx_packet_pool_suspension_list) -> tx_thread_suspended_previous) -> tx_thread_suspended_next =
                    thread_ptr;
                (pool_ptr -> nx_packet_pool_suspension_list) -> tx_thread_suspended_previous =   thread_ptr;
            }
            else
            {

                /* No other threads are suspended.  Setup the head pointer and
                   just setup this threads pointers to itself.  */
                pool_ptr -> nx_packet_pool_suspension_list =  thread_ptr;
                thread_ptr -> tx_thread_suspended_next =            thread_ptr;
                thread_ptr -> tx_thread_suspended_previous =        thread_ptr;
            }

            /* Increment the suspended thread count.  */
            pool_ptr -> nx_packet_pool_suspended_count++;

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
            NX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, NX_TRACE_PACKET_ALLOCATE, 0, *packet_ptr, 0, 0);

#ifdef NX_ENABLE_PACKET_DEBUG_INFO
            if (thread_ptr -> tx_thread_suspend_status == NX_SUCCESS)
            {

                /* Add debug information. */
                NX_PACKET_DEBUG(__FILE__, __LINE__, *packet_ptr);
            }
#endif /* NX_ENABLE_PACKET_DEBUG_INFO */

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

    /* Update the trace event with the status.  */
    NX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, NX_TRACE_PACKET_ALLOCATE, 0, *packet_ptr, 0, 0);

    /* Return completion status.  */
    return(status);
}

