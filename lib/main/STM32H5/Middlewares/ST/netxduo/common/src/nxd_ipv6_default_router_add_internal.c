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
/**   Internet Protocol version 6 Default Router Table (IPv6 router)      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/
#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ipv6.h"
#include "nx_nd_cache.h"


#ifdef FEATURE_NX_IPV6
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_ipv6_default_router_add_interanl                PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets IPv6 routing table entry.                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    router_addr                           Destination Network addr      */
/*    router_lifetime                       Life time information         */
/*    if_ptr                                Pointer to the interface      */
/*    router_type                           dynamic or static             */
/*    _ret                                  Return value, indicates where */
/*                                             the new entry is located.  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    COPY_IPV6_ADDRESS                     Copy IP addresses             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_ipv6_default_router_add          Add default router to IP      */
/*                                              default router list       */
/*    _nx_icmpv6_process_redirect           Process ICMPv6 redirect       */
/*                                              message                   */
/*    _nx_icmpv6_process_ra                 Process incoming Router       */
/*                                            Advertisement packets       */
/*                                                                        */
/*  NOTE                                                                  */
/*                                                                        */
/*    Make sure nx_ip_prortection mutex is obtained before entering this  */
/*    function.                                                           */
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
UINT  _nxd_ipv6_default_router_add_internal(NX_IP *ip_ptr,
                                            ULONG *router_addr,
                                            ULONG router_lifetime,
                                            NX_INTERFACE *if_ptr,
                                            INT router_type,
                                            NX_IPV6_DEFAULT_ROUTER_ENTRY **_ret)
{

UINT                          i;
UINT                          first_available = (UINT)0xFFFFFFFF;
ULONG                         address_type;
NX_IPV6_DEFAULT_ROUTER_ENTRY *ret = NX_NULL;
NXD_IPV6_ADDRESS             *ipv6_address;


    /* If a router pointer is provided, initialize it to NULL. */
    if (_ret)
    {
        *_ret = NX_NULL;
    }

    /* Verify gateway address is reachable. */
    address_type = IPv6_Address_Type(router_addr);
    if (address_type & IPV6_ADDRESS_UNICAST)
    {

        /* It is a unicast address. */
        if (address_type & IPV6_ADDRESS_GLOBAL)
        {

            /* It is a global address. */
            /* Point to the first address unit in the interface. */
            ipv6_address = if_ptr -> nxd_interface_ipv6_address_list_head;
            while (ipv6_address)
            {

                /* Check whether destination is on link. */
                if (_nxd_ipv6_find_max_prefix_length(router_addr, ipv6_address -> nxd_ipv6_address,
                                                     ipv6_address -> nxd_ipv6_address_prefix_length) >= ipv6_address -> nxd_ipv6_address_prefix_length)
                {

                    /* Router address is on link. */
                    break;
                }

                /* Point to the next address unit. */
                ipv6_address = ipv6_address -> nxd_ipv6_address_next;
            }

            if (ipv6_address == NX_NULL)
            {

                /* Gateway address is unreachable. */
                return(NX_IP_ADDRESS_ERROR);
            }
        }
    }
    else
    {

        /* Gateway address is unreachable. */
        return(NX_IP_ADDRESS_ERROR);
    }

    /* Search through the list for an already existing entry. */
    for (i = 0; i < NX_IPV6_DEFAULT_ROUTER_TABLE_SIZE; i++)
    {

        /* Does this slot contain a valid router? */
        if (ip_ptr -> nx_ipv6_default_router_table[i].nx_ipv6_default_router_entry_flag)
        {

            /* Check for matching router address. */
            if (CHECK_IPV6_ADDRESSES_SAME(router_addr, ip_ptr -> nx_ipv6_default_router_table[i].nx_ipv6_default_router_entry_router_address) &&
                if_ptr == ip_ptr -> nx_ipv6_default_router_table[i].nx_ipv6_default_router_entry_interface_ptr)
            {

                /* Its a match! */
                ret = &ip_ptr -> nx_ipv6_default_router_table[i];

                /* Update the router lifetime with the specified input. */
                ret -> nx_ipv6_default_router_entry_life_time = (USHORT)router_lifetime;

                /* Set a pointer to the router location in the table. */
                if (_ret)
                {
                    *_ret = ret;
                }

                return(NX_SUCCESS);
            }
        }
        else
        {

            /* Flag this as a slot we can use to add the new router. */
            if (first_available == (UINT)0xFFFFFFFF)
            {
                first_available = i;
            }
        }
    }

    /* Did we find an empty slot in the table? */
    if (first_available != (UINT)0xFFFFFFFF)
    {

        /* Set up local pointer. */
        ret = &ip_ptr -> nx_ipv6_default_router_table[first_available];

        /* Copy the router's address into the router table.  */
        COPY_IPV6_ADDRESS(router_addr, ret -> nx_ipv6_default_router_entry_router_address);

        /* Add the specified input to the router record. */
        ret -> nx_ipv6_default_router_entry_flag = (UCHAR)(router_type | NX_IPV6_ROUTE_TYPE_VALID);
        ret -> nx_ipv6_default_router_entry_life_time = (USHORT)router_lifetime;

        /* Set the interface index.  */
        ret -> nx_ipv6_default_router_entry_interface_ptr = if_ptr;

        /* Has no entry in the cache table. Neighbor Discovery process handles this
           automaticaly. */
        ret -> nx_ipv6_default_router_entry_neighbor_cache_ptr = NX_NULL;

        /* Update the count of routers currently in the table. */
        ip_ptr -> nx_ipv6_default_router_table_size++;
    }
    else
    {

        /* No empty slot. */
        return(NX_NO_MORE_ENTRIES);
    }

    /* If a router pointer was supplied, set it to the location of the router
       we just added to the table. */
    if (_ret)
    {
        *_ret = ret;
    }

    /* Successful completion, we're done! */
    return(NX_SUCCESS);
}


#endif /* FEATURE_NX_IPV6 */

