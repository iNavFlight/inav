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
/*    _nx_secure_tls_allocate_handshake_packet            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function allocates a packet, positions the prepend_ptr and     */
/*    append_ptr suitable for TLS handshake packets.                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_pool                           The pool to allocate from     */
/*    packet_ptr                            Pointer to the allocated      */
/*                                            packet                      */
/*    wait_option                           Controls timeout actions      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_packet_allocate        Allocate internal TLS packet  */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_server_handshake       TLS Server state machine      */
/*    _nx_secure_tls_client_handshake       TLS Client state machine      */
/*    _nx_secure_tls_session_renegotiate    Renegotiate TLS session       */
/*    _nx_secure_tls_session_start          Start TLS session             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_allocate_handshake_packet(NX_SECURE_TLS_SESSION *tls_session,
                                              NX_PACKET_POOL *packet_pool,
                                              NX_PACKET **packet_ptr, ULONG wait_option)
{
UINT status;

    /* Release the protection before suspending on nx_packet_allocate. */
    tx_mutex_put(&_nx_secure_tls_protection);

    status = _nx_secure_tls_packet_allocate(tls_session, packet_pool, packet_ptr, wait_option);

    /* Get the protection after nx_packet_allocate. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    if (status != NX_SECURE_TLS_SUCCESS)
    {
        return(NX_SECURE_TLS_ALLOCATE_PACKET_FAILED);
    }

    if (((ULONG)((*packet_ptr) -> nx_packet_data_end) - (ULONG)((*packet_ptr) -> nx_packet_prepend_ptr)) <
        NX_SECURE_TLS_HANDSHAKE_HEADER_SIZE)
    {

        /* Packet buffer is too small. */
        nx_packet_release(*packet_ptr);
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Allocate space for the handshake header. */
    (*packet_ptr) -> nx_packet_prepend_ptr += NX_SECURE_TLS_HANDSHAKE_HEADER_SIZE;
    (*packet_ptr) -> nx_packet_append_ptr = (*packet_ptr) -> nx_packet_prepend_ptr;


    return(NX_SECURE_TLS_SUCCESS);
}

