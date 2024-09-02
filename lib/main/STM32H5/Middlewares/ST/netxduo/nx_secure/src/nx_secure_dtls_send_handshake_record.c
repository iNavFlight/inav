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
/*    _nx_secure_dtls_send_handshake_record               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends a DTLS Handshake record, populating the header  */
/*    in the NX_PACKET structure before passing the packet along to the   */
/*    generic DTLS record send function.                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          DTLS control block            */
/*    send_packet                           Packet to be sent             */
/*    handshake_type                        TLS handshake message type    */
/*    wait_option                           Controls TCP send options     */
/*    include_in_finished                   Indicates the packet is       */
/*                                            included in finished hash   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_dtls_send_record           Send the DTLS record          */
/*    _nx_secure_tls_handshake_hash_update  Update Finished message hash  */
/*    nx_secure_tls_packet_release          Release packet                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_client_handshake      DTLS client state machine     */
/*    _nx_secure_dtls_server_handshake      DTLS server state machine     */
/*    _nx_secure_dtls_session_start         Actual DTLS session start call*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            released packet securely,   */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_dtls_send_handshake_record(NX_SECURE_DTLS_SESSION *dtls_session,
                                           NX_PACKET *send_packet, UCHAR handshake_type,
                                           ULONG wait_option, UINT include_in_finished)
{
UINT       status;
UCHAR     *packet_buffer;
ULONG      length;
NX_PACKET *current_packet;
ULONG      fragment_length;


    /* Build up the DTLS handshake header.
     * Structure:
     * |  1   |   3    |        2        |        3        |        3        |
     * | Type | Length |  Msg Sequence # | Fragment offset | Fragment length |
     */

    /* Actual record data length is the total size of the packet data minus the size of the header.
     * This is a handshake record so we also need to subtract the handshake header. */
    /* Length of the data in the packet. */
    length = send_packet -> nx_packet_length;

    /* Back off the prepend_ptr by NX_SECURE_TLS_RECORD_HEADER_SIZE */
    send_packet -> nx_packet_prepend_ptr -= NX_SECURE_DTLS_HANDSHAKE_HEADER_SIZE;
    send_packet -> nx_packet_length += NX_SECURE_DTLS_HANDSHAKE_HEADER_SIZE;

    /* Pick up the address where the handshake message starts. */
    packet_buffer = send_packet -> nx_packet_prepend_ptr;
    fragment_length = length;


    /* First byte is the message type. */
    packet_buffer[0] = handshake_type;

    /* Next up is the length field (3 bytes). */
    packet_buffer[1] = (UCHAR)((length & 0xFF0000) >> 16);
    packet_buffer[2] = (UCHAR)((length & 0xFF00) >> 8);
    packet_buffer[3] = (UCHAR)(length & 0xFF);

    /* Account for the header in the length. */
    length = length + (USHORT)NX_SECURE_DTLS_HANDSHAKE_HEADER_SIZE;

    /* uint16 message seq. */
    packet_buffer[4] = (UCHAR)(dtls_session -> nx_secure_dtls_local_handshake_sequence >> 8);
    packet_buffer[5] = (UCHAR)(dtls_session -> nx_secure_dtls_local_handshake_sequence);

    /* Increment message sequence number. */
    dtls_session -> nx_secure_dtls_local_handshake_sequence++;

    /* uint24 fragment offset. */
    packet_buffer[6] = 0x00;
    packet_buffer[7] = 0x00;
    packet_buffer[8] = 0x00;

    /* uint24 fragment length. */
    packet_buffer[9]  = (UCHAR)((fragment_length & 0xFF0000) >> 16);
    packet_buffer[10] = (UCHAR)((fragment_length & 0xFF00) >> 8);
    packet_buffer[11] = (UCHAR)(fragment_length & 0xFF);

    /* length += 8; 8 additional bytes in DTLS handshake header. */

    /* Hash this handshake message. We do not hash HelloRequest messages so check that we aren't doing that.
       Hashes include the handshake layer header but not the record layer header. */
    /* "include_in_finished" is set to 0 when we don't want to include the message in the Finished hash - this
     * is true for the first ClientHello sent by the DTLS Client, but not the second. */
    if (include_in_finished && handshake_type != NX_SECURE_TLS_HELLO_REQUEST && handshake_type != NX_SECURE_TLS_HELLO_VERIFY_REQUEST)
    {
        /* Account for large records that exceed the packet size and are chained in multiple packets
           such as large certificate messages with multiple certificates. */
        current_packet = send_packet;
        do
        {
            /* Update the handshake hash with the data. */
            length = (ULONG)(current_packet -> nx_packet_append_ptr - current_packet -> nx_packet_prepend_ptr);
            _nx_secure_tls_handshake_hash_update(&dtls_session -> nx_secure_dtls_tls_session, current_packet -> nx_packet_prepend_ptr,
                                                 (UINT)length);

            /* Advance the packet pointer to the next packet in the chain. */
            current_packet = current_packet -> nx_packet_next;
        } while (current_packet != NX_NULL);
    }

    /* Finally, send the record off to the client. */
    status = _nx_secure_dtls_send_record(dtls_session, send_packet, NX_SECURE_TLS_HANDSHAKE, wait_option);

    if (status != NX_SUCCESS)
    {
        /* Release packet on send error. */
        nx_secure_tls_packet_release(send_packet);
    }

    return(status);
}
#endif /* NX_SECURE_ENABLE_DTLS */

