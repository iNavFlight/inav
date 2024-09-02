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
#include "nx_ip.h"
#include "nx_udp.h"
#include "nx_packet.h"
#ifdef FEATURE_NX_IPV6
#include "nx_ipv6.h"
#endif /* FEATURE_NX_IPV6 */

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_udp_socket_send                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the UDP socket send              */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to UDP socket         */
/*    packet_ptr                            Pointer to UDP packet         */
/*    ip_address                            IP address                    */
/*    port                                  16-bit UDP port number        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Actual completion status      */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_NOT_ENABLED                        UDP not enabled               */
/*    NX_IP_ADDRESS_ERROR                   NULL address input            */
/*    NX_INVALID_PORT                       Invalid UDP socket port       */
/*    NX_UNDERFLOW                          Invalid pointer to packet data*/
/*    NX_OVERFLOW                           Invalid pointer to packet data*/
/*    NX_CALLER_ERROR                       Invalid (non thread) caller   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_udp_socket_send                   Actual UDP socket send        */
/*                                            function                    */
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
UINT  _nxe_udp_socket_send(NX_UDP_SOCKET *socket_ptr, NX_PACKET **packet_ptr_ptr,
                           ULONG ip_address, UINT port)
{

#ifndef NX_DISABLE_IPV4
NX_PACKET *packet_ptr;
UINT       status;


    /* Setup packet pointer.  */
    packet_ptr =  *packet_ptr_ptr;

    /* Check for invalid input pointers.  */
    /*lint -e{923} suppress cast of ULONG to pointer.  */
    if ((socket_ptr == NX_NULL) || (socket_ptr -> nx_udp_socket_id != NX_UDP_ID) ||
        (packet_ptr == NX_NULL) || (packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next != ((NX_PACKET *)NX_PACKET_ALLOCATED)))
    {

        return(NX_PTR_ERROR);
    }

    /* Check to see if UDP is enabled.  */
    if (!(socket_ptr -> nx_udp_socket_ip_ptr) -> nx_ip_udp_packet_receive)
    {
        return(NX_NOT_ENABLED);
    }

    /* Check for invalid IP address.  */
    if (ip_address == NX_NULL)
    {
        return(NX_IP_ADDRESS_ERROR);
    }

    /* Check for an invalid port.  */
    if (((ULONG)port) > (ULONG)NX_MAX_PORT)
    {
        return(NX_INVALID_PORT);
    }

    /* Check for an invalid packet prepend pointer.  */
    /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
    if ((INT)(packet_ptr -> nx_packet_prepend_ptr - packet_ptr -> nx_packet_data_start) < (INT)(sizeof(NX_IPV4_HEADER) + sizeof(NX_UDP_HEADER)))
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

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual UDP socket send function.  */
    status =  _nx_udp_socket_send(socket_ptr, packet_ptr, ip_address, port);

    /* Determine if the packet send was successful.  */
    if (status == NX_SUCCESS)
    {

        /* Yes, now clear the application's packet pointer so it can't be accidentally
           used again by the application.  This is only done when error checking is
           enabled.  */
        *packet_ptr_ptr =  NX_NULL;
    }

    /* Return completion status.  */
    return(status);
#else
    NX_PARAMETER_NOT_USED(socket_ptr);
    NX_PARAMETER_NOT_USED(packet_ptr_ptr);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(port);

    return(NX_NOT_SUPPORTED);
#endif /* !NX_DISABLE_IPV4  */
}

