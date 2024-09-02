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
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_dtls_process_handshake_header            PORTABLE C      */
/*                                                           6.1.3        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes a DTLS Handshake record header, which is    */
/*    at the beginning of each DTLS Handshake message, encapsulated       */
/*    within the DTLS record itself.                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_buffer                         Pointer to incoming packet    */
/*    message_type                          Return message type value     */
/*    header_size                           Input size of packet buffer   */
/*                                            Return size of header       */
/*    message_length                        Return length of message      */
/*    message_seq                           Return sequence of message    */
/*    fragment_offset                       Return offset of fragment     */
/*    fragment_length                       Return length of fragment     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_client_handshake      DTLS client state machine     */
/*    _nx_secure_dtls_server_handshake      DTLS server state machine     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Timothy Stapko           Modified comment(s),          */
/*                                            improved buffer length      */
/*                                            verification,               */
/*                                            resulting in version 6.1.3  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_dtls_process_handshake_header(UCHAR *packet_buffer, USHORT *message_type,
                                              UINT *header_size, UINT *message_length,
                                              UINT *message_seq, UINT *fragment_offset,
                                              UINT *fragment_length)
{

    /* Check buffer length. */
    if (*header_size < NX_SECURE_DTLS_HANDSHAKE_HEADER_SIZE)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* The message being passed in to this function should already be stripped of the TLS header
       so the first byte in the packet/record is our handshake message type. */
    *message_type = packet_buffer[0];
    packet_buffer++;

    /* Get the length of the TLS data. */
    *message_length = (UINT)((packet_buffer[0] << 16) + (packet_buffer[1] << 8) + packet_buffer[2]);
    packet_buffer += 3;

    /* Extract message sequence number. */
    *message_seq = (UINT)((packet_buffer[0] << 8) + packet_buffer[1]);
    packet_buffer += 2;

    /* Extract fragment offset. */
    *fragment_offset = (UINT)((packet_buffer[0] << 16) + (packet_buffer[1] << 8) + packet_buffer[2]);
    packet_buffer += 3;

    /* Extract fragment length. */
    *fragment_length = (UINT)((packet_buffer[0] << 16) + (packet_buffer[1] << 8) + packet_buffer[2]);

    /* We have extracted the DTLS header. */
    *header_size = NX_SECURE_DTLS_HANDSHAKE_HEADER_SIZE;

    return(NX_SECURE_TLS_SUCCESS);
}
#endif /* NX_SECURE_ENABLE_DTLS */

