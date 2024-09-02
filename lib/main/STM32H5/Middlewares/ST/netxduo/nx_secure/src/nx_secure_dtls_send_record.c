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
#include "nx_packet.h"
#include "nx_udp.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_dtls_send_record                         PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encapsulates the DTLS record layer send functionality */
/*    The incoming packet data is wrapped in a DTLS record, which includes*/
/*    a header and footer (for encrypted data). Also, all encryption of   */
/*    application data is handled here.                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          Pointer to DTLS session       */
/*    send_packet                           Packet data to send           */
/*    record_type                           DTLS record type              */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Record send status            */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_dtls_hash_record           Generate hash of payload      */
/*    _nx_secure_tls_record_payload_encrypt Encrypt payload               */
/*    _nx_secure_tls_session_iv_size_get    Get IV size for this session  */
/*    _nxd_udp_socket_send                  Send UDP packet               */
/*    _nxd_udp_socket_source_send           Send UDP packet with specific */
/*                                            source address              */
/*    nx_secure_tls_packet_release          Release packet                */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_client_handshake      DTLS client state machine     */
/*    _nx_secure_dtls_send_handshake_record Send DTLS handshake record    */
/*    _nx_secure_dtls_server_handshake      DTLS server state machine     */
/*    _nx_secure_dtls_session_end           Actual DTLS session end call  */
/*    _nx_secure_dtls_session_receive       Receive DTLS data             */
/*    _nx_secure_dtls_session_send          Actual DTLS session send call */
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
/*  07-29-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            checked seq number overflow,*/
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_dtls_send_record(NX_SECURE_DTLS_SESSION *dtls_session, NX_PACKET *send_packet,
                                 UCHAR record_type, ULONG wait_option)
{
UINT                   status;
UINT                   message_length;
UCHAR                 *mac_secret;
UCHAR                 *record_header;
UCHAR                  record_hash[NX_SECURE_TLS_MAX_HASH_SIZE];
UINT                   hash_length;
ULONG                  length;
USHORT                 iv_size;
UCHAR                 *data;
NX_SECURE_TLS_SESSION *tls_session;
UCHAR                  epoch_seq_num[8];

    NX_PARAMETER_NOT_USED(wait_option);

    /* Pointer to the actual packet data for hashing and encryption. */
    data = send_packet -> nx_packet_prepend_ptr;

    /* Length of the data in the packet. */
    length = send_packet -> nx_packet_length;

    /* Get a pointer to TLS state. */
    tls_session = &dtls_session -> nx_secure_dtls_tls_session;

    /* See if this is an active session, we need to account for the IV if the session cipher
       uses one. */
    if (tls_session -> nx_secure_tls_local_session_active)
    {

        /* Get the size of the IV used by the session cipher. */
        status = _nx_secure_tls_session_iv_size_get(tls_session, &iv_size);

        if (status != NX_SUCCESS)
        {
            return(status);
        }

        /* Back off the pointer to the point before the IV data allocation
           (can be 0). Increases length since we are moving the prepend pointer. */
        send_packet -> nx_packet_prepend_ptr -= iv_size;
        send_packet -> nx_packet_length += iv_size;
    }

    /* Get a pointer to our record header. */
    record_header = send_packet -> nx_packet_prepend_ptr - NX_SECURE_DTLS_RECORD_HEADER_SIZE;

    /* Build the TLS record header. */
    record_header[0] = record_type;

    /* Set the version number. */
    record_header[1] = (UCHAR)((tls_session -> nx_secure_tls_protocol_version & 0xFF00) >> 8);
    record_header[2] = (UCHAR)(tls_session -> nx_secure_tls_protocol_version & 0x00FF);

    /* DTLS Epoch counter. */
    record_header[3] = (UCHAR)(dtls_session -> nx_secure_dtls_local_epoch >> 8);
    record_header[4] = (UCHAR)(dtls_session -> nx_secure_dtls_local_epoch);

    /* DTLS sequence number. */
    record_header[5]  = (UCHAR)(tls_session -> nx_secure_tls_local_sequence_number[1] >> 8);
    record_header[6]  = (UCHAR)(tls_session -> nx_secure_tls_local_sequence_number[1]);
    record_header[7]  = (UCHAR)(tls_session -> nx_secure_tls_local_sequence_number[0] >> 24);
    record_header[8]  = (UCHAR)(tls_session -> nx_secure_tls_local_sequence_number[0] >> 16);
    record_header[9]  = (UCHAR)(tls_session -> nx_secure_tls_local_sequence_number[0] >> 8);
    record_header[10] = (UCHAR)(tls_session -> nx_secure_tls_local_sequence_number[0]);

    epoch_seq_num[0] = record_header[10];
    epoch_seq_num[1] = record_header[9];
    epoch_seq_num[2] = record_header[8];
    epoch_seq_num[3] = record_header[7];
    epoch_seq_num[4] = record_header[6];
    epoch_seq_num[5] = record_header[5];
    epoch_seq_num[6] = record_header[4];
    epoch_seq_num[7] = record_header[3];

    /* Increment the sequence number. */
    if ((tls_session -> nx_secure_tls_local_sequence_number[0] + 1) == 0)
    {
        /* Check for overflow of the 32-bit number. */
        tls_session -> nx_secure_tls_local_sequence_number[1]++;

        if (tls_session -> nx_secure_tls_local_sequence_number[1] == 0)
        {

            /* Check for overflow of the 64-bit unsigned number. As it should not reach here
               in practical, we return a general error to prevent overflow theoretically. */
            return(NX_NOT_SUCCESSFUL);
        }
    }
    tls_session -> nx_secure_tls_local_sequence_number[0]++;

    /* DTLS message length. */
    message_length = length;
    record_header[11] = (UCHAR)((message_length & 0xFF00) >> 8);
    record_header[12] = (UCHAR)(message_length & 0x00FF);

    /* If the session is active, hash and encrypt the record payload using
       the session keys and chosen ciphersuite. */
    if (tls_session -> nx_secure_tls_local_session_active)
    {
        /* Select our proper MAC secret for hashing. */
        if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
        {
            /* If we are a server, we need to use the client's MAC secret. */
            mac_secret = tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_write_mac_secret;
        }
        else
        {
            /* We are a client, so use the server's MAC secret. */
            mac_secret = tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_write_mac_secret;
        }

        if (send_packet -> nx_packet_next)
        {

            /* Chained packet is not supported. */
            return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
        }

        /* Generate the hash on the plaintext data. */
        _nx_secure_dtls_hash_record(dtls_session, tls_session -> nx_secure_tls_local_sequence_number, record_header,
                                    NX_SECURE_DTLS_RECORD_HEADER_SIZE, data, length, record_hash, &hash_length, mac_secret);

        if ((hash_length > ((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(&data[length]))) ||
            (hash_length > sizeof(record_hash)))
        {

            /* Packet buffer is too small. */
            return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
        }

        /* Append the hash to the plaintext data before encryption. */
        NX_SECURE_MEMCPY(&data[length], record_hash, hash_length); /* Use case of memcpy is verified. */

#ifdef NX_SECURE_KEY_CLEAR
        NX_SECURE_MEMSET(record_hash, 0, hash_length);
#endif /* NX_SECURE_KEY_CLEAR  */

        send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_append_ptr + hash_length;
        send_packet -> nx_packet_length = send_packet -> nx_packet_length + hash_length;

        /* Finally, encrypt the entire record including the hash. Note that the length
         * can be changed by the encryption as IVs and padding may be added. */
        _nx_secure_tls_record_payload_encrypt(tls_session, send_packet, (ULONG *)epoch_seq_num, record_type);
    }

    /* The encryption above may have changed the payload length, so get the length from
       the packet and use it to update the record header. */
    message_length = send_packet -> nx_packet_length;

    /* Set the length of the record following encryption. */
    record_header[11] = (UCHAR)((message_length & 0xFF00) >> 8);
    record_header[12] = (UCHAR)(message_length & 0x00FF);

    /* Back off the prepend_ptr for TLS Record header. Note the packet_length field is adjusted
       prior to nx_tcp_socket_send() */
    send_packet -> nx_packet_prepend_ptr -= NX_SECURE_DTLS_RECORD_HEADER_SIZE;

    /* Adjust packet length */
    send_packet -> nx_packet_length += NX_SECURE_DTLS_RECORD_HEADER_SIZE;

    /* DTLS handshake re-transmit handling. */
    if ((record_type == NX_SECURE_TLS_HANDSHAKE) ||
        (record_type == NX_SECURE_TLS_CHANGE_CIPHER_SPEC))
    {
        /* See if the list already has entries. */
        if (dtls_session -> nx_secure_dtls_transmit_sent_head)
        {
            /* Other packets are on the list already, add this one to the tail.  */
            (dtls_session -> nx_secure_dtls_transmit_sent_tail) -> nx_packet_union_next.nx_packet_tcp_queue_next =  send_packet;
            dtls_session -> nx_secure_dtls_transmit_sent_tail =  send_packet;
        }
        else
        {
            /* Empty list, just setup the head and tail to the current packet.  */
            dtls_session -> nx_secure_dtls_transmit_sent_head =  send_packet;
            dtls_session -> nx_secure_dtls_transmit_sent_tail =  send_packet;

            /* Setup a timeout for the packet at the head of the list.  */
            dtls_session -> nx_secure_dtls_handshake_timeout =  NX_SECURE_DTLS_RETRANSMIT_TIMEOUT;
            dtls_session -> nx_secure_dtls_timeout_retries =  0;
        }

        /* Increase the count of packets in the transmit queue. */
        dtls_session -> nx_secure_dtls_transmit_sent_count++;

        /* Mark the packet as in the transmit queue so it isn't released by the send. */
        send_packet -> nx_packet_union_next.nx_packet_tcp_queue_next =  (NX_PACKET *)NX_PACKET_ENQUEUED;
    }

    /* Release the protection before suspending on nx_tcp_socket_send. */
    tx_mutex_put(&_nx_secure_tls_protection);

    /* If local IP address index is set, call _nxd_udp_socket_source_send
       to ensure the source IP address is correct.  */
    if (dtls_session -> nx_secure_dtls_local_ip_address_index == 0xffffffff)
    {

        /* Send the UDP packet(s) containing our record. */
        status = _nxd_udp_socket_send(dtls_session -> nx_secure_dtls_udp_socket, send_packet,
                                      &dtls_session -> nx_secure_dtls_remote_ip_address,
                                      dtls_session -> nx_secure_dtls_remote_port);
    }
    else
    {

        /* Send the UDP packet(s) containing our record. */
        status = _nxd_udp_socket_source_send(dtls_session -> nx_secure_dtls_udp_socket, send_packet,
                                             &dtls_session -> nx_secure_dtls_remote_ip_address,
                                             dtls_session -> nx_secure_dtls_remote_port,
                                             dtls_session -> nx_secure_dtls_local_ip_address_index);
    }

    /* Get the protection after nx_tcp_socket_send. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    if (status != NX_SUCCESS)
    {
        nx_secure_tls_packet_release(send_packet);
        return(NX_SECURE_TLS_TCP_SEND_FAILED);
    }

    return(NX_SECURE_TLS_SUCCESS);
}
#endif /* NX_SECURE_ENABLE_DTLS */

