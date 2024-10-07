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
/*    _nx_igmp_multicast_interface_leave_internal         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles the request to leave the specified multicast  */
/*    group.                                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    group_address                         Multicast group to join       */
/*    interface_index                       Interface index               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*    NX_ENTRY_NOT_FOUND                    Group address not found in the*/
/*                                                IGMP join list          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*    (ip_link_driver)                      Associated IP link driver     */
/*    nx_igmp_interface_report_send         Send IGMP group reports       */
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
UINT  _nx_igmp_multicast_interface_leave_internal(NX_IP *ip_ptr, ULONG group_address, UINT interface_index)
{

UINT          i;
NX_IP_DRIVER  driver_request;
NX_INTERFACE *nx_interface;


    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_IGMP_MULTICAST_LEAVE, ip_ptr, group_address, 0, 0, NX_TRACE_IGMP_EVENTS, 0, 0);

    /* Obtain the IP mutex so we can search the multicast join list.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    nx_interface = &ip_ptr -> nx_ip_interface[interface_index];

    /* Search the multicast join list for the group request.  */

    for (i = 0; i < NX_MAX_MULTICAST_GROUPS; i++)
    {

        /* Determine if the specified entry is present.  */
        if ((ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_join_list == group_address) &&
            (ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_join_interface_list == nx_interface))
        {

            /* Yes, we have found the same entry.  */

            /* Decrease the join count.  */
            ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_join_count--;

            /* Determine if there are no other join requests for this group address.  */
            if (ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_join_count == 0)
            {

                /* Clear the group join value.  */
                ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_join_list = 0;

                /* Un-register the new multicast group with the underlying driver.  */
                driver_request.nx_ip_driver_ptr =                    ip_ptr;
                driver_request.nx_ip_driver_command =                NX_LINK_MULTICAST_LEAVE;
                driver_request.nx_ip_driver_physical_address_msw =   NX_IP_MULTICAST_UPPER;
                driver_request.nx_ip_driver_physical_address_lsw =   NX_IP_MULTICAST_LOWER | (group_address & NX_IP_MULTICAST_MASK);
                driver_request.nx_ip_driver_interface =              ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_join_interface_list;

                /* If trace is enabled, insert this event into the trace buffer.  */
                NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_IO_DRIVER_MULTICAST_LEAVE, ip_ptr, 0, 0, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

                (ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_join_interface_list -> nx_interface_link_driver_entry)(&driver_request);

                /* Clear the interface entry for version IGMPv1. Don't need it anymore. */
                ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_join_interface_list = NX_NULL;

#ifndef NX_DISABLE_IGMP_INFO
                /* Decrement the IGMP groups joined count.  */
                ip_ptr -> nx_ip_igmp_groups_joined--;
#endif

#ifndef NX_DISABLE_IGMPV2
                /* Check if the entry is a local multicast join. */
                if (ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_update_time == NX_WAIT_FOREVER)
                {

                    /* It is. Release the IP protection.  */
                    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

                    /* Return success.  */
                    return(NX_SUCCESS);
                }

                /* IGMPv1 hosts do not send a leave group message. */
                if (ip_ptr -> nx_ip_igmp_router_version == NX_IGMP_HOST_VERSION_1)
                {

                    /* Release the IP protection.  */
                    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

                    /* Return success!  */
                    return(NX_SUCCESS);
                }

                /* Build and send the leave report packet. */
                _nx_igmp_interface_report_send(ip_ptr, group_address, interface_index, NX_FALSE);

#endif /* NX_DISABLE_IGMPV2 */
            }

            /* Release the IP protection.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));

            /* Return success.  */
            return(NX_SUCCESS);
        }
    }

    /* At this point we know that the supplied entry was not found.  */

    /* Release the protection of the IP instance.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return an error code.  */
    return(NX_ENTRY_NOT_FOUND);
}
#endif /* !NX_DISABLE_IPV4  */

