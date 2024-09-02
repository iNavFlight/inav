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
/** NetX Secure Component                                                 */
/**                                                                       */
/**    Datagram Transport Layer Security (DTLS)                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_dtls.h"
#include "nx_ipv6.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_dtls_session_client_info_get             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns relevant information about a remote client    */
/*    that is connected to a DTLS server session instance.                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          DTLS session control block    */
/*    client_ip_address                     Return remote host IP address */
/*    client_port                           Return remote host UDP port   */
/*    local_port                            Return local UDP port         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    COPY_IPV6_ADDRESS                     Copy IPv6 address to return   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_dtls_session_client_info_get(NX_SECURE_DTLS_SESSION *dtls_session,
                                             NXD_ADDRESS *client_ip_address, UINT *client_port, UINT *local_port)
{
#ifdef NX_SECURE_ENABLE_DTLS
NX_UDP_SOCKET *socket_ptr;
NXD_ADDRESS *ip_address;

    /* Get our UDP socket. */
    socket_ptr = dtls_session->nx_secure_dtls_udp_socket;

    /* Make sure we have received a packet on the UDP socket - otherwise
       we don't have a DTLS connection and can't get info from a non-existent Client! */
    if(socket_ptr->nx_udp_socket_receive_head == NX_NULL)
    {
        return(NX_NOT_CONNECTED);
    }

    /* Get our local IP address for the deep copy below. */
    ip_address = &(dtls_session->nx_secure_dtls_remote_ip_address);

    /* Copy address version. */
    client_ip_address -> nxd_ip_version = ip_address -> nxd_ip_version;

#ifndef NX_DISABLE_IPV4
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V4)
    {
        /* Pickup the source IP address.  */
        client_ip_address -> nxd_ip_address.v4 = ip_address -> nxd_ip_address.v4;
    }
#endif /* NX_DISABLE_IPV4 */

#ifdef FEATURE_NX_IPV6
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V6)
    {
        COPY_IPV6_ADDRESS(ip_address -> nxd_ip_address.v6,
                          client_ip_address -> nxd_ip_address.v6);
    }
#endif /* FEATURE_NX_IPV6 */

    /* Copy the remote and local ports to our return parameters. */
    *client_port = dtls_session->nx_secure_dtls_remote_port;
    *local_port = dtls_session->nx_secure_dtls_local_port;

    return(NX_SUCCESS);
#else
    NX_PARAMETER_NOT_USED(dtls_session);
    NX_PARAMETER_NOT_USED(client_ip_address);
    NX_PARAMETER_NOT_USED(client_port);
    NX_PARAMETER_NOT_USED(local_port);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}

