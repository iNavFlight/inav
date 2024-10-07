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

#ifdef NX_SECURE_KEY_CLEAR
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_packet_release                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function releases packet with all data in the packet cleared.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            NX_PACKET to release          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_packet_release                     Release packet                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    nx_secure_dtls_client_handshake.c                                   */
/*    nx_secure_dtls_process_record.c                                     */
/*    nx_secure_dtls_receive_callback.c                                   */
/*    nx_secure_dtls_retransmit_queue_flush.c                             */
/*    nx_secure_dtls_send_handshake_record.c                              */
/*    nx_secure_dtls_send_record.c                                        */
/*    nx_secure_dtls_session_end.c                                        */
/*    nx_secure_dtls_session_receive.c                                    */
/*    nx_secure_dtls_session_reset.c                                      */
/*    nx_secure_dtls_session_start.c                                      */
/*    nx_secure_tls_1_3_client_handshake.c                                */
/*    nx_secure_tls_1_3_server_handshake.c                                */
/*    nx_secure_tls_client_handshake.c                                    */
/*    nx_secure_tls_handshake_process.c                                   */
/*    nx_secure_tls_packet_release.c                                      */
/*    nx_secure_tls_process_record.c                                      */
/*    nx_secure_tls_record_payload_decrypt.c                              */
/*    nx_secure_tls_send_handshake_record.c                               */
/*    nx_secure_tls_server_handshake.c                                    */
/*    nx_secure_tls_session_end.c                                         */
/*    nx_secure_tls_session_receive_records.c                             */
/*    nx_secure_tls_session_renegotiate.c                                 */
/*    nx_secure_tls_session_start.c                                       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_packet_release(NX_PACKET *packet_ptr)
{
NX_PACKET *current_packet;

    /* Clear all data in chained packet. */
    current_packet = packet_ptr;
    while (current_packet)
    {
        NX_SECURE_MEMSET(current_packet -> nx_packet_data_start, 0,
                         (ULONG)current_packet -> nx_packet_data_end -
                         (ULONG)current_packet -> nx_packet_data_start);
        current_packet = current_packet -> nx_packet_next;
    }
    return(nx_packet_release(packet_ptr));
}
#endif /* NX_SECURE_KEY_CLEAR */

