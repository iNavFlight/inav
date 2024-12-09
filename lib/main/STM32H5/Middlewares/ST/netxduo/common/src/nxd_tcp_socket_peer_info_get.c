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
#include "nx_tcp.h"
#include "nx_ipv6.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_tcp_socket_peer_info_get                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves IP address and port number of the peer      */
/*    connected to the specified TCP socket.                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to the TCP sockete    */
/*    peer_ip_address                       Pointer to the IP address     */
/*                                             of the peer.               */
/*    peer_port                             Pointer to the port number    */
/*                                             of the peer.               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Actual completion status      */
/*    NX_NOT_CONNECTED                      TCP socket is not connected   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection             */
/*    tx_mutex_put                          Release protection            */
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
UINT  _nxd_tcp_socket_peer_info_get(NX_TCP_SOCKET *socket_ptr,
                                    NXD_ADDRESS *peer_ip_address,
                                    ULONG *peer_port)
{
#ifdef TX_ENABLE_EVENT_TRACE
ULONG  ip_address_lsw = 0;
#endif /* TX_ENABLE_EVENT_TRACE */
NX_IP *ip_ptr;


    /* Setup IP pointer. */
    ip_ptr = socket_ptr -> nx_tcp_socket_ip_ptr;

    /* Obtain the IP mutex so we can examine the bound port.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Make sure the TCP connection has been established. */
    if ((socket_ptr -> nx_tcp_socket_state <= NX_TCP_LISTEN_STATE) ||
        (socket_ptr -> nx_tcp_socket_state > NX_TCP_ESTABLISHED))
    {

        /* Release protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        return(NX_NOT_CONNECTED);
    }

    /* Determine the peer IP address */

    /* Assign the IP address type (IPv4 or IPv6) */
    peer_ip_address -> nxd_ip_version = socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version;

    /* If address type is IPv4, just copy one word. */
#ifndef NX_DISABLE_IPV4
    if (socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V4)
    {
        peer_ip_address -> nxd_ip_address.v4 = socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v4;
    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    if (socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
    {

        /* Return the IP address of the peer connected to the TCP socket. */
        COPY_IPV6_ADDRESS(socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v6,
                          peer_ip_address -> nxd_ip_address.v6);
    }
#endif /* FEATURE_NX_IPV6 */

    /* Determine the peer port number and return the port number of the peer
       connected to the TCP socket. */
    *peer_port = socket_ptr -> nx_tcp_socket_connect_port;

    /* Release protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

#ifdef TX_ENABLE_EVENT_TRACE

#ifndef NX_DISABLE_IPV4
    if (peer_ip_address -> nxd_ip_version == NX_IP_VERSION_V4)
    {
        ip_address_lsw = peer_ip_address -> nxd_ip_address.v4;
    }
#endif /* NX_DISABLE_IPV4 */

#ifdef FEATURE_NX_IPV6
    if (peer_ip_address -> nxd_ip_version == NX_IP_VERSION_V6)
    {

        ip_address_lsw = peer_ip_address -> nxd_ip_address.v6[3];
    }

#endif /* FEATURE_NX_IPV6 */

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NXD_TRACE_TCP_SOCKET_PEER_INFO_GET, socket_ptr, ip_address_lsw, *peer_port, 0, NX_TRACE_TCP_EVENTS, 0, 0);

#endif /* TX_ENABLE_EVENT_TRACE */

    /* Return successful completion status.  */
    return(NX_SUCCESS);
}

