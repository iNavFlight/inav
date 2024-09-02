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
/*    _nx_secure_dtls_packet_allocate                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function allocates a packet for DTLS.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          DTLS control block            */
/*    pool_ptr                              Pool to allocate packet from  */
/*    packet_ptr                            Pointer to place allocated    */
/*                                            packet pointer              */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_session_iv_size_get    Get IV size for this session. */
/*    nx_packet_allocate                    NetX Packet allocation call.  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
/*    _nx_secure_dtls_allocate_handshake_packet                           */
/*                                          Allocate DTLS handshake packet*/
/*    _nx_secure_dtls_client_handshake      DTLS client state machine     */
/*    _nx_secure_dtls_server_handshake      DTLS server state machine     */
/*    _nx_secure_dtls_session_end           Actual DTLS session end call. */
/*    _nx_secure_dtls_session_receive       Receive DTLS data             */
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
UINT _nx_secure_dtls_packet_allocate(NX_SECURE_DTLS_SESSION *dtls_session, NX_PACKET_POOL *pool_ptr,
                                     NX_PACKET **packet_ptr, ULONG wait_option)
{
#ifdef NX_SECURE_ENABLE_DTLS
UINT                   status;
ULONG                  packet_type;
USHORT                 iv_size;
NX_SECURE_TLS_SESSION *tls_session;


    if (dtls_session -> nx_secure_dtls_remote_ip_address.nxd_ip_version == NX_IP_VERSION_V4)
    {
        packet_type = NX_IPv4_UDP_PACKET;
    }
    else
    {
        packet_type = NX_IPv6_UDP_PACKET;
    }


    status =  nx_packet_allocate(pool_ptr, packet_ptr, packet_type, wait_option);

    if (status != NX_SUCCESS)
    {
        return(NX_SECURE_TLS_ALLOCATE_PACKET_FAILED);
    }

    if (((ULONG)((*packet_ptr) -> nx_packet_data_end) - (ULONG)((*packet_ptr) -> nx_packet_prepend_ptr)) <
        (NX_SECURE_DTLS_RECORD_HEADER_SIZE + 2u)) /* At least 2 bytes for Alert message. */
    {

        /* Packet buffer is too small. */
        nx_packet_release(*packet_ptr);
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Advance the packet prepend pointer past the record header. */
    (*packet_ptr) -> nx_packet_prepend_ptr += NX_SECURE_DTLS_RECORD_HEADER_SIZE;

    /* Get a pointer to TLS state. */
    tls_session = &dtls_session -> nx_secure_dtls_tls_session;

    /* If TLS session is active, allocate space for the IV that precedes the data in
       certain ciphersuites. */
    if (tls_session -> nx_secure_tls_local_session_active)
    {
        /* Get the size of the IV used by the session cipher. */
        status = _nx_secure_tls_session_iv_size_get(tls_session, &iv_size);

        if (status != NX_SUCCESS)
        {
            return(status);
        }

        if ((iv_size + 2u) > ((ULONG)((*packet_ptr) -> nx_packet_data_end) - (ULONG)((*packet_ptr) -> nx_packet_prepend_ptr)))
        {

            /* Packet buffer is too small to hold IV. */
            nx_packet_release(*packet_ptr);
            *packet_ptr = NX_NULL;
            return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
        }

        /* Don't do anything if no IV is required. */
        if (iv_size > 0)
        {
            /* Pre-allocate space for the session cipher IV and clear it out. */
            NX_SECURE_MEMSET((*packet_ptr) -> nx_packet_prepend_ptr, 0, iv_size);
            (*packet_ptr) -> nx_packet_prepend_ptr += iv_size;
        }
    }

    /* Make sure our append and prepend pointers are pointing to the same thing - when
       the packet is allocated it is "empty" from a user perspective. */
    (*packet_ptr) -> nx_packet_append_ptr = (*packet_ptr) -> nx_packet_prepend_ptr;
    (*packet_ptr) -> nx_packet_length = 0;

    return(NX_SECURE_TLS_SUCCESS);
#else
    NX_PARAMETER_NOT_USED(dtls_session);
    NX_PARAMETER_NOT_USED(pool_ptr);
    NX_PARAMETER_NOT_USED(packet_ptr);
    NX_PARAMETER_NOT_USED(wait_option);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}

