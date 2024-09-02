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

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_dtls_retransmit_queue_flush              PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function flushes the DTLS transmit queue when the appropriate  */
/*    response is received from the remote host, clearing out the         */
/*    previously-sent flight, resetting the queue for the next flight.    */
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
/*    nx_secure_tls_packet_release          Release packet                */
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
/*                                            released packet securely,   */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Timothy Stapko           Modified comment(s),          */
/*                                            fixed packet leak,          */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
VOID  _nx_secure_dtls_retransmit_queue_flush(NX_SECURE_DTLS_SESSION *dtls_session)
{
TX_INTERRUPT_SAVE_AREA
NX_PACKET *packet_ptr;
NX_PACKET *next_packet_ptr;


    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* Setup packet pointer.  */
    packet_ptr =  dtls_session -> nx_secure_dtls_transmit_sent_head;

    /* Clear the head and the tail pointers.  */
    dtls_session -> nx_secure_dtls_transmit_sent_head =  NX_NULL;
    dtls_session -> nx_secure_dtls_transmit_sent_tail =  NX_NULL;

    /* Loop to clear all the packets out.  */
    while (packet_ptr &&
           (packet_ptr != (NX_PACKET *)NX_PACKET_ENQUEUED))
    {

        /* Disable interrupts.  */
        TX_DISABLE

        /* Pickup the next queued packet.  */
        next_packet_ptr =  packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next;

        /* Mark the packet as no longer being in a TCP queue.  */
        /*lint -e{923} suppress cast of ULONG to pointer.  */
        packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next =  (NX_PACKET *)NX_PACKET_ALLOCATED;

        /* Has the packet been transmitted?  */
        if (packet_ptr -> nx_packet_queue_next ==  ((NX_PACKET *)NX_DRIVER_TX_DONE))
        {

            /* Yes, the driver has already released the packet.  */

            /* Restore interrupts.  */
            TX_RESTORE

            /* Release the packet.  */
            nx_secure_tls_packet_release(packet_ptr);
        }
        else
        {

            /* Restore interrupts.  */
            TX_RESTORE
        }

        /* Move to the next packet.  */
        packet_ptr =  next_packet_ptr;

        /* Decrease the queued packet count.  */
        dtls_session -> nx_secure_dtls_transmit_sent_count--;
    }

    /* Release the protection before suspending on event. */
    tx_mutex_put(&_nx_secure_tls_protection);
}
#endif /* NX_SECURE_ENABLE_DTLS */

