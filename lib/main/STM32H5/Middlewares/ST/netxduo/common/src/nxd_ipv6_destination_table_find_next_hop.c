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
#include "nx_ipv6.h"
#include "nx_icmpv6.h"

#ifdef FEATURE_NX_IPV6

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_ipv6_destination_table_find_next_hop           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This is an internal function.  It searches the destination table    */
/*    (equivalent to IPv4 static routing table) for a possible match.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP.                */
/*    destination_ip                        IP address to search for.     */
/*    next_hop                              Storage space for next hop    */
/*                                             address.                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Valid next hop returned       */
/*    NX_NOT_SUCCESSFUL                     Invalid pointer input or no   */
/*                                            match found; next hop not   */
/*                                            returned                    */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmpv6_process_redirect           Process incoming redirect     */
/*                                             message.                   */
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
UINT _nxd_ipv6_destination_table_find_next_hop(NX_IP *ip_ptr, ULONG *destination_ip, ULONG *next_hop)
{

UINT i, table_size;
UINT status;


    status = NX_NOT_SUCCESSFUL;

    /* Next hop storage must not be valid. */
    NX_ASSERT(next_hop != NX_NULL);

    /* Set a local variable for convenience. */
    table_size = ip_ptr -> nx_ipv6_destination_table_size;

    /* Check the num of destination. */
    if (table_size == 0)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    /* Loop through all entries. */
    for (i = 0; table_size && (i < NX_IPV6_DESTINATION_TABLE_SIZE); i++)
    {

        /* Skip invalid entries. */
        if (!ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_valid)
        {
            continue;
        }

        /* Keep track of valid entries we have checked. */
        table_size--;

        /* Check whether or not the address is the same. */
        if (CHECK_IPV6_ADDRESSES_SAME(ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_destination_address, destination_ip))
        {

            /* Copy next hop address to user-supplied storage. */
            COPY_IPV6_ADDRESS(ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_next_hop, next_hop);

            status = NX_SUCCESS;

            /* break out of the for loop */
            break;
        }
    }


    return(status);
}


#endif  /* FEATURE_NX_IPV6 */

