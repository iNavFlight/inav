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
/**   Address Resolution Protocol (ARP)                                   */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_arp.h"
#include "nx_packet.h"

#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_arp_packet_send                                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function builds an ARP packet and calls the associated driver  */
/*    to send it out on the specified network interface.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    destination_ip                        Destination IP address        */
/*    nx_interface                          Interface to transmit out     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_allocate                   Allocate a packet for the     */
/*                                            ARP request                 */
/*    [ip_link_driver]                      User supplied link driver     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX Source Code                                                    */
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
VOID  _nx_arp_packet_send(NX_IP *ip_ptr, ULONG destination_ip, NX_INTERFACE *nx_interface)
{

NX_PACKET   *request_ptr;
ULONG       *message_ptr;
NX_IP_DRIVER driver_request;

    /* nx_interface must not be NX_NULL. */
    NX_ASSERT(nx_interface != NX_NULL);

    /* Allocate a packet to build the ARP message in.  */
#ifdef NX_ENABLE_DUAL_PACKET_POOL
    /* Allocate from auxiliary packet pool first. */
    if (_nx_packet_allocate(ip_ptr -> nx_ip_auxiliary_packet_pool, &request_ptr, (NX_PHYSICAL_HEADER + NX_ARP_MESSAGE_SIZE), NX_NO_WAIT))
    {
        if (ip_ptr -> nx_ip_auxiliary_packet_pool != ip_ptr -> nx_ip_default_packet_pool)
#endif /* NX_ENABLE_DUAL_PACKET_POOL */
        {
            if (_nx_packet_allocate(ip_ptr -> nx_ip_default_packet_pool, &request_ptr, (NX_PHYSICAL_HEADER + NX_ARP_MESSAGE_SIZE), NX_NO_WAIT))
            {

                /* Error getting packet, so just get out!  */
                return;
            }
        }
#ifdef NX_ENABLE_DUAL_PACKET_POOL
        else
        {

            /* Error getting packet, so just get out!  */
            return;
        }
    }
#endif /* NX_ENABLE_DUAL_PACKET_POOL */

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, request_ptr);

    /* Stamp the packet with the outgoing interface information. */
    /*lint -e{644} suppress variable might not be initialized, since "request_ptr" was initialized in _nx_packet_allocate. */
    request_ptr -> nx_packet_address.nx_packet_interface_ptr = nx_interface;

#ifndef NX_DISABLE_ARP_INFO
    /* Increment the ARP requests sent count.  */
    ip_ptr -> nx_ip_arp_requests_sent++;
#endif

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_ARP_REQUEST_SEND, ip_ptr, destination_ip, request_ptr, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

    /* Build the ARP request packet.  */

    /* Setup the size of the ARP message.  */
    request_ptr -> nx_packet_length =  NX_ARP_MESSAGE_SIZE;

    /* Setup the prepend pointer.  */
    request_ptr -> nx_packet_prepend_ptr -= NX_ARP_MESSAGE_SIZE;

    /* Setup the pointer to the message area.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    message_ptr =  (ULONG *)request_ptr -> nx_packet_prepend_ptr;

    /* Write the Hardware type into the message.  */
    *message_ptr =      (ULONG)(NX_ARP_HARDWARE_TYPE << 16) | (NX_ARP_PROTOCOL_TYPE);
    *(message_ptr + 1) =  (ULONG)(NX_ARP_HARDWARE_SIZE << 24) | (NX_ARP_PROTOCOL_SIZE << 16) |
        NX_ARP_OPTION_REQUEST;

    /*lint -e{613} suppress possible use of null pointer, since nx_interface must not be NULL.  */
    *(message_ptr + 2) =  (ULONG)(nx_interface -> nx_interface_physical_address_msw << 16) |
        (nx_interface -> nx_interface_physical_address_lsw >> 16);
    *(message_ptr + 3) =  (ULONG)(nx_interface -> nx_interface_physical_address_lsw << 16) |
        (nx_interface -> nx_interface_ip_address >> 16);
    *(message_ptr + 4) =  (ULONG)(nx_interface -> nx_interface_ip_address << 16);
    *(message_ptr + 5) =  (ULONG)0;
    *(message_ptr + 6) =  (ULONG)destination_ip;

    /* Endian swapping logic.  If NX_LITTLE_ENDIAN is specified, these macros will
       swap the endian of the ARP message.  */
    NX_CHANGE_ULONG_ENDIAN(*(message_ptr));
    NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 1));
    NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 2));
    NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 3));
    NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 4));
    NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 5));
    NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 6));

    /* Set up the driver request. */
    driver_request.nx_ip_driver_ptr =                   ip_ptr;
    driver_request.nx_ip_driver_command =               NX_LINK_ARP_SEND;
    driver_request.nx_ip_driver_packet =                request_ptr;
    driver_request.nx_ip_driver_physical_address_msw =  0xFFFFUL;
    driver_request.nx_ip_driver_physical_address_lsw =  0xFFFFFFFFUL;
    driver_request.nx_ip_driver_interface            =  nx_interface;

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_IO_DRIVER_ARP_SEND, ip_ptr, request_ptr, request_ptr -> nx_packet_length, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, request_ptr);

    /* Send the ARP request to the driver.  */
    /*lint -e{613} suppress possible use of null pointer, since nx_interface must not be NULL.  */
    (nx_interface -> nx_interface_link_driver_entry)(&driver_request);
}
#endif /* !NX_DISABLE_IPV4  */

