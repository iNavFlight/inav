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
/*    _nx_tcp_socket_receive_queue_flush                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function releases all packets in the specified socket's        */
/*    receive queue.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to socket             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_release                    Release a packet              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcp_socket_connection_reset       Socket connection reset       */
/*                                            processing                  */
/*    _nx_tcp_server_socket_unaccept        Server socket unaccept        */
/*                                            processing                  */
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
VOID  _nx_tcp_socket_receive_queue_flush(NX_TCP_SOCKET *socket_ptr)
{

NX_PACKET *packet_ptr;
NX_PACKET *next_packet_ptr;


    /* Setup packet pointer.  */
    packet_ptr =  socket_ptr -> nx_tcp_socket_receive_queue_head;

    /* Clear the head and the tail pointers.  */
    socket_ptr -> nx_tcp_socket_receive_queue_head =  NX_NULL;
    socket_ptr -> nx_tcp_socket_receive_queue_tail =  NX_NULL;

    /* Loop to clear all the packets out.  */
    while (socket_ptr -> nx_tcp_socket_receive_queue_count)
    {

        /* Pickup the next queued packet.  */
        next_packet_ptr =  packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next;

        /* Mark it as allocated so it will be released.  */
        /*lint -e{923} suppress cast of ULONT to pointer.  */
        packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next =  (NX_PACKET *)NX_PACKET_ALLOCATED;

        /* Release the packet.  */
        _nx_packet_release(packet_ptr);

        /* Move to the next packet.  */
        packet_ptr =  next_packet_ptr;

        /* Decrease the queued packet count.  */
        socket_ptr -> nx_tcp_socket_receive_queue_count--;
    }
}

