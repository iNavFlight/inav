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
/**   Internet Protocol (IPv6)                                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


#define NX_SOURCE_CODE


/* Include necessary system files.  */
#include "nx_api.h"
#include "nx_ipv6.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_ipv6_multicast_interface_join                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This service allows an application to join a specific IPv6          */
/*    multicast address on a specific physical interface.  The link       */
/*    driver is notified to add the multicast address.                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    group_address                         IPv6 multicast address        */
/*    interface_index                       Index to the phyical interface*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*    memset                                Clear the memory              */
/*    (ip_link_driver)                      Device driver entry point     */
/*    CHECK_IPV6_ADDRESSES_SAME             IPv6 address compare          */
/*    CHECK_UNSPECIFIED_ADDRESS             Check for :: address          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
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
UINT  _nxd_ipv6_multicast_interface_join(NX_IP *ip_ptr, NXD_ADDRESS *group_address, UINT interface_index)
{
#if defined(NX_ENABLE_IPV6_MULTICAST) && defined(FEATURE_NX_IPV6)

UINT          i;
UINT          first_free;
NX_IP_DRIVER  driver_request;
NX_INTERFACE *nx_interface;

    /* Obtain the IP mutex so we can search the multicast join list.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    nx_interface = &ip_ptr -> nx_ip_interface[interface_index];

    /* Search the multicast join list for either the same group request.  */
    first_free =     NX_MAX_MULTICAST_GROUPS;

    for (i = 0; i < NX_MAX_MULTICAST_GROUPS; i++)
    {

        /* Determine if the specified entry is already in the multicast join list.  */
        if ((nx_interface == ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_interface_list) &&
            (CHECK_IPV6_ADDRESSES_SAME(ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_list, group_address -> nxd_ip_address.v6)))
        {

            /* Yes, we have found the same entry.  The only thing required in this
               case is to increment the join count and return.  */
            ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_count++;

            /* Release the IP protection.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));

            /* Return number of the group.  */
            return(NX_SUCCESS);
        }

        /* Check for an empty entry.  */
        if (CHECK_UNSPECIFIED_ADDRESS(ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_list) && (first_free == NX_MAX_MULTICAST_GROUPS))
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

    /* Set it up in the IP control structures.  */
    ip_ptr -> nx_ipv6_multicast_entry[first_free].nx_ip_mld_join_list[0]        =    group_address -> nxd_ip_address.v6[0];
    ip_ptr -> nx_ipv6_multicast_entry[first_free].nx_ip_mld_join_list[1]        =    group_address -> nxd_ip_address.v6[1];
    ip_ptr -> nx_ipv6_multicast_entry[first_free].nx_ip_mld_join_list[2]        =    group_address -> nxd_ip_address.v6[2];
    ip_ptr -> nx_ipv6_multicast_entry[first_free].nx_ip_mld_join_list[3]        =    group_address -> nxd_ip_address.v6[3];
    ip_ptr -> nx_ipv6_multicast_entry[first_free].nx_ip_mld_join_interface_list =    nx_interface;
    ip_ptr -> nx_ipv6_multicast_entry[first_free].nx_ip_mld_join_count          =    1;

    /* Increment the MLD groups joined count.  */
    ip_ptr -> nx_ipv6_multicast_groups_joined++;

    /* Register the new multicast group with the underlying driver to
       ensure that there is room for the new group at the hardware level.  */
    driver_request.nx_ip_driver_ptr                  =   ip_ptr;
    driver_request.nx_ip_driver_command              =   NX_LINK_MULTICAST_JOIN;
    driver_request.nx_ip_driver_physical_address_msw =   0x00003333;
    driver_request.nx_ip_driver_physical_address_lsw =   group_address -> nxd_ip_address.v6[3];
    driver_request.nx_ip_driver_interface            =   nx_interface;

    (nx_interface -> nx_interface_link_driver_entry)(&driver_request);

    /* Check the return driver status.   */
    /*lint -e{644} suppress variable might not be initialized, since "nx_ip_driver_status" was initialized in nx_interface_link_driver_entry. */
    if (driver_request.nx_ip_driver_status != NX_SUCCESS)
    {

        /* Release the protection of the IP instance.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return an error code to indicate there are no more group addresses
           available.  */
        return(NX_OVERFLOW);
    }

    /* Release the protection over the IP instance.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return SUCCESS.  */
    return(NX_SUCCESS);

#else
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(group_address);
    NX_PARAMETER_NOT_USED(interface_index);

    return(NX_NOT_SUPPORTED);

#endif /* NX_ENABLE_IPV6_MULTICAST && FEATURE_NX_IPV6 */
}

