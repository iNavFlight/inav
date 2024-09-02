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
/*    _nx_igmp_multicast_interface_join_internal          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles the request to join the specified multicast   */
/*    group on a specified network device.                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    group_address                         Multicast group to join       */
/*    interface_index                       Index to the interface        */
/*    update_time                           Update time                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*    (ip_link_driver)                      Associated IP link driver     */
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
UINT  _nx_igmp_multicast_interface_join_internal(NX_IP *ip_ptr, ULONG group_address,
                                                 UINT interface_index, UINT update_time)
{

UINT          i;
UINT          first_free;
NX_IP_DRIVER  driver_request;
NX_INTERFACE *nx_interface;

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_IGMP_MULTICAST_JOIN, ip_ptr, group_address, interface_index, 0, NX_TRACE_IGMP_EVENTS, 0, 0);

    /* Obtain the IP mutex so we can search the multicast join list.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    nx_interface = &ip_ptr -> nx_ip_interface[interface_index];

    /* Search the multicast join list for either the same group request.  */
    first_free =     NX_MAX_MULTICAST_GROUPS;

    for (i = 0; i < NX_MAX_MULTICAST_GROUPS; i++)
    {

        /* Determine if the specified entry is already in the multicast join list.  */
        if ((ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_join_list == group_address) &&
            (ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_join_interface_list == nx_interface))
        {

            /* Yes, we have found the same entry.  The only thing required in this
               case is to increment the join count and return.  */
            ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_join_count++;

            /* Release the IP protection.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));

            /* Return success!  */
            return(NX_SUCCESS);
        }

        /* Check for an empty entry.  */
        if ((ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_join_list == 0) &&
            (first_free == NX_MAX_MULTICAST_GROUPS))
        {

            /* Remember the first free entry.  */
            first_free =  i;
        }
    }

    /* At this point, we have a new entry.   First, check to see if there is an available
       entry.  */
    if (first_free == NX_MAX_MULTICAST_GROUPS)
    {

        /* Release the protection of the IP instance.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return an error code to indicate there are no more group addresses
           available.  */
        return(NX_NO_MORE_ENTRIES);
    }

    /* Register the new multicast group with the underlying driver to
       ensure that there is room for the new group at the hardware level.  */
    driver_request.nx_ip_driver_ptr                  =   ip_ptr;
    driver_request.nx_ip_driver_command              =   NX_LINK_MULTICAST_JOIN;
    driver_request.nx_ip_driver_physical_address_msw =   NX_IP_MULTICAST_UPPER;
    driver_request.nx_ip_driver_physical_address_lsw =   NX_IP_MULTICAST_LOWER | (group_address & NX_IP_MULTICAST_MASK);
    driver_request.nx_ip_driver_interface            =   nx_interface;

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_IO_DRIVER_MULTICAST_JOIN, ip_ptr, 0, 0, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

    (nx_interface -> nx_interface_link_driver_entry)(&driver_request);

    /* Check the return driver status.   */
    /*lint -e{644} suppress variable might not be initialized, since "nx_ip_driver_status" was initialized in nx_interface_link_driver_entry. */
    if (driver_request.nx_ip_driver_status != NX_SUCCESS)
    {

        /* Release the protection of the IP instance.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return an error code to indicate there are no more group addresses
           available.  */
        return(NX_NO_MORE_ENTRIES);
    }

    /* Set it up in the IP control structures.  */
    ip_ptr -> nx_ipv4_multicast_entry[first_free].nx_ipv4_multicast_join_list =           group_address;
    ip_ptr -> nx_ipv4_multicast_entry[first_free].nx_ipv4_multicast_join_interface_list = nx_interface;
    ip_ptr -> nx_ipv4_multicast_entry[first_free].nx_ipv4_multicast_join_count =          1;
    ip_ptr -> nx_ipv4_multicast_entry[first_free].nx_ipv4_multicast_update_time =         update_time;   /* Update on next IGMP periodic  */
    ip_ptr -> nx_ipv4_multicast_entry[first_free].nx_ipv4_multicast_loopback_enable =     ip_ptr -> nx_ip_igmp_global_loopback_enable;

#ifndef NX_DISABLE_IGMP_INFO
    /* Increment the IGMP groups joined count.  */
    ip_ptr -> nx_ip_igmp_groups_joined++;
#endif

    /* Release the protection over the IP instance.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return a successful status!  */
    return(NX_SUCCESS);
}
#endif /* !NX_DISABLE_IPV4  */

