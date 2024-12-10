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
/**   Internet Control Message Protocol (ICMP)                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_nd_cache.h"
#include "nx_icmpv6.h"


#ifdef FEATURE_NX_IPV6


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmp_DAD_clear_NDCache_entry                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to the IP instance    */
/*    ip_addr                               Neighbor IPv6 address of cache*/
/*                                             entry to delete            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                        Obtain exclusive lock (on table)*/
/*    tx_mutex_put                        Release exclusive lock          */
/*    _nx_nd_cache_find_entry             Find cache entry by IP address  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmpv6_perform_DAD              Verify IPv6 address is unique   */
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
void _nx_icmpv6_DAD_clear_NDCache_entry(NX_IP *ip_ptr, ULONG *ip_addr)
{

ND_CACHE_ENTRY *NDCacheEntry;

    /* Find the ND CACHE entry.  */
    if (_nx_nd_cache_find_entry(ip_ptr, ip_addr, &NDCacheEntry) == NX_SUCCESS)
    {

        /*lint -e{644} suppress variable might not be initialized, since "NDCacheEntry" was initialized in _nx_nd_cache_find_entry. */
        NDCacheEntry -> nx_nd_cache_nd_status = ND_CACHE_STATE_INVALID;
    }

    return;
}
#endif /* FEATURE_NX_IPV6 */

