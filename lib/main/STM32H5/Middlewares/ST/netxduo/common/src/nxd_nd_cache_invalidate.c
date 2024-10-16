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
/*    _nxd_nd_cache_invalidate                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function invalidates the entire IPv6 Neighbor Discovery cache. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*    _nxd_nd_cache_invalidate_internal     Actual cache table invalidate */
/*                                          service                       */
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
UINT _nxd_nd_cache_invalidate(NX_IP *ip_ptr)
{

#ifdef FEATURE_NX_IPV6
INT idx;

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NXD_TRACE_ND_CACHE_INVALIDATE, ip_ptr, 0, 0, 0, NX_TRACE_ARP_EVENTS, 0, 0);

    /* Obtain the protection. */
    tx_mutex_get(&ip_ptr -> nx_ip_protection, TX_WAIT_FOREVER);

    /* Invalidate all entries. */
    for (idx = 0; idx < NX_IPV6_NEIGHBOR_CACHE_SIZE; idx++)
    {
        _nx_nd_cache_delete_internal(ip_ptr, &(ip_ptr -> nx_ipv6_nd_cache[idx]));
    }

    /* Release the protection. */
    tx_mutex_put(&ip_ptr -> nx_ip_protection);

    return(NX_SUCCESS);

#else /* !FEATURE_NX_IPV6 */

    NX_PARAMETER_NOT_USED(ip_ptr);

    return(NX_NOT_SUPPORTED);

#endif /* FEATURE_NX_IPV6 */
}

