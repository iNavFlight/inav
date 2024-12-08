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

#ifdef NX_SECURE_ENABLE_DTLS
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_dtls_send_helloverifyrequest             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function populates an NX_PACKET with a HelloVerifyRequest      */
/*    message.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          DTLS control block            */
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
/*    _nx_secure_dtls_server_handshake      DTLS server state machine     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_dtls_send_helloverifyrequest(NX_SECURE_DTLS_SESSION *dtls_session,
                                             NX_PACKET *send_packet)
{
UINT   length;
UINT   i;
UINT   random_value;
UCHAR *packet_buffer;
USHORT protocol_version;

    /* Parse the HelloVerifyRequest message.
     * Structure:
     * |      2       |       1       |  <Cookie Length>   |
     * | DTLS version | Cookie length | Server Cookie data |
     */

    if (((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr)) <
        (3u + dtls_session -> nx_secure_dtls_cookie_length))
    {

        /* Packet buffer is too small to hold random and ID. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Use our length as an index into the buffer. */
    length = 0;

    packet_buffer = send_packet -> nx_packet_append_ptr;

    /* First two bytes of the HelloVerifyRequest following the header are the (D)TLS major and minor version numbers.
       The version should be DTLS version 1.0 regardless of the version of TLS that is expected to be negotiated.*/
    protocol_version = NX_SECURE_DTLS_VERSION_1_0;

    packet_buffer[length]     = (UCHAR)((protocol_version & 0xFF00) >> 8);
    packet_buffer[length + 1] = (UCHAR)(protocol_version & 0x00FF);
    length += 2;

    /* Set the cookie length. */
    dtls_session -> nx_secure_dtls_cookie_length = 20;
    packet_buffer[length] = (UCHAR)(dtls_session -> nx_secure_dtls_cookie_length);
    length += 1;

    /* Generate the cookie. */
    for (i = 0; i < dtls_session -> nx_secure_dtls_cookie_length; i += (UINT)sizeof(random_value))
    {
        random_value = (UINT)NX_RAND();
        NX_CHANGE_ULONG_ENDIAN(random_value);
        NX_SECURE_MEMCPY(&dtls_session -> nx_secure_dtls_cookie[i],
               (UCHAR *)&random_value, sizeof(random_value)); /* Use case of memcpy is verified. */
    }

    if (dtls_session -> nx_secure_dtls_cookie_length > sizeof(dtls_session -> nx_secure_dtls_cookie))
    {

        /* Packet buffer is too small to hold cookie. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Copy the cookie into the packet. */
    NX_SECURE_MEMCPY(&packet_buffer[length], dtls_session -> nx_secure_dtls_cookie, dtls_session -> nx_secure_dtls_cookie_length); /* Use case of memcpy is verified. */
    length += dtls_session -> nx_secure_dtls_cookie_length;

    /* Save off and return the number of bytes we wrote and need to send. */
    send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_append_ptr + length;
    send_packet -> nx_packet_length = send_packet -> nx_packet_length + length;

    return(NX_SECURE_TLS_SUCCESS);
}
#endif /* NX_SECURE_ENABLE_DTLS */

