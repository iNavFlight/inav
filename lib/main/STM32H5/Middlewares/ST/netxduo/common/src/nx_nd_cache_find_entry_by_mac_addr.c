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
#include "nx_ipv6.h"
#include "nx_nd_cache.h"

#ifdef FEATURE_NX_IPV6


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_nd_cache_find_entry_by_mac_addr                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function finds an entry in the ND cache that is       */
/*    mapped to the specified MAC address.                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                   Pointer to IP instance                     */
/*    physical_msw             Physical address, most significant word    */
/*    physical_lsw             Physical address, least significant word   */
/*    nd_cache_entry           User specified storage space for pointer to*/
/*                               the corresponding ND cache.              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                   NX_SUCCESS: The ND cache entry is located. */
/*                                nd_cache_entry contains valid value.    */
/*                             NX_NOT_SUCCESSFUL:  The ND cache entry     */
/*                                cannot be found, or nd_cache_entry is   */
/*                                NULL.                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get             Obtain protection mutex                    */
/*    tx_mutex_put             Release protection mutex                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_nd_cache_ip_address_find  User level service                   */
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
UINT _nx_nd_cache_find_entry_by_mac_addr(NX_IP *ip_ptr, ULONG physical_msw,
                                         ULONG physical_lsw, ND_CACHE_ENTRY **nd_cache_entry)
{
INT   i;
ULONG mac_msw, mac_lsw;

    /* Initialize the return value. */
    *nd_cache_entry = NX_NULL;

    /* Loop to match the physical address.  */
    for (i = 0; i < NX_IPV6_NEIGHBOR_CACHE_SIZE; i++)
    {

        /* Check the interface pointer.  */
        if (ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_interface_ptr == NX_NULL)
        {
            continue;
        }

        /* Check the ND CACHE status.  */
        if (ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_nd_status == ND_CACHE_STATE_INVALID)
        {
            continue;
        }

        /* Set the physical address.  */
        mac_msw = ((ULONG)ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_mac_addr[0] << 8) | ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_mac_addr[1];
        mac_lsw = ((ULONG)ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_mac_addr[2] << 24) | ((ULONG)ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_mac_addr[3] << 16) |
            ((ULONG)ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_mac_addr[4] << 8) | ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_mac_addr[5];

        /* Check the physical address.  */
        if ((mac_msw == physical_msw) && (mac_lsw == physical_lsw))
        {

            /* Find a match */
            *nd_cache_entry = &ip_ptr -> nx_ipv6_nd_cache[i];

            return(NX_SUCCESS);
        }
    }

    return(NX_NOT_SUCCESSFUL);
}

#endif /* FEATURE_NX_IPV6 */

