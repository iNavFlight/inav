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

VOID _nx_secure_dtls_receive_callback(NX_UDP_SOCKET *socket_ptr);

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_dtls_server_start                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function starts a DTLS server on the internal UDP socket.      */
/*    It is assumed that the DTLS server instance was created previously  */
/*    and the UDP socket is ready to listen on the port specified in the  */
/*    create call.                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            DTLS server control block     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_udp_socket_bind                    Bind UDP socket to port       */
/*    nx_udp_socket_receive_notify          Set receive callback          */
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
UINT _nx_secure_dtls_server_start(NX_SECURE_DTLS_SERVER *server_ptr)
{
#ifdef NX_SECURE_ENABLE_DTLS
UINT status;

    /* Bind the UDP socket to the assigned port. */
    status =  nx_udp_socket_bind(&(server_ptr->nx_dtls_server_udp_socket), server_ptr->nx_dtls_server_listen_port,
                                 server_ptr->nx_dtls_server_timeout);

    if(status != NX_SUCCESS)
    {
        return(status);
    }
    
    /* Setup the UDP socket with our internal receive callback. */
    nx_udp_socket_receive_notify(&(server_ptr->nx_dtls_server_udp_socket), _nx_secure_dtls_receive_callback);

    return(NX_SUCCESS);
#else
    NX_PARAMETER_NOT_USED(server_ptr);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}

