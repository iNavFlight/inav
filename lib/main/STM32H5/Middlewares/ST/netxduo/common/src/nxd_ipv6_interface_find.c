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
#include "nx_ipv6.h"

#ifdef FEATURE_NX_IPV6
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_ipv6_interface_find                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds a suitable outgoing interface based on the      */
/*    destination IP address.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    dest_address                          Destination IP address        */
/*    ipv6_interface_index                  Index interface to use as the */
/*                                            outgoing  interface.        */
/*    ipv6_address_index                    Address index to use as the   */
/*                                            source address              */
/*    if_ptr                                Outgoing interface if not null*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Internal function.                                                  */
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
UINT _nxd_ipv6_interface_find(NX_IP *ip_ptr, ULONG *dest_address,
                              NXD_IPV6_ADDRESS **ipv6_addr, NX_INTERFACE *if_ptr)
{

UINT                          i;
NXD_IPV6_ADDRESS             *ipv6_address;
NX_IPV6_DEFAULT_ROUTER_ENTRY *rt_entry;
UINT                          start_index;
UINT                          end_index;
ULONG                         address_type = IPv6_Address_Type(dest_address);

    /* ipv6_addr must not be NULL. */
    NX_ASSERT(ipv6_addr != NX_NULL);

#ifdef NX_ENABLE_IPV6_MULTICAST
    /* Check for a multicast destination address.*/
    if (address_type & IPV6_ADDRESS_MULTICAST)
    {


        /* Search the address whether match our mld join list.  */
        for (i = 0; i < NX_MAX_MULTICAST_GROUPS; i++)
        {

            /* Skip empty entries. */
            if (ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_interface_list == NX_NULL)
            {
                continue;
            }

            /* Skip not matched interface. */
            if (if_ptr && (ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_interface_list != if_ptr))
            {
                continue;
            }

            /* Skip multicast entry whose related interface is down. */
            if (ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_interface_list -> nx_interface_link_up == NX_FALSE)
            {
                continue;
            }

            if (CHECK_IPV6_ADDRESSES_SAME(ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_list, dest_address))
            {
                *ipv6_addr = ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_interface_list -> nxd_interface_ipv6_address_list_head;

                /* Check whether there's address on the interface. */
                if (*ipv6_addr == NX_NULL)
                {
                    return(NX_NO_INTERFACE_ADDRESS);
                }

                /* The address must be valid in list queue of interface. */
                NX_ASSERT((*ipv6_addr) -> nxd_ipv6_address_valid);
                return(NX_SUCCESS);
            }
        }
    }
#endif /* NX_ENABLE_IPV6_MULTICAST  */

    /* Loop through addresses. */
    if (address_type & IPV6_ADDRESS_UNICAST)
    {

        /* Unicast address. Is the destination one of local address? */
        for (i = 0; i < NX_MAX_IPV6_ADDRESSES; i++)
        {

            /* Compare the address. */
            if ((ip_ptr -> nx_ipv6_address[i].nxd_ipv6_address_valid) &&
                (CHECK_IPV6_ADDRESSES_SAME(ip_ptr -> nx_ipv6_address[i].nxd_ipv6_address, dest_address)))
            {

                /* Found a proper address. */
                *ipv6_addr = &(ip_ptr -> nx_ipv6_address[i]);
                return(NX_SUCCESS);
            }
        }
    }

    if (if_ptr)
    {

        /* Search addresses from specified interface only. */
        start_index = if_ptr -> nx_interface_index;
        end_index = (UINT)(if_ptr -> nx_interface_index + 1);
    }
    else
    {

        /* Search addressed from all interfaces. */
        start_index = 0;
        end_index = NX_MAX_PHYSICAL_INTERFACES;
    }

    /* Loop through interfaces. */
    for (i = start_index; i < end_index; i++)
    {

        /* Skip interface which is down. */
        if (ip_ptr -> nx_ip_interface[i].nx_interface_link_up == NX_FALSE)
        {
            continue;
        }

        /* Point to the first address unit in the interface. */
        for (ipv6_address = ip_ptr -> nx_ip_interface[i].nxd_interface_ipv6_address_list_head;
             ipv6_address;
             ipv6_address = ipv6_address -> nxd_ipv6_address_next)
        {

            /* Skip address that is not valid. */
            if (ipv6_address -> nxd_ipv6_address_state != NX_IPV6_ADDR_STATE_VALID)
            {
                continue;
            }

            if (address_type & IPV6_ADDRESS_LINKLOCAL)
            {

                /* Destination address is link local. */
                if (IPv6_Address_Type(ipv6_address -> nxd_ipv6_address) & IPV6_ADDRESS_LINKLOCAL)
                {

                    /* Found link local address as source. */
                    break;
                }
            }
            else if (_nxd_ipv6_find_max_prefix_length(dest_address, ipv6_address -> nxd_ipv6_address,
                                                      ipv6_address -> nxd_ipv6_address_prefix_length) >=
                     ipv6_address -> nxd_ipv6_address_prefix_length)
            {

                /* Found a proper outgoing address. */
                break;
            }
            /* Check for a multicast destination address.*/
            else if (address_type & IPV6_ADDRESS_MULTICAST)
            {

                /* This indicates a global IP multicast. */
                /* So we need to make a best guess at the
                   address index of the host global address. */

                /* Determine(LLA vs Global IP address type from the higher order bytes. */
                if ((dest_address[0] & 0x000F0000) == 0x00020000)
                {

                    /* Check for a link local IP address. */
                    if (IPv6_Address_Type(ipv6_address -> nxd_ipv6_address) & IPV6_ADDRESS_LINKLOCAL)
                    {

                        /* This will do! */
                        break;
                    }
                }
                /* Check for a global IP address. */
                else if (IPv6_Address_Type(ipv6_address -> nxd_ipv6_address) & IPV6_ADDRESS_GLOBAL)
                {

                    /* This will do! */
                    break;
                }
            }
        }

        if (ipv6_address)
        {

            /* Found a proper address. */
            *ipv6_addr = ipv6_address;
            return(NX_SUCCESS);
        }
    }

#ifndef NX_DISABLE_LOOPBACK_INTERFACE
    /* Get ipv6 address in the loop back interface. */
    ipv6_address = ip_ptr -> nx_ip_interface[NX_LOOPBACK_INTERFACE].nxd_interface_ipv6_address_list_head;
    if (ipv6_address)
    {

        /* Check whether the destination is loop back address. */
        if (CHECK_IPV6_ADDRESSES_SAME(ipv6_address -> nxd_ipv6_address, dest_address))
        {
            *ipv6_addr = ipv6_address;
            return(NX_SUCCESS);
        }
    }
#endif /* NX_DISABLE_LOOPBACK_INTERFACE */

    /* Is the destination address gloabl? */
    if (address_type & IPV6_ADDRESS_GLOBAL)
    {

        /* Yes. Check default router. */
        for (i = 0; i < NX_IPV6_DEFAULT_ROUTER_TABLE_SIZE; i++)
        {

            rt_entry = &ip_ptr -> nx_ipv6_default_router_table[i];

            /* Skip invalid entries. */
            if (rt_entry -> nx_ipv6_default_router_entry_flag == 0)
            {
                continue;
            }

            /* Skip interface which is down. */
            if (rt_entry -> nx_ipv6_default_router_entry_interface_ptr -> nx_interface_link_up == NX_FALSE)
            {
                continue;
            }

            /* Skip not matched interface. */
            if (if_ptr && (rt_entry -> nx_ipv6_default_router_entry_interface_ptr != if_ptr))
            {
                continue;
            }

            /* Get first address from interface. */
            ipv6_address = rt_entry -> nx_ipv6_default_router_entry_interface_ptr -> nxd_interface_ipv6_address_list_head;

            while (ipv6_address)
            {

                /* Check for a valid address. */
                if (ipv6_address -> nxd_ipv6_address_state != NX_IPV6_ADDR_STATE_VALID)
                {
                    ipv6_address = ipv6_address -> nxd_ipv6_address_next;
                }
                /* Check for link-local address. */
                else if (IPv6_Address_Type(ipv6_address -> nxd_ipv6_address) & IPV6_ADDRESS_LINKLOCAL)
                {
                    ipv6_address = ipv6_address -> nxd_ipv6_address_next;
                }
                else
                {
                    *ipv6_addr = ipv6_address;

                    /* Found global address as link-local address. */
                    return(NX_SUCCESS);
                }
            }
        }
    }

    /* No available interface. */
    return(NX_NO_INTERFACE_ADDRESS);
}

#endif /* FEATURE_NX_IPV6 */

