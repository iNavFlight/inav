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

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_dtls_session_receive                     PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function receives data from an active DTLS session, handling   */
/*    all decryption and verification before returning the data to the    */
/*    caller in the supplied NX_PACKET structure.                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          DTLS session control block    */
/*    packet_ptr_ptr                        Pointer to return packet      */
/*    wait_option                           Indicates how long the caller */
/*                                          should wait for a packet      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_dtls_packet_allocate       Allocate internal DTLS packet */
/*    _nx_secure_dtls_process_record        Process DTLS record data      */
/*    _nx_secure_dtls_send_record           Send the DTLS record          */
/*    _nx_secure_tls_map_error_to_alert     Map internal error to alert   */
/*    _nx_secure_tls_send_alert             Send TLS alert                */
/*    nx_secure_tls_packet_release          Release packet                */
/*    nx_udp_socket_receive                 Receive UDP data              */
/*    nxd_udp_source_extract                Extract UDP information       */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*    tx_thread_preemption_change           Disable thread preemption     */
/*    tx_thread_sleep                       Thread sleep                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_secure_dtls_session_start         Actual DTLS session start call*/
/*    _nx_secure_dtls_session_end           End of a session              */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            released packet securely,   */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Timothy Stapko           Modified comment(s),          */
/*                                            fixed out-of-order handling,*/
/*                                            resulting in version 6.1.10 */
/*  07-29-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed compiler errors when  */
/*                                            IPv4 is disabled,           */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_dtls_session_receive(NX_SECURE_DTLS_SESSION *dtls_session,
                                     NX_PACKET **packet_ptr_ptr, ULONG wait_option)
{
#ifdef NX_SECURE_ENABLE_DTLS
UINT                   status;
NX_PACKET             *packet_ptr = NX_NULL;
NX_PACKET             *send_packet = NX_NULL;
ULONG                  bytes_processed;
#if 0
UINT                   remote_port;
#endif
ULONG                  packet_length;
UINT                   error_number;
UINT                   alert_number;
UINT                   alert_level;
NX_SECURE_TLS_SESSION *tls_session;
NX_UDP_SOCKET         *udp_socket;
UINT                   old_threshold;
NXD_ADDRESS            source_address;
UINT                   source_port;


    /* Process all records in the packet we received - decrypt, authenticate, and
     * strip TLS record header/footer, placing data in the return packet.
     */

    /* Get a working pointer to our internal TLS session. */
    tls_session = &(dtls_session -> nx_secure_dtls_tls_session);

    status = NX_CONTINUE;

    /* Continue processing UDP datagrams until we get a valid DTLS record or an error. */
    while (status == NX_CONTINUE)
    {


#ifndef NX_SECURE_TLS_SERVER_DISABLED
        /* If we are a server, the UDP packet was assigned to the session queue in
           the UDP receive callback. */
        if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
        {
            /* We received a UDP packet in the receive callback. Use the head of the queue
               for the next packet and remove it from the queue. */

            /* Get the mutex. */
            tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

            /* If we don't have a packet yet, don't do anything. */
            if (dtls_session -> nx_secure_dtls_receive_queue_head == NX_NULL)
            {
                if (wait_option == NX_NO_WAIT)
                {

                    /* No packet.  */
                    tx_mutex_put(&_nx_secure_tls_protection);
                    return(NX_NO_PACKET);
                }

                /* Disable preemption before waiting for packet. */
                tx_thread_preemption_change(tx_thread_identify(), 0, &old_threshold);

                if (dtls_session -> nx_secure_dtls_thread_suspended)
                {

                    /* Another thread is already suspended. */
                    tx_thread_preemption_change(tx_thread_identify(), old_threshold, &old_threshold);
                    tx_mutex_put(&_nx_secure_tls_protection);
                    return(NX_ALREADY_SUSPENDED);
                }

                /* Set thread waiting for packet. */
                dtls_session -> nx_secure_dtls_thread_suspended = tx_thread_identify();

                /* Release mutex. */
                tx_mutex_put(&_nx_secure_tls_protection);

                /* Sleep to wait for packet. */
                /* The wait process could be aborted if packet received.
                   * And then dtls_session -> nx_secure_dtls_thread_suspended is cleared. */
                tx_thread_sleep(wait_option);

                /* Get mutex. */
                tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

                /* Restore preemption . */
                tx_thread_preemption_change(tx_thread_identify(), old_threshold, &old_threshold);

                if (!dtls_session -> nx_secure_dtls_session_in_use)
                {

                    /* Session is not in use.  */
                    tx_mutex_put(&_nx_secure_tls_protection);
                    return(NX_SECURE_TLS_SESSION_UNINITIALIZED);
                }

                if ((volatile NX_PACKET *)dtls_session -> nx_secure_dtls_receive_queue_head == NX_NULL)
                {

                    /* Still no packet. */
                    dtls_session -> nx_secure_dtls_thread_suspended = NX_NULL;
                    tx_mutex_put(&_nx_secure_tls_protection);
                    return(NX_NO_PACKET);
                }
            }

            /* Get the received packet.  */
            packet_ptr = dtls_session -> nx_secure_dtls_receive_queue_head;

            /* Remove the packet from the queue. */
            dtls_session -> nx_secure_dtls_receive_queue_head = packet_ptr -> nx_packet_queue_next;
            packet_ptr -> nx_packet_queue_next = NX_NULL;

            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);

            status = NX_SUCCESS;
        }
        else
#endif
        {
            /* If we are a client, just receive the UDP packet directly. */
            udp_socket = dtls_session -> nx_secure_dtls_udp_socket;

            status = nx_udp_socket_receive(udp_socket, &packet_ptr, wait_option);

            if (status)
            {
                /* Error in socket receive. */
                return(status);
            }

            /* Extract the source IP address and port.  */
            status = nxd_udp_source_extract(packet_ptr, &source_address, &source_port);

            /* With DTLS, we need to send back to the remote host on the port they used to send us data.
               Check if the source IP address is same as the stored remote IP address,
               and if the source port is same as the stored remote port.  */
            if ((status) ||
                (source_port != dtls_session -> nx_secure_dtls_remote_port) ||
                (source_address.nxd_ip_version != dtls_session -> nx_secure_dtls_remote_ip_address.nxd_ip_version))
            {
                status = NX_CONTINUE;
            }
            else
            {
#ifndef NX_DISABLE_IPV4
                if (dtls_session -> nx_secure_dtls_remote_ip_address.nxd_ip_version == NX_IP_VERSION_V4)
                {

                    /* Compare the IPv4 address.  */
                    if (dtls_session -> nx_secure_dtls_remote_ip_address.nxd_ip_address.v4 != source_address.nxd_ip_address.v4)
                    {
                        status = NX_CONTINUE;
                    }
                }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
                if (dtls_session -> nx_secure_dtls_remote_ip_address.nxd_ip_version == NX_IP_VERSION_V6)
                {

                    /* Compare the IPv6 address.  */
                    if ((dtls_session -> nx_secure_dtls_remote_ip_address.nxd_ip_address.v6[0] != source_address.nxd_ip_address.v6[0]) ||
                        (dtls_session -> nx_secure_dtls_remote_ip_address.nxd_ip_address.v6[1] != source_address.nxd_ip_address.v6[1]) ||
                        (dtls_session -> nx_secure_dtls_remote_ip_address.nxd_ip_address.v6[2] != source_address.nxd_ip_address.v6[2]) ||
                        (dtls_session -> nx_secure_dtls_remote_ip_address.nxd_ip_address.v6[3] != source_address.nxd_ip_address.v6[3]))
                    {
                        status = NX_CONTINUE;
                    }
                }
#endif /* FEATURE_NX_IPV6 */
            }

            if (status)
            {

                /* This packet is from unkown address or port. Ignore it.  */
                nx_secure_tls_packet_release(packet_ptr);
                status = NX_CONTINUE;
                continue;
            }
        }

        /* Set local IP address index. It will be used to send record.  */
        if (dtls_session -> nx_secure_dtls_local_ip_address_index == 0xffffffff)
        {
            if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V4)
            {
                dtls_session -> nx_secure_dtls_local_ip_address_index = packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_index;
            }
#ifdef FEATURE_NX_IPV6
            else if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V6)
            {
                if (packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr)
                {
                    dtls_session -> nx_secure_dtls_local_ip_address_index = packet_ptr->nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_index;
                }
            }
#endif /* FEATURE_NX_IPV6 */
        }

        /* Process all records in the packet we received - decrypt, authenticate, and
         * strip TLS record header/footer, placing data in the return packet.
         */
        if (packet_ptr -> nx_packet_next)
        {

            /* Chained packet is not supported. */
            status = NX_SECURE_TLS_INVALID_PACKET;
        }
        else
        {

            packet_length = packet_ptr -> nx_packet_length;
            bytes_processed = 0;
            while (packet_length > 0)
            {
                /* If we have multiple records in the datagram, advance the pointer.
                   If only a single record, bytes_processed is 0. */
                packet_ptr -> nx_packet_prepend_ptr += bytes_processed;

                /* Get the protection. */
                tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

                /* The UDP datagram may contain more than one DTLS record... */
                status = _nx_secure_dtls_process_record(dtls_session, packet_ptr, 0, &bytes_processed, wait_option);

                /* Release the protection. */
                tx_mutex_put(&_nx_secure_tls_protection);

                if (status && (status != NX_CONTINUE))
                {

                    /* Error status, just break.  */
                    break;
                }

                if (bytes_processed > packet_length)
                {
                    break;
                }

                /* Advance the packet pointer to the next DTLS record in the datagram. */
                packet_length -= bytes_processed;
            }
        }

        if (status != NX_SUCCESS)
        {

            /* Clear out the packet, we don't want any of the data in it. */
            nx_secure_tls_packet_release(packet_ptr);

            if (status == NX_SECURE_TLS_ALERT_RECEIVED)
            {
                /* See if the alert was a CloseNotify */
                if(tls_session -> nx_secure_tls_received_alert_level == NX_SECURE_TLS_ALERT_LEVEL_WARNING &&
                   tls_session -> nx_secure_tls_received_alert_value == NX_SECURE_TLS_ALERT_CLOSE_NOTIFY)
                {
                    /* Close the connection */
                    status = NX_SECURE_TLS_CLOSE_NOTIFY_RECEIVED;
                }
                /* Dont send alert to remote host if we recevied an alert */
            }
            else if (status != NX_CONTINUE)
            {
                /* Error status, send alert back to remote host. */
                /* Get our alert number and level from our status. */
                error_number = status;

                /* Get the protection. */
                tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

                _nx_secure_tls_map_error_to_alert(error_number, &alert_number, &alert_level);

                /* Release the protection before suspending on nx_packet_allocate. */
                tx_mutex_put(&_nx_secure_tls_protection);

                status = _nx_secure_dtls_packet_allocate(dtls_session,
                                                         tls_session -> nx_secure_tls_packet_pool,
                                                         &send_packet, wait_option);

                if (status == NX_SUCCESS)
                {

                    /* Get the protection after nx_packet_allocate. */
                    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

                    _nx_secure_tls_send_alert(tls_session, send_packet, (UCHAR)alert_number, (UCHAR)alert_level);
                    _nx_secure_dtls_send_record(dtls_session, send_packet, NX_SECURE_TLS_ALERT, wait_option);

                    /* Release the protection. */
                    tx_mutex_put(&_nx_secure_tls_protection);
                }
                status = error_number;
                return(status);
            }
        }
    }

    /* Return our completed packet. */
    *packet_ptr_ptr = packet_ptr;

    return(status);
#else
    NX_PARAMETER_NOT_USED(dtls_session);
    NX_PARAMETER_NOT_USED(packet_ptr_ptr);
    NX_PARAMETER_NOT_USED(wait_option);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}

