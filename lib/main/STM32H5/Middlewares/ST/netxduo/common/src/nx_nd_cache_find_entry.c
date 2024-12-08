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
/*    nx_nd_cache_find_entry                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function finds an entry in the ND cache that is       */
/*    mapped to the specified IPv6 address.                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                   IP instance pointer                        */
/*    dest_ip                  The IP address to match                    */
/*    nd_cache_entry           User specified storage space of pointer to */
/*                                the corresponding ND cache.             */
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
/*    None                                                                */
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
UINT _nx_nd_cache_find_entry(NX_IP *ip_ptr,
                             ULONG *dest_ip, ND_CACHE_ENTRY **nd_cache_entry)
{
UINT i;
UINT index;

    /* Initialize the return value. */
    *nd_cache_entry = NX_NULL;

    /* Compute a simple hash based on the dest_ip */
    index = (UINT)((dest_ip[0] + dest_ip[1] + dest_ip[2] + dest_ip[3]) %
                   (NX_IPV6_NEIGHBOR_CACHE_SIZE));

    for (i = 0; i < NX_IPV6_NEIGHBOR_CACHE_SIZE; i++)
    {

        if ((ip_ptr -> nx_ipv6_nd_cache[index].nx_nd_cache_nd_status != ND_CACHE_STATE_INVALID) &&
            (ip_ptr -> nx_ipv6_nd_cache[index].nx_nd_cache_interface_ptr) &&
            (CHECK_IPV6_ADDRESSES_SAME(&ip_ptr -> nx_ipv6_nd_cache[index].nx_nd_cache_dest_ip[0], dest_ip)))
        {

            /* find the entry */
            *nd_cache_entry = &ip_ptr -> nx_ipv6_nd_cache[index];

            return(NX_SUCCESS);
        }

        index++;

        /* Check for overflow */
        if (index == NX_IPV6_NEIGHBOR_CACHE_SIZE)
        {
            index = 0;
        }
    }

    return(NX_NOT_SUCCESSFUL);
}

#endif /* FEATURE_NX_IPV6 */

