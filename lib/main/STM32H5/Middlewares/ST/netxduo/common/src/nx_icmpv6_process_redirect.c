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



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmpv6_process_redirect                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an incoming ICMPv6 redirect message.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            ICMP packet pointer           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    IPv6_Address_Type                   Find IP address type.           */
/*    _nxd_ipv6_destination_table_find_next_hop                           */
/*                                        Finds next hop address.         */
/*    _nxd_ipv6_router_lookup             Routine that finds default      */
/*                                           router.                      */
/*    _nx_icmpv6_validate_options         Check for valid option headers  */
/*    _nx_nd_cache_find_entry             Find entry in ND cache by IP    */
/*                                           address                      */
/*   _nx_nd_cache_add                     Add entry to ND cache           */
/*   _nxd_ipv6_default_router_add_internal Update router list with        */
/*                                           redirect next hop            */
/*   _nx_icmpv6_send_queued_packets       Transmit queued packets waiting */
/*                                           for physical mapping         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmpv6_packet_process           Main ICMPv6 packet pocess       */
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

#ifndef NX_DISABLE_ICMPV6_REDIRECT_PROCESS

VOID _nx_icmpv6_process_redirect(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

NX_ICMPV6_REDIRECT_MESSAGE   *redirect_ptr;
NX_IPV6_HEADER               *ip_header;
UINT                          status;
ULONG                         source_address_type;
ULONG                         router_address[4];
ULONG                         error = 0;
ND_CACHE_ENTRY               *nd_entry = NX_NULL;
ND_CACHE_ENTRY               *NDCacheEntry = NX_NULL;
UINT                          i;
NX_ICMPV6_OPTION             *option_ptr;
ULONG                         packet_length, option_length;
NX_IPV6_DEFAULT_ROUTER_ENTRY *rt_entry;
NX_INTERFACE                 *interface_ptr;
#ifdef NX_ENABLE_IPV6_PATH_MTU_DISCOVERY
NX_ICMPV6_OPTION_MTU         *mtu_ptr;
#endif
ULONG                         mtu;
ULONG                         mtu_timeout;
NX_IPV6_DESTINATION_ENTRY    *dest_entry_ptr;

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

#ifndef NX_DISABLE_RX_SIZE_CHECKING
    if (packet_ptr -> nx_packet_length < sizeof(NX_ICMPV6_REDIRECT_MESSAGE))
    {
#ifndef NX_DISABLE_ICMP_INFO

        /* Increment the ICMP invalid message count.  */
        ip_ptr -> nx_ip_icmp_invalid_packets++;
#endif

        /* Invalid ICMP message, just release it.  */
        _nx_packet_release(packet_ptr);
        return;
    }
#endif /* NX_DISABLE_RX_SIZE_CHECKING */

    /* Get interface pointer. */
    interface_ptr = packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_attached;

    /* Initialize the MTU. */
    mtu = interface_ptr -> nx_interface_ip_mtu_size;
    mtu_timeout = NX_WAIT_FOREVER;

    /* Locate the IPv6 header. */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    ip_header = (NX_IPV6_HEADER *)(packet_ptr -> nx_packet_ip_header);

    /* Locate the redirect message. */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    redirect_ptr = (NX_ICMPV6_REDIRECT_MESSAGE *)(packet_ptr -> nx_packet_prepend_ptr);

    /* RFC 2461 4.5 the IPv6 and ICMPv6 header fields. */

    /* Convert the destination and target address to host byte order. */
    NX_IPV6_ADDRESS_CHANGE_ENDIAN(redirect_ptr -> nx_icmpv6_redirect_target_address);
    NX_IPV6_ADDRESS_CHANGE_ENDIAN(redirect_ptr -> nx_icmpv6_redirect_destination_address);

    /* Verify that sender address is link-local address. */
    source_address_type = IPv6_Address_Type(ip_header -> nx_ip_header_source_ip);
    if ((source_address_type & 0xFF) != IPV6_ADDRESS_LINKLOCAL)
    {
        error = 1;
    }
    /* Verify that hop limit is 255 */
    else if ((ip_header -> nx_ip_header_word_1 & 0xFF) != 0xFF)
    {
        error = 1;
    }
    /* Verifty that ICMP code is zero. */
    else if (redirect_ptr -> nx_icmpv6_redirect_icmpv6_header.nx_icmpv6_header_code != 0)
    {
        error = 1;
    }

    /* The IP source address of the redirect is the same as the current first-hop
       router for the specified ICMP destination address. */

    /* First look for redirect in the destination table. */
    if (_nxd_ipv6_destination_table_find_next_hop(ip_ptr, redirect_ptr -> nx_icmpv6_redirect_destination_address,
                                                  router_address) == NX_SUCCESS)
    {

        /* Make sure the source is the current 1st hop router. */
        if (!CHECK_IPV6_ADDRESSES_SAME(router_address, ip_header -> nx_ip_header_source_ip))
        {
            error = 1;
        }
    }
    /* Next check if the redirect is the default router. */
    /* Suppress cast of pointer to pointer, since it is necessary  */
    else if (_nxd_ipv6_router_lookup(ip_ptr, interface_ptr, router_address, /*lint -e{929}*/ (void **)&NDCacheEntry))
    {
        /* No default router. */
        error = 1;
    }

    /* Make sure the source is the current 1st hop router. */
    if (!CHECK_IPV6_ADDRESSES_SAME(ip_header -> nx_ip_header_source_ip, router_address))
    {
        error = 1;
    }
    /* The destination address field may not be multicast. */
    else if (IPv6_Address_Type(redirect_ptr -> nx_icmpv6_redirect_destination_address) & IPV6_ADDRESS_MULTICAST)
    {
        error = 1;
    }
    /* Is target address is different from destination address?  */
    else if (!(CHECK_IPV6_ADDRESSES_SAME(redirect_ptr -> nx_icmpv6_redirect_destination_address,
                                         redirect_ptr -> nx_icmpv6_redirect_target_address)))
    {

        /* Yes, so the target is a router, therefore the target address must be link-local.*/
        if ((IPv6_Address_Type(redirect_ptr -> nx_icmpv6_redirect_target_address) & IPV6_ADDRESS_LINKLOCAL) == 0)
        {
            /* It isn't. */
            error = 1;
        }
    }

    /* Release the packet if it fails these validation checks. */
    if (error)
    {

#ifndef NX_DISABLE_ICMP_INFO

        /* Increment the ICMP invalid packet error. */
        ip_ptr -> nx_ip_icmp_invalid_packets++;
#endif /* NX_DISABLE_ICMP_INFO */

        _nx_packet_release(packet_ptr);
        return;
    }

    /* If there are additional options, it could be TLLA or the redirected header. */
    if (packet_ptr -> nx_packet_length - sizeof(NX_ICMPV6_REDIRECT_MESSAGE))
    {

        /*lint -e{923} suppress cast between pointer and ULONG, since it is necessary  */
        option_ptr = (NX_ICMPV6_OPTION *)NX_UCHAR_POINTER_ADD(redirect_ptr, sizeof(NX_ICMPV6_REDIRECT_MESSAGE));
        packet_length = packet_ptr -> nx_packet_length - (ULONG)sizeof(NX_ICMPV6_REDIRECT_MESSAGE);

        /* Validate option fields. */
        if (_nx_icmpv6_validate_options(option_ptr, (INT)packet_length, 0) == NX_NOT_SUCCESSFUL)
        {

#ifndef NX_DISABLE_ICMP_INFO

            /* Increment the ICMP invalid packet error. */
            ip_ptr -> nx_ip_icmp_invalid_packets++;
#endif /* NX_DISABLE_ICMP_INFO */

            _nx_packet_release(packet_ptr);
            return;
        }

        while (packet_length)
        {

            /* The packet contains target link layer address. */
            if (option_ptr -> nx_icmpv6_option_type == ICMPV6_OPTION_TYPE_TRG_LINK_ADDR)
            {
                status = _nx_nd_cache_find_entry(ip_ptr, redirect_ptr -> nx_icmpv6_redirect_target_address, &nd_entry);


                if (status != NX_SUCCESS)
                {

                    /* The entry is not found in the ND cache table.  Simply add it, and we are done. */
                    /*lint -e{929} -e{740} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                    if (_nx_nd_cache_add(ip_ptr, redirect_ptr -> nx_icmpv6_redirect_target_address,
                                         interface_ptr,
                                         (CHAR *)&option_ptr -> nx_icmpv6_option_data, 0, ND_CACHE_STATE_STALE,
                                         packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr, &nd_entry) == NX_SUCCESS)
                    {

                        /* If the destination and target addresses are different, the redirect is to a router. */
                        if (!CHECK_IPV6_ADDRESSES_SAME(redirect_ptr -> nx_icmpv6_redirect_target_address,
                                                       redirect_ptr -> nx_icmpv6_redirect_destination_address))
                        {

                            /* We need to fill in the IsRouter field in the cache entry
                               with the default router. */
                            _nxd_ipv6_default_router_add_internal(ip_ptr,
                                                                  redirect_ptr -> nx_icmpv6_redirect_target_address, /* TLLA in Redirect ICMPv6 options */
                                                                  0,
                                                                  interface_ptr,
                                                                  NX_IPV6_ROUTE_TYPE_UNSOLICITATED, &rt_entry);

                            /* And cross link this entry with the entry in the router list. */
                            /*lint -e{644} suppress variable might not be initialized, since "rt_entry" was initialized in _nxd_ipv6_default_router_add_internal. */
                            /*lint -e{613} suppress possible use of null pointer, since "nd_entry" was set in _nx_nd_cache_add. */
                            nd_entry -> nx_nd_cache_is_router = rt_entry;
                        }
                    }
                }
                else
                {

                /* This entry already exists.  If the mac address is the same, do not update the entry. Otherwise,
                   update the entry and set the state to STALE (RFC2461 7.2.3) */
                ULONG mac_msw, mac_lsw, new_msw, new_lsw;

                /*lint -e{928} suppress cast of pointer to pointer, since it is necessary  */
                UCHAR *new_mac = (UCHAR *)&option_ptr -> nx_icmpv6_option_data;


                    mac_msw = ((ULONG)(nd_entry -> nx_nd_cache_mac_addr[0]) << 8) | (nd_entry -> nx_nd_cache_mac_addr[1]);
                    new_msw = ((ULONG)(new_mac[0]) << 8) | (new_mac[1]);
                    mac_lsw = ((ULONG)(nd_entry -> nx_nd_cache_mac_addr[2]) << 24) | ((ULONG)(nd_entry -> nx_nd_cache_mac_addr[3]) << 16) |
                        ((ULONG)(nd_entry -> nx_nd_cache_mac_addr[4]) << 8) | nd_entry -> nx_nd_cache_mac_addr[5];
                    new_lsw = ((ULONG)(new_mac[2]) << 24) | ((ULONG)(new_mac[3]) << 16) | ((ULONG)(new_mac[4]) << 8) | new_mac[5]; /* lgtm[cpp/overflow-buffer] */

                    if ((mac_msw != new_msw) || (mac_lsw != new_lsw)) /* If the new MAC is different. */
                    {

                        /* Update the MAC address. */
                        for (i = 0; i < 6; i++)
                        {
                            nd_entry -> nx_nd_cache_mac_addr[i] = new_mac[i];
                        }

                        /* Set the state to STALE.  */
                        nd_entry -> nx_nd_cache_nd_status = ND_CACHE_STATE_STALE;

                        /* Set the interface. */
                        nd_entry -> nx_nd_cache_interface_ptr = interface_ptr;
                    }

                    /* If there are packets chained on the entry waiting to be transmitted:  */
                    if (nd_entry -> nx_nd_cache_packet_waiting_head) /* There are packets waiting to be transmitted */
                    {
                        _nx_icmpv6_send_queued_packets(ip_ptr, nd_entry);
                    }
                }
            }

#ifdef NX_ENABLE_IPV6_PATH_MTU_DISCOVERY
            else if (option_ptr -> nx_icmpv6_option_type == ICMPV6_OPTION_TYPE_MTU)
            {

                /* Get a local pointer to the MTU option. */
                /*lint -e{929} -e{740} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                mtu_ptr = (NX_ICMPV6_OPTION_MTU *)option_ptr;

                /* Apply the router's next hop (on link) path MTU. */
                mtu = mtu_ptr -> nx_icmpv6_option_mtu_path_mtu;
                NX_CHANGE_ULONG_ENDIAN(mtu);

                if (mtu < NX_MINIMUM_IPV6_PATH_MTU)
                {

                    /* Update the timeout of MTU. */
                    mtu_timeout = NX_PATH_MTU_INCREASE_WAIT_INTERVAL_TICKS;
                }
            }
#endif  /* NX_ENABLE_IPV6_PATH_MTU_DISCOVERY */

            option_length = (UINT)(option_ptr -> nx_icmpv6_option_length << 3);
            packet_length -= option_length;

            /*lint -e{923} suppress cast between pointer and ULONG, since it is necessary  */
            option_ptr = (NX_ICMPV6_OPTION *)NX_UCHAR_POINTER_ADD(option_ptr, option_length);
        }
    }

    /*
     * If the packet contains target link layer address, we should have added the link layer address
     * to the ND cache, and the next block is not executed.
     * However if the packet does not contain target link layer address (thus nd_cache is NULL at
     * this point, we create an entry.
     */
    if (nd_entry == NX_NULL)
    {
        status = _nx_nd_cache_find_entry(ip_ptr, redirect_ptr -> nx_icmpv6_redirect_target_address, &nd_entry);

        if (status != NX_SUCCESS)
        {
            if (_nx_nd_cache_add_entry(ip_ptr, redirect_ptr -> nx_icmpv6_redirect_target_address,
                                       packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr, &nd_entry) != NX_SUCCESS)
            {

#ifndef NX_DISABLE_ICMP_INFO

                /* Increment the ICMP invalid packet error. */
                ip_ptr -> nx_ip_icmp_invalid_packets++;
#endif /* NX_DISABLE_ICMP_INFO */

                /* Release the packet. */
                _nx_packet_release(packet_ptr);

                return;
            }
        }
    }

    /* Find/update existing entry or add a new one for this router in the destination table.
       A new entry will be assigned default path MTU and MTU timeout values.  */
    status = _nx_icmpv6_dest_table_add(ip_ptr, redirect_ptr -> nx_icmpv6_redirect_destination_address,
                                       &dest_entry_ptr, redirect_ptr -> nx_icmpv6_redirect_target_address,
                                       mtu, mtu_timeout,
                                       packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr);
    if (status)
    {

#ifndef NX_DISABLE_ICMP_INFO
        /* Increment the ICMP invalid packet error. */
        ip_ptr -> nx_ip_icmp_invalid_packets++;
#endif /* NX_DISABLE_ICMP_INFO */

        /* Release the packet. */
        _nx_packet_release(packet_ptr);

        return;
    }

    /* Cross link the nd_entry. */
    /*lint -e{644} suppress variable might not be initialized, since "dest_entry_ptr" was initialized _nx_icmpv6_dest_table_find or _nx_icmpv6_dest_table_add. */
    dest_entry_ptr -> nx_ipv6_destination_entry_nd_entry = nd_entry;

    /* Release the packet. */
    _nx_packet_release(packet_ptr);
}
#endif  /* NX_DISABLE_ICMPV6_REDIRECT_PROCESS */



#endif /* FEATURE_NX_IPV6 */

