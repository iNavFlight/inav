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

/* Include necessary system files. */

#include "nx_api.h"
#include "nx_tcp.h"
#include "nx_arp.h"
#include "nx_igmp.h"
#include "nx_ip.h"
#include "nx_packet.h"
#ifdef FEATURE_NX_IPV6
#include "nx_icmpv6.h"
#endif /* FEATURE_NX_IPV6 */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_interface_detach                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function detaches a physical network interface from the IP     */
/*    instance.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    index                                 IP interface index            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*    _nx_tcp_socket_connection_reset       Reset TCP connection          */
/*    _nx_arp_interface_entries_delete      Remove specified ARP entries  */
/*    _nx_invalidate_destination_entry      Invalidate the entry in the   */
/*                                           destination                  */
/*    _nx_nd_cache_interface_entries_delete Delete ND cache entries       */
/*                                            associated with specific    */
/*                                            interface                   */
/*    link_driver_entry                     Link driver                   */
/*    memset                                Zero out the interface        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
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
UINT _nx_ip_interface_detach(NX_IP *ip_ptr, UINT index)
{

NX_INTERFACE     *interface_ptr;
NX_IP_DRIVER      driver_request;
NX_TCP_SOCKET    *socket_ptr;
NXD_IPV6_ADDRESS *next_ipv6_address;
NXD_IPV6_ADDRESS *ipv6_address;
UINT              i;
#ifdef NX_ENABLE_IP_STATIC_ROUTING
UINT              j;
#endif

#ifdef FEATURE_NX_IPV6
NX_IPV6_DEFAULT_ROUTER_ENTRY *rt_entry;
#endif


    interface_ptr = &(ip_ptr -> nx_ip_interface[index]);

    /* Check interface status. */
    if (interface_ptr -> nx_interface_valid == NX_FALSE)
    {

        /* Invalid interface. */
        return(NX_INVALID_INTERFACE);
    }

    /* Obtain the IP internal mutex before calling the driver. */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Loop through the list of created TCP sockets for this IP instance. */
    socket_ptr = ip_ptr -> nx_ip_tcp_created_sockets_ptr;
    if (socket_ptr != NX_NULL)
    {
        do
        {
            /* Reset TCP connection which is established or in progress on this interface. */
            if (socket_ptr -> nx_tcp_socket_connect_interface == interface_ptr)
            {
                _nx_tcp_socket_connection_reset(socket_ptr);
            }

            /* Move to the next. */
            socket_ptr = socket_ptr -> nx_tcp_socket_created_next;
        } while (socket_ptr != ip_ptr -> nx_ip_tcp_created_sockets_ptr);
    }

    /* Release the IP internal mutex. */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

#ifndef NX_DISABLE_IPV4
    /* Remove ARP entries associated with  this interface from ARP table. */
    _nx_arp_interface_entries_delete(ip_ptr, index);
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    /* Delete ND cache entries associated with this interface. */
    _nx_nd_cache_interface_entries_delete(ip_ptr, index);
#endif /* FEATURE_NX_IPV6 */

    /* Obtain the IP internal mutex before calling the driver. */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

#ifndef NX_DISABLE_IPV4
#ifdef NX_ENABLE_IP_STATIC_ROUTING
    /* Remove router iterms associated with the interface that will be detached from route table. */
    for (i = 0; i < ip_ptr -> nx_ip_routing_table_entry_count; i++)
    {

        /* Is this router iterm related to the interface to be detached. */
        if (ip_ptr -> nx_ip_routing_table[i].nx_ip_routing_entry_ip_interface == interface_ptr)
        {

            /* Yes, we need to remove this iterm. */
            /* If the entry is not the last one, we need to shift the table to fill the hole. */
            for (j = i; j < ip_ptr -> nx_ip_routing_table_entry_count; j++)
            {
                ip_ptr -> nx_ip_routing_table[j].nx_ip_routing_dest_ip            =
                    ip_ptr -> nx_ip_routing_table[j + 1].nx_ip_routing_dest_ip;
                ip_ptr -> nx_ip_routing_table[j].nx_ip_routing_net_mask           =
                    ip_ptr -> nx_ip_routing_table[j + 1].nx_ip_routing_net_mask;
                ip_ptr -> nx_ip_routing_table[j].nx_ip_routing_next_hop_address   =
                    ip_ptr -> nx_ip_routing_table[j + 1].nx_ip_routing_next_hop_address;
                ip_ptr -> nx_ip_routing_table[j].nx_ip_routing_entry_ip_interface =
                    ip_ptr -> nx_ip_routing_table[j + 1].nx_ip_routing_entry_ip_interface;
            }

            ip_ptr -> nx_ip_routing_table[j - 1].nx_ip_routing_dest_ip            = 0;
            ip_ptr -> nx_ip_routing_table[j - 1].nx_ip_routing_net_mask           = 0;
            ip_ptr -> nx_ip_routing_table[j - 1].nx_ip_routing_next_hop_address   = 0;
            ip_ptr -> nx_ip_routing_table[j - 1].nx_ip_routing_entry_ip_interface = NX_NULL;

            ip_ptr -> nx_ip_routing_table_entry_count--;
        }
    }
#endif /* NX_ENABLE_IP_STATIC_ROUTING  */
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    /* Remove IPv6 routers associated with this interface. */
    for (i = 0; i < NX_IPV6_DEFAULT_ROUTER_TABLE_SIZE; i++)
    {
        /* Set local pointer for convenience. */
        rt_entry = &ip_ptr -> nx_ipv6_default_router_table[i];

        /* Does this slot contain a router. */
        if (rt_entry -> nx_ipv6_default_router_entry_flag & NX_IPV6_ROUTE_TYPE_VALID)
        {
            /* Is this router iterm related to the interface to be detached. */
            if (rt_entry -> nx_ipv6_default_router_entry_interface_ptr == interface_ptr)
            {

                /* Yes, we need to remove this router iterm. */
                /* Clean any entries in the destination table for this router.  */
                _nx_invalidate_destination_entry(ip_ptr, rt_entry -> nx_ipv6_default_router_entry_router_address);

                /* Mark the entry as empty. */
                rt_entry -> nx_ipv6_default_router_entry_flag = 0;

                /* Clear the interface pointer .*/
                rt_entry -> nx_ipv6_default_router_entry_interface_ptr = NX_NULL;

                /* Decrease the count of available routers. */
                ip_ptr -> nx_ipv6_default_router_table_size--;
            }
        }
    }
#endif

#ifndef NX_DISABLE_IPV4
    /* Clear gateway address related to the interface to be detached. */
    if (ip_ptr -> nx_ip_gateway_interface == interface_ptr)
    {
        ip_ptr -> nx_ip_gateway_interface = NX_NULL;
        ip_ptr -> nx_ip_gateway_address   = 0;
    }


    /* Leave multicast groups related to the interface to be detached. */
    for (i = 0; i < NX_MAX_MULTICAST_GROUPS; i++)
    {

        /* Skip entries not related to the interface. */
        if (ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_join_interface_list != interface_ptr)
        {
            continue;
        }

        /* Set join count to 1 so force send multicast leave command. */
        ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_join_count = 1;

        /* Leave the multicast group. */
        _nx_igmp_multicast_interface_leave_internal(ip_ptr,
                                                    ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_join_list,
                                                    index);
    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef NX_ENABLE_IPV6_MULTICAST
    /* Leave IPv6 multicast groups related to the interface to be detached. */
    for (i = 0; i < NX_MAX_MULTICAST_GROUPS; i++)
    {
        if (ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_interface_list == interface_ptr)
        {
            ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_count = 0;

            /* Clear the group join value. */
            memset(ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_list, 0, 4 * sizeof(ULONG));

            /* Decrement the MLD groups joined count. */
            ip_ptr -> nx_ipv6_multicast_groups_joined--;

            /* Un-register the new multicast group with the underlying driver.  */
            driver_request.nx_ip_driver_ptr =                    ip_ptr;
            driver_request.nx_ip_driver_command =                NX_LINK_MULTICAST_LEAVE;
            driver_request.nx_ip_driver_physical_address_msw =   0x00003333;
            driver_request.nx_ip_driver_physical_address_lsw =
                ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_list[3];
            driver_request.nx_ip_driver_interface =              interface_ptr;

            (interface_ptr -> nx_interface_link_driver_entry)(&driver_request);

            /* Now clear the interface entry. */
            ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_interface_list = NX_NULL;
        }
    }
#endif

    /* Detach the interface. */
    /* First detach the interface from the device. */
    driver_request.nx_ip_driver_ptr         = ip_ptr;
    driver_request.nx_ip_driver_command     = NX_LINK_INTERFACE_DETACH;
    driver_request.nx_ip_driver_interface   = interface_ptr;

    (interface_ptr -> nx_interface_link_driver_entry)(&driver_request);

    /* Release the IP internal mutex. */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Clear out IPv6 address. */
    next_ipv6_address = interface_ptr -> nxd_interface_ipv6_address_list_head;

    while (next_ipv6_address)
    {
        ipv6_address      = next_ipv6_address;
        next_ipv6_address = next_ipv6_address -> nxd_ipv6_address_next;
        memset(ipv6_address, 0, sizeof(NXD_IPV6_ADDRESS));
    }

    /* Zero out the interface. */
    memset(interface_ptr, 0, sizeof(NX_INTERFACE));

    /* reserve the index. */
    interface_ptr -> nx_interface_index = (UCHAR)index;

    return(NX_SUCCESS);
}

