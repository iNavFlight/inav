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
/**   Internet Group Management Protocol (IGMP)                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_packet.h"
#include "nx_igmp.h"
#include "nx_ip.h"
#include "tx_thread.h"


#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_igmp_packet_receive                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles reception of IGMP packets on the "all hosts"  */
/*    multicast address.  If this routine is called from an ISR, the      */
/*    IGMP packet is queued.  Otherwise, if this routine is called from   */
/*    the IP helper thread, the processing of the IGMP packet is called   */
/*    directly.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    packet_ptr                            IGMP packet pointer           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_igmp_packet_process               Process the IGMP packet       */
/*    tx_event_flags_set                    Set event flags for IP helper */
/*                                            thread                      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ip_packet_receive                 Raw IP packet receive         */
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
VOID  _nx_igmp_packet_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

TX_INTERRUPT_SAVE_AREA


    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

#ifndef NX_DISABLE_RX_SIZE_CHECKING

    /* Check for valid packet length.  */
    if (packet_ptr -> nx_packet_length < sizeof(NX_IGMP_HEADER))
    {

#ifndef NX_DISABLE_IGMP_INFO
        /* Increment the IGMP invalid packet error.  */
        ip_ptr -> nx_ip_igmp_invalid_packets++;
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
           end of the IGMP message queue and wakeup the IP helper thread.  */

        /* Disable interrupts.  */
        TX_DISABLE

        /* Add the packet to the IGMP message queue.  */
        if (ip_ptr -> nx_ip_igmp_queue_head)
        {

            /* Link the current packet to the list head.  */
            packet_ptr -> nx_packet_queue_next =  ip_ptr -> nx_ip_igmp_queue_head;
        }
        else
        {

            /* Empty queue, add to the head of the IGMP message queue.  */
            packet_ptr -> nx_packet_queue_next =  NX_NULL;
        }

        /* Add debug information. */
        NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

        /* Update the queue head pointer.  */
        ip_ptr -> nx_ip_igmp_queue_head =  packet_ptr;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Wakeup IP thread for processing one or more messages in the IGMP queue.  */
        tx_event_flags_set(&(ip_ptr -> nx_ip_events), NX_IP_IGMP_EVENT, TX_OR);
    }
    else
    {

        /* The IP message was deferred, so this routine is called from the IP helper
           thread and thus may call the IGMP processing directly.  */
        _nx_igmp_packet_process(ip_ptr, packet_ptr);
    }
}
#endif /* !NX_DISABLE_IPV4  */

