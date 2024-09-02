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
/*    _nx_secure_tls_1_3_server_handshake                 PORTABLE C      */
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
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
UINT _nx_secure_tls_1_3_server_handshake(NX_SECURE_TLS_SESSION *tls_session, UCHAR *packet_buffer,
                                         UINT data_length, ULONG wait_option)
{
#ifndef NX_SECURE_TLS_SERVER_DISABLED
UINT                                  status;
USHORT                                message_type;
UINT                                  header_bytes;
UINT                                  message_length;
NX_PACKET                            *send_packet;
NX_PACKET_POOL                       *packet_pool;
UCHAR                                *packet_start;
NX_SECURE_TLS_SERVER_STATE            old_server_state;

    /* Basic state machine for handshake:
     * 1. We have received a handshake message, now process the header.
     * 2. Then process the message itself and populate the TLS socket structure.
     * 3. Follow up with whatever actions are needed.
     */

    /* TLS 1.3 handshake state machine (TLS 1.3 RFC 8446):

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

        /* Reduce total length by the size of this message. */
        data_length -= (message_length + header_bytes);

        /* Process the message itself information from the header. */
        status = NX_SECURE_TLS_SUCCESS;
        switch (message_type)
        {
        case NX_SECURE_TLS_CLIENT_HELLO:
            old_server_state = tls_session -> nx_secure_tls_server_state;
            tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_IDLE;

            /* Client is establishing a TLS session with our server. */
            status = _nx_secure_tls_process_clienthello(tls_session, packet_buffer, message_length);

            if(status != NX_SUCCESS)
            {
                break;
            }

            if (tls_session -> nx_secure_tls_1_3 == NX_FALSE)
            {

                /* Negotiate a version of TLS prior to TLS 1.3. */
                return(NX_SUCCESS);
            }

            if (old_server_state == NX_SECURE_TLS_SERVER_STATE_IDLE)
            {
            
                /* Initialize the handshake hash - we should have chosen a ciphersuite, so we can
                 * initialize the hash state now using the chosen hash. */
                _nx_secure_tls_handshake_hash_init(tls_session);
            }

            if (tls_session -> nx_secure_tls_server_state != NX_SECURE_TLS_SERVER_STATE_SEND_HELLO_RETRY)
            {
                tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_SEND_HELLO;
            }
            break;
#ifdef NX_SECURE_ENABLE_CLIENT_CERTIFICATE_VERIFY
        case NX_SECURE_TLS_CERTIFICATE_MSG:
            /* Client sent certificate message (in response to a request from us. Process it now. */
            status = _nx_secure_tls_process_remote_certificate(tls_session, packet_buffer, message_length, data_length);

            /* If client sends an empty Certificate message, server should abort the handshake with a "certificate_required" alert. */
            if (status == NX_SECURE_TLS_EMPTY_REMOTE_CERTIFICATE_RECEIVED)
            {
                status = NX_SECURE_TLS_CERTIFICATE_REQUIRED;
            }

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
        case NX_SECURE_TLS_FINISHED:

            /* Save the transcript hash to this point for processing Finished. */
            status = _nx_secure_tls_1_3_transcript_hash_save(tls_session, NX_SECURE_TLS_TRANSCRIPT_IDX_SERVER_FINISHED, NX_TRUE);
            if(status != NX_SUCCESS)
            {
                break;
            }

            /* Final handshake message from the client, process it (verify the client handshake hash). */
            status = _nx_secure_tls_process_finished(tls_session, packet_buffer, message_length);
            if(status != NX_SUCCESS)
            {
                break;
            }

            /* Save the transcript hash to this point for key generation - Server Finished was processed so save it. */
            status = _nx_secure_tls_1_3_transcript_hash_save(tls_session, NX_SECURE_TLS_TRANSCRIPT_IDX_CLIENT_FINISHED, NX_TRUE);
            if(status != NX_SUCCESS)
            {
                break;
            }

            tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_FINISH_HANDSHAKE;
            break;
/*      Invalid message types in tls 1.3. 
        case NX_SECURE_TLS_CLIENT_KEY_EXCHANGE:
        case NX_SECURE_TLS_HELLO_VERIFY_REQUEST:
        case NX_SECURE_TLS_HELLO_REQUEST:
        case NX_SECURE_TLS_SERVER_HELLO:
        case NX_SECURE_TLS_SERVER_KEY_EXCHANGE:
        case NX_SECURE_TLS_CERTIFICATE_REQUEST:
        case NX_SECURE_TLS_SERVER_HELLO_DONE:
        case NX_SECURE_TLS_CERTIFICATE_URL:
        case NX_SECURE_TLS_CERTIFICATE_STATUS:
*/
        default:
            /* The message received was not a valid TLS server handshake message, send alert and return. */
            status = NX_SECURE_TLS_HANDSHAKE_FAILURE;
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
            if((message_type == NX_SECURE_TLS_CLIENT_HELLO) && (tls_session -> nx_secure_tls_server_state != NX_SECURE_TLS_SERVER_STATE_SEND_HELLO_RETRY))
            {
                /* Save the transcript hash for ClientHello - ClientHello was processed so we now have a ciphersuite and know what
                   our hash method is. */
                status = _nx_secure_tls_1_3_transcript_hash_save(tls_session, NX_SECURE_TLS_TRANSCRIPT_IDX_CLIENTHELLO, NX_TRUE);
                if(status != NX_SUCCESS)
                {
                    return status;
                }
            }
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


            /* Save the transcript hash to this point for key generation - ServerHello was sent so use it. */
            status = _nx_secure_tls_1_3_transcript_hash_save(tls_session, NX_SECURE_TLS_TRANSCRIPT_IDX_SERVERHELLO, NX_TRUE);
            if(status != NX_SUCCESS)
            {
                break;
            }

            NX_ASSERT(tls_session -> nx_secure_tls_session_ciphersuite != NX_NULL);


            /* In TLS 1.3, we now have enough information to generate the handshake encryption keys and start encrypting messages. */
            /* Generate TLS 1.3 keys and secrets for the handshake. */
            status = _nx_secure_tls_1_3_generate_handshake_keys(tls_session);
            if(status != NX_SUCCESS)
            {
                break;
            }

            /* Now turn on encryption. We have generated the handshake keys so the rest of the handshake is encrypted. */
            tls_session -> nx_secure_tls_local_session_active = 1;
            tls_session -> nx_secure_tls_remote_session_active = 1;

            /* Send encrypted server extensions (if any) here. */
        
            /* We have now started the encrypted TLS 1.3 handshake. Send encrypted extensions now. */
            status = _nx_secure_tls_allocate_handshake_packet(tls_session, packet_pool, &send_packet, wait_option);

            if (status != NX_SUCCESS)
            {
                break;
            }

            status = _nx_secure_tls_send_encrypted_extensions(tls_session, send_packet);

            if (status != NX_SUCCESS)
            {
                break;
            }

            status = _nx_secure_tls_send_handshake_record(tls_session, send_packet, NX_SECURE_TLS_ENCRYPTED_EXTENSIONS, wait_option);
            if (status != NX_SUCCESS)
            {
                break;
            }

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

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
            /* For PSK ciphersuites, don't send the certificate message. */
            if (tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm != NX_CRYPTO_KEY_EXCHANGE_PSK &&
                tls_session->nx_secure_tls_credentials.nx_secure_tls_client_psk.nx_secure_tls_psk_data_size == 0)
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

                /* Save the transcript hash up to the certificate for the CertificateVerify (next message to send). */
                status = _nx_secure_tls_1_3_transcript_hash_save(tls_session, NX_SECURE_TLS_TRANSCRIPT_IDX_CERTIFICATE, NX_TRUE);
                if(status != NX_SUCCESS)
                {
                    break;
                }


                status = _nx_secure_tls_allocate_handshake_packet(tls_session, packet_pool, &send_packet, wait_option);

                if (status != NX_SUCCESS)
                {
                    break;
                }

                _nx_secure_tls_send_certificate_verify(tls_session, send_packet);


                status = _nx_secure_tls_send_handshake_record(tls_session, send_packet, NX_SECURE_TLS_CERTIFICATE_VERIFY, wait_option);
                if (status != NX_SUCCESS)
                {
                    break;
                }


#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
            }
#endif

            /* Save the transcript hash to this point for generating Finished. */
            status = _nx_secure_tls_1_3_transcript_hash_save(tls_session, NX_SECURE_TLS_TRANSCRIPT_IDX_SERVER_FINISHED, NX_TRUE);
            if(status != NX_SUCCESS)
            {
                break;
            }

            /* We have completed sending everything from the server side and generated our session keys, now send the Finished. */
            status = _nx_secure_tls_allocate_handshake_packet(tls_session, packet_pool, &send_packet, wait_option);
            if (status != NX_SUCCESS)
            {
                break;
            }

            /* Populate the packet with our Finished Message. */
            _nx_secure_tls_send_finished(tls_session, send_packet);
            status = _nx_secure_tls_send_handshake_record(tls_session, send_packet, NX_SECURE_TLS_FINISHED, wait_option);
            if(status != NX_SUCCESS)
            {
                break;
            }

            /* Save the finished message now - overwrite previous hash. */
            status = _nx_secure_tls_1_3_transcript_hash_save(tls_session, NX_SECURE_TLS_TRANSCRIPT_IDX_SERVER_FINISHED, NX_TRUE);
            if(status != NX_SUCCESS)
            {
                break;
            }

            /* Re-generate TLS 1.3 keys and secrets including the Server Finished message for the application data session. */
            status = _nx_secure_tls_1_3_generate_session_keys(tls_session);
            if(status != NX_SUCCESS)
            {
                break;
            }

            /* In TLS 1.3, the server sends everything in a single flight - ServerHello through Finished, so
             * no need to go into the other states. */

            /* Now switch to the session keys for our local session since we have sent the Server Finished message. */
            status = _nx_secure_tls_1_3_session_keys_set(tls_session, NX_SECURE_TLS_KEY_SET_LOCAL);
            if(status != NX_SUCCESS)
            {
                break;
            }

            /* We still need to process the Client responses - certificate (if needed), and Finished. */
            tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_HELLO_SENT;

            break;
        case NX_SECURE_TLS_SERVER_STATE_SEND_HELLO_RETRY:
            /* We have received and processed a client hello retry requst. Now respond to the client appropriately. */
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
            break;
        case NX_SECURE_TLS_SERVER_STATE_CLIENT_CERTIFICATE:

            /* Save the transcript hash up to the certificate for the CertificateVerify (next message to send). */
            status = _nx_secure_tls_1_3_transcript_hash_save(tls_session, NX_SECURE_TLS_TRANSCRIPT_IDX_CERTIFICATE, NX_TRUE);
            break;
        case NX_SECURE_TLS_SERVER_STATE_CERTIFICATE_VERIFY:
            /* We processed the certificate above, do nothing. */
            break;
        case NX_SECURE_TLS_SERVER_STATE_FINISH_HANDSHAKE:

            /* Now switch to the session keys for our remote session since we have processed the Finished from the Client. */
            status = _nx_secure_tls_1_3_session_keys_set(tls_session, NX_SECURE_TLS_KEY_SET_REMOTE);
            if(status != NX_SUCCESS)
            {
                break;
            }

            /* Post-Auth server messages (if any) are sent here. */

            /* For session resumption, send a NewSessionTicket message to allow for resumption PSK to be generated. */
            status = _nx_secure_tls_allocate_handshake_packet(tls_session, packet_pool, &send_packet, wait_option);
            if (status != NX_SUCCESS)
            {
                break;
            }

            /* Populate the packet with our NewSessionTicket Message. */
            status = _nx_secure_tls_send_newsessionticket(tls_session, send_packet);
            status = _nx_secure_tls_send_handshake_record(tls_session, send_packet, NX_SECURE_TLS_NEW_SESSION_TICKET, wait_option);
            if(status != NX_SUCCESS)
            {
                break;
            }

            /* If we get here, the Client Finished was processed without errors and the handshake is complete. */
            tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_HANDSHAKE_FINISHED;
            break;
/*      case NX_SECURE_TLS_SERVER_STATE_KEY_EXCHANGE: // tls 1.3: No CLIENT_KEY_EXCHANGE message!
 */
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

#endif
