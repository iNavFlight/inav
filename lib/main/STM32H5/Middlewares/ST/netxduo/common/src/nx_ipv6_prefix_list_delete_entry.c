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

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ipv6.h"
#include "nx_nd_cache.h"




#ifdef FEATURE_NX_IPV6
/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Component                                                        */
/**                                                                       */
/**   Internet Protocol version 6 Prefix Table (IPv6 prefix table)        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ipv6_prefix_list_delete_entry                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes an entry from the prefix list.                */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    entry                                 Entry to be deleted           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    CHECK_IPV6_ADDRESS_SAME               Compare two IPv6 addresses    */
/*    SET_SOLICITED_NODE_MULTICAST_ADDRESS  Get the solicited-node        */
/*                                            multicast address           */
/*    _nx_ipv6_multicast_leave              Leave an IPv6 mulcast group   */
/*    [ipv6_address_change_notify]          User callback function        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ipv6_prefix_list_delete           Delete a prefix entry.        */
/*    _nx_ipv6_prefix_router_timer_tick     Router list timeout           */
/*                                                                        */
/*  NOTE                                                                  */
/*                                                                        */
/*    This function cannot be called from ISR.                            */
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
VOID _nx_ipv6_prefix_list_delete_entry(NX_IP *ip_ptr, NX_IPV6_PREFIX_ENTRY *entry)
{

UINT              i;
ULONG             address_prefix[4];
NXD_IPV6_ADDRESS *interface_ipv6_address_prev, *interface_ipv6_address;
ULONG             multicast_address[4];
#ifdef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
UINT              ipv6_addr_index;
#endif /* NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY */

    /* Invalidate the interface IP address if we obtained the
       interface IP address based on the prefix information. */

    /* Search through each physical interface for a match. */
    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
    {

        /* Get a pointer to the first address in the interface list. */
        interface_ipv6_address = ip_ptr -> nx_ip_interface[i].nxd_interface_ipv6_address_list_head;
        interface_ipv6_address_prev = NX_NULL;

        /* Search the address list for a match. */
        while (interface_ipv6_address)
        {

            /* Is this interface address valid? */
            if (interface_ipv6_address -> nxd_ipv6_address_state != NX_IPV6_ADDR_STATE_UNKNOWN &&
                interface_ipv6_address -> nxd_ipv6_address_ConfigurationMethod == NX_IPV6_ADDRESS_BASED_ON_INTERFACE)
            {

                /* Yes.  Extract the prefix to match on. The prefix length is 64 bits. */
                address_prefix[0] = interface_ipv6_address -> nxd_ipv6_address[0];
                address_prefix[1] = interface_ipv6_address -> nxd_ipv6_address[1];
                address_prefix[2] = 0;
                address_prefix[3] = 0;

                /* Do we have a match?  */
                if (CHECK_IPV6_ADDRESSES_SAME(address_prefix, entry -> nx_ipv6_prefix_entry_network_address))
                {

                    /* Yes, invalidate this address.  */
                    interface_ipv6_address -> nxd_ipv6_address_valid = NX_FALSE;
                    interface_ipv6_address -> nxd_ipv6_address_state = NX_IPV6_ADDR_STATE_UNKNOWN;

                    interface_ipv6_address -> nxd_ipv6_address_ConfigurationMethod = NX_IPV6_ADDRESS_NOT_CONFIGURED;

#ifndef NX_DISABLE_IPV6_DAD
                    interface_ipv6_address -> nxd_ipv6_address_DupAddrDetectTransmit = 0;
#endif /* NX_DISABLE_IPV6_DAD */

                    /* Update the list. */
                    if (interface_ipv6_address_prev == NX_NULL)
                    {
                        ip_ptr -> nx_ip_interface[i].nxd_interface_ipv6_address_list_head = interface_ipv6_address -> nxd_ipv6_address_next;
                    }
                    else
                    {
                        interface_ipv6_address_prev -> nxd_ipv6_address_next = interface_ipv6_address -> nxd_ipv6_address_next;
                    }

                    /* Delete the associated multicast address. */
                    SET_SOLICITED_NODE_MULTICAST_ADDRESS(multicast_address, interface_ipv6_address -> nxd_ipv6_address);

                    _nx_ipv6_multicast_leave(ip_ptr, &multicast_address[0], interface_ipv6_address -> nxd_ipv6_address_attached);

#ifdef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
                    /* If the address change notify callback is set, invoke the callback function. */
                    if (ip_ptr -> nx_ipv6_address_change_notify)
                    {
                        ipv6_addr_index = (ULONG)interface_ipv6_address -> nxd_ipv6_address_index;
                        ip_ptr -> nx_ipv6_address_change_notify(ip_ptr, NX_IPV6_ADDRESS_LIFETIME_EXPIRED, i, ipv6_addr_index,
                                                                &interface_ipv6_address -> nxd_ipv6_address[0]);
                    }
#endif /* NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY */

                    /* Clear the address at last. */
                    SET_UNSPECIFIED_ADDRESS(interface_ipv6_address -> nxd_ipv6_address);

                    /* Address for this interface is found. Just break. */
                    break;
                }
            }

            /* Set the previous address. */
            interface_ipv6_address_prev = interface_ipv6_address;

            /* Get the next address. */
            interface_ipv6_address = interface_ipv6_address -> nxd_ipv6_address_next;
        } /* while (interface_ipv6_address) */
    } /* for(i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++) */

    /* Unlink the previous node, if it exists. */
    if (entry -> nx_ipv6_prefix_entry_prev == NX_NULL)
    {
        ip_ptr -> nx_ipv6_prefix_list_ptr = entry -> nx_ipv6_prefix_entry_next;
    }
    else
    {
        entry -> nx_ipv6_prefix_entry_prev -> nx_ipv6_prefix_entry_next = entry -> nx_ipv6_prefix_entry_next;
    }

    /* Unlink the next node if it exists. */
    if (entry -> nx_ipv6_prefix_entry_next)
    {
        entry -> nx_ipv6_prefix_entry_next -> nx_ipv6_prefix_entry_prev = entry -> nx_ipv6_prefix_entry_prev;
    }

    /* Clean up this entry. */
    entry -> nx_ipv6_prefix_entry_next = NX_NULL;
    entry -> nx_ipv6_prefix_entry_prev = NX_NULL;

    /* Put entry onto the free list.*/
    if (ip_ptr -> nx_ipv6_prefix_entry_free_list == NX_NULL)
    {
        /* Free list is empty.  Set entry to be the first on the list. */
        ip_ptr -> nx_ipv6_prefix_entry_free_list = entry;
    }
    else
    {

        /* Free list is not empty.  Insert the entry to the head of the list. */
        ip_ptr -> nx_ipv6_prefix_entry_free_list -> nx_ipv6_prefix_entry_prev = entry;
        entry -> nx_ipv6_prefix_entry_next = ip_ptr -> nx_ipv6_prefix_entry_free_list;
        ip_ptr -> nx_ipv6_prefix_entry_free_list = entry;
    }

    /* All done. Return. */
    return;
}


#endif /* FEATURE_NX_IPV6 */

