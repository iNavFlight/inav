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
/**   Neighbor Discovery Cache                                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_nd_cache.h"
#ifdef FEATURE_NX_IPV6
#include "nx_ipv6.h"
#endif /* FEATURE_NX_IPV6 */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_nd_cache_entry_ip_address_find                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds an IPv6 address in the neighbor discovery       */
/*    cache table based on user-speicified MAC address.                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    ip_address                            Pointer to the IP address     */
/*                                            to search for               */
/*    physical_msw                          Physical address, most        */
/*                                            significant word            */
/*    physical_lsw                          Physical address, least       */
/*                                            significant word            */
/*    interface_index                       Index to the network          */
/*                                            interface through which the */
/*                                            node is reachable.          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_nd_cache_find_entry_by_mac_addr   Internal find IP address in   */
/*                                          ND cache table mapped to      */
/*                                            input by mac address.       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
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
UINT  _nxd_nd_cache_ip_address_find(NX_IP *ip_ptr,
                                    NXD_ADDRESS *ip_address,
                                    ULONG physical_msw,
                                    ULONG physical_lsw,
                                    UINT *interface_index)
{
#ifdef FEATURE_NX_IPV6

ND_CACHE_ENTRY *entry;

    /* Find ND entry according to the given MAC address. */
    /* Obtain the protection. */
    tx_mutex_get(&ip_ptr -> nx_ip_protection, TX_WAIT_FOREVER);

    if (_nx_nd_cache_find_entry_by_mac_addr(ip_ptr, physical_msw, physical_lsw, &entry) != NX_SUCCESS)
    {

        /* Release the protection. */
        tx_mutex_put(&ip_ptr -> nx_ip_protection);

        /* No such MAC address found in cache table. */
        return(NX_ENTRY_NOT_FOUND);
    }

    /* Copy the IP address and version from the cache entry into the address structure. */
    ip_address -> nxd_ip_version = NX_IP_VERSION_V6;

    /*lint -e{644} suppress variable might not be initialized, since "entry" was initialized as long as previous call is NX_SUCCESS. */
    COPY_IPV6_ADDRESS(entry -> nx_nd_cache_dest_ip, ip_address -> nxd_ip_address.v6);

    /* If trace is enabled, insert this event into the trace buffer. */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_ND_CACHE_IP_ADDRESS_FIND, ip_ptr, ip_address -> nxd_ip_address.v6[3], physical_msw, physical_lsw, NX_TRACE_ARP_EVENTS, 0, 0);

    /* Get the interface_index.  */
    *interface_index = entry -> nx_nd_cache_interface_ptr -> nx_interface_index;

    /* Release the protection. */
    tx_mutex_put(&ip_ptr -> nx_ip_protection);

    /* Successful completion*/
    return(NX_SUCCESS);

#else /* !FEATURE_NX_IPV6 */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(physical_msw);
    NX_PARAMETER_NOT_USED(physical_lsw);
    NX_PARAMETER_NOT_USED(interface_index);

    return(NX_NOT_SUPPORTED);

#endif /* FEATURE_NX_IPV6 */
}

