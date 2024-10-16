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
/*    _nx_secure_dtls_client_session_start                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function starts a DTLS session for a DTLS client given a UDP   */
/*    socket. The client session must have been previously initialized.   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          DTLS control block            */
/*    udp_socket                            UDP socket pointer            */
/*    wait_option                           Suspension option             */
/*    ip_address                            Address of remote server      */
/*    port                                  Port on remote server         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_dtls_session_start         Start the DTLS session        */
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
UINT _nx_secure_dtls_client_session_start(NX_SECURE_DTLS_SESSION *dtls_session, NX_UDP_SOCKET *udp_socket, NXD_ADDRESS *ip_address, UINT port, UINT wait_option)
{
#ifdef NX_SECURE_ENABLE_DTLS
UINT status;
NXD_ADDRESS *remote_ip;

    /* Get our local IP address for the deep copy below. */
    remote_ip = &(dtls_session->nx_secure_dtls_remote_ip_address);

    /* Copy address version. */
    remote_ip -> nxd_ip_version = ip_address -> nxd_ip_version;

    #ifndef NX_DISABLE_IPV4
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V4)
    {
        /* Pickup the source IP address.  */
        remote_ip -> nxd_ip_address.v4 = ip_address -> nxd_ip_address.v4;

    }
    #endif /* NX_DISABLE_IPV4 */

    #ifdef FEATURE_NX_IPV6
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V6)
    {
        COPY_IPV6_ADDRESS(ip_address -> nxd_ip_address.v6,
                          remote_ip -> nxd_ip_address.v6);

    }
    #endif /* FEATURE_NX_IPV6 */

    /* Assign port to session. */
    dtls_session->nx_secure_dtls_remote_port = port;

    /* Session is in use. */
    dtls_session -> nx_secure_dtls_session_in_use = NX_TRUE;

    /* Call old API for now. */
    status = _nx_secure_dtls_session_start(dtls_session, udp_socket, NX_TRUE, wait_option);

    return(status);
#else
    NX_PARAMETER_NOT_USED(dtls_session);
    NX_PARAMETER_NOT_USED(udp_socket);
    NX_PARAMETER_NOT_USED(wait_option);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(port);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}
