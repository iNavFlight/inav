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
#include "nx_ip.h"
#include "nx_ipv6.h"
#include "nx_packet.h"
#include "nx_icmpv6.h"

#ifdef FEATURE_NX_IPV6


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ipv6_packet_send                                PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function prepends an IP header and sends an IP packet to the   */
/*    appropriate link driver.  Caller needs to fill in the correct       */
/*    source and destination addresses into the packet source and         */
/*    destination address.  Caller also makes sure that the packet        */
/*    interface address is valid (not in tentative state), and source     */
/*    address is not unspecified e.g. NULL.                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Pointer to packet to send     */
/*    protocol                              Protocol being encapsulated   */
/*    payload_size                          Size of the payload           */
/*    hop_limit                             Hop limit value to set in IP  */
/*                                             header.                    */
/*    src_address                           Source address                */
/*    dest_address                          Destination address           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ipv6_header_add                   Add IPv6 header               */
/*    _nx_packet_transmit_release           Release transmit packet       */
/*    _nx_nd_cache_add_entry                Add new entry to ND Cache     */
/*    IPv6_Address_Type                     Find IPv6 address type        */
/*    _nx_packet_copy                       Packet copy                   */
/*    _nx_ip_packet_deferred_receive        Places received packets in    */
/*                                            deferred packet queue       */
/*    _nx_icmpv6_send_ns                    Send neighbor solicitation    */
/*    _nxd_ipv6_search_onlink               Find onlink match             */
/*    _nx_ipv6_fragment_processing          Fragment processing           */
/*    (ip_link_driver)                      User supplied link driver     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX Source Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  08-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            supported TCP/IP offload,   */
/*                                            resulting in version 6.1.8  */
/*                                                                        */
/**************************************************************************/
VOID _nx_ipv6_packet_send(NX_IP *ip_ptr, NX_PACKET *packet_ptr,
                          ULONG protocol, ULONG payload_size, ULONG hop_limit,
                          ULONG *src_address, ULONG *dest_address)
{

UINT                       status = NX_SUCCESS;
ULONG                      address_type;
UINT                       next_hop_mtu;
ULONG                      fragment = NX_TRUE;
#ifdef NX_ENABLE_IPV6_PATH_MTU_DISCOVERY
NX_IPV6_DESTINATION_ENTRY *next_hop_dest_entry_ptr;
#endif /* NX_ENABLE_IPV6_PATH_MTU_DISCOVERY */
NX_IP_DRIVER               driver_request;
NX_PACKET                 *remove_packet;
NX_PACKET                 *packet_copy;
UINT                       same_address;
NX_INTERFACE              *if_ptr;
NX_IPV6_DESTINATION_ENTRY *dest_entry_ptr;

    /*lint -e{644} suppress variable might not be initialized, since "packet_ptr" was initialized. */
    if_ptr = packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_attached;

    /* Interface can not be NULL. */
    NX_ASSERT(if_ptr != NX_NULL);

#ifdef NX_ENABLE_TCPIP_OFFLOAD
    if (if_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_TCPIP_OFFLOAD)
    {
#ifndef NX_DISABLE_IP_INFO

        /* Increment the IP invalid packet error.  */
        ip_ptr -> nx_ip_invalid_transmit_packets++;
#endif

        /* Ignore sending all packets for TCP/IP offload. Release the packet.  */
        _nx_packet_transmit_release(packet_ptr);

        /* Return... nothing more can be done!  */
        return;
    }
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

    /* Add IPv6 header. */
    if (_nx_ipv6_header_add(ip_ptr, &packet_ptr, protocol, payload_size,
                            hop_limit, src_address, dest_address, &fragment) != NX_SUCCESS)
    {

        /* Failed to add header. */
        return;
    }

#ifdef NX_ENABLE_IP_PACKET_FILTER
    /* Check if the IP packet filter is set. */
    if (ip_ptr -> nx_ip_packet_filter)
    {

        /* Yes, call the IP packet filter routine. */
        if (ip_ptr -> nx_ip_packet_filter((VOID *)(packet_ptr -> nx_packet_prepend_ptr),
                                          NX_IP_PACKET_OUT) != NX_SUCCESS)
        {

            /* Drop the packet. */
            _nx_packet_transmit_release(packet_ptr);
            return;
        }
    }

    /* Check if the IP packet filter extended is set. */
    if (ip_ptr -> nx_ip_packet_filter_extended)
    {

        /* Yes, call the IP packet filter extended routine. */
        if (ip_ptr -> nx_ip_packet_filter_extended(ip_ptr, packet_ptr, NX_IP_PACKET_OUT) != NX_SUCCESS)
        {

            /* Drop the packet. */
            _nx_packet_transmit_release(packet_ptr);
            return;
        }
    }
#endif /* NX_ENABLE_IP_PACKET_FILTER */

    next_hop_mtu = if_ptr -> nx_interface_ip_mtu_size;
    packet_ptr -> nx_packet_ip_header = packet_ptr -> nx_packet_prepend_ptr;

    /* Check if the host is sending itself a packet. */
    same_address = (UINT)CHECK_IPV6_ADDRESSES_SAME(dest_address, src_address);

    /* If it is, consider this a loopback address. */
    if (same_address == 1)
    {

        address_type = IPV6_ADDRESS_LOOPBACK;
    }
    else
    {

        /* Otherwise check if this packet sending to a known loopback address. */
        address_type = IPv6_Address_Type(dest_address);
    }

    /* Handle the internal loopback case. */
    if (address_type == IPV6_ADDRESS_LOOPBACK)
    {

        if (_nx_packet_copy(packet_ptr, &packet_copy, ip_ptr -> nx_ip_default_packet_pool, NX_NO_WAIT) == NX_SUCCESS)
        {

#ifdef NX_ENABLE_INTERFACE_CAPABILITY

            /* Compute checksum for upper layer protocol. */
            /*lint -e{644} suppress variable might not be initialized, since "packet_copy" was initialized as long as return value is NX_SUCCESS. */
            if (packet_copy -> nx_packet_interface_capability_flag)
            {
                _nx_ip_packet_checksum_compute(packet_copy);
            }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

            /* Get the interface of copied packet. */
            /*lint --e{644} suppress variable might not be initialized, since "packet_copy" was initialized as long as return value is NX_SUCCESS. */
            packet_copy -> nx_packet_address.nx_packet_interface_ptr = packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_attached;

#ifndef NX_DISABLE_IP_INFO

            /* Increment the IP packet sent count. */
            ip_ptr -> nx_ip_total_packets_sent++;

            /* Increment the IP bytes sent count. */
            ip_ptr -> nx_ip_total_bytes_sent += packet_ptr -> nx_packet_length - (ULONG)sizeof(NX_IPV6_HEADER);
#endif

            /* Send the packet to this IP's receive processing like it came in from the driver. */
            _nx_ip_packet_deferred_receive(ip_ptr, packet_copy);
        }
#ifndef NX_DISABLE_IP_INFO
        else
        {
            /* Increment the IP send packets dropped count. */
            ip_ptr -> nx_ip_send_packets_dropped++;

            /* Increment the IP transmit resource error count. */
            ip_ptr -> nx_ip_transmit_resource_errors++;
        }
#endif
        /* Release the transmit packet. */
        _nx_packet_transmit_release(packet_ptr);
        return;
    }

    /* Initial the driver request. */
    driver_request.nx_ip_driver_ptr                  = ip_ptr;
    driver_request.nx_ip_driver_command              = NX_LINK_PACKET_SEND;
    driver_request.nx_ip_driver_packet               = packet_ptr;
    driver_request.nx_ip_driver_interface            = NX_NULL;

    /* Determine if physical mapping is needed by this link driver. */
    if (if_ptr -> nx_interface_address_mapping_needed)
    {

        /* Is this packet a multicast ? */
        if ((dest_address[0] & (ULONG)0xFF000000) == (ULONG)0xFF000000)
        {


            /* Set up the driver request. */
            driver_request.nx_ip_driver_physical_address_msw = 0x00003333;
            driver_request.nx_ip_driver_physical_address_lsw = dest_address[3];
            driver_request.nx_ip_driver_interface            = if_ptr;

            /* It is; is path MTU enabled? */
#ifdef NX_ENABLE_IPV6_PATH_MTU_DISCOVERY

            /* It is. We will check the path MTU for the packet destination to
               determine if we need to fragment the packet. */

            /* Lookup the multicast destination in the destination table.  */
            status = _nx_icmpv6_dest_table_find(ip_ptr, dest_address, &dest_entry_ptr, 0, 0);

            /* Did we find it in the table? */
            /*lint -e{644} suppress variable might not be initialized, since "dest_entry_ptr" was initialized as long as status is NX_SUCCESS. */
            if (status == NX_SUCCESS)
            {
                next_hop_mtu = dest_entry_ptr -> nx_ipv6_destination_entry_path_mtu;
            }

#endif  /* NX_ENABLE_IPV6_PATH_MTU_DISCOVERY */
        }
        else
        {

        /* Obtain MAC address */
        ND_CACHE_ENTRY *NDCacheEntry = NX_NULL;
        ULONG           next_hop_address[4];

            SET_UNSPECIFIED_ADDRESS(next_hop_address);

            /* Lookup the packet destination in the destination table. */
            status = _nx_icmpv6_dest_table_find(ip_ptr, dest_address, &dest_entry_ptr, 0, 0);

            /* Was a matching entry found? */
            if (status != NX_SUCCESS)
            {

                /* No; If the packet is either onlink or there is no default router,
                   just copy the packet destination address to the 'next hop' address.  */

                if (_nxd_ipv6_search_onlink(ip_ptr, dest_address))
                {
                    COPY_IPV6_ADDRESS(dest_address, next_hop_address);

                    /* Add the next_hop in destination table. */
                    status = _nx_icmpv6_dest_table_add(ip_ptr, dest_address, &dest_entry_ptr,
                                                       next_hop_address, if_ptr -> nx_interface_ip_mtu_size,
                                                       NX_WAIT_FOREVER, packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr);

                    /* Get the NDCacheEntry. */
                    if (status == NX_SUCCESS)
                    {
                        NDCacheEntry = dest_entry_ptr -> nx_ipv6_destination_entry_nd_entry;
                    }
                }
                /* Check whether or not we have a default router. */
                /* Suppress cast of pointer to pointer, since it is necessary  */
                else if (_nxd_ipv6_router_lookup(ip_ptr, if_ptr, next_hop_address, /*lint -e{929}*/ (void **)&NDCacheEntry) == NX_SUCCESS)
                {
                    /* Add the next_hop in destination table. */
                    status = _nx_icmpv6_dest_table_add(ip_ptr, dest_address, &dest_entry_ptr,
                                                       next_hop_address, if_ptr -> nx_interface_ip_mtu_size,
                                                       NX_WAIT_FOREVER, packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr);

                    /* If the default router did not has a reachable ND_CACHE_ENTRY. Get the NDCacheEntry. */
                    /*lint -e{644} suppress variable might not be initialized, since "NDCacheEntry" was initialized in _nxd_ipv6_route_lookup. */
                    if ((status == NX_SUCCESS) && !NDCacheEntry)
                    {
                        NDCacheEntry = dest_entry_ptr -> nx_ipv6_destination_entry_nd_entry;
                    }
                }

                /* Destination table add failed. */
                if (status)
                {

                    /* Release the transmit packet. */
                    _nx_packet_transmit_release(packet_ptr);

                    /* Can't send it. */
                    return;
                }
            }
            /* Find a valid destination cache, set the nd cache and next hop address. */
            else
            {

                /* Get the destination and next hop address. */
                NDCacheEntry = dest_entry_ptr -> nx_ipv6_destination_entry_nd_entry;
                COPY_IPV6_ADDRESS(dest_entry_ptr -> nx_ipv6_destination_entry_next_hop, next_hop_address);
                NX_ASSERT(NDCacheEntry -> nx_nd_cache_nd_status != ND_CACHE_STATE_INVALID);
            }

            /* According RFC2461 ch 7.3.3, as long as the entry is valid and not in INCOMPLETE state,
               the IP layer should use the cached link layer address.  */
            if ((NDCacheEntry -> nx_nd_cache_nd_status >= ND_CACHE_STATE_REACHABLE) &&
                (NDCacheEntry -> nx_nd_cache_nd_status <= ND_CACHE_STATE_PROBE))
            {

            UCHAR *mac_addr;

                mac_addr = NDCacheEntry -> nx_nd_cache_mac_addr;

                /* Assume we find the mac */
                driver_request.nx_ip_driver_physical_address_msw = ((ULONG)mac_addr[0] << 8) | mac_addr[1];
                driver_request.nx_ip_driver_physical_address_lsw =
                    ((ULONG)mac_addr[2] << 24) | ((ULONG)mac_addr[3] << 16) | ((ULONG)mac_addr[4] << 8) | mac_addr[5];
                driver_request.nx_ip_driver_interface            = if_ptr;

                /* Check if path MTU Discovery is enabled first. */

#ifdef NX_ENABLE_IPV6_PATH_MTU_DISCOVERY

                /* It is.  To know if we need to fragment this packet we need the path MTU for the packet
                   destination.  */

                /* If this destination has a non null next hop, we need to ascertain the next hop MTU.  */

                /* Get the path MTU for the actual destination. */
                next_hop_mtu = dest_entry_ptr -> nx_ipv6_destination_entry_path_mtu;


                /* Find the next hop in the destination table. */
                status = _nx_icmpv6_dest_table_find(ip_ptr, next_hop_address, &next_hop_dest_entry_ptr, 0, 0);

                if (status == NX_SUCCESS)
                {

                    /* Now compare the destination path MTU with the next hop path MTU*/
                    /*lint -e{644} suppress variable might not be initialized, since "next_hop_dest_entry_ptr" was initialized as long as status is NX_SUCCESS. */
                    if ((next_hop_dest_entry_ptr -> nx_ipv6_destination_entry_path_mtu > 0) &&
                        (next_hop_mtu > next_hop_dest_entry_ptr -> nx_ipv6_destination_entry_path_mtu))
                    {

                        /* Update the path mtu to reflect the next hop route. */
                        next_hop_mtu = next_hop_dest_entry_ptr -> nx_ipv6_destination_entry_path_mtu;
                    }
                }

#endif  /* NX_ENABLE_IPV6_PATH_MTU_DISCOVERY */

                /* If the entry is in STALE state, move it to DELAY state. */
                if (NDCacheEntry -> nx_nd_cache_nd_status == ND_CACHE_STATE_STALE)
                {
                    NDCacheEntry -> nx_nd_cache_nd_status = ND_CACHE_STATE_DELAY;

                    /* Start the Delay first probe timer */
                    NDCacheEntry -> nx_nd_cache_timer_tick = NX_DELAY_FIRST_PROBE_TIME;
                }
            }
            else
            {

                /* No MAC address was found in our cache table.  Start the Neighbor Discovery (ND)
                   process to get it. */

                /* Ensure the current packet's queue next pointer to NULL */
                packet_ptr -> nx_packet_queue_next = NX_NULL;

                /* Determine if the queue is empty. */
                if (NDCacheEntry -> nx_nd_cache_packet_waiting_head == NX_NULL)
                {
                    /* ICMPv6 is enabled */
                    if (ip_ptr -> nx_ip_icmpv6_packet_process)
                    {

                        /* Queue up this packet */
                        NDCacheEntry -> nx_nd_cache_packet_waiting_head = packet_ptr;
                        NDCacheEntry -> nx_nd_cache_packet_waiting_tail = packet_ptr;
                        NDCacheEntry -> nx_nd_cache_packet_waiting_queue_length = 1;

                        /* Add debug information. */
                        NX_PACKET_DEBUG(NX_PACKET_ND_WAITING_QUEUE, __LINE__, packet_ptr);

                        /* Set the outgoing address and interface to the cache entry.  */
                        NDCacheEntry -> nx_nd_cache_outgoing_address = packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr;
                        NDCacheEntry -> nx_nd_cache_interface_ptr = if_ptr;

                        /* Is this a new entry? */
                        if (NDCacheEntry -> nx_nd_cache_nd_status == ND_CACHE_STATE_CREATED)
                        {

                            /* Start Neighbor discovery process by advancing to the incomplete state. */
                            NDCacheEntry -> nx_nd_cache_nd_status = ND_CACHE_STATE_INCOMPLETE;
                        }

                        /* Note that the 2nd last parameter sendUnicast is set to Zero. In this case
                           the last arg NDCacheEntry is not being used in _nx_icmpv6_send_ns. */
                        _nx_icmpv6_send_ns(ip_ptr, next_hop_address,
                                           1, packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr, 0, NDCacheEntry);

                        NDCacheEntry -> nx_nd_cache_num_solicit = NX_MAX_MULTICAST_SOLICIT - 1;
                        NDCacheEntry -> nx_nd_cache_timer_tick = ip_ptr -> nx_ipv6_retrans_timer_ticks;
                    }
                    else
                    {

                        _nx_packet_transmit_release(packet_ptr);
#ifndef NX_DISABLE_IP_INFO

                        /* Increment the IP transmit resource error count.  */
                        ip_ptr -> nx_ip_transmit_resource_errors++;

                        /* Increment the IP send packets dropped count.  */
                        ip_ptr -> nx_ip_send_packets_dropped++;
#endif
                    }
                    return;
                }

                /* The ND process already started.  Simply queue up this packet */
                NDCacheEntry -> nx_nd_cache_packet_waiting_tail -> nx_packet_queue_next = packet_ptr;
                NDCacheEntry -> nx_nd_cache_packet_waiting_tail = packet_ptr;
                NDCacheEntry -> nx_nd_cache_packet_waiting_queue_length++;

                /* Add debug information. */
                NX_PACKET_DEBUG(NX_PACKET_ND_WAITING_QUEUE, __LINE__, packet_ptr);

                /* Check if the number of packets enqueued exceeds the allowed number. */
                if (NDCacheEntry -> nx_nd_cache_packet_waiting_queue_length > NX_ND_MAX_QUEUE_DEPTH)
                {

                    /* Yes, so delete the first packet. */
                    remove_packet = NDCacheEntry -> nx_nd_cache_packet_waiting_head;

                    NDCacheEntry -> nx_nd_cache_packet_waiting_head = remove_packet -> nx_packet_queue_next;

                    /* Update the queued packet count for this cache entry. */
                    NDCacheEntry -> nx_nd_cache_packet_waiting_queue_length--;

                    _nx_packet_transmit_release(remove_packet);
#ifndef NX_DISABLE_IP_INFO
                    /* Increment the IP transmit resource error count.  */
                    ip_ptr -> nx_ip_transmit_resource_errors++;

                    /* Increment the IP send packets dropped count.  */
                    ip_ptr -> nx_ip_send_packets_dropped++;
#endif
                }

                return;
            }
        }
    }
    else
    {

        /* This IP instance does not require any IP-to-physical mapping.  */
        /* Build the driver request.  */

        driver_request.nx_ip_driver_physical_address_msw = 0;
        driver_request.nx_ip_driver_physical_address_lsw = 0;
        driver_request.nx_ip_driver_interface            = if_ptr;
    }

    /* Does the packet payload exceed next hop MTU?  */
    if (packet_ptr -> nx_packet_length > next_hop_mtu)
    {
#ifndef NX_DISABLE_FRAGMENTATION
#ifdef NX_IPSEC_ENABLE
        /* Check the fragment status, transport mode SAs can not carry fragment, RFC 4301 page 66&88.  */
        /*lint -e{774} suppress boolean always evaluates to True, since the value fragment can changed when NX_IPSEC_ENABLE is defined. */
        if (fragment == NX_TRUE)
        {
#endif  /* NX_IPSEC_ENABLE */

            /* Yes; ok to fragment the packet payload. */
            _nx_ipv6_fragment_process(&driver_request, next_hop_mtu);
#ifdef NX_IPSEC_ENABLE
        }
        else
        {

#ifndef NX_DISABLE_IP_INFO

            /* Increment the IP send packets dropped count.  */
            ip_ptr -> nx_ip_send_packets_dropped++;
#endif
            /* Just release the packet.  */
            _nx_packet_transmit_release(packet_ptr);
        }
#endif  /* NX_IPSEC_ENABLE */

#else   /* NX_DISABLE_FRAGMENTATION */

#ifndef NX_DISABLE_IP_INFO
        /* Increment the IP send packets dropped count.  */
        ip_ptr -> nx_ip_send_packets_dropped++;
#endif
        /* Just release the packet.  */
        _nx_packet_transmit_release(packet_ptr);
#endif  /* NX_DISABLE_FRAGMENTATION */

        /* This packet send is complete, just return.  */
        return;
    }

    /* The packet requires no fragmentation. Proceed with sending the packet. */

#ifndef NX_DISABLE_IP_INFO

    /* Increment the IP packet sent count.  */
    ip_ptr -> nx_ip_total_packets_sent++;

    /* Increment the IP bytes sent count.  */
    ip_ptr -> nx_ip_total_bytes_sent +=  packet_ptr -> nx_packet_length - (ULONG)sizeof(NX_IPV6_HEADER);
#endif

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* Driver entry must not be NULL. */
    NX_ASSERT(if_ptr -> nx_interface_link_driver_entry != NX_NULL);

    /* Send the IP packet out on the network via the attached driver.  */
    (if_ptr -> nx_interface_link_driver_entry)(&driver_request);
}

#endif /* FEATURE_NX_IPV6 */

