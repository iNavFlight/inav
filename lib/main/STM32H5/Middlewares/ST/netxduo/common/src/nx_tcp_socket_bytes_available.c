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
#include "nx_packet.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_socket_bytes_available                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function determines the number of bytes available on a TCP     */
/*    socket for reception.                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to the TCP sockete    */
/*    bytes_available                       Number of bytes returned to   */
/*                                             the caller.                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection             */
/*    tx_mutex_put                          Release protection            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
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
UINT  _nx_tcp_socket_bytes_available(NX_TCP_SOCKET *socket_ptr, ULONG *bytes_available)
{
NX_IP         *ip_ptr;
NX_PACKET     *packet_ptr;
UINT           data_size;
NX_TCP_HEADER *header_ptr;
UINT           header_length;
INT            done = 0;

    /* Setup IP pointer. */
    ip_ptr = socket_ptr -> nx_tcp_socket_ip_ptr;

    /* Obtain the IP mutex so we can examine the bound port.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    *bytes_available = 0;


    /* Make sure the TCP connection has been established. */
    if ((socket_ptr -> nx_tcp_socket_state <= NX_TCP_LISTEN_STATE) ||
        (socket_ptr -> nx_tcp_socket_state > NX_TCP_ESTABLISHED))
    {
        /* Release protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        return(NX_NOT_CONNECTED);
    }

    packet_ptr = socket_ptr -> nx_tcp_socket_receive_queue_head;

    if (packet_ptr == NX_NULL)
    {

        /* The receive queue is empty. */
        /* Release protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        return(NX_SUCCESS);
    }

    /* Loop through all the packets on the queue and find out the total
       number of bytes in the rx queue that are available to the applciation. */
    do
    {
        /*lint -e{923} suppress cast of ULONG to pointer.  */
        if (packet_ptr -> nx_packet_queue_next == ((NX_PACKET *)NX_PACKET_READY))
        {

            /* Compute the size of TCP payload in this packet */
            /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
            header_ptr =  (NX_TCP_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

            /* Calculate the header size for this packet.  */
            header_length = (UINT)((header_ptr -> nx_tcp_header_word_3 >> NX_TCP_HEADER_SHIFT) * sizeof(ULONG));

            data_size = (UINT)(packet_ptr -> nx_packet_length - header_length);
            *bytes_available += data_size;

            if (packet_ptr == socket_ptr -> nx_tcp_socket_receive_queue_tail)
            {
                /* Already reached the last packet.  */
                done = 1;
            }
            else
            {
                /* Move on to the next packet. */
                packet_ptr = packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next;
            }
        }
        else
        {
            /* If the packet has not been acked yet, then just return the
               amount of bytes available so far. */
            done = 1;
        }
    } while (!done);

    /* Release protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    return(NX_SUCCESS);
}

