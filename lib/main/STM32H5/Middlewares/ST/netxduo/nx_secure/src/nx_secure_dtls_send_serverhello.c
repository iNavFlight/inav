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
/*    _nx_secure_dtls_send_serverhello                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates a ServerHello message, which is used to     */
/*    respond to an incoming ClientHello message and provide the remote   */
/*    TLS Client with the chosen ciphersuite and key generation data.     */
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
/*    [nx_crypto_operation]                 Crypto operation              */
/*    [nx_secure_tls_session_time_function] Get the current time for the  */
/*                                            DTLS timestamp              */
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
UINT _nx_secure_dtls_send_serverhello(NX_SECURE_DTLS_SESSION *dtls_session, NX_PACKET *send_packet)
{
UINT   length;
UINT   gmt_time;
UINT   random_value;
UINT   i;
USHORT ciphersuite;

#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
UINT                                  status;
UCHAR                                *extension_total_length_ptr;
USHORT                                extension_total_length;
USHORT                                extension_length;
USHORT                                extension_type;
NX_CRYPTO_METHOD                     *crypto_method;
NX_CRYPTO_EXTENDED_OUTPUT             extended_output;
#endif
UCHAR                                *packet_buffer;
NX_SECURE_TLS_SESSION                *tls_session;
USHORT                                protocol_version;


    /* Build up the ServerHello message.
     * Structure:
     * |     2       |          4 + 28          |    1       |   <SID len>  |      2      |      1      |    2     | <Ext. Len> |
     * | TLS version |  Random (time + random)  | SID length |  Session ID  | Ciphersuite | Compression | Ext. Len | Extensions |
     */

    tls_session = &dtls_session -> nx_secure_dtls_tls_session;

    if ((6u + sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random) +
         tls_session -> nx_secure_tls_session_id_length) >
        ((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_prepend_ptr)))
    {

        /* Packet buffer is too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    packet_buffer = send_packet -> nx_packet_prepend_ptr;

    /* Use our length as an index into the buffer. */
    length = 0;

    /* First two bytes of the hello following the header are the TLS major and minor version numbers. */
    /* DTLS version. */
    protocol_version = tls_session -> nx_secure_tls_protocol_version;

    packet_buffer[length]     = (UCHAR)((protocol_version & 0xFF00) >> 8);
    packet_buffer[length + 1] = (UCHAR)(protocol_version & 0x00FF);
    length += 2;

    /* Set the Server random data, used in key generation. First 4 bytes is GMT time. */
    gmt_time = 0;
    if (tls_session -> nx_secure_tls_session_time_function != NX_NULL)
    {
        gmt_time = tls_session -> nx_secure_tls_session_time_function();
    }
    NX_CHANGE_ULONG_ENDIAN(gmt_time);

    NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random, (UCHAR *)&gmt_time, sizeof(gmt_time)); /* Use case of memcpy is verified. */

    /* Next 28 bytes is random data. */
    for (i = 0; i < 28; i += (UINT)sizeof(random_value))
    {
        random_value = (UINT)NX_RAND();
        *(tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random + (i + 4))     = (UCHAR)(random_value);
        *(tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random + (i + 5)) = (UCHAR)(random_value >> 8);
        *(tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random + (i + 6)) = (UCHAR)(random_value >> 16);
        *(tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random + (i + 7)) = (UCHAR)(random_value >> 24);
    }

    /* Copy the random data into the packet. */
    NX_SECURE_MEMCPY(&packet_buffer[length], tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random,
                     sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random)); /* Use case of memcpy is verified. */
    length += sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random);

    /* Session ID length is one byte. */
    packet_buffer[length] = tls_session -> nx_secure_tls_session_id_length;
    length++;

    /* Session ID follows. */
    if (tls_session -> nx_secure_tls_session_id_length > 0)
    {
        NX_SECURE_MEMCPY(&packet_buffer[length], tls_session -> nx_secure_tls_session_id, tls_session -> nx_secure_tls_session_id_length); /* Use case of memcpy is verified. */
        length += tls_session -> nx_secure_tls_session_id_length;
    }

    /* Finally, our chosen ciphersuite - this is selected when we receive the Client Hello. */
    ciphersuite = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_ciphersuite;
    packet_buffer[length]     = (UCHAR)(ciphersuite >> 8);
    packet_buffer[length + 1] = (UCHAR)ciphersuite;

    length += 2;

    /* Compression method - for now this is NULL. */
    packet_buffer[length] = 0x0;
    length++;

    /* TLS extensions will go here once supported. */

#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
    if (tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECJPAKE)
    {
        if (((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_prepend_ptr)) < 12u)
        {

            /* Packet buffer is too small. */
            return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
        }

        crypto_method = (NX_CRYPTO_METHOD*)tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth;

        extension_total_length_ptr = &packet_buffer[length];
        extension_total_length = 10;
        length += 2;

        /* ec_point_formats Extension.  */
        extension_type = NX_SECURE_TLS_EXTENSION_EC_POINT_FORMATS;
        packet_buffer[length] = (UCHAR)((extension_type >> 8) & 0xFF);
        packet_buffer[length + 1] = (UCHAR)(extension_type & 0xFF);
        length += 2;

        /* ec_point_formats Length: 2.  */
        extension_length = 2;
        packet_buffer[length] = (UCHAR)((extension_length >> 8) & 0xFF);
        packet_buffer[length + 1] = (UCHAR)(extension_length & 0xFF);
        length += 2;

        /* EC point formats Length: 1.  */
        packet_buffer[length] = 1;
        length += 1;

        /* EC point format: uncompressed(0).  */
        packet_buffer[length] = 0;
        length += 1;

        /* ecjpake_key_kp_pair Extension.  */
        extension_type = NX_SECURE_TLS_EXTENSION_ECJPAKE_KEY_KP_PAIR;
        packet_buffer[length] = (UCHAR)((extension_type >> 8) & 0xFF);
        packet_buffer[length + 1] = (UCHAR)(extension_type & 0xFF);
        length += 2;

        /* ecjpake_key_kp_pair Length.  */
        length += 2;

        extended_output.nx_crypto_extended_output_data = &packet_buffer[length];
        extended_output.nx_crypto_extended_output_length_in_byte =
            (ULONG)send_packet -> nx_packet_data_end - (ULONG)&packet_buffer[length];
        extended_output.nx_crypto_extended_output_actual_size = 0;
        status = crypto_method -> nx_crypto_operation(NX_CRYPTO_ECJPAKE_SERVER_HELLO_GENERATE,
                                                      NX_NULL,
                                                      crypto_method,
                                                      NX_NULL, 0,
                                                      NX_NULL, 0, NX_NULL,
                                                      (UCHAR *)&extended_output,
                                                      sizeof(extended_output),
                                                      tls_session -> nx_secure_public_auth_metadata_area,
                                                      tls_session -> nx_secure_public_auth_metadata_size,
                                                      NX_NULL, NX_NULL);
        if (status)
        {
            return(status);
        }

        /* Set length for ecjpake_key_kp_pair extension. */
        extension_length = (USHORT)extended_output.nx_crypto_extended_output_actual_size;
        packet_buffer[length - 2] = (UCHAR)((extension_length >> 8) & 0xFF);
        packet_buffer[length - 1] = (UCHAR)(extension_length & 0xFF);

        extension_total_length = (USHORT)(extension_total_length +
                                          extended_output.nx_crypto_extended_output_actual_size);
        length += extended_output.nx_crypto_extended_output_actual_size;

        /* Set extension total length. */
        extension_total_length_ptr[0] = (UCHAR)((extension_total_length >> 8) & 0xFF);
        extension_total_length_ptr[1] = (UCHAR)(extension_total_length & 0xFF);
    }
#endif

    /* Finally, we have a complete length and can put it into the buffer. Before that,
       save off and return the number of bytes we wrote and need to send. */
    send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_prepend_ptr + length;
    send_packet -> nx_packet_length = send_packet -> nx_packet_length + length;

    return(NX_SECURE_TLS_SUCCESS);
}
#endif /* NX_SECURE_ENABLE_DTLS */

