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
/*    _nx_secure_tls_packet_allocate                      PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function allocates a packet for TLS application.               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
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
/*    nx_packet_allocate                    NetX Packet allocation call   */
/*    _nx_secure_tls_session_iv_size_get    Get IV size for this session  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
/*    _nx_secure_tls_allocate_handshake_packet                            */
/*                                          Allocate TLS packet           */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*    _nx_secure_tls_server_handshake       TLS server state machine      */
/*    _nx_secure_tls_session_end            End of a session              */
/*    _nx_secure_tls_session_receive_records                              */
/*                                          Receive TLS records           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Timothy Stapko           Modified comment(s), and      */
/*                                            fixed the packet leak,      */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_packet_allocate(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET_POOL *pool_ptr,
                                    NX_PACKET **packet_ptr, ULONG wait_option)
{
UINT   status;
ULONG  packet_type;
USHORT iv_size;

    if (tls_session -> nx_secure_tls_tcp_socket -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V4)
    {
        packet_type = NX_IPv4_TCP_PACKET;
    }
    else
    {
        packet_type = NX_IPv6_TCP_PACKET;
    }

    status =  nx_packet_allocate(pool_ptr, packet_ptr, packet_type, wait_option);


    if (status != NX_SUCCESS)
    {
        return(status);
    }

    if (((ULONG)((*packet_ptr) -> nx_packet_data_end) - (ULONG)((*packet_ptr) -> nx_packet_prepend_ptr)) <
        (NX_SECURE_TLS_RECORD_HEADER_SIZE + 2u)) /* At least 2 bytes for Alert message. */
    {

        /* Packet buffer is too small. */
        nx_packet_release(*packet_ptr);
        *packet_ptr = NX_NULL;
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Advance the packet prepend pointer past the record header. */
    (*packet_ptr) -> nx_packet_prepend_ptr += NX_SECURE_TLS_RECORD_HEADER_SIZE;

    /* If TLS session is active, allocate space for the IV that precedes the data in
       certain ciphersuites. */
    if (tls_session -> nx_secure_tls_local_session_active)
    {
        /* Get the size of the IV used by the session cipher. */
        status = _nx_secure_tls_session_iv_size_get(tls_session, &iv_size);

        if (status != NX_SUCCESS)
        {

            /* Fail to get the size of the IV. */
            nx_packet_release(*packet_ptr);
            *packet_ptr = NX_NULL;
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
}

