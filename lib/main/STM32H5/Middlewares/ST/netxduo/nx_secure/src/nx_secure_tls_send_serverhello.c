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

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
const UCHAR _nx_secure_tls_hello_retry_request_random[32] =
{
    0xCF, 0x21, 0xAD, 0x74, 0xE5, 0x9A, 0x61, 0x11, 0xBE, 0x1D, 0x8C, 0x02, 0x1E, 0x65, 0xB8, 0x91,
    0xC2, 0xA2, 0x11, 0x16, 0x7A, 0xBB, 0x8C, 0x5E, 0x07, 0x9E, 0x09, 0xE2, 0xC8, 0xA8, 0x33, 0x9C
};

const UCHAR _nx_secure_tls_1_2_random[8] =
{
    0x44, 0x4F, 0x57, 0x4E, 0x47, 0x52, 0x44, 0x01
};

const UCHAR _nx_secure_tls_1_1_random[8] =
{
    0x44, 0x4F, 0x57, 0x4E, 0x47, 0x52, 0x44, 0x00
};
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_send_serverhello                     PORTABLE C      */
/*                                                           6.1.9        */
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
/*    tls_session                           TLS control block             */
/*    send_packet                           Packet used to send message   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_send_serverhello_extensions                          */
/*                                          Send TLS ServerHello extension*/
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_server_handshake       TLS server state machine      */
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
/*  10-15-2021     Timothy Stapko           Modified comment(s), fixed    */
/*                                            compilation issue with      */
/*                                            TLS 1.3 and disabling TLS   */
/*                                            server,                     */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_send_serverhello(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *send_packet)
{
#ifndef NX_SECURE_TLS_SERVER_DISABLED
ULONG  length;
UINT   gmt_time;
UINT   random_value;
UINT   i;
USHORT ciphersuite;
UCHAR *packet_buffer;
USHORT protocol_version;
UINT   status;


    /* Build up the ServerHello message.
     * Structure:
     * |     2       |          4 + 28          |    1       |   <SID len>  |      2      |      1      |    2     | <Ext. Len> |
     * | TLS version |  Random (time + random)  | SID length |  Session ID  | Ciphersuite | Compression | Ext. Len | Extensions |
     */

    if ((6u + sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random)) >
        ((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr)))
        
    {

        /* Packet buffer is too small to hold random. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    packet_buffer = send_packet -> nx_packet_append_ptr;

    /* Use our length as an index into the buffer. */
    length = 0;

    /* First two bytes of the hello following the header are the TLS major and minor version numbers. */
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

    NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random, (UCHAR *)&gmt_time, sizeof(gmt_time)); /* Use case of memcpy is verified. lgtm[cpp/banned-api-usage-required-any] */

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if (tls_session -> nx_secure_tls_server_state == NX_SECURE_TLS_SERVER_STATE_SEND_HELLO_RETRY)
    {
        NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random,
                         _nx_secure_tls_hello_retry_request_random,
                         sizeof(_nx_secure_tls_hello_retry_request_random)); /* Use case of memcpy is verified. */
    }
    else if (!(tls_session -> nx_secure_tls_1_3) && !(tls_session -> nx_secure_tls_protocol_version_override))
    {
        /* Next 20 bytes is random data. */
        for (i = 4; i <= 20; i += (UINT)sizeof(random_value))
        {
            random_value = (UINT)NX_RAND();
            tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random[i]     = (UCHAR)(random_value);
            tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random[i + 1] = (UCHAR)(random_value >> 8);
            tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random[i + 2] = (UCHAR)(random_value >> 16);
            tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random[i + 3] = (UCHAR)(random_value >> 24);
        }

        /* RFC 8446 4.1.3:
           TLS 1.3 servers which negotiate TLS 1.2 or below in response to a ClientHello 
           MUST set the last 8 bytes of their Random value specially in their ServerHello. */
        if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2)
        {
            NX_SECURE_MEMCPY(&(tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random[24]),
                             _nx_secure_tls_1_2_random,
                             sizeof(_nx_secure_tls_1_2_random)); /* Use case of memcpy is verified. */
        }
        else
        {
            NX_SECURE_MEMCPY(&(tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random[24]),
                             _nx_secure_tls_1_1_random,
                             sizeof(_nx_secure_tls_1_1_random)); /* Use case of memcpy is verified. */
        }
    }
    else
#endif
    {
        
        /* Next 28 bytes is random data. */
        for (i = 4; i <= 28; i += (UINT)sizeof(random_value))
        {
            random_value = (UINT)NX_RAND();
            tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random[i]     = (UCHAR)(random_value);
            tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random[i + 1] = (UCHAR)(random_value >> 8);
            tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random[i + 2] = (UCHAR)(random_value >> 16);
            tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random[i + 3] = (UCHAR)(random_value >> 24);
        }
    }

    /* Copy the random data into the packet. */
    NX_SECURE_MEMCPY(&packet_buffer[length], tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random, /* lgtm[cpp/banned-api-usage-required-any] */
                     sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random)); /* Use case of memcpy is verified. */
    length += sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random);

    /* Session ID length is one byte. Session ID data follows if we ever implement session resumption. */
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if(tls_session->nx_secure_tls_1_3)
    {
        /* TLS 1.3 doesn't use the session ID, but the Server must echo the server ID provided in the ClientHello
           back to the client or the client will abort the handshake (RFC 8446, Section 4.1.3). */

        packet_buffer[length] = tls_session->nx_secure_tls_session_id_length;
        length++;

        if ((length + tls_session->nx_secure_tls_session_id_length + 3) >
            ((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr)))
        {

            /* Packet buffer is too small to hold random. */
            return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
        }

        NX_SECURE_MEMCPY(&packet_buffer[length], tls_session -> nx_secure_tls_session_id, tls_session->nx_secure_tls_session_id_length); /* Use case of memcpy is verified. */
        length += tls_session->nx_secure_tls_session_id_length;
    }
    else
#endif
    {
        packet_buffer[length] = 0;
        length++;
    }
    /* Finally, our chosen ciphersuite - this is selected when we receive the Client Hello. */
    ciphersuite = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_ciphersuite;
    packet_buffer[length]     = (UCHAR)(ciphersuite >> 8);
    packet_buffer[length + 1] = (UCHAR)ciphersuite;
    length += 2;

    /* Compression method - for now this is NULL. */
    packet_buffer[length] = 0x0;
    length++;

    /* ============ TLS ServerHello extensions. ============= */
    status = _nx_secure_tls_send_serverhello_extensions(tls_session, packet_buffer, &length,
                                                        ((ULONG)(send_packet -> nx_packet_data_end) -
                                                         (ULONG)packet_buffer));

    if (status)
    {
        return(status);
    }

    /* Finally, we have a complete length and can put it into the buffer. Before that,
       save off and return the number of bytes we wrote and need to send. */
    send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_append_ptr + length;
    send_packet -> nx_packet_length = send_packet -> nx_packet_length + length;

    return(status);  
#else

    NX_PARAMETER_NOT_USED(send_packet);

    /* If TLS Server is disabled and we have processed a ClientKeyExchange, something is wrong... */
    tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_ERROR;   
    
    /* Server is disabled, we shouldn't be sending a ServerHello - error! */
    return(NX_SECURE_TLS_INVALID_STATE);
#endif    
    
}

