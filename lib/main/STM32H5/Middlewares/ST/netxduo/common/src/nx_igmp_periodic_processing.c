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
/*    _nx_igmp_periodic_processing                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles the sending of periodic processing of IGMP    */
/*    join report messages.                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_igmp_interface_report_send         Send IGMP group report        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ip_thread_entry                   IP helper thread              */
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
VOID  _nx_igmp_periodic_processing(NX_IP *ip_ptr)
{

UINT          i;
UINT          status;
UINT          sent_count = 0;
NX_INTERFACE *interface_ptr;
UINT          interface_index;


    /* Search the multicast join list for pending IGMP responses.  */
    for (i = 0; i < NX_MAX_MULTICAST_GROUPS; i++)
    {

        /* Determine if the specified entry is active.  */
        if (ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_join_list)
        {

            /* Now determine if a response is pending.  */
            if ((ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_update_time > 0) &&
                (ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_update_time != NX_WAIT_FOREVER))
            {

                /* Yes, it is active.  Decrement and check for expiration.  */
                ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_update_time--;

                /* We don't want to decrement a join group if we cannot send it. Check
                   if we've already sent a packet on this periodic. */
                if ((ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_update_time == 0) && (sent_count > 0))
                {

                    /* Restore the timeout to 1 second because we cannot send on this periodic; we've already sent out a packet. */
                    ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_update_time = 1;
                }

                /* Has time expired and have we not sent an IGMP report in this period?  */
                if (ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_update_time == 0)
                {

                    /* Yes, time has expired and we have not yet sent a packet out on this periodic. */

                    /* Build and send the join report packet. */
                    interface_ptr = ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_join_interface_list;

                    interface_index = (UINT)interface_ptr -> nx_interface_index;

                    /* Build a IGMP host response packet for a join report and send it!  */
                    status = _nx_igmp_interface_report_send(ip_ptr, ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_join_list,
                                                            interface_index, NX_TRUE);

                    if (status == NX_SUCCESS)
                    {

                        /* Update the sent count. Only one report sent per IP periodic. */
                        sent_count++;
                    }
                }
            }
        }
    }
}
#endif /* !NX_DISABLE_IPV4  */

