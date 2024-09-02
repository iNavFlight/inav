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
#include "nx_ipv6.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_ipv6_address_set                               PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the IP address and the prefix length for the     */
/*    supplied IP instance.                                               */
/*                                                                        */
/*    For the IPv6 address, an application is only allowed to change the  */
/*    global IP address.  The link local address is generated through the */
/*    underlying interface address, therefore cannot be reconfigured by   */
/*    user level applications.                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP control block pointer      */
/*    interface_index                       The index to the physical     */
/*                                            interface this address      */
/*                                            belongs to                  */
/*    ip_address                            Pointer to IP address         */
/*                                            structure                   */
/*    prefix_length                         Prefix length                 */
/*    address_index                         Index to the IPv6 address     */
/*                                            table                       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ipv6_multicast_join               Multicast group join service  */
/*    [ipv6_address_change_notify]          User callback function        */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
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
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            added internal ip address   */
/*                                            change notification,        */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
UINT  _nxd_ipv6_address_set(NX_IP *ip_ptr, UINT interface_index, NXD_ADDRESS *ip_address, ULONG prefix_length, UINT *address_index)
{
#ifdef FEATURE_NX_IPV6

TX_INTERRUPT_SAVE_AREA
#ifdef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
VOID                (*address_change_notify)(NX_IP *, UINT, UINT, UINT, ULONG *);
VOID                (*address_change_notify_internal)(NX_IP *, UINT, UINT, UINT, ULONG *);
#endif /* NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY */
NXD_IPV6_ADDRESS *ipv6_addr;
NXD_IPV6_ADDRESS *interface_ipv6_address;
UINT              index = (UINT)0xFFFFFFFF;
UINT              i;
ULONG             multicast_address[4];

    /* Place protection while the IPv6 address is modified. */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    if (ip_address)
    {

        /* Perform duplicate address detection.  */
        for (i = 0; i < NX_MAX_IPV6_ADDRESSES; i++)
        {
            if ((ip_ptr -> nx_ipv6_address[i].nxd_ipv6_address_valid) &&
                (ip_ptr -> nx_ipv6_address[i].nxd_ipv6_address[0] == ip_address -> nxd_ip_address.v6[0]) &&
                (ip_ptr -> nx_ipv6_address[i].nxd_ipv6_address[1] == ip_address -> nxd_ip_address.v6[1]) &&
                (ip_ptr -> nx_ipv6_address[i].nxd_ipv6_address[2] == ip_address -> nxd_ip_address.v6[2]) &&
                (ip_ptr -> nx_ipv6_address[i].nxd_ipv6_address[3] == ip_address -> nxd_ip_address.v6[3]))
            {

                /* The IPv6 address already exists.  */
                tx_mutex_put(&(ip_ptr -> nx_ip_protection));
                return(NX_DUPLICATED_ENTRY);
            }
        }
    }

    /* Find an avaiable IPv6 address structure. */
    for (i = 0; i < NX_MAX_IPV6_ADDRESSES; i++)
    {
        /* Look for invalid entries. */
        if (!ip_ptr -> nx_ipv6_address[i].nxd_ipv6_address_valid)
        {

            /* An available entry is found. */
            index = i;
            break;
        }
    }

    if (index == (UINT)0xFFFFFFFF)
    {
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));
        return(NX_NO_MORE_ENTRIES);
    }

    if (address_index)
    {
        *address_index = index;
    }

    /* Pointer to the IPv6 address that needs to be modified. */
    ipv6_addr = &(ip_ptr -> nx_ipv6_address[index]);

    /* The address is null.  */
    if ((!ip_address) && (prefix_length == 10))
    {

    /* No address supplied. Assume it is link local address, constructed from the physical interface. */
    ULONG word2, word3;

        /* Construct Interface Identifier, following RFC2464, page 3 */
        /* Assign link local address.
           LL address is constructed by:
           0xFE80::{64 bit interface ID}.  See RFC 4291 */

        word2 = ip_ptr -> nx_ip_interface[interface_index].nx_interface_physical_address_msw << 16 |
            ((ip_ptr -> nx_ip_interface[interface_index].nx_interface_physical_address_lsw & 0xFF000000) >> 16) | 0xFF;

        /* Fix the 2nd lower-order bit of the 1st byte, RFC2464, page 3 */
        word2 = (word2 & 0xFDFFFFFF) | (~(word2 | 0xFDFFFFFF));
        word3 = (ip_ptr -> nx_ip_interface[interface_index].nx_interface_physical_address_lsw & 0x00FFFFFF) | 0xFE000000;

        /* Point to interface link list head  */
        interface_ipv6_address = ip_ptr -> nx_ip_interface[interface_index].nxd_interface_ipv6_address_list_head;

        /* Perform link local duplicate address detection.  */
        while (interface_ipv6_address)
        {
            if ((interface_ipv6_address -> nxd_ipv6_address[0] == 0xFE800000) &&
                (interface_ipv6_address -> nxd_ipv6_address[1] == 0x00000000) &&
                (interface_ipv6_address -> nxd_ipv6_address[2] == word2) &&
                (interface_ipv6_address -> nxd_ipv6_address[3] == word3))
            {

                /* The IPv6 address already exists.  */
                tx_mutex_put(&(ip_ptr -> nx_ip_protection));
                return(NX_DUPLICATED_ENTRY);
            }

            /* Walk down the list.  */
            interface_ipv6_address = interface_ipv6_address -> nxd_ipv6_address_next;
        }

        /* Set up the link local address. */
        ipv6_addr -> nxd_ipv6_address[0] = 0xFE800000;
        ipv6_addr -> nxd_ipv6_address[1] = 0x00000000;
        ipv6_addr -> nxd_ipv6_address[2] = word2;
        ipv6_addr -> nxd_ipv6_address[3] = word3;
    }
    else if (ip_address != NX_NULL)
    {
        ipv6_addr -> nxd_ipv6_address[0] = ip_address -> nxd_ip_address.v6[0];
        ipv6_addr -> nxd_ipv6_address[1] = ip_address -> nxd_ip_address.v6[1];
        ipv6_addr -> nxd_ipv6_address[2] = ip_address -> nxd_ip_address.v6[2];
        ipv6_addr -> nxd_ipv6_address[3] = ip_address -> nxd_ip_address.v6[3];
    }
    else
    {
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));
        return(NX_IP_ADDRESS_ERROR);
    }

    ipv6_addr -> nxd_ipv6_address_valid = NX_TRUE;
    ipv6_addr -> nxd_ipv6_address_type = NX_IP_VERSION_V6;
    ipv6_addr -> nxd_ipv6_address_prefix_length = (UCHAR)(prefix_length & 0xFF);
    ipv6_addr -> nxd_ipv6_address_next = NX_NULL;

    /* Attach to the interface.  */
    ipv6_addr -> nxd_ipv6_address_attached = &ip_ptr -> nx_ip_interface[interface_index];

    /* Point to interface link list head  */
    interface_ipv6_address = ip_ptr -> nx_ip_interface[interface_index].nxd_interface_ipv6_address_list_head;

    /* Walk to the end of the list.  */
    while (interface_ipv6_address)
    {

        /* If the next entry does not exist, we already reach the end of the list.
            Add the IPv6 address towards the end. */
        if (interface_ipv6_address -> nxd_ipv6_address_next)
        {
            interface_ipv6_address = interface_ipv6_address -> nxd_ipv6_address_next;
        }
        else
        {
            interface_ipv6_address -> nxd_ipv6_address_next = ipv6_addr;
            break;
        }
    }

    /* Check whether the head is NULL  */
    if (interface_ipv6_address == NX_NULL)
    {
        /* This interface does not have IPv6 addresses yet.  */
        ip_ptr -> nx_ip_interface[interface_index].nxd_interface_ipv6_address_list_head = ipv6_addr;
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NXD_TRACE_IPV6_INTERFACE_ADDRESS_SET, ip_ptr, ipv6_addr -> nxd_ipv6_address[3], prefix_length, index, NX_TRACE_IP_EVENTS, 0, 0);

    /* Set the configuration type to manual. */
    ipv6_addr -> nxd_ipv6_address_ConfigurationMethod = NX_IPV6_ADDRESS_MANUAL_CONFIG;

    /* Release the IP protection.  nx_ipv6_multicast_join would need to obtain the mutex. */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));
    /* Join the solicited-node multicast group */
    /* FF02::1:FFXX:XXXX */
    SET_SOLICITED_NODE_MULTICAST_ADDRESS(multicast_address, ipv6_addr -> nxd_ipv6_address);
    _nx_ipv6_multicast_join(ip_ptr, &multicast_address[0], ipv6_addr -> nxd_ipv6_address_attached);

    /* Obtain the IP protection again. */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    TX_DISABLE
#ifndef NX_DISABLE_IPV6_DAD

    /* If ICMPv6 is enabled, mark the address as tentative, per RFC2462.
       If DAD is not enabled, start the address in VALID state. */
    if (ip_ptr -> nx_ip_icmpv6_packet_process)
    {

        /* Start DAD */
        ipv6_addr -> nxd_ipv6_address_state                 = NX_IPV6_ADDR_STATE_TENTATIVE;
        ipv6_addr -> nxd_ipv6_address_DupAddrDetectTransmit = NX_IPV6_DAD_TRANSMITS;

        /* If trace is enabled, log the start of DAD process. */
        NX_TRACE_IN_LINE_INSERT(NXD_TRACE_IPV6_INITIATE_DAD_PROCESS, ip_ptr, 0, 0, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);
    }
    else
    {

        /* If ICMPv6 is not enabled on this interface, the DAD process is eliminated,
           so mark the input IP address directly as valid. */
        ipv6_addr -> nxd_ipv6_address_state                 = NX_IPV6_ADDR_STATE_VALID;
        ipv6_addr -> nxd_ipv6_address_DupAddrDetectTransmit = 0;
    }
#else
    /* Set the input address as valid. */
    ipv6_addr -> nxd_ipv6_address_state                     = NX_IPV6_ADDR_STATE_VALID;
#endif

    /* Restore interrupts.  */
    TX_RESTORE
#ifdef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
    /* Pickup the current notification callback and additional information pointers.  */
    address_change_notify =  ip_ptr -> nx_ipv6_address_change_notify;
    address_change_notify_internal =  ip_ptr -> nx_ipv6_address_change_notify_internal;
#endif /* NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY */
    /* Release the protection while the IPv6 address is modified. */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

#ifdef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
    /* Is the application configured for notification of address changes and/or
       prefix_length change?  */
    if (address_change_notify)
    {

        /* Yes, call the application's address change notify function.  */
        (address_change_notify)(ip_ptr, NX_IPV6_ADDRESS_MANUAL_CONFIG, interface_index, index, &ipv6_addr -> nxd_ipv6_address[0]);
    }

    /* Is the internal application configured for notification of address changes and/or
       prefix_length change?  */
    if ((address_change_notify_internal) && (ipv6_addr -> nxd_ipv6_address_state == NX_IPV6_ADDR_STATE_VALID))
    {

        /* Yes, call the internal application's address change notify function.  */
        (address_change_notify_internal)(ip_ptr, NX_IPV6_ADDRESS_MANUAL_CONFIG, interface_index, index, &ipv6_addr -> nxd_ipv6_address[0]);
    }
#endif /* NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY */

    /* Return completion status.  */
    return(NX_SUCCESS);

#else /* !FEATURE_NX_IPV6 */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(interface_index);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(prefix_length);
    NX_PARAMETER_NOT_USED(address_index);

    return(NX_NOT_SUPPORTED);

#endif /* FEATURE_NX_IPV6 */
}

