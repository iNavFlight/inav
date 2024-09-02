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
/*    _nx_secure_tls_server_handshake                     PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function runs the TLS Server mode state machine. It processes  */
/*    an incoming handshake record and takes appropriate action to        */
/*    advance the TLS Server handshake.                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Pointer into record buffer    */
/*    wait_option                           Controls timeout actions      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_allocate_handshake_packet                            */
/*                                          Allocate TLS packet           */
/*    _nx_secure_tls_generate_keys          Generate session keys         */
/*    _nx_secure_tls_handshake_hash_init    Initialize Finished hash      */
/*    _nx_secure_tls_handshake_hash_update  Update Finished hash          */
/*    _nx_secure_tls_packet_allocate        Allocate internal TLS packet  */
/*    _nx_secure_tls_process_client_key_exchange                          */
/*                                          Process ClientKeyExchange     */
/*    _nx_secure_tls_process_clienthello    Process ClientHello           */
/*    _nx_secure_tls_process_finished       Process Finished message      */
/*    _nx_secure_tls_process_handshake_header                             */
/*                                          Process handshake header      */
/*    _nx_secure_tls_process_remote_certificate                           */
/*                                          Process server certificate    */
/*    _nx_secure_tls_send_certificate       Send TLS certificate          */
/*    _nx_secure_tls_send_certificate_request                             */
/*                                          Send TLS CertificateRequest   */
/*    _nx_secure_tls_send_changecipherspec  Send ChangeCipherSpec         */
/*    _nx_secure_tls_send_finished          Send Finished message         */
/*    _nx_secure_tls_send_handshake_record  Send TLS handshake record     */
/*    _nx_secure_tls_send_record            Send TLS records              */
/*    _nx_secure_tls_send_server_key_exchange                             */
/*                                          Send ServerKeyExchange        */
/*    _nx_secure_tls_send_serverhello       Send TLS ServerHello          */
/*    _nx_secure_tls_session_keys_set       Set session keys              */
/*    nx_secure_tls_packet_release          Release packet                */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_process_record         Process TLS record data       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            released packet securely,   */
/*                                            fixed certificate buffer    */
/*                                            allocation,                 */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Timothy Stapko           Modified comment(s),          */
/*                                            improved buffer length      */
/*                                            verification,               */
/*                                            resulting in version 6.1.3  */
/*  02-02-2021     Timothy Stapko           Modified comment(s), added    */
/*                                            support for fragmented TLS  */
/*                                            Handshake messages,         */
/*                                            resulting in version 6.1.4  */
/*  07-29-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            removed duplicated alert,   */
/*                                            resulting in version 6.1.12 */
/*  10-31-2022     Yanwu Cai                Modified comment(s),          */
/*                                            fixed handling of multiple  */
/*                                            handshake messages,         */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_server_handshake(NX_SECURE_TLS_SESSION *tls_session, UCHAR *packet_buffer,
                                     UINT data_length, ULONG wait_option)
{
#ifndef NX_SECURE_TLS_SERVER_DISABLED
UINT                                  status;
UINT                                  temp_status;
USHORT                                message_type;
UINT                                  header_bytes;
UINT                                  message_length;
NX_PACKET                            *send_packet;
NX_PACKET_POOL                       *packet_pool;
UCHAR                                *packet_start;
const NX_CRYPTO_METHOD               *method_ptr = NX_NULL;

    /* Basic state machine for handshake:
     * 1. We have received a handshake message, now process the header.
     * 2. Then process the message itself and populate the TLS socket structure.
     * 3. Follow up with whatever actions are needed.
     */

    /* Loop through multiple messages in a single record. This can happen if the remote host
       packs multiple handshake messages into a single TLS record. */
    while (data_length > 0)
    {

        /* Save a pointer to the start of our packet for the hash that happens below. */
        packet_start = packet_buffer;

        header_bytes = data_length;

        status = _nx_secure_tls_process_handshake_header(packet_buffer, &message_type, &header_bytes, &message_length);

        if (status != NX_SECURE_TLS_SUCCESS)
        {
            return(status);
        }

        /* Check for fragmented message. */
        if((message_length + header_bytes) > data_length)
        {
            /* Incomplete message! A single message is fragmented across several records. We need to obtain the next fragment. */
            tls_session -> nx_secure_tls_handshake_record_expected_length = message_length + header_bytes;

            tls_session -> nx_secure_tls_handshake_record_fragment_state = NX_SECURE_TLS_HANDSHAKE_RECEIVED_FRAGMENT;

            return(NX_SECURE_TLS_HANDSHAKE_FRAGMENT_RECEIVED);
        }

        /* Advance the buffer pointer past the handshake header. */
        packet_buffer += header_bytes;

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

        /* Reduce total length by the size of this message. */
        data_length -= (message_length + header_bytes);

        /* Process the message itself information from the header. */
        status = NX_SECURE_TLS_SUCCESS;
        switch (message_type)
        {
        case NX_SECURE_TLS_CLIENT_HELLO:
            /* Client is establishing a TLS session with our server. */
            status = _nx_secure_tls_process_clienthello(tls_session, packet_buffer, message_length);
            tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_SEND_HELLO;
            break;
#ifdef NX_SECURE_ENABLE_CLIENT_CERTIFICATE_VERIFY
        case NX_SECURE_TLS_CERTIFICATE_MSG:
            /* Client sent certificate message (in response to a request from us. Process it now. */
            status = _nx_secure_tls_process_remote_certificate(tls_session, packet_buffer, message_length, data_length);
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
            status = _nx_secure_tls_process_client_key_exchange(tls_session, packet_buffer, message_length, NX_SECURE_TLS);
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
            /* If we encountered an error in message processing set the state to the error condition. */
            tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_ERROR;
        }
        else
        {
            /* Hash this handshake message. We do not hash HelloRequest messages, but since only the server will send them,
               we do not worry about them here because these are only messages received from the client at this point.
               Hashes include the handshake layer header but not the record layer header. */
            _nx_secure_tls_handshake_hash_update(tls_session, packet_start, (UINT)(message_length + header_bytes));
        }

        /* Now take any actions based on state set in the message processing. */
        switch (tls_session -> nx_secure_tls_server_state)
        {
        case NX_SECURE_TLS_SERVER_STATE_SEND_HELLO:
            /* We have received and processed a client hello. Now respond to the client appropriately. */
            status = _nx_secure_tls_allocate_handshake_packet(tls_session, packet_pool, &send_packet, wait_option);

            if (status != NX_SUCCESS)
            {
                break;
            }

            status = _nx_secure_tls_send_serverhello(tls_session, send_packet);

            if (status != NX_SUCCESS)
            {
                break;
            }

            status = _nx_secure_tls_send_handshake_record(tls_session, send_packet, NX_SECURE_TLS_SERVER_HELLO, wait_option);

            if (status != NX_SUCCESS)
            {
                break;
            }

            NX_ASSERT(tls_session -> nx_secure_tls_session_ciphersuite != NX_NULL);

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
            /* For PSK ciphersuites, don't send the certificate message. */
            if (tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm != NX_CRYPTO_KEY_EXCHANGE_PSK)
            {
#endif /* NX_SECURE_ENABLE_PSK_CIPHERSUITES */
                status = _nx_secure_tls_allocate_handshake_packet(tls_session, packet_pool, &send_packet, wait_option);

                if (status != NX_SUCCESS)
                {
                    break;
                }

                _nx_secure_tls_send_certificate(tls_session, send_packet, wait_option);
                status = _nx_secure_tls_send_handshake_record(tls_session, send_packet, NX_SECURE_TLS_CERTIFICATE_MSG, wait_option);
                if (status != NX_SUCCESS)
                {
                    break;
                }

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
            }
#endif /* NX_SECURE_ENABLE_PSK_CIPHERSUITES */

#if defined(NX_SECURE_ENABLE_ECC_CIPHERSUITE) || defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES)
            if (NX_FALSE
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
                || tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_cipher -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECDHE
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
                || tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_PSK
#endif /* NX_SECURE_ENABLE_PSK_CIPHERSUITES */
                )
            {
                /* PSK and ECDHE ciphersuites use the ServerKeyExchange message to send cryptographic information. */
                status = _nx_secure_tls_allocate_handshake_packet(tls_session, packet_pool, &send_packet, wait_option);

                if (status != NX_SUCCESS)
                {
                    break;
                }

                /* This is a PSK ciphersuite so we always send a ServerKeyExchange message. */
                status = _nx_secure_tls_send_server_key_exchange(tls_session, send_packet);
                if (status != NX_SUCCESS)
                {
                    break;
                }

                status = _nx_secure_tls_send_handshake_record(tls_session, send_packet, NX_SECURE_TLS_SERVER_KEY_EXCHANGE, wait_option);
                if (status != NX_SUCCESS)
                {
                    break;
                }
            }
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE || NX_CRYPTO_KEY_EXCHANGE_PSK */

#ifdef NX_SECURE_ENABLE_CLIENT_CERTIFICATE_VERIFY
            /* Application has requested that we request and verify the remote Client certificate. */
            if (tls_session -> nx_secure_tls_verify_client_certificate)
            {
                /* Allocate a packet for our certificate request message.  */
                status = _nx_secure_tls_allocate_handshake_packet(tls_session, packet_pool, &send_packet, wait_option);

                if (status != NX_SUCCESS)
                {
                    break;
                }

                /* Populate our packet with the desired message (CertificateRequest). */
                status = _nx_secure_tls_send_certificate_request(tls_session, send_packet);
                NX_ASSERT(status == NX_SUCCESS);

                status = _nx_secure_tls_send_handshake_record(tls_session, send_packet, NX_SECURE_TLS_CERTIFICATE_REQUEST, wait_option);
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
            /* Allocate a new packet for the ServerHelloDone. */
            status = _nx_secure_tls_allocate_handshake_packet(tls_session, packet_pool, &send_packet, wait_option);

            if (status != NX_SUCCESS)
            {
                break;
            }

            /* Server hello done message is 0 bytes, but it still has a TLS header so don't modify the length here. */
            status = _nx_secure_tls_send_handshake_record(tls_session, send_packet, NX_SECURE_TLS_SERVER_HELLO_DONE, wait_option);

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
        case NX_SECURE_TLS_SERVER_STATE_KEY_EXCHANGE:
            break;
        case NX_SECURE_TLS_SERVER_STATE_FINISH_HANDSHAKE:

            /* Release the protection before suspending on nx_packet_allocate. */
            tx_mutex_put(&_nx_secure_tls_protection);

            /* We have received everything we need to complete the handshake and keys have been
             * generated above. Now end the handshake with a ChangeCipherSpec (indicating following
             * messages are encrypted) and the encrypted Finished message. */

            status = _nx_secure_tls_packet_allocate(tls_session, packet_pool, &send_packet, wait_option);

            /* Get the protection after nx_packet_allocate. */
            tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

            if (status != NX_SUCCESS)
            {
                break;
            }

            _nx_secure_tls_send_changecipherspec(tls_session, send_packet);

            /* ChangeCipherSpec is NOT a handshake message, so send as a normal TLS record. */
            status = _nx_secure_tls_send_record(tls_session, send_packet, NX_SECURE_TLS_CHANGE_CIPHER_SPEC, wait_option);

            if (status != NX_SUCCESS)
            {
                nx_secure_tls_packet_release(send_packet);
                break;
            }

            /* Reset the sequence number now that we are starting a new session. */
            NX_SECURE_MEMSET(tls_session -> nx_secure_tls_local_sequence_number, 0, sizeof(tls_session -> nx_secure_tls_local_sequence_number));

            /* The local session is now active since we sent the changecipherspec message.
               NOTE: Do not set the keys until after the changecipherspec message has been passed to the send record
               routine - this call causes encryption and hashing to happen on records. ChangeCipherSpec should be the last
               un-encrypted/un-hashed record sent. For a renegotiation handshake, CCS is the last message encrypted using
               the original session keys. */

            /* Set our local session keys since we sent a CCS message. */
            status = _nx_secure_tls_session_keys_set(tls_session, NX_SECURE_TLS_KEY_SET_LOCAL);

            if (status != NX_SUCCESS)
            {
                break;
            }

            /* We processed the incoming finished message above, so now we can send our own finished message. */
            status = _nx_secure_tls_allocate_handshake_packet(tls_session, packet_pool, &send_packet, wait_option);
            if (status != NX_SUCCESS)
            {
                break;
            }

            _nx_secure_tls_send_finished(tls_session, send_packet);
            status = _nx_secure_tls_send_handshake_record(tls_session, send_packet, NX_SECURE_TLS_FINISHED, wait_option);

            /* For server, cleanup hash handler after sent the finished message to server. */
#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
            method_ptr = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_handshake_hash_sha256_method;

            if (method_ptr -> nx_crypto_cleanup != NX_NULL)
            {
                temp_status = method_ptr -> nx_crypto_cleanup(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata);
                if(temp_status != NX_CRYPTO_SUCCESS)
                {
                    status = temp_status;
                }

            }
#endif /* (NX_SECURE_TLS_TLS_1_2_ENABLED) */

#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
            method_ptr = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_handshake_hash_md5_method;
            if (method_ptr != NX_NULL && method_ptr -> nx_crypto_cleanup != NX_NULL)
            {
                temp_status = method_ptr -> nx_crypto_cleanup(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_md5_metadata);
                if(temp_status != NX_CRYPTO_SUCCESS)
                {
                    status = temp_status;
                }

            }

            method_ptr = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_handshake_hash_sha1_method;
            if (method_ptr != NX_NULL && method_ptr -> nx_crypto_cleanup != NX_NULL)
            {
                temp_status = method_ptr -> nx_crypto_cleanup(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha1_metadata);
                if(temp_status != NX_CRYPTO_SUCCESS)
                {
                    status = temp_status;
                }

            }
#endif /* (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED) */

            tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_HANDSHAKE_FINISHED;

            break;
        default: /* NX_SECURE_TLS_SERVER_STATE_ERROR */
            /* Default is to break out of the switch because either we encountered an error or
               we are in an invalid state. DO NOT change the value of "status" here because
               we need its value in the alert processing below. NOTE: we should never
               get to this branch with status == NX_SUCCESS because the state will only
               be set to NX_SECURE_TLS_SERVER_STATE_ERROR if status indicates an error. */
            NX_ASSERT(status != NX_SUCCESS);
            break;
        }
        
        /* If we have an error at this point, we have experienced a problem in sending
           handshake messages, which is some type of internal issue. */
        if (status != NX_SUCCESS)
        {

            return(status);
        }
        
        /* Advance the buffer pointer past the message. */
        packet_buffer += message_length;
    } /* End while. */

    return(NX_SUCCESS);
#else /* TLS Server disabled. */

    /* We don't use the parameters since this is an error case. */
    NX_PARAMETER_NOT_USED(packet_buffer);
    NX_PARAMETER_NOT_USED(wait_option);
    NX_PARAMETER_NOT_USED(data_length);

    /* If TLS Server is disabled and we are in the server state machine, something is wrong... */
    tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_ERROR;
    return(NX_SECURE_TLS_INVALID_STATE);
#endif
}

