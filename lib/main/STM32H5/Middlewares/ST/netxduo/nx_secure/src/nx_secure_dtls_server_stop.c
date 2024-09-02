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

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_dtls_server_stop                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function stops a DTLS server from processing incoming UDP      */
/*    datagrams and resets the state of all associated DTLS sessions. To  */
/*    free up resources, see nx_secure_dtls_server_delete.                */
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
/*    nx_udp_socket_receive_notify          Reset receive notify function */
/*    _nx_secure_dtls_session_reset         Reset internal DTLS sessions  */
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
UINT _nx_secure_dtls_server_stop(NX_SECURE_DTLS_SERVER *server_ptr)
{
#ifdef NX_SECURE_ENABLE_DTLS
UINT i;
UINT status;

    /* Setup the UDP socket with our internal receive callback. */
    nx_udp_socket_receive_notify(&(server_ptr->nx_dtls_server_udp_socket), NX_NULL);

    /* Reset all DTLS sessions. This clears any active sessions and allows the
       server to be re-used. */
    for(i = 0; i < server_ptr->nx_dtls_server_sessions_count; ++i)
    {
        _nx_secure_dtls_session_reset(&(server_ptr->nx_dtls_server_sessions[i]));
    }

    /* Unbind the UDP socket.  */
    status = nx_udp_socket_unbind(&(server_ptr -> nx_dtls_server_udp_socket));
    if (status)
    {
        return(status);
    }

    return(NX_SUCCESS);
#else
    NX_PARAMETER_NOT_USED(server_ptr);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}

