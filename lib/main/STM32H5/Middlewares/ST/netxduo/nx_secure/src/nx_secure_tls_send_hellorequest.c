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
/**    Transport Layer Security (TLS)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_tls.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_send_hellorequest                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates a HelloRequest message, which is used by a  */
/*    TLS server to indicate to the remote TLS client host that it wishes */
/*    to perform a re-negotiation handshake. The client should respond    */
/*    with a ClientHello as long as the active TLS session is valid.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    send_packet                           Packet used to send message   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_session_renegotiate    Renegotiate TLS session       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            fixed renegotiation bug,    */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
#ifndef NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION
UINT _nx_secure_tls_send_hellorequest(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *send_packet)
{
    NX_PARAMETER_NOT_USED(tls_session);
    NX_PARAMETER_NOT_USED(send_packet);

#ifndef NX_SECURE_TLS_SERVER_DISABLED
    /* Indicate that we have initiated a renegotiation by sending a HelloRequest to the remote client. */
    tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_HELLO_REQUEST;
#endif

    return(NX_SECURE_TLS_SUCCESS);
}
#endif /* NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION */
