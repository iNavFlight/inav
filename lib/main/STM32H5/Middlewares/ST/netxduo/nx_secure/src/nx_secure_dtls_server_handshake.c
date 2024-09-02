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
/*    _nx_secure_dtls_server_handshake                    PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function runs the DTLS Server mode state machine. It processes */
/*    an incoming handshake record and takes appropriate action to        */
/*    advance the DTLS Server handshake.                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          TLS control block             */
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
/*    _nx_secure_dtls_process_clienthello   Process ClientHello           */
/*    _nx_secure_dtls_process_handshake_header                            */
/*                                          Process handshake header      */
/*    _nx_secure_dtls_retransmit_queue_flush                              */
/*                                          Flush retransmit queue        */
/*    _nx_secure_dtls_send_handshake_record Send DTLS handshake record    */
/*    _nx_secure_dtls_send_helloverifyrequest                             */
/*                                          Send DTLS HelloVerifyRequest  */
/*    _nx_secure_dtls_send_record           Send DTLS records             */
/*    _nx_secure_dtls_send_serverhello      Send DTLS ServerHello         */
/*    _nx_secure_tls_generate_keys          Generate session keys         */
/*    _nx_secure_tls_handshake_hash_init    Initialize Finished hash      */
/*    _nx_secure_tls_handshake_hash_update  Update Finished hash          */
/*    _nx_secure_tls_process_client_key_exchange                          */
/*                                          Process key exchange          */
/*    _nx_secure_tls_process_finished       Process Finished message      */
/*    _nx_secure_tls_send_certificate       Send TLS certificate          */
/*    _nx_secure_tls_send_changecipherspec  Send ChangeCipherSpec         */
/*    _nx_secure_tls_send_finished          Send Finished message         */
/*    _nx_secure_tls_send_server_key_exchange                             */
/*                                          Send ServerKeyExchange        */
/*    _nx_secure_tls_session_keys_set       Set session keys              */
/*    _nx_secure_tls_process_remote_certificate                           */
/*                                          Process remote certificate    */
/*    _nx_secure_tls_process_certificate_verify                           */
/*                                          Process certificate verify    */
/*    _nx_secure_tls_send_certificate_request                             */
/*                                          Send certificate request      */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*    [nx_secure_dtls_receive_notify]       Notify aaplication of packet  */
/*                                            receive                     */
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
/*                                            fixed renegotiation bug,    */
/*                                            fixed certificate buffer    */
/*                                            allocation,                 */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Timothy Stapko           Modified comment(s),          */
/*                                            improved buffer length      */
/*                                            verification,               */
/*                                            resulting in version 6.1.3  */
/*  01-31-2022     Timothy Stapko           Modified comment(s),          */
/*                                            fixed out-of-order handling,*/
/*                                            resulting in version 6.1.10 */
/*  07-29-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            removed duplicated alert,   */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_dtls_server_handshake(NX_SECURE_DTLS_SESSION *dtls_session, UCHAR *packet_buffer,
                                      UINT data_length, ULONG wait_option)
{
#ifndef NX_SECURE_TLS_SERVER_DISABLED
UINT                                  status;
USHORT                                message_type;
UINT                                  header_bytes;
UINT                                  message_length;
UINT                                  message_seq;
UINT                                  fragment_offset;
UINT                                  fragment_length;
NX_PACKET                            *send_packet;
NX_PACKET_POOL                       *packet_pool;
UCHAR                                *packet_start;
NX_SECURE_TLS_SESSION                *tls_session;
UCHAR                                *fragment_buffer;


    /* Basic state machine for handshake:
     * 1. We have received a handshake message, now process the header.
     * 2. Then process the message itself and populate the TLS socket structure.
     * 3. Follow up with whatever actions are needed.
     */

    /* Save a pointer to the start of our packet for the hash that happens below. */
    packet_start = packet_buffer;



    /* Get a reference to TLS state. */
    tls_session = &dtls_session -> nx_secure_dtls_tls_session;

    /* Use the TLS packet buffer for fragment processing. */
    fragment_buffer = tls_session->nx_secure_tls_packet_buffer;

    header_bytes = data_length;

    /* First, process the handshake message to get our state and any data therein. */
    status = _nx_secure_dtls_process_handshake_header(packet_buffer, &message_type, &header_bytes,
                                                      &message_length, &message_seq, &fragment_offset, &fragment_length);

    if (status == NX_SUCCESS)
    {
        /* For now, if we see a repeated message sequence, assume an unnecessary retransmission and ignore. */
        /* Don't ignore sequence 0 - it's a new handshake request! */
        if (message_seq < dtls_session -> nx_secure_dtls_remote_handshake_sequence)
        {
            /* Re-transmitted message. */
            return(NX_CONTINUE);
        }

        /* When we receive a message fragment, subtract it from the current fragment length. */
        if ((header_bytes + fragment_length) > data_length)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
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

            /* If the recontructed message has a sequence number not equal to the expected, it's
               a retransmission or out-of-order message we need to ignore. */
            if (message_seq != dtls_session -> nx_secure_dtls_expected_handshake_sequence)
            {
                return(NX_CONTINUE);
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
            packet_buffer = fragment_buffer + header_bytes;
            packet_start =  fragment_buffer;
        }
    }

    if (status != NX_SECURE_TLS_SUCCESS)
    {
        return(status);
    }

    /* Get reference to the packet pool so we can allocate a packet for all send operations.  */
    packet_pool = tls_session -> nx_secure_tls_packet_pool;

    /* We need to hash all of the handshake messages that we receive and send. If this message is a ClientHello,
       then we need to initialize the hashes (TLS 1.1 uses both MD5 and SHA-1). The final hash is generated
       in the "Finished" message.  */
    if (message_type == NX_SECURE_TLS_CLIENT_HELLO)
    {
        /* Initialize the handshake hashes used for the Finished message. */
        _nx_secure_tls_handshake_hash_init(tls_session);
    }

    /* Process the message itself information from the header. */
    switch (message_type)
    {
    case NX_SECURE_TLS_CLIENT_HELLO:
        /* Client is establishing a TLS session with our server. */
        status = _nx_secure_dtls_process_clienthello(dtls_session, packet_buffer, message_length);

        /* This is the end of a flight, clear out the transmit queue. */
        _nx_secure_dtls_retransmit_queue_flush(dtls_session);
        break;
#ifdef NX_SECURE_ENABLE_CLIENT_CERTIFICATE_VERIFY
    case NX_SECURE_TLS_CERTIFICATE_MSG:
        /* Client sent certificate message (in response to a request from us. Process it now. */
        status = _nx_secure_tls_process_remote_certificate(tls_session, packet_buffer, message_length, message_length);
        tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_CLIENT_CERTIFICATE;
        break;
    case NX_SECURE_TLS_CERTIFICATE_VERIFY:
        /* Client has responded to a certificate request with a CertificateVerify message. */
        status = _nx_secure_tls_process_certificate_verify(tls_session, packet_buffer, message_length);

        if(status == NX_SUCCESS)
        {
            /* If remote certificate verification was a success, we have received credentials
               from the remote host and may now pass Finished message processing once received. */
            tls_session -> nx_secure_tls_received_remote_credentials = NX_TRUE;
        }

        tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_CERTIFICATE_VERIFY;
        break;
#endif /* NX_SECURE_ENABLE_CLIENT_CERTIFICATE_VERIFY */
    case NX_SECURE_TLS_CLIENT_KEY_EXCHANGE:
        /* Received a client key exchange message, meaning it is time to generate keys if we can. */
        status = _nx_secure_tls_process_client_key_exchange(tls_session, packet_buffer, message_length, NX_SECURE_DTLS);
        tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_KEY_EXCHANGE;

        if (status == NX_SECURE_TLS_SUCCESS)
        {
            /* Generate our key material from the data collected thus far and put it all into our
               socket structure. */
            status = _nx_secure_tls_generate_keys(tls_session);
        }
        break;
    case NX_SECURE_TLS_FINISHED:
        /* Final handshake message from the client, process it (verify the client handshake hash). */
        status = _nx_secure_tls_process_finished(tls_session, packet_buffer, message_length);
        tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_FINISH_HANDSHAKE;

        /* This is the end of a flight, clear out the transmit queue. */
        _nx_secure_dtls_retransmit_queue_flush(dtls_session);
        break;
    case NX_SECURE_TLS_HELLO_VERIFY_REQUEST:
    case NX_SECURE_TLS_HELLO_REQUEST:
    case NX_SECURE_TLS_SERVER_HELLO:
    case NX_SECURE_TLS_SERVER_KEY_EXCHANGE:
    case NX_SECURE_TLS_CERTIFICATE_REQUEST:
    case NX_SECURE_TLS_SERVER_HELLO_DONE:
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

    /* Hash this handshake message. We do not hash HelloRequest messages, but since only the server will send them,
       we do not worry about them here because these are only messages received from the client at this point.
       Hashes include the handshake layer header but not the record layer header. */
    _nx_secure_tls_handshake_hash_update(tls_session, packet_start, (UINT)(message_length + header_bytes));

    /* Now take any actions based on state set in the message processing. */
    switch (tls_session -> nx_secure_tls_server_state)
    {
    case NX_SECURE_TLS_SERVER_STATE_ALERT_SENT:
        /* This means an error was encountered at some point in processing a valid message. At this point
           the alert was sent, so just return a status indicating as much. */
        return(NX_SECURE_TLS_HANDSHAKE_FAILURE);
        break;
    case NX_SECURE_TLS_SERVER_STATE_SEND_HELLO_VERIFY:
        /* We have received and processed a client hello. Now respond to the client appropriately. */
        status = _nx_secure_dtls_allocate_handshake_packet(dtls_session, packet_pool, &send_packet, wait_option);

        if (status != NX_SUCCESS)
        {
            break;
        }

        _nx_secure_dtls_send_helloverifyrequest(dtls_session, send_packet);
        status = _nx_secure_dtls_send_handshake_record(dtls_session, send_packet, NX_SECURE_TLS_HELLO_VERIFY_REQUEST, wait_option, 1);

        if (status != NX_SUCCESS)
        {
            break;
        }

        if (!dtls_session -> nx_secure_dtls_tls_session.nx_secure_tls_local_session_active)
        {

            /* Ignore the protocol version in a ClientHello without cookies. */
            dtls_session -> nx_secure_dtls_tls_session.nx_secure_tls_protocol_version = 0;
        }

        break;
    case NX_SECURE_TLS_SERVER_STATE_SEND_HELLO:
        /* We have received and processed a client hello. Now respond to the client appropriately. */
        status = _nx_secure_dtls_allocate_handshake_packet(dtls_session, packet_pool, &send_packet, wait_option);

        if (status != NX_SUCCESS)
        {
            break;
        }

        _nx_secure_dtls_send_serverhello(dtls_session, send_packet);
        status = _nx_secure_dtls_send_handshake_record(dtls_session, send_packet, NX_SECURE_TLS_SERVER_HELLO, wait_option, 1);

        if (status != NX_SUCCESS)
        {
            break;
        }

        /* For PSK and ECJPAKE ciphersuites, don't send the certificate message. */
        if (tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm != NX_CRYPTO_KEY_EXCHANGE_PSK &&
            tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm != NX_CRYPTO_KEY_EXCHANGE_ECJPAKE)
        {
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

#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) || defined(NX_SECURE_ENABLE_ECC_CIPHERSUITE)
        if (tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_PSK ||
            tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECJPAKE ||
            tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_cipher -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECDHE)
        {
            /* PSK or ECJPAKE ciphersuites use the ServerKeyExchange message to send cryptographic information. */
            status = _nx_secure_dtls_allocate_handshake_packet(dtls_session, packet_pool, &send_packet, wait_option);

            if (status != NX_SUCCESS)
            {
                break;
            }

            /* This is a PSK or ECJPAKE ciphersuite so we always send a ServerKeyExchange message. */
            status = _nx_secure_tls_send_server_key_exchange(tls_session, send_packet);
            if (status != NX_SUCCESS)
            {
                break;
            }

            status = _nx_secure_dtls_send_handshake_record(dtls_session, send_packet, NX_SECURE_TLS_SERVER_KEY_EXCHANGE, wait_option, 1);
            if (status != NX_SUCCESS)
            {
                break;
            }
        }
#endif

#ifdef NX_SECURE_ENABLE_CLIENT_CERTIFICATE_VERIFY
        /* Application has requested that we request and verify the remote Client certificate. */
        if (tls_session -> nx_secure_tls_verify_client_certificate)
        {
            /* Allocate a packet for our certificate request message.  */
            status = _nx_secure_dtls_allocate_handshake_packet(dtls_session, packet_pool, &send_packet, wait_option);

            if (status != NX_SUCCESS)
            {
                break;
            }

            /* Populate our packet with the desired message (CertificateRequest). */
            status = _nx_secure_tls_send_certificate_request(tls_session, send_packet);
            NX_ASSERT(status == NX_SUCCESS);

            status = _nx_secure_dtls_send_handshake_record(dtls_session, send_packet, NX_SECURE_TLS_CERTIFICATE_REQUEST, wait_option, 1);
            if (status != NX_SUCCESS)
            {
                break;
            }
        }
        else
#endif
        {
            /* Server is not expecting credentials, so indicate that we have received the client's credentials
               to pass Finished processing. */
            tls_session -> nx_secure_tls_received_remote_credentials = NX_TRUE;
        }

        status = _nx_secure_dtls_allocate_handshake_packet(dtls_session, packet_pool, &send_packet, wait_option);

        if (status != NX_SUCCESS)
        {
            break;
        }

        /* Server hello done message is 0 bytes, but it still has a TLS header so don't modify the length here. */
        status = _nx_secure_dtls_send_handshake_record(dtls_session, send_packet, NX_SECURE_TLS_SERVER_HELLO_DONE, wait_option, 1);

        if (status != NX_SUCCESS)
        {
            break;
        }


        tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_HELLO_SENT;

        break;
    case NX_SECURE_TLS_SERVER_STATE_CLIENT_CERTIFICATE:
        /* We processed the certificate above, do nothing. */
        break;
    case NX_SECURE_TLS_SERVER_STATE_CERTIFICATE_VERIFY:
        /* We processed the certificate above, do nothing. */
        break;
    case NX_SECURE_TLS_SERVER_STATE_HELLO_SENT:
        /* We sent a serverhellodone message, but we haven't received the next client message - this
           is likely an error.  */
        break;
    case NX_SECURE_TLS_SERVER_STATE_KEY_EXCHANGE:
        break;
    case NX_SECURE_TLS_SERVER_STATE_FINISH_HANDSHAKE:
        /* Release the protection before suspending on nx_packet_allocate. */
        tx_mutex_put(&_nx_secure_tls_protection);

        /* We have received everything we need to complete the handshake and keys have been
         * generated above. Now end the handshake with a ChangeCipherSpec (indicating following
         * messages are encrypted) and the encrypted Finished message. */

        status = _nx_secure_dtls_packet_allocate(dtls_session, packet_pool, &send_packet, wait_option);

        /* Get the protection after nx_packet_allocate. */
        tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

        if (status != NX_SUCCESS)
        {
            break;
        }

        _nx_secure_tls_send_changecipherspec(tls_session, send_packet);

        /* ChangeCipherSpec is NOT a handshake message, so send as a normal TLS record. */
        status = _nx_secure_dtls_send_record(dtls_session, send_packet, NX_SECURE_TLS_CHANGE_CIPHER_SPEC, wait_option);

        if (status != NX_SUCCESS)
        {
            break;
        }

        /* The local session is now active since we sent the changecipherspec message.
           NOTE: Do not set this flag until after the changecipherspec message has been passed to the send record
           routine - this flag causes encryption and hashing to happen on records. ChangeCipherSpec should be the last
           un-encrypted/un-hashed record sent. */
        tls_session -> nx_secure_tls_local_session_active = 1;

        /* For DTLS, reset sequence number and advance epoch right after CCS message is sent. */
        NX_SECURE_MEMSET(tls_session -> nx_secure_tls_local_sequence_number, 0, sizeof(tls_session -> nx_secure_tls_local_sequence_number));

        status = _nx_secure_tls_session_keys_set(tls_session, NX_SECURE_TLS_KEY_SET_LOCAL);

        if (status != NX_SUCCESS)
        {
            break;
        }

        /* Advance the DTLS epoch - all messages after the ChangeCipherSpec are in a new epoch. */
        dtls_session -> nx_secure_dtls_local_epoch = (USHORT)(dtls_session -> nx_secure_dtls_local_epoch + 1);

        /* We processed the incoming finished message above, so now we can send our own finished message. */
        status = _nx_secure_dtls_allocate_handshake_packet(dtls_session, packet_pool, &send_packet, wait_option);
        if (status != NX_SUCCESS)
        {
            break;
        }

        _nx_secure_tls_send_finished(tls_session, send_packet);
        status = _nx_secure_dtls_send_handshake_record(dtls_session, send_packet, NX_SECURE_TLS_FINISHED, wait_option, 1);


        tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_HANDSHAKE_FINISHED;

        /* Check if application data is received before state change to NX_SECURE_TLS_SERVER_STATE_HANDSHAKE_FINISHED.  */
        if (dtls_session -> nx_secure_dtls_receive_queue_head)
        {

            /* Notify application.  */
            dtls_session -> nx_secure_dtls_server_parent -> nx_secure_dtls_receive_notify(dtls_session);
        }

        break;
    case NX_SECURE_TLS_SERVER_STATE_HANDSHAKE_FINISHED:
        /* Handshake is complete. */
        break;
    default:
        status = NX_SECURE_TLS_INVALID_STATE;
    }


    return(status);
#else /* TLS Server disabled. */

    /* We don't use the parameters since this is an error case. */
    NX_PARAMETER_NOT_USED(packet_buffer);
    NX_PARAMETER_NOT_USED(wait_option);

    /* If TLS Server is disabled and we are in the server state machine, something is wrong... */
    dtls_session -> nx_secure_dtls_tls_session.nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_ERROR;
    return(NX_SECURE_TLS_INVALID_STATE);
#endif
}
#endif /* NX_SECURE_ENABLE_DTLS */

