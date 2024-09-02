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
/**   User Datagram Protocol (UDP)                                        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_udp.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_udp_socket_source_send                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends UDP packet using specified local interface      */
/*    IPv4 or IPv6 address as source.                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to UDP socket         */
/*    packet_ptr                            Pointer to UDP packet         */
/*    ip_address                            IP address                    */
/*    port                                  16-bit UDP port number        */
/*    address_index                         Index of IPv4 or IPv6 address */
/*                                            to use as the source address*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_udp_socket_send                  Send the UDP packet           */
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
UINT  _nxd_udp_socket_source_send(NX_UDP_SOCKET *socket_ptr, NX_PACKET *packet_ptr,
                                  NXD_ADDRESS *ip_address, UINT port, UINT address_index)
{
UINT   status;
NX_IP *ip_ptr;

    /* Setup the pointer to the associated IP instance.  */
    ip_ptr =  socket_ptr -> nx_udp_socket_ip_ptr;

    /* Store address information into the packet structure. */
#ifndef NX_DISABLE_IPV4
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V4)
    {
        packet_ptr -> nx_packet_address.nx_packet_interface_ptr = &(ip_ptr -> nx_ip_interface[address_index]);
    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V6)
    {
        packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr = &ip_ptr -> nx_ipv6_address[address_index];

        if (packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_state != NX_IPV6_ADDR_STATE_VALID)
        {
            return(NX_NO_INTERFACE_ADDRESS);
        }
    }
#endif /* FEATURE_NX_IPV6 */

    /* Call udp_socket_send service */
    status = _nxd_udp_socket_send(socket_ptr, packet_ptr, ip_address, port);

    return(status);
}

