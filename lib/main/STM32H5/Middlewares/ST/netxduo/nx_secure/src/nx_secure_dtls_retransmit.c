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
/*    _nx_secure_dtls_retransmit                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is  primarily responsible for the retransmission of   */
/*    dropped handshake message flights.                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          DTLS control block            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_udp_socket_send                  Send UDP packet               */
/*    _nxd_udp_socket_source_send           Send UDP packet with specific */
/*                                            source addresss             */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_client_handshake      DTLS client state machine     */
/*    _nx_secure_dtls_server_handshake      DTLS server state machine     */
/*    _nx_secure_dtls_session_start         Actual DTLS session start call*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID  _nx_secure_dtls_retransmit(NX_SECURE_DTLS_SESSION *dtls_session)
{
NX_PACKET *packet_ptr;
NX_PACKET *next_ptr;

    /* Get a pointer to the sent queue. */
    packet_ptr = dtls_session -> nx_secure_dtls_transmit_sent_head;

    /* Check for flights that need to be retransmitted. */
    while (packet_ptr &&
           (packet_ptr != (NX_PACKET *)NX_PACKET_ENQUEUED) &&
           (packet_ptr -> nx_packet_queue_next == (NX_PACKET *)NX_DRIVER_TX_DONE))
    {

        /* Get a pointer to the next packet in the queue. */
        next_ptr = packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next;

        /* Clear the queue next pointer in this packet. */
        packet_ptr -> nx_packet_queue_next =  NX_NULL;

        /* Skip UDP header. */
        packet_ptr -> nx_packet_prepend_ptr += sizeof(NX_UDP_HEADER);
        packet_ptr -> nx_packet_length -= sizeof(NX_UDP_HEADER);

        /* Release the protection. */
        tx_mutex_put(&_nx_secure_tls_protection);

        /* Re-send the packets in the queue. */
        /* If local IP address index is set, call _nxd_udp_socket_source_send
           to ensure the source IP address is correct.  */
        if (dtls_session -> nx_secure_dtls_local_ip_address_index == 0xffffffff)
        {

            /* Send the UDP packet(s) containing our record. */
            _nxd_udp_socket_send(dtls_session -> nx_secure_dtls_udp_socket, packet_ptr,
                                 &dtls_session -> nx_secure_dtls_remote_ip_address,
                                 dtls_session -> nx_secure_dtls_remote_port);
        }
        else
        {

            /* Send the UDP packet(s) containing our record. */
            _nxd_udp_socket_source_send(dtls_session -> nx_secure_dtls_udp_socket, packet_ptr,
                                        &dtls_session -> nx_secure_dtls_remote_ip_address,
                                        dtls_session -> nx_secure_dtls_remote_port,
                                        dtls_session -> nx_secure_dtls_local_ip_address_index);
        }

        /* Get the protection. */
        tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

        /* Advance to the next packet. */
        packet_ptr = next_ptr;
    }

    dtls_session -> nx_secure_dtls_timeout_retries++;
    dtls_session -> nx_secure_dtls_handshake_timeout = NX_SECURE_DTLS_RETRANSMIT_TIMEOUT <<
        (dtls_session -> nx_secure_dtls_timeout_retries * NX_SECURE_DTLS_RETRANSMIT_RETRY_SHIFT);
    if (dtls_session -> nx_secure_dtls_handshake_timeout > NX_SECURE_DTLS_MAXIMUM_RETRANSMIT_TIMEOUT)
    {
        dtls_session -> nx_secure_dtls_handshake_timeout = NX_SECURE_DTLS_MAXIMUM_RETRANSMIT_TIMEOUT;
    }
}
#endif /* NX_SECURE_ENABLE_DTLS */

