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
/*    _nx_tcp_packet_send_rst                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends a RST from the specified socket.                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to socket             */
/*    header_ptr                            Pointer to received header    */
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
/*    _nx_tcp_socket_disconnect             Disconnect processing         */
/*    _nx_tcp_socket_state_syn_received     Socket SYN received processing*/
/*    _nx_tcp_socket_state_syn_sent         Socket SYN sent processing    */
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
VOID  _nx_tcp_packet_send_rst(NX_TCP_SOCKET *socket_ptr, NX_TCP_HEADER *header_ptr)
{

    /* Reset Generation, RFC793, Section3.4, Page37, the RST packet is set up based on if the incoming packet has the ACK bit set. */
    /* If the incoming segment has an ACK field, the reset takes its sequence number from the ACK field of the segment,
       otherwise the reset has sequence number zero and the ACK field is set to the sum of the sequence number and segment length of the incoming segment.  */

    /* Check for the ACK bit in the incoming TCP header.  */
    if (header_ptr -> nx_tcp_header_word_3 & NX_TCP_ACK_BIT)
    {
        _nx_tcp_packet_send_control(socket_ptr, NX_TCP_RST_BIT, header_ptr -> nx_tcp_acknowledgment_number,
                                    0, 0, 0, NX_NULL);
    }
    else
    {
        _nx_tcp_packet_send_control(socket_ptr, (NX_TCP_RST_BIT | NX_TCP_ACK_BIT), 0,
                                    header_ptr -> nx_tcp_sequence_number, 0, 0, NX_NULL);
    }

#ifndef NX_DISABLE_TCP_INFO
    /* Increment the resets sent count.  */
    socket_ptr -> nx_tcp_socket_ip_ptr -> nx_ip_tcp_resets_sent++;
#endif /* NX_DISABLE_TCP_INFO */
}

