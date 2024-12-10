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
/*    _nxd_ipv6_search_onlink                              PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks whether a given IP address is on link.         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance                   */
/*    dest_addr                             The destination address to    */
/*                                             be checked.                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                0: Address is not onlink      */
/*                                          1: Address is onlink          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    nx_ipv6_packet_send                  Sends IPv6 packets from upper  */
/*                                           layer to remote host         */
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
INT _nxd_ipv6_search_onlink(NX_IP *ip_ptr, ULONG *dest_addr)
{

UINT                  addr_index;
NX_IPV6_PREFIX_ENTRY *prefix_entry;
NXD_IPV6_ADDRESS     *ipv6_address;


    /* First special case is the link local address. All these
       addresses are onlink.  */
    if (IPv6_Address_Type(dest_addr) & IPV6_ADDRESS_LINKLOCAL)
    {
        return(1);
    }

    /* Set a local pointer for convenience. */
    prefix_entry = ip_ptr -> nx_ipv6_prefix_list_ptr;

    /* Loop through the prefix table. Prefixes are the IPv6 equivalent of
       network domains in IPv4.  */
    while (prefix_entry)
    {

        /* Check whether or not the destination address is matched. */
        if (CHECK_IP_ADDRESSES_BY_PREFIX(dest_addr,
                                         prefix_entry -> nx_ipv6_prefix_entry_network_address,
                                         prefix_entry -> nx_ipv6_prefix_entry_prefix_length))
        {
            return(1);
        }

        /* No match. Try the next prefix. */
        prefix_entry = prefix_entry -> nx_ipv6_prefix_entry_next;
    }

    /* If no matches found in the prefix list, search the manually configured IPv6 interface addresses. */
    for (addr_index = 0; addr_index < NX_MAX_IPV6_ADDRESSES; addr_index++)
    {

        ipv6_address = &ip_ptr -> nx_ipv6_address[addr_index];
        /* Skip invalid entries. */
        if (!(ipv6_address -> nxd_ipv6_address_valid))
        {
            continue;
        }

        /* Skip non-manually configured entires. */
        if (ipv6_address -> nxd_ipv6_address_ConfigurationMethod != NX_IPV6_ADDRESS_MANUAL_CONFIG)
        {
            continue;
        }

        /* Check whether or not the destination address is matched. */
        if (CHECK_IP_ADDRESSES_BY_PREFIX(dest_addr,
                                         ipv6_address -> nxd_ipv6_address,
                                         ipv6_address -> nxd_ipv6_address_prefix_length))
        {
            return(1);
        }
    }


    /* No matches found. Not an onlink address. */
    return(0);
}

#endif /* FEATURE_NX_IPV6 */

