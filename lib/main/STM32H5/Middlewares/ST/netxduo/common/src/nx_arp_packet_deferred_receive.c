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
/**   Address Resolution Protocol (ARP)                                   */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ip.h"
#include "nx_packet.h"
#include "nx_arp.h"

#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_arp_packet_deferred_receive                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function receives an ARP packet from the link driver (usually  */
/*    the link driver's input ISR) and places it in the deferred receive  */
/*    ARP packet queue.  This moves the minimal receive ARP packet        */
/*    processing from the ISR to the IP helper thread.                    */
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
/*    tx_event_flags_set                    Wakeup IP helper thread       */
/*    _nx_packet_release                    Packet release function       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application I/O Driver                                              */
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
VOID  _nx_arp_packet_deferred_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

TX_INTERRUPT_SAVE_AREA


    /* Disable interrupts.  */
    TX_DISABLE

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* Check to see if ARP is enabled on this IP instance.  */
    if (!ip_ptr -> nx_ip_arp_queue_process)
    {

        /* ARP is not enabled.  */

#ifndef NX_DISABLE_ARP_INFO
        /* Increment the ARP invalid messages count...  */
        ip_ptr -> nx_ip_arp_invalid_messages++;
#endif

        /* Restore interrupts.  */
        TX_RESTORE

        /* Since ARP is not enabled, just release the packet.  */
        _nx_packet_release(packet_ptr);

        /* Return to caller.  */
        return;
    }

    /* Check to see if the ARP deferred processing queue is empty.  */
    if (ip_ptr -> nx_ip_arp_deferred_received_packet_head)
    {

        /* Not empty, just place the packet at the end of the ARP deferred queue.  */
        (ip_ptr -> nx_ip_arp_deferred_received_packet_tail) -> nx_packet_queue_next =  packet_ptr;
        packet_ptr -> nx_packet_queue_next =  NX_NULL;
        ip_ptr -> nx_ip_arp_deferred_received_packet_tail =  packet_ptr;

        /* Restore interrupts.  */
        TX_RESTORE
    }
    else
    {

        /* Empty ARP deferred receive processing queue.  Just setup the head pointers and
           set the event flags to ensure the IP helper thread looks at the ARP deferred
           processing queue.  */
        ip_ptr -> nx_ip_arp_deferred_received_packet_head =  packet_ptr;
        ip_ptr -> nx_ip_arp_deferred_received_packet_tail =  packet_ptr;
        packet_ptr -> nx_packet_queue_next =                 NX_NULL;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Wakeup IP helper thread to process the ARP deferred receive.  */
        tx_event_flags_set(&(ip_ptr -> nx_ip_events), NX_IP_ARP_REC_EVENT, TX_OR);
    }
}
#endif /* !NX_DISABLE_IPV4  */

