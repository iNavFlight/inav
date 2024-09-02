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
#include "nx_tcp.h"
#include "nx_ipv6.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_socket_state_last_ack                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes packets during the LAST ACK state,          */
/*    which is the state at the end of a passive disconnect, i.e. a       */
/*    disconnect issued by the other side of the connection.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to owning socket      */
/*    tcp_header_ptr                        Pointer to packet header      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_socket_thread_resume          Resume suspended thread       */
/*    _nx_tcp_socket_block_cleanup          Cleanup the socket block      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcp_socket_packet_process         Process TCP packet for socket */
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
VOID  _nx_tcp_socket_state_last_ack(NX_TCP_SOCKET *socket_ptr, NX_TCP_HEADER *tcp_header_ptr)
{

    /* Determine if the incoming message is an ACK message.  */
    if (tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_ACK_BIT)
    {

        /*   If it is proper, finish the disconnect. */
        if ((tcp_header_ptr -> nx_tcp_acknowledgment_number == socket_ptr -> nx_tcp_socket_tx_sequence) &&
            (tcp_header_ptr -> nx_tcp_sequence_number == socket_ptr -> nx_tcp_socket_rx_sequence))
        {

            /* Cleanup the transmission control block.  */
            _nx_tcp_socket_block_cleanup(socket_ptr);

            /* Determine if we need to wake a thread suspended on the disconnection.  */
            if (socket_ptr -> nx_tcp_socket_disconnect_suspended_thread)
            {

                /* Resume suspended thread.  */
                _nx_tcp_socket_thread_resume(&(socket_ptr -> nx_tcp_socket_disconnect_suspended_thread), NX_SUCCESS);
            }

#ifndef NX_DISABLE_EXTENDED_NOTIFY_SUPPORT

            /* Is a disconnect callback registered with the TCP socket?  */
            if (socket_ptr -> nx_tcp_disconnect_complete_notify)
            {

                /* Call the application's disconnect_complete callback function. */
                (socket_ptr -> nx_tcp_disconnect_complete_notify)(socket_ptr);
            }
#endif
        }
    }
}

