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

#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_route_find                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*     This function finds an outgoing interface and the next hop address */
/*     for a given destination address.  The caller may also set desired  */
/*     interface information in the ip_interface_ptr input.  For multicast*/
/*     or limited broadcast, this routine looks for the first enabled     */
/*     interface (link up) starting with the primary interface if         */
/*     a hint was not set by the caller.  For directed broadcast or       */
/*     unicast destinations, the hint is ignored and the proper outgoing  */
/*     interface is selected.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                IN              Pointer to IP instance        */
/*    destination_address   IN              Destination address           */
/*    ip_interface_ptr      OUT             Interface to use, must point  */
/*                                            to valid storage space.     */
/*    next_hop_address      OUT             IP address for the next hop,  */
/*                                            must point to valid storage */
/*                                            space.                      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Operation was successful      */
/*    NX_IP_ADDRESS_ERROR                   No suitable interface found   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_arp_dynamic_entry_set.c           ARP entry set                 */
/*    _nx_icmp_ping                         Transmit ICMP echo request    */
/*    _nx_ip_packet_send                    IP packet transmit            */
/*    _nx_tcp_client_socket_connect         TCP Client socket connection  */
/*    _nx_udp_socket_send                   UDP packet send               */
/*                                                                        */
/*  NOTE:                                                                 */
/*                                                                        */
/*    None                                                                */
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
ULONG  _nx_ip_route_find(NX_IP *ip_ptr, ULONG destination_address, NX_INTERFACE **ip_interface_ptr, ULONG *next_hop_address)
{

NX_INTERFACE *interface_ptr;
ULONG         i;

    /* Initialize the next hop address. */
    *next_hop_address = 0;

    /* Determine if the destination_address is multicast or directed broadcast. */
    if (((destination_address & NX_IP_CLASS_D_MASK) == NX_IP_CLASS_D_TYPE) ||
        (destination_address  == NX_IP_LIMITED_BROADCAST))
    {

        *next_hop_address = destination_address;

        /* If the caller did not set the ip_interface value, find a link enabled 
           interface, starting with the primary interface, for transmission.  */
        if (*ip_interface_ptr == NX_NULL)
        {

            /* Find an interface whose link is up. */
            for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
            {

                if (ip_ptr -> nx_ip_interface[i].nx_interface_link_up)
                {
                    *ip_interface_ptr = &(ip_ptr -> nx_ip_interface[i]);
                    return(NX_SUCCESS);
                }
            }
        }
        /* If the specified interface is up, return success. */
        else if ((*ip_interface_ptr) -> nx_interface_link_up)
        {
            return(NX_SUCCESS);
        }

        /* No available interface. */
        return(NX_IP_ADDRESS_ERROR);
    }

    /* Search through the interfaces associated with the IP instance,
       check if the the destination address is one of the local interface addresses. */
    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
    {

        /* Use a local variable for convenience. */
        interface_ptr = &(ip_ptr -> nx_ip_interface[i]);

        /* Check for a valid interface that maps onto the same network domain as the destination address. */
        if ((interface_ptr -> nx_interface_valid) &&
            (interface_ptr -> nx_interface_link_up) &&
            (interface_ptr -> nx_interface_ip_address == destination_address) &&
            ((*ip_interface_ptr == NX_NULL) ||
             (*ip_interface_ptr == interface_ptr)))
        {

            /* Yes, use the entry information for interface and next hop. */
            *ip_interface_ptr = interface_ptr;
            *next_hop_address = destination_address;
            return(NX_SUCCESS);
        }
    }

#ifdef NX_ENABLE_IP_STATIC_ROUTING

    /* Search through the routing table for a suitable interface. */
    for (i = 0; i < ip_ptr -> nx_ip_routing_table_entry_count; i++)
    {

        /* Get the interface. */
        interface_ptr = ip_ptr -> nx_ip_routing_table[i].nx_ip_routing_entry_ip_interface;

        /* Skip interface that is not up. */
        if (interface_ptr -> nx_interface_link_up == NX_FALSE)
        {
            continue;
        }

        /* Does this table entry match the destination table network domain?*/
        if (ip_ptr -> nx_ip_routing_table[i].nx_ip_routing_dest_ip ==
            (destination_address & ip_ptr -> nx_ip_routing_table[i].nx_ip_routing_net_mask))
        {

            /* Yes, is next hop address still reachable? */
            if (interface_ptr -> nx_interface_ip_network !=
                (ip_ptr -> nx_ip_routing_table[i].nx_ip_routing_next_hop_address &
                 interface_ptr -> nx_interface_ip_network_mask))
            {
                continue;
            }

            /* Use the entry information for interface and next hop. */
            if (*ip_interface_ptr == NX_NULL)
            {
                *ip_interface_ptr = interface_ptr;
            }
            else if (*ip_interface_ptr != interface_ptr)
            {
                continue;
            }

            *next_hop_address = ip_ptr -> nx_ip_routing_table[i].nx_ip_routing_next_hop_address;

            return(NX_SUCCESS);
        }
    }

#endif /* NX_ENABLE_IP_STATIC_ROUTING */

    /* Search through the interfaces associated with the IP instance,
       check if the entry exists. */
    for (i = 0; i < NX_MAX_IP_INTERFACES; i++)
    {

        /* Use a local variable for convenience. */
        interface_ptr = &(ip_ptr -> nx_ip_interface[i]);

        /* Check for a valid interface that maps onto the same network domain as the destination address. */
        if ((interface_ptr -> nx_interface_valid) &&
            (interface_ptr -> nx_interface_link_up) &&
            ((interface_ptr -> nx_interface_ip_network_mask & destination_address) == interface_ptr -> nx_interface_ip_network))
        {

            /* Yes, use the entry information for interface and next hop. */
            if (*ip_interface_ptr == NX_NULL)
            {
                *ip_interface_ptr = interface_ptr;
            }
            /* Match loopback interface.  */
            /* Suppress constant value, since "NX_MAX_IP_INTERFACES" can be redefined. */
#if (NX_MAX_IP_INTERFACES == (NX_MAX_PHYSICAL_INTERFACES + 1))
            else if (i == NX_MAX_PHYSICAL_INTERFACES)
            {
                *ip_interface_ptr = interface_ptr;
            }
#endif
            else if (*ip_interface_ptr != interface_ptr)
            {
                continue;
            }

            *next_hop_address = destination_address;

            return(NX_SUCCESS);
        }
    }

    /* Search the interfaces for IPv4 Link-Local Address according to RFC3927, section2.6.  */
    /* Determine if destination addrss is link-local address(169.254/16 Hexadecimal:0xA9FE0000).  */
    if ((destination_address & 0xFFFF0000) == 0xA9FE0000)
    {

        /* Yes, check if the interface is set.  */
        if (*ip_interface_ptr)
        {

            /* Determine if the interface is valid.  */
            if (((*ip_interface_ptr) -> nx_interface_valid) &&
                ((*ip_interface_ptr) -> nx_interface_link_up))
            {

                /* Set the next hop address.  */
                *next_hop_address = destination_address;

                return(NX_SUCCESS);
            }
        }
        else
        {

            /* Search through the interfaces associated with the IP instance, set the inteface as first valid interface.  */
            for (i = 0; i < NX_MAX_IP_INTERFACES; i++)
            {

                /* Check for a valid interface that the address is link-local address.  */
                if ((ip_ptr -> nx_ip_interface[i].nx_interface_valid) &&
                    (ip_ptr -> nx_ip_interface[i].nx_interface_link_up))
                {

                    /* Yes, use the entry information for interface and next hop. */
                    *ip_interface_ptr = &(ip_ptr -> nx_ip_interface[i]);
                    *next_hop_address = destination_address;

                    return(NX_SUCCESS);
                }
            }
        }
    }

    /* Does the IP instance have a gateway? */
    if ((ip_ptr -> nx_ip_gateway_address) &&
        (ip_ptr -> nx_ip_gateway_interface) &&
        (ip_ptr -> nx_ip_gateway_interface -> nx_interface_link_up))
    {

        /* Get the interface. */
        interface_ptr = ip_ptr -> nx_ip_gateway_interface;

        /* Yes, is gateway address still reachable? */
        if (interface_ptr -> nx_interface_ip_network !=
            (ip_ptr -> nx_ip_gateway_address &
             interface_ptr -> nx_interface_ip_network_mask))
        {
            return(NX_IP_ADDRESS_ERROR);
        }

        /* Use the gateway as default. */
        if (*ip_interface_ptr == NX_NULL)
        {
            *ip_interface_ptr = interface_ptr;
        }
        else if (*ip_interface_ptr != interface_ptr)
        {
            return(NX_IP_ADDRESS_ERROR);
        }

        *next_hop_address = ip_ptr -> nx_ip_gateway_address;

        return(NX_SUCCESS);
    }

    /* Determine if source addrss is link-local address(169.254/16 Hexadecimal:0xA9FE0000).  */
    if (*ip_interface_ptr)
    {

        /* Determine if the interface is valid and the address of interface is link-local address.  */
        if (((*ip_interface_ptr) -> nx_interface_valid) &&
            ((*ip_interface_ptr) -> nx_interface_link_up) &&
            (((*ip_interface_ptr) -> nx_interface_ip_address & 0xFFFF0000) == 0xA9FE0000))
        {

            /* Set the next hop address.  */
            *next_hop_address = destination_address;

            return(NX_SUCCESS);
        }
    }
    else
    {

        /* Search through the interfaces associated with the IP instance,
           check if interface is valid and the address of interface is link-local address. */
        for (i = 0; i < NX_MAX_IP_INTERFACES; i++)
        {

            /* Use a local variable for convenience. */
            interface_ptr = &(ip_ptr -> nx_ip_interface[i]);

            /* Check for a valid interface that the address is link-local address.  */
            if ((interface_ptr -> nx_interface_valid) &&
                (interface_ptr -> nx_interface_link_up) &&
                ((interface_ptr -> nx_interface_ip_address & 0xFFFF0000) == 0xA9FE0000))
            {

                /* Yes, use the entry information for interface and next hop. */
                *ip_interface_ptr = interface_ptr;
                *next_hop_address = destination_address;

                return(NX_SUCCESS);
            }
        }
    }

    /* Cannot find a proper way to transmit this packet.
       Return the error status. */
    return(NX_IP_ADDRESS_ERROR);
}
#endif /* NX_DISABLE_IPV4 */

