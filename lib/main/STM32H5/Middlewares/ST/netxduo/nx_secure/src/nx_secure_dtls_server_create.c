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
/*    _nx_secure_dtls_server_create                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates an instance of a DTLS server to handle        */
/*    incoming DTLS requests on a particular UDP port. Due to the fact    */
/*    that UDP is stateless, DTLS requests from multiple clients can come */
/*    in on a single port while other DTLS sessions are active. Thus, the */
/*    server is needed to maintain active sessions and properly route     */
/*    incoming messages to the proper handler.                            */
/*                                                                        */
/*    The session buffer parameter is used to hold the control blocks for */
/*    all the possible simultaneous DTLS sessions for the DTLS server. It */
/*    should be allocated with a size that is an even multiple of the     */
/*    size of the NX_SECURE_DTLS_SESSION control block structure.         */
/*                                                                        */
/*    To calculate the necessary metadata size, the API                   */
/*    nx_secure_tls_metadata_size_calculate may be used.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            DTLS server control block     */
/*    ip_ptr                                Pointer to IP instance        */
/*    port                                  Server port                   */
/*    timeout                               Timeout value                 */
/*    session_buffer                        DTLS sessions buffer          */
/*    session_buffer_size                   Size of DTLS sessions buffer  */
/*    crypto_table                          Crypto table                  */
/*    crypto_metadata_buffer                Encryption metadata buffer    */
/*    crypto_metadata_size                  Encryption metadata size      */
/*    packet_reassembly_buffer              DTLS reassembly buffer        */
/*    packet_reassembly_buffer_size         Size of reassembly buffer     */
/*    connect_notify                        Callback for new connections  */
/*    receive_notify                        Callback for received data    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_dtls_session_create        Initialize DTLS control block */
/*    nx_udp_socket_create                  Set up UDP socket             */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
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
UINT _nx_secure_dtls_server_create(NX_SECURE_DTLS_SERVER *server_ptr, NX_IP *ip_ptr, UINT port, ULONG timeout,
                                   VOID *session_buffer, UINT session_buffer_size,
                                   const NX_SECURE_TLS_CRYPTO *crypto_table,
                                   VOID *crypto_metadata_buffer, ULONG crypto_metadata_size,
                                   UCHAR *packet_reassembly_buffer, UINT packet_reassembly_buffer_size,
                                   UINT (*connect_notify)(NX_SECURE_DTLS_SESSION *dtls_session, NXD_ADDRESS *ip_address, UINT port),
                                   UINT (*receive_notify)(NX_SECURE_DTLS_SESSION *dtls_session))
{
#ifdef NX_SECURE_ENABLE_DTLS
UINT status;
UINT i;
NX_SECURE_DTLS_SESSION *current_session;
ULONG session_metadata_size;
UINT  session_pkt_buffer_size;
UCHAR *session_pkt_buffer;
UCHAR *session_metadata;
UINT num_sessions;
NX_SECURE_DTLS_SERVER *tail_ptr;

    /* Figure out number of sessions. */
    num_sessions = session_buffer_size / sizeof(NX_SECURE_DTLS_SESSION);

    /* Check sessions buffer size. */
    if(num_sessions == 0)
    {
        return(NX_INVALID_PARAMETERS);
    }

    /* Set server parameters. */
    server_ptr->nx_dtls_server_ip_ptr = ip_ptr;
    server_ptr->nx_dtls_server_listen_port = port;
    server_ptr->nx_dtls_server_sessions_count = num_sessions;
    server_ptr->nx_dtls_server_timeout = timeout;

    /* Set up session buffer. */
    server_ptr->nx_dtls_server_sessions = (NX_SECURE_DTLS_SESSION*)session_buffer;

    /* Setup per-session packet buffer. */
    session_pkt_buffer_size = packet_reassembly_buffer_size / num_sessions;
    session_pkt_buffer = packet_reassembly_buffer;

    /* Get our per-session metadata. */
    session_metadata = crypto_metadata_buffer;
    session_metadata_size = crypto_metadata_size / num_sessions;

    /* Set up UDP socket. */
    status = nx_udp_socket_create(ip_ptr, &(server_ptr->nx_dtls_server_udp_socket), "DTLS Server",
                                  NX_IP_NORMAL, NX_FRAGMENT_OKAY, timeout, 8192);

    /* Store a pointer to our DTLS server instance in our UDP socket.
       This enables the receive callback to access the right server. */
    server_ptr->nx_dtls_server_udp_socket.nx_udp_socket_reserved_ptr = server_ptr;

    if(status != NX_SUCCESS)
    {
        return(status);
    }

    /* Assign callbacks. */
    server_ptr->nx_secure_dtls_receive_notify = receive_notify;
    server_ptr->nx_secure_dtls_connect_notify = connect_notify;

    /* Reset the protocol version override. */
    server_ptr -> nx_dtls_server_protocol_version_override = 0;

    /* Initialize sessions. */
    for(i = 0; i < num_sessions; ++i)
    {
        /* Get the current session. */
        current_session = &(server_ptr->nx_dtls_server_sessions[i]);

        /* Initialize each DTLS session - don't add remote certificate data here. */
        status = _nx_secure_dtls_session_create(current_session, crypto_table, session_metadata, session_metadata_size,
                                                session_pkt_buffer, session_pkt_buffer_size, 0, NX_NULL, 0);

        if(status != NX_SUCCESS)
        {

            /* Delete the created UDP socket.  */
            nx_udp_socket_delete(&(server_ptr->nx_dtls_server_udp_socket));
            return(status);
        }

        /* Assign the parent server instance. */
        current_session->nx_secure_dtls_server_parent = server_ptr;

        /* Assign other parameters. */
        current_session->nx_secure_dtls_local_port = port;
        current_session->nx_secure_dtls_udp_socket = &(server_ptr->nx_dtls_server_udp_socket);

        /* Advance metadata buffer. */
        session_metadata = &session_metadata[session_metadata_size];

        /* Advance packet buffer. */
        session_pkt_buffer = &session_pkt_buffer[session_pkt_buffer_size];
    }

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* Place the new DTLS server control block on the list of created DTLS server. */
    if (_nx_secure_dtls_server_created_ptr)
    {

        /* Pickup tail pointer. */
        tail_ptr = _nx_secure_dtls_server_created_ptr -> nx_dtls_server_created_previous;

        /* Place the new DTLS server control block in the list. */
        _nx_secure_dtls_server_created_ptr -> nx_dtls_server_created_previous = server_ptr;
        tail_ptr -> nx_dtls_server_created_next = server_ptr;

        /* Setup this DTLS server's created links. */
        server_ptr -> nx_dtls_server_created_previous = tail_ptr;
        server_ptr -> nx_dtls_server_created_next = _nx_secure_dtls_server_created_ptr;
    }
    else
    {

        /* The created DTLS server list is empty. Add DTLS server control block to empty list. */
        _nx_secure_dtls_server_created_ptr = server_ptr;
        server_ptr -> nx_dtls_server_created_previous = server_ptr;
        server_ptr -> nx_dtls_server_created_next = server_ptr;
    }
    _nx_secure_dtls_server_created_count++;

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    return(NX_SUCCESS);
#else
    NX_PARAMETER_NOT_USED(server_ptr);
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(port);
    NX_PARAMETER_NOT_USED(timeout);
    NX_PARAMETER_NOT_USED(session_buffer);
    NX_PARAMETER_NOT_USED(session_buffer_size);
    NX_PARAMETER_NOT_USED(crypto_table);
    NX_PARAMETER_NOT_USED(crypto_metadata_buffer);
    NX_PARAMETER_NOT_USED(crypto_metadata_size);
    NX_PARAMETER_NOT_USED(packet_reassembly_buffer);
    NX_PARAMETER_NOT_USED(packet_reassembly_buffer_size);
    NX_PARAMETER_NOT_USED(connect_notify);
    NX_PARAMETER_NOT_USED(receive_notify);


    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}

