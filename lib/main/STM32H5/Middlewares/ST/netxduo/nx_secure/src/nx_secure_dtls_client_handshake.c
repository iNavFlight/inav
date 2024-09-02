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
/*    _nx_secure_dtls_client_handshake                    PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function runs the DTLS Client mode state machine. It processes */
/*    an incoming handshake record and takes appropriate action to        */
/*    advance the DTLS Client handshake.                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          DTLS control block            */
/*    packet_buffer                         Pointer into record buffer    */
/*    data_length                           Length of data                */
/*    wait_option                           Controls timeout actions      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_dtls_allocate_handshake_packet                           */
/*                                          Allocate DTLS handshake packet*/
/*    _nx_secure_dtls_packet_allocate       Allocate internal DTLS packet */
/*    _nx_secure_dtls_process_handshake_header                            */
/*                                          Process handshake header      */
/*    _nx_secure_dtls_process_helloverifyrequest                          */
/*                                          Process HelloVerifyRequest    */
/*    _nx_secure_dtls_retransmit_queue_flush                              */
/*                                          Flush retransmit queue        */
/*    _nx_secure_dtls_send_clienthello      Send ClientHello              */
/*    _nx_secure_dtls_send_handshake_record Send DTLS handshake record    */
/*    _nx_secure_dtls_send_record           Send DTLS records             */
/*    _nx_secure_tls_generate_keys          Generate session keys         */
/*    _nx_secure_tls_generate_premaster_secret                            */
/*                                          Generate premaster secret     */
/*    _nx_secure_tls_handshake_hash_update  Update Finished hash          */
/*    _nx_secure_tls_process_certificate_request                          */
/*                                          Process certificate request   */
/*    _nx_secure_tls_process_finished       Process Finished message      */
/*    _nx_secure_tls_process_remote_certificate                           */
/*                                          Process remote certificate    */
/*    _nx_secure_tls_process_server_key_exchange                          */
/*                                          Process ServerKeyExchange     */
/*    _nx_secure_tls_process_serverhello    Process ServerHello           */
/*    _nx_secure_tls_send_certificate       Send DTLS certificate         */
/*    _nx_secure_tls_send_certificate_verify                              */
/*                                          Send certificate verify       */
/*    _nx_secure_tls_send_changecipherspec  Send ChangeCipherSpec         */
/*    _nx_secure_tls_send_client_key_exchange                             */
/*                                          Send ClientKeyExchange        */
/*    _nx_secure_tls_send_finished          Send Finished message         */
/*    _nx_secure_tls_session_keys_set       Set session keys              */
/*    nx_secure_tls_packet_release          Release packet                */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_process_record        Process DTLS record data      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            verified memmove use cases, */
/*                                            released packet securely,   */
/*                                            fixed certificate buffer    */
/*                                            allocation,                 */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Timothy Stapko           Modified comment(s),          */
/*                                            improved buffer length      */
/*                                            verification, added null    */
/*                                            pointer checking,           */
/*                                            resulting in version 6.1.3  */
/*  01-31-2022     Timothy Stapko           Modified comment(s),          */
/*                                            fixed out-of-order handling,*/
/*                                            resulting in version 6.1.10 */
/*  07-29-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            removed duplicated alert,   */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_dtls_client_handshake(NX_SECURE_DTLS_SESSION *dtls_session, UCHAR *packet_buffer,
                                      UINT data_length, ULONG wait_option)
{
#ifndef NX_SECURE_TLS_CLIENT_DISABLED
UINT                   status;
USHORT                 message_type = NX_SECURE_TLS_INVALID_MESSAGE;
UINT                   header_bytes;
UINT                   message_length;
UCHAR                 *data_start = NX_NULL;
NX_PACKET             *send_packet = NX_NULL;
NX_PACKET_POOL        *packet_pool;
UINT                   message_seq;
UINT                   fragment_offset;
UINT                   fragment_length;
UCHAR                 *fragment_buffer;
NX_SECURE_TLS_SESSION *tls_session;

    /* Basic state machine for handshake:
     * 1. We have received a handshake message, now process the header.
     * 2. Then process the message itself and populate the TLS socket structure.
     * 3. Follow up with whatever actions are needed.
     */

    /* Get a reference to the internal TLS state for ease of use. */
    tls_session = &dtls_session -> nx_secure_dtls_tls_session;

    /* Use the TLS packet buffer for fragment processing. */
    fragment_buffer = tls_session->nx_secure_tls_packet_buffer;

    while (data_length > 0)
    {
        header_bytes = data_length;

        /* First, process the handshake message to get our state and any data therein. */
        status = _nx_secure_dtls_process_handshake_header(packet_buffer, &message_type, &header_bytes,
                                                          &message_length, &message_seq, &fragment_offset, &fragment_length);

        if (status == NX_SUCCESS)
        {

            /* Out-of-order plus fragmentation:
             * - If message sequence > current message sequence
             *     - New message
             * - If message_sequence <= current message sequence
             *    - If fragment_length < message length, we have a fragment
             *    - Once fragment is reassembled, check message sequence for repeats
             */

            /* If we see a repeated message sequence, assume an unnecessary retransmission and ignore. */
            if (message_seq < dtls_session -> nx_secure_dtls_remote_handshake_sequence)
            {
                /* Re-transmitted message. */
                return(NX_SUCCESS);
            }

            /* Check the fragment_length with the lenght of packet buffer. */
            if ((header_bytes + fragment_length) > data_length)
            {
                return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
            }

            /* Check available area of buffer. */
            if ((fragment_offset + fragment_length) > tls_session -> nx_secure_tls_packet_buffer_size ||
                (header_bytes + message_length) > tls_session -> nx_secure_tls_packet_buffer_size)
            {
                return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
            }

            /* If this message sequence isn't what we expect, continue reading packets. */ 
            if(message_seq != dtls_session -> nx_secure_dtls_expected_handshake_sequence)
            {
                return(NX_SECURE_TLS_OUT_OF_ORDER_MESSAGE);
            }

            /* If we have a new sequence number, we have a new record (may be fragmented). Unless
               the sequence number is 0, which means it is the first record. */
            if (message_seq > dtls_session -> nx_secure_dtls_remote_handshake_sequence || (message_seq == 0 && fragment_offset == 0))
            {
                /* New record starting, reset the fragment length and handshake sequence number. */
                dtls_session -> nx_secure_dtls_remote_handshake_sequence = message_seq;
                dtls_session -> nx_secure_dtls_fragment_length = message_length;
            }

            if (fragment_length > dtls_session -> nx_secure_dtls_fragment_length)
            {
                return(NX_SECURE_TLS_INVALID_PACKET);
            }

            /* When we receive a message fragment, subtract it from the current fragment length. */
            dtls_session -> nx_secure_dtls_fragment_length -= fragment_length;

            /* Copy the fragment data (minus the header) into the reassembly buffer. */
            NX_SECURE_MEMCPY(&fragment_buffer[fragment_offset], &packet_buffer[header_bytes], fragment_length); /* Use case of memcpy is verified. */

            /* If we still have fragments to add, just return success. */
            if (dtls_session -> nx_secure_dtls_fragment_length > 0)
            {
                return(NX_SUCCESS);
            }
            else
            {
                /* At beginning of handshake, reset the expected sequence number. */
                if (message_seq == 0)
                {
                    dtls_session -> nx_secure_dtls_expected_handshake_sequence = 0;
                }

                /* If the recontructed message has a sequence not equal to the expected, it's
                   a retransmission we need to ignore. */
                if (message_seq != dtls_session -> nx_secure_dtls_expected_handshake_sequence)
                {
                    return(NX_SUCCESS);
                }

                /* Our next expected handshake message sequence number is 1 greater than this one. */
                dtls_session -> nx_secure_dtls_expected_handshake_sequence = message_seq + 1;

                /* Put the header into the packet buffer, adjusting the fields to create a seam-less
                 * DTLS record. */
                NX_SECURE_MEMMOVE(&fragment_buffer[header_bytes], fragment_buffer, message_length); /* Use case of memmove is verified. */

                /* Reconstruct the header in the fragment buffer so we can hash the
                   reconstructed record as if it were never fragmented. */

                /* Type. */
                fragment_buffer[0] = (UCHAR)message_type;

                /* Length. */
                fragment_buffer[1] = (UCHAR)(message_length >> 16);
                fragment_buffer[2] = (UCHAR)(message_length >> 8);
                fragment_buffer[3] = (UCHAR)(message_length);

                /* Sequence. */
                fragment_buffer[4] = (UCHAR)(message_seq  >> 8);
                fragment_buffer[5] = (UCHAR)(message_seq);

                /* Fragment offset is now 0. */
                fragment_buffer[6] = 0;
                fragment_buffer[7] = 0;
                fragment_buffer[8] = 0;

                /* Fragment length is now == message length. */
                fragment_buffer[9] = (UCHAR)(message_length >> 16);
                fragment_buffer[10] = (UCHAR)(message_length >> 8);
                fragment_buffer[11] = (UCHAR)(message_length);

                /* We have a reassembled DTLS record, use that for the handshake. */
                data_start = fragment_buffer;
            }
        }

        if (status != NX_SECURE_TLS_SUCCESS)
        {
            return(status);
        }

        /* Allocate a packet for all send operations.  */
        packet_pool = tls_session -> nx_secure_tls_packet_pool;

        /* Hash this handshake message. We do not hash HelloRequest messages.
           Hashes include the handshake layer header but not the record layer header. */
        if (message_type != NX_SECURE_TLS_HELLO_REQUEST && message_type != NX_SECURE_TLS_FINISHED &&
            message_type != NX_SECURE_TLS_HELLO_VERIFY_REQUEST)
        {
            _nx_secure_tls_handshake_hash_update(tls_session, data_start, message_length + header_bytes);
        }

        /* Reduce total length by the size of this fragment. */
        data_length -= (fragment_length + header_bytes);
        data_start += header_bytes;

        /* Process the message itself information from the header. */
        switch (message_type)
        {
        case NX_SECURE_TLS_HELLO_VERIFY_REQUEST:
            /* Initial DTLS message from server, contains a cookie for later transmissions. */
            status = _nx_secure_dtls_process_helloverifyrequest(dtls_session, data_start, message_length);

            if (status)
            {

                /* This is the end of a flight, clear out the transmit queue. */
                _nx_secure_dtls_retransmit_queue_flush(dtls_session);
            }
            break;
        case NX_SECURE_TLS_SERVER_HELLO:
            /* Server has responded to our ClientHello. */
            status = _nx_secure_tls_process_serverhello(tls_session, data_start, message_length);
            break;
        case NX_SECURE_TLS_CERTIFICATE_MSG:
            /* Server has sent its certificate message. */
            status = _nx_secure_tls_process_remote_certificate(tls_session, data_start, message_length, message_length);
            break;
        case NX_SECURE_TLS_SERVER_HELLO_DONE:
            /* Server has responded to our ClientHello. */
            /* A ServerHelloDone does not contain any data - it simply changes state. */
            tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_SERVERHELLO_DONE;

            /* This is the end of a flight, clear out the transmit queue. */
            _nx_secure_dtls_retransmit_queue_flush(dtls_session);
            break;
        case NX_SECURE_TLS_SERVER_KEY_EXCHANGE:
            /* Server has sent a key exchange message, used for certain ciphersuites (DH and PSK mainly). */
            status = _nx_secure_tls_process_server_key_exchange(tls_session, data_start, message_length);
            break;
        case NX_SECURE_TLS_CERTIFICATE_REQUEST:
            /* Server has requested we provide a client certificate. */
            status = _nx_secure_tls_process_certificate_request(tls_session, data_start, message_length);
            break;
        case NX_SECURE_TLS_FINISHED:
            /* Final handshake message from the server, process it (verify the server handshake hash). */
            status = _nx_secure_tls_process_finished(tls_session, data_start, message_length);

            /* This is the end of a flight, clear out the transmit queue. */
            _nx_secure_dtls_retransmit_queue_flush(dtls_session);
            break;
        case NX_SECURE_TLS_HELLO_REQUEST:
            /* Server has requested we restart the session. If we are in the middle of a handshake already
             * (session is not active) then ignore. If we are in an active session, we can choose to
             * send a ClientHello (start the handshake again) or send a no_renegotiation alert. */

            /* A HelloRequest does not contain any data - it simply changes state. */
            tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_HELLO_REQUEST;
            break;
        case NX_SECURE_TLS_CERTIFICATE_VERIFY:
        case NX_SECURE_TLS_CLIENT_KEY_EXCHANGE:
        case NX_SECURE_TLS_CLIENT_HELLO:
        case NX_SECURE_TLS_INVALID_MESSAGE:
        case NX_SECURE_TLS_CERTIFICATE_URL:
        case NX_SECURE_TLS_CERTIFICATE_STATUS:
        default:
            /* The message received was not a valid TLS server handshake message, send alert and return. */
            status = NX_SECURE_TLS_UNEXPECTED_MESSAGE;
            break;
        }

        /* Check for errors in processing messages. */
        if (status != NX_SECURE_TLS_SUCCESS)
        {

            return(status);
        }

        /* Now take any actions based on state set in the message processing. */
        switch (tls_session -> nx_secure_tls_client_state)
        {
        case NX_SECURE_TLS_CLIENT_STATE_IDLE:
            /* Client isn't doing anything right now. */
            break;
        case NX_SECURE_TLS_CLIENT_STATE_ERROR:
        case NX_SECURE_TLS_CLIENT_STATE_ALERT_SENT:
            /* This means an error was encountered at some point in processing a valid message. At this point
               the alert was sent, so just return a status indicating as much. */
            return(NX_SECURE_TLS_HANDSHAKE_FAILURE);
            break;
        case NX_SECURE_TLS_CLIENT_STATE_HELLO_REQUEST:
            /* Server sent a hello request, indicating it wants to restart the handshake process with a new ClientHello. */
            if (tls_session -> nx_secure_tls_local_session_active)
            {
                /* Respond to the HelloRequest with a "no_renegotiation" alert since we don't want to restart the handshake. */
                status = NX_SECURE_TLS_NO_RENEGOTIATION_ERROR;
            }
            /* If we are still in a handshake (session is not active) then ignore the message. */
            break;
        case NX_SECURE_TLS_CLIENT_STATE_HELLO_VERIFY:

            /* Get ClientHello packet from header of transmit queue. */
            send_packet = dtls_session -> nx_secure_dtls_transmit_sent_head;
            if ((send_packet == NX_NULL) || (send_packet -> nx_packet_queue_next != ((NX_PACKET *)NX_DRIVER_TX_DONE)))
            {

                /* Invalid packet. */
                status = NX_INVALID_PACKET;
                break;
            }

            /* Clear the transmit queue. */
            dtls_session -> nx_secure_dtls_transmit_sent_head = NX_NULL;
            dtls_session -> nx_secure_dtls_transmit_sent_tail = NX_NULL;
            dtls_session -> nx_secure_dtls_transmit_sent_count = 0;

            /* Reset the packet. */
            send_packet -> nx_packet_prepend_ptr += sizeof(NX_UDP_HEADER);
            send_packet -> nx_packet_length -= sizeof(NX_UDP_HEADER);
            send_packet -> nx_packet_union_next.nx_packet_tcp_queue_next = (NX_PACKET *)NX_PACKET_ALLOCATED;

            _nx_secure_dtls_send_clienthello(dtls_session, send_packet);

            status = _nx_secure_dtls_send_handshake_record(dtls_session, send_packet, NX_SECURE_TLS_CLIENT_HELLO, wait_option, 1);
            break;
        case NX_SECURE_TLS_CLIENT_STATE_SERVERHELLO:
            /* We processed a serverhello above. Don't do anything right now. */
            break;
        case NX_SECURE_TLS_CLIENT_STATE_SERVER_CERTIFICATE:
            /* Processed a server certificate above. Here, we extract the public key and do any verification
               we want - the TLS implementation will verify certificate authenticity by checking the issuer
               signature, but any other verification will be done by the caller via a callback. */
            break;
        case NX_SECURE_TLS_CLIENT_STATE_SERVER_KEY_EXCHANGE:
            break;
        case NX_SECURE_TLS_CLIENT_STATE_CERTIFICATE_REQUEST:
            /* Set flag to send CertificateVerify once we have received ServerHelloDone. */
            tls_session -> nx_secure_tls_client_certificate_requested = 1;
            break;
        case NX_SECURE_TLS_CLIENT_STATE_SERVERHELLO_DONE:
            /* We received a ServerHelloDone, meaning we now have all the information we need to generate
               our key material. First check if the server requested our client certificate. */

            if (tls_session -> nx_secure_tls_client_certificate_requested)
            {

                /* The server has requested a client certificate. Provide that certificate to the server here. */
                status = _nx_secure_dtls_allocate_handshake_packet(dtls_session, packet_pool, &send_packet, wait_option);

                if (status != NX_SUCCESS)
                {
                    break;
                }

                _nx_secure_tls_send_certificate(tls_session, send_packet, wait_option);

                status = _nx_secure_dtls_send_handshake_record(dtls_session, send_packet, NX_SECURE_TLS_CERTIFICATE_MSG, wait_option, 1);

                if (status != NX_SUCCESS)
                {
                    break;
                }
            }

            /* Now, generate the pre-master secret that is used to generate keys for our session. */
            status = _nx_secure_tls_generate_premaster_secret(tls_session, NX_SECURE_DTLS);
            if (status != NX_SUCCESS)
            {
                break;
            }

            /* We have received and processed a ServerHelloDone. Now respond to the client appropriately. */
            status = _nx_secure_dtls_allocate_handshake_packet(dtls_session, packet_pool, &send_packet, wait_option);

            if (status != NX_SUCCESS)
            {
                break;
            }

            /* Generate and send the ClientKeyExchange message. */
            _nx_secure_tls_send_client_key_exchange(tls_session, send_packet);
            status = _nx_secure_dtls_send_handshake_record(dtls_session, send_packet, NX_SECURE_TLS_CLIENT_KEY_EXCHANGE, wait_option, 1);

            if (status != NX_SUCCESS)
            {
                break;
            }

            /* After sending ClientKeyExchange, we need to send a CertificateVerify message if the
               server has requested a certificate. */
            if (tls_session -> nx_secure_tls_client_certificate_requested)
            {
                /* We can now clear the flag since this is the last specific certificate message sent. */
                tls_session -> nx_secure_tls_client_certificate_requested = 0;

                /* Allocate packet for CertificateVerify. */
                status = _nx_secure_dtls_allocate_handshake_packet(dtls_session, packet_pool, &send_packet, wait_option);

                if (status != NX_SUCCESS)
                {
                    break;
                }

                _nx_secure_tls_send_certificate_verify(tls_session, send_packet);

                status = _nx_secure_dtls_send_handshake_record(dtls_session, send_packet, NX_SECURE_TLS_CERTIFICATE_VERIFY, wait_option, 1);

                if (status != NX_SUCCESS)
                {
                    break;
                }
            }

            /* Generate our key material from the data collected thus far and put it all into our
               socket structure. Don't call generate keys before sending the client_key_exchange message
               since it needs the pre-master secret and this call clears it out (for security). */
            status = _nx_secure_tls_generate_keys(tls_session);

            if (status != NX_SUCCESS)
            {
                break;
            }

            /* Release the protection before suspending on nx_packet_allocate. */
            tx_mutex_put(&_nx_secure_tls_protection);

            /* We have received everything we need to complete the handshake. Keys have been
             * generated above. Now end the handshake with a ChangeCipherSpec (indicating following
             * messages are encrypted) and the encrypted Finished message. */
            status = _nx_secure_dtls_packet_allocate(dtls_session, packet_pool, &send_packet, wait_option);

            /* Get the protection after nx_packet_allocate. */
            tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

            if (status != NX_SUCCESS)
            {
                break;
            }

            /* ChangeCipherSpec is NOT a handshake message, so send as a normal TLS record. */
            _nx_secure_tls_send_changecipherspec(tls_session, send_packet);
            status = _nx_secure_dtls_send_record(dtls_session, send_packet, NX_SECURE_TLS_CHANGE_CIPHER_SPEC, wait_option);

            if (status != NX_SUCCESS)
            {
                /* Release packet on send error. */
                nx_secure_tls_packet_release(send_packet);
                break;
            }
            /* The local session is now active since we sent the changecipherspec message.
               NOTE: Do not set this flag until after the changecipherspec message has been passed to the send record
               routine - this flag causes encryption and hashing to happen on records. ChangeCipherSpec should be the last
               un-encrypted/un-hashed record sent. */

            /* For DTLS, reset sequence number and advance epoch right after CCS message is sent. */
            NX_SECURE_MEMSET(tls_session -> nx_secure_tls_local_sequence_number, 0, sizeof(tls_session -> nx_secure_tls_local_sequence_number));

            status = _nx_secure_tls_session_keys_set(tls_session, NX_SECURE_TLS_KEY_SET_LOCAL);

            if (status != NX_SUCCESS)
            {
                break;
            }

            /* Advance the DTLS epoch - all messages after the ChangeCipherSpec are in a new epoch. */
            dtls_session -> nx_secure_dtls_local_epoch = (USHORT)(dtls_session -> nx_secure_dtls_local_epoch + 1);

            /* We can now send our finished message, which will be encrypted using the chosen ciphersuite. */
            status = _nx_secure_dtls_allocate_handshake_packet(dtls_session, packet_pool, &send_packet, wait_option);

            if (status != NX_SUCCESS)
            {
                break;
            }

            /* Generate and send the finished message, which completes the handshake. */
            _nx_secure_tls_send_finished(tls_session, send_packet);
            status = _nx_secure_dtls_send_handshake_record(dtls_session, send_packet, NX_SECURE_TLS_FINISHED, wait_option, 1);

            break;
        case NX_SECURE_TLS_CLIENT_STATE_HANDSHAKE_FINISHED:
            /* We processed a server finished message, completing the handshake. Verify all is good and if so,
               continue to the encrypted session. */
            break;
        default:
            status = NX_SECURE_TLS_INVALID_STATE;
        }

        /* If we have an error at this point, we have experienced a problem in sending
           handshake messages, which is some type of internal issue. */
        if (status != NX_SUCCESS)
        {

            return(status);
        }

        /* Advance the buffer pointer past the fragment. */
        packet_buffer += (fragment_length + header_bytes);
    } /* End while. */
    return(NX_SUCCESS);
#else /* TLS Client disabled. */

    /* We don't use the parameters since this is an error case. */
    NX_PARAMETER_NOT_USED(packet_buffer);
    NX_PARAMETER_NOT_USED(wait_option);
    NX_PARAMETER_NOT_USED(data_length);

    /* If TLS Client is disabled and we are in the client state machine, something is wrong... */
    dtls_session -> nx_secure_dtls_tls_session.nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_ERROR;
    return(NX_SECURE_TLS_INVALID_STATE);
#endif
}
#endif /* NX_SECURE_ENABLE_DTLS */

