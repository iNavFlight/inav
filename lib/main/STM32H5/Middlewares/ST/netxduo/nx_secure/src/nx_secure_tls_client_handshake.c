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
/*    _nx_secure_tls_client_handshake                     PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function runs the TLS Client mode state machine. It processes  */
/*    an incoming handshake record and takes appropriate action to        */
/*    advance the TLS Client handshake.                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Pointer into record buffer    */
/*    data_length                           Length of packet buffer       */
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
/*    _nx_secure_tls_generate_premaster_secret                            */
/*                                          Generate premaster secret     */
/*    _nx_secure_tls_handshake_hash_update  Update Finished hash          */
/*    _nx_secure_tls_packet_allocate        Allocate internal TLS packet  */
/*    _nx_secure_tls_process_certificate_request                          */
/*                                          Process certificate request   */
/*    _nx_secure_tls_process_finished       Process Finished message      */
/*    _nx_secure_tls_process_handshake_header                             */
/*                                          Process handshake header      */
/*    _nx_secure_tls_process_remote_certificate                           */
/*                                          Process server certificate    */
/*    _nx_secure_tls_process_server_key_exchange                          */
/*                                          Process ServerKeyExchange     */
/*    _nx_secure_tls_process_serverhello    Process ServerHello           */
/*    _nx_secure_tls_remote_certificate_free_all                          */
/*                                          Free all remote certificates  */
/*    _nx_secure_tls_send_certificate       Send TLS certificate          */
/*    _nx_secure_tls_send_certificate_verify                              */
/*                                          Send certificate verify       */
/*    _nx_secure_tls_send_changecipherspec  Send ChangeCipherSpec         */
/*    _nx_secure_tls_send_client_key_exchange                             */
/*                                          Send ClientKeyExchange        */
/*    _nx_secure_tls_send_clienthello       Send ClientHello              */
/*    _nx_secure_tls_send_finished          Send Finished message         */
/*    _nx_secure_tls_send_handshake_record  Send TLS handshake record     */
/*    _nx_secure_tls_send_record            Send TLS records              */
/*    _nx_secure_tls_session_keys_set       Set session keys              */
/*    nx_secure_tls_packet_release          Release packet                */
/*    [nx_secure_tls_session_renegotiation_callback]                      */
/*                                          Renegotiation callback        */
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
/*                                            fixed renegotiation bug,    */
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
/*  03-02-2021     Timothy Stapko           Modified comment(s),          */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.1.5  */
/*  07-29-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            removed duplicated alert,   */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_client_handshake(NX_SECURE_TLS_SESSION *tls_session, UCHAR *packet_buffer,
                                     UINT data_length, ULONG wait_option)
{
#ifndef NX_SECURE_TLS_CLIENT_DISABLED
UINT            status;
UINT            temp_status;
USHORT          message_type = NX_SECURE_TLS_INVALID_MESSAGE;
UINT            header_bytes;
UINT            message_length;
UINT            packet_buffer_length = data_length;
UCHAR          *packet_start;
NX_PACKET      *send_packet = NX_NULL;
NX_PACKET_POOL *packet_pool;
const NX_CRYPTO_METHOD
               *method_ptr = NX_NULL;

    /* Basic state machine for handshake:
     * 1. We have received a handshake message, now process the header.
     * 2. Then process the message itself and populate the TLS socket structure.
     * 3. Follow up with whatever actions are needed.
     */

    /* Loop through multiple messages in a single record. This can happen if the remote host
       packs multiple handshake messages into a single TLS record. */
    while (data_length > 0)
    {
        /* Save a pointer to the actual packet data (before we do fragment reassembly, etc. below)
         * so we can hash it. */
        packet_start = packet_buffer;

        header_bytes = data_length;

        /* First, process the handshake message to get our state and any data therein. */
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

        /* Allocate a packet for all send operations.  */
        packet_pool = tls_session -> nx_secure_tls_packet_pool;

        /* Hash this handshake message. We do not hash HelloRequest messages.
           Hashes include the handshake layer header but not the record layer header. */
        if (message_type != NX_SECURE_TLS_HELLO_REQUEST && message_type != NX_SECURE_TLS_FINISHED &&
            message_type != NX_SECURE_TLS_HELLO_VERIFY_REQUEST && message_type != NX_SECURE_TLS_SERVER_HELLO)
        {
            _nx_secure_tls_handshake_hash_update(tls_session, packet_start, message_length + header_bytes);
        }

        /* Reduce total length by the size of this message. */
        data_length -= (message_length + header_bytes);

        /* Process the message itself information from the header. */
        status = NX_SECURE_TLS_HANDSHAKE_FAILURE;
        switch (message_type)
        {
        case NX_SECURE_TLS_SERVER_HELLO:
            /* Server has responded to our ClientHello. */
            status = _nx_secure_tls_process_serverhello(tls_session, packet_buffer, message_length);
            break;
        case NX_SECURE_TLS_CERTIFICATE_MSG:
            /* Server has sent its certificate message. */
            status = _nx_secure_tls_process_remote_certificate(tls_session, packet_buffer, message_length, packet_buffer_length);
            break;
        case NX_SECURE_TLS_SERVER_HELLO_DONE:
            /* Server has responded to our ClientHello. */
            /* A ServerHelloDone does not contain any data - it simply changes state. */
            tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_SERVERHELLO_DONE;
            status = NX_SECURE_TLS_SUCCESS;
            break;
        case NX_SECURE_TLS_SERVER_KEY_EXCHANGE:
            /* Server has sent a key exchange message, used for certain ciphersuites (DH and PSK mainly). */
            status = _nx_secure_tls_process_server_key_exchange(tls_session, packet_buffer, message_length);
            break;
        case NX_SECURE_TLS_CERTIFICATE_REQUEST:
            /* Server has requested we provide a client certificate. */
            status = _nx_secure_tls_process_certificate_request(tls_session, packet_buffer, message_length);
            break;
        case NX_SECURE_TLS_FINISHED:
            /* Final handshake message from the server, process it (verify the server handshake hash). */
            status = _nx_secure_tls_process_finished(tls_session, packet_buffer, message_length);

            /* For client, cleanup hash handler after received the finished message from server. */
            /* NOTE: we want to run all of the nx_crypto_cleanup calls regardless of the status of the finished processing above
                     so use a secondary status to track their return status values. */
#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
            method_ptr = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_handshake_hash_sha256_method;

            /* NOTE: we want to run all of the nx_crypto_cleanup calls regardless of the  */
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

            break;
        case NX_SECURE_TLS_HELLO_REQUEST:
            /* Server has requested we restart the session. If we are in the middle of a handshake already
             * (session is not active) then ignore. If we are in an active session, we can choose to
             * send a ClientHello (start the handshake again) or send a no_renegotiation alert. */
            if (tls_session -> nx_secure_tls_local_session_active)
            {
#ifndef NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION
                /* A HelloRequest does not contain any data - it simply changes state. */
                tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_HELLO_REQUEST;

                if (tls_session -> nx_secure_tls_renegotation_enabled && tls_session -> nx_secure_tls_secure_renegotiation)
                {
                    tls_session -> nx_secure_tls_renegotiation_handshake = NX_TRUE;

                    /* On a session resumption free all certificates for the new session.
                     * SESSION RESUMPTION: if session resumption is enabled, don't free!!
                     */
                    status = _nx_secure_tls_remote_certificate_free_all(tls_session);

                    if (status != NX_SUCCESS)
                    {
                        return(status);
                    }
                }
                else
#endif /* NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION */
                {
                    /* Session renegotiation is disabled, so this is an error! */
                    return(NX_SECURE_TLS_NO_RENEGOTIATION_ERROR);
                }
            }

            break;
        case NX_SECURE_TLS_HELLO_VERIFY_REQUEST:  /* DTLS ONLY! */
        case NX_SECURE_TLS_CERTIFICATE_VERIFY:
        case NX_SECURE_TLS_CLIENT_KEY_EXCHANGE:
        case NX_SECURE_TLS_CLIENT_HELLO:
        case NX_SECURE_TLS_INVALID_MESSAGE:
        case NX_SECURE_TLS_CERTIFICATE_URL:
        case NX_SECURE_TLS_CERTIFICATE_STATUS:
        default:
            /* The message received was not a valid TLS server handshake message, send alert and return. */
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
        case NX_SECURE_TLS_CLIENT_STATE_RENEGOTIATING:
            /* Client isn't doing anything right now. */
            break;
        case NX_SECURE_TLS_CLIENT_STATE_ERROR:
        case NX_SECURE_TLS_CLIENT_STATE_ALERT_SENT:
            /* This means an error was encountered at some point in processing a valid message. At this point
               the alert was sent, so just return a status indicating as much. */
            return(NX_SECURE_TLS_HANDSHAKE_FAILURE);
#ifndef NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION
        case NX_SECURE_TLS_CLIENT_STATE_HELLO_REQUEST:
            /* Server sent a hello request, indicating it wants to restart the handshake process with a new ClientHello. */
            if (tls_session -> nx_secure_tls_local_session_active)
            {
                /* See if renegotiation is enabled. */
                if (tls_session -> nx_secure_tls_renegotation_enabled && tls_session -> nx_secure_tls_secure_renegotiation)
                {
                    /* Invoke user callback to notify application of renegotiation request. */
                    if (tls_session -> nx_secure_tls_session_renegotiation_callback != NX_NULL)
                    {
                        status = tls_session -> nx_secure_tls_session_renegotiation_callback(tls_session);

                        if (status != NX_SUCCESS)
                        {
                            return(status);
                        }
                    }

                    /* If we are currently in a session, we have a renegotiation handshake. */
                    tls_session -> nx_secure_tls_renegotiation_handshake = NX_TRUE;

                    /* Allocate a handshake packet so we can send the ClientHello. */
                    status = _nx_secure_tls_allocate_handshake_packet(tls_session, tls_session -> nx_secure_tls_packet_pool, &send_packet, wait_option);

                    if (status != NX_SUCCESS)
                    {
                        return(status);
                    }

                    /* Populate our packet with clienthello data. */
                    status = _nx_secure_tls_send_clienthello(tls_session, send_packet);

                    if (status != NX_SUCCESS)
                    {
                        return(status);
                    }

                    /* Send the ClientHello to kick things off. */
                    status = _nx_secure_tls_send_handshake_record(tls_session, send_packet, NX_SECURE_TLS_CLIENT_HELLO, wait_option);
                }
                else
                {
                    /* Respond to the HelloRequest with a "no_renegotiation" alert since we don't want to restart the handshake. */
                    status = NX_SECURE_TLS_NO_RENEGOTIATION_ERROR;
                }
            }
            /* If we are still in a handshake (session is not active) then ignore the message. */
            break;
#endif /* NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION */
        case NX_SECURE_TLS_CLIENT_STATE_SERVERHELLO:
            /* We received a serverhello above. It is time to update the hash for the handshake. */

            if(tls_session->nx_secure_tls_key_material.nx_secure_tls_handshake_cache_length > 0)
            {
                /* We have some cached messages from earlier in the handshake that we need to process. Generally
                   this will just be the ClientHello. */
                status = _nx_secure_tls_handshake_hash_update(tls_session, tls_session->nx_secure_tls_key_material.nx_secure_tls_handshake_cache,
                                                              tls_session->nx_secure_tls_key_material.nx_secure_tls_handshake_cache_length);
                if(status != NX_SUCCESS)
                {
                    return(status);
                }

                /* Indicate that all cached messages have been hashed. */
                tls_session->nx_secure_tls_key_material.nx_secure_tls_handshake_cache_length = 0;

                _nx_secure_tls_handshake_hash_update(tls_session, packet_start, message_length + header_bytes);
            }
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
                status = _nx_secure_tls_allocate_handshake_packet(tls_session, packet_pool, &send_packet, wait_option);

                if (status != NX_SUCCESS)
                {
                    break;
                }

                status = _nx_secure_tls_send_certificate(tls_session, send_packet, wait_option);

                if (status != NX_SUCCESS)
                {
                    break;
                }

                status = _nx_secure_tls_send_handshake_record(tls_session, send_packet, NX_SECURE_TLS_CERTIFICATE_MSG, wait_option);

                if (status != NX_SUCCESS)
                {
                    break;
                }
            }

            /* Now, generate the pre-master secret that is used to generate keys for our session. */
            status = _nx_secure_tls_generate_premaster_secret(tls_session, NX_SECURE_TLS);
            if (status != NX_SUCCESS)
            {
                break;
            }

            /* We have received and processed a ServerHelloDone. Now respond to the client appropriately. */
            status = _nx_secure_tls_allocate_handshake_packet(tls_session, packet_pool, &send_packet, wait_option);

            if (status != NX_SUCCESS)
            {
                break;
            }

            /* Generate and send the ClientKeyExchange message. */
            status = _nx_secure_tls_send_client_key_exchange(tls_session, send_packet);

            if (status != NX_SUCCESS)
            {
                break;
            }

            status = _nx_secure_tls_send_handshake_record(tls_session, send_packet, NX_SECURE_TLS_CLIENT_KEY_EXCHANGE, wait_option);

            if (status != NX_SUCCESS)
            {
                break;
            }

            /* After sending ClientKeyExchange, we need to send a CertificateVerify message if the
               server has requested a certificate. If no certificate is available, this flag will
               be cleared after the empty certificate message is sent. */
            if (tls_session -> nx_secure_tls_client_certificate_requested)
            {
                /* We can now clear the flag since this is the last specific certificate message sent. */
                tls_session -> nx_secure_tls_client_certificate_requested = 0;

                /* Allocate packet for CertificateVerify. */
                status = _nx_secure_tls_allocate_handshake_packet(tls_session, packet_pool, &send_packet, wait_option);

                if (status != NX_SUCCESS)
                {
                    break;
                }

                status = _nx_secure_tls_send_certificate_verify(tls_session, send_packet);

                if (status != NX_SUCCESS)
                {
                    break;
                }

                status = _nx_secure_tls_send_handshake_record(tls_session, send_packet, NX_SECURE_TLS_CERTIFICATE_VERIFY, wait_option);

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
            status = _nx_secure_tls_packet_allocate(tls_session, packet_pool, &send_packet, wait_option);

            /* Get the protection after nx_packet_allocate. */
            tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

            if (status != NX_SUCCESS)
            {
                break;
            }

            /* ChangeCipherSpec is NOT a handshake message, so send as a normal TLS record. */
            _nx_secure_tls_send_changecipherspec(tls_session, send_packet);

            status = _nx_secure_tls_send_record(tls_session, send_packet, NX_SECURE_TLS_CHANGE_CIPHER_SPEC, wait_option);

            if (status != NX_SUCCESS)
            {
                /* Release packet on send error. */
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

            /* Set our local session keys since we are sent a CCS message. */
            _nx_secure_tls_session_keys_set(tls_session, NX_SECURE_TLS_KEY_SET_LOCAL);

            /* We can now send our finished message, which will be encrypted using the chosen ciphersuite. */
            status = _nx_secure_tls_allocate_handshake_packet(tls_session, packet_pool, &send_packet, wait_option);

            if (status != NX_SUCCESS)
            {
                break;
            }

            /* Generate and send the finished message, which completes the handshake. */
            _nx_secure_tls_send_finished(tls_session, send_packet);

            status = _nx_secure_tls_send_handshake_record(tls_session, send_packet, NX_SECURE_TLS_FINISHED, wait_option);

            break;
        case NX_SECURE_TLS_CLIENT_STATE_HANDSHAKE_FINISHED:
            /* We processed a server finished message, completing the handshake. Verify all is good and if so,
               continue to the encrypted session. */
            break;
        case NX_SECURE_TLS_CLIENT_STATE_HELLO_VERIFY: /* DTLS ONLY! */
        default:
            status = NX_SECURE_TLS_INVALID_STATE;
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
#else /* TLS Client disabled. */

    /* We don't use the parameters since this is an error case. */
    NX_PARAMETER_NOT_USED(packet_buffer);
    NX_PARAMETER_NOT_USED(wait_option);
    NX_PARAMETER_NOT_USED(data_length);

    /* If TLS Client is disabled and we are in the client state machine, something is wrong... */
    tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_ERROR;
    return(NX_SECURE_TLS_INVALID_STATE);
#endif
}

