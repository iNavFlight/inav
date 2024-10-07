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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_packet_send_ack                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends an ACK from the specified socket.               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to socket             */
/*    tx_sequence                           Transmit sequence number      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_packet_send_control           Send TCP control packet       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcp_fast_periodic_processing      Delayed ACK processing        */
/*    _nx_tcp_periodic_processing           Regular periodic processing   */
/*    _nx_tcp_socket_receive                Packet receive processing     */
/*    _nx_tcp_socket_state_ack_check        Socket state ACK processing   */
/*    _nx_tcp_socket_state_data_check       Socket state date processing  */
/*    _nx_tcp_socket_state_established      Socket state established      */
/*                                            processing                  */
/*    _nx_tcp_socket_state_fin_wait2        Socket state FIN wait-2       */
/*                                            processing                  */
/*    _nx_tcp_socket_state_fin_wait1        Socket state FIN wait         */
/*                                            processing                  */
/*    _nx_tcp_socket_state_syn_sent         Socket state SYN sent         */
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
VOID  _nx_tcp_packet_send_ack(NX_TCP_SOCKET *socket_ptr, ULONG tx_sequence)
{
    _nx_tcp_packet_send_control(socket_ptr, NX_TCP_ACK_BIT, tx_sequence,
                                socket_ptr -> nx_tcp_socket_rx_sequence, 0, 0, NX_NULL);

    /* Setup a new delayed ACK timeout.  */
    socket_ptr -> nx_tcp_socket_delayed_ack_timeout =  _nx_tcp_ack_timer_rate;
}

