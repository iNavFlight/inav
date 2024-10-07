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
#include "nx_ip.h"
#include "nx_ipv6.h"

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_udp_socket_source_send                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs error checking on the UDP socket send        */
/*    via source address service.                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to the UDP socket     */
/*    packet_ptr                            Pointer to packet to send     */
/*    ip_address                            Pointer to destination address*/
/*    port                                  Destination port number       */
/*    address_index                         Index of IPv4 or IPv6 address */
/*                                            to use as the source address*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Actual completion status      */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_NOT_ENABLED                        UDP not enabled on IP instance*/
/*    NX_IP_ADDRESS_ERROR                   Invalid input address         */
/*    NX_INVALID_INTERFACE                  Invalid interface input       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*   _nxde_udp_socket_source_send           Actual UDP socket socket      */
/*                                             send function              */
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

UINT  _nxde_udp_socket_source_send(NX_UDP_SOCKET *socket_ptr, NX_PACKET *packet_ptr,
                                   NXD_ADDRESS *ip_address, UINT port, UINT address_index)
{

UINT status;
UINT ip_header_size;


    /* Check for invalid input pointers.  */
    if ((socket_ptr == NX_NULL) || (packet_ptr == NX_NULL) || (ip_address == NX_NULL) || (socket_ptr -> nx_udp_socket_id != NX_UDP_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Verify UDP is enabled.  */
    if (!(socket_ptr -> nx_udp_socket_ip_ptr) -> nx_ip_udp_packet_receive)
    {
        return(NX_NOT_ENABLED);
    }

    /* Check that the destination address version is either IPv4 or IPv6. */
#ifndef NX_DISABLE_IPV4
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V4)
    {
        if (address_index >= NX_MAX_IP_INTERFACES)
        {
            return(NX_INVALID_INTERFACE);
        }

        if (ip_address -> nxd_ip_address.v4 == 0)
        {
            return(NX_IP_ADDRESS_ERROR);
        }

        ip_header_size = (UINT)sizeof(NX_IPV4_HEADER);
    }
    else
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V6)
    {
        if (address_index >= (NX_MAX_IPV6_ADDRESSES + NX_LOOPBACK_IPV6_ENABLED))
        {
            return(NX_INVALID_INTERFACE);
        }

        if (CHECK_UNSPECIFIED_ADDRESS(&ip_address -> nxd_ip_address.v6[0]))
        {
            return(NX_IP_ADDRESS_ERROR);
        }

        ip_header_size = (UINT)sizeof(NX_IPV6_HEADER);
    }
    else
#endif /* FEATURE_NX_IPV6 */
    {
        return(NX_IP_ADDRESS_ERROR);
    }

    /* Check for an invalid packet prepend pointer.  */
    /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
    if ((INT)(packet_ptr -> nx_packet_prepend_ptr - packet_ptr -> nx_packet_data_start) < (INT)(ip_header_size + sizeof(NX_UDP_HEADER)))
    {

#ifndef NX_DISABLE_UDP_INFO
        /* Increment the total UDP invalid packet count.  */
        (socket_ptr -> nx_udp_socket_ip_ptr) -> nx_ip_udp_invalid_packets++;

        /* Increment the total UDP invalid packet count for this socket.  */
        socket_ptr -> nx_udp_socket_invalid_packets++;
#endif

        /* Return error code.  */
        return(NX_UNDERFLOW);
    }

    /* Check for an invalid packet append pointer.  */
    /*lint -e{946} suppress pointer subtraction, since it is necessary. */
    if (packet_ptr -> nx_packet_append_ptr > packet_ptr -> nx_packet_data_end)
    {

#ifndef NX_DISABLE_UDP_INFO
        /* Increment the total UDP invalid packet count.  */
        (socket_ptr -> nx_udp_socket_ip_ptr) -> nx_ip_udp_invalid_packets++;

        /* Increment the total UDP invalid packet count for this socket.  */
        socket_ptr -> nx_udp_socket_invalid_packets++;
#endif

        /* Return error code.  */
        return(NX_OVERFLOW);
    }

    /* Check for an invalid port.  */
    if (((ULONG)port) > (ULONG)NX_MAX_PORT)
    {
        return(NX_INVALID_PORT);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual UDP socket send function.  */
    status =  _nxd_udp_socket_source_send(socket_ptr, packet_ptr, ip_address, port, address_index);

    /* Return completion status.  */
    return(status);
}

