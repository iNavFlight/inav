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
/*    _nxd_nd_cache_hardware_address_find                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds the hardware address in the neighbor discovery  */
/*    (ND) cache table mapped to the input IP address.                    */
/*    address.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    ip_address                            Pointer to the IP address to  */
/*                                            search for                  */
/*    physical_msw                          Physical address, most        */
/*                                            signifcant word             */
/*    physical_lsw                          Physical address, least       */
/*                                            signifcant word             */
/*    interface_index                       Pointer to interface through  */
/*                                            which the neighbor can be   */
/*                                            reached.                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*    _nx_nd_cache_find_entry               Find entry by IP address.     */
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
UINT  _nxd_nd_cache_hardware_address_find(NX_IP *ip_ptr,
                                          NXD_ADDRESS *ip_address,
                                          ULONG *physical_msw,
                                          ULONG *physical_lsw,
                                          UINT *interface_index)
{
#ifdef FEATURE_NX_IPV6

ND_CACHE_ENTRY *entry;

    /* Obtain the protection. */
    tx_mutex_get(&ip_ptr -> nx_ip_protection, TX_WAIT_FOREVER);

    /* Find ND cache entry for a given IP address. */
    if (_nx_nd_cache_find_entry(ip_ptr, ip_address -> nxd_ip_address.v6, &entry))
    {
        /* Release the protection. */
        tx_mutex_put(&ip_ptr -> nx_ip_protection);

        /* Not found */
        return(NX_ENTRY_NOT_FOUND);
    }

    /* Construct the MAC address. */
    /*lint -e{644} suppress variable might not be initialized, since "entry" was initialized when the return value of _nx_nd_cache_find_entry is NX_SUCCESS. */
    *physical_msw = ((ULONG)entry -> nx_nd_cache_mac_addr[0]) << 8 | (ULONG)entry -> nx_nd_cache_mac_addr[1];
    *physical_lsw = ((ULONG)entry -> nx_nd_cache_mac_addr[2]) << 24 | ((ULONG)entry -> nx_nd_cache_mac_addr[3]) << 16 |
        ((ULONG)entry -> nx_nd_cache_mac_addr[4]) << 8 | (ULONG)entry -> nx_nd_cache_mac_addr[5];

    /* Get the interface_index.  */
    *interface_index = (entry -> nx_nd_cache_interface_ptr -> nx_interface_index);

    /* Release the protection. */
    tx_mutex_put(&ip_ptr -> nx_ip_protection);

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

