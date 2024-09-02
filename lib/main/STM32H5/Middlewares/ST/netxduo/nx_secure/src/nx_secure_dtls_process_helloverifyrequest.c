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

#if !defined(NX_SECURE_TLS_CLIENT_DISABLED) && defined(NX_SECURE_ENABLE_DTLS)
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_dtls_process_helloverifyrequest          PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an incoming HelloVerifyRequest message.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          DTLS control block            */
/*    packet_buffer                         Pointer to message data       */
/*    message_length                        Length of message data (bytes)*/
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
/*    _nx_secure_dtls_client_handshake      DTLS client state machine     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), improved */
/*                                            buffer length verification, */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Timothy Stapko           Modified comment(s),          */
/*                                            updated cookie handling,    */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_dtls_process_helloverifyrequest(NX_SECURE_DTLS_SESSION *dtls_session,
                                                UCHAR *packet_buffer, UINT message_length)
{
UINT length;


    /* Parse the HelloVerifyRequest message.
     * Structure:
     * |      2       |       1       |  <Cookie Length>   |
     * | DTLS version | Cookie length | Server Cookie data |
     */

    /* Use our length as an index into the buffer. */
    length = 0;

    /* First two bytes of the server hello following the header are the TLS major and minor version numbers. */
    length += 2;

    /* Get the cookie length. */
    dtls_session -> nx_secure_dtls_cookie_length = packet_buffer[length];
    length += 1;

    if (dtls_session -> nx_secure_dtls_cookie_length > NX_SECURE_DTLS_MAX_COOKIE_LENGTH)
    {
        dtls_session -> nx_secure_dtls_cookie_length = 0;
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    if ((3u + dtls_session -> nx_secure_dtls_cookie_length) > message_length)
    {
        dtls_session -> nx_secure_dtls_cookie_length = 0;
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* Save off the cookie pointer. */
    dtls_session -> nx_secure_dtls_client_cookie_ptr = &packet_buffer[length];

    /* Set our state to indicate we sucessfully parsed the HelloVerifyRequest. */
    dtls_session -> nx_secure_dtls_tls_session.nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_HELLO_VERIFY;

    return(NX_SECURE_TLS_SUCCESS);
}
#endif /* !defined(NX_SECURE_TLS_CLIENT_DISABLED) && defined(NX_SECURE_ENABLE_DTLS) */

