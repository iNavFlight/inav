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


/* Bring in externs for caller checking code.  */
NX_CALLER_CHECKING_EXTERNS


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_tcp_socket_info_get                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the socket information get       */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to the TCP socket     */
/*    tcp_packets_sent                      Destination for number of     */
/*                                            packets sent                */
/*    tcp_bytes_sent                        Destination for number of     */
/*                                            bytes sent                  */
/*    tcp_packets_received                  Destination for number of     */
/*                                            packets received            */
/*    tcp_bytes_received                    Destination for number of     */
/*                                            bytes received              */
/*    tcp_retransmit_packets                Destination for number of     */
/*                                            retransmit packets          */
/*    tcp_packets_queued                    Destination for number of     */
/*                                            receive packets queued      */
/*    tcp_checksum_errors                   Destination for number of     */
/*                                            checksum errors             */
/*    tcp_socket_state                      Destination for the current   */
/*                                            socket state                */
/*    tcp_transmit_queue_depth              Destination for number of     */
/*                                            sockets still in transmit   */
/*                                            queue                       */
/*    tcp_transmit_window                   Destination for number of     */
/*                                            bytes in transmit window    */
/*    tcp_receive_window                    Destination for number of     */
/*                                            bytes in receive window     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_socket_info_get               Actual socket information get */
/*                                            function                    */
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
UINT  _nxe_tcp_socket_info_get(NX_TCP_SOCKET *socket_ptr, ULONG *tcp_packets_sent, ULONG *tcp_bytes_sent,
                               ULONG *tcp_packets_received, ULONG *tcp_bytes_received,
                               ULONG *tcp_retransmit_packets, ULONG *tcp_packets_queued,
                               ULONG *tcp_checksum_errors, ULONG *tcp_socket_state,
                               ULONG *tcp_transmit_queue_depth, ULONG *tcp_transmit_window,
                               ULONG *tcp_receive_window)
{

UINT status;


    /* Check for invalid input pointers.  */
    if ((socket_ptr == NX_NULL) || (socket_ptr -> nx_tcp_socket_id != NX_TCP_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Check to see if TCP is enabled.  */
    if (!(socket_ptr -> nx_tcp_socket_ip_ptr) -> nx_ip_tcp_packet_receive)
    {
        return(NX_NOT_ENABLED);
    }

    /* Check for appropriate caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call actual TCP socket information get function.  */
    status =  _nx_tcp_socket_info_get(socket_ptr, tcp_packets_sent, tcp_bytes_sent, tcp_packets_received,
                                      tcp_bytes_received, tcp_retransmit_packets, tcp_packets_queued,
                                      tcp_checksum_errors, tcp_socket_state, tcp_transmit_queue_depth,
                                      tcp_transmit_window, tcp_receive_window);

    /* Return completion status.  */
    return(status);
}

