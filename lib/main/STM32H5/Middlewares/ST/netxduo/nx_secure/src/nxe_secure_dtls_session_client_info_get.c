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
/*    _nxe_secure_dtls_session_client_info_get            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors when getting DTLS client info.      */
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
/*    _nx_secure_dtls_session_client_info_get                             */
/*                                          Actual function call          */
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
UINT _nxe_secure_dtls_session_client_info_get(NX_SECURE_DTLS_SESSION *dtls_session,
                                              NXD_ADDRESS *client_ip_address, UINT *client_port, UINT *local_port)
{
#ifdef NX_SECURE_ENABLE_DTLS
NX_UDP_SOCKET *socket_ptr;
UINT status;

    /* Check pointers. */
    if (dtls_session == NX_NULL || client_ip_address == NX_NULL ||
        client_port == NX_NULL || local_port == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Make sure the session is initialized. */
    if (dtls_session->nx_secure_dtls_tls_session.nx_secure_tls_id != NX_SECURE_TLS_ID)
    {
        return(NX_SECURE_TLS_SESSION_UNINITIALIZED);
    }

    /* Get our UDP socket. */
    socket_ptr = dtls_session->nx_secure_dtls_udp_socket;

    /* Check UDP socket. */
    if (socket_ptr == NX_NULL)
    {
        return(NX_INVALID_SOCKET);
    }

    /* Call actual function. */
    status = _nx_secure_dtls_session_client_info_get(dtls_session, client_ip_address, client_port, local_port);

    return(status);
#else
    NX_PARAMETER_NOT_USED(dtls_session);
    NX_PARAMETER_NOT_USED(client_ip_address);
    NX_PARAMETER_NOT_USED(client_port);
    NX_PARAMETER_NOT_USED(local_port);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}

