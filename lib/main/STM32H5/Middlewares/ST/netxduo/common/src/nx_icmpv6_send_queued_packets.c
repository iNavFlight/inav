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
/**   Internet Control Message Protocol (ICMP)                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE

#include "nx_api.h"
#include "nx_ipv6.h"
#include "nx_icmpv6.h"
#include "nx_packet.h"

#ifdef FEATURE_NX_IPV6

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmpv6_send_queued_packets                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends out queued packets after their destination      */
/*    link layer address becomes known, i.e, after receiving a            */
/*    neighbor advertisement message from the destination host.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    nd_entry                              Destination host ND entry     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ipv6_fragment_process             IPv6 fragmentation routine    */
/*    [_nx_ip_link_driver_entry]            Ethernet Device Driver Entry  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmp_packet_process               Main ICMP packet pocess       */
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
VOID _nx_icmpv6_send_queued_packets(NX_IP *ip_ptr, ND_CACHE_ENTRY *nd_entry)
{

NX_IP_DRIVER driver_request;
UCHAR       *mac_addr;
NX_PACKET   *queued_list_head, *ip_packet_ptr;
UINT         next_hop_mtu;
#ifndef NX_DISABLE_FRAGMENTATION
#ifdef NX_ENABLE_IPV6_PATH_MTU_DISCOVERY
NX_IPV6_DESTINATION_ENTRY *dest_entry_ptr;
NX_IPV6_HEADER            *ip_header_ptr;
ULONG                      status;
#endif  /* NX_ENABLE_IPV6_PATH_MTU_DISCOVERY */
#endif  /* NX_DISABLE_FRAGMENTATION */

TX_INTERRUPT_SAVE_AREA

    /* nd_entry must not be NX_NULL. */
    NX_ASSERT(nd_entry != NX_NULL);

    /* The packet waiting head must not be NX_NULL. */
    NX_ASSERT(nd_entry -> nx_nd_cache_packet_waiting_head != NX_NULL);

    /*lint --e{613} suppress possible use of null pointer, since "nd_entry" must not be null. */
    queued_list_head = nd_entry -> nx_nd_cache_packet_waiting_head;

    mac_addr = nd_entry -> nx_nd_cache_mac_addr;

    /* Build the driver request packet.  */
    driver_request.nx_ip_driver_physical_address_msw =  ((ULONG)(mac_addr[0]) << 8)  | mac_addr[1];
    driver_request.nx_ip_driver_physical_address_lsw =  ((ULONG)(mac_addr[2]) << 24) | ((ULONG)(mac_addr[3]) << 16) | ((ULONG)(mac_addr[4]) << 8) | mac_addr[5];
    driver_request.nx_ip_driver_ptr                  =  ip_ptr;
    driver_request.nx_ip_driver_command              =  NX_LINK_PACKET_SEND;
    driver_request.nx_ip_driver_interface            =  nd_entry -> nx_nd_cache_interface_ptr;
    driver_request.nx_ip_driver_status               =  NX_SUCCESS;

    /* Loop through all the queued packets. */
    while (queued_list_head)
    {

        /* Set a pointer to the start of the queue. */
        ip_packet_ptr = queued_list_head;
        queued_list_head = queued_list_head -> nx_packet_queue_next;

        /* Clear the packet's queue next pointer */
        ip_packet_ptr -> nx_packet_queue_next = NX_NULL;

        /* Add this packet to the driver request (to send). */
        driver_request.nx_ip_driver_packet = ip_packet_ptr;

        /* Set the next hop MTU.  */
        next_hop_mtu = driver_request.nx_ip_driver_interface -> nx_interface_ip_mtu_size;

        /* Check if path MTU Discovery is enabled first. */
#ifdef NX_ENABLE_IPV6_PATH_MTU_DISCOVERY

        /* It is.  To know if we need to fragment this packet we need the path MTU for the packet
           destination.  */

        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        ip_header_ptr = (NX_IPV6_HEADER *)ip_packet_ptr -> nx_packet_prepend_ptr;

        /* Convert destination address to host byte order. */
        NX_IPV6_ADDRESS_CHANGE_ENDIAN(ip_header_ptr -> nx_ip_header_destination_ip);

        /* Find the entry in the cache table for packet's destination. */
        status = _nx_icmpv6_dest_table_find(ip_ptr, ip_header_ptr -> nx_ip_header_destination_ip, &dest_entry_ptr, 0, 0);

        /* Convert destination address back to net byte order. */
        NX_IPV6_ADDRESS_CHANGE_ENDIAN(ip_header_ptr -> nx_ip_header_destination_ip);

        /* Check for successful search and location of packet destination data. */
        if (status == NX_SUCCESS)
        {

        ULONG                      next_hop_address[4];
        NX_IPV6_DESTINATION_ENTRY *next_hop_dest_entry_ptr;

            /* If this destination has a non null next hop, we need to ascertain the next hop MTU.  */

            /* Get the path MTU for the actual destination. */
            next_hop_mtu = dest_entry_ptr -> nx_ipv6_destination_entry_path_mtu;

            /* Set the next hop address.  */
            COPY_IPV6_ADDRESS(dest_entry_ptr -> nx_ipv6_destination_entry_next_hop, next_hop_address);

            /* Check the next hop address.  */
            if (!CHECK_UNSPECIFIED_ADDRESS(&(next_hop_address[0])))
            {

                /* Find the next hop in the destination table. */
                status = _nx_icmpv6_dest_table_find(ip_ptr, next_hop_address, &next_hop_dest_entry_ptr, 0, 0);

                if (status == NX_SUCCESS)
                {


                    /* Now compare the destination path MTU with the next hop path MTU*/
                    if ((next_hop_dest_entry_ptr -> nx_ipv6_destination_entry_path_mtu > 0) &&
                        (next_hop_mtu > next_hop_dest_entry_ptr -> nx_ipv6_destination_entry_path_mtu))
                    {

                        /* Update the path mtu to reflect the next hop route. */
                        next_hop_mtu = next_hop_dest_entry_ptr -> nx_ipv6_destination_entry_path_mtu;
                    }
                }
            }
        }
#endif  /* NX_ENABLE_IPV6_PATH_MTU_DISCOVERY */

        /* Does the packet payload exceed next hop MTU?  */
        if (ip_packet_ptr -> nx_packet_length > next_hop_mtu)
        {
#ifndef NX_DISABLE_FRAGMENTATION

            /* Yes; ok to fragment the packet payload. */
            _nx_ipv6_fragment_process(&driver_request, next_hop_mtu);

#else   /* NX_DISABLE_FRAGMENTATION */

#ifndef NX_DISABLE_IP_INFO
            /* Increment the IP send packets dropped count.  */
            ip_ptr -> nx_ip_send_packets_dropped++;
#endif
            /* Just release the packet.  */
            _nx_packet_transmit_release(ip_packet_ptr);
#endif  /* NX_DISABLE_FRAGMENTATION */
        }
        else
        {

            /* The packet requires no fragmentation. Proceed with sending the packet. */

#ifndef NX_DISABLE_IP_INFO

            /* Increment the IP packet sent count.  */
            ip_ptr -> nx_ip_total_packets_sent++;

            /* Increment the IP bytes sent count.  */
            ip_ptr -> nx_ip_total_bytes_sent +=  ip_packet_ptr -> nx_packet_length - (ULONG)sizeof(NX_IPV6_HEADER);
#endif /* !NX_DISABLE_IP_INFO */

            /* Add debug information. */
            NX_PACKET_DEBUG(__FILE__, __LINE__, ip_packet_ptr);

            /* Send the queued IP packet out on the network via the attached driver.  */
            (ip_packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_attached -> nx_interface_link_driver_entry)(&driver_request);
        }
    }

    TX_DISABLE
    /*
       If the ND Cache is in STALE state, move it to DELAY state.
       This situation happens when we receive the LLA (link local address) through
       unsoliciated RA (router advertisement message. In this situation,
       the entry is in STALE state, and a packet has been transmitted,
       so the entry needs to be in DELAY state.
     */
    if (nd_entry -> nx_nd_cache_nd_status == ND_CACHE_STATE_STALE)
    {

        nd_entry -> nx_nd_cache_nd_status = ND_CACHE_STATE_DELAY;

        /* Start the Delay first probe timer */
        nd_entry -> nx_nd_cache_timer_tick = NX_DELAY_FIRST_PROBE_TIME;
    }
    TX_RESTORE

    /* Clean up the nd_entry */
    nd_entry -> nx_nd_cache_packet_waiting_head = NX_NULL;
    nd_entry -> nx_nd_cache_packet_waiting_tail = NX_NULL;

    /* Clean up the queue length variable. */
    nd_entry -> nx_nd_cache_packet_waiting_queue_length = 0;
}
#endif /* FEATURE_NX_IPV6 */

