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
#include "nx_udp.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_dtls_receive_callback                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function serves as the notification callback provided to NetX  */
/*    that is invoked when a UDP packet has been received. It checks the  */
/*    DTLS session cache for a matching client (based on IP address and   */
/*    port) and then invokes the application callback with the            */
/*    appropriate session object. If a previous session was not found, a  */
/*    new session is allocated (if available) and the DTLS handshake is   */
/*    initiated. Once the handshake is complete the user callback is      */
/*    invoked as above.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            UDP socket                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    N/A                                                                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_udp_source_extract               Extract IP address and port   */
/*    _nx_udp_socket_port_get               Get local port                */
/*    nx_secure_dtls_session_cache_find     Check DTLS session cache      */
/*    nx_secure_dtls_session_cache_get_new  Get new DTLS session          */
/*    nx_secure_tls_packet_release          Release packet                */
/*    nx_packet_allocate                    Allocate packet               */
/*    _nxd_udp_socket_send                  Send UDP packet               */
/*    nx_udp_socket_receive                 Receive packet                */
/*    tx_thread_wait_abort                  Abort wait process            */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*    [nx_secure_dtls_error_notify]         Notify application of error   */
/*    [nx_secure_dtls_receive_notify]       Notify application of packet  */
/*                                            receive                     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
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
VOID _nx_secure_dtls_receive_callback(NX_UDP_SOCKET *socket_ptr)
{
#ifdef NX_SECURE_ENABLE_DTLS
TX_INTERRUPT_SAVE_AREA

NXD_ADDRESS ip_address;
UINT remote_port;
UINT local_port;
UINT status;
ULONG packet_type;
NX_PACKET *packet_ptr;
UCHAR *send_data;
NX_PACKET *temp_packet_ptr;
NX_SECURE_DTLS_SERVER *dtls_server;
NX_SECURE_DTLS_SESSION *dtls_session;

    if(socket_ptr -> nx_udp_socket_receive_head == NX_NULL)
    {
      return;
    }

    /* Extract IP address and port from received packet. */
    socket_ptr -> nx_udp_socket_receive_head -> nx_packet_prepend_ptr += sizeof(NX_UDP_HEADER);
    status =  _nxd_udp_source_extract(socket_ptr->nx_udp_socket_receive_head, &ip_address, &remote_port);
    socket_ptr -> nx_udp_socket_receive_head -> nx_packet_prepend_ptr -= sizeof(NX_UDP_HEADER);
    
    if(status != NX_SUCCESS)
    {
        return;
    }

    /* Extract local port from socket. */
    status = _nx_udp_socket_port_get(socket_ptr, &local_port);

    if(status != NX_SUCCESS)
    {
        return;
    }

    /* Get the DTLS server from our UDP socket so we can access the session cache. */
    dtls_server = (NX_SECURE_DTLS_SERVER*)(socket_ptr -> nx_udp_socket_reserved_ptr);

    /* Check session cache for existing DTLS session. */
    status = nx_secure_dtls_session_cache_find(dtls_server, &dtls_session, &ip_address, remote_port, local_port);

    /* Do we have an established session? */
    if(status == NX_SECURE_DTLS_SESSION_NOT_FOUND)
    {

        /* If received packet is ALERT, just drop it.  */
        if ((socket_ptr -> nx_udp_socket_receive_head -> nx_packet_length < NX_SECURE_DTLS_RECORD_HEADER_SIZE) ||
            (socket_ptr -> nx_udp_socket_receive_head -> nx_packet_prepend_ptr[8] == NX_SECURE_TLS_ALERT))
        {

            /* Lockout interrupts.  */
            TX_DISABLE

            /* Remove the header packet from the queue.  */
            packet_ptr = socket_ptr -> nx_udp_socket_receive_head;
            socket_ptr -> nx_udp_socket_receive_head = packet_ptr -> nx_packet_queue_next;
            packet_ptr -> nx_packet_queue_next = NX_NULL;
            
            /* If this was the last packet, set the tail pointer to NULL.  */
            if (socket_ptr -> nx_udp_socket_receive_head == NX_NULL)
            {
                socket_ptr -> nx_udp_socket_receive_tail =  NX_NULL;
            }

            /* Decrease the queued packet count.  */
            socket_ptr -> nx_udp_socket_receive_count--;

            /* Restore interrupts.  */
            TX_RESTORE

            /* Release the packet.  */
            nx_secure_tls_packet_release(packet_ptr);

            return;
        }

        /* Get a new session. */
        status = nx_secure_dtls_session_cache_get_new(dtls_server, &dtls_session, &ip_address, remote_port, local_port);

        /* Make sure we got a session. */
        if(status == NX_SECURE_TLS_NO_FREE_DTLS_SESSIONS)
        {
            /* No session? Drop the connection with an internal error alert.
               See RFC 5246 Sec. 7.2.2 - internal error is used for memory allocation failures.

               We don't have a DTLS session, so build a simple DTLS alert record to send. */

            /* Lockout interrupts.  */
            TX_DISABLE

            /* Remove the header packet from the queue.  */
            packet_ptr = socket_ptr -> nx_udp_socket_receive_head;
            socket_ptr -> nx_udp_socket_receive_head = packet_ptr -> nx_packet_queue_next;
            packet_ptr -> nx_packet_queue_next = NX_NULL;
            
            /* If this was the last packet, set the tail pointer to NULL.  */
            if (socket_ptr -> nx_udp_socket_receive_head == NX_NULL)
            {
                socket_ptr -> nx_udp_socket_receive_tail =  NX_NULL;
            }

            /* Decrease the queued packet count.  */
            socket_ptr -> nx_udp_socket_receive_count--;

            /* Restore interrupts.  */
            TX_RESTORE

            /* Release the packet.  */
            nx_secure_tls_packet_release(packet_ptr);

            /* Get a packet to send the alert to the client. */
            if (ip_address.nxd_ip_version == NX_IP_VERSION_V4)
            {
                packet_type = NX_IPv4_UDP_PACKET;
            }
            else
            {
                packet_type = NX_IPv6_UDP_PACKET;
            }

            status =  nx_packet_allocate(socket_ptr->nx_udp_socket_ip_ptr->nx_ip_default_packet_pool,
                                         &packet_ptr, packet_type, NX_NO_WAIT);
            if(status != NX_SUCCESS)
            {
                return;
            }

            if (((ULONG)(packet_ptr -> nx_packet_data_end) - (ULONG)(packet_ptr -> nx_packet_append_ptr)) < 15)
            {

                /* Packet buffer too small. */
                nx_secure_tls_packet_release(packet_ptr);
                return;
            }

            send_data = packet_ptr -> nx_packet_append_ptr;

            /* Build the DTLS record header. */
            send_data[0] = NX_SECURE_TLS_ALERT;

            /* Set the version number - use DTLS 1.2. */
            send_data[1] = (UCHAR)(NX_SECURE_DTLS_VERSION_MAJOR);
            send_data[2] = (UCHAR)(NX_SECURE_DTLS_VERSION_MINOR_1_2);

            /* DTLS Epoch counter. */
            send_data[3] = 0;
            send_data[4] = 0;

            /* DTLS sequence number. */
            send_data[5]  = 0;
            send_data[6]  = 0;
            send_data[7]  = 0;
            send_data[8]  = 0;
            send_data[9]  = 0;
            send_data[10] = 0;

            /* DTLS message length - 2 bytes for the alert. */
            send_data[11] = 0;
            send_data[12] = 2;

            /* Populate the record with the alert level and alert number to send to the remote host. */
            send_data[13] = (UCHAR)(NX_SECURE_TLS_ALERT_LEVEL_FATAL);
            send_data[14] = (UCHAR)(NX_SECURE_TLS_ALERT_INTERNAL_ERROR);

            /* Make sure the caller has the right length of data to send. */
            packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_append_ptr + 15;
            packet_ptr -> nx_packet_length = 15;

            /* Send the UDP packet containing our record. */
            status = _nxd_udp_socket_send(socket_ptr, packet_ptr, &ip_address, remote_port);

            /* Notify the application that we had to drop an incoming connection. */
            if(dtls_server->nx_secure_dtls_error_notify != NX_NULL)
            {
                dtls_server->nx_secure_dtls_error_notify(NX_NULL, NX_SECURE_TLS_NO_FREE_DTLS_SESSIONS);
            }

            return;
        }

        /* Receive our packet. */
        status =  nx_udp_socket_receive(socket_ptr, &packet_ptr, NX_NO_WAIT);
        if (status)
        {
            return;
        }

        /* New session, retrieve the UDP packet and place in our session receive queue. */
        dtls_session -> nx_secure_dtls_receive_queue_head = packet_ptr;

        /* Make sure the queue pointer is cleared. */
        packet_ptr -> nx_packet_queue_next = NX_NULL;

        /* Notify the application of a new connection - it is up to the application to call
           nx_secure_dtls_session_start to kick off the handshake! */
        dtls_server -> nx_secure_dtls_connect_notify(dtls_session, &ip_address, remote_port);
    }
    else
    {

        /* Receive our packet. */
        status =  nx_udp_socket_receive(socket_ptr, &packet_ptr, NX_NO_WAIT);
        if (status)
        {
            return;
        }

        /* Get the protection before modifying the queue pointer. */
        tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

        if (!dtls_session -> nx_secure_dtls_session_in_use)
        {

            /* Session is not in use.  */
            tx_mutex_put(&_nx_secure_tls_protection);
            return;
        }

        /* Established session, make sure that we append to the end of the receive queue. */
        if (dtls_session -> nx_secure_dtls_receive_queue_head == NX_NULL)
        {
            dtls_session -> nx_secure_dtls_receive_queue_head = packet_ptr;
        }
        else
        {
            temp_packet_ptr = dtls_session -> nx_secure_dtls_receive_queue_head;
            while(temp_packet_ptr -> nx_packet_queue_next != NX_NULL)
            {
                temp_packet_ptr = temp_packet_ptr -> nx_packet_queue_next;
            }

            temp_packet_ptr -> nx_packet_queue_next = packet_ptr;
        }

        /* Make sure the queue pointer is cleared. */
        packet_ptr -> nx_packet_queue_next = NX_NULL;

        /* Is there any thread waiting for packet? */
        if (dtls_session -> nx_secure_dtls_thread_suspended)
        {
            /* Yes. Just abort it. */
            tx_thread_wait_abort(dtls_session -> nx_secure_dtls_thread_suspended);
            dtls_session -> nx_secure_dtls_thread_suspended = NX_NULL;
        }

        /* If the handshake isn't finished, don't notify application. */
        if (dtls_session -> nx_secure_dtls_tls_session.nx_secure_tls_server_state < NX_SECURE_TLS_SERVER_STATE_HANDSHAKE_FINISHED)
        {

            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);
            return;
        }

        /* Release the protection. */
        tx_mutex_put(&_nx_secure_tls_protection);

        /* Invoke the session callback to notify application of packet receive. */
        dtls_server -> nx_secure_dtls_receive_notify(dtls_session);
    }
#else
    NX_PARAMETER_NOT_USED(socket_ptr);

    return;
#endif /* NX_SECURE_ENABLE_DTLS */
}

