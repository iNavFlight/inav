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
/*    _nxd_ipv6_multicast_interface_leave                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This service removes the specific IPv6 multicast address from the   */
/*    sepcific physical interface.  The link driver is also notified of   */
/*    the removal of the IPv6 multicast address.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    group_address                         IPv6 multicast address        */
/*    interface_index                       Index to physical interface   */
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
UINT  _nxd_ipv6_multicast_interface_leave(NX_IP *ip_ptr, NXD_ADDRESS *group_address, UINT interface_index)
{
#if defined(NX_ENABLE_IPV6_MULTICAST) && defined(FEATURE_NX_IPV6)

UINT          i;
NX_IP_DRIVER  driver_request;
NX_INTERFACE *nx_interface;

    /* Obtain the IP mutex so we can search the multicast join list.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    nx_interface = &ip_ptr -> nx_ip_interface[interface_index];

    for (i = 0; i < NX_MAX_MULTICAST_GROUPS; i++)
    {

        /* Determine if the specified entry is present.  */
        if ((nx_interface == ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_interface_list) && (CHECK_IPV6_ADDRESSES_SAME(ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_list, group_address -> nxd_ip_address.v6)))
        {

            /* Yes, we have found the same entry.  */

            /* Decrease the join count.  */
            ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_count--;

            /* Determine if there are no other join requests for this group address.  */
            if (ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_count == 0)
            {

                /* Clear the group join value.  */
                memset(ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_list, 0, 4 * sizeof(ULONG));

                /* Decrement the MLD groups joined count.  */
                ip_ptr -> nx_ipv6_multicast_groups_joined--;

                /* Un-register the new multicast group with the underlying driver.  */
                driver_request.nx_ip_driver_ptr =                    ip_ptr;
                driver_request.nx_ip_driver_command =                NX_LINK_MULTICAST_LEAVE;
                driver_request.nx_ip_driver_physical_address_msw =   0x00003333;
                driver_request.nx_ip_driver_physical_address_lsw =   group_address -> nxd_ip_address.v6[3];
                driver_request.nx_ip_driver_interface =              nx_interface;

                (ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_interface_list -> nx_interface_link_driver_entry)(&driver_request);

                /* Now clear the interface entry. */
                ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_interface_list = NX_NULL;
            }

            /* Release the IP protection.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));

            /* Return success!  */
            return(NX_SUCCESS);
        }
    }

    /* At this point we know that the supplied entry was not found.  */

    /* Release the protection of the IP instance.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return an error code.  */
    return(NX_ENTRY_NOT_FOUND);

#else
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(group_address);
    NX_PARAMETER_NOT_USED(interface_index);

    return(NX_NOT_SUPPORTED);

#endif /* NX_ENABLE_IPV6_MULTICAST && FEATURE_NX_IPV6 */
}

