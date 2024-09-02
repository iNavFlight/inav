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
/**   Internet Protocol (IP)                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ip.h"
#include "nx_ipv6.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_ipv6_address_delete                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*  This function removes the IPv6 address at the specified address list  */
/*  index on the IP instance.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                            IP control block pointer          */
/*    address_index                     Index into IPv6 address list      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                      Get protection mutex              */
/*    tx_mutex_put                      Put protection mutex              */
/*    nx_ipv6_multicast_leave           Leave IPv6 multicast group        */
/*    [ipv6_address_change_notify]      User callback function            */
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
UINT  _nxd_ipv6_address_delete(NX_IP *ip_ptr, UINT address_index)
{
#ifdef FEATURE_NX_IPV6
UINT              result;
NXD_IPV6_ADDRESS *ipv6_address, *address_list;
#ifdef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
VOID              (*address_change_notify)(NX_IP *, UINT, UINT, UINT, ULONG *);
UINT              if_index;
ULONG             obsoleted_address[4];

    address_change_notify = NX_NULL;
#endif /* NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY */


    result = NX_NO_INTERFACE_ADDRESS;

    /* Place protection while the IPv6 address is modified. */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);


    /* Get the ip address.  */
    ipv6_address = &ip_ptr -> nx_ipv6_address[address_index];

    /* Check the validity of the address.  */
    if (ipv6_address -> nxd_ipv6_address_valid)
    {

        /* Delete the information in the list.  */
        address_list = ipv6_address -> nxd_ipv6_address_attached -> nxd_interface_ipv6_address_list_head;


        /* The delete address is in the head of the list.  */
        if (ipv6_address == address_list)
        {

            ipv6_address -> nxd_ipv6_address_attached -> nxd_interface_ipv6_address_list_head = ipv6_address -> nxd_ipv6_address_next;
            result = NX_SUCCESS;
        }
        else
        {
            /* Find the address in the list.  */
            while (address_list && (address_list -> nxd_ipv6_address_next != ipv6_address))
            {

                /* Move to the next address. */
                address_list = address_list -> nxd_ipv6_address_next;
            }

            /* Break out of the while loop, either the address has been found, or the end of the list has been reached. */
            if (address_list)
            {
                address_list -> nxd_ipv6_address_next = ipv6_address -> nxd_ipv6_address_next;
                result = NX_SUCCESS;
            }
        }

        if (result == NX_SUCCESS)
        {
        ULONG multicast_address[4];

            SET_SOLICITED_NODE_MULTICAST_ADDRESS(multicast_address, ipv6_address -> nxd_ipv6_address);
            /* First remove the corresponding solicited node multicast address. */
            _nx_ipv6_multicast_leave(ip_ptr, &multicast_address[0], ipv6_address -> nxd_ipv6_address_attached);

#ifdef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
            /* Pickup the current notification callback and additional information pointers.  */
            address_change_notify =  ip_ptr -> nx_ipv6_address_change_notify;

            obsoleted_address[0] = ipv6_address -> nxd_ipv6_address[0];
            obsoleted_address[1] = ipv6_address -> nxd_ipv6_address[1];
            obsoleted_address[2] = ipv6_address -> nxd_ipv6_address[2];
            obsoleted_address[3] = ipv6_address -> nxd_ipv6_address[3];

            if_index = ipv6_address -> nxd_ipv6_address_attached -> nx_interface_index;
#endif /* NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY */

            /* At this point ipv6_address is off the interface IPv6 address list. */
            memset(ipv6_address, 0, sizeof(NXD_IPV6_ADDRESS));

            /* Set index of ipv6_address. */
            ipv6_address -> nxd_ipv6_address_index = (UCHAR)address_index;
        }
    }

    /* Release the protection while the IPv6 address is modified. */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));


#ifdef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
    /* Is the application configured for notification of address changes and/or
       prefix_length change?  */
    if (address_change_notify && (result == NX_SUCCESS))
    {

        /* Yes, call the application's address change notify function.  */
        /*lint -e{644} suppress variable might not be initialized, since "if_index" was initialized as long as result is NX_SUCCESS. */
        (address_change_notify)(ip_ptr, NX_IPV6_ADDRESS_MANUAL_DELETE, if_index, address_index, &obsoleted_address[0]);
    }
#endif /* NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY */

    /* Return completion status.  */
    return(result);

#else /* !FEATURE_NX_IPV6 */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(address_index);

    return(NX_NOT_SUPPORTED);

#endif /* FEATURE_NX_IPV6 */
}

