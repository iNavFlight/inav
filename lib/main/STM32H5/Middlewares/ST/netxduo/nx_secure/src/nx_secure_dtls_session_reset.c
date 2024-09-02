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

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_dtls_session_reset                       PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function resets a DTLS session object, clearing out all data   */
/*    for initialization or re-use.                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          DTLS session control block    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_thread_wait_abort                  Abort wait process            */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*    _nx_secure_tls_session_reset          Clear out the session         */
/*    nx_secure_tls_packet_release          Release packet                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
/*    _nx_secure_dtls_server_stop           Stop DTLS server              */
/*    _nx_secure_dtls_session_delete        Delete the DTLS session       */
/*    _nx_secure_dtls_session_end           End of a session              */
/*    nx_secure_dtls_session_cache_delete   Delete a session              */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            released packet securely,   */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Timothy Stapko           Modified comment(s),          */
/*                                            updated cookie handling,    */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_dtls_session_reset(NX_SECURE_DTLS_SESSION *dtls_session)
{
#ifdef NX_SECURE_ENABLE_DTLS
NX_PACKET *packet_ptr = NX_NULL;
NX_PACKET *next_packet_ptr;

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* UDP doesn't have a persistent state like TCP, so save off IP address index and Port. */
    dtls_session -> nx_secure_dtls_local_ip_address_index = 0xffffffff;
    dtls_session -> nx_secure_dtls_local_port = 0;

    /* Reset remote port and address. */
    dtls_session -> nx_secure_dtls_remote_ip_address.nxd_ip_version = 0;
    dtls_session -> nx_secure_dtls_remote_port = 0;

    /* Reset session state. */
    dtls_session -> nx_secure_dtls_session_in_use = NX_FALSE;

    /* Reset the cookie. */
    dtls_session -> nx_secure_dtls_cookie_length = 0;
    NX_SECURE_MEMSET(dtls_session -> nx_secure_dtls_cookie, 0, sizeof(dtls_session -> nx_secure_dtls_cookie));
    dtls_session -> nx_secure_dtls_client_cookie_ptr = NX_NULL;

    /* Reset the fragment length. */
    dtls_session -> nx_secure_dtls_fragment_length = 0;

    /* Reset the handshake sequence numbers. */
    dtls_session -> nx_secure_dtls_local_handshake_sequence = 0;
    dtls_session -> nx_secure_dtls_remote_handshake_sequence = 0;
    dtls_session -> nx_secure_dtls_expected_handshake_sequence = 0;

    /* Reset the DTLS epoch. */
    dtls_session -> nx_secure_dtls_local_epoch = 0;
    dtls_session -> nx_secure_dtls_remote_epoch = 0;

    /* Is there any thread waiting for packet? */
    if (dtls_session -> nx_secure_dtls_thread_suspended)
    {

        /* Yes. Just abort it. */
        tx_thread_wait_abort(dtls_session -> nx_secure_dtls_thread_suspended);
        dtls_session -> nx_secure_dtls_thread_suspended = NX_NULL;
    }

    /* Reset the receive queue.  */
    if (dtls_session -> nx_secure_dtls_receive_queue_head)
    {
        packet_ptr = dtls_session -> nx_secure_dtls_receive_queue_head;
        dtls_session -> nx_secure_dtls_receive_queue_head = NX_NULL;
    }

    /* Reset the internal TLS session state. */
    _nx_secure_tls_session_reset(&dtls_session -> nx_secure_dtls_tls_session);

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    /* Release the queued packets.  */
    while (packet_ptr)
    {
        next_packet_ptr = packet_ptr -> nx_packet_queue_next;
        nx_secure_tls_packet_release(packet_ptr);
        packet_ptr = next_packet_ptr;
    }

    return(NX_SUCCESS);
#else
    NX_PARAMETER_NOT_USED(dtls_session);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}

