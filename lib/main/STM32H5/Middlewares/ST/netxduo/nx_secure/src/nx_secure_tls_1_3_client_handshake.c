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
/*    _nx_secure_tls_1_3_client_handshake                 PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function runs the TLS Client mode state machine for TLSv1.3.   */
/*    It processes an incoming handshake record and takes appropriate     */
/*    action to advance the TLS Client handshake. The TLSv1.3 handshake   */
/*    state machine differs fairly significantly from the earlier TLS     */
/*    versions so we have split the 1.3 handshake into its own function.  */
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
/*    _nx_secure_tls_map_error_to_alert     Map internal error to alert   */
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
/*    _nx_secure_tls_send_alert             Send TLS alert                */
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
/*  07-29-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            updated alert message for   */
/*                                            downgrade protection,       */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/

/* State transition table for TLS 1.3 Client. *

 Current state                  Processing Function         Valid next states (SendAlert is a next state for all others)
 ------------------------------------------------------------------------------------------------
 ClientHelloSent                process_clienthello         ServerHelloReceived
 ServerHelloReceived            process_serverhello         EncryptedExtensionsReceived
 EncryptedExtensionsReceived    process_server_extensions   CertificateRequestRx, CertificateRx,
                                                            CertificateVerifyRx, FinishedRx
 CertificateRequestRx           process_cert_request        CertificateRx, FinishedRx,
 CertificateRx                  process_certificate         CertificateVerifyRx, FinishedRx
 CertificateVerifyRx            process_certificate_verify  FinishedRx
 FinishedRx                     process_finished            SendCert, SendCertVerify, SendFinished
 SendCert                       send_certificate            SendCertVerify
 SendCertVerify                 send_certificate_verify     SendFinished
 SendFinished                   send_finished               ApplicationData
*/


#if (NX_SECURE_TLS_TLS_1_3_ENABLED)

/* Defined in nx_secure_tls_send_serverhello.c */
extern const UCHAR _nx_secure_tls_1_2_random[8];
extern const UCHAR _nx_secure_tls_1_1_random[8];

UINT _nx_secure_tls_1_3_client_handshake(NX_SECURE_TLS_SESSION *tls_session, UCHAR *packet_buffer,
                                         UINT data_length, ULONG wait_option)
{
#ifndef NX_SECURE_TLS_CLIENT_DISABLED
UINT            status;
USHORT          message_type = NX_SECURE_TLS_INVALID_MESSAGE;
UINT            header_bytes;
UINT            message_length;
UINT            packet_buffer_length = data_length;
UCHAR          *packet_start;
NX_PACKET      *send_packet = NX_NULL;
NX_PACKET_POOL *packet_pool;
const NX_CRYPTO_METHOD
               *method_ptr = NX_NULL;
const UCHAR    *server_random;

    /* Basic state machine for handshake:
     * 1. We have received a handshake message, now process the header.
     * 2. Then process the message itself and populate the TLS socket structure.
     * 3. Follow up with whatever actions are needed.
     */

    /* TLS 1.3 handshake state machine (TLS 1.3 RFC draft 28):

        Key  ^ ClientHello
        Exch | + key_share*
             | + signature_algorithms*
             | + psk_key_exchange_modes*
             v + pre_shared_key*         -------->
                                                               ServerHello  ^ Key
                                                              + key_share*  | Exch
                                                         + pre_shared_key*  v
                                                     {EncryptedExtensions}  ^  Server
                                                     {CertificateRequest*}  v  Params
                                                            {Certificate*}  ^
                                                      {CertificateVerify*}  | Auth
                                                                {Finished}  v
                                         <--------     [Application Data*]
             ^ {Certificate*}
        Auth | {CertificateVerify*}
             v {Finished}                -------->
               [Application Data]        <------->      [Application Data]

              +  Indicates noteworthy extensions sent in the
                 previously noted message.

              *  Indicates optional or situation-dependent
                 messages/extensions that are not always sent.

              {} Indicates messages protected using keys
                 derived from a [sender]_handshake_traffic_secret.

              [] Indicates messages protected using keys
                 derived from [sender]_application_traffic_secret_N

               Figure 1: Message flow for full TLS Handshake

       -  Key Exchange: Establish shared keying material and select the
          cryptographic parameters.  Everything after this phase is
          encrypted.

       -  Server Parameters: Establish other handshake parameters (whether
          the client is authenticated, application layer protocol support,
          etc.).

       -  Authentication: Authenticate the server (and optionally the
          client) and provide key confirmation and handshake integrity.
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
        if (tls_session->nx_secure_tls_local_session_active && message_type != NX_SECURE_TLS_HELLO_REQUEST && message_type != NX_SECURE_TLS_HELLO_VERIFY_REQUEST
            && message_type != NX_SECURE_TLS_FINISHED)
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

            /* RFC 8446 4.1.3:
               TLS 1.3 clients receiving a ServerHello indicating TLS 1.2 or below MUST check that 
               the last 8 bytes are not equal to either of these values. */
            if ((status == NX_SUCCESS) && (tls_session -> nx_secure_tls_1_3 == NX_FALSE))
            {
                if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2)
                {
                    server_random = _nx_secure_tls_1_2_random;
                }
                else
                {
                    server_random = _nx_secure_tls_1_1_random;
                }

                if (NX_SECURE_MEMCMP(&(tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random[24]),
                                     server_random, 8) == 0)
                {
                    status = NX_SECURE_TLS_DOWNGRADE_DETECTED;
                }
                else
                {

                    /* Server negotiates a version of TLS prior to TLS 1.3. */
                    return(NX_SUCCESS);
                }
            }

            /* In TLS 1.3, we generate keys now because all further messages will be encrypted (see state machine below). */

            break;
        case NX_SECURE_TLS_ENCRYPTED_EXTENSIONS:
            /* RFC 8446, section 4.3.1, page 59.
             * The server MUST send this message immediately after the ServerHello message. */
            if (tls_session -> nx_secure_tls_client_state != NX_SECURE_TLS_CLIENT_STATE_SERVERHELLO)
            {
                break;
            }

            status = _nx_secure_tls_process_encrypted_extensions(tls_session, packet_buffer, message_length);
            break;
        case NX_SECURE_TLS_CERTIFICATE_MSG:
            /* Server has sent its certificate message. */
            status = _nx_secure_tls_process_remote_certificate(tls_session, packet_buffer, message_length, packet_buffer_length);

            if (status == NX_SUCCESS)
            {
                /* Save the transcript hash up to the certificate for the CertificateVerify (next message to send). */
                status = _nx_secure_tls_1_3_transcript_hash_save(tls_session, NX_SECURE_TLS_TRANSCRIPT_IDX_CERTIFICATE, NX_TRUE);
            }
            else if (status == NX_SECURE_TLS_EMPTY_REMOTE_CERTIFICATE_RECEIVED)
            {
                /* If server supplies an empty Certificate message, the client MUST abort the handshake with a "decode_error" alert. */
                status = NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH;
            }
            break;
        case NX_SECURE_TLS_CERTIFICATE_REQUEST:
            /* RFC 8446, section 4.3.2, page 59.
             * If sent, MUST follow EncryptedExtensions. */
            if (tls_session -> nx_secure_tls_client_state != NX_SECURE_TLS_CLIENT_STATE_ENCRYPTED_EXTENSIONS)
            {
                break;
            }

            /* Server has requested we provide a client certificate. */
            status = _nx_secure_tls_process_certificate_request(tls_session, packet_buffer, message_length);
            break;
        case NX_SECURE_TLS_FINISHED:

            /* Save the transcript hash to this point for processing Finished. */
            status = _nx_secure_tls_1_3_transcript_hash_save(tls_session, NX_SECURE_TLS_TRANSCRIPT_IDX_SERVER_FINISHED, NX_TRUE);
            if (status != NX_SUCCESS)
            {
                break;
            }

            /* Final handshake message from the server, process it (verify the server handshake hash). */
            status = _nx_secure_tls_process_finished(tls_session, packet_buffer, message_length);
            if (status != NX_SUCCESS)
            {
                break;
            }

            /* Update the transcript hash with the Finished.  */
            _nx_secure_tls_handshake_hash_update(tls_session, packet_start, message_length + header_bytes);

            /* For client, cleanup hash handler after received the finished message from server. */
            method_ptr = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_handshake_hash_sha256_method;
            if (method_ptr -> nx_crypto_cleanup != NX_NULL)
            {
                status = method_ptr -> nx_crypto_cleanup(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata);
            }
            break;
        case NX_SECURE_TLS_CERTIFICATE_VERIFY:
            /* Handle server-sent certificate verify. */
            status = _nx_secure_tls_process_certificate_verify(tls_session, packet_buffer, message_length);
            break;
        case NX_SECURE_TLS_NEW_SESSION_TICKET:
            /* TLS 1.3 post-handshake message. */
            status = NX_SUCCESS;
            break;
/*      case NX_SECURE_TLS_SERVER_KEY_EXCHANGE: // TLS 1.2 and earlier ONLY!
        case NX_SECURE_TLS_SERVER_HELLO_DONE: // TLS 1.2 and earlier ONLY!
        case NX_SECURE_TLS_HELLO_REQUEST: // TLS 1.2 and earlier ONLY!
        case NX_SECURE_TLS_HELLO_VERIFY_REQUEST: // DTLS ONLY!
        case NX_SECURE_TLS_CLIENT_KEY_EXCHANGE:
        case NX_SECURE_TLS_CLIENT_HELLO:
        case NX_SECURE_TLS_INVALID_MESSAGE:
        case NX_SECURE_TLS_CERTIFICATE_URL:
        case NX_SECURE_TLS_CERTIFICATE_STATUS:
 */
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
            break;
        case NX_SECURE_TLS_CLIENT_STATE_HELLO_RETRY:
            if(tls_session->nx_secure_tls_key_material.nx_secure_tls_handshake_cache_length > 0)
            {
                /* We have some cached messages from earlier in the handshake that we need to process. Generally
                   this will just be the ClientHello in TLS 1.3. */
                status = _nx_secure_tls_handshake_hash_update(tls_session, tls_session->nx_secure_tls_key_material.nx_secure_tls_handshake_cache,
                                                              tls_session->nx_secure_tls_key_material.nx_secure_tls_handshake_cache_length);
                if(status != NX_SUCCESS)
                {
                    break;
                }

                /* Indicate that all cached messages have been hashed. */
                tls_session->nx_secure_tls_key_material.nx_secure_tls_handshake_cache_length = 0;
            }

            /* Update the transcript hash with the ServerHelloRetry. */
            _nx_secure_tls_handshake_hash_update(tls_session, packet_start, message_length + header_bytes);

            /* Allocate a handshake packet so we can send the ClientHello. */
            status = _nx_secure_tls_allocate_handshake_packet(tls_session, tls_session -> nx_secure_tls_packet_pool, &send_packet, wait_option);

            if (status != NX_SUCCESS)
            {
                return(status);
            }

            /* Populate our packet with clienthello data. */
            status = _nx_secure_tls_send_clienthello(tls_session, send_packet);

            if (status == NX_SUCCESS)
            {

                /* To avoid a hash update on the second ClientHello packet switch to idle state (pre-handshake). */
                tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_IDLE;

                /* Send the ClientHello. */
                status = _nx_secure_tls_send_handshake_record(tls_session, send_packet, NX_SECURE_TLS_CLIENT_HELLO, wait_option);

                /* This state is used to avoid processing a second HelloRetryRequest packet if one is sent in error. */
                tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_HELLO_RETRY;
            }

            /* If anything after the allocate fails, we need to release our packet. */
            if (status != NX_SUCCESS)
            {

                /* Release the protection. */
                nx_secure_tls_packet_release(send_packet);
                return(status);
            }
            break;
        case NX_SECURE_TLS_CLIENT_STATE_SERVERHELLO:
            /* We processed a serverhello above. In TLS 1.3, it is time to generate keys and switch on encryption for the handshake. */

            if(tls_session->nx_secure_tls_key_material.nx_secure_tls_handshake_cache_length > 0)
            {
                /* We have some cached messages from earlier in the handshake that we need to process. Generally
                   this will just be the ClientHello in TLS 1.3. */
                status = _nx_secure_tls_handshake_hash_update(tls_session, tls_session->nx_secure_tls_key_material.nx_secure_tls_handshake_cache,
                                                              tls_session->nx_secure_tls_key_material.nx_secure_tls_handshake_cache_length);
                if(status != NX_SUCCESS)
                {
                    break;
                }

                /* Indicate that all cached messages have been hashed. */
                tls_session->nx_secure_tls_key_material.nx_secure_tls_handshake_cache_length = 0;

                /* Save the transcript hash for ClientHello - ServerHello was processed so we now have a ciphersuite and know what
                   out hash method is. */
                status = _nx_secure_tls_1_3_transcript_hash_save(tls_session, NX_SECURE_TLS_TRANSCRIPT_IDX_CLIENTHELLO, NX_TRUE);
                if(status != NX_SUCCESS)
                {
                    break;
                }
            }

            /* Update the transcript hash with the ServerHello. */
            _nx_secure_tls_handshake_hash_update(tls_session, packet_start, message_length + header_bytes);

            /* Save the transcript hash to this point for key generation - ServerHello was processed so use it. */
            status = _nx_secure_tls_1_3_transcript_hash_save(tls_session, NX_SECURE_TLS_TRANSCRIPT_IDX_SERVERHELLO, NX_TRUE);
            if(status != NX_SUCCESS)
            {
                break;
            }

            /* Generate TLS 1.3 keys and secrets for the handshake. */
            status = _nx_secure_tls_1_3_generate_handshake_keys(tls_session);
            if(status != NX_SUCCESS)
            {
                break;
            }

            /* Now turn on encryption. */
            tls_session -> nx_secure_tls_local_session_active = 1;
            tls_session -> nx_secure_tls_remote_session_active = 1;

            break;
        case NX_SECURE_TLS_CLIENT_STATE_ENCRYPTED_EXTENSIONS:
            /* Processed an encrypted extensions message above. */
            break;
        case NX_SECURE_TLS_CLIENT_STATE_SERVER_CERTIFICATE:
            /* Processed a server certificate above. Here, we extract the public key and do any verification
               we want - the TLS implementation will verify certificate authenticity by checking the issuer
               signature, but any other verification will be done by the caller via a callback. */
            break;
        case NX_SECURE_TLS_CLIENT_STATE_CERTIFICATE_REQUEST:
            /* Set flag to send CertificateVerify once we have received ServerHelloDone. */
            tls_session -> nx_secure_tls_client_certificate_requested = 1;
            break;
        case NX_SECURE_TLS_CLIENT_STATE_HANDSHAKE_FINISHED:
            /* We received a Finished, meaning we now have all the information we need to generate
               our session key material. First check if the server requested our client certificate. */

            /* Save the transcript hash to this point for key generation - Server Finished was processed so save it. */
            status = _nx_secure_tls_1_3_transcript_hash_save(tls_session, NX_SECURE_TLS_TRANSCRIPT_IDX_SERVER_FINISHED, NX_TRUE);
            if(status != NX_SUCCESS)
            {
                break;
            }


            /* Check for certificate request. */
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

                /* Save the transcript hash up to the certificate for the CertificateVerify (next message to send). */
                status = _nx_secure_tls_1_3_transcript_hash_save(tls_session, NX_SECURE_TLS_TRANSCRIPT_IDX_CERTIFICATE, NX_TRUE);
                if (status != NX_SUCCESS)
                {
                    break;
                }
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


            /* We processed a server finished message, completing the handshake. Verify all is good and if so,
               continue to the encrypted session. */

            /* Generate TLS 1.3 keys and secrets for the application data session. */
            status = _nx_secure_tls_1_3_generate_session_keys(tls_session);
            if(status != NX_SUCCESS)
            {
                break;
            }


            /* Now switch to the session keys for our remote session since we have processed the Finished from the server. */
            status = _nx_secure_tls_1_3_session_keys_set(tls_session, NX_SECURE_TLS_KEY_SET_REMOTE);
            if(status != NX_SUCCESS)
            {
                break;
            }

            /* Save the transcript hash to this point for generating Finished. */
            status = _nx_secure_tls_1_3_transcript_hash_save(tls_session, NX_SECURE_TLS_TRANSCRIPT_IDX_SERVER_FINISHED, NX_TRUE);
            if(status != NX_SUCCESS)
            {
                break;
            }

            /* We can now send our finished message, which will be encrypted using the chosen ciphersuite. */
            status = _nx_secure_tls_allocate_handshake_packet(tls_session, packet_pool, &send_packet, wait_option);

            if (status != NX_SUCCESS)
            {
                break;
            }

            /* Generate and send the finished message, which completes the handshake. */
            _nx_secure_tls_send_finished(tls_session, send_packet);

            status = _nx_secure_tls_send_handshake_record(tls_session, send_packet, NX_SECURE_TLS_FINISHED, wait_option);

            /* Save the transcript hash to this point for key generation - Client Finished was just sent so save it. */
            status = _nx_secure_tls_1_3_transcript_hash_save(tls_session, NX_SECURE_TLS_TRANSCRIPT_IDX_CLIENT_FINISHED, NX_TRUE);
            if(status != NX_SUCCESS)
            {
                break;
            }

            /* Now switch to the session keys for our local session since we have sent the Finished method. */
            status = _nx_secure_tls_1_3_session_keys_set(tls_session, NX_SECURE_TLS_KEY_SET_LOCAL);
            if(status != NX_SUCCESS)
            {
                break;
            }

            /* Generate TLS 1.3 keys and secrets for the post-handshake (e.g. session resumption PSK). */
            status = _nx_secure_tls_1_3_generate_session_keys(tls_session);
            if(status != NX_SUCCESS)
            {
                break;
            }


            break;
/*      Cases not handled in TLS 1.3 (for reference):
        case NX_SECURE_TLS_CLIENT_STATE_HELLO_REQUEST: // TLS 1.3: No HelloRequest message!
        case NX_SECURE_TLS_CLIENT_STATE_HELLO_VERIFY: // DTLS ONLY!
        case NX_SECURE_TLS_CLIENT_STATE_SERVER_KEY_EXCHANGE: // TLS 1.3: No ServerKeyExchange message!
        case NX_SECURE_TLS_CLIENT_STATE_SERVERHELLO_DONE: // TLS 1.3: No ServerHelloDone message!
 */
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
#endif
