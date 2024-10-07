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
/*    _nxd_nd_cache_entry_set                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates an entry with the specified IPv6 address and  */
/*    hardware MAC address mapping and adds it to the Neighbor Discovery  */
/*    (ND) cache.                                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to the IP instance    */
/*    dest_ip                               Pointer to the IP address     */
/*                                            to add (map)                */
/*    interface_index                       Index to the network          */
/*                                            interface                   */
/*    mac                                   Pointer to the MAC address to */
/*                                            be added (map)              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_nd_cache_add                      Actual function to add an     */
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
UINT _nxd_nd_cache_entry_set(NX_IP *ip_ptr, ULONG *dest_ip, UINT interface_index, CHAR *mac)
{
#ifdef FEATURE_NX_IPV6

ND_CACHE_ENTRY *nd_cache_entry;
UINT            status;


    /* If trace is enabled, insert this event into the trace buffer. */
    NX_TRACE_IN_LINE_INSERT(NXD_TRACE_ND_CACHE_ENTRY_SET, dest_ip[3], ((mac[0] << 16) | mac[1]), ((mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) | mac[5]),
                            0, NX_TRACE_ARP_EVENTS, 0, 0);

    /* Call the actual cache entry add service. */
    status = _nx_nd_cache_add(ip_ptr, dest_ip, ip_ptr -> nx_ipv6_address[interface_index].nxd_ipv6_address_attached, mac, 1, ND_CACHE_STATE_REACHABLE, &(ip_ptr -> nx_ipv6_address[interface_index]), &nd_cache_entry);

    return(status);

#else /* !FEATURE_NX_IPV6 */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(dest_ip);
    NX_PARAMETER_NOT_USED(interface_index);
    NX_PARAMETER_NOT_USED(mac);

    return(NX_NOT_SUPPORTED);

#endif /* FEATURE_NX_IPV6 */
}

