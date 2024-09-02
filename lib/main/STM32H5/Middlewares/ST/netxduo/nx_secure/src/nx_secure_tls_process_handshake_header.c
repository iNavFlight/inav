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
/**    Transport Layer Security (TLS)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_tls.h"



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_process_handshake_header             PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes a TLS Handshake record header, which is     */
/*    at the beginning of each TLS Handshake message, encapsulated within */
/*    the TLS record itself.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_buffer                         Pointer to incoming packet    */
/*    message_type                          Return message type value     */
/*    header_size                           Input size of packet buffer   */
/*                                            Return size of header       */
/*    message_length                        Return length of message      */
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
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*    _nx_secure_tls_server_handshake       TLS server state machine      */
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
/*  04-25-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            removed unused code,        */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_process_handshake_header(UCHAR *packet_buffer, USHORT *message_type,
                                             UINT *header_size, UINT *message_length)
{

    /* Check buffer length. */
    if (*header_size < 4)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* The message being passed in to this function should already be stripped of the TLS header
       so the first byte in the packet/record is our handshake message type. */
    *message_type = packet_buffer[0];
    packet_buffer++;

    /* Get the length of the TLS data. */
    *message_length = (UINT)((packet_buffer[0] << 16) + (packet_buffer[1] << 8) + packet_buffer[2]);

    /* We have extracted 4 bytes of the header. */
    *header_size = 4;

    return(NX_SECURE_TLS_SUCCESS);
}

