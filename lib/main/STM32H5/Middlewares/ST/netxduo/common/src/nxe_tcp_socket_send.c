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
/**   Transmission Control Protocol (TCP)                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ip.h"
#include "nx_tcp.h"
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
/*    _nxe_tcp_socket_send                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the TCP socket send              */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to socket             */
/*    packet_ptr                            Pointer to packet to send     */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_socket_send                   Actual TCP socket send        */
/*                                            function                    */
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
UINT  _nxe_tcp_socket_send(NX_TCP_SOCKET *socket_ptr, NX_PACKET **packet_ptr_ptr, ULONG wait_option)
{

NX_PACKET *packet_ptr;
UINT       status;
UINT       ip_header_size;

    /* Setup packet pointer.  */
    packet_ptr =  *packet_ptr_ptr;

    /* Check for invalid input pointers.  */
    if ((socket_ptr == NX_NULL) || (socket_ptr -> nx_tcp_socket_id != NX_TCP_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for an invalid packet pointer.  */
    /*lint -e{923} suppress cast of ULONG to pointer.  */
    if ((packet_ptr == NX_NULL) || (packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next != ((NX_PACKET *)NX_PACKET_ALLOCATED)))
    {
        return(NX_INVALID_PACKET);
    }

    /* Check to see if TCP is enabled.  */
    if (!(socket_ptr -> nx_tcp_socket_ip_ptr) -> nx_ip_tcp_packet_receive)
    {
        return(NX_NOT_ENABLED);
    }

    /* Check for an invalid packet prepend pointer.  */
#ifndef NX_DISABLE_IPV4
    if (socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V4)
    {
        ip_header_size = (UINT)sizeof(NX_IPV4_HEADER);
    }
    else
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    if (socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
    {
        ip_header_size = (UINT)sizeof(NX_IPV6_HEADER);
    }
    else
#endif /* FEATURE_NX_IPV6 */
    {
        return(NX_NOT_CONNECTED);
    }

    /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
    if ((INT)(packet_ptr -> nx_packet_prepend_ptr - packet_ptr -> nx_packet_data_start) < (INT)(ip_header_size + sizeof(NX_TCP_HEADER)))
    {

#ifndef NX_DISABLE_TCP_INFO

        /* Increment the TCP invalid packet count.  */
        (socket_ptr -> nx_tcp_socket_ip_ptr) -> nx_ip_tcp_invalid_packets++;
#endif

        /* Return error code.  */
        return(NX_UNDERFLOW);
    }

    /* Check for an invalid packet append pointer.  */
    /*lint -e{946} suppress pointer subtraction, since it is necessary. */
    if (packet_ptr -> nx_packet_append_ptr > packet_ptr -> nx_packet_data_end)
    {

#ifndef NX_DISABLE_TCP_INFO

        /* Increment the TCP invalid packet count.  */
        (socket_ptr -> nx_tcp_socket_ip_ptr) -> nx_ip_tcp_invalid_packets++;
#endif

        /* Return error code.  */
        return(NX_OVERFLOW);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual TCP socket send function.  */
    status =  _nx_tcp_socket_send(socket_ptr, packet_ptr, wait_option);

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
}

