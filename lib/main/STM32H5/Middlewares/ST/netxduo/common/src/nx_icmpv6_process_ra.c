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
/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_packet.h"
#include "nx_ip.h"
#include "nx_ipv6.h"
#include "nx_icmpv6.h"

#ifdef FEATURE_NX_IPV6

#ifndef NX_DISABLE_ICMPV6_ROUTER_ADVERTISEMENT_PROCESS

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmpv6_process_ra                               PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes incoming ICMPv6 router advertisement        */
/*    messages.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            ICMP packet pointer           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_ipv6_default_router_add_interanl Sets IPv6 router              */
/*    _nx_ipv6_multicast_join               Join IPv6 multicast group     */
/*    _nx_ipv6_prefix_list_delete           Remove an entry from the      */
/*                                            prefix list                 */
/*    _nx_ipv6_prefix_list_add_entry        Add an entry to the prefix    */
/*                                            list                        */
/*    _nx_nd_cache_add                      Add an ND cache entry         */
/*    [ipv6_address_change_notify]          User callback function        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmpv6_packet_process             Main ICMP packet pocess       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            option length verification, */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            added internal ip address   */
/*                                            change notification,        */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
VOID _nx_icmpv6_process_ra(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

INT                           router_type;
INT                           packet_length;
INT                           prefix_length;
UINT                          i;
UINT                          status;
ULONG                         time_val;
ND_CACHE_ENTRY               *nd_entry;
NX_ICMPV6_HEADER             *header_ptr;
NX_ICMPV6_RA                 *ra_ptr;
NX_ICMPV6_OPTION             *option_ptr;
NX_ICMPV6_OPTION_PREFIX      *prefix_ptr;
#ifdef NX_ENABLE_IPV6_PATH_MTU_DISCOVERY
NX_ICMPV6_OPTION_MTU         *mtu_ptr = NULL;
#endif
NX_INTERFACE                 *if_ptr;
NX_IPV6_HEADER               *ipv6_header;
NX_IPV6_DEFAULT_ROUTER_ENTRY *rt_entry;
#ifdef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
UINT                          interface_index;
#endif /* NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY */

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* Initialize the ND cache table entry to NULL */
    nd_entry = NX_NULL;

    /* Get a pointer to the ICMP message header.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    header_ptr =  (NX_ICMPV6_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

    /* If the RA message is invalid, we simply return. */
    if (_nx_icmpv6_validate_ra(packet_ptr) != NX_SUCCESS)
    {

#ifndef NX_DISABLE_ICMP_INFO

        /* Increment the ICMP invalid packet error. */
        ip_ptr -> nx_ip_icmp_invalid_packets++;
#endif /* NX_DISABLE_ICMP_INFO */

        _nx_packet_release(packet_ptr);
        return;
    }

    /* Get a pointer to the router advertisement packet structure. */
    /*lint -e{929} suppress cast of pointer to pointer, since it is necessary  */
    ra_ptr = (NX_ICMPV6_RA *)header_ptr;

    /* If router advertisement flag callback is set, invoke the callback function. */
    if (ip_ptr -> nx_icmpv6_ra_flag_callback)
    {
        ip_ptr -> nx_icmpv6_ra_flag_callback(ip_ptr, (UINT)ra_ptr -> nx_icmpv6_ra_flag);
    }

    /* Set a pointer to the IPv6 header. */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    ipv6_header = (NX_IPV6_HEADER *)packet_ptr -> nx_packet_ip_header;

    /* Obtain the pointer to the incoming interface. */
    if_ptr = packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_attached;

    /* (1) Determine if this message comes from a periodic refresh
       or this is in response to a router solicitation.
       This is a periodic refresh if the destination address is an all-router multicast address.
     */

    if (ra_ptr -> nx_icmpv6_ra_router_lifetime == 0)
    {

    NXD_ADDRESS router_address;

        /* This router is no long valid.  */

        /* Copy the source address from the IPv6 header to a router address data
           which we will use to delete it from the router table. */
        router_address.nxd_ip_version = NX_IP_VERSION_V6;
        router_address.nxd_ip_address.v6[0] = ipv6_header -> nx_ip_header_source_ip[0];
        router_address.nxd_ip_address.v6[1] = ipv6_header -> nx_ip_header_source_ip[1];
        router_address.nxd_ip_address.v6[2] = ipv6_header -> nx_ip_header_source_ip[2];
        router_address.nxd_ip_address.v6[3] = ipv6_header -> nx_ip_header_source_ip[3];


        /* Delete it from our default router table.*/
        _nxd_ipv6_default_router_delete(ip_ptr, &router_address);
    }

    /* Does the RA packet have a valid retrans_timer? */
    if (ra_ptr -> nx_icmpv6_ra_retrans_time)
    {
        /* Pickup the retrans_time value.*/
        time_val = ra_ptr -> nx_icmpv6_ra_retrans_time;

        /* Byte swapping. */
        NX_CHANGE_ULONG_ENDIAN(time_val);

        /* Yes; Reset our re-trans timer. */
        /* Conver timer ticks (in ms) into IP fast timeout value. */
        ip_ptr -> nx_ipv6_retrans_timer_ticks = time_val * NX_IP_FAST_TIMER_RATE / 1000;

        /* If the retrans_timer is smaller than tick resolution, set it to 1.  */
        if (ip_ptr -> nx_ipv6_retrans_timer_ticks == 0)
        {
            ip_ptr -> nx_ipv6_retrans_timer_ticks = 1;
        }
    }

    /* Does the router advertisement have a valid reachable time? */
    if (ra_ptr -> nx_icmpv6_ra_reachable_time)
    {


        /* Yes; set a local variable to store it. */
        time_val = ra_ptr -> nx_icmpv6_ra_reachable_time;

        /* Byte swapping. */
        NX_CHANGE_ULONG_ENDIAN(time_val);

        /* Convert reachable timer to seconds. */
        ip_ptr -> nx_ipv6_reachable_timer = time_val / 1000;

        /* In case the reachable timer is less than 1 second, set reachable timer to 1 second. */
        if (ip_ptr -> nx_ipv6_reachable_timer == 0)
        {
            ip_ptr -> nx_ipv6_reachable_timer = 1;
        }
    }

    if (IPv6_Address_Type(ipv6_header -> nx_ip_header_destination_ip) & IPV6_ALL_NODE_MCAST)
    {

        /* The destination address points to all-nodes (unsolicited) multicast.  Therefore this
           is a periodic refresh. */
        router_type = NX_IPV6_ROUTE_TYPE_UNSOLICITATED;
    }
    else
    {
        router_type = NX_IPV6_ROUTE_TYPE_SOLICITATED;
    }

    /* (2) Process option field */
    packet_length = (INT)packet_ptr -> nx_packet_length - (INT)sizeof(NX_ICMPV6_RA);

    /*lint -e{923} suppress cast between pointer and ULONG, since it is necessary  */
    option_ptr = (NX_ICMPV6_OPTION *)NX_UCHAR_POINTER_ADD(ra_ptr, sizeof(NX_ICMPV6_RA));

    /* Going through the rest of the packet options. */
    while (packet_length > 0)
    {

        /* If there are prefix info options, we pick the last one. */

        /* Is the current option a prefix option? */
        if (option_ptr -> nx_icmpv6_option_type == ICMPV6_OPTION_TYPE_PREFIX_INFO)
        {

            /* Validate option length before cast to avoid OOB access. */
            if ((UINT)(option_ptr -> nx_icmpv6_option_length << 3) < sizeof(NX_ICMPV6_OPTION_PREFIX))
            {
#ifndef NX_DISABLE_ICMP_INFO

                /* Increment the ICMP invalid packet error. */
                ip_ptr -> nx_ip_icmp_invalid_packets++;
#endif /* NX_DISABLE_ICMP_INFO */

                _nx_packet_release(packet_ptr);
                return;
            }

            /* Yes, set a local pointer to the option. */
            /*lint -e{929} -e{826} -e{740} suppress cast of pointer to pointer, since it is necessary  */
            prefix_ptr = (NX_ICMPV6_OPTION_PREFIX *)option_ptr;

            /* Take care of the endian-ness of the prefix address. */
            NX_IPV6_ADDRESS_CHANGE_ENDIAN(prefix_ptr -> nx_icmpv6_option_prefix);

            /* Is this a link local address (prefix)?  */
            if ((prefix_ptr -> nx_icmpv6_option_prefix[0] & (ULONG)0xFFC00000) == (ULONG)0xFE800000)
            {

                /* Yes.  Ignore (skip) this option, as per  RFC 4861 6.3.4
                   and RFC 4862 5.5.3, and continue. */
                packet_length -= (option_ptr -> nx_icmpv6_option_length << 3);

                /* Get the next option. */
                /*lint -e{923} suppress cast between pointer and ULONG, since it is necessary  */
                option_ptr = (NX_ICMPV6_OPTION *)NX_UCHAR_POINTER_ADD(option_ptr, ((option_ptr -> nx_icmpv6_option_length) << 3));

                continue;
            }

            /* So far the prefix information is valid.
               So take care of the endian-ness. */
            NX_CHANGE_ULONG_ENDIAN(prefix_ptr -> nx_icmpv6_option_prefix_valid_lifetime);
            NX_CHANGE_ULONG_ENDIAN(prefix_ptr -> nx_icmpv6_option_prefix_preferred_lifetime);

            /* Does the prefix have a valid lifetime? */
            if (prefix_ptr -> nx_icmpv6_option_prefix_preferred_lifetime > prefix_ptr -> nx_icmpv6_option_prefix_valid_lifetime)
            {

                /* Ignore this option, according to RFC 4862 5.5.3(c) and continue. */
                packet_length -= (option_ptr -> nx_icmpv6_option_length << 3);

                /* Get a pointer to the next option. Abort processing the current option any further. */
                /*lint -e{923} suppress cast between pointer and ULONG, since it is necessary  */
                option_ptr = (NX_ICMPV6_OPTION *)NX_UCHAR_POINTER_ADD(option_ptr, ((option_ptr -> nx_icmpv6_option_length) << 3));

                continue;
            }

            /* Determine whether or not the on-link flag is set. */
            /* Ignore the prefix information option when it is link-local. Page 55, Section 6.3.4, RFC 4861. */
            if (prefix_ptr -> nx_icmpv6_option_prefix_flag & 0x80)
            {

                /* This prefix option contains onlink prefix information. */
                /* The following process follows RFC 4861 6.3.4 */

                prefix_length = prefix_ptr -> nx_icmpv6_option_prefix_length;

                if (prefix_ptr -> nx_icmpv6_option_prefix_valid_lifetime == 0)
                {

                    /* Invalidate the prefix list entry, RFC 4861 6.3.4, p55. */
                    /* Stateless address with this prefix is deleted when prefix is deleted. */
                    _nx_ipv6_prefix_list_delete(ip_ptr, prefix_ptr -> nx_icmpv6_option_prefix, prefix_length);
                }
                else
                {

                    /* This prefix is onlink, and valid_lifetime is non-zero.
                       So add the prefix to our list. RFC 4861 6.3.4 p55.*/
                    status = _nx_ipv6_prefix_list_add_entry(ip_ptr, prefix_ptr -> nx_icmpv6_option_prefix,
                                                            (ULONG)prefix_length, prefix_ptr -> nx_icmpv6_option_prefix_valid_lifetime);

                    /* Check for "A" bit. */
                    if ((prefix_ptr -> nx_icmpv6_option_prefix_flag & 0x40) &&
                        (prefix_ptr -> nx_icmpv6_option_prefix_length == (128 - NX_IPV6_HOST_ID_LENGTH)) &&
                        (status == NX_SUCCESS))
                    {
                    /* The autonomous flag is set */

                    /* Set first_unused to be an invalid entry. */
                    UINT              first_unused = NX_MAX_IPV6_ADDRESSES;
                    ULONG             word2, word3;
                    ULONG             address[4];
                    NXD_IPV6_ADDRESS *ipv6_address;

                        /* Find an entry that shares the same prefix. */
                        /* Note: RFC 4862 5.5.3(d) specifies that the search is limited to IPv6
                           addresses formed by address autoconfiguration.  Towards the end of 5.5.3(d),
                           the RFC explains that the search may still lead to address conflict (by not
                           searching for addresses configured manually or via DHCP.  Therefore this
                           implemenation chooses to search the entire IPv6 global address, in order to
                           avoid conflict IP addresses. */
                        for (i = 0; i < NX_MAX_IPV6_ADDRESSES; i++)
                        {

                            if (ip_ptr -> nx_ipv6_address[i].nxd_ipv6_address_valid == NX_FALSE)
                            {

                                /* Found the first unused entry. */
                                first_unused = i;
                                break;
                            }
                        }

                        if ((first_unused != NX_MAX_IPV6_ADDRESSES)
#ifdef NX_IPV6_STATELESS_AUTOCONFIG_CONTROL
                            && (if_ptr -> nx_ipv6_stateless_address_autoconfig_status == NX_STATELESS_ADDRESS_AUTOCONFIG_ENABLED)
#endif /* NX_IPV6_STATELESS_AUTOCONFIG_CONTROL */
                           )
                        {

                            /* If there are no global addresses with such a prefix, and there is an unused entry,
                               a new global address is formed. RFC 4862 5.5.3(d), p18 */

                            ipv6_address = &ip_ptr -> nx_ipv6_address[first_unused];

                            /* Find 64-bit interface ID. See RFC 4291 */
                            word2 = if_ptr -> nx_interface_physical_address_msw << 16 |
                                ((if_ptr -> nx_interface_physical_address_lsw & 0xFF000000) >> 16) | 0xFF;

                            /* Fix the 2nd lower-order bit of the 1st byte */
                            word2 = (word2 & 0xFDFFFFFF) | (~(word2 | 0xFDFFFFFF));
                            word3 = (if_ptr -> nx_interface_physical_address_lsw & 0x00FFFFFF) | 0xFE000000;

                            ipv6_address -> nxd_ipv6_address_valid = NX_TRUE;
                            ipv6_address -> nxd_ipv6_address_type = NX_IP_VERSION_V6;

                            ipv6_address -> nxd_ipv6_address_attached = if_ptr;
                            ipv6_address -> nxd_ipv6_address[0] = prefix_ptr -> nx_icmpv6_option_prefix[0];
                            ipv6_address -> nxd_ipv6_address[1] = prefix_ptr -> nx_icmpv6_option_prefix[1];
                            ipv6_address -> nxd_ipv6_address[2] = word2;
                            ipv6_address -> nxd_ipv6_address[3] = word3;

                            ipv6_address -> nxd_ipv6_address_next = if_ptr -> nxd_interface_ipv6_address_list_head;
                            if_ptr -> nxd_interface_ipv6_address_list_head = ipv6_address;

#ifndef NX_DISABLE_IPV6_DAD
                            /* Set the address to Tentative, so the stack can start DAD process. */
                            ipv6_address -> nxd_ipv6_address_state = NX_IPV6_ADDR_STATE_TENTATIVE;
#else /* !NX_DISABLE_IPV6_DAD */
                            /* DAD is disabled.  Set the address to VALID. */
                            ipv6_address -> nxd_ipv6_address_state = NX_IPV6_ADDR_STATE_VALID;
#endif /* NX_DISABLE_IPV6_DAD */

                            /* Join the solicited-node multicast group */
                            /* FF02::1:FFXX:XXXX */
                            SET_SOLICITED_NODE_MULTICAST_ADDRESS(address, ipv6_address -> nxd_ipv6_address);
                            _nx_ipv6_multicast_join(ip_ptr, address, ipv6_address -> nxd_ipv6_address_attached);

                            ipv6_address -> nxd_ipv6_address_prefix_length = (UCHAR)prefix_length;
                            ipv6_address -> nxd_ipv6_address_ConfigurationMethod = NX_IPV6_ADDRESS_BASED_ON_INTERFACE;
#ifndef NX_DISABLE_IPV6_DAD
                            ipv6_address -> nxd_ipv6_address_DupAddrDetectTransmit = NX_IPV6_DAD_TRANSMITS - 1;
#endif /* NX_DISABLE_IPV6_DAD */

#ifdef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
                            if (ip_ptr -> nx_ipv6_address_change_notify)
                            {
                                interface_index = if_ptr -> nx_interface_index;
                                (ip_ptr -> nx_ipv6_address_change_notify)(ip_ptr, NX_IPV6_ADDRESS_STATELESS_AUTO_CONFIG, interface_index,
                                                                          first_unused, &ipv6_address -> nxd_ipv6_address[0]);
                            }
                            if ((ip_ptr -> nx_ipv6_address_change_notify_internal) && (ipv6_address -> nxd_ipv6_address_state == NX_IPV6_ADDR_STATE_VALID))
                            {
                                interface_index = if_ptr -> nx_interface_index;
                                (ip_ptr -> nx_ipv6_address_change_notify_internal)(ip_ptr, NX_IPV6_ADDRESS_STATELESS_AUTO_CONFIG, interface_index,
                                                                                   first_unused, &ipv6_address -> nxd_ipv6_address[0]);
                            }
#endif /* NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY */
                        }
                    }
                }
            }
        }
        else if (option_ptr -> nx_icmpv6_option_type == ICMPV6_OPTION_TYPE_SRC_LINK_ADDR)
        {

            status = _nx_nd_cache_find_entry(ip_ptr, ipv6_header -> nx_ip_header_source_ip, &nd_entry);

            /* Find the IP address from our local nd cache. */
            if (status != NX_SUCCESS)
            {

                /* If the source IP address is not in our nd cache, add it. */
                /*lint -e{929} -e{826} -e{740} suppress cast of pointer to pointer, since it is necessary  */
                _nx_nd_cache_add(ip_ptr, ipv6_header -> nx_ip_header_source_ip, if_ptr,
                                 (CHAR *)&option_ptr -> nx_icmpv6_option_data, 0, ND_CACHE_STATE_STALE,
                                 packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr, &nd_entry);
            }
            else
            {
            /* This entry already exists.  If the mac address is the same, do not update the entry.
               Otherwise, update the entry and set the state to STALE (RFC2461 7.2.3) */
            ULONG mac_msw, mac_lsw, new_msw, new_lsw;

            /*lint -e{928} suppress cast from pointer to pointer, since it is necessary  */
            UCHAR *new_mac = (UCHAR *)&option_ptr -> nx_icmpv6_option_data;

                /* build two MAC addresses for comparison. */
                /*lint --e{613} -e{644} suppress possible use of null pointer, since "nd_entry" was set to none NULL by _nx_nd_cache_find_entry. */
                mac_msw = ((ULONG)(nd_entry -> nx_nd_cache_mac_addr[0]) << 8) | (nd_entry -> nx_nd_cache_mac_addr[1]);
                mac_lsw = ((ULONG)(nd_entry -> nx_nd_cache_mac_addr[2]) << 24) | ((ULONG)(nd_entry -> nx_nd_cache_mac_addr[3]) << 16) |
                    ((ULONG)(nd_entry -> nx_nd_cache_mac_addr[4]) << 8) | nd_entry -> nx_nd_cache_mac_addr[5];
                new_msw = ((ULONG)(new_mac[0]) << 8) | (new_mac[1]);
                new_lsw = ((ULONG)(new_mac[2]) << 24) | ((ULONG)(new_mac[3]) << 16) | ((ULONG)(new_mac[4]) << 8) | new_mac[5]; /* lgtm[cpp/overflow-buffer] */
                if ((mac_msw != new_msw) || (mac_lsw != new_lsw))
                {

                    /* Router updates its MAC address.  We update our entry as well, and place the
                       entry into STALE state for a quick address check. */

                    /* Set the mac address. */
                    for (i = 0; i < 6; i++)
                    {
                        nd_entry -> nx_nd_cache_mac_addr[i] = new_mac[i];
                    }

                    /* Set the state to STALE.  */
                    nd_entry -> nx_nd_cache_nd_status = ND_CACHE_STATE_STALE;

                    /* Set the interface. */
                    nd_entry -> nx_nd_cache_interface_ptr = if_ptr;
                }


                /* Since we received source LLA, we shall transmit any packets that are pending
                   address resolution of this target. */
                if (nd_entry -> nx_nd_cache_packet_waiting_head) /* There are packets waiting to be transmitted */
                {

                    _nx_icmpv6_send_queued_packets(ip_ptr, nd_entry);
                }
            }
        }
#ifdef NX_ENABLE_IPV6_PATH_MTU_DISCOVERY

        /* Check for an MTU update from the router. */
        else if (option_ptr -> nx_icmpv6_option_type == ICMPV6_OPTION_TYPE_MTU)
        {

        NX_IPV6_DESTINATION_ENTRY *dest_entry_ptr;
        UINT                       mtu_size;

            /* Get a local pointer to the MTU option data. */
            /*lint -e{929} -e{826} -e{740} suppress cast of pointer to pointer, since it is necessary  */
            mtu_ptr = (NX_ICMPV6_OPTION_MTU *)option_ptr;

            mtu_size = mtu_ptr -> nx_icmpv6_option_mtu_path_mtu;

            NX_CHANGE_ULONG_ENDIAN(mtu_size);


            /* Make sure the MTU size does not exceed the link MTU size. */
            if (mtu_size > if_ptr -> nx_interface_ip_mtu_size)
            {
                mtu_size = if_ptr -> nx_interface_ip_mtu_size;
            }

            /* Add destination table entry. */
            _nx_icmpv6_dest_table_add(ip_ptr, ipv6_header -> nx_ip_header_source_ip, &dest_entry_ptr,
                                      ipv6_header -> nx_ip_header_source_ip /* Next Hop address */,
                                      mtu_size, NX_WAIT_FOREVER,
                                      packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr);
        }
#endif

        /* Update the amount of packet option data remaining. */
        packet_length -= (option_ptr -> nx_icmpv6_option_length << 3);

        /* Get a pointer to the next option. */
        /*lint -e{923} suppress cast between pointer and ULONG , since it is necessary  */
        option_ptr  = (NX_ICMPV6_OPTION *)NX_UCHAR_POINTER_ADD(option_ptr, ((option_ptr -> nx_icmpv6_option_length) << 3));
    }

    /* All options are processed.  No errors encountered.  */

    /* (3) Add this entry to the routing table. */
    if (ra_ptr -> nx_icmpv6_ra_router_lifetime)
    {

        NX_CHANGE_USHORT_ENDIAN(ra_ptr -> nx_icmpv6_ra_router_lifetime);

        /* Add the router into our default router table. */
        _nxd_ipv6_default_router_add_internal(ip_ptr, ipv6_header -> nx_ip_header_source_ip,     /* Next Hop address */
                                              ra_ptr -> nx_icmpv6_ra_router_lifetime, if_ptr,
                                              router_type, &rt_entry);

        /* Link router entry and its corresponding nd_entry.
           Note that at this point, nd_entry may not be valid.
           When a packet is transmitted using this router, and the nd_entry is invalid (NX_NULL)
           ipv6 send routine shall use neighbor discovery process to find the address. */
        /*lint -e{644} suppress variable might not be initialized, since "rt_entry" was initialized in _nxd_ipv6_default_router_add_internal. */
        if (rt_entry && nd_entry)
        {
            rt_entry -> nx_ipv6_default_router_entry_neighbor_cache_ptr = (void *)nd_entry;
            nd_entry -> nx_nd_cache_is_router = rt_entry;
        }

        if (ra_ptr -> nx_icmpv6_ra_hop_limit)
        {
            ip_ptr -> nx_ipv6_hop_limit = ra_ptr -> nx_icmpv6_ra_hop_limit;
        }

#ifdef NX_ENABLE_IPV6_PATH_MTU_DISCOVERY

        /* Special case destination table entry, the router itself.  Note that the destination is the.
           same as the next hop. Adding the router to the destination table  gives us a place to
           store path MTU updates as RAs (router advertisements) containing MTU option data. */

        /* Have we already established path MTU for this router? */
        if (mtu_ptr == 0)
        {

        NX_IPV6_DESTINATION_ENTRY *dest_entry_ptr;

            /* No, create an entry in the destination table for it, and set
               the path MTU to our default path MTU. */
            _nx_icmpv6_dest_table_add(ip_ptr, ipv6_header -> nx_ip_header_source_ip, &dest_entry_ptr,
                                      ipv6_header -> nx_ip_header_source_ip,
                                      if_ptr -> nx_interface_ip_mtu_size, NX_WAIT_FOREVER,
                                      packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr);
        }
#endif
    }

#ifndef NX_DISABLE_ICMPV6_ROUTER_SOLICITATION

    /* Received a valid RS... Stop RA if it is still running. */
    if_ptr -> nx_ipv6_rtr_solicitation_count = 0;

#endif

    /* release packet and return. */
    _nx_packet_release(packet_ptr);
}

#endif /* NX_DISABLE_ICMPV6_ROUTER_ADVERTISEMENT_PROCESS */

#endif /* FEATURE_NX_IPV6 */

