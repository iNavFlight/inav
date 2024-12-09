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

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ipv6.h"
#include "nx_nd_cache.h"




#ifdef FEATURE_NX_IPV6
/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Component                                                        */
/**                                                                       */
/**   Internet Protocol version 6 Prefix Table (IPv6 prefix table)        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/




/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ipv6_prefix_list_delete                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function deletes an entry from the prefix list based  */
/*    on the given prefix.                                                */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    prefix                                128-bit IPv6 prefix.          */
/*    prefix_length                         Length of the prefix.         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ipv6_prefix_list_delete_entry     Delete IPv6 prefix entry      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmpv6_process_ra                 ICMPv6 Router Advertisement   */
/*                                          process routine.              */
/*                                                                        */
/*  NOTE                                                                  */
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
VOID _nx_ipv6_prefix_list_delete(NX_IP *ip_ptr, ULONG *prefix, INT prefix_length)
{

NX_IPV6_PREFIX_ENTRY *current;


    /* Quick reference to the head of the prefix list. */
    current = ip_ptr -> nx_ipv6_prefix_list_ptr;

    /* Go through all the entries. */
    while (current)
    {

        /* If prefix length matches, and the prefix addresses also match...*/
        if ((current -> nx_ipv6_prefix_entry_prefix_length == (ULONG)prefix_length) &&
            CHECK_IPV6_ADDRESSES_SAME(prefix, current -> nx_ipv6_prefix_entry_network_address))
        {

            /* Delete this entry. */
            _nx_ipv6_prefix_list_delete_entry(ip_ptr, current);

            /* All done. return */
            return;
        }
        /* Move to the next entry. */
        current = current -> nx_ipv6_prefix_entry_next;
    }

    /* No match was found. */
    return;
}


#endif /* FEATURE_NX_IPV6 */

