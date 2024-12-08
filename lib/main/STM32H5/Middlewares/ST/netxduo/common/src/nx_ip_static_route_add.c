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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_static_route_add                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds a static routing entry to the routing table.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    network_address                       Network address, in host byte */
/*                                            order.                      */
/*    net_mask                              Network Mask, in host byte    */
/*                                            order.                      */
/*    next_hop                              Next Hop address, in host     */
/*                                            byte order.                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain a protection mutex     */
/*    tx_mutex_put                          Release protection mutex      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
/*                                                                        */
/*  NOTE:                                                                 */
/*                                                                        */
/*    next hop address must be on the local network.                      */
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
UINT  _nx_ip_static_route_add(NX_IP *ip_ptr, ULONG network_address,
                              ULONG net_mask, ULONG next_hop)
{
#if !defined(NX_DISABLE_IPV4) && defined(NX_ENABLE_IP_STATIC_ROUTING)
INT           i;
NX_INTERFACE *nx_ip_interface = NX_NULL;

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_IP_STATIC_ROUTE_ADD, ip_ptr, network_address, net_mask, next_hop, NX_TRACE_IP_EVENTS, 0, 0);

    /* Obtain the IP mutex so we can manipulate the internal routing table. */
    /* This routine does not need to be protected by mask off interrupt
       because it cannot be invoked from ISR. */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Make sure next hop is on one of the interfaces. */
    for (i = 0; i < NX_MAX_IP_INTERFACES; i++)
    {

        if (ip_ptr -> nx_ip_interface[i].nx_interface_valid &&
            ((next_hop & (ip_ptr -> nx_ip_interface[i].nx_interface_ip_network_mask)) == ip_ptr -> nx_ip_interface[i].nx_interface_ip_network))
        {

            nx_ip_interface = &(ip_ptr -> nx_ip_interface[i]);

            /* Break out of the for loop */
            break;
        }
    }

    /* If no matching interface, return the error status. */
    if (nx_ip_interface == NX_NULL)
    {

        /* Unlock the mutex, and return the error status. */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        return(NX_IP_ADDRESS_ERROR);
    }

    /* Obtain the network address, based on net_mask passed in. */
    network_address = network_address & net_mask;

    /* Search through the routing table, check whether the same entry exists. */
    for (i = 0; i < (INT)ip_ptr -> nx_ip_routing_table_entry_count; i++)
    {

        if (ip_ptr -> nx_ip_routing_table[i].nx_ip_routing_dest_ip == network_address &&
            ip_ptr -> nx_ip_routing_table[i].nx_ip_routing_net_mask == net_mask)
        {

            /* Found the same entry: only need to update the next hop field */
            ip_ptr -> nx_ip_routing_table[i].nx_ip_routing_next_hop_address = next_hop;

            /* All done.  Unlock the mutex, and return */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));
            return(NX_SUCCESS);
        }

        /* The net mask that has more top bits set has a greater numerical value than the
           one with less top bits set.  During a search we want to match smaller nets
           (more top bits set, or larger numerical value) before matching larger nets. */
        if (ip_ptr -> nx_ip_routing_table[i].nx_ip_routing_net_mask <= net_mask)
        {

        INT j;

            /* Check whether the table is full. */
            if (ip_ptr -> nx_ip_routing_table_entry_count == NX_IP_ROUTING_TABLE_SIZE)
            {

                tx_mutex_put(&(ip_ptr -> nx_ip_protection));
                return(NX_OVERFLOW);
            }

            /* The entry pointed to by "i" is a larger network (thus smaller net mask
               value.  The new entry needs to be inserted before "i".

               To do so, we need to make room for the new entry, by shifting entries i and
               after one slot. */
            for (j = (INT)ip_ptr -> nx_ip_routing_table_entry_count - 1; j >= i; j--)
            {

                ip_ptr -> nx_ip_routing_table[j + 1].nx_ip_routing_dest_ip =
                    ip_ptr -> nx_ip_routing_table[j].nx_ip_routing_dest_ip;
                ip_ptr -> nx_ip_routing_table[j + 1].nx_ip_routing_net_mask =
                    ip_ptr -> nx_ip_routing_table[j].nx_ip_routing_net_mask;
                ip_ptr -> nx_ip_routing_table[j + 1].nx_ip_routing_next_hop_address =
                    ip_ptr -> nx_ip_routing_table[j].nx_ip_routing_next_hop_address;
                ip_ptr -> nx_ip_routing_table[j + 1].nx_ip_routing_entry_ip_interface =
                    ip_ptr -> nx_ip_routing_table[j].nx_ip_routing_entry_ip_interface;
            }

            break;
        }
    }

    /* Check whether the table is full. */
    if (i == NX_IP_ROUTING_TABLE_SIZE)
    {
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));
        return(NX_OVERFLOW);
    }

    ip_ptr -> nx_ip_routing_table[i].nx_ip_routing_dest_ip = network_address;
    ip_ptr -> nx_ip_routing_table[i].nx_ip_routing_net_mask = net_mask;
    ip_ptr -> nx_ip_routing_table[i].nx_ip_routing_next_hop_address = next_hop;
    ip_ptr -> nx_ip_routing_table[i].nx_ip_routing_entry_ip_interface = nx_ip_interface;

    ip_ptr -> nx_ip_routing_table_entry_count++;

    /* Unlock the mutex. */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return success to the caller.  */
    return(NX_SUCCESS);

#else /* !NX_DISABLE_IPV4 && NX_ENABLE_IP_STATIC_ROUTING  */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(network_address);
    NX_PARAMETER_NOT_USED(net_mask);
    NX_PARAMETER_NOT_USED(next_hop);

    return(NX_NOT_SUPPORTED);

#endif /* !NX_DISABLE_IPV4 && NX_ENABLE_IP_STATIC_ROUTING  */
}

