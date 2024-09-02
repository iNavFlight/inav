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
#include "nx_ipv6.h"
#include "nx_icmpv6.h"

#ifdef NX_IPSEC_ENABLE
#include "nx_ipsec.h"
#endif /* NX_IPSEC_ENABLE */


#ifdef FEATURE_NX_IPV6


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmpv6_process_na                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function processes incoming neighbor advertisement    */
/*    messages. It also updates the ND cache and the default router list. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                            IP stack instance                 */
/*    packet_ptr                        Received NA packet                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_release                  Release packet back to pool     */
/*    _nx_nd_cache_add                    Add entry to cache table        */
/*    _nx_icmpv6_send_queued_packets      Send out packets queued waiting */
/*                                           for physical mapping to IP   */
/*                                           address                      */
/*    _nx_icmpv6_validate_neighbor_message Validate ICMPv6 packet         */
/*    _nx_icmpv6_DAD_failure              Mark IP address status invalid  */
/*    _nx_nd_cache_find_entry             Find entry based on dest address*/
/*    _nxd_ipv6_find_default_router_from_address                          */
/*                                        Find default router by address  */
/*    _nxd_ipv6_default_router_delete     Clear the default router bound  */
/*                                           to the IP                    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmpv6_process                                                  */
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
VOID _nx_icmpv6_process_na(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

ND_CACHE_ENTRY   *nd_entry = NX_NULL;

/*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
NX_ICMPV6_ND     *nd_ptr = (NX_ICMPV6_ND *)(packet_ptr -> nx_packet_prepend_ptr);
NX_ICMPV6_OPTION *option_ptr = NX_NULL;
INT               error = 0;
INT               lla_same = 0;
UINT              option_length;
NX_IPV6_HEADER   *ipv6_header_ptr;
#ifndef NX_DISABLE_IPV6_DAD
UINT              i;
#endif /* NX_DISABLE_IPV6_DAD */

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* Take care of endian-ness. */
    NX_IPV6_ADDRESS_CHANGE_ENDIAN(nd_ptr -> nx_icmpv6_nd_targetAddress);
    NX_CHANGE_ULONG_ENDIAN(nd_ptr -> nx_icmpv6_nd_flag);

    /* Validate the neighbor advertisement message. */
    if (_nx_icmpv6_validate_neighbor_message(packet_ptr) != NX_SUCCESS)
    {
        error = 1;
    }
    else
    {

        /* Points to the IPv6 header. */
        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        ipv6_header_ptr = (NX_IPV6_HEADER *)packet_ptr -> nx_packet_ip_header;

        /* Find the option field. */
        /*lint -e{923} suppress cast between pointer and ULONG, since it is necessary  */
        option_ptr = (NX_ICMPV6_OPTION *)NX_UCHAR_POINTER_ADD(nd_ptr, sizeof(NX_ICMPV6_ND));
        option_length = (UINT)packet_ptr -> nx_packet_length - (UINT)sizeof(NX_ICMPV6_ND);

        /* Find the TLLA option */
        while (option_length > 0)
        {
            /* Check if this is a Target LLA option. */
            if (option_ptr -> nx_icmpv6_option_type == ICMPV6_OPTION_TYPE_TRG_LINK_ADDR)
            {
                break;
            }

            /* Get the next option. */
            option_length -= ((UINT)(option_ptr -> nx_icmpv6_option_length) << 3);

            /*lint -e{923} suppress cast between pointer and ULONG, since it is necessary  */
            option_ptr = (NX_ICMPV6_OPTION *)NX_UCHAR_POINTER_ADD(option_ptr, ((option_ptr -> nx_icmpv6_option_length) << 3));
        }

        /* Check for no option included. */
        if (option_length == 0)
        {

            option_ptr = NX_NULL;
        }

        /* Determine the NA packet destination type. */
        /* Is the destination a multicast address? */
        if ((ipv6_header_ptr -> nx_ip_header_destination_ip[0] & (ULONG)0xFF000000) == (ULONG)0xFF000000)
        {

            /* Yes; Were there any options in the NA packet? */
            if (!option_ptr)
            {

                /* No, this is an invalid NA packet (No TLLA). */
                error = 1;
            }
        }

#ifndef NX_DISABLE_IPV6_DAD

        /* Find the same address as target address in IPv6 address structure.
           Assume target address is 4-byte aligned.*/
        for (i = 0; i < NX_MAX_IPV6_ADDRESSES; i++)
        {

            if (CHECK_IPV6_ADDRESSES_SAME(ip_ptr -> nx_ipv6_address[i].nxd_ipv6_address,
                                          nd_ptr -> nx_icmpv6_nd_targetAddress))
            {
                if (ip_ptr -> nx_ipv6_address[i].nxd_ipv6_address_state == NX_IPV6_ADDR_STATE_TENTATIVE)
                {

                    /* Sender sends a NS in response to a DAD request we sent out.
                       Mark DAD failure. */
                    _nx_icmpv6_DAD_failure(ip_ptr, &ip_ptr -> nx_ipv6_address[i]);

#ifndef NX_DISABLE_ICMP_INFO

                    /* Increment the ICMP invalid packet error. */
                    ip_ptr -> nx_ip_icmp_invalid_packets++;
#endif /* NX_DISABLE_ICMP_INFO */

                    /* Release the packet and we are done. */
                    _nx_packet_release(packet_ptr);
                    return;
                }

                break;
            }
        }
#endif /* NX_DISABLE_IPV6_DAD */

        /* Find the ND entry */
        if (_nx_nd_cache_find_entry(ip_ptr, nd_ptr -> nx_icmpv6_nd_targetAddress, &nd_entry) != NX_SUCCESS)
        {
            /* The entry does not exist, it indicates that we are not
               expecting this NA.  So silently ignore this packet,
               according to RFC 2461 7.2.5 */
            error = 1;
        }
    }

    /* Do not process the NA any further if any errors detected. */
    if (error)
    {

#ifndef NX_DISABLE_ICMP_INFO

        /* Increment the ICMP invalid packet error. */
        ip_ptr -> nx_ip_icmp_invalid_packets++;
#endif /* NX_DISABLE_ICMP_INFO */

        /* Release the packet and we are done. */
        _nx_packet_release(packet_ptr);
        return;
    }

    /* Check whether or not supplied LLA is the same as the cached one */
    if (option_ptr)
    {

    /* Compare the link-layer address. */
    USHORT *new_lla, *lla;

        /*lint -e{929} suppress cast of pointer to pointer, since it is necessary  */
        new_lla = (USHORT *)&option_ptr -> nx_icmpv6_option_data;

        /*lint -e{927} suppress cast of pointer to pointer, since it is necessary  */
        /*lint -e{644} suppress variable might not be initialized, since "nd_entry" was initialized in _nx_nd_cache_find_entry. */
        lla = (USHORT *)nd_entry -> nx_nd_cache_mac_addr;
        if ((new_lla[0] == lla[0]) && (new_lla[1] == lla[1]) && (new_lla[2] == lla[2])) /* lgtm[cpp/overflow-buffer] */
        {

            /* No change in LLA. */
            lla_same = 1;
        }
    }

    /* Determine if the entry is a router. */
    if (nd_ptr -> nx_icmpv6_nd_flag & 0x80000000)
    {

        /* Yes; Does the corresonding ND entry have the IsRouter flag set? */
        if (nd_entry -> nx_nd_cache_is_router == NX_NULL)
        {

        NX_IPV6_DEFAULT_ROUTER_ENTRY *rt_entry;

            /* No; Find the default router entry. */
            rt_entry =
                _nxd_ipv6_find_default_router_from_address(ip_ptr, nd_ptr -> nx_icmpv6_nd_targetAddress);

            /* Check the default router.  */
            if (rt_entry)
            {

                /* Set the IsRouter flag in the ND entry. */
                nd_entry -> nx_nd_cache_is_router = rt_entry;

                /* Set this link as the corresponding ND entry. */
                rt_entry -> nx_ipv6_default_router_entry_neighbor_cache_ptr = (VOID *)nd_entry;
            }
        }
    }
    else
    {

        /*
         * The neighbor advertisement message indicates that it is not a router.
         * However if our ND cache still marks it as a router, that means the neighbor is
         * no longer acting as a router, and we shall clean up our records.
         */
        if (nd_entry -> nx_nd_cache_is_router)
        {

        NXD_ADDRESS router_address;
        UINT        clear_router_flag = 1;

            /* Only if the TLLA option indicates the TLLA is unchanged! */
            if (option_ptr && !lla_same)
            {

                /* And only if the override bit not is set. */
                if ((nd_ptr -> nx_icmpv6_nd_flag & 0x20000000) == 0)
                {
                    clear_router_flag = 0;
                }
            }

            /* Did we decide to clear the router status of this cache entry? */
            if (clear_router_flag)
            {

                /* Yes, Ok to clear the cache entry router status! */

                /* The IsRouter points to the entry in the default router table.
                   We first remove the link between the router table entry and the nd cache entry. */
                nd_entry -> nx_nd_cache_is_router -> nx_ipv6_default_router_entry_neighbor_cache_ptr = NX_NULL;


                /* Remove the entry from the default router list. */
                router_address.nxd_ip_version = NX_IP_VERSION_V6;
                router_address.nxd_ip_address.v6[0] = nd_ptr -> nx_icmpv6_nd_targetAddress[0];
                router_address.nxd_ip_address.v6[1] = nd_ptr -> nx_icmpv6_nd_targetAddress[1];
                router_address.nxd_ip_address.v6[2] = nd_ptr -> nx_icmpv6_nd_targetAddress[2];
                router_address.nxd_ip_address.v6[3] = nd_ptr -> nx_icmpv6_nd_targetAddress[3];

                /* We must remove the entry from the default router list. */
                _nxd_ipv6_default_router_delete(ip_ptr, &router_address);

                /* Set the router flag to NULL. */
                nd_entry -> nx_nd_cache_is_router = NX_NULL;
            }
        }
    }

    /*
     * An ND entry exists.  If the cache entry is in incomplete state,
     * add the NA contains LLA option, if the NA is unsolicitated,
     * add the LLA to the cache in STALE state.
     */
    if ((nd_entry -> nx_nd_cache_nd_status == ND_CACHE_STATE_INCOMPLETE) ||
        (nd_entry -> nx_nd_cache_nd_status == ND_CACHE_STATE_CREATED))
    {

        /* Find the target link layer options. */
        if (option_ptr)
        {

            /* If the solicitation flag is set, set state to REACHABLE; if not,
               set state to STALE.*/
            /*lint -e{929} -e{740} -e{826} suppress cast of pointer to pointer, since it is necessary  */
            _nx_nd_cache_add(ip_ptr, nd_ptr -> nx_icmpv6_nd_targetAddress,
                             packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_attached,
                             (CHAR *)&option_ptr -> nx_icmpv6_option_data, 0,
                             nd_ptr -> nx_icmpv6_nd_flag & 0x40000000 ? ND_CACHE_STATE_REACHABLE : ND_CACHE_STATE_STALE,
                             packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr, &nd_entry);

            /* Any queued packets?. */
            if (nd_entry -> nx_nd_cache_packet_waiting_head)
            {

                /* Send the queued packets out. */
                _nx_icmpv6_send_queued_packets(ip_ptr, nd_entry);
            }
        }

        /* All done. Release the packet and return.  */
        _nx_packet_release(packet_ptr);
        return;
    }

    /* When we get there, we have an ND entry that has valid (REACHABLE) LLA. */

    if ((nd_ptr -> nx_icmpv6_nd_flag & 0x20000000) == 0 && option_ptr && (!lla_same))
    {
        /*
         * Override bit is clear.
         * If the link layer address is different from the one in our cache entry,
         * and the entry is REACHABLE, update the entry to STALE.
         */
        if (nd_entry -> nx_nd_cache_nd_status == ND_CACHE_STATE_REACHABLE)
        {

            nd_entry -> nx_nd_cache_nd_status = ND_CACHE_STATE_STALE;
            nd_entry -> nx_nd_cache_timer_tick = 0;
        }
    }
    else
    {

        /* Processing according to RFC2461 7.2.5. */
        /* If LLA is supplied and is different from cache value, update the cache. */
        if (option_ptr && !lla_same)
        {
        USHORT *new_lla, *lla;

            /*lint -e{929} -e{927} suppress cast of pointer to pointer, since it is necessary  */
            new_lla = (USHORT *)&option_ptr -> nx_icmpv6_option_data;

            /*lint -e{927} suppress cast of pointer to pointer, since it is necessary  */
            lla = (USHORT *)nd_entry -> nx_nd_cache_mac_addr;
            lla[0] = new_lla[0];
            lla[1] = new_lla[1]; /* lgtm[cpp/overflow-buffer] */
            lla[2] = new_lla[2]; /* lgtm[cpp/overflow-buffer] */
        }
        if (nd_ptr -> nx_icmpv6_nd_flag & 0x40000000) /* S bit is set, force cache entry to REACHABLE */
        {

            nd_entry -> nx_nd_cache_nd_status = ND_CACHE_STATE_REACHABLE;
            nd_entry -> nx_nd_cache_timer_tick = ip_ptr -> nx_ipv6_reachable_timer;
        }
        else if (option_ptr && (!lla_same)) /* S bit is clear and either TLLA is not supplied or it is different. */
        {
            nd_entry -> nx_nd_cache_nd_status = ND_CACHE_STATE_STALE;
            nd_entry -> nx_nd_cache_timer_tick = 0;
        }
    }

    _nx_packet_release(packet_ptr);
    return;
}

#endif /* FEATURE_NX_IPV6 */

