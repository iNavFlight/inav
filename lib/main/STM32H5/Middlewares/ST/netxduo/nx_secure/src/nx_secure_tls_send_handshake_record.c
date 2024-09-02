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
/*    _nx_secure_tls_send_handshake_record                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends a TLS Handshake record, populating the header   */
/*    in the NX_PACKET structure before passing the packet along to the   */
/*    generic TLS record send function.                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    send_packet                           Packet to be sent             */
/*    handshake_type                        TLS handshake message type    */
/*    wait_option                           Controls TCP send options     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_handshake_hash_update  Update Finished message hash  */
/*    _nx_secure_tls_send_record            Send the TLS record           */
/*    nx_secure_tls_packet_release          Release packet                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*    _nx_secure_tls_server_handshake       TLS server state machine      */
/*    _nx_secure_tls_session_start          Start TLS session             */
/*    _nx_secure_tls_session_renegotiate    Renegotiate TLS session       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            released packet securely,   */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_send_handshake_record(NX_SECURE_TLS_SESSION *tls_session,
                                          NX_PACKET *send_packet, UCHAR handshake_type,
                                          ULONG wait_option)
{
UINT       status;
UCHAR     *packet_buffer;
ULONG      length;
NX_PACKET *current_packet;
UINT       buffer_offset;

    /* Build up the TLS handshake header.
     * Structure:
     * |  1   |   3    |
     * | Type | Length |
     */

    /* Length of the data in the packet. */
    length = send_packet -> nx_packet_length;

    /* Back off the prepend_ptr by NX_SECURE_TLS_RECORD_HEADER_SIZE. This adds to the length because
       we are reclaiming the data from before the prepend pointer. */
    send_packet -> nx_packet_prepend_ptr -= NX_SECURE_TLS_HANDSHAKE_HEADER_SIZE;
    send_packet -> nx_packet_length += NX_SECURE_TLS_HANDSHAKE_HEADER_SIZE;

    /* Pick up the address where the handshake message starts. */
    packet_buffer = send_packet -> nx_packet_prepend_ptr;


    /* First byte is the message type. */
    packet_buffer[0] = handshake_type;

    /* Next up is the length field (3 bytes). */
    packet_buffer[1] = (UCHAR)((length & 0xFF0000) >> 16);
    packet_buffer[2] = (UCHAR)((length & 0xFF00) >> 8);
    packet_buffer[3] = (UCHAR)(length & 0xFF);

    /* Account for the 4 bytes of header in the length. */
    length = length + (USHORT)NX_SECURE_TLS_HANDSHAKE_HEADER_SIZE;

    /* Hash this handshake message. We do not hash HelloRequest messages so check that we aren't doing that.
       Hashes include the handshake layer header but not the record layer header. */
    if (handshake_type != NX_SECURE_TLS_HELLO_REQUEST)
    {
        /* Account for large records that exceed the packet size and are chained in multiple packets
           such as large certificate messages with multiple certificates. */
        current_packet = send_packet;

        buffer_offset = 0;
        do
        {
            /* Update the handshake hash with the data. */
            length = (ULONG)(current_packet -> nx_packet_append_ptr - current_packet -> nx_packet_prepend_ptr);

            /* If using TLS 1.3 and no ciphersuite is chosen, we don't yet know what the handshake hash routine will be,
               so cache the message data to be hashed later. */
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
            if((tls_session->nx_secure_tls_1_3 && tls_session->nx_secure_tls_session_ciphersuite == 0x0) ||
               (handshake_type == NX_SECURE_TLS_CLIENT_HELLO))
#else
            if (handshake_type == NX_SECURE_TLS_CLIENT_HELLO)
#endif /* (NX_SECURE_TLS_TLS_1_3_ENABLED) */
            {
                NX_SECURE_MEMCPY(&tls_session->nx_secure_tls_key_material.nx_secure_tls_handshake_cache[buffer_offset], /* lgtm[cpp/banned-api-usage-required-any] */
                                 current_packet -> nx_packet_prepend_ptr, (UINT)length); /* Use case of memcpy is verified. */

                /* Advance the length. */
                buffer_offset += (UINT)length;
                tls_session->nx_secure_tls_key_material.nx_secure_tls_handshake_cache_length += (UINT)length;
            }
            else
            {
                _nx_secure_tls_handshake_hash_update(tls_session, current_packet -> nx_packet_prepend_ptr,
                                                     (UINT)length);
            }

            /* Advance the packet pointer to the next packet in the chain. */
            current_packet = current_packet -> nx_packet_next;
        } while (current_packet != NX_NULL);
    }


    /* Finally, send the record off to the client. */
    status = _nx_secure_tls_send_record(tls_session, send_packet, NX_SECURE_TLS_HANDSHAKE, wait_option);

    if (status != NX_SUCCESS)
    {
        /* Release packet on send error. */
        nx_secure_tls_packet_release(send_packet);
    }

    return(status);
}

