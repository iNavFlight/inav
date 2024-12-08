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
/*    _nx_secure_tls_session_receive_records              PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function receives data from the TCP socket and handles TLS     */
/*    record processing. It's purpose is to avoid recursion in the        */
/*    higher-level nx_secure_tls_session_receive service.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_ptr_ptr                        Pointer to return packet      */
/*    wait_option                           Indicates how long the caller */
/*                                          should wait for the response  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_map_error_to_alert     Map internal error to alert   */
/*    _nx_secure_tls_packet_allocate        Allocate internal TLS packet  */
/*    _nx_secure_tls_process_record         Process TLS record data       */
/*    _nx_secure_tls_send_alert             Send TLS alert                */
/*    _nx_secure_tls_send_record            Send the TLS record           */
/*    nx_secure_tls_packet_release          Release packet                */
/*    nx_tcp_socket_receive                 Receive TCP data              */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_handshake_process      Process TLS handshake         */
/*    _nx_secure_tls_session_receive        Receive TCP data              */
/*    _nx_secure_tls_session_renegotiate    Renegotiate TLS session       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            supported chained packet,   */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), added    */
/*                                            conditional TLS 1.3 build,  */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
UINT  _nx_secure_tls_session_receive_records(NX_SECURE_TLS_SESSION *tls_session,
                                             NX_PACKET **packet_ptr_ptr, ULONG wait_option)
{
UINT           status;
NX_PACKET     *packet_ptr;
NX_TCP_SOCKET *tcp_socket;
ULONG          bytes_processed = 0;
ULONG          packet_fragment_length;
NX_PACKET     *send_packet = NX_NULL;
UINT           error_number;
UINT           alert_number;
UINT           alert_level;
NX_PACKET     *current_packet;
NX_PACKET     *previous_packet;
UCHAR          handshake_finished = NX_FALSE;

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* Access the internal TCP socket handle in our TLS socket. */
    tcp_socket = tls_session -> nx_secure_tls_tcp_socket;

    /* Session receive logic:
     * 1. Receive incoming packets
     * 2. Process records and receive while full record is not yet received.
     * 3. If renegotiation inititated, process the renegotiation handshake.
     *    3a. Process entire handshake (receive TCP packets, process records)
     *    3b. Once handshake processed, receive any new packets, but only if
     *        the remote host initiated the renegotiation.
     */


    status = NX_CONTINUE;
    if (tls_session -> nx_secure_record_queue_header)
    {

        /* Process all records in the packet we received - decrypt, authenticate, and
         * strip TLS record header/footer, placing data in the return packet.
         */
        status = _nx_secure_tls_process_record(tls_session, NX_NULL, &bytes_processed, wait_option);
    }

    while (status == NX_CONTINUE)
    {

        /* Release the protection before suspending on nx_tcp_socket_receive. */
        tx_mutex_put(&_nx_secure_tls_protection);

        /* Receive a packet over the TCP connection. */
        status =  nx_tcp_socket_receive(tcp_socket, &packet_ptr, wait_option);


        if (status != NX_SUCCESS)
        {
            return(status);
        }

        /* Get the protection after nx_tcp_socket_receive. */
        tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

        /* Process all records in the packet we received - decrypt, authenticate, and
         * strip TLS record header/footer, placing data in the return packet.
         */
        status = _nx_secure_tls_process_record(tls_session, packet_ptr, &bytes_processed, wait_option);
    }

    /* Cleanup if the record processing was successful or if we have a renegotiation attempt. */
    if (status == NX_SUCCESS 
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
        || status == NX_SECURE_TLS_POST_HANDSHAKE_RECEIVED
#endif /* (NX_SECURE_TLS_TLS_1_3_ENABLED) */
        )
    {

        /* Remove processed packets. Data in released packet will be cleared by nx_secure_tls_packet_release. */
        tls_session -> nx_secure_record_queue_header -> nx_packet_length -= bytes_processed;
        current_packet = tls_session -> nx_secure_record_queue_header;
        previous_packet = NX_NULL;
        while (current_packet)
        {
            packet_fragment_length = (ULONG)(current_packet -> nx_packet_append_ptr) - (ULONG)(current_packet -> nx_packet_prepend_ptr);

            /* Determine if all data in the current fragment have been processed. */
            if (packet_fragment_length <= bytes_processed)
            {
                bytes_processed -= packet_fragment_length;
            }
            else
            {
                current_packet -> nx_packet_prepend_ptr += bytes_processed;
                bytes_processed = 0;
                break;
            }
            previous_packet = current_packet;
            current_packet = current_packet -> nx_packet_next;
        }

        if (!current_packet)
        {
            nx_secure_tls_packet_release(tls_session -> nx_secure_record_queue_header);
            tls_session -> nx_secure_record_queue_header = NX_NULL;
        }
        else if (previous_packet)
        {

            /* Release trimmed packets. */
            /* Packets from tls_session -> nx_secure_record_queue_header till previous_packet can be trimmed. */
            previous_packet -> nx_packet_next = NX_NULL;

            /* Update the length and last packet of remaining packets. */
            current_packet -> nx_packet_length = tls_session -> nx_secure_record_queue_header -> nx_packet_length;
            current_packet -> nx_packet_last = tls_session -> nx_secure_record_queue_header -> nx_packet_last;

            /* Correct the last packet to be trimmed. */
            tls_session -> nx_secure_record_queue_header -> nx_packet_last = previous_packet;
            nx_secure_tls_packet_release(tls_session -> nx_secure_record_queue_header);

            /* Update the remaining packets. */
            tls_session -> nx_secure_record_queue_header = current_packet;
        }

        if (bytes_processed)
        {

            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);

            return(NX_SECURE_TLS_INVALID_PACKET);
        }

#ifndef NX_SECURE_TLS_CLIENT_DISABLED
        if ((tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_CLIENT) &&
            (tls_session -> nx_secure_tls_client_state == NX_SECURE_TLS_CLIENT_STATE_HANDSHAKE_FINISHED))
        {
            handshake_finished = NX_TRUE;
        }
#endif /* NX_SECURE_TLS_CLIENT_DISABLED */

#ifndef NX_SECURE_TLS_SERVER_DISABLED
        if ((tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER) &&
            (tls_session -> nx_secure_tls_server_state == NX_SECURE_TLS_SERVER_STATE_HANDSHAKE_FINISHED))
        {
            handshake_finished = NX_TRUE;
        }
#endif /* NX_SECURE_TLS_CLIENT_DISABLED */

        if (handshake_finished)
        {
            if (tls_session -> nx_secure_record_decrypted_packet == NX_NULL)
            {
                /* Release the protection. */
                tx_mutex_put(&_nx_secure_tls_protection);

                return(NX_SECURE_TLS_INVALID_PACKET);
            }

            *packet_ptr_ptr = tls_session -> nx_secure_record_decrypted_packet;
            tls_session -> nx_secure_record_decrypted_packet = NX_NULL;
        }
    }
    else
    {
        /* Error status, send alert back to remote host. */
        /* Get our alert number and level from our status. */
        error_number = status;
        _nx_secure_tls_map_error_to_alert(error_number, &alert_number, &alert_level);

#ifndef NX_SECURE_TLS_SERVER_DISABLED
        if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
        {
            tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_ERROR;
        }
#endif

#ifndef NX_SECURE_TLS_CLIENT_DISABLED
        if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_CLIENT)
        {
            tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_ERROR;
        }
#endif

        /* If the error was an alert from the remote host, don't do anything. Otherwise, we need
           to send an alert of our own. */
        if (error_number != NX_SECURE_TLS_ALERT_RECEIVED)
        {
            /* Release the protection before suspending on nx_packet_allocate. */
            tx_mutex_put(&_nx_secure_tls_protection);

            status = _nx_secure_tls_packet_allocate(tls_session, tls_session -> nx_secure_tls_packet_pool, &send_packet, wait_option);

            /* Get the protection after nx_packet_allocate. */
            tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

            if (status == NX_SUCCESS)
            {
                _nx_secure_tls_send_alert(tls_session, send_packet, (UCHAR)alert_number, (UCHAR)alert_level);
                status = _nx_secure_tls_send_record(tls_session, send_packet, NX_SECURE_TLS_ALERT, wait_option);

                /* If we didn't send the alert successfully, we need to release the packet. */
                if (status != NX_SUCCESS)
                {
                    nx_secure_tls_packet_release(send_packet);
                }
            }

            /* Make sure we clear keys whenever we enter an error state. Note that if a Fatal alert
               was received, the session was reset at that point. */
            _nx_secure_tls_session_reset(tls_session);
        }
        status = error_number;

    }
    tx_mutex_put(&_nx_secure_tls_protection);
    return(status);
}

