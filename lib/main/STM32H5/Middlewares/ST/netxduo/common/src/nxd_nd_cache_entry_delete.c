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
/*    _nxd_nd_cache_entry_delete                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes an entry in the ND cache table.               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    dest_ip                               Pointer to IP address to      */
/*                                            be deleted                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*    _nx_nd_cache_delete_internal          Actual function to add an     */
/*                                             entry to the cache.        */
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
UINT  _nxd_nd_cache_entry_delete(NX_IP *ip_ptr, ULONG *dest_ip)
{
#ifdef FEATURE_NX_IPV6
UINT            status;
ND_CACHE_ENTRY *entry;

    /* Obtain the protection. */
    tx_mutex_get(&ip_ptr -> nx_ip_protection, TX_WAIT_FOREVER);

    /* If trace is enabled, insert this event into the trace buffer. */
    NX_TRACE_IN_LINE_INSERT(NXD_TRACE_ND_CACHE_DELETE, dest_ip[3], 0, 0, 0, NX_TRACE_ARP_EVENTS, 0, 0);

    /* If not found in the ND cache return an error status. */
    if (_nx_nd_cache_find_entry(ip_ptr, dest_ip, &entry) != NX_SUCCESS)
    {

        /* Release the protection. */
        tx_mutex_put(&ip_ptr -> nx_ip_protection);
        return(NX_ENTRY_NOT_FOUND);
    }

    /*lint -e{644} suppress variable might not be initialized, since "entry" was initialized as long as previous call is NX_SUCCESS. */
    status = _nx_nd_cache_delete_internal(ip_ptr, entry);

    /* Release the protection. */
    tx_mutex_put(&ip_ptr -> nx_ip_protection);

    return(status);

#else /* !FEATURE_NX_IPV6 */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(dest_ip);

    return(NX_NOT_SUPPORTED);

#endif /* FEATURE_NX_IPV6 */
}

