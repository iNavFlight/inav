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
/** NetX Component                                                        */
/**                                                                       */
/**   Transmission Control Protocol (TCP)                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ip.h"
#include "nx_packet.h"
#include "nx_tcp.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_periodic_processing                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes periodic TCP processing for detecting       */
/*    TCP transmit timeouts.  If a transmit timeout occurs, the packet    */
/*    is simply resent and a new timeout is setup.                        */
/*                                                                        */
/*    Note this requires that NX_ENABLE_TCP_KEEPALIVE is enabled when the */
/*    NetX library is built.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_packet_send_ack               Send ACK probe message        */
/*    _nx_tcp_socket_connection_reset       Reset the connection          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ip_thread_entry                   IP helper thread              */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID  _nx_tcp_periodic_processing(NX_IP *ip_ptr)
{

#ifdef NX_ENABLE_TCP_KEEPALIVE
NX_TCP_SOCKET *socket_ptr;
ULONG          sockets;
UINT           keepalive_enabled = NX_FALSE;


    /* Pickup the number of created TCP sockets.  */
    sockets =  ip_ptr -> nx_ip_tcp_created_sockets_count;

    /* Pickup the first socket.  */
    socket_ptr =  ip_ptr -> nx_ip_tcp_created_sockets_ptr;

    /* Get the keepalive status of the current socket. */
    if (socket_ptr)
    {

        keepalive_enabled = socket_ptr -> nx_tcp_socket_keepalive_enabled;
    }

    /* Loop through the created sockets.  */
    while ((sockets--) && socket_ptr)
    {

        /* Is keep alive enabled on this socket? */
        if (keepalive_enabled)
        {

            /* Yes; Check for the socket having a TCP Keepalive timer active.  */
            if ((socket_ptr -> nx_tcp_socket_state == NX_TCP_ESTABLISHED) && (socket_ptr -> nx_tcp_socket_keepalive_timeout))
            {

                /* Decrement the socket's keepalive timeout timer.  */
                socket_ptr -> nx_tcp_socket_keepalive_timeout--;

                /* Determine if the keepalive timeout has expired.  */
                if (!socket_ptr -> nx_tcp_socket_keepalive_timeout)
                {

                    /* Yes, the timeout has expired.  Increment the retries and
                       determine if there are any retries left.  */
                    socket_ptr -> nx_tcp_socket_keepalive_retries++;
                    if (socket_ptr -> nx_tcp_socket_keepalive_retries <= NX_TCP_KEEPALIVE_RETRIES)
                    {

                        /* Yes, there are more retries left.  Set the next timeout expiration
                           to the retry time instead of the initial time.  */
                        socket_ptr -> nx_tcp_socket_keepalive_timeout =  NX_TCP_KEEPALIVE_RETRY;

                        /* Send Keepalive ACK probe to see if the other side of the connection
                           is still there.  We need to decrement the ACK number first for the
                           probe message and then restore the value immediately after the ACK
                           is sent.  */
                        _nx_tcp_packet_send_ack(socket_ptr, (socket_ptr -> nx_tcp_socket_tx_sequence - 1));
                    }
                    else
                    {

                        /* The Keepalive timer retries have failed, enter a CLOSED state
                           via the reset processing.  */
                        _nx_tcp_socket_connection_reset(socket_ptr);
                    }
                }
            }
        }

        /* Move to the next TCP socket.  */
        socket_ptr =  socket_ptr -> nx_tcp_socket_created_next;

        /* Get the socket's keep alive status. */
        keepalive_enabled = socket_ptr -> nx_tcp_socket_keepalive_enabled;
    }
#else
    NX_PARAMETER_NOT_USED(ip_ptr);
#endif
}

