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
/*    _nx_icmpv6_dest_table_find                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds a destination table entry based on destination  */
/*    address.  If no match is found, a the destination entry pointer is  */
/*    set to NULL, but a successful search status returned.               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                               Pointer to IP                  */
/*    destination_address                  Destination IP address.        */
/*    dest_entry_ptr                       Pointer to location of matching*/
/*                                             table entry, if any.       */
/*    path_mtu                             Optional path mtu update.      */
/*                                           Note: caller shall make sure */
/*                                           MTU does not exceed physical */
/*                                           link MTU size.               */
/*    mtu_timeout                          Optional entry timeout update  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Search successfully completed */
/*                                             regardless if match found  */
/*    NX_NOT_SUCCESSFUL                     Invalid input; search aborted */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    nx_icmpv6_send_queued_packets         Send queued ICMPv6 packets    */
/*    nx_ipv6_send_packet                   Send specified IPv6 packet    */
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
UINT _nx_icmpv6_dest_table_find(NX_IP *ip_ptr, ULONG *destination_address, NX_IPV6_DESTINATION_ENTRY **dest_entry_ptr,
                                ULONG path_mtu, ULONG mtu_timeout)
{

UINT i, table_size;

    /* Destination address must be valid. */
    NX_ASSERT((destination_address != NX_NULL) && (dest_entry_ptr != NULL));

    /* Set a local variable for convenience. */
    table_size = ip_ptr -> nx_ipv6_destination_table_size;

    /* Check the destination num. */
    if (table_size == 0)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    /* Initialize the return value. */
    *dest_entry_ptr = NX_NULL;

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
        if (CHECK_IPV6_ADDRESSES_SAME(&ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_destination_address[0], destination_address))
        {

#ifdef NX_ENABLE_IPV6_PATH_MTU_DISCOVERY

            /* Update the table entry if the supplied path MTU is non zero and
               does not exceed our IP instance MTU. */
            if (path_mtu > 0)
            {

                /* Do we have a valid timeout e.g. did this information come from an RA packet?
                   Or is a packet too big message indicating we need to decrease our path MTU? */
                if ((mtu_timeout > 0) || (ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_path_mtu > path_mtu))
                {

                    /* OK to change the path MTU. */
                    ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_path_mtu = path_mtu;

                    /* Was a valid timeout supplied? */
                    if (mtu_timeout > 0)
                    {

                        /* Yes;  Ok to update table entry with the specified timeout. */
                        ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_MTU_timer_tick = mtu_timeout;
                    }
                    else
                    {
                        /* No; the path MTU changed in response to a packet too big error
                           message.  Set the table entry timeout to the required wait
                           interval.  We cannot attempt to restore (increase) the path MTU
                           before this time out expires. */
                        ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_MTU_timer_tick = NX_PATH_MTU_INCREASE_WAIT_INTERVAL_TICKS;
                    }
                }

                /* Else this information presumably comes a Packet too Big message.
                   RFC 1981 disallows increasing a path MTU based on a PTB message
                   (decreasing is allowed and required). */
            }
#else
            NX_PARAMETER_NOT_USED(path_mtu);
            NX_PARAMETER_NOT_USED(mtu_timeout);
#endif /* NX_ENABLE_IPV6_PATH_MTU_DISCOVERY */

            *dest_entry_ptr = &ip_ptr -> nx_ipv6_destination_table[i];

            return(NX_SUCCESS);
        }
    }

    return(NX_NOT_SUCCESSFUL);
}

#endif  /* FEATURE_NX_IPV6 */

