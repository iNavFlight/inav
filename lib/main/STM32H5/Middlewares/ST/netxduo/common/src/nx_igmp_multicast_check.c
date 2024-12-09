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
/**   Internet Group Management Protocol (IGMP)                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_igmp.h"


#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_igmp_multicast_check                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks the list of joined multicast addresses to see  */
/*    if the incoming address matches.  If the specified group is         */
/*    "all hosts" or if a match is found, NX_TRUE is returned to the      */
/*    caller.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    group                                 Multicast group IP address    */
/*    nx_interface                          Pointer to interface          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_TRUE                               If a match is found           */
/*    NX_FALSE                              Otherwise                     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ipv4_packet_receive               Raw IP packet receive         */
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
UINT  _nx_igmp_multicast_check(NX_IP *ip_ptr, ULONG group, NX_INTERFACE *nx_interface)
{

UINT i;

    /* Check for "all hosts" group.  We always assume all hosts membership.  */
    /*lint -e{835} -e{845} suppress operating on zero. */
    if (group ==  NX_ALL_HOSTS_ADDRESS)
    {
        return(NX_TRUE);
    }

    /* Loop through the IP multicast join list to find the matching group that is being
       responded to by another host on this same network.  */

    for (i = 0; i < NX_MAX_MULTICAST_GROUPS; i++)
    {

        /* Check for a match.  */
        if ((ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_join_list == group) &&
            (nx_interface == ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_join_interface_list))
        {
            return(NX_TRUE);
        }
    }

    /* Otherwise, we have searched the entire list, return false.  */
    return(NX_FALSE);
}
#endif /* !NX_DISABLE_IPV4  */

