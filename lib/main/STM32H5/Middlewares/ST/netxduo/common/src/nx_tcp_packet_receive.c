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
/**   Transmission Control Protocol (TCP)                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ip.h"
#include "nx_packet.h"
#include "nx_tcp.h"
#include "tx_thread.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_packet_receive                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function receives a TCP packet from the IP receive             */
/*    processing.  If this routine is called from an ISR, it simply       */
/*    places the new message on the TCP message queue, and wakes up the   */
/*    IP processing thread.  If this routine is called from the IP helper */
/*    thread, then the TCP message is processed directly.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Pointer to packet to send     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_packet_process                Process TCP packet            */
/*    tx_event_flags_set                    Set event flags for IP helper */
/*                                            thread                      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ip_packet_receive                 Dispatch received IP packets  */
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
VOID  _nx_tcp_packet_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

TX_INTERRUPT_SAVE_AREA


    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

#ifndef NX_DISABLE_RX_SIZE_CHECKING

    /* Check for valid packet length.  */
    if (packet_ptr -> nx_packet_length < sizeof(NX_TCP_HEADER))
    {

#ifndef NX_DISABLE_TCP_INFO
        /* Increment the TCP invalid packet error.  */
        ip_ptr -> nx_ip_tcp_invalid_packets++;
#endif

        /* Invalid packet length, just release it.  */
        _nx_packet_release(packet_ptr);

        /* The function is complete, just return!  */
        return;
    }
#endif

    /* Determine if this routine is being called from an ISR.  */
    if ((TX_THREAD_GET_SYSTEM_STATE()) || (&(ip_ptr -> nx_ip_thread) != _tx_thread_current_ptr))
    {

        /* If system state is non-zero, we are in an ISR. If the current thread is not the IP thread,
           we need to prevent unnecessary recursion in loopback.  Just place the message at the
           end of the TCP message queue and wakeup the IP helper thread.  */

        /* Disable interrupts.  */
        TX_DISABLE

        /* Add the packet to the TCP message queue.  */
        if (ip_ptr -> nx_ip_tcp_queue_head)
        {

            /* Link the current packet at the end of the queue.  */
            (ip_ptr -> nx_ip_tcp_queue_tail) -> nx_packet_queue_next =  packet_ptr;
            ip_ptr -> nx_ip_tcp_queue_tail =                            packet_ptr;
            packet_ptr -> nx_packet_queue_next =                        NX_NULL;

            /* Increment the count of incoming TCP packets queued.  */
            ip_ptr -> nx_ip_tcp_received_packet_count++;
        }
        else
        {

            /* Empty queue, add to the head of the TCP message queue.  */
            ip_ptr -> nx_ip_tcp_queue_head =        packet_ptr;
            ip_ptr -> nx_ip_tcp_queue_tail =        packet_ptr;
            packet_ptr -> nx_packet_queue_next =    NX_NULL;

            /* Set the initial count TCP packets queued.  */
            ip_ptr -> nx_ip_tcp_received_packet_count =  1;
        }

        /* Restore interrupts.  */
        TX_RESTORE

        /* Wakeup IP thread for processing one or more messages in the TCP queue.  */
        tx_event_flags_set(&(ip_ptr -> nx_ip_events), NX_IP_TCP_EVENT, TX_OR);
    }
    else
    {

        /* The IP message was deferred, so this routine is called from the IP helper
           thread and thus may call the TCP processing directly.  */
        _nx_tcp_packet_process(ip_ptr, packet_ptr);
    }
}

